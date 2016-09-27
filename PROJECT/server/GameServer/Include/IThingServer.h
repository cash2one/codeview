
#ifndef __THINGSERVER_ITHINGSERVER_H__
#define __THINGSERVER_ITHINGSERVER_H__
#include "IThing.h"

#include "DSystem.h"
#include "UniqueIDGenerator.h"

struct IGameServer;
struct IGameWorld;

#ifndef BUILD_THINGSERVER_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"ThingServer.lib")
#endif
#endif


struct IThingServer
{
	//释放
	virtual void Release(void) = 0;

	virtual IGameWorld * GetGameWorld() = 0;

	virtual void Close()=0;

	
	//装载玩家入内存
	virtual bool LoadActor(UID uidActor) = 0;

	//接收后台发来的消息
	virtual void WebOnRecv(UINT8 nCmd, IBuffer & ib) = 0;
};

BCL_API  IThingServer * CreateThingServer(IGameServer *pServerGlobal);



#endif
