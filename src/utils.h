#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <cctype>

static inline void split(const std::string &s, const char delimiter, std::vector<std::string> *result)
{

    size_t start = -1, i = 0;
    while (i < s.length())
    {
        if (!isspace(s[i]))
        {
            start = i++;
            while (i < s.length() && !isspace(s[i]))
                i++;
            result->push_back(s.substr(start, i - start));
        }
        i++;
    }
}