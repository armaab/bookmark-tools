#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <atomic>
#include <optional>
#include <string>
#include <vector>

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

namespace Ui
{
class MainWindow;
}

class PDFMarkThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void OpenTOCFile();
    void OpenPDFFiles();
    void SelectOutputFile();
    void StartRun();
    void HandleStdout(const QString &msg);
    void HandleStderr(const QString &msg);
    void DoneAddPDFMark(bool error, const QString &msg);
    void ClearPDFFiles();
    void ResetInputs();
    void RemovePDFFiles();

private:
    void SetDisableInput(bool disabled);

private:
    Ui::MainWindow *ui_;
    PDFMarkThread *pdf_mark_thread_;
};


class PDFMarkThread : public QThread
{
    Q_OBJECT

public:
    PDFMarkThread() {};
    ~PDFMarkThread();

    void AddPDFMark(std::string toc_file, std::string pdf_output, std::vector<std::string> pdf_files, int page_offset);

protected:
    void run() override;

signals:
    void OutputMessage(const QString &s);
    void ErrorMessage(const QString &s);
    void Done(bool error, const QString &msg);

public slots:
    void Cancel();

private:
    static int CancleFunc(void *caller_handle);
    static int GsdllStdout(void *instance, const char *buf, int len);
    static int GsdllStderr(void *instance, const char *str, int len);

private:
    enum class Status
    {
        kQuit,
        kNewJob,
        kIdle,
    };

private:
    std::atomic_int cancel_ = 0;

    QMutex mux_;
    QWaitCondition cond_;

    std::string toc_file_;
    std::string pdf_output_;
    std::vector<std::string> pdf_files_;
    int page_offset_;
    Status status_ = Status::kIdle;
};

#endif // MAINWINDOW_H
