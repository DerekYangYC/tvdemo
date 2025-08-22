#include <iostream>
#include <boost/asio.hpp>

#include "base/singleton.h"
#include "base/logging.h"
#include "base/asynclogging.h"
//#include "../../base/profile.h"


#include "hqclient.h"
#include "statinfo.h"
#include "config.h"
#include "filemananger.h"

using namespace std;

AsyncLogging* g_asyncLog = NULL;
uint64_t g_llCurrentDate = 0;

void asyncOutput(const char* msg, int len)
{
	if (g_asyncLog != NULL)
	{
		g_asyncLog->append(msg, len);
	}
}

extern vector<string> SplitString(const string& str, const string& key);

std::string g_strTestStock;


int main()
{
	AsyncLogging log("totalview.txt", 1024 * 1024 * 1024);
	log.start();
	g_asyncLog = &log;
	Logger::setOutput(asyncOutput);

	boost::asio::io_service ios;
	// 防止io service退出， 加一个work
	boost::asio::io_service::work work(ios);

	g_Config.LoadFromFile("config.ini");
	string strIP, strTemp;
	strTemp = g_Config["datasource"];
	string::size_type pos = strTemp.find_first_of(":");
	if (pos == string::npos || pos == strTemp.size() - 1)
	{
		LOG_FATAL << "hq datasource error : " << strTemp;
		return 0;
	}
	strIP = strTemp.substr(0, pos);
	string strPort = strTemp.c_str() + pos + 1;
	int16_t usPort = atoi(strPort.c_str());

	HQClient* pClient = new HQClient(ios);
	pClient->Connect(strIP.c_str(), usPort);

	std::thread t([&ios]() {  ios.run(); });
	
	Singleton<FileMananger>::Instance().Init();

	while (1)
	{		
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		Singleton<StatInfo>::Instance().OnTime();
		pClient->OnTime();
	}
	
	t.join();
	return 1;
}