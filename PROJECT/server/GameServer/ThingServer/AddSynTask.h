#ifndef __THINGSERVER_ADDSYNTASK_H__
#define __THINGSERVER_ADDSYNTASK_H__

//加入帮派任务

#include "EventTask.h"

class AddSynTask : public EventTask
{
public:
	AddSynTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);

	virtual bool Create();
};

#endif
