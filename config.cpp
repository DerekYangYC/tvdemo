#include "config.h"
#include <fstream>
#include <sstream>
#include "base/logging.h"

Config g_Config;

Config::Config()
{
}


Config::~Config()
{
}

void Config::ReadFile(const string& strFile)
{
	ifstream infile(strFile.c_str(), ios::in);
	if (!infile)
	{
		LOG_ERROR << "Open Config File Failed";
		printf("ERROR Open Config F %s failed.", strFile.c_str());
		return ;
	}

	string strTemp;
	string strValue;

	int nLine = 0;
	while (getline(infile, strTemp))
	{
		nLine++;

		string str;
		for (size_t i = 0; i < strTemp.size(); ++i)
		{
			if (strTemp[i] == '#')
			{
				break;
			}
			if (strTemp[i] != ' ' && strTemp[i] != '\r' && strTemp[i] != '\n' && strTemp[i] != '\t')
			{
				str.push_back(strTemp[i]);
			}			
		}

		if (str.size() == 0)
		{
			continue;
		}

		string::size_type pos = str.find_first_of("=");
		if (pos == str.size() - 1 || pos == string::npos)
		{
	//		LOG_WARN << "config error, line " << nLine << " : " << strTemp;
			continue;
		}

		string strName = str.substr(0, pos);
		string strValue = str.substr(pos + 1, str.size() - 1 - pos);
		if (m_mapValues.find(strName) != m_mapValues.end())
		{
			LOG_WARN << "config error, duplicated key : " << strName;
			continue;
		}

		m_mapValues[strName] = strValue;
	}
}

bool Config::LoadFromFile(const string& strFile)
{
	ReadFile(strFile);
	
	CheckHKConfig();

	return true;
}

bool Config::CheckHKConfig()
{
	string vtKeys[] = {"datasource"};

	bool flag = true;
	for (size_t i = 0; i < sizeof(vtKeys) / sizeof(string); ++i)
	{
		if (m_mapValues.find(vtKeys[i]) == m_mapValues.end())
		{
			LOG_ERROR << "config error, hk not found " << vtKeys[i];
			flag = false;
		}
	}

	return flag;
}

