

#ifndef __THINGSERVER_IMONSTER_H__
#define __THINGSERVER_IMONSTER_H__


#include "ICreature.h"

struct IMonster : public ICreature
{

	//获得物理伤害
	virtual INT32  GetPhysicalDamage() = 0;

	//获得法术伤害
	virtual INT32  GetMagicDamage() = 0;

	//获得防御值
	virtual INT32  GetDefense() = 0;

};




#endif

