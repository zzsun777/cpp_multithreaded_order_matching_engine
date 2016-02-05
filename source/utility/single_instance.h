#ifndef _SINGLE_INSTANCE_
#define _SINGLE_INSTANCE_

#include <cstdint>
#include <string>
#include <boost/noncopyable.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace utility
{
class SingleInstance : public boost::noncopyable
{
    public:
    
        explicit SingleInstance(int singleInstancePort = 666);
        ~SingleInstance();
     
        // Move ctor deletion
        SingleInstance(SingleInstance&& other) = delete;
        // Move assignment operator deletion
        SingleInstance& operator=(SingleInstance&& other) = delete;
    
        bool operator()();
    
    private:
    
#ifdef __linux__
        int m_socketFD = -1;
        int m_rc;
        uint16_t m_port;
#elif _WIN32
        HANDLE m_mutex;
        unsigned long  m_lastError;
#endif
};

}//namespace
#endif