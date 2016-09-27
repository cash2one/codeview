#ifndef __THINGSERVER_ISYNPART_H__
#define __THINGSERVER_ISYNPART_H__

#include "IThingPart.h"

struct ISynPart : public IThingPart
{
	//获得今天参加帮战的战斗次数
	virtual UINT16	GetJoinCombatNumToday() = 0;

	//获得今天最多可参加帮战的战斗次数
	virtual UINT16	GetMaxJoinCombatNumToday() = 0;

	//设置今天最多可参加帮战的战斗次数
	virtual void	SetMaxJoinCombatNumToday(UINT16 MaxJoinCombatNum) = 0;

	//获得今天得到的声望
	virtual INT32	GetGetNumWeek() = 0;

	//增加今天的战斗次数
	virtual void	AddCombatNumToday(INT16 AddNum) = 0;

	//增加今天获得的声望
	virtual void	AddGetCredit(INT32 Credit) = 0;

};

#endif
