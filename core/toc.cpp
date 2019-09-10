#include "toc.h"

#include <cctype>
#include <fstream>
#include <sstream>

#include "exception.h"
#include "utils.h"

struct Entry
{
    int depth;
    TOCEntry entry;
};

static Entry ParseLine(const std::string &line, int line_num)
{
    int depth = 0;
    while (depth < line.size() && line[depth] == '*')
        ++depth;

    if (depth >= line.size() || line[depth] != '!')
    {
        throw MalformattedTOCFile(line_num, "expected a '!'");
    }

    int end = line.size() - 1;
    while (end >= 0 && std::isspace(static_cast<unsigned char>(line[end])))
        --end;

    int space_end = end;
    int page_num = 0;
    int factor = 1;

    while (space_end >= 0 && std::isdigit(static_cast<unsigned char>(line[space_end])))
    {
        page_num += factor * (line[space_end] - '0');
        factor *= 10;
        --space_end;
    }

    if (line[space_end] == '-')
    {
        page_num = -page_num;
        --space_end;
    }

    if (space_end == end || !std::isspace(static_cast<unsigned char>(line[space_end])))
    {
        throw MalformattedTOCFile(line_num, "no page number found");
    }

    int title_end = space_end - 1;
    while (title_end >= 0 &&
           std::isspace(static_cast<unsigned char>(line[title_end])))
        --title_end;

    int title_len = title_end - depth;
    if (title_len <= 0)
    {
        throw MalformattedTOCFile(line_num, "no title found");
    }

    Entry e;
    e.depth = depth;
    e.entry.page = page_num;
    e.entry.title = std::string(line, depth + 1, title_len);
    return e;
}

static std::vector<Entry> ReadTOCFile(const std::string &filename)
{
    std::vector<Entry> entries;

    std::ifstream input(filename);
    std::string line;
    int line_num = 1;

    while (std::getline(input, line))
    {
        str_trim(line);

        if (line.empty())
            continue;

        entries.emplace_back(ParseLine(line, line_num));
        line_num++;
    }
    return entries;
}

static int GenerateTOC(std::vector<TOCEntry> &list, std::vector<Entry> &&entries,
                       int cur_depth = 0, int start_idx = 0)
{
    int i = start_idx;
    while (i < entries.size() && entries[i].depth == cur_depth)
    {
        TOCEntry &e = entries[i].entry;
        i = GenerateTOC(e.children, std::move(entries), cur_depth + 1, i + 1);
        list.emplace_back(std::move(e));
    }

    if (i < entries.size() && entries[i].depth > cur_depth)
    {
        std::ostringstream buffer;
        buffer << "incorrect depth of TOC entry, expected a value <= " << cur_depth
               << ", got " << entries[i].depth;
        throw MalformattedTOCFile(start_idx + 1, buffer.str());
    }
    return i;
}

TOC ParseTOCFile(const std::string &filename)
{
    std::vector<Entry> entries = ReadTOCFile(filename);
    TOC toc;
    GenerateTOC(toc.entries, std::move(entries));
    return toc;
}
