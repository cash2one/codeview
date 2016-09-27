#ifndef __THINGSERVER_GOODSNUMTASK_H__
#define __THINGSERVER_GOODSNUMTASK_H__
//要多少物品的任务
#include "EventTask.h"

class GoodsNumTask : public EventTask
{
public:
	GoodsNumTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);

	virtual bool Create();

	virtual void OnEvent(XEventData & EventData);
};

#endif
