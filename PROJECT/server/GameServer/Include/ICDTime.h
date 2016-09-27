#ifndef __GOODSSERVER_ICDTIME_H__
#define __GOODSSERVER_ICDTIME_H__


#include "DSystem.h"
#include "UniqueIDGenerator.h"
struct IActor;

//使用冷却时间的要继承这个接口
struct ICDTimeEndHandler
{
	//如果一个类里有多个用到CD时间，可以用flag来区分不同的
	virtual void  CDTimeEndBackFunc(const UID uidActor, TCDTimerID CDTime_ID) = 0;
};


struct ICDTimeMgr
{
	//////////////////////////////////////////////////////////////////////////
	//登记冷却的信息(会检测配置表中是否有，没有则不登记)
	//func:CD时间到时的回调函数
	//////////////////////////////////////////////////////////////////////////
	virtual void RegistCDTime(IActor * pActor, TCDTimerID CDTime_ID, ICDTimeEndHandler * pCDTimeEndHandler = 0) = 0;

	//////////////////////////////////////////////////////////////////////////
	//检测指定的CD时间是否已到
	//func:CD时间到时的回调函数
	//CD时间已到返回true,CD时间未到返回false
	//////////////////////////////////////////////////////////////////////////
	virtual bool IsCDTimeOK(IActor * pActor, TCDTimerID CDTime_ID) = 0;

	//卸载这个玩家身上的所有冷却时间
	virtual void UnLoadCDTime(IActor * pActor) = 0;

	//卸载这个玩家身上指定的冷却时间
	virtual void UnLoadCDTime(IActor * pActor, TCDTimerID CDTime_ID) = 0;


};

#endif
