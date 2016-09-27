
#ifndef __THINGSERVER_DOUFAPART_H__
#define __THINGSERVER_DOUFAPART_H__


#include "IDouFaPart.h"
#include <vector>
#include "IEventServer.h"
#include <map>
#include "ITimeAxis.h"
#include "ICombatPart.h"

struct IActor;




struct DouFaEnemyInfo
{
	DouFaEnemyInfo() : m_UserEnemy(UID()),m_bFinish(false), m_bWin(false){}
	UID		m_UserEnemy;
	UINT16	m_Facade;
	char	m_Name[THING_NAME_LEN];
	bool	m_bFinish;		//是否已经比试过
	bool	m_bWin;
	UINT8	m_vipLevel;		//vip等级
};

//夺宝数据
struct DuoBaoInfo
{
	DuoBaoInfo() : m_CreditUp(0), m_GetCredit(0), m_LastChangeCreditUp(0), m_UserLvIndex(INVALID_INDEX) ,m_DuoBaoLvIndex(INVALID_INDEX), m_GroupID(0), m_RunBeginTime(0), m_bTeam(false) {}

	UINT32	m_CreditUp;				//本周声望上限

	UINT32	m_GetCredit;			//本周获得声望

	UINT32	m_LastChangeCreditUp;	//最后一次修改声望上限的时间

	INT8	m_UserLvIndex;			//参加夺宝的玩家等级组下标

	INT8	m_DuoBaoLvIndex;		//参加夺宝的夺宝等级组下标

	TGroupID m_GroupID;				//配对成功组ID

	bool	m_bTeam;				//是否组队夺宝

	UINT32	m_RunBeginTime;			//夺跑逃跑时间
};

//挑战数据
struct ChallengeInfo
{
	//今日参加的挑战次数
	INT16						m_JoinChallengeNum;

	//最后一次参加挑战的时间
	UINT32						m_LastJoinChallengeTime;

	//今日最多可挑战次数
	UINT16						m_MaxChallengeNumToday;
};


class DouFaPart : public IDouFaPart, public IEventListener , public  ITimerSink,public ICombatObserver
{
	enum enListenType
	{
		enListenType_DouFa = 0,	//斗法
		enListenType_QieCuo,	//切磋
	};

	enum enDouFaTimer_ID
	{
		enDouFaTimer_ID_Challenge = 0, //挑战定时器
		enDouFaTimer_ID_Run,			//逃跑惩罚
	};

public:
	DouFaPart();
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

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata);

	virtual void OnEvent(XEventData & EventData);

	virtual void OnTimer(UINT32 timerID);

public:
	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

public:
	//打开斗法标签
	virtual void OpenDouFaLabel();

	//打开切磋标签
	virtual void OpenQieCuoLabel();

	//斗法
	virtual void DouFaCombat(UID uidEnemy);

	//获得今天参加的挑战次数
	virtual INT16 GetChallengeNum();

	//得到今天获得的荣誉值
	virtual INT32	GetGetHonorToday();

	//刷新切磋对手
	virtual void FlushQieCuoEnemy();

	//切磋战斗
	virtual void QieCuoBattle(UID uidEnemy);

	//刷新斗法的对手
	virtual void FlushDouFaEnemy();

	//增加荣誉值
	virtual void AddHonor(INT32 nHonor);

	//参加挑战
	virtual void JoinChallenge();

	//清除挑战次数
	virtual void ClearChallengeNum();

	//最大可参加挑战次数
	virtual UINT8 MaxChallengeNum();

	//得到今天最大可获得荣誉值
	virtual INT32	GetMaxGetHonorToday();

	//设置今天最大挑战次数
	virtual void SetMaxChallengeToday(UINT16 MaxChallengeNum);

	//得到本周声望上限
	virtual	UINT32 GetMaxCreditWeek();

	//得到本周获得声望
	virtual UINT32 GetCreditWeek();

	//夺宝获得声望
	virtual void   AddCreditDuoBao(INT32 AddCredit);

	//是否正在夺宝中
	virtual bool   IsInDuoBao();

	//设置玩家参加夺宝的玩家等级组下标
	virtual void   SetUserLvIndex(INT8 UserLvIndex);

	//得到玩家参加夺宝的玩家等级组下标
	virtual INT8   GetUserLvIndex();

	//得到配对组ID
	virtual TGroupID GetGroupID();

	//设置配对组ID
	virtual void	SetGroupID(TGroupID groupID);

	//是否在逃跑惩罚期间
	virtual bool	IsInRunTime();

	//开启逃跑惩罚
	virtual void	StartRunTime();

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing();

	//得到逃跑惩罚剩余时间
	virtual UINT32	GetLeftRunTime();

	//是否组队夺宝
	virtual bool	IsTeamDuoBao();

	//设置是否组队夺宝
	virtual void	SetIsTeamDuoBao(bool bIsTeam);

	//清除夺宝数据
	virtual void	ClearDuoBaoData();

	//设置玩家参加夺宝的夺宝等级组下标
	virtual void	SetDuoBaoLvIndex(INT8 DuoBaoLvIndex);

	//得到玩家参加夺宝的夺宝等级组下标
	virtual INT8	GetDuoBaoLvIndex();

