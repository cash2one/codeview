#ifndef _THINGSERVER_ITEAMPART_H__
#define _THINGSERVER_ITEAMPART_H__

#include "IThingPart.h"
#include "UniqueIDGenerator.h"

struct IActor;

enum enCreateTeamType
{
	enCreateTeamType_Point = 0,		//指定组队
	enCreateTeamType_Fast,			//快速组队
};

struct ITeamPart : public IThingPart
{
	virtual ~ITeamPart(){}

	//当前是否有组队
	virtual	bool	IsHaveTeam()	= 0;

	//得到队友
	virtual IActor *GetTeamMember() = 0;

	//得到队长
	virtual IActor *GetTeamLeader() = 0;

	//邀请组队
	virtual bool	InviteTeam(UID uidUser, enCreateTeamType type) = 0;

	//同意组队
	virtual void	AgreeTeam(UID uidUser) = 0;

	//拒绝组队
	virtual void	RefuseTeam(UID uidUser) = 0;

	//退出队伍
	virtual bool	QuitTeam() = 0;

	//创建队伍
	virtual UINT8	CreateTeam(UID uidUser, bool bTeamLeader) = 0;

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing() = 0;

	//设置是否在快速组队
	virtual void	SetIsInFastTeam(bool bInFastTeam) = 0;

	//得到是否在快速组队
	virtual bool	GetIsInFastTeam() = 0;

	/************下面是同步两人队伍PART数据的函数*********************/
public:
	
	virtual void	SetTeamData(UID uidUser, bool bLeader) = 0;

	virtual void	ClearTeamData(bool bMeQuit, bool bSendMsg = true) = 0;
};

#endif
