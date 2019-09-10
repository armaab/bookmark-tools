#include "utils.h"

void str_trim(std::string &s, const std::string &whitespace)
{
    size_t start_pos = s.find_first_not_of(whitespace);
    size_t end_pos = s.find_last_not_of(whitespace);

    if (start_pos == std::string::npos || end_pos == std::string::npos)
    {
        s.clear();
        return;
    }

    size_t len = end_pos - start_pos + 1;
    for (size_t i = 0; i < len; ++i)
        s[i] = s[i + start_pos];

    s.resize(len);
}
