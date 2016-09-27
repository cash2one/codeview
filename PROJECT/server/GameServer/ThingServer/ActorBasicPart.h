
#ifndef __THINGSERVER_ACTORBASIC_PART_H__
#define __THINGSERVER_ACTORBASIC_PART_H__


#include "IActorBasicPart.h"
#include <hash_set>
#include "ITimeAxis.h"

struct IActor;
struct IGodSword;

class ActorBasicPart : public IActorBasicPart,public ITimerSink
{
	//定时器ID
	enum enActorBasicTimerID
	{
		enActorBasicTimerID_OnLineTime = 0,  //在线时间定时器
	};

public:
	ActorBasicPart();

	virtual ~ActorBasicPart();

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

	virtual void OnTimer(UINT32 timerID);

public:
		//属性变动
	virtual void OnActorPropChange(IActor* pActor,enCrtProp PropID,INT32 nNewValue,INT32 nOldValue); 

	//获得当天进入该仙剑次数
    INT32 GetEnterGodSwordFuBenNumOfDay(IGodSword * pGodSword);

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing();

private:
	//经验变动
	void OnExpChange(enCrtProp PropID,INT32 nNewValue);

	//升级
   void OnLevelUp(enCrtProp PropID,INT32 nNewValue);

   //资质变动
   void OnAptitudeChange(enCrtProp PropID,INT32 nNewValue);

   //灵力变动
   void OnSpiritChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //护盾变动
   void OnShieldChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //身法变动
   void OnAvoidChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //气血上限变动
   void OnBloodUpChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //爆击变动
   void OnCritChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //坚韧变动
   void OnTenacitypChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //命中变动
   void OnHitChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //回避变动
   void OnDodgeChange(enCrtProp PropID, INT32 nNewValue, INT32 nOldValue);

   //定时器开始
   void StartTimer(UINT32 nTimerID, UINT32 nTimeNum);

   //定时器结束
   void StopTimer(UINT32 nTimerID);

   //少于18岁的防沉迷提示
   void WallowTip(UINT32 nOnLineTime);

private:
	IActor * m_pActor;

	bool	 m_bFirstTimer;
};












#endif
