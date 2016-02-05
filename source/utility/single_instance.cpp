#include "single_instance.h"

#ifdef __linux__
#include <unistd.h>
#include <netinet/in.h>
#endif

using namespace std;

namespace utility
{
    
SingleInstance::SingleInstance(int singleInstancePort)
{
#ifdef __linux__
    m_socketFD = -1;
    m_rc = 1;
    m_port = singleInstancePort;
#elif _WIN32
    m_mutex = CreateMutex(NULL, FALSE, "VERY_UNIQUE"); //do early
    m_lastError = GetLastError(); //save for use later...
#endif    
}

SingleInstance::~SingleInstance()
{
#ifdef __linux__
    if (m_socketFD != -1)
    {
        close(m_socketFD);
    }
#elif _WIN32
    if (m_mutex)
    {
        CloseHandle(m_mutex); 
        m_mutex = NULL; 
    }
#endif  
}

bool SingleInstance::operator()()
{
#ifdef __linux__
    if (m_socketFD == -1 || m_rc)
    {
        m_socketFD = -1;
        m_rc = 1;

        if ((m_socketFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        {
            //Could not create socket
            return false;
        }
        else
        {
            struct sockaddr_in name;
            name.sin_family = AF_INET;
            name.sin_port = htons (m_port);
            name.sin_addr.s_addr = htonl (INADDR_ANY);
            m_rc = bind (m_socketFD, (struct sockaddr *) &name, sizeof (name));
        }
    }
    return (m_socketFD != -1 && m_rc == 0);
#elif _WIN32
    return (ERROR_ALREADY_EXISTS == m_lastError)?false:true;
#endif  
}

}//namespace