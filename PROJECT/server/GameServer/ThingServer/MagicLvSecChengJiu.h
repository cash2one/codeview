#ifndef __THINGSERVER_MAGICLVSECCHENGJIU_H__
#define __THINGSERVER_MAGICLVSECCHENGJIU_H__

//学习法术等级区间成就
#include "EventChengJiu.h"

class MagicLvSecChengJiu : public EventChengJiu
{
public:
	virtual bool Create(const  SChengJiuCnfg * pChengJiuCnfg);

	virtual void OnEvent(XEventData & EventData);
};


#endif
