#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <utility>

#include <QFileDialog>
#include <QMessageBox>

#include "toc.h"
#include "pdf_toc.h"

static int GsdllStdin(void *instance, char *buf, int len);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui_(new Ui::MainWindow),
                                          pdf_mark_thread_(new PDFMarkThread())
{
    ui_->setupUi(this);

    connect(ui_->open_toc_btn, &QPushButton::clicked, this, &MainWindow::OpenTOCFile);

    connect(ui_->add_pdf_btn, &QPushButton::clicked, this, &MainWindow::OpenPDFFiles);
    connect(ui_->remove_pdf_btn, &QPushButton::clicked, this, &MainWindow::RemovePDFFiles);
    connect(ui_->clear_pdfs_btn, &QPushButton::clicked, this, &MainWindow::ClearPDFFiles);

    connect(ui_->save_output_pdf_btn, &QPushButton::clicked, this, &MainWindow::SelectOutputFile);

    connect(ui_->start_btn, &QPushButton::clicked, this, &MainWindow::StartRun);
    connect(ui_->cancel_btn, &QPushButton::clicked, pdf_mark_thread_, &PDFMarkThread::Cancel);
    connect(ui_->reset_btn, &QPushButton::clicked, this, &MainWindow::ResetInputs);

    connect(pdf_mark_thread_, &PDFMarkThread::OutputMessage, this, &MainWindow::HandleStdout);
    connect(pdf_mark_thread_, &PDFMarkThread::ErrorMessage, this, &MainWindow::HandleStderr);
    connect(pdf_mark_thread_, &PDFMarkThread::Done, this, &MainWindow::DoneAddPDFMark);

    ui_->pdf_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui_->cancel_btn->setDisabled(true);

    pdf_mark_thread_->start();
}

MainWindow::~MainWindow()
{
    delete ui_;
    delete pdf_mark_thread_;
}

void MainWindow::OpenTOCFile()
{
    QString filename = QFileDialog::getOpenFileName(this, "Open the TOC file.");
    if (!filename.isEmpty())
        ui_->toc_file_input->setText(filename);
}

void MainWindow::OpenPDFFiles()
{
    QStringList pdf_files = QFileDialog::getOpenFileNames(this, tr("Open PDF file(s)."));
    for (const auto &f : pdf_files)
    {
        ui_->pdf_list->addItem(f);
    }
}

void MainWindow::SelectOutputFile()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Seletec output file"));
    if (!filename.isEmpty())
        ui_->output_filename->setText(filename);
}

void MainWindow::StartRun()
{
    ui_->output_area->clear();

    // Get toc filename
    std::string toc_file = ui_->toc_file_input->text().trimmed().toStdString();
    if (toc_file.empty())
    {
        QMessageBox::warning(this, "Error", tr("No TOC file selected!"));
        return;
    }

    // Get output filename
    std::string output_file = ui_->output_filename->text().trimmed().toStdString();
    if (output_file.empty())
    {
        QMessageBox::warning(this, "Error", tr("No output file selected!"));
        return;
    }

    // Get the list of PDF filenames.
    int num_pdf_files = ui_->pdf_list->count();
    if (num_pdf_files == 0)
    {
        QMessageBox::warning(this, "Error", tr("No PDF files selected!"));
        return;
    }

    std::vector<std::string> pdf_files;
    pdf_files.reserve(num_pdf_files);

    for (int i = 0; i < num_pdf_files; ++i)
    {
        std::string f = ui_->pdf_list->item(i)->text().trimmed().toStdString();
        if (f.empty())
        {
            QMessageBox::warning(this, "Error", tr("PDF file name should not be empty!"));
            return;
        }
        pdf_files.emplace_back(std::move(f));
    }

    int page_offset = ui_->page_offset_input->value();
    pdf_mark_thread_->AddPDFMark(std::move(toc_file), std::move(output_file), std::move(pdf_files), page_offset);
    SetDisableInput(true);
}

void MainWindow::HandleStdout(const QString &msg)
{
    ui_->output_area->append(msg);
}

