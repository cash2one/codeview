
#ifndef __GAMESERVER_GAMESERVER_H__
#define __GAMESERVER_GAMESERVER_H__

#include "IBasicService.h"
#include "ITimeAxis.h"
#include <boost/asio.hpp>
#include "ISocketSystem.h"
#include "IGameServer.h"
#include "IDBProxyClient.h"
#include <hash_map>
#include "IConfigServer.h"
#include "MessageDispatch.h"
#include "IGoodsServer.h"
#include "IRelationServer.h"
#include "ICombatServer.h"
#include "ChatSrvProxy.h"
#include "IThirdPart.h"

struct ILoginSrvProxy;

class Session;

struct IThingServer;

struct ISyndicateMgr;

class GameServer : public IGameServer, public ISocketSink ,public IDBProxyClientSink,public ITimerSink,public IThirdPartSink
{
	typedef std::hash_map<UINT32,Session*> mapSession;

	enum { enTimerID_OffLine = 0, enTimerID_SaveToDB , enTimerID_Stop, enTimerID_StopTip,enTimerID_MultipExp,};

public:
   GameServer();
   
   ~GameServer();

   	bool Init(const char* szConfigFileName);
	bool Run();

   static GameServer* GetInstance();

   	//移除会话
	void RemoveSession(UINT64 Session);

	//离线
	void OffLineSession(UINT32 Session);


public:
   		//取得时间轴
	virtual ITimeAxis* GetTimeAxis(void);

	//取得事件服务器
	virtual IEventServer* GetEventServer(void);

	//返回游戏世界对象
	virtual	IGameWorld	*	GetGameWorld();

	virtual IBasicService * GetBasicService();

	virtual ISocketSystem  * GetSocketSystem();

	virtual IDBProxyClient * GetDBProxyClient();

	virtual IConfigServer * GetConfigServer();

	virtual IMessageDispatch * GetMessageDispatch();

	virtual IGoodsServer  * GetGoodsServer();

	virtual IRelationServer * GetRelationServer();

	virtual ICombatServer  * GetCombatServer();

	virtual ISyndicateMgr *   GetSyndicateMgr();

	IKeywordFilter * GetKeywordFilter();

	virtual ICryptService * GetCryptService();

		//获取服务器ID
	virtual TServerID GetServerID();

		//获取服务器信息
	virtual const SServer_Info & GetServerInfo() ;

		//获取游戏世界ID
	virtual TGameWorldID GetGameWorldID(); 

	//获取服务器状态
	virtual int GetServerState();

	//获取冷却接口
	virtual ICDTimeMgr * GetCDTimeMgr();

	//发送给聊天服
	virtual bool SendToChatServer(OStreamBuffer & osb);

	INT32 GetClientCnfgFileSize();

	INT32 GetClientResFileSize();

	//发送客户端配置文件
	void SendClientCnfgFile(Session * pSession);

	//发送客户端资源文件
	void SendClientResFile(Session * pSession);

	//得到游戏全局配置信息
	virtual const SConfigParam & GetConfigParam();

			//读取文件
    bool ReadFile(const std::string & strFileName,OBuffer16k & ob);

	virtual IThingServer * GetThingServer();

	//封号
	virtual void	SealNo_Push(SDB_SealNo & DBSealNo);

	//是否被封号
	virtual bool	IsSealNo(TUserID userID);

	//设置客服信息
	virtual void	SetServiceInfo(std::string & ServiceTel, std::string & ServiceEmail, std::string & ServiceQQ);

	//设置版本信息
	virtual void	SetVersionInfo(std::string & LastVersion, std::string & MinVersion, std::string & InstallPackageUrl);

	//重新获取关键字
	virtual void	ReGetKeywordFilter();

	//设置多倍经验信息
	virtual void	SetMultipExpInfo(float MultipExp,UINT8 MinMultipExpLv,UINT8 MaxMultipExpLv,UINT32 MultipExpBeginTime,UINT32 MultipExpEndTime);


public:
	//ISocketSink接口
		//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

public:
	//IDBProxyClientSink接口
		// 前置机回调
	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual bool    RequestDBByActor(IActor* pActor,enDBCmd ReqCmd, OStreamBuffer & ReqOb);

	//装载玩家入内存
	virtual bool LoadActor(UID uidActor);

public:
	virtual void OnTimer(UINT32 timerID);

private:
		void handle_stop();

		//装备关闭
		void handle_stop_prepare();

		//服务器启动了
       void OnServerStart();

	   //服务器停止了
       void OnServerStop();

	   //关闭各个模块
       void StopService();

		
		//获得关键词
		void HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

		//记录服务器在线人数
		void SaveOnlineInfo(int onlinenum);


		//服务器信息返回
void  HandleDBRetGetServerInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata);

//获取平台信息返回
void HandleDBRetGetPlatformInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata);

		//关服提示
		void StopServiceTip(const char * pszTipContent);

		void OnMultipExp();

private:
	bool m_bStop; //是否即将停机
	bool m_ServerStop; //服务器的各个模块是否已关闭

    mapSession m_mapSession;  //管理所有在线客户端会话

	 mapSession m_mapOffLineSession;  //管理所有离线客户端会话

	 std::vector<ILoginSrvProxy *> m_pLoginSrvProxy; //登陆服代理

	ISocketSystem  * m_pSocketSystem;
	IDBProxyClient * m_pDBProxyClient;
	IBasicService  * m_pBasicService;
    IGameWorld     * m_pGameWorld;
	ITimeAxis      * m_pTimeAxis;
	IEventServer   * m_pEventServer;
	IThingServer   * m_pThingServer;
	IConfigServer  * m_pConfigServer;
	IGoodsServer   * m_pGoodsServer;
	IRelationServer * m_pRelationServer;
	ICombatServer   * m_pCombatServer;
	ISyndicateMgr   * m_pSyndicateMgr;

	ChatSrvProxy * m_pChatSrvProxy;

	IThirdPart   * m_pThirdPartProxy;

	INT32       m_ClientCnfgFileSize; //客户端配置文件大小。

	INT32       m_ClientResFileSize;//资源文件大小

	OBuffer     m_CleintCnfgFile;

	OBuffer     m_CleintResFile; //资源文件



	boost::asio::signal_set * m_pSignals_;

	MessageDispatch  m_MessageDispatch;

	SServer_Info   m_ServerInfo;

	//封号的玩家
	typedef std::map<TUserID, SDB_SealNo> MAP_SEALNO;

	MAP_SEALNO		m_mapSealNo;

	int				m_leftMinute;	//关服时用,还剩多少分钟关服
};








#endif

