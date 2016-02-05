#ifndef _CONFIG_FILE_
#define _CONFIG_FILE_

#include<string>
#include<vector>
#include<unordered_map>

namespace utility
{

class ConfigFile
{
    public :

        void loadFromFile(const std::string& fileName);
        const std::string& getStringValue(const std::string& attribute) const;
        bool getBoolValue(const std::string& attribute) const;
        int getIntVaue(const std::string& attribute) const;
        std::vector<std::string> getArray(const std::string& attribute) const;

    private:

        mutable std::unordered_multimap<std::string, std::string> m_dictionary;
};

}//namespace
#endif