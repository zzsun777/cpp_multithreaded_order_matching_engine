#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include<string>
#include<vector>
#include<exception>
#include<unordered_map>

namespace utility
{

class ConfigFile
{
    public :

        void loadFromFile(const std::string& fileName) throw(std::runtime_error);
        const std::string& getStringValue(const std::string& attribute) const throw(std::invalid_argument);
        bool getBoolValue(const std::string& attribute) const throw(std::invalid_argument);
        int getIntVaue(const std::string& attribute) const throw(std::invalid_argument);
        std::vector<std::string> getArray(const std::string& attribute);

    private:

        mutable std::unordered_multimap<std::string, std::string> m_dictionary;
};

}//namespace
#endif