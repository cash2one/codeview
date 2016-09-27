#ifndef __THINGSERVER_CONTRIBUTIONTASK_H__
#define __THINGSERVER_CONTRIBUTIONTASK_H__
//使用帮派贡献的任务
#include "EventTask.h"

class ContributionTask : public EventTask
{
public:
	ContributionTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);

	virtual void OnEvent(XEventData & EventData);
};

#endif
