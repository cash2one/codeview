
#ifndef __THINGSERVER_DOUFAMGR_H__
#define __THINGSERVER_DOUFAMGR_H__


#include "IMessageDispatch.h"
#include "IDBProxyClient.h"
#include "Challenge.h"
#include "ITimeAxis.h"
#include "DuoBaoWar.h"

class DouFaMgr : public IMsgRootDispatchSink, public IDBProxyClientSink, public ITimerSink
{
public:
	enum enTimerID
	{
		enTimerID_Sunday,		//每周日0点0分0秒
		enTimerID_Everyday,		//每天发放挑战奖励
	};

public:
    DouFaMgr();
	~DouFaMgr();

	
	bool Create();

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata = 0);

	virtual void OnTimer(UINT32 timerID);

	//设置每周日晚0点0时的定时器
	virtual void SetSundayTimer();

	//设置每天晚上0点0时的定时器
	virtual void SetEverydayTimer();

	//退出或掉线或顶号或离队
	void	QuitDuoBao(IActor * pQuiter);

private:
	//打开斗法标签
	void OpenDouFaLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开切磋标签
	void OpenQieCuoLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//斗法
	void DouFaCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开夺宝标签
	void OpenDuoBaoLabe(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//加入夺宝战
	void AddToDuoBaoCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//刷新切磋对象
	void FlushQieCuo(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//切磋战斗
	void QieCuoBattle(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//刷新斗法对手
	void FlushDouFaEnemy(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//刷新我的挑战排行
	void FlushMyChalleng(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看挑战等级组排行
	void ViewChallengeLvGroup(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//挑战玩家
	void ChallengeUser(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看挑战奖励
	void ViewChallengeForward(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//每周日0点对区间挑战前5%名的奖励(前5名除外)
	void ChallengeForwardLvGroup_B();

	//每周日0点对区间挑战前5名的奖励
	void ChallengeForwardLvGroup();

	//
	void HandleChallengeLvGroupForward(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleChallengeLvGroupTotalNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);


	//打开夺宝
	void OpenDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//加入夺宝
	void JoinDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//点击战斗准备框按钮
	void ReadySelect(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//刷新我的挑战回放
	void FlushMyChallengRecord(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看战斗回放
	void ViewChallengeRecord(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//设置区间挑战总人数
	void SetTotalNumLvGroup();

	//得到区间挑战总人数
	INT32 GetTotalNumLvGroup();

	//取消夺宝
	void CancelDuoBao(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//购买战斗次数
	void BuyCombatNum(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	Challenge	m_Challenge;

	DuoBaoWar	m_DuoBaoWar;

	bool		m_bFirstTimer;

	INT32		m_TotalNum;

};









#endif

