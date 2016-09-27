#ifndef __XJCQ_GAMESRV_TRADE_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_TRADE_CMD_PROTOCOL_H__

#pragma pack(push,1)





//交易

//消息
enum enTradeCmd VC_PACKED_ONE
{
	enTradeCmd_NeedSell,									//寄售物品
	enTradeCmd_BuyGoods,									//购买物品
	enTradeCmd_ViewTradeGoods,								//查看指定标签的交易物品
	enTradeCmd_EnterTrade,									//进入交易市场
	enTradeCmd_GoodsInfo,									//查看交易物品的详细信息
	enTradeCmd_EnterMyTradeGoods,							//进入我的摊位
	enTradeCmd_ViewMyTradeGoods,							//查看我的摊位(寄售中的物品)
	enTradeCmd_CancelMyTradeGoods,							//撤消我的交易物品

	enTradeCmd_Max,
}PACKED_ONE;

//交易，消息头
struct TradeHeader : public AppPacketHeader
{
	TradeHeader(enTradeCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Trade;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//操作结果码
enum enTradeRetCode VC_PACKED_ONE
{
	enTradeRetCode_OK,
	enTradeRetCode_MaxNum,					//已达到寄售物品最大数量
	enTradeRetCode_NoMoney,					//钱不够
	enTradeRetCode_NoGoodsNum,				//输入的物品数量大于背包里的物品数量
	enTradeRetCode_PacketFull,				//由于背包容量已满，已发送至邮箱
	enTradeRetCode_NoStone,					//灵石不足
	enTradeRetCode_NotZero,					//输入的仙石数不能小于1

	enTradeRetCode_Max,
}PACKED_ONE;

//把物品拿去寄售
struct CS_SellGoods_Req
{
	CS_SellGoods_Req() : m_UidGoods(0), m_GoodsNum(0), m_Price(0){}
	UID			m_UidGoods;					//物品的UID值
	UINT16		m_GoodsNum;					//物品的数量
	UINT32		m_Price;					//物品的价格
};



//把物品拿去寄售
struct SC_TradeResult_Rsp
{
	SC_TradeResult_Rsp() : m_ResultRetCode(enTradeRetCode_OK)
	{
	}

	enTradeRetCode m_ResultRetCode;			//结果码
};

enum enTradeLabel VC_PACKED_ONE
{
	enTradeLabel_Common,					//普通物品
	enTradeLabel_Equip,						//装备
	enTradeLabel_GodSword,					//仙剑
	enTradeLabel_Talisman,					//法宝
	enTradeLabel_Gem,						//宝石

	//下面是属于装备的子标签，只用于区分，直接从装备标签中找
	enTradeLabel_Head,						//头部
	enTradeLabel_Shoulder,					//肩部
	enTradeLabel_Breast,					//胸部
	enTradeLabel_Leg,						//腿部
	enTradeLabel_Foot,						//脚部

	enTradeLabel_Max,
}PACKED_ONE;
//购买物品
struct CS_BuyGoods_Req
{
	CS_BuyGoods_Req() : m_UidGoods(UID()){}
	UINT8		m_TradeLabel;				//购买的物品所在标签
	UID			m_UidGoods;					//要买的物品UID
};

//查看指定标签的交易物品
struct CS_ViewTradeGoods_Req
{
	CS_ViewTradeGoods_Req(){
		m_TradeLabel = enTradeLabel_Equip;
	}
	enTradeLabel	m_TradeLabel;
};

//查看市场指定标签的物品,先发物品数量
struct SC_ViewTradeGoodsNum_Rsp
{
	SC_ViewTradeGoodsNum_Rsp() : m_LabelTradeGoodsNum(0){}
	UINT16			m_LabelTradeGoodsNum;		//这个标签的交易物品数量
//	ViewGoodsSellerInfo_Rsp GoodSell[m_LabelTradeGoodsNum];
};

//查看市场指定标签的物品,发卖家名字
struct ViewGoodsSellerInfo_Rsp
{
	ViewGoodsSellerInfo_Rsp(){
		MEM_ZERO(this);
	}
	char			m_szSellerName[THING_NAME_LEN];
	UID				m_UidGoods;
	UINT32			m_Price;		//价格	
};

//查看我的摊位在寄售中的物品
struct  SC_ViewMyTradeGoodsNum_Rsp
{
	SC_ViewMyTradeGoodsNum_Rsp() : m_LabelTradeGoodsNum(0){}
	UINT16			m_LabelTradeGoodsNum;		//这个标签的交易物品数量
//	ViewMyGoodsSellerInfo_Rsp GoodSell[m_LabelTradeGoodsNum];	
};

//查看我的摊位在寄售中的物品
struct ViewMyGoodsSellerInfo_Rsp
{
	ViewMyGoodsSellerInfo_Rsp() : m_UidGoods(UID()), m_Price(0), m_TotalTime(0), m_LostTime(0)
	{
	}
	UID				m_UidGoods;
	UINT32			m_Price;		//价格
	UINT32			m_TotalTime;	//寄售总时间(秒)
	UINT32			m_LostTime;		//已过时间
};

//查看交易物品的详细信息
struct CS_SeeTradeGoodsInfo_Req
{
	CS_SeeTradeGoodsInfo_Req() : m_TradeLabel(enTradeLabel_Equip), m_uidGoods(UID(0))
	{
	}

	enTradeLabel	m_TradeLabel;
	UID				m_uidGoods;
};

//撤消我的交易物品
struct CS_CancelMyTradeGoods_Req
{
	CS_CancelMyTradeGoods_Req() : m_uidGoods(0)
	{
	}

	UID			m_uidGoods;
};

struct SC_CancelMyTradeGoods_Rsp
{
	SC_CancelMyTradeGoods_Rsp() : m_ResultRetCode(enTradeRetCode_OK)
	{
	}

	enTradeRetCode m_ResultRetCode;			//结果码
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
