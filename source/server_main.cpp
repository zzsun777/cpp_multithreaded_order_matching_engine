#include <exception>
#include <iostream>
#include <cstdlib>
using namespace std;

#include <utility/config_file.h>
#include <utility/single_instance.h>
#include <utility/logger.h>
#include <utility/file_utility.h>
#include <server/server.h>

void onError(const string& message, int exit_code);

namespace program_errors
{
    const int INVALID_CONFIG_FILE = 1;
    const int ALREADY_RUNNING = 2;
    const int RUNTIME_ERROR = 3;
    const int INSUFFICIENT_MEMORY = 4;
    const int UNKNOWN_PROBLEM = 5;
}


int main ()
{
    ///////////////////////////////////////////
    // Load configuration file
    vector<string> symbols;
    bool hyperThreading{true};
    bool centralOrderPinThreadsToCores{false};
    int centralOrderBookQueueSizePerThread{0};
    int centralOrderBookThreadStackSize{0};
    int singleInstanceTCPPortNumber{0};

    try
    {
        utility::ConfigFile configuration;
        configuration.loadFromFile("ome.ini");

        singleInstanceTCPPortNumber = configuration.getIntVaue("SINGLE_INSTANCE_TCP_PORT");
        
        utility::Logger::getInstance().initialise(configuration.getIntVaue("LOG_BUFFER_SIZE"));
        utility::Logger::getInstance().setLogFile("oms_log.txt");
        utility::Logger::getInstance().enableFileLogging(configuration.getBoolValue("FILE_LOGGING_ENABLED"));
        utility::Logger::getInstance().enableConsoleOutput(configuration.getBoolValue("CONSOLE_OUTPUT_ENABLED"));

        symbols = configuration.getArray("SYMBOL");
        if (symbols.size() == 0) { throw std::runtime_error("No symbol found in the ini file"); }

        centralOrderPinThreadsToCores = configuration.getBoolValue("CENTRAL_ORDER_BOOK_PIN_THREADS_TO_CORES");
        hyperThreading = configuration.getBoolValue("HYPER_THREADING");
        centralOrderBookQueueSizePerThread = configuration.getIntVaue("CENTRAL_ORDER_BOOK_WORK_QUEUE_SIZE_PER_THREAD");
        centralOrderBookThreadStackSize = configuration.getIntVaue("CENTRAL_ORDER_BOOK_THREAD_STACK_SIZE");
    }
    catch (std::invalid_argument & e)
    {
        onError(e.what(), program_errors::INVALID_CONFIG_FILE);
    }
    catch (std::runtime_error & e)
    {
        onError(e.what(), program_errors::INVALID_CONFIG_FILE);
    }
    
     //////////////////////////////////////////
    // Single instance protection
    utility::SingleInstance singleton(singleInstanceTCPPortNumber);
    
    if ( !singleton() )
    {
        onError("Ome process is running already.", program_errors::ALREADY_RUNNING);
    }

    //////////////////////////////////////////
    // Backup FIX engine logs if exists
    utility::createDirectory("old_quickfix_logs");
    utility::backupDirectory("quickfix_log", "quickfix_log_" + utility::getCurrentDateTime("%d_%m_%Y_%H_%M_%S"), "old_quickfix_logs");
    //////////////////////////////////////////
    try
    {
        // Start logger
        utility::Logger::getInstance().start();
        LOG_INFO("Main thread", "starting")

        Server application( "quickfix_server.cfg",
                          centralOrderPinThreadsToCores,
                          centralOrderBookThreadStackSize,
                          hyperThreading,
                          centralOrderBookQueueSizePerThread,
                          symbols
                          );

        // Run the server
        application.run();
    }
    catch (std::invalid_argument & e)
    {
        onError(e.what(), program_errors::RUNTIME_ERROR);
    }
    catch (std::runtime_error & e)
    {
        onError(e.what(), program_errors::RUNTIME_ERROR);
    }
    catch (std::bad_alloc & )
    {
        onError("Insufficient memory", program_errors::INSUFFICIENT_MEMORY);
    }
    catch (...)
    {
        onError("Unknown exception occured", program_errors::UNKNOWN_PROBLEM);
    }
    //////////////////////////////////////////
    // Application exit
    LOG_INFO("Main thread", "Ending")
    utility::Logger::getInstance().shutdown();
    return 0;
}

void onError(const string& message, int exit_code)
{
    std::cerr << message << std::endl;
    if ( utility::Logger::getInstance().isAlive() )
    {
        LOG_ERROR("Main thread", "Ending")
        utility::Logger::getInstance().shutdown();
    }
    std::exit(exit_code);
}