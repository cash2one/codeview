#ifndef __THINGSERVER_TEAMMGR_H__
#define __THINGSERVER_TEAMMGR_H__

#include "IMessageDispatch.h"
#include <map>
#include <vector>
#include "DSystem.h"
#include "UniqueIDGenerator.h"

class TeamMgr : public IMsgRootDispatchSink
{
public:
	TeamMgr();

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	void	QuitWait(IActor * pActor);

	//副本快速组队
	void	FuBenFastCreateTeam(IActor *pActor, TFuBenID FuBenID);

private:
	//邀请组队
	void	InviteTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//同意组队
	void	AgreeTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//拒绝组队
	void	RefuseTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//退出队伍
	void	QuitTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//退出等待或者等待时间到
	void	QuitWaitOrTimeEnd(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//设置组队阵形
	void	SetTeamLineup(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开队伍信息
	void	OpenTeamInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//设置组队参战状态
	void	SetTeamCombatState(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	typedef std::map<TFuBenID, UID>		MAP_FASTTEAM;

	//正在等待快速组队的玩家
	MAP_FASTTEAM	m_WaitFastTeam;
};


#endif
