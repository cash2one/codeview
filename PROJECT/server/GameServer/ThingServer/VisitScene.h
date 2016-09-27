#ifndef __THINGSERVER_VISITSCENE_H__
#define __THINGSERVER_VISITSCENE_H__

#include "ThingServer.h"
#include "IThing.h"

class VisitScene
{
public:
	bool operator ()(IThing* pThing)
	{
		if( 0 == pThing){
			return false;
		}

		if(pThing->GetThingClass()==enThing_Class_Actor)
		{
			IActor * pActor = (IActor*)pThing;
			pActor->ComeBackMainScene();
			return true;
		}

		g_pGameServer->GetGameWorld()->DestroyThing(pThing->GetUID());

		return true;
	}
};

#endif
