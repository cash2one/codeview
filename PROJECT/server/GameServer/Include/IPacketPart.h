
#ifndef __THINGSERVER_IPACKET_PART_H__
#define __THINGSERVER_IPACKET_PART_H__

#include "IThingPart.h"
#include "UniqueIDGenerator.h"

struct IGoods;

//需要批量增加的物品信息
struct SAddGoodsInfo
{
	TGoodsID  m_GoodsID;
	INT32     m_nNum;
	bool      m_bBinded;

	SAddGoodsInfo()
	{
		m_GoodsID = INVALID_GOODS_ID;
		m_nNum    = 1;
		m_bBinded = false;
	}
};

//使用物品加状态时会用到
struct SAddStatus
{
	SAddStatus() : m_PacketRetCode(enPacketRetCode_OK),m_Index(0),m_bReplace(false),m_UseGoodsNum(0){ }

	enPacketRetCode m_PacketRetCode;
	UINT8			m_Index;		//检测到第几个状态组
	UINT16			m_UseGoodsNum;	//使用的物品数量
	std::string		m_strUserDesc;
	bool			m_bReplace;		//是否替换,否为只提示，是为让玩家选择是否替换状态
};

struct IPacketPart : public IThingPart
{
	//背包是否已满
	virtual bool  IsFull() = 0;

	//获得背包物品
	virtual IGoods * GetGoods(UID uidGoods) = 0;

	//获得背包物品
	virtual IGoods * GetGoods(INT32 Index) = 0;

	//获得背包数量
	virtual INT32    GetGoodsNum() = 0;

	//获得背包当前容量
	virtual INT32   GetCapacity() = 0;

	//判断能否增加
	virtual bool CanAddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded= false) = 0;

	//增加物品到背包
	virtual bool AddGoods(UID uidGoods,bool bNotifyClientCreate=true) = 0;

		//增加物品到背包
	virtual bool AddGoods(TGoodsID GoodsID,INT32 nNum=1,bool bBinded= false)=0;

	//从背包移除物品
	virtual bool RemoveGoods(UID uidGoods) = 0;

	//销毁物品
	virtual bool DestroyGoods(UID uidGoods) = 0;

		//销毁物品
	virtual bool DestroyGoods(TGoodsID GoodsID,INT32 nNum=1) = 0;

	//扩充背包容量
	virtual bool ExtendCapacity() = 0;

	//使用      bOk为是否确认使用,有些物品会叫玩家确认使用
	virtual bool UseGoods(UID uidGoods,UID uidTarget = UID(),INT32 nNum=1, UINT8  Index = 0) = 0;

	//丢弃
	virtual bool DiscardGoods(UID uidGoods) = 0;

	//装备
	virtual bool Equip(UID uidGoods,UID uidTarget,UINT8 pos) = 0;

	//合成
	virtual bool Compose(TGoodsID gidOutput,INT16 OutputNum) = 0;

	//获得空格数
    virtual INT32 GetSpace() = 0;

	//是否有指定数量的物品
	virtual bool HaveGoods(TGoodsID GoodsID,INT32 nNum=1)=0;

	//是否可以批量增加物品
	virtual bool CanAddGoods(const SAddGoodsInfo * pGoodsInfo,INT32 nNum) = 0;
	
	//出售物品
	virtual void SellGoods(UID uidGoods, INT32 nNum) = 0;

	//是否可以批量增加物品，vectGoods格式为:GoodsID,nNum,GoodsID,nNum,.......
	virtual bool CanAddGoods(const std::vector<UINT16> & vectGoods) = 0;

	//重置所有背包中的仙剑副本进度
    virtual void  ResetGodSwordFuBen() = 0;

	//获得背包指定物品的数量
	virtual INT32 HaveGoodsNum(TGoodsID GoodID) = 0;

		//加入已发送客户端物品配置记录
	virtual void InsertSendGoodsCnfg(TGoodsID GoodsID) = 0;

	//是否已经发送过物品配置记录
	virtual bool IsAlreadySend(TGoodsID GoodsID) = 0;

	//清空已发送客户端的所有配置记录
	virtual void ClearSendCnfg() = 0;

	//加入已发送客户端招募角色配置记录
	virtual void InsertSendEmployeeCnfg(TActorID EmployeeID) = 0;

	//是否已经发送过招募角色配置记录
	virtual bool IsAlreadySendEmployeeCnfg(TActorID EmployeeID) = 0;

	//加入已发送客户端法术等级配置记录
	virtual void InsertSendMagicLevelCnfg(TMagicID	MagicID) = 0;

	//是否已经发送过法术等级配置记录
	virtual bool IsAlreadySendMagicLevelCnfg(TMagicID MagicID) = 0;

		//加入已发送客户端创建Thing记录
	virtual void InsertSendThing(IThing * pThing) = 0;

	//是否已发送客户端创建Thing记录
	virtual bool IsAlreadySendThing(UID uidThing) = 0;

	//清除已发送给客户端的thing记录
	virtual void ClearSendThing(UID uidThing) = 0;

	//加入已发送客户端帮派技能配置记录
	virtual void InsertSendSynMagicCnfg(TSynMagicID SynMagicID) = 0;

	//是否已发送客户端帮派技能配置记录
	virtual bool IsAlreadySendSynMagic(TSynMagicID SynMagicID) = 0;

	//销毁指定物品
	virtual bool DestroyGoods(UID uidGoods, INT32 nNum) = 0;

	//背包物品保存
    virtual void SaveGoodsToDB(IGoods * pGoods) = 0;
};



#endif

