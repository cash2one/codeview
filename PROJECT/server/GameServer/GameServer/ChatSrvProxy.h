

#ifndef __GAMESERVER_CHATSRV_PROXY_H__
#define __GAMESERVER_CHATSRV_PROXY_H__

#include "ISocketSystem.h"
#include "IGameServer.h"

#include <string>

class ChatSrvProxy :  public ISocketSink					
{	
public:
   ChatSrvProxy();
   
   virtual ~ChatSrvProxy();

public:
	
	virtual void Release();

	//szLonginSrvIp :登陆服IP,
	//LoginSrvPort ：登陆服端口
	//ReportInterval ： 定时汇报间隔
	virtual bool Init(IGameServer * pGameServer,const char * szLonginSrvIp,int LoginSrvPort);

	virtual void Close();

public:
	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

	//发送数据
	virtual bool Send(OStreamBuffer & osb);

private:

	//连接登陆服
	bool ConnectLoginSrv();

private:
	std::string   m_strLoginSrvIp;	
	TSockID       m_SocketID;
	IGameServer * m_pGameServer;
	UINT16        m_LoginSrvPort;
};












#endif
