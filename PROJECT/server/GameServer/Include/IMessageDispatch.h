

#ifndef __GAMESERVER_IMESSAGEDISPATCH_H__
#define __GAMESERVER_IMESSAGEDISPATCH_H__

#include "ProtocolHeader.h"

struct IActor;

//MSG_ROOT消息通道
struct IMsgRootDispatchSink
{
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib) = 0;
};


//MSG_ROOT消息分发器
struct IMessageDispatch
{
	//订阅MSG_ROOT消息
	virtual bool AddRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink) = 0;
	//取消订阅MSG_ROOT消息
	virtual bool DelRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink) = 0;

	//分发MSG_ROOT消息
	virtual void DispatchMessage(IActor *pActor, IBuffer & ib) = 0;

};

#endif