void MainWindow::HandleStderr(const QString &msg)
{
    ui_->output_area->append(msg);
}

void MainWindow::DoneAddPDFMark(bool error, const QString &msg)
{
    if (error)
    {
        QMessageBox::warning(this, "Error", msg);
    }
    SetDisableInput(false);
}

void MainWindow::ClearPDFFiles()
{
    ui_->pdf_list->clear();
}

void MainWindow::ResetInputs()
{
    ui_->toc_file_input->clear();
    ui_->pdf_list->clear();
    ui_->output_filename->clear();
    ui_->page_offset_input->setValue(0);

    ui_->output_area->clear();
}

void MainWindow::RemovePDFFiles()
{
    QList<QListWidgetItem *> selected_items = ui_->pdf_list->selectedItems();
    for (QListWidgetItem *item : selected_items)
    {
        delete ui_->pdf_list->takeItem(ui_->pdf_list->row(item));
    }
}

void MainWindow::SetDisableInput(bool disabled)
{
    ui_->toc_file_input->setDisabled(disabled);
    ui_->open_toc_btn->setDisabled(disabled);

    ui_->add_pdf_btn->setDisabled(disabled);
    ui_->remove_pdf_btn->setDisabled(disabled);
    ui_->clear_pdfs_btn->setDisabled(disabled);

    ui_->output_filename->setDisabled(disabled);
    ui_->save_output_pdf_btn->setDisabled(disabled);

    ui_->page_offset_input->setDisabled(disabled);
    ui_->start_btn->setDisabled(disabled);
    ui_->reset_btn->setDisabled(disabled);
    ui_->cancel_btn->setDisabled(!disabled);
}

PDFMarkThread::~PDFMarkThread()
{
    ++cancel_;
    {
        QMutexLocker locker(&mux_);
        status_ = Status::kQuit;
    }
    cond_.notify_one();
    wait();
}

void PDFMarkThread::run()
{
    while (true)
    {
        mux_.lock();
        while (status_ == Status::kIdle)
            cond_.wait(&mux_);

        if (status_ == Status::kQuit)
            break;

        std::string toc_file(std::move(toc_file_));
        std::string pdf_output(std::move(pdf_output_));
        std::vector<std::string> pdf_files(std::move(pdf_files_));
        int page_offset = page_offset_;

        status_ = Status::kIdle;
        mux_.unlock();

        cancel_ = 0;

        try
        {
            TOC toc = ParseTOCFile(toc_file);
            ::AddPDFMark(toc, page_offset, pdf_files, pdf_output, GsdllStdout, GsdllStderr, CancleFunc, this);
            emit Done(false, "");
        }
        catch (const std::exception &e)
        {
            emit Done(true, QString::fromStdString(std::string("Error: ") + e.what()));
        }
    }
}

void PDFMarkThread::AddPDFMark(std::string toc_file, std::string pdf_output, std::vector<std::string> pdf_files, int page_offset)
{
    {
        QMutexLocker locker(&mux_);
        toc_file_ = std::move(toc_file);
        pdf_output_ = std::move(pdf_output);
        pdf_files_ = std::move(pdf_files);
        page_offset_ = page_offset;
        status_ = Status::kNewJob;
    }
    cond_.notify_one();
}

void PDFMarkThread::Cancel()
{
    cancel_ = 1;
}

int PDFMarkThread::GsdllStdout(void *instance, const char *str, int len)
{
    auto window = static_cast<PDFMarkThread *>(instance);
    emit window->OutputMessage(QString::fromUtf8(str, len));
    return len;
}

int PDFMarkThread::GsdllStderr(void *instance, const char *str, int len)
{
    auto window = static_cast<PDFMarkThread *>(instance);
    emit window->ErrorMessage(QString::fromUtf8(str, len));
    return len;
}

int PDFMarkThread::CancleFunc(void *caller_handle)
{
    return static_cast<PDFMarkThread *>(caller_handle)->cancel_;
}
