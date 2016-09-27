
#ifndef __THINGSERVER_FUMODONGPART_H__
#define __THINGSERVER_FUMODONGPART_H__

#include "IFuMoDongPart.h"
#include "ITimeAxis.h"
#include "IEventServer.h"
#include "ICombatPart.h"
#include "IConfigServer.h"
#include <hash_map>

struct IActor;

struct SFuMoDongData
{
	UINT32  m_EndOnHookTime;  //开始挂机时间，为零表示不在挂机状态
	INT32   m_AccelNumOfDay;          //当天加速次数
	UINT32  m_LastAccelTime;      //最后一次加速时间
	INT32   m_Level;                //伏魔洞的层次
	UINT32  m_LastGiveExpTime;   //最后一次给予挂机经验时间
	INT32   m_GiveExp;          //每次给予的经验
};

struct SCombatMonster
{
	UINT8   m_Level;
	UID     m_uidMonster;
};

class FuMoDongPart : public IFuMoDongPart
	               , public  ITimerSink,public IEventListener,public ICombatObserver
				 
{
	enum enFuMoDongPartTIMER_ID
	{
		enFuMoDongPartTIMER_ID_GiveExp = 0, //每分钟给予经验
	};

public:
	FuMoDongPart();
	virtual ~FuMoDongPart();

public:
	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);


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

		//进入
	virtual void Enter();

	//自动打怪
	virtual void AutoKillMonster(CS_AutoKillMonster_Req & Req);

	//取消自动打怪
	virtual void CancelKillMonster(CS_CancelKillMonster_Req & Req);

	//加速打怪
	virtual void AccelKillMonster(CS_AccelKillMonster_Req & Req);

	//手动打怪
	virtual void KillMonster(CS_KillMonster_Req & Req);

	//清除今天挂机加速的使用次数
	virtual void ClearAccellNum();

	virtual void OnEvent(XEventData & EventData);

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing();

private:

	//同步挂机数据
	void SyncOnHookData();

	//结束挂机状态
	void StopOnHook();

	//获得挂机余下时间
	INT32 GetOnHookRemainTime();

	//启动给予经验定时器
	void StartGiveExpTimer();

	//计算每次可以获得的经验
	INT32 CalculateExpPerTimes();

	//定时给予经验
	INT32 GiveExp();

	//获得给予经验时间间隔
	INT32 GetGiveExpTimeSpace();

	//获得当天加速次数
	INT32 GetAccelNumOfDay();

	//获得每天最大加速次数
    INT32 GetMaxAccelNumOfDay();

	//获得每次加速需要消耗的仙石
	INT32 GetConsumeMoney();

	//获得每次加速获得的经验
	INT32 GetAccelExp();

	//获得每次加速减少的挂机时间
	INT32 GetReduceTime();

	enKMRetCode CombatWithNpc(UINT8 level);

	   //计算打怪可以获得的经验
 INT32 CalculateExp(TMonsterID MonsterID);

	//得到伏魔洞可挂机时长
	UINT32	GetAutoKillHourNum();

	//定时给予掉落
	void GiveDropGoods(std::vector<TGoodsID> & vectGoods);

	//定时给予自动打怪奖励
	void GiveAutoForward();

	//自动打怪
	bool BeginAutoKillMonster(const SFuMoDongCnfg* pFuMoDongCnfg);

	//打怪获得经验数
	UINT32 GetCombatExpNum();


private:
	SFuMoDongData m_FuMoDongData;
	IActor * m_pActor;

	typedef std::hash_map<UINT64,SCombatMonster> MAP_MONSTER;

	MAP_MONSTER m_mapMonster;
};




#endif
