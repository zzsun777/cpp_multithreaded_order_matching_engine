#include "client_request.h"
#include "client_application.h"

#include <string>
#include <exception>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <boost/format.hpp>

#include <utility/utility.h>
#include <utility/pretty_exception.h>

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

        if (quickFixConfigFile.length() < 5) { throw std::runtime_error("Invalid FIX engine config file"); }

        // Backup FIX engine logs if exists
        utility::createDirectory("old_quickfix_logs");
        utility::backupDirectory("quickfix_log", "quickfix_log_" + utility::getCurrentDateTime("%d_%m_%Y_%H_%M_%S"), "old_quickfix_logs");

        // Run the application
        ClientApplication application(csvTestFile, quickFixConfigFile);
        application.run();
        
        utility::deleteFile(quickFixConfigFile);
    }
    catch (std::runtime_error & e)
    {
        std::cout << e.what();
        return 1;
    }

    return 0;
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