#ifndef _STRING_UTILITY_H_
#define _STRING_UTILITY_H_

#include <cctype>
#include <string>
#include <algorithm>

namespace utility
{

inline bool replaceInString(std::string& str, const std::string& from, const std::string& to)
{
    auto start_pos = str.find(from);

    if (start_pos == std::string::npos)
    {
        return false;
    }

    str.replace(start_pos, from.length(), to);
    return true;
}

template <typename T>
inline T lowercase(const T& input)
{
    T ret = input;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
}

template <typename T>
inline T uppercase(const T& input)
{
    T ret = input;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
    return ret;
}
    
}// namespace

#endif