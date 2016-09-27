#ifndef __THINGSERVER_ICDTIMERPART_H__
#define __THINGSERVER_ICDTIMERPART_H__

#include "IActor.h"
#include "IThingPart.h"
#include "DSystem.h"
#include "UniqueIDGenerator.h"


struct IActor;

//冷却时间到，要回调的可以继承这个接口
struct ICDTimerEndHandler
{
	virtual void  CDTimeEndBackFunc(const UID uidActor, int CDTime_ID) = 0;
};

struct ICDTimerPart : public IThingPart
{
	//////////////////////////////////////////////////////////////////////////
	//登记冷却的信息(会检测配置表中是否有，没有则不登记)
	//func:CD时间到时的回调函数
	//bMaster:是否是所有角色共用的
	//////////////////////////////////////////////////////////////////////////
	virtual void RegistCDTime(TCDTimerID CDTime_ID, bool bMaster = false, ICDTimerEndHandler * pCDTimeEndHandler = 0) = 0;

	//////////////////////////////////////////////////////////////////////////
	//检测指定的CD时间是否已到
	//bMaster:是否要获取主角的冷却时间(有些冷却是所有角色共用的)
	//CD时间已到返回true,CD时间未到返回false
	//////////////////////////////////////////////////////////////////////////
	virtual bool IsCDTimeOK(TCDTimerID CDTime_ID, bool bMaster = false) = 0;

	//卸载指定的冷却时间
	virtual void UnLoadCDTime(TCDTimerID CDTime_ID) = 0;

	//卸载所有冷却时间
	virtual void UnLoadAllCDTime() = 0;

	//得到CD剩余时间
	virtual UINT32 GetRemainCDTime(TCDTimerID CDTime_ID) = 0;
};

#endif
