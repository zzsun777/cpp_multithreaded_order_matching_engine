#ifndef _DATE_TIME_UTILITY_H_
#define _DATE_TIME_UTILITY_H_

#include <string>
#include <chrono>
#include <sstream>
#include <ctime>
#include <cstddef>

namespace utility
{

// Could use anonymous namespace or static keyword since C++11 removed deprecation
// Even though functions here don`t operate on static data
// preferred inline to avoid code bloat

inline std::string getCurrentDateTime(const char* format = "%d-%m-%Y %H:%M:%S")
{
#if defined( _MSC_VER ) || ( __GNUC__ > 4 )
    auto now = std::chrono::system_clock::now();
    auto inTimeT = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&inTimeT), format);
    return ss.str();
#else
    // In C++11 std::put_time does this more easily, but in my tests
    // you need minimum GCC 5.1 , so using C library in this case
    time_t rawTime;
    struct tm * timeInfo;
    const std::size_t buffer_size = 32;
    char buffer[buffer_size];

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(buffer, buffer_size, format, timeInfo);
    std::string ret(buffer);
    return ret;
#endif
}
    
}// namespace

#endif