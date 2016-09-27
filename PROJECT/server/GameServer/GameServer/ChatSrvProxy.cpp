
#include "ChatSrvProxy.h"
#include "IBasicService.h"
#include "IGameServer.h"
#include "ServerProtocol.h"
#include "TBuffer.h"
#include "StringUtil.h"


ChatSrvProxy::ChatSrvProxy()
{	
	m_LoginSrvPort = 0;
	m_pGameServer = 0;
}

ChatSrvProxy::~ChatSrvProxy()
{
}



void ChatSrvProxy::Release()
{
	delete this;
}

//szLonginSrvIp :登陆服IP,
//LoginSrvPort ：登陆服端口
//ReportInterval ： 定时汇报间隔
bool ChatSrvProxy::Init(IGameServer * pGameServer,const char * szLonginSrvIp,int LoginSrvPort)
{
	if(0==pGameServer || 0==szLonginSrvIp)
	{
		return false;
	}

	m_pGameServer = pGameServer;
	m_LoginSrvPort = LoginSrvPort;
	m_strLoginSrvIp = szLonginSrvIp;

	if(ConnectLoginSrv()==false)
	{
		return false;
	}

	return true;

}

	//连接登陆服
bool ChatSrvProxy::ConnectLoginSrv()
{
	m_SocketID = m_pGameServer->GetSocketSystem()->Connect(m_strLoginSrvIp.c_str(),m_LoginSrvPort,this,enPacketHeaderType_Len);

	if(m_SocketID.IsInvalid())
	{
		TRACE("<error> %s : %d 连接聊天服失败 ip=%s port = %d!",__FUNCTION__,__LINE__,m_strLoginSrvIp.c_str(),m_LoginSrvPort);
		return false;
	}

	OBuffer1k ob;
	

	SS_Login_ChatSrv_Req Req;

	Req.ServerID = g_ConfigParam.m_ServerID;
	strncpy(Req.szServerName, g_ConfigParam.m_szServerName,MEM_SIZE(Req.szServerName));

	ob << ServerHeader(enServerCmd_SS_LoginChatSrv,SIZE_OF(Req)) << Req;

	
	return m_pGameServer->GetSocketSystem()->Send(m_SocketID,ob.TakeOsb());
}

void ChatSrvProxy::Close()
{
	if(!m_SocketID.IsInvalid())
	{
		 m_pGameServer->GetSocketSystem()->ShutDown(m_SocketID);
		 m_SocketID = INVALID_SOCKID;
	}
}


//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void ChatSrvProxy::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	TRACE("连接聊天服[%s:%d]成功！",m_strLoginSrvIp.c_str(),m_LoginSrvPort);
}

//数据到达,
void ChatSrvProxy::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
}

//连接关闭
void ChatSrvProxy::OnClose(TSockID sockid)
{
	 m_SocketID = INVALID_SOCKID;
}

//发送数据
 bool  ChatSrvProxy::Send(OStreamBuffer & osb)
 {
	 if(m_SocketID.IsInvalid())
	 {
		 if(ConnectLoginSrv()==false)
		 {
			 return false;
		 }
	 }

	return m_pGameServer->GetSocketSystem()->Send(m_SocketID,osb);
 }

