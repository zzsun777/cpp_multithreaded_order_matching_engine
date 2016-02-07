#include "client_request.h"
#include "client_application.h"

#include <string>
#include <exception>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <boost/format.hpp>

#include <utility/file_utility.h>
#include <utility/string_utility.h>
#include <utility/datetime_utility.h>
#include <utility/pretty_exception.h>

namespace program_errors
{
    const int INVALID_CONFIG_FILE = 1;
    const int ALREADY_RUNNING = 2;
    const int RUNTIME_ERROR = 3;
    const int INSUFFICIENT_MEMORY = 4;
    const int UNKNOWN_PROBLEM = 5;
}

void onError(const string& message, int exit_code);

void createQuickFixConfigFile(const string& templateFile, const string& server, const string &clientName, const string& outputFileName);

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        std::cout << "usage: " << argv[0]
            << " quickfix_config_template_file server_address client_name testcase_file" << std::endl;
        return 0;
    }

    try
    {
        string quickFixTemplateFile = argv[1];
        string targetServer = argv[2];
        string clientName = argv[3];
        string csvTestFile = argv[4];
        
        string quickFixConfigFile = clientName + ".cfg";
        createQuickFixConfigFile(quickFixTemplateFile, targetServer, clientName, quickFixConfigFile);

        if (quickFixConfigFile.length() < 5) { throw std::invalid_argument("Invalid FIX engine config file"); }

        // Backup FIX engine logs if exists
        utility::createDirectory("old_quickfix_logs");
        utility::backupDirectory("quickfix_log", "quickfix_log_" + utility::getCurrentDateTime("%d_%m_%Y_%H_%M_%S"), "old_quickfix_logs");

        // Run the application
        ClientApplication application(csvTestFile, quickFixConfigFile);
        application.run();
        
        utility::deleteFile(quickFixConfigFile);
    }
    catch (std::invalid_argument & e)
    {
        onError(e.what(), program_errors::RUNTIME_ERROR);
    }
    catch (std::runtime_error & e)
    {
        onError(e.what(), program_errors::RUNTIME_ERROR);
    }
    catch (std::bad_alloc& e)
    {
        onError(e.what(), program_errors::INSUFFICIENT_MEMORY);
    }
    catch (...)
    {
        onError("Unknown exception caught", program_errors::UNKNOWN_PROBLEM);
    }

    return 0;
}

void onError(const string& message, int exit_code)
{
    std::cerr << message << std::endl;
    std::exit(exit_code);
}

void createQuickFixConfigFile(const string& templateFile, const string& server, const string &clientName, const string &outputFileName)
{
    // Delete out file if already exists
    if( utility::doesFileExist(outputFileName))
    {
        utility::deleteFile(outputFileName);
    }
    // Get template file contents as string
    ifstream templateFileStream(templateFile);
    string templateFileData( (istreambuf_iterator<char>(templateFileStream)), istreambuf_iterator<char>());
    // Inject server and client name into it
    utility::replaceInString(templateFileData, "%CLIENT%", clientName);
    utility::replaceInString(templateFileData, "%SERVER%", server);
    // Output 
    ofstream outFile;
    outFile.open (outputFileName);
    outFile << templateFileData;
    outFile.close();
}