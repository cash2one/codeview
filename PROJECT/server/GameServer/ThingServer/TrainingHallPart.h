

#ifndef __THINGSERVER_TRAININGHALL_H__
#define __THINGSERVER_TRAININGHALL_H__

#include "ITrainingHallPart.h"

#include "ITimeAxis.h"

#include "memory.h"

struct IActor;

//练功堂数据
struct STrainingHallData
{
	STrainingHallData()
	{
		MEM_ZERO(this);
	}

	INT32	 m_RemainTime;		//剩余时间
	UINT32	 m_LastFinishTime;	//最后一次完成练功时间
	INT16	 m_OnHookNum;		//已挂机次数
	//INT32	 m_GetExp;			//获得经验
};

class TrainingHallPart : public ITrainingHallPart
	                      , public  ITimerSink,public IEventListener
{
	enum enTrainingHallTIMER_ID
	{
		enTrainingHallTIMER_ID_GiveExp = 0, //每分钟给予经验
		enTrainingHallTIMER_ID_Night12 = 1,	//晚上12点定时器
		enTrainingHallTIMER_ID_EndXiWu = 2,	//结束习武的定时器
	};

public:
	 TrainingHallPart();
	 virtual ~TrainingHallPart();

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
	virtual void InitPrivateClient() ;
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

     virtual void Enter();

	//开始练功
	virtual void StartTraining();

	//停止练功
	virtual void StopTraining();

	virtual void OnTimer(UINT32 timerID);

	//立即完成练功
	virtual void TrainingFinishNow();

	//清除练功堂次数限制
	virtual void ClearTrainNum();

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing();

	virtual void OnEvent(XEventData & EventData);

	//开始习武
	virtual void StartXiWu(UID uidActor);

	//同步习武
	virtual void SycXiWuData();

	//取消习武
	virtual void CancelXiWu();

private:

	//同步练功数据
    void SyncTrainingData(INT32 nPerExp = 0);

	//获得每次练功的时长
	INT32 GetTrainingTimeSpace();

	//获得每次挂机可练功次数
	INT32 GetMaxTrainingNum();

	//获得剩余练功次数
	INT32 GetRemainNum();

	//获得每次练功可获得经验
	INT32 GetExpOfPerTraining();

	//每次挂机需要消防灵石
	INT32 GetConsumeSpiritStone();

	//启动定时器
	void StartTimer();

	//一次练功结束,给予经验
	UINT32  GiveExp(); 

	//获得每天最大挂机次数
	INT16 TrainingHallPart::GetTotalOnhookNum();

	//获得已挂机次数
   INT16 TrainingHallPart::GetCurOnhookNum();

   //上线如果还在练功中,则进入
   void  Enter_OnLine();

	//继续练功
   void  ContinueTraining();

   //得到使用VIP立即完成次数
   UINT8 GetVipFinishTrainNum();

   //得到可练功时长
   UINT32 GetTrainingTimeLong();

   //得到习武经验
   UINT32 GetXiWuExp();

   //习武结束
   void	  EndXiWu();

   //练功结束
   void	  EndTraining();

private:
	IActor *  m_pActor;
	STrainingHallData  m_TrainingHallData;

	enTrainingStatus	  m_Status;				//是否在暂停状态

	UINT8	   m_VipFinishTrainNum;			//今天使用VIP立即完成次数

	UINT32	   m_LastVipFinishTrainTime;	//最后使用VIP立即完成时间

	UINT32	   m_TrainingTimeLong;			//可练功总时长

	UINT32		m_ChangeTimeLongTime;		//最后改变可练功总时长的时间

	UINT32		m_BeginXiWuTime;			//习武开始时间

	UID			m_uidXiWuActor;				//参加习武的招募角色
};



#endif

