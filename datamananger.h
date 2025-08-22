#pragma once

#include <string>
#include <map>
#include <set>
#include <sstream>

#include "protocol.h"
#include "tv.pb.h"

using namespace std;

struct OrderDetail
{
	OrderDetail()
	{
		price_ = 0;
		vol_ = 0;
	}

	int64_t price_;
	int64_t vol_;
};

struct MyOrder
{
	MyOrder()
	{
		time_ = 0;
	}
	string symbol_;
	int64_t time_;
	map<int64_t, int64_t> bidlist_;
	map<int64_t, int64_t> asklist_;
};

class DataMananger
{
public:
	DataMananger();
	~DataMananger();

public:
	void OnInstrument(szfiu::StockDefine& data);
	void OnOrder(FiuOrder& data);

	void Init()
	{
		m_mapOrder.clear();
	}

private:

	map<string, szfiu::StockDefine> m_mapInstrument;

	map<string, MyOrder> m_mapOrder;

	stringstream m_ss;
};

