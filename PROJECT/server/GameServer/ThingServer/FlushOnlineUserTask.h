#ifndef __THINGSERVER_FLUSHONLINEUSERTASK_H__
#define __THINGSERVER_FLUSHONLINEUSERTASK_H__

#include "IVisitWorldThing.h"
#include "IActor.h"
#include "ITaskPart.h"

//刷新在线玩家任务

class FlushOnlineUserTask : public IVisitWorldThing
{
public:
	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing){
			return;
		}

		IActor * pActor = static_cast<IActor *>(pThing);

		if( 0 == pActor->GetCrtProp(enCrtProp_ActorUserID)){
			//招募角色没有任务
			return;
		}
		
		ITaskPart * pTaskPart = pActor->GetTaskPart();
		if( 0 == pTaskPart){
			return;
		}

		pTaskPart->FlushTask();
	}
};

#endif
