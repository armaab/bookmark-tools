#pragma once

#include <cstdio>
#include <string>

class TmpFile
{
public:
    TmpFile() { file_name_ = std::tmpnam(nullptr); }
    ~TmpFile() { std::remove(file_name_.c_str()); }
    const std::string &GetName() const { return file_name_; }

protected:
    std::string file_name_;
};

void str_trim(std::string &s, const std::string &whitespace = " \n\r\t\f\v");
