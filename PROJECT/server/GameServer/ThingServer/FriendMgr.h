#ifndef __THINGSERVER_FRIENDMGR_H__
#define __THINGSERVER_FRIENDMGR_H__

#include "IMessageDispatch.h"
#include "IDBProxyClient.h"
#include "IEventServer.h"

class FriendMgr : public IMsgRootDispatchSink, public IDBProxyClientSink, public IEventListener
{
public:
	FriendMgr(void);
	~FriendMgr(void);

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void	OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

public:
	//IDBProxyClientSink接口
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual void OnEvent(XEventData & EventData);

private:
	//添加新好友
	void	AddFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//
	void	HandleAddFriend(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//删除好友
	void	DeleteFriend(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//
	void	HandleDeleteFriend(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

		
	//查看好友
	void	ViewFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//查看好友度改变事件
	void	ViewFriendEnvent(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//查看好友信息标签里的信息
	void	ViewFriendMsg(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//查看同城的在线玩家
	void	ViewOneCityOnlineUser(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//拜访好友
	void	VisitFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//查看其它玩家人物信息
	void	ViewUserInfo(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//离开查看其它玩家人物信息界面
	void	LeaveViewUserInfo(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//离开拜访好友界面
	void	LeaveVisitFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib);

	//删除好友信息
	void	DeleteFriendMsg(IActor * pActor,UINT8 nCmd, IBuffer & ib);

private:
	void	SendUserInfo(IActor * pRecvActor, IActor * pSendActor);

private:
	typedef std::map<UID, UID> MAP_VIEWUSERINFO;

	MAP_VIEWUSERINFO		m_mapViewUserInfo;

};

#endif
