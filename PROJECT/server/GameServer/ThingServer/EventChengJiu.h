
#ifndef __THINGSERVER_EVENTCHENGJIU_H__
#define __THINGSERVER_EVENTCHENGJIU_H__

#include "IEventServer.h"
#include "DSystem.h"

struct IActor;
struct SChengJiuCnfg;

//成就
struct IChengJiu
{
	virtual bool Create(const  SChengJiuCnfg * pChengJiuCnfg)=0;

	virtual void Release()=0;
	
	virtual const  SChengJiuCnfg * GetChengJiuCnfg()=0;
};


class EventChengJiu : public IChengJiu,  public IEventListener
{
public:
	EventChengJiu();
	virtual ~EventChengJiu();

	public:
	virtual void OnEvent(XEventData & EventData);

public:

	virtual bool Create(const  SChengJiuCnfg * pChengJiuCnfg);

	virtual void Release();

	virtual const  SChengJiuCnfg * GetChengJiuCnfg();

protected:
	const  SChengJiuCnfg * m_pChengJiuCnfg;
};


#endif
