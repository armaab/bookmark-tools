#pragma once

#include <exception>
#include <sstream>
#include <string>

class Exception : public std::exception
{
public:
    Exception() = default;
    explicit Exception(std::string message) : message_(std::move(message)) {}
    const char *what() const noexcept override { return message_.c_str(); }

protected:
    std::string message_;
};

class MalformattedTOCFile : public Exception
{
public:
    MalformattedTOCFile(int line, const std::string &message)
    {
        std::ostringstream buffer;
        buffer << "mal-formatted TOC file at line " << line << ": " << message;
        message_ = buffer.str();
    }
};

class InvalidUTF8 : public MalformattedTOCFile
{
public:
    InvalidUTF8(int line, const std::string &message)
        : MalformattedTOCFile(line, "invalid utf8: " + message) {}
};
