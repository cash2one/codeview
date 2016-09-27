
#ifndef __THINGSERVER_ACTIVITY_PART_H__
#define __THINGSERVER_ACTIVITY_PART_H__

#include "IActivityPart.h"
#include <hash_map>
#include "FieldDef.h"
#include "GameSrvProtocol.h"
#include "ITimeAxis.h"

struct IActor;

//一个活动的数据
struct SActivityData
{
	SActivityData()
	{
		MEM_ZERO(this);
	}

	UINT16       m_ActivityID;  //活动ID
	bool         m_bFinished;   //是否已完成
	bool         m_bTakeAward;  //是否已领奖
	UINT32       m_ActivityProgress;  //进度
};



class ActivityPart : public IActivityPart , public ITimerSink
{
	enum { 
		     enTimerID_Online = 0,  //在线奖励倒计时
			 enTimerID_Update    ,  //24:00更新至初始状态
	     };
public:
     ActivityPart();
	 virtual ~ActivityPart();


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


	//打开日常
	virtual void  OpenDialy();

	//打开签到
	virtual void OpenSignIn();

	//签到
	virtual void SignIn();

	//领取签到奖励
	virtual void TakeSignInAward(UINT8  AwardID);

	//打开活动
	virtual void OpenActivity();

		//领取活动奖励
	virtual void TakeActivityAward(UINT16  ActivityID);

			//推进进度
	virtual void AdvanceProgress(UINT16  ActivityID ); 

		//领取在线奖励
	virtual void TakeOnlineAward();

	   //通知领取在线奖励
void ActivityPart::OnlineAwardNotic();

	 //获得新手指引的步骤
	 virtual INT16 GetNewPlayerGuideIndex() ;

	 //设置新手指引步骤
	 virtual void SetNewPlayerGuideIndex(INT16 Index);

	 	 	 //获得新手指引上下文
	 virtual const char* GetNewPlayerGuideContext();

	 //设置新手指引上下文
	 virtual void SetNewPlayerGuideContext(const char * pContext);

	 	 //多倍经验
	 virtual void OnMultipExp(); 

public:
	void OnTimer(UINT32 timerID);

private:
	//获得当天签到次数
UINT8 ActivityPart::GetSignInNumOfDay();
//获得当月签到次数
UINT8 ActivityPart::GetSignInNumOfMonth();

void  ActivityPart::GetDailyInfo( SDailyInfo & DailyInfo);

  SActivityData * GetActivityData(UINT16 ActivityID);



  //启动在线奖励定时器
  void StartTimer();

  //停止定时器
  void StopTimer();

private:
	IActor * m_pActor; 

	INT16       m_NewPlayerGuideIndex;  //新手索引步骤

	std::string m_strGuideContext;

	UINT32      m_NextOnlineAwardTime;  //下次在线奖励时间

	UINT16      m_OnLineAwardID;       //在线奖励ID
	bool        m_bCanTakeOnlineAward; //是否可以领取在线奖励
	UINT32      m_LastOnLineAwardRestTime; //在线奖励，最后一次复位时间
	UINT8       m_OffLineNum; //连接下线次数


	bool        m_bClientOpenActivity; //客户端是否打开过活动栏

	UINT32      m_LastSignInTime;    //最后一次签到时间
	UINT8       m_SignInNumOfMonth;  //本月签到次数

	TBitArray<MAX_DAILY_AWARD_NUM>  m_AwardHistory;  //领奖记录

	typedef std::hash_map<UINT16,SActivityData>  MAP_ACTIVITY;

	MAP_ACTIVITY  m_mapActivity; //活动
};











#endif
