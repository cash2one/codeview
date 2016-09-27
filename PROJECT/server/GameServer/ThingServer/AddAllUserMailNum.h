#ifndef __THINGSERVER_ADDALLUSERMAILNUM_H__
#define __THINGSERVER_ADDALLUSERMAILNUM_H__

//给所有玩家写系统邮件时，要增加所有在线玩家的邮件数量

#include "IThing.h"
#include "IActor.h"
#include "IMailPart.h"

class AddAllUserMailNum
{
public:
	void operator ()(IThing * pThing)
	{
		if( 0 == pThing || enThing_Class_Actor != pThing->GetThingClass()){
			return;
		}

		IActor * pActor = (IActor * )pThing;

		if( pActor->GetMaster() != 0){
			return;
		}

		IMailPart * pMailPart = pActor->GetMailPart();
		if( 0 == pMailPart){
			return;
		}

		pMailPart->AddMailNum(1);
	}
};

#endif
