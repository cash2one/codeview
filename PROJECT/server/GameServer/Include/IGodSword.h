
#ifndef __THINGSERVER_IGODSWORD_H__
#define __THINGSERVER_IGODSWORD_H__


#include "IEquipment.h"
#include <string>

struct ICreature;

struct IMagic;
struct IActor;

struct IGodSword : public IEquipment
{
	//获得剑气值
	virtual int  GetSwordkee() = 0;

	//获得法术值
   virtual int  GetMagicValue() = 0;


   	//获得法术
	virtual IMagic * GetMagic() = 0;

	//提升仙剑级别
	virtual bool UpgradeSword(IActor * pActor,INT8 value = 1) = 0;

};



#endif
