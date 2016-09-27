
#ifndef __CHATSERVER_ACTOR_H__
#define __CHATSERVER_ACTOR_H__

#include "GameServerProxy.h"

class Actor : public ISocketSink
{
public:

	Actor(GameServerProxy * pGameServerProxy);

	virtual ~Actor();

	bool Create(const CS_EnterChatSrv_Req & Req);

	virtual void Release();

	//设置套接字
	void SetSocketID(TSockID sockid);

	//


public:
	
	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid); 

public:
	//得到帮派ID
	virtual TSynID GetSynID();

	//得到UID
	virtual UID	   GetUID();

private:
	GameServerProxy  * m_pGameServerProxy;
	UID       m_uidActor; //UID
	TUserID   m_UserID;  //用户ID	
	TSynID	  m_SynID  ; //帮派ID  
	char      m_szName[THING_NAME_LEN];  //名称
	TSockID   m_Socketid;  //套接字
};



#endif
