
#include "MessageDispatch.h"


MessageDispatch::MessageDispatch()
{
}

MessageDispatch::~MessageDispatch()
{
}

   //订阅MSG_ROOT消息
bool MessageDispatch::AddRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink)
{
	if(Category>=enMsgCategory_Max || pSink==0)
	{
		return false;
	}

	m_pMessageRootSink[Category].insert(m_pMessageRootSink[Category].begin(),pSink);

	return true;

}
	//取消订阅MSG_ROOT消息
 bool MessageDispatch::DelRootMessageSink(enMsgCategory Category, IMsgRootDispatchSink *pSink)
 {
	 RootMsgHandlerList::iterator it = std::find(m_pMessageRootSink[Category].begin(),m_pMessageRootSink[Category].end(),pSink);
	 if(it != m_pMessageRootSink[Category].end())
	 {
		 m_pMessageRootSink[Category].erase(it);
	 }

	 return false;
 }

	//分发MSG_ROOT消息
void MessageDispatch::DispatchMessage(IActor *pActor, IBuffer & ib)
{
	AppPacketHeader Header;
	ib >> Header;

	if(ib.Error())
	{
		return;
	}

	if(Header.MsgCategory>=enMsgCategory_Max)
	{
		return ;
	}

	RootMsgHandlerList & List = m_pMessageRootSink[Header.MsgCategory];

	 RootMsgHandlerList::iterator end = List.end();

	 for( RootMsgHandlerList::iterator it = List.begin(); it != end;++it)
	 {
		 (*it)->OnRecv(pActor,Header.command,ib);
	 }



}
