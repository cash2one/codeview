
#ifndef __THINGSERVER_IDOUFAPART_H__
#define __THINGSERVER_IDOUFAPART_H__


#include "IThingPart.h"
#include "GameSrvProtocol.h"
#include "DBProtocol.h"

struct IDouFaPart : public IThingPart
{
	//打开斗法标签
	virtual void OpenDouFaLabel() = 0;

	//打开切磋标签
	virtual void OpenQieCuoLabel() = 0;

	//斗法战斗
	virtual void DouFaCombat(UID uidEnemy) = 0;

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) = 0;

	//获得今天参加的挑战次数
	virtual INT16 GetChallengeNum() = 0;

	//得到今天获得的荣誉值
	virtual INT32	GetGetHonorToday() = 0;

	//刷新切磋对手
	virtual void FlushQieCuoEnemy() = 0;

	//切磋战斗
	virtual void QieCuoBattle(UID uidEnemy) = 0;

	//刷新斗法的对手
	virtual void FlushDouFaEnemy() = 0;

	//增加荣誉值
	virtual void AddHonor(INT32 nHonor) = 0;

	//参加挑战
	virtual void JoinChallenge() = 0;

	//清除挑战次数
	virtual void ClearChallengeNum() = 0;

	//最大可参加活动次数
	virtual UINT8 MaxChallengeNum() = 0;

	//得到今天最大可获得荣誉值
	virtual INT32	GetMaxGetHonorToday() = 0;

	//设置今天最大挑战次数
	virtual void	SetMaxChallengeToday(UINT16 MaxChallengeNum) = 0;

	//得到本周声望上限
	virtual	UINT32 GetMaxCreditWeek() = 0;

	//得到本周获得声望
	virtual UINT32 GetCreditWeek() = 0;

	//夺宝获得声望
	virtual void   AddCreditDuoBao(INT32 AddCredit) = 0;

	//是否正在夺宝中
	virtual bool   IsInDuoBao() = 0;

	//设置玩家参加夺宝的玩家等级组下标
	virtual void   SetUserLvIndex(INT8 UserLvIndex) = 0;

	//得到玩家参加夺宝的玩家等级组下标
	virtual INT8   GetUserLvIndex() = 0;

	//得到配对组ID
	virtual TGroupID GetGroupID() = 0;

	//设置配对组ID
	virtual void	SetGroupID(TGroupID groupID) = 0;

	//是否在逃跑惩罚期间
	virtual bool	IsInRunTime() = 0;

	//开启逃跑惩罚
	virtual void	StartRunTime() = 0;

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing() = 0;

	//得到逃跑惩罚剩余时间
	virtual UINT32	GetLeftRunTime() = 0;

	//是否组队夺宝
	virtual bool	IsTeamDuoBao() = 0;

	//设置是否组队夺宝
	virtual void	SetIsTeamDuoBao(bool bIsTeam) = 0;

	//清除夺宝数据
	virtual void	ClearDuoBaoData() = 0;

	//设置玩家参加夺宝的夺宝等级组下标
	virtual void	SetDuoBaoLvIndex(INT8 DuoBaoLvIndex) = 0;

	//得到玩家参加夺宝的夺宝等级组下标
	virtual INT8	GetDuoBaoLvIndex() = 0;

};


#endif
