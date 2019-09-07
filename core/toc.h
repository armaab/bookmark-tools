#pragma once

#include <string>
#include <utility>
#include <vector>

struct TOCEntry
{
    std::string title;
    int page;
    std::vector<TOCEntry> children;

    TOCEntry() = default;
    TOCEntry(TOCEntry &&) = default;
    TOCEntry(std::string title, int page) : title(std::move(title)), page(page) {}
};

struct TOC
{
    std::vector<TOCEntry> entries;
};

TOC ParseTOCFile(const std::string &filename);
