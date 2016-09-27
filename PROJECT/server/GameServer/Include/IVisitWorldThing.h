#ifndef __THINGSERVER_IVISITWORLDTHING_H__
#define __THINGSERVER_IVISITWORLDTHING_H__

struct IThing;

struct IVisitWorldThing
{
//	virtual void operator()(IThing * pThing) = 0;

	virtual void Visit(IThing * pThing) = 0;
};

#endif
