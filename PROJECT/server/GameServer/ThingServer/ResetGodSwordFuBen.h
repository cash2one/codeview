#ifndef __THINGSERVER_RESETGODSWORDFUBEN_H__
#define __THINGSERVER_RESETGODSWORDFUBEN_H__

#include "IVisitWorldThing.h"
#include "IActor.h"
#include "IFuBenPart.h"

//每天24点重置在线玩家的仙剑副本

class ResetGodSwordFuBen : public IVisitWorldThing
{
public:
	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing){
			return;
		}

		IActor * pActor = static_cast<IActor *>(pThing);

		if( 0 != pActor->GetMaster()){
			//招募角色没有副本PART
			return;
		}
		
		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
		if( 0 == pFuBenPart){
			return;
		}

		pFuBenPart->ResetGodSwordFuBen();
	}
};

#endif
