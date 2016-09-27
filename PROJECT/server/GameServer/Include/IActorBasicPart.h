
#ifndef __THINGSERVER_IACTORBASIC_PART_H__
#define __THINGSERVER_IACTORBASIC_PART_H__

#include "IThingPart.h"
#include "ICreature.h"

#include "UniqueIDGenerator.h"

struct IActorBasicPart : public IThingPart
{
	//属性变动
	virtual void OnActorPropChange(IActor* pActor,enCrtProp PropID,INT32 nNewValue,INT32 nOldValue) = 0;

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing() = 0;
};


#endif
