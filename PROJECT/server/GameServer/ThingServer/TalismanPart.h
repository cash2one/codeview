
#ifndef __THINGSERVER_TALISMAN_PART_H__
#define __THINGSERVER_TALISMAN_PART_H__

#include "DBProtocol.h"
#include "ITalismanPart.h"
#include <hash_map>
#include <vector>
#include "UniqueIDGenerator.h"
#include "ITimeAxis.h"
#include "IEventServer.h"
#include "TalismanWorldFuBen.h"


struct IActor;
struct IGoods;
struct ITalisman;

class TalismanPart : public ITalismanPart,public ITimerSink, public IEventListener
{
	//定时器ID
	enum eTalismanTimerID
	{
		eTalismanTimerID_Occupation = 0,	//占领
		eTalismanTimerID_GetSoul,			//获取灵魄
	};

public:
	TalismanPart();
	virtual ~TalismanPart();

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
	virtual IThing*		GetMaster(void) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close() ;

	//保存数据
	virtual void SaveData();

			//孕育法宝物
	virtual bool SpawnTalismanSpawnGoods(UID  uidTalisman);

		//领取法宝孕育物
	virtual bool TakeTalismanSpawnGoods(UID  uidTalisman);

		//提升品质点
	virtual bool UpgradeTalismanQuality(UID  uidTalisman,TGoodsID gidProp);


	//玩家是否可以进入法宝世界
	virtual bool CanEnterTalismanWorld(TTalismanWorldID TalismanWorldID);

	//进入法宝世界
	virtual bool EnterTalismanWorld(CS_TalismanWorldEnter_Req & Req);

	//退出法宝世界
	virtual void LeaveTalismanWorld(UINT32 GameID);

	//游戏消息
	virtual void GameMessage(CSC_TalismanWorldGame & Req,IBuffer & ib) ;

	//占领
	virtual bool Occupation(UID uidMonster, UINT8	Level, UINT8 floor);

	//占领结束
	virtual void OverOccupation(bool bBeaten = false);

	//得到占领信息
	virtual const OccupationInfo * GetOccupationInfo(); 

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing();

	virtual void OnTimer(UINT32 timerID);

	//检测法宝世界次数
	virtual bool Check_EnterTalismanWorldNum();

	//打开法宝世界
	virtual void OpenTalismanWorld();

	//得到今天进入法宝世界次数
   virtual UINT8	GetEnterTWNumToday();

   //得到今天vip进入法宝世界次数
   virtual UINT8	GetVipEnterTWNumToday();

   //增加进入法宝世界次数
   virtual void		AddEnterNum();

   //设置下的法宝世界场景
   virtual void		SetTWSceneID(TSceneID SceneID);

   virtual void OnEvent(XEventData & EventData);

   //清除法宝世界的次数
   virtual void ClearTWEnterNum();

private:
  //获得已在孕育的法宝数量
  INT32 GetTalismanSpawnNum();


  //是否可以用该道具提升品质
   bool CanUpgradeQualityWithProp( ITalisman * pTalisman,TGoodsID gidProp,INT32 & QualityPoint,INT32 & CanUseNum);

   //获取灵魄
   void		GiveGhostSoul();

private:
	IActor * m_pActor;

	typedef std::hash_map<TTalismanWorldID,STalismanWorldRecord> MAP_TALISMAN_RECORD;

	MAP_TALISMAN_RECORD   m_mapTalismanRecord;

	//玩家正在孕育的法宝
	std::vector<UID>    m_vectSpawnTalisman;

	UINT32   m_GameID;  //游戏ID

	//占领信息
	OccupationInfo		m_OccupationInfo;

	//占领时获得的灵魄数量
	UINT32				m_GetGhostSoul;

	//今天进入法宝世界次数
	UINT8				m_EnterTWNumToday;

	//最后一次进入法宝世界时间
	UINT32				m_LastEnterTWTime;

	//今天VIP进入法宝世界次数
	UINT8				m_VipEnterTWNumToday;

	//最后一次VIP进入法宝世界时间
	UINT32				m_LastVipEnterTWTime;

	//法宝世界占领获得灵魄次数
	UINT16				m_GetGhostTimes;

	//最近正在下的法宝世界场景
	TSceneID			m_TWSceneID;	

	TalismanWorldFuBen	m_TalismanWorldFuBen;
};








#endif
