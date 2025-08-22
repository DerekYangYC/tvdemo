#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <ctime>
#include <iomanip>
#include <functional>
//#include <zlib.h>
#include "protocol.h"
#include "hqclient.h"

#include "base/logging.h"
#include "base/object_pool.h"
#include "base/singleton.h"
#include "datamananger.h"

using namespace std;
using namespace std::chrono;

extern uint64_t GetUSTime(const uint64_t& t);

void HQClient::Connect(const string& strIP, const uint16_t& port)
{
	LOG_INFO << "begin to connect to " << strIP << ":" << port;
	m_strIP = strIP;
	m_usPort = port;

	m_NetBuffer.retrieveAll();
	
	boost::asio::ip::tcp::endpoint  endpoint(boost::asio::ip::address::from_string(strIP.c_str()), m_usPort);

	m_Socket.async_connect(endpoint,
		std::bind(&HQClient::OnConnected, this, std::placeholders::_1));
}

void HQClient::OnConnected(const boost::system::error_code& error)
{
	if (error)
	{
		if (error.value() != boost::system::errc::operation_canceled)
		{
			std::cout << "connect error: " << error.value() << std::endl;
		}
		return;
	}

	// 1. subscribe all instrument 
	FiuHeartBeat msg;
	msg.send_ = 1;

	boost::asio::async_write(m_Socket,
		boost::asio::buffer((const char*)&msg,
			sizeof(msg)),
		[this](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
			}
			else
			{
				LOG_ERROR << "send data failed : " << ec.value();
			}
		});

	// begin to receive data
	DoRead();
}

void HQClient::DoRead()
{
	m_Socket.async_read_some(boost::asio::buffer(/*m_NetBuffer.beginWrite()*/
		m_Data, 65536),
		std::bind(&HQClient::OnRead,
			this,
			std::placeholders::_1,
			std::placeholders::_2));
}

void HQClient::OnRead(const boost::system::error_code& error, size_t bytes)
{
	if (error)
	{
		LOG_WARN << "socket disconnected : " << error.value() << "," << bytes;
		return;
	}	

	m_NetBuffer.append(m_Data, bytes);

	DoRead();

	Singleton<StatInfo>::Instance().m_ullRecvBytes += bytes;	

	m_LastMsgTime = Timestamp::now();

	ProcessBuffer();
	
}

void HQClient::ProcessBuffer()
{
	if (m_NetBuffer.readableBytes() < sizeof(FiuHeader))
	{
		return;
	}

	while (1)
	{
		if (m_NetBuffer.readableBytes() < sizeof(FiuHeader))
		{
			break;
		}

		FiuHeader* pHeader = (FiuHeader*)m_NetBuffer.peek();
		// len : header + body + checksum
		uint32_t uLen = pHeader->len_;

		if (uLen == 0)
		{
			LOG_WARN << "msg length is 0";
			m_NetBuffer.retrieveAll();
			break;
		}

		if (m_NetBuffer.readableBytes() < uLen)
		{
			break;
		}

		const char* pBegin = m_NetBuffer.peek() + sizeof(FiuHeader);
		const char* pEnd = m_NetBuffer.peek() + uLen;
		string strTemp(pBegin, pEnd);

		switch (pHeader->type_)
		{
		case Fiu_TV_SystemEvent:
		{
			szfiu::SystemEvent data;
			data.ParseFromString(strTemp);
			cout << (char)data.event() << "," << GetUSTime(data.time()) << endl;
			break;
		}
		case Fiu_TV_Define:
		{
			szfiu::StockDefine data;
			data.ParseFromString(strTemp);
			Singleton<DataMananger>::Instance().OnInstrument(data);

			break;
		}
		case Fiu_TV_Init:
		{
			Singleton<DataMananger>::Instance().Init();
			break;
		}
		case Fiu_TV_Order:
		{
			FiuOrder* pMsg = (FiuOrder*)pHeader;
			Singleton<DataMananger>::Instance().OnOrder(*pMsg);
			break;
		}
		case Fiu_Heartbeat:
		{
			break;
		}
		case Fiu_TV_BaseTime:
		{
			break;
		}
		default:
			if (m_mapUnknownMsg.find(pHeader->type_) == m_mapUnknownMsg.end())
			{
				cout << "unknown msg : " << (uint32_t)pHeader->type_ << endl;
				m_mapUnknownMsg[pHeader->type_] = 0;
			}
			m_mapUnknownMsg[pHeader->type_]++;
			break;
		}

		
		m_NetBuffer.retrieve(uLen);

		if (pHeader->type_ != Fiu_Heartbeat)
		{
			Singleton<StatInfo>::Instance().m_uMsgCount++;
		}
	}
}

void HQClient::OnTime()
{
	Timestamp now = Timestamp::now();

	//if (timeDifference(now, m_LastMsgTime) > 5)
	//{
	//	m_Socket.close();

	//	Connect(m_strIP, m_usPort);
	//	m_LastMsgTime = now;
	//}
}
