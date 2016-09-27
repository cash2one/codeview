
#ifndef __THINGSERVER_IFUMODONG_H__
#define __THINGSERVER_IFUMODONG_H__

#include "IThingPart.h"
#include "GameSrvProtocol.h"

struct IFuMoDongPart : public IThingPart
{
	//进入
	virtual void Enter()=0;

	//自动打怪
	virtual void AutoKillMonster(CS_AutoKillMonster_Req & Req)=0;

	//取消自动打怪
	virtual void CancelKillMonster(CS_CancelKillMonster_Req & Req)=0;

	//加速打怪
	virtual void AccelKillMonster(CS_AccelKillMonster_Req & Req)=0;

	//手动打怪
	virtual void KillMonster(CS_KillMonster_Req & Req)=0;

	//清除今天挂机加速的使用次数
	virtual void ClearAccellNum() = 0;

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing() = 0;
};


#endif
