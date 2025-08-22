#pragma once


#include <map>
#include <string>
#include <vector>
#include <deque>
#include <memory>

#include "base/timestamp.h"
#include "base/asynclogging.h"

using namespace std;

class HQClientSession;

/*************************************
行情快照文件
**************************************/
class HQSnapShotFile
{
public:
	HQSnapShotFile(const std::string& filename,
		int flushInterval = 3);

	~HQSnapShotFile();

	void append(const char* logline, int len);

	void start()
	{
		running_ = true;
		thread_.reset(new std::thread(std::bind(&HQSnapShotFile::threadFunc, this)));
		
		latch_.wait();
	}

	void stop()
	{
		running_ = false;
		cond_.notify_all();
		thread_->join();
	}

private:

	// declare but not define, prevent compiler-synthesized functions
	HQSnapShotFile(const HQSnapShotFile&);  // ptr_container
	void operator=(const HQSnapShotFile&);  // ptr_container

	void threadFunc();

	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef Buffer* BufferPtr;
	typedef vector<BufferPtr > BufferVector;


	const int flushInterval_;
	bool running_;
	string basename_;
	size_t rollSize_;
	shared_ptr<std::thread> thread_;

	CountDownLatch latch_;
	std::mutex mutex_;
	condition_variable cond_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;

	int64_t todayDateTime_;
};

class FileMananger
{
public:
	FileMananger();
	~FileMananger();

	void Init();

	void LogStockDict(const string& str)
	{
		if (m_pStockDictLog)
		{
			m_pStockDictLog->append(str.c_str(), str.size());
		}
	}

	void LogTest(const string& str)
	{
		if (m_pStockDictLog)
		{
			m_pStockDictLog->append(str.c_str(), str.size());
		}
	}


	void LogOrder(const string& str)
	{
		if (m_pOrderLog)
		{
			m_pOrderLog->append(str.c_str(), str.size());
		}
	}

	void LogSnapShot(const string& str)
	{
		if (m_pSnapShotLog)
		{
			m_pSnapShotLog->append(str.c_str(), str.size());
		}
	}

	void LogChannelSecond(const string& str)
	{
		if (m_pChannelSecondLog)
		{
			m_pChannelSecondLog->append(str.c_str(), str.size());
		}
	}

	void LogInstStatus(const string& str)
	{
		if (m_pInstStatusLog)
		{
			m_pInstStatusLog->append(str.c_str(), str.size());
		}
	}

	void LogSystemEvent(const string& str)
	{
		if (m_pSystemEvent)
		{
			m_pSystemEvent->append(str.c_str(), str.size());
		}
	}


private:
	shared_ptr<HQSnapShotFile> m_pStockDictLog;
	shared_ptr<HQSnapShotFile> m_pOrderLog;
	shared_ptr<HQSnapShotFile> m_pSnapShotLog;
	shared_ptr<HQSnapShotFile> m_pInstStatusLog;

	shared_ptr<HQSnapShotFile> m_pChannelSecondLog;
	shared_ptr<HQSnapShotFile> m_pSystemEvent;
};