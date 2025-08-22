#include "filemananger.h"
#include <string>
#include <sstream>

#include <memory>
#include <functional>
#include <fstream>

#include "base/logging.h"
#include "base/logfile.h"
#include "base/singleton.h"
#include "config.h"

using namespace std;

HQSnapShotFile::HQSnapShotFile(const string& file,
	int flushInterval)
	: flushInterval_(flushInterval),
	running_(false),
	basename_(file),
	rollSize_(0),
	latch_(1),
	currentBuffer_(new Buffer),
	nextBuffer_(new Buffer),
	buffers_(),
	todayDateTime_(0)
{
	currentBuffer_->bzero();
	nextBuffer_->bzero();
	buffers_.reserve(16);

	string filename;
	filename.reserve(basename_.size() + 64);
	filename = basename_;

	time_t now = time(NULL);
	basename_ = filename;

	todayDateTime_ = (now - 5400) / 86400;
}

HQSnapShotFile::~HQSnapShotFile()
{
	if (running_)
	{
		stop();
	}
}

void HQSnapShotFile::threadFunc()
{
	assert(running_ == true);

	latch_.countDown();
	LogFile* pOutput = new LogFile(basename_, rollSize_, false);
	BufferPtr newBuffer1(new Buffer);
	BufferPtr newBuffer2(new Buffer);
	newBuffer1->bzero();
	newBuffer2->bzero();
	BufferVector buffersToWrite;
	buffersToWrite.reserve(16);
	while (running_)
	{
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(buffersToWrite.empty());

		{
			std::unique_lock<mutex> lock(mutex_);
			if (buffers_.empty())  // unusual usage!
			{
				cond_.wait_for(lock, chrono::seconds(flushInterval_));
			}
			buffers_.push_back(currentBuffer_);
			currentBuffer_ = newBuffer1;
			newBuffer1 = NULL;
			buffersToWrite.swap(buffers_);
			if (!nextBuffer_)
			{
				nextBuffer_ = newBuffer2;
				newBuffer2 = NULL;
			}
		}

		assert(!buffersToWrite.empty());

		if (buffersToWrite.size() > 25)
		{
			char buf[256];

			snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
				Timestamp::now().toFormattedString().c_str(),
				buffersToWrite.size() - 2);

			fputs(buf, stderr);
			pOutput->append(buf, static_cast<int>(strlen(buf)));
			buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
		}

		for (size_t i = 0; i < buffersToWrite.size(); ++i)
		{
			// FIXME: use unbuffered stdio FILE ? or use ::writev ?
			pOutput->append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
		}

		if (buffersToWrite.size() > 2)
		{
			// drop non-bzero-ed buffers, avoid trashing
			buffersToWrite.resize(2);
		}

		if (!newBuffer1)
		{
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.back();
			newBuffer1->reset();
			buffersToWrite.pop_back();
		}

		if (!newBuffer2)
		{
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.back();
			newBuffer2->reset();
			buffersToWrite.pop_back();
		}

		buffersToWrite.clear();
		pOutput->flush();

		// 如果是新的一天， 重新打开新文件
		time_t now = time(NULL);
		//struct tm tm_time;
		//::localtime_r(&now, &tm_time); // FIXME TimeZone::fromUtcTime

		//int64_t nowDateTime = (tm_time.tm_year + 1900) * 10000 + (tm_time.tm_mon + 1) * 100 + tm_time.tm_mday;
		int64_t nowDateTime = (now - 3600 * 4) / 86400;
		if (nowDateTime > todayDateTime_)
		{
			todayDateTime_ = nowDateTime;

			delete pOutput;

			pOutput = new LogFile(basename_, rollSize_, false);

		}
	}
	pOutput->flush();
}

void HQSnapShotFile::append(const char* logline, int len)
{
	//std::unique_lock<std::mutex> lock(mutex_);
	std::unique_lock<mutex> lock(mutex_);

	if (currentBuffer_->avail() > len)
	{
		currentBuffer_->append(logline, len);
	}
	else
	{
		buffers_.push_back(currentBuffer_);

		if (nextBuffer_)
		{
			currentBuffer_ = nextBuffer_;
			nextBuffer_ = NULL;
		}
		else
		{
			currentBuffer_ = new Buffer; // Rarely happens
		}
		currentBuffer_->append(logline, len);
		cond_.notify_all();
	}
}

FileMananger::FileMananger()
{

}

FileMananger::~FileMananger()
{

}

void FileMananger::Init()
{

	string strBinPath = g_Config["binpath"];
	string strMarket = g_Config["market"];
	string strLogPath = g_Config["logpath"];
	string strFileName;

	string str = Timestamp::now().toFormattedString();
	string strDate = str.substr(0, 8);	

	{
		string strFileName = strLogPath + "stockdict.csv." + strDate;
		ofstream of(strFileName.c_str(), ios::out);
	}

	{
		string strFileName = strLogPath + "order.csv." + strDate;
		ofstream of(strFileName.c_str(), ios::out);
	}

	{
		string strFileName = strLogPath + "snapshot.csv." + strDate;
		ofstream of(strFileName.c_str(), ios::out);
	}	

	{
		string strFileName = strLogPath + "preclose.csv." + strDate;
		ofstream of(strFileName.c_str(), ios::out);
	}

	{
		m_pStockDictLog = make_shared<HQSnapShotFile>(strLogPath + "stockdict.csv");
		m_pStockDictLog->start();
		string strTitle = "count,type,size,totalsize\n";
		m_pStockDictLog->append(strTitle.c_str(), strTitle.size());
	}	

	{
		m_pOrderLog = make_shared<HQSnapShotFile>(strLogPath + "order.csv");
		m_pOrderLog->start();
		string strTitle = "symbol,market,bidpx,bidsize,offerpx,offersize,time,recvtime\n";
		m_pOrderLog->append(strTitle.c_str(), strTitle.size());
	}

	{
		m_pSnapShotLog = make_shared<HQSnapShotFile>(strLogPath + "snapshot.csv");
		m_pSnapShotLog->start();
		string strTitle = "symbol,time,recvtime,preclose,last,open,high,low,close,totalvol,turnover,lasttradeprice,lasttradeqty,usavol,regsho,direction\n";
		m_pSnapShotLog->append(strTitle.c_str(), strTitle.size());
	}

	{
		m_pChannelSecondLog = make_shared<HQSnapShotFile>(strLogPath + "channel_second.csv");
		m_pChannelSecondLog->start();
		string strTitle = "markettime,gathertime,transtime,clienttime\n";
		m_pChannelSecondLog->append(strTitle.c_str(), strTitle.size());
	}

	{
		m_pInstStatusLog = make_shared<HQSnapShotFile>(strLogPath + "instrument_status.csv");
		m_pInstStatusLog->start();
		string strTitle = "symbol,preclose,time,localtime\n";
		m_pInstStatusLog->append(strTitle.c_str(), strTitle.size());
	}

	{
		m_pSystemEvent = make_shared<HQSnapShotFile>(strLogPath + "system_event.csv");
		m_pSystemEvent->start();
		string strTitle = "time,event\n";
		m_pSystemEvent->append(strTitle.c_str(), strTitle.size());
	}
}
