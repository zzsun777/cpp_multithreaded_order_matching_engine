#ifndef _FILE_UTILITY_H_
#define _FILE_UTILITY_H_

#include <string>
#include <fstream>

#ifdef __linux__
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#elif _WIN32
#include <windows.h>
#endif

namespace utility
{

inline bool doesFileExist(const std::string& fileName)
{
    std::ifstream f(fileName.c_str());
    bool retval = false;

    if (f.good())
    {
        retval = true;
    }

    f.close();
    return retval;
}

inline bool deleteFile(const std::string& fileName)
{
    bool success = true;
#ifdef __linux__
    if ( unlink(fileName.c_str()) != 0 )
    {
        success = false;
    }
#elif _WIN32
    if (DeleteFile(fileName.c_str()) == 0)
    {
        success = false;
    }
#endif

    return success;
}

inline bool backupDirectory(const std::string& existingName, const std::string& newName, const std::string& targetDirectory)
{
    bool success = true;
#ifdef __linux__
    auto actualNewName = targetDirectory + "//" + newName;
    // The input should have forward slashes , not Windows specific back slashes
    if( rename(existingName.c_str(), actualNewName.c_str()) != 0)
    {
        return false;
    }
#elif _WIN32
    auto actualNewName = targetDirectory + "\\" + newName;
    if (MoveFile(existingName.c_str(), actualNewName.c_str()) == 0)
    {
        success = false;
    }
#endif
    return success;
}

inline bool createDirectory(const std::string& dirName)
{
    bool success = true;
#ifdef __linux__
    if( mkdir(dirName.c_str(), 777) != 0)
    {
        success = false;
    }
#elif _WIN32
    if (CreateDirectory(dirName.c_str(), nullptr) == 0)
    {
        success = false;
    }
#endif
    
    return success;
}
    
}// namespace

#endif