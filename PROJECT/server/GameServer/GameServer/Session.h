
#ifndef __GAMESERVER_SESSION_H__
#define __GAMESERVER_SESSION_H__

#include "ISession.h"
#include "ISocketSystem.h"
#include "SessionState.h"

struct IActor;
struct ISessionState;



class Session : public ISession
{
public:
    Session(TSockID socketid);

	virtual ~Session();

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//切换会话状态
	void	GotoState(enPlayerSessionState state, void * pContext );

	//设置角色对象
	void	SetActor(IActor * pActor) { m_pActor = pActor;}

	IActor * Actor(){return m_pActor;}

		//是否在线
	virtual bool IsOnLine(){return !m_SocketID.IsInvalid();}

		//设置通讯套接字
	virtual void SetSocket(TSockID socketid); 

public:

		//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);


	//关闭会话
	virtual	void KickUser( UINT8 KickType);

	//给玩家客户端发送数据
	virtual bool SendData(OStreamBuffer & osb) ;
	
	//切换到退出状态,
	virtual	void	Logout();    

	//返回角色的会话状态
	virtual enPlayerSessionState GetCurrentStateID();

	virtual	UINT32		GetSessionID();

	//装载玩家入内存
	virtual bool LoadActor(const UID & uidActor);

	void Close();

	//获得对方IP
    const char * GetRemoteIP();

private:
   TSockID  m_SocketID;

   UINT32   m_SessionID;

   IActor * m_pActor;

   //当前状态
   ISessionState * m_pCurState;

   EnterServerState m_EnterServerState;  //进入游戏状态
   GamePlayState    m_GamePlayState;     //玩游戏态
   LogoutState      m_LogoutState;       //退出游戏态
};









#endif
