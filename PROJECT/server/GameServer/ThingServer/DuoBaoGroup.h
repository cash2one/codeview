#ifndef __THINGSERVER_DUOBAOGROUP_H__
#define __THINGSERVER_DUOBAOGROUP_H__

#include "IActor.h"
#include <vector>
#include <map>
#include <set>
#include <list>
#include "ThingServer.h"
#include "Pair.h"

class DuoBaoGroup;
class DuoBaoWar;

//夺宝等级组
class DuoBaoLevelGroup
{
public:
	DuoBaoLevelGroup(INT32 MinLevel, INT32 MaxLevel, DuoBaoGroup * pDuoBaoGroup);

	//玩家的夺宝等级是否属于该组
	bool	IsInGroup(INT32 DuoBaoLv);

	//开始配对
	void	BeginPair(PairGroup * pGroup);

	//取出最后没配对成功的玩家
	UID		Pop_NoPairOK();

	//清空等待配对表
	void	ClearWaitPair();

	//加入配对
	void	PushToPair(UID uidUser);

	//取消配对
	bool	CancelPair(IActor * pActor);

private:
	INT32 m_MinLevel;
	INT32 m_MaxLevel;

	//所属的玩家等级组
	DuoBaoGroup * m_pDuoBaoGroup;

	//正在等待配队的玩家
	std::vector<UID>	m_vecWait;
};

//配对组
class PairGroup : public ITimerSink
{
	enum enPairGroup
	{
		enPairGroup_Ready = 0,	//准备战斗倒计时
	};
public:
	bool Create(DuoBaoWar * pDuoBaoWar, TGroupID GroupID);

	void ClearData();

public:

	void Push_Pair(IActor * pActor, IActor * pEnemy, Pair * pPair, bool bTeam);

	virtual void OnTimer(UINT32 timerID);

	TGroupID GetGroupID();

	//配对结束
	void PairEnd();

	//选择退出战斗
	void	QuitCombat(IActor * pActor);

	//选择战斗
	void	SelectCombat(IActor * pActor);

private:
	TGroupID							m_GroupID;

	std::vector<Pair *>					m_vecPair;

	std::map<UID, Pair *>				m_mapPair;

	DuoBaoWar						  * m_pDuoBaoWar;
};

//玩家等级组
class DuoBaoGroup
{
public:
	DuoBaoGroup(UINT8 MinLevel,UINT8 MaxLevel, bool bTeamGroup, DuoBaoWar * pDuoBaoWar);

	bool	Create();

	//开始匹配
	void	BeginPair();

	//进行跨级配对
	void	PairOtherLevel(PairGroup * pGroup);

	//此两玩家配对
	void	PairOK(UID uidUser, UID uidEnemy, PairGroup * pGroup);

	//增加玩家的匹配次数
	void	AddUserPairNum(UID uidUser);

	//加入配对
	UINT8	JoinDuoBao(IActor * pActor, INT8 & DuoBaoLvIndex);

	//取消配对
	void	CancelPair(UID uidUser);

private:
	UINT8								m_MinLevel;
	UINT8								m_MaxLevel;

	//是否是组队模式
	bool								m_bTeamGroup;

	//各夺宝等级组
	std::vector<DuoBaoLevelGroup>		m_vecDuoBaoLvGroup;

	//记录下玩家的匹配次数
	std::map<UID, UINT8>				m_mapPairNum;

	DuoBaoWar						  * m_pDuoBaoWar;
};


#endif
