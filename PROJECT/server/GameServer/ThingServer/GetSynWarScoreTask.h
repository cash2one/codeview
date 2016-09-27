#ifndef __THINGSERVER_GETSYNWARSCORETASK_H__
#define __THINGSERVER_GETSYNWARSCORETASK_H__

//获得帮派功勋
#include "EventTask.h"

class GetSynWarScoreTask : public EventTask
{
public:
	GetSynWarScoreTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);

	virtual void OnEvent(XEventData & EventData);
};


#endif
