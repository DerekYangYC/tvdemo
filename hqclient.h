#pragma once

#include <string>
#include <map>
#include <set>
#include <memory>

#include "base/timestamp.h"
#include "base/logging.h"
#include "base/singleton.h"

#include <boost/asio.hpp>

#include "filemananger.h"
#include "statinfo.h"
#include "net/netbuffer.h"
#include "protocol.h"

//#include "filemananger.h"

using namespace std;


class HQClient
{
public:
	HQClient(boost::asio::io_service& ios)
		: m_Socket(ios)
	{
		m_usPort = 0;
	}
	~HQClient() 
	{
		LOG_INFO << "delete HQClient " << m_strIP << ":" << m_usPort;
	}

	virtual void Logon()
	{
	}

	void OnRead(const boost::system::error_code& error, size_t bytes);
	void OnConnected(const boost::system::error_code& error);
	void DoRead();

	void ProcessBuffer();

	void OnTime();
	void Connect(const string& strIP, const uint16_t& port);

protected:
	SzFiu::Network::NetBuffer m_NetBuffer;
	uint64_t m_ullRecvBytes;

	string m_strIP;
	uint16_t m_usPort;

	boost::asio::ip::tcp::socket m_Socket;


	Timestamp m_LastTime;

	Timestamp m_LastHeartTime;
	Timestamp m_LastMsgTime; // 用于重连 

	string	m_strTemp;  // 未完字符串 
	char m_Data[65536];

	map<int64_t, int64_t> m_mapUnknownMsg;

	//szfiu::PBClientOrder m_PBOrder;
	//szfiu::PBClientSnapshot m_PBSnapshot;
};
