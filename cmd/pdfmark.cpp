#include <exception>
#include <iostream>

#include "pdf_toc.h"
#include "toc.h"

using namespace std;

static int gsdll_stdout(void *instance, const char *str, int len)
{
    fwrite(str, 1, len, stdout);
    fflush(stdout);
    return len;
}

static int gsdll_stderr(void *instance, const char *str, int len)
{
    fwrite(str, 1, len, stderr);
    fflush(stderr);
    return len;
}

int main(int argc, const char **argv)
{
    vector<string> pdf_files = {"in.pdf"};
    try
    {
        TOC toc = ParseTOCFile("toc");
        AddPDFMark(toc, 10, pdf_files, "output.pdf", gsdll_stdout,
                   gsdll_stderr, nullptr, nullptr);
    }
    catch (const std::exception &e)
    {
        cout << "error: " << e.what() << endl;
    }
    return 0;
}
