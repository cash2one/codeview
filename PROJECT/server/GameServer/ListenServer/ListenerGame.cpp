#include "GameSrvProtocol.h"
#include "ListenerGame.h"
#include "ListenerLogin.h"



ListenerGame::ListenerGame()
{
	m_ServerID = 0;
	m_ServerPort = 0;

	m_UserID = 0;

	MEM_ZERO(m_szTicket);

	m_bLastOk = true;
}

bool ListenerGame::Create(TServerID ServerID, const char *szServerName)
{
	m_ServerID = ServerID;
	strncpy(m_szServerName, szServerName, sizeof(m_szServerName));
	return true;
}

bool ListenerGame::Reset(const char *ServerIp, UINT16 ServerPort, TUserID userID, const char * szTicket)
{
	if ( !m_bLastOk )
	{
		TRACE("服务器%s(%d)连接不上", m_szServerName, m_ServerID);

		ListenerLogin::GetInstance()->SendMailNoticeNoOpen(m_ServerID, false, false);

		//断连接
		ISocketSystem  * pSocketSystem = ListenerLogin::GetInstance()->GetSocketSystem();

		if ( 0 == pSocketSystem )
		{
			TRACE("<error> %s : %d Line 获取网络系统失败！！", __FUNCTION__, __LINE__);
			return false;
		}

		pSocketSystem->ShutDown(m_SockID);
	}

	strncpy(m_ServerIp, ServerIp, sizeof(m_ServerIp));
	m_ServerPort = ServerPort;

	ISocketSystem  * pSocketSystem = ListenerLogin::GetInstance()->GetSocketSystem();

	if ( 0 == pSocketSystem )
	{
		TRACE("<error> %s : %d Line 获取网络系统失败！！", __FUNCTION__, __LINE__);
		return false;
	}

	m_UserID = userID;

	strncpy(m_szTicket, szTicket, sizeof(m_szTicket));

	m_SockID = pSocketSystem->Connect(ServerIp,ServerPort,this,enPacketHeaderType_Len, 60);

	if( m_SockID.IsInvalid()){
		TRACE("服务器%s(%d)连接不上", m_szServerName, m_ServerID);
		ListenerLogin::GetInstance()->SendMailNoticeNoOpen(m_ServerID, true, false);
		return false;
	}

	m_bLastOk = false;

	return true;
}

//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void ListenerGame::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
//	TRACE("连接服务器%s成功", m_szServerName);

	//进入游戏服
	const SConfigParam & Param = ListenerLogin::GetInstance()->GetConfigParam();

	CS_EnterGame_Req Req;

	strncpy(Req.ClientVersion, (const char *)Param.m_ClientVersion, sizeof(Req.ClientVersion));
	strncpy(Req.szClientCnfgVersion, (const char *)Param.m_szClientCnfgVersion, sizeof(Req.szClientCnfgVersion));
	strncpy(Req.szClientResVersion, (const char *)Param.m_szClientResVersion, sizeof(Req.szClientResVersion));
	strncpy(Req.szTicket, m_szTicket, sizeof(Req.szTicket));
	Req.UserID = m_UserID;

	OBuffer1k ob;
	ob << GameFrameHeader(enGameFrameCmd_CS_Enter, sizeof(Req)) << Req;
	ListenerLogin::GetInstance()->GetSocketSystem()->Send(socketid, ob.TakeOsb());
}

//数据到达,
void ListenerGame::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	OBuffer4k ob(osb);
	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader Header;
	ib >> Header;

	switch( Header.command)
	{
	case enGameFrameCmd_SC_Enter:
		{
			SC_EnterGame_Rsp Rsp;

			ib >> Rsp;

			if ( ib.Error() )
			{
				TRACE("<error> %s : %d Line 包长度错误！！", __FUNCTION__, __LINE__);
				break;
			}

			TRACE("<error> %s : %d Line 登录游戏服返回错误码：%d", __FUNCTION__, __LINE__, Rsp.Result);

			if ( Rsp.Result == enEnterGameRetCode_Unknow || Rsp.Result == enEnterGameRetCode_DBError )
			{
				m_bLastOk = false;
				ListenerLogin::GetInstance()->SendMailNoticeNoOpen(m_ServerID, false, false);
				return;
			}
		}
		break;
	}

	TRACE("进入游戏服成功！！服务器%s", m_szServerName);

	m_bLastOk = true;

	//断连接
	ISocketSystem  * pSocketSystem = ListenerLogin::GetInstance()->GetSocketSystem();

	if ( 0 == pSocketSystem )
	{
		TRACE("<error> %s : %d Line 获取网络系统失败！！", __FUNCTION__, __LINE__);
		return;
	}

	pSocketSystem->ShutDown(m_SockID);
}

//连接关闭
void ListenerGame::OnClose(TSockID sockid)
{

}
