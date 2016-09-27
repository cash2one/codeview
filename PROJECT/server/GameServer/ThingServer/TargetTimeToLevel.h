#ifndef __THINGSERVER_TARGETTIMETOLEVEL_H__

#define __THINGSERVER_TARGETTIMETOLEVEL_H__

#include "EventActivity.h"
#include "IDBProxyClient.h"

class TargetTimeToLevel : public EventActivity,public IDBProxyClientSink
{
public:
	virtual bool Create(const  SActivityCnfg * pActivityCnfg, bool bRegistEvent = true);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata = 0);

	virtual void OnEvent(XEventData & EventData);

private:
	void	HandleGetFirstOpenServerTime(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
};

#endif
