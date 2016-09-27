
#ifndef __GAMESERVER_MESSAGEDISPATCH_H__
#define __GAMESERVER_MESSAGEDISPATCH_H__

#include "IMessageDispatch.h"
#include <list>

class MessageDispatch : public IMessageDispatch
{
	typedef std::list<IMsgRootDispatchSink *>		RootMsgHandlerList;

public:
	MessageDispatch();
	virtual ~MessageDispatch();

public:
   //订阅MSG_ROOT消息
	virtual bool AddRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink);
	//取消订阅MSG_ROOT消息
	virtual bool DelRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink);

	//分发MSG_ROOT消息
	virtual void DispatchMessage(IActor *pActor, IBuffer & ib);

private:
   RootMsgHandlerList m_pMessageRootSink[enMsgCategory_Max];
};



#endif
