#ifndef __THINGSERVER_EMPOLYEENUMTASK_H__
#define __THINGSERVER_EMPLOYEENUMTASK_H__
//招募第几名角色的任务
#include "EventTask.h"

class EmployeeNumTask : public EventTask
{
public:
	EmployeeNumTask(TaskPart * pTaskPart,const STaskCnfg *  pTaskCnfg);

	virtual bool Create();

	virtual void OnEvent(XEventData & EventData);
};

#endif
