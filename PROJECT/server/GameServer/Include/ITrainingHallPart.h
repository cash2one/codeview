
#ifndef __THINGSERVER_ITRAININGHALL_H__
#define __THINGSERVER_ITRAININGHALL_H__

#include "IThingPart.h"


struct ITrainingHallPart : public IThingPart
{
	//进入
	virtual void Enter() = 0;

	//开始练功
	virtual void StartTraining()=0;

	//停止练功
	virtual void StopTraining() = 0;

	//立即完成练功
	virtual void TrainingFinishNow() = 0;

	//清除练功堂次数限制
	virtual void ClearTrainNum() = 0;

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing() = 0;

	//开始习武
	virtual void StartXiWu(UID uidActor) = 0;

	//取消习武
	virtual void CancelXiWu() = 0;
};


#endif

