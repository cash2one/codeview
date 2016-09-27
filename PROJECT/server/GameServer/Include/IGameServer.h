

#ifndef __GAMESERVER_IGAMESERVER_H__
#define __GAMESERVER_IGAMESERVER_H__

#include <stdlib.h>

#include "UniqueIDGenerator.h"

#include "DSystem.h"
#include "DBProtocol.h"



struct ITimeAxis;
struct IEventServer;
struct IBasicService;
struct IGameWorld;
struct ISocketSystem;
struct IDBProxyClient;
struct IActor;
struct IConfigServer;
struct IMessageDispatch;
struct IGoodsServer;
struct IRelationServer;
struct ICombatServer;
struct ISyndicateMgr;
struct ICDTimeMgr;
struct IKeywordFilter;
struct ICryptService;
struct IThingServer;

//登陆服信息
struct SLoginServerInfo
{
	FIELD_BEGIN();

	FIELD(TInt8Array<20> ,   m_szLoginSrvIp);  //登陆服务器IP
	FIELD(UINT16 ,           m_LoginSrvPort); //登陆服务器端口
	FIELD(TInt8Array<IP_LEN> ,   m_szGameSrvIp);  //本服务器对外IP
	FIELD(UINT16 ,           m_GameSrvPort); //本服务器对外端口

	FIELD_END();
};




//配置参数
struct SConfigParam
{
	FIELD_BEGIN();
	FIELD(TServerID,         m_ServerID); //服务器ID
	FIELD(TGameWorldID,     m_GameWorldID); //游戏世界
	FIELD(TInt8Array<GAME_SERVER_NAME_LEN>,    m_szServerName);   //本服务器名称
	FIELD(TInt8Array<20>,    m_szIp);   //本服务器监听IP
	FIELD(UINT16 ,           m_Port);   //本服务器监听端口
	FIELD(INT16 ,            m_NetIoThreadNum);  //网络IO线程数
	FIELD(INT16,             m_KeepLiveTime);     //连接心跳超时
	FIELD(INT32,             m_MaxConnNum); //最大连接数
	FIELD(TInt8Array<20> ,   m_szDBIp);  //数据库前端服务器IP
	FIELD(UINT16 ,           m_DBPort); //数据库前端服务器端口	
	FIELD(bool,              m_bCommend);     //是否推荐
	FIELD(bool,              m_bNew);         //是否是新服
	FIELD(int,               m_ReportInterval); //向登陆服定时汇报
	FIELD(std::vector<SLoginServerInfo>   ,m_vectLoginServerInfo); //登陆服信息
	FIELD(SLoginServerInfo,  m_ChatServerInfo); //聊天服信息
	FIELD(bool,				 m_bOpenControlCmd); ///是否开启控制台命令
	FIELD(INT16              ,m_GamePlatform); //平台
	FIELD_END();

};

extern SConfigParam g_ConfigParam;


//服务器信息
struct SServer_Info
{
	std::string          m_strLastVersion;
	std::string          m_strMinVersion;
	std::string          m_strInstallPackageUrl;
	std::string          m_strerverName;
	std::string          m_strServiceTel;
	std::string          m_strServiceEmail;
	std::string          m_strServiceQQ;
	UINT32				 m_OpenServiceTime;	//第一次开服时间
	UINT8                m_IsNewServer; //是否是新服

	float                m_fMultipExp;      //多倍经验倍数
	UINT8                m_MinMultipExpLv; //享受多倍经验最低玩家等级
	UINT8                m_MaxMultipExpLv; //享受多倍经验最高玩家等级
	UINT32               m_MultipExpBeginTime; //多倍经验开始时间
	UINT32               m_MultipExpEndTime;   //多倍经验结束时间
};


struct IGameServer
{
		//取得时间轴
	virtual ITimeAxis* GetTimeAxis(void) = 0;


	//取得事件服务器
	virtual IEventServer* GetEventServer(void) = 0;

	//返回游戏世界对象
	virtual	IGameWorld	*	GetGameWorld()=0;

	virtual IBasicService * GetBasicService() = 0;

	virtual ISocketSystem  * GetSocketSystem() = 0;

	virtual IDBProxyClient * GetDBProxyClient() = 0;

	virtual IConfigServer * GetConfigServer() = 0;

	virtual IMessageDispatch * GetMessageDispatch()=0;

	virtual IGoodsServer  * GetGoodsServer()=0;

	virtual IRelationServer * GetRelationServer() = 0;

	virtual ICombatServer  * GetCombatServer() = 0;

	virtual ISyndicateMgr *   GetSyndicateMgr() = 0;

	virtual IKeywordFilter * GetKeywordFilter() = 0;

	virtual ICryptService * GetCryptService()=0; 

	//获取服务器ID
	virtual TServerID GetServerID() = 0;

	//获取服务器信息
	virtual const SServer_Info & GetServerInfo() = 0;

		//获取游戏世界ID
	virtual TGameWorldID GetGameWorldID() = 0;

	//获取服务器状态
	virtual int GetServerState() = 0;

	virtual bool    RequestDBByActor(IActor* pActor,enDBCmd ReqCmd, OStreamBuffer & ReqOb) = 0;

		//装载玩家入内存
	virtual bool LoadActor(UID uidActor) = 0;

	//获取冷却接口
	virtual ICDTimeMgr * GetCDTimeMgr() = 0;

	//发送给聊天服
	virtual bool SendToChatServer(OStreamBuffer & osb) = 0;

	//得到游戏全局配置信息
	virtual const SConfigParam & GetConfigParam() = 0;

	//封号
	virtual void	SealNo_Push(SDB_SealNo & DBSealNo) = 0;

	//是否被封号
	virtual bool	IsSealNo(TUserID userID) = 0;

	//设置客服信息
	virtual void	SetServiceInfo(std::string & ServiceTel, std::string & ServiceEmail, std::string & ServiceQQ) = 0;

	//设置版本信息
	virtual void	SetVersionInfo(std::string & LastVersion, std::string & MinVersion, std::string & InstallPackageUrl) = 0;

	//重新获取关键字
	virtual void	ReGetKeywordFilter() = 0;

	virtual IThingServer * GetThingServer() = 0;

	//设置多倍经验信息
	virtual void	SetMultipExpInfo(float MultipExp,UINT8 MinMultipExpLv,UINT8 MaxMultipExpLv,UINT32 MultipExpBeginTime,UINT32 MultipExpEndTime) = 0;
};


#endif
