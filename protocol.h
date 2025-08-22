#pragma once

#include <vector>
#include <stdint.h>
#include <string.h>

using namespace std;

#pragma pack(push,1)

enum SZFiuType
{
	Fiu_Heartbeat = 0,

	Fiu_TV_Order = 1,
	Fiu_TV_Level = 2,
	Fiu_TV_Define = 3,
	Fiu_TV_Imbalance = 4, 
	Fiu_TV_MoneyFlow = 5,
	Fiu_TV_MoneyFlow_ALL = 6,
	Fiu_TV_OrderAdd = 7,
	Fiu_TV_OrderExe = 8,
	Fiu_TV_OrderReplace = 9,
	Fiu_TV_OrderDelete = 10,
	Fiu_TV_OrderCancel = 11,
	Fiu_TV_SystemEvent = 12,
	Fiu_TV_CrossTrade = 13,

	Fiu_TV_BaseTime = 66,

	Fiu_TV_Zlib = 80,
	Fiu_TV_Init = 81,
	Fiu_TV_Subscribe = 82, 
};

struct FiuHeader
{
	FiuHeader()
	{
		memset(this, 0, sizeof(*this));
		len_ = sizeof(*this);
	}
	uint16_t	len_;
	uint8_t		type_;
};

struct FiuBaseTime : public FiuHeader
{
	FiuBaseTime()
	{
		memset(this, 0, sizeof(*this));
		len_ = sizeof(*this);
		type_ = Fiu_TV_BaseTime;
	}

	uint64_t base_time_;
	uint64_t recv_time_;
	uint64_t tran_time_;
};

struct FiuOrder : public FiuHeader
{
	FiuOrder()
	{
		memset(this, 0, sizeof(*this));
		len_ = sizeof(*this);
		type_ = Fiu_TV_Order;
	}
	char symbol_[12];
	int64_t price_;
	int64_t	qty_;
	char	dir_;  // B, S
	int64_t time_;
};

struct FiuHeartBeat : public FiuHeader
{
	FiuHeartBeat()
	{
		memset(this, 0, sizeof(*this));
		len_ = sizeof(*this);
		type_ = Fiu_Heartbeat;

		send_ = 0;
	}
	uint8_t		send_; // 0 not send, 1 send data
};

// 不发送订阅列表表示订阅全部 
struct FiuSubScribe : public FiuHeader
{
	FiuSubScribe()
	{
		memset(this, 0, sizeof(FiuSubScribe));
		type_ = Fiu_TV_Subscribe;
		len_ = sizeof(FiuSubScribe);
	}
};

struct FiuStockDict : public FiuHeader
{
	FiuStockDict()
	{
		memset(this, 0, sizeof(FiuStockDict));
		type_ = Fiu_TV_Define;
		len_ = sizeof(FiuStockDict);
	}

	uint16_t		code_;
	char			symbol_[8];
	char			market_;
	char			financial_;
	int32_t			lot_size_;
	char			lot_only_;
	char			class_;
	char			sub_type_[2];
	char			auth_;
	char			short_sale_;
	char			ipo_flag_;
	char			luld_;
	char			etp_flag_;
	int32_t			etp_factor_;
	char			inverse_;

	int64_t			time_;
};


#pragma pack(pop)
