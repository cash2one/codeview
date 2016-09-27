#ifndef __XJCQ_GAMESRV_SHOPMALL_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_SHOPMALL_CMD_PROTOCOL_H__

#pragma pack(push,1)

//充值订单号长度
#define MAX_RECHARGE_ORDER_LEN  31

//充值卡号
#define MAX_RECHARGE_CARDNO_LEN  31

//充值卡号密码
#define MAX_RECHARGE_CARDPWD_LEN  31

//商城

enum enShopMallCmd VC_PACKED_ONE
{
	enShopMallCmd_Enter,		//进入商城
	enShopMallCmd_ChangeLabel,	//切换标签显示
	enShopMallCmd_Buy,			//购买

	enShopMallCmd_CommitOrder,    //提交订单
	enShopMallCmd_Recharge,    //充值

	enShopMallCmd_OpenVipView,	//打开VIP页面

	enShopMallCmd_Max,
}PACKED_ONE;

//商城，消息头
struct ShopMallHeader : public AppPacketHeader
{
	ShopMallHeader(enShopMallCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_ShopMall;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//商城里的商店类别
enum enShopMallLabel VC_PACKED_ONE
{
	enShopMall_Hot,				//热卖物品(用仙石购买)
	enShopMall_Gen,				//普通物品(用仙石购买)
	enShopMall_Baoshi,			//宝石物品(用仙石购买)
	enShopMall_Ticket,			//用礼卷购买商店
	enShopMall_Stone,			//用灵石购买商店
	enShopMall_Honor,			//用荣誉购买商店
	enShopMall_Credit,			//用声望购买商店


	enShopMall_Max,
}PACKED_ONE;

//货币类型
enum enShopMoneyType VC_PACKED_ONE
{
	enShopMoneyType_GodStone = 0,
	enShopMoneyType_Ticket,
	enShopMoneyType_Stone,
	enShopMoneyType_Honor,
	enShopMoneyType_Credit,//声望
}PACKED_ONE;

//商城物品的基本数据
struct SShopMallData
{
	UINT32			m_id;
	TGoodsID		m_idGoods;			//物品ID
	UINT16			m_GoodsNum;			//物品数量
	enShopMoneyType m_MoneyType;		//货币类型
	INT32			m_MoneyNum;			//货币数量
	UINT16			m_Index;			//顺序

};

//发送出去的数据
struct SC_ShopMallData_Rsp
{
	SC_ShopMallData_Rsp(){
		m_nGoodsNum = 0;
	}

	INT32         m_nGoodsNum;	//物品的个数
//	SShopMallData m_GoodsShopData[m_nGoodsNum];
};

struct CS_ChangeLaber_Req
{
	enShopMallLabel	m_nLaber;
};

struct CS_BuyItemShop_Req
{
	enShopMallLabel	m_Label;	//哪个标签商店
	UINT32			m_ID;
	UINT8			m_GoodsNum;
};

enum enShopMallRetCode VC_PACKED_ONE
{
	enShopMallRetCode_OK,			//OK
	enShopMallRetCode_NoMoney,		//仙石不足
	enShopMallRetCode_NoTicket,		//礼卷不足
	enShopMallRetCode_NoStone,		//灵石不足
	enShopMallRetCode_NoHonor,		//荣誉不足
	enShopMallRetCode_PacketFull,	//背包已满，请清理背包
	enShopMallRetCode_NoItem,		//此类商店中没找到此物品
	enShopMallRetCode_NumLimit,		//超过可购买限量
	enShopMallRetCode_NoBuy,		//该商品已下架 
	enShopMallRetCode_OverGoods,	//该商品已卖完

	enShopMallRetCode_ErrorRecharge,	//充值失败
	enShopMallRetCode_NoCredit,		//声望不足
}PACKED_ONE;

struct SC_BuyShopMall_Rsp
{
	SC_BuyShopMall_Rsp(){
		
		m_Result = enShopMallRetCode_OK;
	}

	enShopMallRetCode m_Result;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
通道名称	通道ID
神州付	001
易宝骏网一卡通	010
易宝神州行	006
易宝联通卡	007
易宝盛大点卡	005
易宝征途卡	012
易宝电信卡	016
支付宝	014
*/

enum enPayChanel VC_PACKED_ONE
{
	enPayChanel_ShenZhouFu = 0,  //神州付
	enPayChanel_JunWang = 1,     //骏网
	enPayChanel_ShenZhouXing = 2,  //神州行
	enPayChanel_LianTong = 3,  //联通
	enPayChanel_ShengDa = 4,  //盛大

	enPayChanel_ChengTu = 5,  //征途
	enPayChanel_DianXin = 6,  //电信
	enPayChanel_ZhiFuBao = 7,  //支付宝


	enPayChanel_Max,		     //
}PACKED_ONE;

//提交订单
struct CS_CommitOrder_Req
{
	UINT8         m_pcid;  //支付方式
	UINT8         m_szCardNo[MAX_RECHARGE_CARDNO_LEN];   //卡号
	UINT8         m_szPwd[MAX_RECHARGE_CARDPWD_LEN];     //密码
	INT32         m_Amount;    //金额
};

//订单号返回
struct SC_CommitOrder_Rsp
{
	enShopMallRetCode m_Result;  //结果
	char  m_szOrder[MAX_RECHARGE_ORDER_LEN];  //订单号
};


//充值
struct CS_Recharge_Req
{
	char  m_szOrder[MAX_RECHARGE_ORDER_LEN];  //订单号
	INT32  m_Amount;  //第三方货币金额
};

//应答
struct SC_Recharge_Rsp
{
	enShopMallRetCode m_Result;  //结果
//	SC_Recharge_Ok	  m_Recharge_Ok;	//OK时发的
//  SC_Recharge_Error m_Error;			//Error时发的
};

struct SC_Recharge_Ok
{
	INT32  m_Amount;  //第三方货币金额
	INT32  m_GodStone; //仙石
};

struct SC_Recharge_Error
{
	char		m_szErrorInfo[DESCRIPT_LEN_500];	//出错内容
};

//打开vip页面
struct SC_OpenVipView
{
	INT32       m_TotalRechargeNum;		//当前充值仙石总数
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
