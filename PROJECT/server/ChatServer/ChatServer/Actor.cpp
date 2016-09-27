

#include "Actor.h"
#include "GameServerProxy.h"


Actor::Actor(GameServerProxy * pGameServerProxy)
{
	m_pGameServerProxy = pGameServerProxy;
	m_UserID = 0;  //用户ID	
	m_SynID  = 0; //帮派ID  
	m_szName[0] = 0;  //名称

}

Actor::~Actor()
{
}

bool Actor::Create(const CS_EnterChatSrv_Req & Req)
{
    m_UserID = Req.UserID;
    m_SynID  = Req.SynID;
	strncpy(m_szName,Req.szName,ARRAY_SIZE(m_szName));
    m_uidActor = Req.uidActor;
	return true;
}

void Actor::Release()
{
	delete this;
}


//数据到达,
void Actor::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
}

//连接关闭
void Actor::OnClose(TSockID sockid)
{
	m_Socketid = TSockID();

	m_pGameServerProxy->RemoveActor(m_uidActor);

	Release();
}

//设置套接字
void Actor::SetSocketID(TSockID sockid)
{
	if(m_Socketid.IsInvalid() == false)
	{
		m_pGameServerProxy->GetSocketSystem()->SetSocketSink(m_Socketid,0);
		m_pGameServerProxy->GetSocketSystem()->ShutDown(m_Socketid);
	}

	m_Socketid = sockid;
}

//得到帮派ID
TSynID Actor::GetSynID()
{
	return m_SynID;
}

//得到UID
UID	   Actor::GetUID()
{
	return m_uidActor;
}
