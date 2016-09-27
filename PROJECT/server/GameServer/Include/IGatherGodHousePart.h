

#ifndef __THINGSERVER_IGATHERGODHOUSE_PART_H__
#define __THINGSERVER_IGATHERGODHOUSE_PART_H__

#include "ITHingPart.h"
#include "GameSrvProtocol.h"

struct IGatherGodHousePart : public IThingPart
{
	//进入
	virtual void Enter() = 0;

	//查看
	virtual void ViewItem(CS_ViewItem_Req & Req) = 0;

	//刷新
	virtual void FlushItem(CS_FlushItem_Req & Req) = 0;

	//购买
	virtual void BuyItem(CS_BuyItem_Req & Req) = 0;

	//获得刷新招幕人员余下时间
	virtual INT32 GetFlushEmployRemainTime() = 0; 

		//获得刷新法术书余下时间
	virtual INT32 GetFlushMagicBookRemainTime() = 0; 

	//自动刷新招募角色
	virtual void  AutoFlushEmployee(UINT16 FlushNum, INT32 Aptitude) = 0;
};


#endif

