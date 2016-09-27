#ifndef __THINGSERVER_SHOPMALL_H__
#define __THINGSERVER_SHOPMALL_H__

#include "IMessageDispatch.h"
#include <hash_map>
#include "IDBProxyClient.h"
#include "ITimeAxis.h"

//商城物品配置
struct SShopMallCnfg
{
	SShopMallCnfg()
	{
		m_GoodsID = 0;
		m_Amount = 0;
		m_MoneyType = enShopMoneyType_GodStone;
		m_MoneyNum = 0;
		m_bBinded = 0;
		m_Type = 0;
		m_EndTime = 0;
		m_StartTime = 0;
		m_LeftFen = 0;
		m_bCanSell = false;
		m_index = 0;
	}

	TGoodsID	m_GoodsID;		//物品ID
	UINT16		m_Amount;		//数量
	enShopMoneyType	m_MoneyType;	//货币类型
	INT32		m_MoneyNum;		//货币数量
	UINT8		m_Type;			//对应不同标签,0为热卖物品(用仙石购买)，1为普通物品(用仙石购买),2为宝石(用仙石购买)，3为得用礼券购买，4为得用灵石购买
	UINT8		m_bBinded;		//是否要绑定
	UINT32		m_StartTime;	//0:无限制时间,非0:有限制时间
	UINT32		m_EndTime;		//0:无限制时间,非0:有限制时间
	INT32		m_LeftFen;		//当前剩余可卖份数,-1表示无限个
	bool		m_bCanSell;		//false:当前不可卖,true:当前可卖
	int			m_index;		//排列顺序
};

class ShopMall : public IMsgRootDispatchSink,public IDBProxyClientSink, public ITimerSink
{
	enum enShopMallTimer
	{
		enShopMallTimer_OneMinute = 0,	//1分钟定时器
	};

public:
	ShopMall();
	~ShopMall();

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual void OnTimer(UINT32 timerID);

public:
	//重新设置商场配置信息
	void	ResetShopMallCnfg();

	//放进商场配置表
	void	Push_ShopMallCnfg(const SDB_Get_ShopMallCnfg & DBShopMallCnfg);

private:
	//进入
	void Enter(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	
	//切换标签显示
	void ChangeLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	
	//购买物品
	void BuyItem(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到商场配置信息
	void HandleGetShopMallCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到商城抒写商店的物品配置集合
	const std::hash_map<UINT32/*id*/, SShopMallCnfg> * GetShopMallCnfgByLable(enShopMallLabel label);

	//打开VIP页面
	void OpenVipView(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	typedef std::hash_map<UINT32/*id*/, SShopMallCnfg>  MAP_MALL;

	std::vector<MAP_MALL>	 m_vecShopMallCnfgByLabel;	//商城购买物品配置
};

#endif
