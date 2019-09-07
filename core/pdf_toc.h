#pragma once

#include <string>
#include <vector>

#include "toc.h"

using OutFunc = int(void *caller_handle, const char *str, int len);
using PollFunc = int(void *caller_handle);

int AddPDFMark(const TOC &toc, int page_offset,
               const std::vector<std::string> &pdf_files,
               const std::string &pdf_out, OutFunc std_out,
               OutFunc std_err, PollFunc poll_func, void *caller_handle);
