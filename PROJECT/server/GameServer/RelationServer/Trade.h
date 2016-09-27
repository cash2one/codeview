#ifndef __RELATIONSERVER_TRADE_H__
#define __RELATIONSERVER_TRADE_H__

#include "IMessageDispatch.h"
#include <hash_map>
#include <vector>
#include "IGoods.h"
#include "GameSrvProtocol.h"
#include "IDBProxyClient.h"
#include "ITimeAxis.h"

struct IActor;

struct STradeGoods
{
	STradeGoods(){
		memset(this, 0, sizeof(*this));
	}
	UID				m_uidSeller;					//卖家的UID值
	char			m_szSellerName[THING_NAME_LEN];	//卖家的名字			
	INT32			m_Price;						//寄售物品的价格
	UINT8			m_SubClass;						//物品的子分类
	UINT64			m_Time;							//物品寄售时的时间
	SDBGoodsData	m_SDBGoodsData;					//物品的数据
};

class Trade : public IMsgRootDispatchSink, public IDBProxyClientSink, public ITimerSink
{	
	enum enTradeTIMER_ID
	{
		enTradeTIMER_ID_CheckDel = 0, //检测交易物品的寄售时间是否到
	};

public:
	Trade();
	~Trade();

	bool	Create();
	void	Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//IDBProxyClientSink接口
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//定时器接口
	virtual void OnTimer(UINT32 timerID);

private:
	//把物品寄售
	void	SellItem(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//购买物品
	void	BuyGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//查看市场指定标签中的物品
	void	ViewTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//进入交易市场
	void	EnterTrade(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//查看交易物品的详细信息
	void	SeeTradeGoodsInfo(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//进入我的摊位
	void	EnterMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//查看我的摊位指定标签的物品
	void	ViewMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//撤消我的交易物品
	void	CancelMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib);

private:
	//是否能够寄售
	bool	CanSellItem(IActor * pActor, const CS_SellGoods_Req & Req);

	//获得玩家的寄售物品集合
	void	GetUserTradeGoods(std::vector<STradeGoods> & vectTradeGoods, const UID & uid_User);

	//插入到寄售集合中去
	void	AddToTradeGoodsList(enGoodsCategory GoodsCategory, const STradeGoods & TradeGoods);

	//把物品从玩家身上移走
	bool	RemoveGoods(IActor * pActor, IGoods * pGoods, INT16 GoodsNum, SDBGoodsData & GoodsData);

	//加载交易数据
	void	LoadTradeData();

	//
	void	HandleLoadTradeData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//根据物品UID值，获取物品的STradeGoods结构
	STradeGoods * GetTradeGoods(enTradeLabel TradeLabel, const UID & uid_Goods);

	//把该物品从交易物品中删除
	void	DeleteTradeGoods(enTradeLabel TradeLabel, const UID uid_Goods);

	//查看市场指定标签中的物品
	void	ViewTradeGoods(IActor *pActor, enTradeLabel TradeLabel);
	
	//交易物品的寄售时间已到，把卖家的交易物品还给卖家
	void	BackSellerGoods(const STradeGoods & TradeGoods);

	//查看我的摊位
	void	ViewMyTradeGoods(IActor *pActor);

	//得到最大可寄售数
	INT16	GetMaxSellGoodsNum(IActor * pActor);

	//把物品通过邮件发送给买家
	bool	SendBuyerGoods(UID uidBuyer, IGoods * pGoods, INT32 Price);

	//把钱通过邮件发送给卖家
	bool	SendSellerMoney(UID uidSeller, INT32 Price, TGoodsID GoodsID);

private:
	typedef	 std::vector<STradeGoods>								VECT_TRADEGOODS;

	typedef	 std::hash_map<UID/*交易物品UID*/, STradeGoods>	MAP_TRADEGOODS;

	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>		m_mapTradeGoods;

	INT16	m_MaxSellGoodsNum;				//最多可以寄售的物品个数(帮派福利还可以增加5个)

	INT16	m_SpendStoneSellGoods;			//寄售一件物品需要花费的灵石数量

	INT32	m_MaxSellTradeGoodsTime;		//寄售物品的最大时长(秒)

};

#endif
