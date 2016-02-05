#ifndef __TRACE_H__
#define __TRACE_H__

#include <cstring>
#include <cstdio>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <syslog.h>
#include <unistd.h>
#endif

namespace utility
{

inline void trace(const char* message, ...)
{
    auto message_length = std::strlen(message);
    //Only C++14 in the project , since they couldn`t make make_unique into C++11 in time...
    std::unique_ptr<char> buffer = make_unique<char>(new  char[message_length]);
    
    va_list args;
    va_start(args, message);
    vsnprintf(buffer.get(), message_length, message, args);
    va_end(args);
#ifdef _WIN32
    OutputDebugStringA(buffer.get());
    OutputDebugStringA("\n");
#else __linux__
    openlog("slog", LOG_PID|LOG_CONS, LOG_USER);
    syslog(LOG_INFO, buffer.get() );
    closelog();
#endif
}

}// namespace

#endif