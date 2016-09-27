
#ifndef __THINGSERVER_ICLONER_H__
#define __THINGSERVER_ICLONER_H__

#include "IActor.h"


struct ICloner : public IActor
{
	//获得真身UID
	virtual UID GetRealBodyUID() = 0;
};







#endif

