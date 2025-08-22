#include "datamananger.h"
#include "base/singleton.h"
#include "filemananger.h"

extern uint64_t g_llCurrentDate;

void TrimString(string& str)
{
	string temp;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] != ' ')
		{
			temp.push_back(str[i]);
		}
	}
	str.swap(temp);
}

vector<string> SplitString(const string& str, const string& key)
{
	vector<string> vec;

	string::size_type pre = 0, next = 0;

	while (pre != string::npos && pre < str.size())
	{
		next = str.find(key, pre);
		if (next == string::npos)
		{
			vec.push_back(str.substr(pre, str.size() - pre));
			return vec;
		}
		vec.push_back(str.substr(pre, next - pre));
		pre = next + 1;

		if (pre == str.size())
		{
			vec.push_back("");
		}
	}

	return vec;
}

int64_t GetNowTime()
{
	auto now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	struct tm* tm_time = localtime(&tt);

	int64_t millSecs = 0;

	auto nowLocalTimeCount = now.time_since_epoch();
	std::chrono::milliseconds now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(nowLocalTimeCount);

	int64_t nDate = (tm_time->tm_year + 1900) * 10000 + (tm_time->tm_mon + 1) * 100 + tm_time->tm_mday;
	int64_t nTime = tm_time->tm_hour * 10000 + tm_time->tm_min * 100 + tm_time->tm_sec;
	int64_t iTime = (nDate * 1000000 + nTime) * 1000 + now_ms.count() % 1000;
	return iTime;
}

uint64_t GetUSTime(const uint64_t& t)
{
	uint64_t time = (t / 1000) % (24 * 3600);
	int hour = time / 3600;
	time = time % 3600;
	int min = time / 60;
	int sec = time % 60;
	int millsec = t % 1000;

	uint64_t temp = g_llCurrentDate + (hour * 10000 + min * 100 + sec) * 1000 + millsec;
	return temp;
}

DataMananger::DataMananger()
{

}

DataMananger::~DataMananger()
{

}


void DataMananger::OnInstrument(szfiu::StockDefine& define)
{
    m_ss.str("");
    m_ss << define.code() << "," << define.symbol() << "," << GetUSTime(define.time()) << "," << define.market() << "," << define.financial() << ","
		<< define.roundlotsize() << "," << define.roundlotonly() << "," << define.classification() << "," << define.subtype() << ","
		<< define.authenticity() << "," << define.shortsale() << "," << define.ipo() << "," << define.luld() << "," << define.etp() << ","
		<< endl;

    Singleton<FileMananger>::Instance().LogStockDict(m_ss.str());

    m_mapInstrument[define.symbol()] = define;

    //if (data.putcall() == 'P')
    //{

    //}
    //m_mapOptChain[data.root()][data.expiration()].insert(data.symbol());
}

void DataMananger::OnOrder(FiuOrder& data)
{
	string symbol = data.symbol_;

	if (data.dir_ == 'B')
	{
		if (data.qty_ == 0)
		{
			m_mapOrder[symbol].bidlist_.erase(data.price_);
		}
		else
		{
			m_mapOrder[symbol].bidlist_[data.price_] = data.qty_;
		}		
	}
	else if(data.dir_ == 'S')
	{
		if (data.qty_ == 0)
		{
			m_mapOrder[symbol].asklist_.erase(data.price_);
		}
		else
		{
			m_mapOrder[symbol].asklist_[data.price_] = data.qty_;
		}
	}
	else
	{
		cout << "direction error : " << data.symbol_ << "," << data.dir_ << "," << data.price_ << "," << data.qty_ << endl;
		return;
	}


	// ´òÓ¡ÈÕÖ¾ 
	if (symbol == "AAPL" || symbol == "BRK.A" || symbol == "TSLA")
	{
		m_ss.str("");
		m_ss << data.symbol_ << "," << data.dir_ << "," << data.price_ << "<" << data.qty_ << ">" << endl;

		auto it0 = m_mapOrder[symbol].bidlist_.rbegin();
		auto it1 = m_mapOrder[symbol].asklist_.begin();

		int count = 0;
		while (it0 != m_mapOrder[symbol].bidlist_.rend() ||
			it1 != m_mapOrder[symbol].asklist_.end())
		{
			if (count >= 20)
			{
				break;
			}

			if (it0 != m_mapOrder[symbol].bidlist_.rend())
			{
				char temp[64] = { ' ' };
				m_ss << it0->first << "," << it0->second;
				temp[63] = '\0';
				m_ss << temp << " <======================> ";

				it0++;
			}

			if (it1 != m_mapOrder[symbol].asklist_.end())
			{
				m_ss << it1->first << "," << it1->second;
				it1++;
			}

			m_ss << endl;

			count++;
		}

		m_ss << endl;

		Singleton<FileMananger>::Instance().LogOrder(m_ss.str());
	}
	
}

