#include "pdf_toc.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include <ghostscript/iapi.h>
#include <ghostscript/ierrors.h>

#include "exception.h"
#include "utf8.h"
#include "utils.h"

static const char *HEX_TABLE = "0123456789ABCDEF";

static const char *IGNORE_GS = R"(
% store the original pdfmark
/originalpdfmark { //pdfmark } bind def

% replace pdfmark with a wrapper that ignores OUT
/pdfmark
{
  {  % begin loop

      { counttomark pop }
    stopped
      { /pdfmark errordict /unmatchedmark get exec stop }
    if

    dup type /nametype ne
      { /pdfmark errordict /typecheck get exec stop }
    if

    dup /OUT eq
      { (Skipping OUT pdfmark\n) print cleartomark exit }
    if

    originalpdfmark exit

  } loop
} def
)";

static void WriteTitle(const std::string &title, int line_num,
                       std::ostream &out)
{
    bool has_unicode = false;
    for (char c : title)
    {
        if ((c & 0x80) != 0)
        {
            has_unicode = true;
            break;
        }
    }
    if (!has_unicode)
    {
        out << '(';
        for (char c : title)
        {
            switch (c)
            {
            case '(':
                out << "\\(";
                break;
            case ')':
                out << "\\)";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\t':
                out << "\\t";
                break;
            case '\b':
                out << "\\b";
                break;
            case '\f':
                out << "\\f";
                break;
            default:
                out << c;
            }
        }
        out << ')';
        return;
    }
    out << "<FEFF";
    int pos = 0;
    int n = title.size();
    while (pos < n)
    {
        int32_t chr;
        int len = NextChar(std::string_view(&title[pos], n - pos), chr);
        if (len == 0 || chr < 0 || chr > 0x10ffff)
        {
            std::ostringstream buffer;
            buffer << "invalid utf8: " << (&title[pos]);
            throw InvalidUTF8(line_num, buffer.str());
        }
        if (chr <= 0xd7ff || (chr >= 0xe000 && chr <= 0xffff))
        {
            out << HEX_TABLE[(chr & 0xf000) >> 12] << HEX_TABLE[(chr & 0xf00) >> 8]
                << HEX_TABLE[(chr & 0xf0) >> 4] << HEX_TABLE[chr & 0xf];
        }
        else if (chr >= 0x100000)
        {
            chr -= 0x100000;
            int32_t hchr = (chr & 0xffc00) >> 10;
            int32_t lchr = chr & 0x3ff;
            out << "D8" << HEX_TABLE[(hchr & 0xf0) >> 4] << HEX_TABLE[hchr & 0xf];
            out << "DC" << HEX_TABLE[(lchr & 0xf0) >> 4] << HEX_TABLE[lchr & 0xf];
        }
        pos += len;
    }
    out << '>';
}

static void WritePDFTOC(const std::vector<TOCEntry> &list, int page_offset,
                        std::ostream &out, int &line_num)
{
    for (const auto &e : list)
    {
        if (!e.children.empty())
        {
            out << "[/Count -" << e.children.size() << " /Title ";
        }
        else
        {
            out << "[/Title ";
        }
        WriteTitle(e.title, line_num, out);
        out << " /Page " << (e.page + page_offset) << " /OUT pdfmark\n";
        ++line_num;

        if (!e.children.empty())
        {
            WritePDFTOC(e.children, page_offset, out, line_num);
        }
    }
}

int AddPDFMark(const TOC &toc, int page_offset,
               const std::vector<std::string> &pdf_files,
               const std::string &pdf_out, OutFunc std_out,
               OutFunc std_err, PollFunc poll_func, void *caller_handle)
{
    // Write pdfmark to a tmp file.
    TmpFile toc_file;
    TmpFile ignore_gs_file;
    {
        std::ofstream toc_stream(toc_file.GetName());
        int line_num = 1;
        toc_stream << "/pdfmark { originalpdfmark } bind def\n";
        WritePDFTOC(toc.entries, page_offset, toc_stream, line_num);

        std::ofstream ignore_gs_stream(ignore_gs_file.GetName());
        ignore_gs_stream << IGNORE_GS;
    }

    // Prepare arguments to gs.
    std::vector<std::string> gsargs;
    gsargs.emplace_back("gs");
    gsargs.emplace_back("-dNOPAUSE");
    gsargs.emplace_back("-dBATCH");
    gsargs.emplace_back("-dSAFER");
    gsargs.emplace_back("-sDEVICE=pdfwrite");

    std::ostringstream buffer;
    buffer << "-sOutputFile=" << pdf_out;
    gsargs.emplace_back(buffer.str());

    gsargs.reserve(gsargs.size() + pdf_files.size() + 2);
    gsargs.emplace_back(ignore_gs_file.GetName());
    gsargs.insert(gsargs.end(), pdf_files.begin(), pdf_files.end());
    gsargs.emplace_back(toc_file.GetName());

    std::vector<char *> gsargs_cstr;
    gsargs_cstr.resize(gsargs.size());

    for (int i = 0; i < gsargs.size(); ++i)
    {
        gsargs_cstr[i] = &gsargs[i][0];
    }

    // Invoke gs.
    void *minst = nullptr;
    int code, code1;
    code = gsapi_new_instance(&minst, caller_handle);
    if (code < 0)
        return 1;
    if (poll_func != nullptr)
        gsapi_set_poll(minst, poll_func);
    gsapi_set_stdio(minst, nullptr, std_out, std_err);
    code = gsapi_set_arg_encoding(minst, GS_ARG_ENCODING_UTF8);

    if (code == 0)
        code = gsapi_init_with_args(minst, gsargs_cstr.size(), &gsargs_cstr[0]);

    code1 = gsapi_exit(minst);
    if ((code == 0) || (code == gs_error_Quit))
        code = code1;

    gsapi_delete_instance(minst);
    minst = nullptr;

    if ((code == 0) || (code == gs_error_Quit))
        return 0;
    return 1;
}
