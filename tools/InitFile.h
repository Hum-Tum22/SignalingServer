#pragma once

#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>


class IniFile
{
public:
    IniFile(const std::string& filename) : filename(filename)
	{
		readIniFile();
	}

	void readIniFileKeyValue(const std::string section, const std::string key, std::string &value)
    {
    	auto itSec = mSectionMap.find(section);
		if(itSec != mSectionMap.end())
		{
			auto itKv = itSec->second.find(key);
			if(itKv != itSec->second.end())
			{
				value = itKv->second;
			}
		}
    }
	void readIniFileSection(const std::string section, std::map<std::string, std::string>& secMap)
    {
    	auto itSec = mSectionMap.find(section);
		if(itSec != mSectionMap.end())
		{
			secMap = itSec->second;
		}
    }
	void writeIniFileKeyValue(const std::string section, const std::string key, const std::string value)
    {
    	auto it = mSectionMap.find(section);
		if(it != mSectionMap.end())
		{
			auto itSecond = it->second.find(key);
			if(itSecond != it->second.end())
			{
				itSecond->second = value;
			}
			else
			{
				it->second[key] = value;
			}
		}
		else
		{
			std::map<std::string, std::string> keyValue;
			keyValue[key] = value;
			mSectionMap[section] = keyValue;
		}
		writeIniFile(mSectionMap);
    }
	void reload()
	{
		readIniFile();
	}

private:
	void readIniFile() 
	{
		std::ifstream file(filename);
		if (!file.is_open())
		{
			std::cout << "Failed to open file: " << filename << std::endl;
			return ;
		}

		std::string line;
		std::string section;
		std::map<std::string, std::string> oneSecMap;
		while (std::getline(file, line))
		{
			line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); }), line.end());
			if (line.empty() || line[0] == ';' || line[0] == '#')
			{
				continue;
			}
				
			if (line[0] == '[' && line[line.length() - 1] == ']')
			{
				if(!section.empty())
				{
					mSectionMap[section] = oneSecMap;
				}
				section = line.substr(1, line.length() - 2);
				oneSecMap.clear();
				continue;
			}

			size_t equalsPos = line.find('=');
			if (equalsPos == std::string::npos)
			{
				continue;
			}

			std::string key = line.substr(0, equalsPos);
			std::string value = line.substr(equalsPos + 1);
			oneSecMap[key] = value;

			//keyValueMap[section + "@" + key] = value;
		}
		mSectionMap[section] = oneSecMap;

		file.close();

		return ;
	}

	void writeIniFile(const std::map<std::string, std::map<std::string, std::string>>& iniData)
	{
		std::ofstream file(filename, std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			std::cout << "Failed to create file: " << filename << std::endl;
			return;
		}
		std::string currentSection;
		for (const auto& pair : iniData)
		{
			file << '[' << pair.first << ']' << std::endl;
			for(const auto& it : pair.second)
			{
				file << it.first << "=" << it.second << std::endl;
			}
		}
		file.close();
	}

    std::string filename;
	std::map<std::string, std::map<std::string, std::string>> mSectionMap;
};
