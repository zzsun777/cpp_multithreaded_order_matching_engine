#include "logger.h"
#include <iostream>
using namespace std;
#include <utility/pretty_exception.h>

namespace utility
{

void Logger::initialise(size_t bufferSize)
{
    assert(bufferSize>0);
    m_buffer.reset(new concurrent::RingBufferMPMC<LogEntry>(bufferSize));
}

void Logger::setLogFile(const string& fileName) throw(std::runtime_error)
{
    //std::lock_guard<std::mutex> scoped_lock(m_logFileLock);

    if (m_logFile.is_open())
    {
        m_logFile.close();
    }

    m_logFile.open(fileName);

    if (! m_logFile.is_open())
    {
        THROW_PRETTY_RUNTIME_EXCEPTION(boost::str(boost::format("Log file %s can`t be opened") % fileName.c_str()))
    }
}

void Logger::log(LogLevel level, const string& sender, const string& message, bool consoleOnly)
{
    LogEntry entry(level, sender, message, consoleOnly);
    m_buffer->push(entry);
}

bool Logger::processLogs()
{
    bool processedAny = false;
    while(m_buffer->count() > 0)
    {
        processedAny = true;
        auto log = m_buffer->pop();

        if (m_consoleOutputEnabled)
        {
            cout << log << endl;
        }

        if (m_fileLoggingEnabled && log.m_consoleOnly == false)
        {
            //std::lock_guard<std::mutex> scoped_lock(m_logFileLock);
            m_logFile << log << endl;
        }
    }
    return processedAny;
}

void* Logger::run()
{
    while ( true )
    {
        if (   ( m_fileLoggingEnabled || m_consoleOutputEnabled ) )
        {
            processLogs();
            
            if (isFinishing() == true )
            {
                processLogs();
                break;
            }
            
        }
        else
        {
            concurrent::Thread::yield();
        }
    }

    m_logFile.close();
    return nullptr;
}

}//namespace