
#include "LoginSrvProxy.h"
#include "IBasicService.h"
#include "IGameServer.h"
#include "ServerProtocol.h"

#include "StringUtil.h"


ILoginSrvProxy * ILoginSrvProxy::CreateLoginSrvProxy()
{
	return new LoginSrvProxy();
}


LoginSrvProxy::LoginSrvProxy()
{	
	m_LoginSrvPort = 0;
	m_pGameServer = 0;
}

LoginSrvProxy::~LoginSrvProxy()
{
}



void LoginSrvProxy::Release()
{
	delete this;
}

//szLonginSrvIp :登陆服IP,
//LoginSrvPort ：登陆服端口
//ReportInterval ： 定时汇报间隔
bool LoginSrvProxy::Init(IGameServer * pGameServer,const char * szLonginSrvIp,int LoginSrvPort,
						 const char * szGameSrvIp,int GameSrvPort,int ReportInterval)
{
	if(0==pGameServer || 0==szLonginSrvIp || ReportInterval<1)
	{
		return false;
	}

	m_pGameServer = pGameServer;
	m_LoginSrvPort = LoginSrvPort;
	m_strLoginSrvIp = szLonginSrvIp;

	m_strGameSrvIp = szGameSrvIp;

	m_GameSrvPort = GameSrvPort;

	 m_pGameServer->GetTimeAxis()->SetTimer(TIMERID_REPORT,this,ReportInterval*1000,"LoginSrvProxy::Init TIMERID_REPORT");

	if(ConnectLoginSrv()==false)
	{
		return false;
	}

	return true;

}

	//连接登陆服
bool LoginSrvProxy::ConnectLoginSrv()
{
	m_SocketID = m_pGameServer->GetSocketSystem()->Connect(m_strLoginSrvIp.c_str(),m_LoginSrvPort,this,enPacketHeaderType_Len);

	if(m_SocketID.IsInvalid())
	{
		TRACE("<error> %s : %d 连接登陆服失败 ip=%s port = %d!",__FUNCTION__,__LINE__,m_strLoginSrvIp.c_str(),m_LoginSrvPort);
		return false;
	}

	OBuffer1k ob;
	

	SS_Login_LoginSrv_Req Req;

	Req.ServerID = g_ConfigParam.m_ServerID;
	Req.bCommend =  g_ConfigParam.m_bCommend;
	Req.bNew =  m_pGameServer->GetServerInfo().m_IsNewServer;
	strncpy(Req.ServerIp,m_strGameSrvIp.c_str(),MEM_SIZE(Req.ServerIp));
	Req.ServerPort =  m_GameSrvPort;
	strncpy(Req.szServerName, m_pGameServer->GetServerInfo().m_strerverName.c_str(),MEM_SIZE(Req.szServerName));
	Req.State = (enServerState)m_pGameServer->GetServerState();

	ob << ServerHeader(enServerCmd_SS_LoginLoginSrv,SIZE_OF(SS_Login_LoginSrv_Req())) << Req;

	
	return m_pGameServer->GetSocketSystem()->Send(m_SocketID,ob.TakeOsb());
}

void LoginSrvProxy::Close()
{
	if(!m_SocketID.IsInvalid())
	{
		 m_pGameServer->GetSocketSystem()->ShutDown(m_SocketID);
		 m_SocketID = INVALID_SOCKID;
	}
}


//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void LoginSrvProxy::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	TRACE("连接登陆服[%s:%d]成功！",m_strLoginSrvIp.c_str(),m_LoginSrvPort);
}

//数据到达,
void LoginSrvProxy::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
}

//连接关闭
void LoginSrvProxy::OnClose(TSockID sockid)
{
	 m_SocketID = INVALID_SOCKID;
}


void LoginSrvProxy::OnTimer(UINT32 timerID)
{
	if(timerID == TIMERID_REPORT)
	{
		ReportState();
	}
}

//定时汇报状态
void LoginSrvProxy::ReportState()
{
	if(m_SocketID.IsInvalid())
	{
		if(ConnectLoginSrv()==false)
		{
			return;
		}
	}

	OBuffer1k ob;

	SS_Report_State_Req Req;

	Req.ServerID = g_ConfigParam.m_ServerID;
	
	Req.State = (enServerState)m_pGameServer->GetServerState();

	ob << ServerHeader(enServerCmd_SS_ReportState,SIZE_OF(SS_Report_State_Req())) << Req;

	
	 m_pGameServer->GetSocketSystem()->Send(m_SocketID,ob.TakeOsb());
}