private:
	//开始切磋战斗
	void	BeginQieCuo(UID uidEnemy);

	//开始斗法战斗
	void	BeginDouFaCombat(UID uidEnemy);

	//斗法每场比赛结果处理
	void	DouFaForward(UID uidEnemy, bool bWin,const OBuffer4k & ob);

	//检测斗法是结束
	bool	IsDouFaOver(bool & bWin);

	//斗法结束处理
	void	DouFaOverForward(bool bWin);

	//OnEnvent的切磋分支
	void	OnEventQieCuo(UID uidEnemy);

	//OnEnvent的斗法分支
	void	OnEventDouFa(UID uidEnemy);

	//发送玩家信息给客户端
	void	SendActorInfo(UID uidUser);

	//进入切磋场景
	bool	EnterQieCuoScene();

	//进入斗法场景
	bool	EnterDouFaScene();

	//切磋得到敌人
	void	HandleQieCuoGetEnemy(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//同步切磋数据
	void	SynQieCuoData();

	//监听玩家的创建
	void	StartListenUserCreate(enListenType ListenType);

	//取消监听玩家的创建
	void	RemoveListenUserCreate(enListenType ListenType);

	//切磋每场比赛结果处理
	void	QieCuoForward(UID uidEnemy, bool bWin, const OBuffer4k & ob);

	//同步斗法数据
	void	SynDouFaData();

	void	HandleFlushDouFaEnemy(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//重新设置今天最多可挑战次数
	void	ResetMaxChallengeNum();

	void	HandleGetMyRankChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	HandleGetMyRankChallengeRecordBuf(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

private:
	IActor *						m_pActor;

	//斗法可选对手列表
	std::map<UID,  DouFaEnemyInfo>	m_mapEnemy;

	//切磋可选对手列表
	std::map<UID, QieCuoUserData>	m_mapQieCuo;


	//今日已获取荣誉值
	INT32							m_GetHonorToday;

	//最后一次获得荣誉的时间
	UINT32							m_LastGetHonorTime;

	//斗法每日获得的荣誉上限
	INT32							m_MaxGetHonorDouFaToday;

	//挑战数据
	ChallengeInfo					m_ChallengeData;

	//夺宝数据
	DuoBaoInfo						m_DuoBaoData;

	//斗法是否有监听人物创建事件
	bool							m_bListenDouFa;

	//切磋是否有监听人物创建事件
	bool							m_bListenQieCuo;

	UINT64                          m_QieCuoCombatID; //切磋战斗

	UINT64                          m_DouFaCombatID; //正在进行的斗法战斗

	UINT8							m_LastFLushDouFaUpLevel;	//上次抽取斗法玩家的等级上限
};













#endif
