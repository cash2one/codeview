

#ifndef __GAMESERVER_LOGINSRV_PROXY_H__
#define __GAMESERVER_LOGINSRV_PROXY_H__

#include "ILoginSrvProxy.h"
#include "ISocketSystem.h"
#include "ITimeAxis.h"

#include <string>

class LoginSrvProxy : public ILoginSrvProxy 
	                 , public ISocketSink
					 , public ITimerSink
{
	enum
	{
		TIMERID_REPORT = 0,  //向登陆服定时汇报
	};
public:
   LoginSrvProxy();
   
   virtual ~LoginSrvProxy();

public:
	
	virtual void Release();

	//szLonginSrvIp :登陆服IP,
	//LoginSrvPort ：登陆服端口
	//ReportInterval ： 定时汇报间隔
	virtual bool Init(IGameServer * pGameServer,const char * szLonginSrvIp,int LoginSrvPort,
		    const char * szGameSrvIp,int GameSrvPort,int ReportInterval);

	virtual void Close();

public:
	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

public:
	void OnTimer(UINT32 timerID);

private:

	//连接登陆服
	bool ConnectLoginSrv();

	//定时汇报状态
	void ReportState();

private:
	std::string   m_strLoginSrvIp;	
	TSockID       m_SocketID;
	IGameServer * m_pGameServer;
	UINT16        m_LoginSrvPort;

	std::string   m_strGameSrvIp;
	int           m_GameSrvPort;
};












#endif
