#ifndef __THINGSERVER_IGODSWORDSHOP_H__
#define __THINGSERVER_IGODSWORDSHOP_H__

#include "IThingPart.h"
#include "GameSrvProtocol.h"

struct IGodSwordShopPart : public IThingPart
{
	//进入剑冢
	virtual void  EnterGodSwordShop() = 0;
	//刷新剑冢
	virtual void  FlushGodSwordShop(enFlushType FlushType) = 0;
	//购买仙剑
	virtual void  BuyGodSword(UINT8 nIndex) = 0;
	//同步剑冢
	virtual void  SyncGodSwordShop() = 0;

	//获得剩余刷新仙剑时间
	virtual INT32 GetRemainFlushTime() = 0;
};

#endif
