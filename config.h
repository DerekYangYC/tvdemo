#pragma once

#include <string>
#include <map>
#include <vector>

using namespace std;

class Config
{
public:
	Config();
	~Config();

	bool LoadFromFile(const string& strFile);
	string operator[](const string& strIndex)
	{
		if (m_mapValues.find(strIndex) == m_mapValues.end())
		{
			return "";
		}
		return m_mapValues[strIndex];
	}

private:
	void ReadFile(const string& strFile);

	bool CheckHKConfig();

public:
	map<string, string> m_mapValues;
	string m_strMarketType;
};

extern Config g_Config;
