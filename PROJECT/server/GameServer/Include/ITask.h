
#ifndef __THINGSERVER_ITASK_H__
#define __THINGSERVER_ITASK_H__

#include "BclHeader.h"
#include "DSystem.h"
#include "TBuffer.h"

struct STaskCnfg;
struct IActor;


struct ITask
{
	virtual bool Create() = 0;

	virtual void Release() = 0;

	//获取任务ID
	virtual TTaskID  GetTaskID() = 0;

	//是否已完成
	virtual bool  IsFinished() = 0;

	virtual const STaskCnfg * GetTaskCnfg() = 0;

	//获得DB数据
	virtual bool  GetDBContext(void * pBuffer,INT32 & len) = 0;

	virtual bool  SetDBContext(const void * pBuffer,INT32  len) = 0;

	//任务复置
	virtual void Reset() = 0;

	//任务信息放到buffer中,发送给客户端
	virtual bool AddToBuffer(IActor * pActor, OBuffer4k & ob) = 0;

	//领取奖励
	virtual bool TakeAward() = 0;

	//任务状态有更新
	virtual void NotifyClientUpdateTaskState() = 0;

	//任务数据信息放到buffer中
	virtual bool AddDBToBuffer(OBuffer4k & ob) = 0;

	//GM命令让任务结束
	virtual bool GMFinishTask() = 0;

	//设置是否需要更新到数据库
	virtual void SetNeedUpdate(bool bNeedUpdate) = 0;

	//获取是否需要更新到数据库
	virtual bool GetNeedUpdate() = 0;
};


#endif

