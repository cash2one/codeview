#ifndef __LISTENSERVER_LISTENERGAME_H__
#define __LISTENSERVER_LISTENERGAME_H__

#include "ISocketSystem.h"
#include "DSystem.h"

class ListenerGame : public ISocketSink
{
public:
	ListenerGame();

	bool Create(TServerID ServerID, const char *szServerName);

	bool Reset(const char *ServerIp, UINT16 ServerPort, TUserID userID, const char * szTicket);

	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

private:
	TServerID     m_ServerID;                              //服务器ID
	char          m_szServerName[GAME_SERVER_NAME_LEN];    //服务器名称
	char          m_ServerIp[IP_LEN];                      //服务器IP
	UINT16        m_ServerPort;                            //服务器端口

	TSockID		  m_SockID;

	TUserID		  m_UserID;

	char		  m_szTicket[TICKET_LEN];

	bool		  m_bLastOk;		//最近一次是否登录成功
};

#endif
