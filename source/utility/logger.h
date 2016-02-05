#ifndef __LOGGER__
#define __LOGGER__

#include <boost/format.hpp>

#include <cstddef>
#include <string>
#include <fstream>
#include <atomic>
#include <mutex>

#include <concurrent/actor.h>
#include <concurrent/ring_buffer_mpmc.hpp>

#include "datetime_utility.h"
#include "singleton.hpp"

namespace utility
{

#define LOG_CONSOLE(SENDER, MESSAGE) (utility::Logger::getInstance().log(utility::LogLevel::LEVEL_INFO,(SENDER),(MESSAGE),true));
#define LOG_INFO(SENDER, MESSAGE) (utility::Logger::getInstance().log(utility::LogLevel::LEVEL_INFO,(SENDER),(MESSAGE)));
#define LOG_WARNING(SENDER, MESSAGE) (utility::Logger::getInstance().log(utility::LogLevel::LEVEL_WARNING,(SENDER),(MESSAGE)));
#define LOG_ERROR(SENDER, MESSAGE) (utility::Logger::getInstance().log(utility::LogLevel::LEVEL_ERROR,(SENDER),(MESSAGE)));
    
enum class LogLevel { LEVEL_INFO, LEVEL_WARNING, LEVEL_ERROR };

#define DEFAULT_LOGGER_RING_BUFFER_SIZE 819200

struct LogEntry
{
    LogLevel m_logLevel;
    std::string m_sender;
    std::string m_message;
    bool m_consoleOnly; // Useful to have this as outputting directly to console is not sync with the logger thread
    LogEntry(){};
    LogEntry(LogLevel level, std::string sender, std::string message, bool consoleOnly) : m_logLevel(level), m_sender(sender), m_message(message), m_consoleOnly(consoleOnly)
    {}

    friend std::ostream& operator<<(std::ostream& os, LogEntry& entry)
    {
        if (entry.m_consoleOnly == true)
        {
            os << boost::str(boost::format("%s") % entry.m_message);
            return os;
        }
        std::string logLevel;

        switch (entry.m_logLevel)
        {
        case LogLevel::LEVEL_INFO:
            logLevel = "INFO";
            break;

        case LogLevel::LEVEL_WARNING:
            logLevel = "WARNING";
            break;

        case LogLevel::LEVEL_ERROR:
            logLevel = "ERROR";
            break;
        }

        os << boost::str(boost::format("%s : %s , %s , %s") % getCurrentDateTime() % logLevel % entry.m_sender % entry.m_message);
        return os;
    }
};

using LogBuffer = std::unique_ptr< concurrent::RingBufferMPMC<LogEntry> >;

class Logger : public concurrent::Actor, public Singleton<Logger>
{    
    public :
        Logger() : Actor("LoggerThread") {}
        ~Logger() { shutdown(); }

        //static Logger* getInstance();
        void initialise(std::size_t bufferSize = DEFAULT_LOGGER_RING_BUFFER_SIZE);
        void enableFileLogging(bool value) { m_fileLoggingEnabled.store(value); }
        void enableConsoleOutput(bool value) { m_consoleOutputEnabled.store(value); }
        void setLogFile(const std::string& fileName);
        void log(LogLevel level, const std::string& sender, const std::string& message, bool consoleOnly=false);
        void* run() override;

    private:
        // Move ctor deletion
        Logger(Logger&& other) = delete;
        // Move assignment operator deletion
        Logger& operator=(Logger&& other) = delete;
        bool processLogs();

        std::ofstream m_logFile;
        std::mutex m_logFileLock;
        LogBuffer m_buffer;
        std::atomic<bool> m_fileLoggingEnabled;
        std::atomic<bool> m_consoleOutputEnabled;
};

}//namespace
#endif