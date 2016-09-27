
#include "GameServer.h"

#include "Session.h"
#include "IActor.h"
#include "ICDTime.h"
#include "DMsgSubAction.h"




Session::Session(TSockID socketid) : m_EnterServerState(this)
                                   , m_GamePlayState(this)
								   , m_LogoutState(this)
{
	m_SocketID = socketid;
	m_pCurState = &m_EnterServerState;

	static UINT32 s_SessionID = 0;
	if(++s_SessionID==0)
	{
		++s_SessionID;
	}
	m_SessionID = s_SessionID;

	m_pActor = 0;
}

Session::~Session()
{
	m_pActor = 0;
}

void Session::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
					  OStreamBuffer & ReqOsb,UINT64 userdata)
{
	m_pCurState->OnDBRet(userID,ReqCmd,nRetCode,RspOsb,ReqOsb,userdata);

}

//切换会话状态
void	Session::GotoState(enPlayerSessionState state, void * pContext )
{
	ISessionState * pNewState=NULL;

	switch(state)
	{
	case	enEnterSceneServerState:
		{
			pNewState = &m_EnterServerState;
		}
		break;
	case enLogoutState:
		{
			pNewState = &m_LogoutState;
		}
		break;
	case	enGamePlayState:
		{
			pNewState = &m_GamePlayState;
		}
		break;
	case enSwitchServerState:
		{
			pNewState =0;
			break;
		}
	default:
		{
			TRACE("wrong state, id = %d", state);

		}
	}

	if(pNewState != m_pCurState)
	{
		if(m_pCurState)
			m_pCurState->Leave(NULL);
		m_pCurState = pNewState;
		if(m_pCurState)
			m_pCurState->Enter(pContext);
	}
}

	//设置通讯套接字
 void Session::SetSocket(TSockID socketid)
 {
	 m_SocketID = socketid;
 }

//数据到达,
void Session::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{	 
	m_pCurState->OnNetDataRecv(osb);
}

//连接关闭
void Session::OnClose(TSockID sockid)
{
	m_SocketID = TSockID();	

	m_pCurState->OnCloseConnect();

	GameServer::GetInstance()->GetSocketSystem()->ShutDown(sockid);
}


//关闭会话
void Session::KickUser( UINT8 KickType)
{	
	IActor * pActor = this->Actor();
	if(pActor)
	{
		//发送事件
		SS_ActorLogout ActorLogout ;
		ActorLogout.m_uidActor =  pActor->GetUID();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogout);
		pActor->OnEvent(msgID,&ActorLogout,SIZE_OF(ActorLogout));

		pActor->SaveData();

		this->SetActor(0);
	}

	if(!m_SocketID.IsInvalid())
	{
		SC_KickOffLine KickOffLine;
	    KickOffLine.m_KickType = (enKickType)KickType;

	    OBuffer1k ob;

	    ob << GameFrameHeader(enGameFrameCmd_SC_Kick,SIZE_OF(KickOffLine)) << KickOffLine;
 
	    SendData(ob.TakeOsb());		

		 GameServer::GetInstance()->GetSocketSystem()->SetSocketSink(m_SocketID,0);

		GameServer::GetInstance()->GetSocketSystem()->ShutDown(m_SocketID);
	}

	Close();
}

//给玩家客户端发送数据
bool Session::SendData(OStreamBuffer & osb)
{
	return GameServer::GetInstance()->GetSocketSystem()->Send(m_SocketID,osb);
}

//切换到退出状态,
void	Session::Logout()
{
	
	if(m_pActor)
	{
		//最后存一次盘
		UID uid = m_pActor->GetUID();

	    m_pActor->SaveData();

			//投票
		SS_UnloadActorContext UnloadActorCnt;

		UnloadActorCnt.m_uidActor = m_pActor->GetUID();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

		if(m_pActor->FireVote(msgID,&UnloadActorCnt,SIZE_OF(UnloadActorCnt))==false)
		{		  
		   return ;
		}

			     
	   Close();
	}
}  

void Session::Close()
{	
	if(m_pActor)
	{		 //再删除主角
	   GameServer::GetInstance()->GetGameWorld()->DestroyThing(m_pActor->GetUID());	
	}

	GameServer::GetInstance()->RemoveSession(GetSessionID());

	delete this;
}

//返回角色的会话状态
enPlayerSessionState Session::GetCurrentStateID()
{
	return m_pCurState->GetStateID();
}

UINT32	Session::GetSessionID()
{
	return m_SessionID;
}

bool	Session::LoadActor(const UID & uidActor)
{
	m_pCurState->LoadActor(uidActor);
	return true;
}

//获得对方IP
const char * Session::GetRemoteIP()
{
    UINT32 Ip = GameServer::GetInstance()->GetSocketSystem()->GetRemote(m_SocketID);

	static char szIp[20]={0};  //必须是静态的
	const unsigned char * ptr = (const unsigned char*)&Ip;
	sprintf_s(szIp,sizeof(szIp),"%d.%d.%d.%d",ptr[3],ptr[2],ptr[1],ptr[0]);

	return szIp;
}
