
#ifndef __THINGSERVER_ITALISMAN_H__
#define __THINGSERVER_ITALISMAN_H__



#include "IEquipment.h"
#include <string>

struct ICreature;

struct IMagic;

struct ITalisman : public IEquipment
{

	//获得品质等级
	virtual UINT8 GetQualityLevel() = 0;

	//获得法术
	virtual IMagic * GetMagic() = 0;

	//开始孕育物品
    virtual bool SpawnGoods(IActor * pActor) = 0;

	//是否在孕育物品
	virtual bool IsSpawnGoods() = 0;

	//该法宝是否是内法宝
	virtual bool IsInTalisman() = 0;

	//孕育时间是否已到
	virtual bool IsEndSpawnTime() = 0;

	//提升品质
	virtual bool UpgradeQuality(IActor * pActor,INT32 QualityPoint) = 0;

};




#endif
