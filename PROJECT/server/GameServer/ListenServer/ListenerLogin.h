#ifndef __LISTENSERVER_LISTENER_H__
#define __LISTENSERVER_LISTENER_H__

#include "ISocketSystem.h"
#include <boost/asio.hpp>
#include "IBasicService.h"
#include "ITimeAxis.h"
#include "FieldDef.h"
#include "LoginSrvProtocol.h"
#include <vector>
#include <map>

class ListenerGame;

struct SConfigParam
{
	FIELD_BEGIN();

	FIELD(TInt8Array<32>,   m_szLoginSrvIp);				///登陆服务器IP
	FIELD(UINT16,           m_LoginSrvPort);				///登陆服务器端口
	FIELD(INT16,            m_NetIoThreadNum);				///网络IO线程数
	FIELD(INT16,            m_KeepLiveTime);				///连接心跳超时
	FIELD(INT32,            m_MaxConnNum);					///最大连接数
	FIELD(TInt8Array<18>,   m_UserName);					///玩家账号
	FIELD(TInt8Array<18>,   m_UserPassword);				///玩家密码
	FIELD(TInt8Array<18>,   m_ClientVersion);				///客户端版本号
	FIELD(std::string,		m_MailAddress);					///客服邮件发件箱地址
	FIELD(std::string,		m_MailPassword);				///客服邮件发件箱密码
	FIELD(std::string,		m_MailServer);					///发件箱的服务器
	FIELD(std::string,		m_MailDesAddress);				///客服邮件收件箱地址
	FIELD(UINT32,			m_ListenTimeSpace);				///监听时间间隔
	FIELD(TInt8Array<MD5_LEN>,	m_szClientCnfgVersion);		///客户端配置文件MD5
	FIELD(TInt8Array<MD5_LEN>,	m_szClientResVersion);		///客户端资源文件MD5

	FIELD_END();
};

//配置的游戏服信息
struct GameServerInfo
{
	GameServerInfo()
	{
		MEM_ZERO(this);
	}

	TServerID			m_ServerID;
	char				m_szServerName[GAME_SERVER_NAME_LEN];    //服务器名称
	bool				m_bInServerList;						 //是否在服务器列表中
};

struct Mail_Data
{
	Mail_Data(){
		memset(this, 0, sizeof(*this));
	}
	char szTheme[DESCRIPT_LEN_50];
	char szBody[DESCRIPT_LEN_380];
};

struct GameServerCnfg
{
	FIELD_BEGIN();

	FIELD(TServerID,	m_ServerID);						//服务器ID
	FIELD(TInt8Array<THING_NAME_LEN>    ,m_szServerName);   //服务器名称

	FIELD_END();
};

class ListenerLogin : public ISocketSink, public ITimerSink
{
	enum enListenerTimer
	{
		enListenerTimer_Control = 0,
	};
public:
	ListenerLogin();

	~ListenerLogin();

	static ListenerLogin * GetInstance();

	bool Init(const char* szFileName);

	//对登录服和游戏服做检测
	void	CheckServer();

	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

	IBasicService  *		GetBasicServer();

	virtual ICryptService * GetCryptService();

	ISocketSystem  *		GetSocketSystem();

	const SConfigParam &	GetConfigParam();

	//通知网管服务器没开启
	void					SendMailNoticeNoOpen(TServerID ServerID, bool bNoOpen, bool bLogin);

	virtual void OnTimer(UINT32 timerID);

private:
	SConfigParam					m_ConfigParam;

	ISocketSystem  *				m_pSocketSystem;
	IBasicService  *				m_pBasicService;
	ITimeAxis      *				m_pTimeAxis;

	TSockID							m_SockID;

	boost::asio::signal_set *		m_pSignals_;

	std::vector<ListenerGame *>		m_vecListenGame;

	std::map<TServerID,GameServerInfo>	m_mapGameInfo;

	std::map<TServerID, ListenerGame *> m_mapListenerGame;

	TUserID							m_UserID;

	char							m_szTicket[TICKET_LEN];
};

#endif
