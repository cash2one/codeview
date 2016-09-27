
#ifndef __THINGSERVER_PACKET_PART_H__
#define __THINGSERVER_PACKET_PART_H__

#include "IGameWorld.h"
#include "IPacketPart.h"
#include <hash_set>

struct IActor;
struct SGoodsCnfg;
struct SGoodsUseCnfg;


class PacketPart : public IPacketPart,public IEventListener
{
	typedef std::hash_set<UID,std::hash<UID>, std::equal_to<UID> > MAP_SET;


public:
    PacketPart();

	virtual ~PacketPart();

public:
    	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	virtual bool  IsFull(); 

	//获得背包物品
	virtual IGoods * GetGoods(UID uidGoods) ;

	//获得背包数量
	virtual INT32    GetGoodsNum();

	//获得背包当前容量
	virtual INT32   GetCapacity();

		//判断能否增加
	virtual bool CanAddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded= false);

	//增加物品到背包
	virtual bool AddGoods(UID uidGoods,bool bNotifyClientCreate=true);

	//增加物品到背包
	virtual bool AddGoods(TGoodsID GoodsID,INT32 nNum=1,bool bBinded= false);

	//从背包移除物品
	virtual bool RemoveGoods(UID uidGoods);

	//销毁物品
	virtual bool DestroyGoods(UID uidGoods) ;

		//销毁物品
	virtual bool DestroyGoods(TGoodsID GoodsID,INT32 nNum=1);

	//扩充背包容量
	virtual bool ExtendCapacity();

	//使用      bOk为是否确认使用,有些物品会叫玩家确认使用
	virtual bool UseGoods(UID uidGoods,UID uidTarget = UID(),INT32 nNum=1, UINT8  Index = 0);

	//丢弃
	virtual bool DiscardGoods(UID uidGoods);

	//装备
	virtual bool Equip(UID uidGoods,UID uidTarget,UINT8 pos);

	//合成
	virtual bool Compose(TGoodsID gidOutput,INT16 OutputNum);

		//是否有指定数量的物品
	virtual bool HaveGoods(TGoodsID GoodsID,INT32 nNum=1);

		//是否可以批量增加物品
	virtual bool CanAddGoods(const SAddGoodsInfo * pGoodsInfo,INT32 nNum); 

		//获得背包物品
	virtual IGoods * GetGoods(INT32 Index);

	//出售物品
	virtual void SellGoods(UID uidGoods, INT32 nNum);

	//是否可以批量增加物品，vectGoods格式为:GoodsID,nNum,GoodsID,nNum,.......
	virtual bool CanAddGoods(const std::vector<UINT16> & vectGoods);

    //重置所有背包中的仙剑副本进度
    virtual void  ResetGodSwordFuBen();

	//销毁指定物品
	virtual bool DestroyGoods(UID uidGoods, INT32 nNum);

	//获得空格数
    virtual INT32 GetSpace();

	//获得背包指定物品的数量
	virtual INT32 HaveGoodsNum(TGoodsID GoodID);

	//背包物品保存
    virtual void  SaveGoodsToDB(IGoods * pGoods);

	////////////////////////////////////////////////////////////////
	//发送配置文件信息

	//加入已发送客户端物品配置记录
	virtual void InsertSendGoodsCnfg(TGoodsID GoodsID);

	//是否已经发送过物品配置记录
	virtual bool IsAlreadySend(TGoodsID GoodsID);

	//清空已发送客户端的所有配置记录
	virtual void ClearSendCnfg();

	//加入已发送客户端招募角色配置记录
	virtual void InsertSendEmployeeCnfg(TActorID EmployeeID);

	//是否已经发送过招募角色配置记录
	virtual bool IsAlreadySendEmployeeCnfg(TActorID EmployeeID);

	//加入已发送客户端法术等级配置记录
	virtual void InsertSendMagicLevelCnfg(TMagicID	MagicID);

	//是否已经发送过法术等级配置记录
	virtual bool IsAlreadySendMagicLevelCnfg(TMagicID MagicID);

		//加入已发送客户端创建Thing记录
	virtual void InsertSendThing(IThing * pThing);

	//是否已发送客户端创建Thing记录
	virtual bool IsAlreadySendThing(UID uidThing);

	//清除已发送给客户端的thing记录
	virtual void ClearSendThing(UID uidThing);

	//加入已发送客户端帮派技能配置记录
	virtual void InsertSendSynMagicCnfg(TSynMagicID SynMagicID);

	//是否已发送客户端帮派技能配置记录
	virtual bool IsAlreadySendSynMagic(TSynMagicID SynMagicID);
	//////////////////////////////////////////////////////////

	public:
		virtual void OnEvent(XEventData & EventData);
	

private:

	//是否可以批量增加物品,需要预留空间
	virtual bool CanAddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded,INT32 & ReservedSpace); 

   IGoods * GetGoodsFromGameWorld(UID uidGoods);

   //销毁物品,返回销毁数量
   INT32 __DestroyGoods(const SGoodsCnfg* pGoodsCnfg,INT32 nNum,bool bBinded);

   //数据库删除装备
   void	 DBDeleteGoods(UID uidGoods);

private:
	//获得已扩充容量次数
	INT16 GetExtendNum();

	//增加进背包
	bool  __AddGoods(IGoods * pGoods, bool bSave = true);

private:
	IActor*              m_pActor;     //玩家
	INT16                m_Capacity;  //容量
	MAP_SET              m_setGoods;  //物品集合   

	MAP_SET				 m_setGoodsCompare;  //保存时比较使用

	//发送给客户端的配置信息
	std::hash_set<TGoodsID>  m_GoodsCnfg;	//记录已发送给客户端的记录

	std::hash_set<TActorID>	 m_EmployeeCnfg;	//记录已发送给客户端招募角色记录

	std::hash_set<TMagicID>	 m_MagicLevelCnfg;	//记录已发送给客户端法术等级记录

	typedef std::hash_set<UID,std::hash<UID>, std::equal_to<UID> > SET_THING;

	SET_THING                m_setThing;      //记录已发送给客户端的thing

	typedef std::hash_set<TSynMagicID>  SET_SYNMAGIC;
	SET_SYNMAGIC	m_SynMagicCnfg;	//记录已发送给客户端的帮派技能记录


};



#endif

