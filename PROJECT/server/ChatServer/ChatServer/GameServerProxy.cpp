
#include "ChatServer.h"
#include "Actor.h"
#include "IBasicService.h"
//#include "ServerProtocol.h"





GameServerProxy::GameServerProxy(TServerID ServerID,ISocketSystem * pSocketSystem,const char * szServerName)
{
	m_ServerID = ServerID;
	m_pSocketSystem = pSocketSystem;
	m_strServerName = szServerName;
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

	if(PacketHeader.MsgCategory == enMsgCategory_Talk)
	{
		switch(PacketHeader.command)
		{
		case 0:
			{
				NULL;
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
	else
	{
		TRACE("<error> %s : %d 接收到的数据消息分类有误，MsgCategory = %d len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			PacketHeader.MsgCategory,ob.Size(),m_pSocketSystem->GetRemote(socketid));

		m_pSocketSystem->ShutDown(socketid);

		return;
	}	

}

//连接关闭
void GameServerProxy::OnClose(TSockID sockid)
{
	ChatServer::GetInstance().OnGameServerClose(m_ServerID);
	m_Socketid = TSockID();
}

//设置连接
void GameServerProxy::SetSockID(TSockID socketid)
{
	if(m_Socketid.IsInvalid() == false)
	{
		m_pSocketSystem->SetSocketSink(m_Socketid,0);
		m_pSocketSystem->ShutDown(m_Socketid);
	}

	m_Socketid = socketid;
}

//玩家登陆
void GameServerProxy::ActorEnter(TSockID socketid,const CS_EnterChatSrv_Req & Req)
{
	Actor * pActor = GetActor(Req.uidActor);

	if(pActor == 0)
	{
		pActor = new Actor(this);
		if(pActor->Create(Req)==false)
		{
			TRACE("<error> %s : %d line 创建玩家[%s]失败!",__FUNCTION__,__LINE__,Req.szName);

			delete pActor;

			return;
		}

		m_mapActor[Req.uidActor] = pActor;

	}

	pActor->SetSocketID(socketid);

}

//获得玩家
Actor * GameServerProxy::GetActor(UID uidActor)
{
	ACTOR_MAP::iterator it = m_mapActor.find(uidActor);

	if(it == m_mapActor.end())
	{
		return 0;
	}

	return it->second;
}

ISocketSystem * GameServerProxy::GetSocketSystem()
{
	return m_pSocketSystem;
}

//移除玩家
void GameServerProxy::RemoveActor(UID uidActor)
{
	m_mapActor.erase(uidActor);
}
