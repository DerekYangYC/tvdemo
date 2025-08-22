#include "statinfo.h"
#include <sstream>
#include <iostream>

#include "base/singleton.h"
#include "base/logging.h"

using namespace std;

void StatInfo::OnTime()
{
	Timestamp now = Timestamp::now();
	if (m_LastTime.microSecondsSinceEpoch() == 0)
	{
		m_LastTime = now;
		return;
	}

	int64_t ullInterval = now.microSecondsSinceEpoch() - m_LastTime.microSecondsSinceEpoch();
	if (ullInterval <= 0)
	{
		m_LastTime = now;
		return;
	}

	if (ullInterval < 100 * 1000) // 100ms
	{
		return;
	}	

	ullInterval = now.microSecondsSinceEpoch() - m_LastTime.microSecondsSinceEpoch();
	uint64_t ullBytes =m_ullRecvBytes - m_ullLastRecvBytes;
	uint64_t ullSendBytes = m_ullSendBytes - m_ullLastSendBytes;
	if (ullInterval > 1000 * 1000 * 1 && (ullBytes > 0 || ullSendBytes > 0))
	{
		// º∆À„Õ¯ÀŸ 
		m_uRecvSpeed = ullBytes * 1000000 / ullInterval;
		m_LastTime = now;
		m_ullLastRecvBytes = m_ullRecvBytes;

		m_uSendSpeed = ullSendBytes * 1000000 / ullInterval;
		m_ullLastSendBytes = m_ullSendBytes;

		m_LastPrintTime = now;
		PrintTestInfo();
	}
}

void StatInfo::PrintTestInfo()
{
	stringstream ss;
	ss << "total recv bytes : " << m_ullRecvBytes << ", recv speed : " << m_uRecvSpeed / 1024.0 << "KB/S";
	ss << ", total send bytes : " << m_ullSendBytes << ", send speed : " << m_uSendSpeed / 1024.0 << "KB/S" << ", send buffer : " << m_ullSendBufferBytes;
	cout << ss.str() << endl;
	LOG_INFO << ss.str();
}