#include "utf8.h"

#include "exception.h"

int NextChar(std::string_view s, int32_t &chr)
{
    char c = s[0];
    int len;

    if ((c & 0x80) == 0)
    {
        len = 1;
        chr = c;
    }
    else if ((c & 0xe0) == 0xc0)
    {
        len = 2;
        chr = c & 0x1f;
    }
    else if ((c & 0xf0) == 0xe0)
    {
        len = 3;
        chr = c & 0xf;
    }
    else if ((c & 0xf8) == 0xf0)
    {
        len = 4;
        chr = c & 0x7;
    }
    else
    {
        return 0;
    }
    if (s.size() < len)
        return -1;
    for (int i = 1; i < len; ++i)
    {
        if ((s[i] & 0xc0) != 0x80)
        {
            return 0;
        }
        chr = (chr << 6) | (s[i] & 0x3f);
    }
    return len;
}
