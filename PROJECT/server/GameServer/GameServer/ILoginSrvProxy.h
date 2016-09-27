

#ifndef __GAMESERVER_ILOGINSRV_PROXY_H__
#define __GAMESERVER_ILOGINSRV_PROXY_H__

struct IGameServer;

//登陆服代理
struct ILoginSrvProxy
{
	virtual void Release() = 0;

	//szLonginSrvIp :登陆服IP,
	//LoginSrvPort ：登陆服端口
	//ReportInterval ： 定时汇报间隔
	virtual bool Init(IGameServer * pGameServer,const char * szLonginSrvIp,int LoginSrvPort, const char * szGameSrvIp,int GameSrvPort,int ReportInterval)=0;

	virtual void Close() = 0;

	static ILoginSrvProxy * CreateLoginSrvProxy();

protected:
	//禁止调用delete,需要调用Release()来释放
	virtual ~ILoginSrvProxy(){}


};



#endif

