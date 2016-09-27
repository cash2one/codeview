
#include "GameServerProxy.h"
#include "ServerProtocol.h"
#include "LoginServer.h"
#include "IBasicService.h"


GameServerProxy::GameServerProxy(TServerID ServerID,ISocketSystem * pSocketSystem)
{
	m_ServerID = ServerID;
	m_pSocketSystem = pSocketSystem;
}

GameServerProxy::~GameServerProxy()
{
}


//数据到达,
void GameServerProxy::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	OBuffer4k ob(osb);

	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader PacketHeader;

	ib >> PacketHeader;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));

		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	if(PacketHeader.MsgCategory != enMsgCategory_GameServer)
	{
		TRACE("<error> %s : %d 接收到的数据消息分类有误，MsgCategory = %d len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			PacketHeader.MsgCategory,ob.Size(),m_pSocketSystem->GetRemote(socketid));

		m_pSocketSystem->ShutDown(socketid);

		return;
	}

	switch(PacketHeader.command)
	{
	case enServerCmd_SS_ReportState:
		{
			HandleReportStateReq(socketid,PacketHeader.command,ib,ob);
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 接收到的数据消息命令有误，command = %d len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			PacketHeader.command,ob.Size(),m_pSocketSystem->GetRemote(socketid));

		    m_pSocketSystem->ShutDown(socketid);
		}
		break;
	}

}

//连接关闭
void GameServerProxy::OnClose(TSockID sockid)
{
	LoginServer::GetInstance().OnGameServerClose(m_ServerID);
	delete this;
}


void GameServerProxy::HandleReportStateReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	SS_Report_State_Req StateReq;
	ib >> StateReq;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
		ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
	}

	LoginServer::GetInstance().HandleReportStateReq(StateReq.ServerID,StateReq.State);
}
