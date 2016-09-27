
#ifndef __CHATSERVER_GAMESERVER_PROXY_H__
#define __CHATSERVER_GAMESERVER_PROXY_H__

#include "DSystem.h"
#include "UniqueIDGenerator.h"
#include "ChatSrvCmd.h"
#include <string>
#include <hash_map>

class Actor;

template<>
struct std::hash<UID>
{
	UINT32 operator()(const UID & uid) const
	{
		//取m_passTime的低10位与m_objectSN合成一个32位比较不集中的key
		return (UINT32)(((uid.m_passTime & 0x3FF) << 22) | uid.m_objectSN); 
	}
};


class GameServerProxy : public ISocketSink
{
public:
	GameServerProxy(TServerID ServerID,ISocketSystem * pSocketSystem,const char * szServerName);

	~GameServerProxy();

public:
	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

	//设置连接
	void SetSockID(TSockID socketid);

	//玩家登陆
	void ActorEnter(TSockID socketid,const CS_EnterChatSrv_Req & Req);

	//获得玩家
	Actor * GetActor(UID uidActor);

	//移除玩家
	void RemoveActor(UID uidActor);

	ISocketSystem * GetSocketSystem();

private:
	//所有游戏对象的Map
	typedef	std::hash_map<UID, Actor*,std::hash<UID>, std::equal_to<UID> >	ACTOR_MAP;

	ACTOR_MAP       m_mapActor; //玩家集合

	TServerID       m_ServerID;
	ISocketSystem * m_pSocketSystem;
	std::string     m_strServerName;
	TSockID         m_Socketid;
};




#endif
