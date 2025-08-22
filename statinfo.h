#pragma once

#include <stdint.h>
#include "base/timestamp.h"

class StatInfo
{
public:
	StatInfo()
	{
		m_ullRecvBytes = 0;
		m_ullLastRecvBytes = 0;
		m_uRecvSpeed = 0;
		m_uMsgCount = 0;
		m_ullSendBytes = 0;
		m_ullLastSendBytes = 0;
		m_ullSendBufferBytes = 0;
		m_uSendSpeed = 0;
		m_uSendMsgCount = 0;
	}

	void OnTime();
	void PrintTestInfo();

	uint64_t m_ullRecvBytes;

	Timestamp m_LastTime;
	Timestamp m_LastPrintTime;
	uint64_t	m_ullLastRecvBytes;   // 上次计算传输速度时的大小 
	uint32_t    m_uRecvSpeed;         // Byte / s
	uint32_t	m_uMsgCount;

	// 已发送数据 
	uint64_t    m_ullSendBufferBytes;
	uint64_t    m_ullSendBytes;
	uint64_t    m_ullLastSendBytes;
	uint32_t    m_uSendSpeed;         // Byte / s
	uint32_t	m_uSendMsgCount;
};
