
#ifndef __THINGSERVER_EVENTACTIVITY_H__
#define __THINGSERVER_EVENTACTIVITY_H__


#include "IEventServer.h"
#include "DSystem.h"

struct IActor;
struct SActivityCnfg;

//成就
struct IActivity
{
	virtual bool Create(const  SActivityCnfg * pActivityCnfg, bool bRegistEvent = true)=0;

	virtual void Release()=0;
	
	virtual const  SActivityCnfg * GetActivityCnfg()=0;
};


class EventActivity : public IActivity,  public IEventListener
{
public:
	EventActivity();
	virtual ~EventActivity();

	public:
	virtual void OnEvent(XEventData & EventData);

public:

	virtual bool Create(const  SActivityCnfg * pActivityCnfg, bool bRegistEvent = true);

	virtual void Release();

	virtual const  SActivityCnfg * GetActivityCnfg();

protected:
	//是否在活动有效期内
	bool	IsInActivityTime();

protected:
	const  SActivityCnfg * m_pActivityCnfg;
};

#endif

