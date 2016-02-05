#include "config_file.h"
#include <boost/tokenizer.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <algorithm>
#include <ctype.h>
#include <cstddef>
#include <utility/pretty_exception.h>
using namespace std;

namespace utility
{

void ConfigFile::loadFromFile(const string& fileName)
{
    // For reusability
    m_dictionary.clear();
    
    ifstream file(fileName);

    if ( ! file.good())
    {
        auto exceptionMessage = boost::str(boost::format("File %s could not be opened") % fileName);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }
    
    file.seekg(0, std::ios::beg);
    string line;
    boost::char_separator<char> seperator("=");
    unsigned long lineNumber(0);

    while ( std::getline(file, line) )
    {
        lineNumber++;
        auto lineLength = line.length();

        if (line.c_str()[0] == '#' || lineLength == 0 ) // Skip comment lines and empty lines
        {
            continue; 
        }

        if ( lineLength < 3)
        {
            auto exceptionMessage = boost::str(boost::format("Line is too short , line number : %d") % lineNumber);
            THROW_PRETTY_EXCEPTION(exceptionMessage)
        }

        size_t equalsPos = line.find("=", 0);

        if (equalsPos  == std::string::npos)
        {
            auto exceptionMessage = boost::str(boost::format("Line doesn`t contain equals sign , line number : %d") % lineNumber);
            THROW_PRETTY_EXCEPTION(exceptionMessage)
        }

        if (line.find("=", equalsPos+1 ) != std::string::npos)
        {
            auto exceptionMessage = boost::str(boost::format("Line contains more than one equals sign , line number : %d") % lineNumber);
            THROW_PRETTY_EXCEPTION(exceptionMessage)
        }
        
        boost::tokenizer<boost::char_separator<char>> tokenizer(line, seperator);
        auto iter = tokenizer.begin();

        string attribute = *iter;
        ++iter;
        string value = *iter;
        m_dictionary.insert( std::make_pair(attribute, value) );
    }

    file.close();
}

const string& ConfigFile::getStringValue(const string& attribute) const
{
    auto element = m_dictionary.find(attribute);
    if (element == m_dictionary.end())
    {
        auto exceptionMessage = boost::str(boost::format("Attribute %s could not be found") % attribute);
        THROW_PRETTY_EXCEPTION(exceptionMessage)
    }
    
    return element->second;
}

bool ConfigFile::getBoolValue(const string& attribute) const
{
    auto stringVal = getStringValue(attribute);
    std::transform(stringVal.begin(), stringVal.end(), stringVal.begin(), ::tolower);
    return (stringVal == "true") ? true : false; 
}

int ConfigFile::getIntVaue(const string& attribute) const
{
    return std::stoi(getStringValue(attribute));
}

vector<string> ConfigFile::getArray(const string& attribute) const
{
    vector<string> ret;
    string actualAttribute = attribute + "[]";

    auto range = m_dictionary.equal_range(actualAttribute);
    
    for_each(
        range.first,
        range.second,
        [&](unordered_multimap<string, string>::value_type& element){ ret.push_back(element.second); }
    );

    return ret;
}

}//namespace