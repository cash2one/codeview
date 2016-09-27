
#include "ITimeAxis.h"
#include "boost/bind.hpp"
#include "IEventServer.h"
#include "GameServer.h"
#include "FileConfig.h"
#include "ProtocolHeader.h"
#include "ILoginSrvProxy.h"
#include "IThingServer.h"
#include "IGameWorld.h"
#include "IActor.h"
#include "Session.h"
#include "ISyndicateMgr.h"
#include "stdio.h"
#include "DMsgSubAction.h"
#include "IKeywordFilter.h"
#include "ICrashDump.h"
#include "CrashReport.h"
#include<stdio.h>
#include "XDateTime.h"
#include "IThirdPart.h"
#include "IActivityPart.h"


SConfigParam g_ConfigParam;


GameServer* GameServer::GetInstance()
{
	static GameServer __Instance;

	return &__Instance;
}

GameServer::GameServer()
{
	m_pSocketSystem = 0;
	m_pDBProxyClient = 0;
	m_pBasicService = 0;
	m_pGameWorld = 0;
	m_pTimeAxis = 0;
	m_pEventServer = 0;
	m_pSignals_ = 0;
	m_pThingServer = 0;
	m_pConfigServer = 0;
	m_pGoodsServer = 0;
	m_pRelationServer = 0;
	m_pCombatServer = 0;
	m_pChatSrvProxy = 0;
	m_bStop = false;
	m_ServerStop = false;
	m_ClientCnfgFileSize = 0;

	m_ClientResFileSize = 0;

	m_pThirdPartProxy = 0;

	m_leftMinute = 3;
}

GameServer::~GameServer()
{
	
	if(m_pSignals_)
	{
		delete m_pSignals_;
		m_pSignals_ = 0;
	}

	if(m_pSocketSystem)
	{
		m_pSocketSystem->Stop();		
	}

	if(this->m_pTimeAxis)
	{
		m_pTimeAxis->Stop();
	}

	if(m_pSocketSystem)
	{		
		m_pSocketSystem->Release();
	}

	if(m_pBasicService)
	{
		m_pBasicService->Release();
	}

	ICrashDump * pCrashDump = ::CreateCrashDump();

	if(pCrashDump != 0)
	{
		pCrashDump->DetachCrashDumpSink(&g_CrashReport);
	}
}


bool GameServer::Init(const char* szFileName)
{
	ICrashDump * pCrashDump = ::CreateCrashDump();

	if(pCrashDump != 0)
	{
		pCrashDump->AttachCrashDumpSink(&g_CrashReport);

		pCrashDump->CrtSetDbgFlag();

		//pCrashDump->CrtSetBreakAlloc(65627);	
	}


	if(szFileName==NULL)
	{
		return false;
	}
	m_pBasicService = ::GetBasicService();

	if(m_pBasicService==NULL || m_pBasicService->Create()==false)
	{
		return false;
	}

	

	ILogTrace * pLogTrace = m_pBasicService->GetLogTrace();

	if(pLogTrace->Create("./GameServer.log")==false)
	{
		return false;
	}

	FileConfig File;
	if(!File.Open(szFileName,"GameServer"))
	{
		TRACE("<error> %s : %d 打开配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	if(File.Read(g_ConfigParam)==false)
	{
		TRACE("<error> %s : %d 读取配置参数失败!",__FUNCTION__,__LINE__);
		return false;
	}

	m_pConfigServer = CreateConfigServer();
	if(m_pConfigServer==0 || m_pConfigServer->Create()==false)
	{
		TRACE("<error> %s : %d创建配置服务组件失败!",__FUNCTION__,__LINE__);
		return false;
	}

	//创建定时器
	m_pTimeAxis= m_pBasicService->GetTimeAxis();
	if(m_pTimeAxis->Create(m_pBasicService->GetIoService())==false)
	{
		TRACE("<error> %s : %d创建时间轴库失败!",__FUNCTION__,__LINE__);
		return false;
	}



	m_pCombatServer =  CreateCombatServer(this);

	if(m_pCombatServer==0)
	{
		TRACE("<error> %s : %d创建CombatServer库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	//创建通讯库
	m_pSocketSystem = CreateSocketSystem();
	if(m_pSocketSystem==NULL)
	{
		TRACE("<error> %s : %d创建通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	if(m_pSocketSystem->Start(m_pBasicService->GetIoService(),g_ConfigParam.m_MaxConnNum,g_ConfigParam.m_NetIoThreadNum,g_ConfigParam.m_KeepLiveTime)==false)
	{
		TRACE("<error> %s : %d 初始化通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	//创建数据库代理客户端
	m_pDBProxyClient = CreateDBProxyClient();

	if(m_pDBProxyClient==NULL)
	{
		TRACE("<error> %s : %d 创建数据库代理组件失败!",__FUNCTION__,__LINE__);

		return false;
	}

	if(m_pDBProxyClient->Init(m_pSocketSystem,(char*)g_ConfigParam.m_szDBIp,g_ConfigParam.m_DBPort)==false)
	{
		TRACE("<error> %s : %d 初始化数据库代理组件失败!",__FUNCTION__,__LINE__);

		return false;
	}

	if(m_pSocketSystem->Listen(g_ConfigParam.m_szIp,g_ConfigParam.m_Port,this,enPacketHeaderType_Len)==false)
	{
		TRACE("<error> %s : %d 通讯库监听失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)g_ConfigParam.m_szIp,g_ConfigParam.m_Port);
		return false;
	}

	m_pEventServer = ::GetEventServer();

	if(m_pEventServer->Create(g_SourceType,MEM_SIZE(g_SourceType))==false)
	{
		TRACE("<error> %s : %d 创建事件服务器失败!",__FUNCTION__,__LINE__);

		return false;
	}

	//连接聊天服
	m_pChatSrvProxy = new ChatSrvProxy();

	if(m_pChatSrvProxy->Init(this,g_ConfigParam.m_ChatServerInfo.m_szLoginSrvIp,g_ConfigParam.m_ChatServerInfo.m_LoginSrvPort)==false)
	{
		TRACE("<error> %s : %d line 连接聊天服[%s : %d]失败!",__FUNCTION__,__LINE__,(char*)g_ConfigParam.m_ChatServerInfo.m_szLoginSrvIp,g_ConfigParam.m_ChatServerInfo.m_LoginSrvPort);
	}

	m_pRelationServer =  CreateRelationServer(this);

	if(m_pRelationServer==0)
	{
		TRACE("<error> %s : %d创建RelationServer库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	m_pThingServer = CreateThingServer(this);

	if(m_pThingServer==0)
	{
		TRACE("<error> %s : %d创建ThingServer库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	m_pGameWorld = m_pThingServer->GetGameWorld();

	m_pGoodsServer = CreateGoodsServer(this);

	if(m_pGoodsServer==0)
	{
		TRACE("<error> %s : %d创建GoodsgServer库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	
	m_pSignals_ = new boost::asio::signal_set(*m_pBasicService->GetIoService());

	m_pSignals_->add(SIGINT);

	m_pSignals_->add(SIGTERM);

#if defined(SIGQUIT)
	m_pSignals_->add(SIGQUIT);
#endif // defined(SIGQUIT)

	m_pSignals_->async_wait(boost::bind(&GameServer::handle_stop_prepare, this));

	//启动离线玩家管理

	///角色数据定时存盘间隔(单位:秒)
	INT32 SaveToDBInterval = this->GetConfigServer()->GetGameConfigParam().m_SaveToDBInterval; 

	if(SaveToDBInterval>0)
	{
		m_pTimeAxis->SetTimer(enTimerID_SaveToDB,this,SaveToDBInterval*1000/4,"GameServer::Init[enTimerID_SaveToDB]");
	}

	INT32 OfflineInterval = this->GetConfigServer()->GetGameConfigParam().m_OfflineInterval; 

	if(OfflineInterval>0)
	{
		m_pTimeAxis->SetTimer(enTimerID_OffLine,this,OfflineInterval*1000,"GameServer::Init[enTimerID_OffLine]");
	}

	//客户端配置文件，读取内存来
	m_ClientCnfgFileSize = 0;

	const std::vector<std::string> & vectFile =  this->GetConfigServer()->GetClientCnfgFile();

	for(int i=0; i<vectFile.size();++i)
	{
		OBuffer16k ob;
		ReadFile(vectFile[i],ob);

		SC_UpdateCnfg UpdateCnfg;
		UpdateCnfg.m_bZip = false;
		UpdateCnfg.m_DataLen = ob.Size();

		INT32 PacketLen = SIZE_OF(UpdateCnfg)+vectFile[i].length()+1+UpdateCnfg.m_DataLen;

		m_CleintCnfgFile << GameFrameHeader(enGameFrameCmd_SC_UpdateCnfg,PacketLen) << UpdateCnfg;

		m_CleintCnfgFile.Push(vectFile[i].c_str(),vectFile[i].length()+1);

		m_CleintCnfgFile << ob;

		m_ClientCnfgFileSize += UpdateCnfg.m_DataLen;
	}

	
	//客户端资源文件，读取内存来
	m_ClientResFileSize = 0;

	const std::vector<std::string> & vectResFile =  this->GetConfigServer()->GetClientResFile();

	for(int i=0; i<vectResFile.size();++i)
	{
		const std::string & strResFile = vectResFile[i];
		OBuffer16k ob;
		ReadFile(strResFile,ob);

		SC_UpdateCnfg UpdateCnfg;
		UpdateCnfg.m_bZip = false;
		UpdateCnfg.m_DataLen = ob.Size();

		INT32 PacketLen = SIZE_OF(UpdateCnfg)+strResFile.length()+1+UpdateCnfg.m_DataLen;

		m_CleintResFile << GameFrameHeader(enGameFrameCmd_SC_UpdateCnfg,PacketLen) << UpdateCnfg;

		m_CleintResFile.Push(strResFile.c_str(),strResFile.length()+1);

		m_CleintResFile << ob;

		m_ClientResFileSize += UpdateCnfg.m_DataLen;
	}


	OnServerStart();

	return true;
}

//读取文件
bool GameServer::ReadFile(const std::string & strFileName,OBuffer16k & ob)
{
	FILE * pFile = ::fopen(strFileName.c_str(),"rb");
	if(pFile==0)
	{
		return false;
	}

	char szBuffer[1024]={0};

	INT32 len = 0;

	while(0!=(len=fread(szBuffer,1,MEM_SIZE(szBuffer),pFile)))
	{
			ob.Push(szBuffer,len);
	}

	::fclose(pFile);

	return true;
}

IThingServer * GameServer::GetThingServer()
{
	return m_pThingServer;
}

//封号
void	GameServer::SealNo_Push(SDB_SealNo & DBSealNo)
{
	m_mapSealNo[DBSealNo.m_UserID] = DBSealNo;
}

//是否被封号
bool	GameServer::IsSealNo(TUserID userID)
{
	MAP_SEALNO::iterator iter = m_mapSealNo.find(userID);
	if( iter == m_mapSealNo.end()){
		return false;
	}

	SDB_SealNo & DBSealNo = iter->second;

	UINT32 nCurTime = CURRENT_TIME();

	if( (0 == DBSealNo.m_EndTime || nCurTime < DBSealNo.m_EndTime) && nCurTime >= DBSealNo.m_BeginTime && (DBSealNo.m_RemoveTime == 0 || nCurTime < DBSealNo.m_RemoveTime))
	{
		return true;
	}

	if( (DBSealNo.m_RemoveTime != 0 && nCurTime >= DBSealNo.m_RemoveTime) || nCurTime >  DBSealNo.m_EndTime){
		//封号时间已到，或者已被解封号
		m_mapSealNo.erase(iter);
	}

	return false;
}

//设置客服信息
void	GameServer::SetServiceInfo(std::string & ServiceTel, std::string & ServiceEmail, std::string & ServiceQQ)
{
	m_ServerInfo.m_strServiceTel = ServiceTel;
	m_ServerInfo.m_strServiceEmail = ServiceEmail;
	m_ServerInfo.m_strServiceQQ	 = ServiceQQ;
}

//设置版本信息
void	GameServer::SetVersionInfo(std::string & LastVersion, std::string & MinVersion, std::string & InstallPackageUrl)
{
	m_ServerInfo.m_strLastVersion = LastVersion;
	m_ServerInfo.m_strMinVersion  = MinVersion;
	m_ServerInfo.m_strInstallPackageUrl = InstallPackageUrl;
}

//重新获取关键字
void	GameServer::ReGetKeywordFilter()
{
	IKeywordFilter * pKeyword = this->GetKeywordFilter();
	if( 0 == pKeyword){
		return;
	}

	pKeyword->ClearKeyword();

	//得闲获取关键词
	DB_Get_Keyword_Info_Req Req;

	OBuffer1k ob;
	ob << Req;
	m_pDBProxyClient->Request(0,enDBCmd_GetKeywordInfo,ob.TakeOsb(),this,0);
}

//设置多倍经验信息
void	GameServer::SetMultipExpInfo(float MultipExp,UINT8 MinMultipExpLv,UINT8 MaxMultipExpLv,UINT32 MultipExpBeginTime,UINT32 MultipExpEndTime)
{
	UINT32 CurTime = CURRENT_TIME();

	if ( CurTime < m_ServerInfo.m_MultipExpBeginTime )
	{
		//关闭旧定时器
		this->GetTimeAxis()->KillTimer(enTimerID_MultipExp,this);	
	}

	m_ServerInfo.m_fMultipExp = MultipExp;
	m_ServerInfo.m_MinMultipExpLv = MinMultipExpLv;
	m_ServerInfo.m_MaxMultipExpLv = MaxMultipExpLv;
	m_ServerInfo.m_MultipExpBeginTime = MultipExpBeginTime;
	m_ServerInfo.m_MultipExpEndTime = MultipExpEndTime;

	if((m_ServerInfo.m_fMultipExp>1.000001 || m_ServerInfo.m_fMultipExp<1.000001) && CurTime < m_ServerInfo.m_MultipExpBeginTime)
	{
		//启动定时器
		this->GetTimeAxis()->SetTimer(enTimerID_MultipExp,this,( m_ServerInfo.m_MultipExpBeginTime-CurTime)*1000,"GameServer::HandleDBRetGetServerInfo[enTimerID_MultipExp]");
	}
	else
	{
		this->OnMultipExp();
	}
}



//服务器启动了
void GameServer::OnServerStart()
{
	//获取关键词
	DB_Get_Keyword_Info_Req Req;

	OBuffer1k ob;

	ob << Req;

	if(m_pDBProxyClient->Request(0,enDBCmd_GetKeywordInfo,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}

	//服务启动
	DB_GameServerStart_Req StartReq;

	StartReq.m_ServerID = g_ConfigParam.m_ServerID;

	ob.Reset();

	ob << StartReq;

	if(m_pDBProxyClient->Request(0,enDBCmd_ServerStart,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}

	//获取第三方接口信息
	if(g_ConfigParam.m_GamePlatform == enGamePlatformType_DangLe)
	{
		DB_GetPlatformInfo_Req Req;
		Req.m_PlatformID = g_ConfigParam.m_GamePlatform;

		ob.Reset();

	   ob << Req;

	   if(m_pDBProxyClient->Request(0,enDBCmd_Get_PlatformInfo,ob.TakeOsb(),this,0)==false)
	   {
		 TRACE("<error> %s :%d line 发起DB请求失败!");
	   }
	}

	//服务器开启时,有要对数据库做的事写在这里
	SDB_OpenServer_Init_Req InitDB;

	ob.Reset();
   ob << InitDB;

   if(m_pDBProxyClient->Request(0,enDBCmd_OpenServer_Init,ob.TakeOsb(),0,0)==false)
   {
	 TRACE("<error> %s :%d line 发起DB请求失败!");
   }
}

//服务器停止了
void GameServer::OnServerStop()
{	
	//把所有玩家踢下线
	mapSession TempSession = m_mapSession;

	if(!TempSession.empty())
	{
		for(mapSession::iterator it = TempSession.begin(); it != TempSession.end(); ++it)
	   {
		Session* pSession = it->second;
		if(pSession)
		{
			pSession->KickUser(enKickType_Stop);
		}
	  }
	}
	else if(m_ServerStop==false)
	{
		OnTimer(enTimerID_OffLine); //释放断线玩家

		printf("正在关闭服务器各模块...");

		//关闭服务器各个模块
        StopService();
		m_ServerStop = true;
	}
	else 
	{
		printf("正在退出服务器....");
		this->GetTimeAxis()->KillAllTimer(this);

        handle_stop();
	}	
}


bool GameServer::Run()
{
	return m_pBasicService->GetIoService()->run();
}

//装备关闭
void GameServer::handle_stop_prepare()
{
	m_bStop = true;

	DB_GameServerStop_Req Req;

	Req.m_ServerID = g_ConfigParam.m_ServerID;

	OBuffer1k ob;

	ob << Req;

	if(m_pDBProxyClient->Request(0,enDBCmd_ServerStop,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}

	for(int i=0; i<m_pLoginSrvProxy.size(); ++i)
	{
		m_pLoginSrvProxy[i]->Close();
		m_pLoginSrvProxy[i]->Release();
	}

	m_pLoginSrvProxy.clear();

	//关服提示
	// fly add	20121106
	this->StopServiceTip(this->GetGameWorld()->GetLanguageStr(10077));
	//this->StopServiceTip("尊敬的玩家，由于服务器需要停机维护，将在三分钟后关闭，请您及时下线,以避免数据丢失！");	

	TRACE("服务器将在三分钟后关闭..........");

	this->GetTimeAxis()->SetTimer(enTimerID_StopTip,this,60*1000,"GameServer::handle_stop_prepare");

	this->GetTimeAxis()->SetTimer(enTimerID_Stop,this,180*1000,"GameServer::handle_stop_prepare");
	
}

//关闭各个模块
void GameServer::StopService()
{
	if(m_pRelationServer)
	{
		m_pRelationServer->Close();		
	}

	if(m_pThingServer)
	{
		m_pThingServer->Close();		
	}	
}

void GameServer::handle_stop()
{
	
	if(m_pCombatServer)
	{
		m_pCombatServer->Release();
	}

	
	if(m_pGoodsServer)
	{
		m_pGoodsServer->Release();
	}

	 if(m_pChatSrvProxy)
	{
		m_pChatSrvProxy->Release();
	}


	if(m_pThingServer)
	{	
		m_pThingServer->Release();
	}

	if(m_pRelationServer)
	{		
		m_pRelationServer->Release();
	}
	
	if(m_pConfigServer)
	{
		m_pConfigServer->Release();
	}	
   
	if(m_pDBProxyClient)
	{
		m_pDBProxyClient->Close();
		m_pDBProxyClient->Release();
	}

	m_pBasicService->GetIoService()->stop();	
}

//移除会话
void GameServer::RemoveSession(UINT64 Session)
{
	mapSession::iterator it = m_mapSession.find(Session);
	if(it != m_mapSession.end())
	{
		m_mapSession.erase(it);
	}
	else 
	{
		m_mapOffLineSession.erase(Session);
	}
}

//离线
void GameServer::OffLineSession(UINT32 nSession)
{
	mapSession::iterator it = m_mapSession.find(nSession);
	if(it != m_mapSession.end())
	{
		Session * pSession = (*it).second;
		m_mapSession.erase(it);

		m_mapOffLineSession[nSession] = pSession;	   
	}	

}

//取得时间轴
ITimeAxis* GameServer::GetTimeAxis(void)
{
	return m_pTimeAxis;
}

//取得事件服务器
IEventServer* GameServer::GetEventServer(void)
{
	return m_pEventServer;
}

//返回游戏世界对象
IGameWorld	*	GameServer::GetGameWorld()
{
	return m_pGameWorld;
}

IBasicService * GameServer::GetBasicService()
{
	return m_pBasicService;
}

ICryptService * GameServer::GetCryptService()
{
	if(m_pBasicService)
	{
		return m_pBasicService->GetCryptService();
	}

	return 0;
}

ISocketSystem  * GameServer::GetSocketSystem()
{
	return m_pSocketSystem;
}

IDBProxyClient * GameServer::GetDBProxyClient()
{
	return m_pDBProxyClient;
}

IConfigServer * GameServer::GetConfigServer()
{
	return m_pConfigServer;
}

IMessageDispatch *  GameServer::GetMessageDispatch()
{
	return &m_MessageDispatch;
}

IGoodsServer  * GameServer::GetGoodsServer()
{
	return m_pGoodsServer;
}

IRelationServer * GameServer::GetRelationServer()
{
	return m_pRelationServer;
}

ICombatServer  * GameServer::GetCombatServer()
{
	return m_pCombatServer;
}

ISyndicateMgr *   GameServer::GetSyndicateMgr()
{
	return m_pRelationServer->GetSyndicateMgr();
}

//获取服务器ID
TServerID GameServer::GetServerID()
{
	return g_ConfigParam.m_ServerID;
}

	//获取服务器信息
 const SServer_Info & GameServer::GetServerInfo() 
 {
	 return this->m_ServerInfo;
 }

//获取游戏世界ID
TGameWorldID GameServer::GetGameWorldID()
{
	return g_ConfigParam.m_GameWorldID;
}

//获取服务器状态
int GameServer::GetServerState()
{
	if (0 == m_pConfigServer){
		return enServerState_Fine;
	}

	return m_pConfigServer->GetServerStatus(m_mapSession.size());
}

INT32 GameServer::GetClientCnfgFileSize()
{
	return this->m_ClientCnfgFileSize;
}

INT32 GameServer::GetClientResFileSize()
{
	return this->m_ClientResFileSize;
}

//发送客户端配置文件
void GameServer::SendClientCnfgFile(Session * pSession)
{
	INT32 len = m_CleintCnfgFile.Size();

	if(len>0)
	{
		OBuffer ob;
		ob.Push(m_CleintCnfgFile.Buffer(),len);
		pSession->SendData(ob.TakeOsb());
	}
}

	//发送客户端资源文件
void GameServer::SendClientResFile(Session * pSession)
{
	INT32 len = m_CleintResFile.Size();

	if(len>0)
	{
		OBuffer ob;
		ob.Push(m_CleintResFile.Buffer(),len);
		pSession->SendData(ob.TakeOsb());
	}
}


//得到游戏全局配置信息
const SConfigParam & GameServer::GetConfigParam()
{
	return g_ConfigParam;
}

//获取冷却接口
ICDTimeMgr * GameServer::GetCDTimeMgr()
{
	IGoodsServer * pGoodsServer = this->GetGoodsServer();
	if( 0 == pGoodsServer){
		return 0;
	}

	return pGoodsServer->GetCDTimeMgr();
}

IKeywordFilter *  GameServer::GetKeywordFilter()
{
	return m_pBasicService->GetKeywordFilter();
}

//ISocketSink接口
//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void GameServer::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	if(m_bStop)
	{
		m_pSocketSystem->ShutDown(socketid);
		return;	
	}

	Session * pSession = new Session(socketid);

	if(m_pSocketSystem->SetSocketSink(socketid,pSession)==false)
	{
		delete pSession;
		return;
	}

	m_mapSession[pSession->GetSessionID()] = pSession;

	return ;

}

//数据到达,
void GameServer::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{

}

//连接关闭
void GameServer::OnClose(TSockID sockid)
{
}


//IDBProxyClientSink接口
// 前置机回调
// nRetCode: 取值于 enDBRetCode
void GameServer::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
						 OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(enDBCmd_GetKeywordInfo == ReqCmd)
	{
		OBuffer4k obReq(ReqOsb);
		OBuffer4k obRsp(RspOsb);

		HandleDBRetGetKeywordInfo(userID,ReqCmd,nRetCode,obRsp,obReq,userdata);
	}
	else if(enDBCmd_ServerStart == ReqCmd)
	{
		OBuffer4k obReq(ReqOsb);
		OBuffer4k obRsp(RspOsb);
		HandleDBRetGetServerInfo(userID,ReqCmd,nRetCode,obRsp,obReq,userdata);
	}
	else if(enDBCmd_Get_PlatformInfo == ReqCmd)
	{
		OBuffer4k obReq(ReqOsb);
		OBuffer4k obRsp(RspOsb);
		HandleDBRetGetPlatformInfo(userID,ReqCmd,nRetCode,obRsp,obReq,userdata);
	}
	else
	{
		mapSession::iterator it = m_mapSession.find(userdata);

		if(it == m_mapSession.end())
		{
			it = m_mapOffLineSession.find(userdata);
			if( it == m_mapOffLineSession.end()){
				return;
			}
		}

		Session * pSession = (*it).second;

		if(pSession)
		{
			pSession->OnDBRet(userID,ReqCmd,nRetCode,RspOsb,ReqOsb,userdata);
		}
	}
}

//获得关键词
void  GameServer::HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("%s 获得关键词库失败 Cmd=%d",__FUNCTION__,ReqCmd);

		return ;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;

	RspIb >> RspHeader;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	INT32 nCout = RspIb.Remain()/SIZE_OF(DB_GetKeyword_Info_Rsp());

	for(int i=0; i<nCout; i++)
	{
		DB_GetKeyword_Info_Rsp Rsp;

		RspIb >> Rsp;

		const char * ptr = (char*)Rsp.m_Keyword;

		GetKeywordFilter()->AddKeyword(ptr);
	}
}

//记录服务器在线人数
void GameServer::SaveOnlineInfo(int onlinenum)
{
	SDB_Save_OnlineInfo_Req DBReq;

	DBReq.m_ServerID = this->GetServerID();
	DBReq.m_OnlineNum = onlinenum;

	OBuffer1k  ob;
	ob << DBReq;

	this->GetDBProxyClient()->Request(DBReq.m_OnlineNum,enDBCmd_Save_OnLineNum,ob.TakeOsb(), 0, 0);
}

//服务器信息返回
void  GameServer::HandleDBRetGetServerInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("%s 获得服务器信息失败 Cmd=%d",__FUNCTION__,ReqCmd);

		return ;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;

	DB_Server_Info ServerInfo;

	RspIb >> RspHeader >> ServerInfo;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	m_ServerInfo.m_strerverName = (char*)ServerInfo.m_szServerName;
	m_ServerInfo.m_strInstallPackageUrl = (char*)ServerInfo.m_szInstallPackageUrl;
	m_ServerInfo.m_strLastVersion = (char*)ServerInfo.m_szLastVersion;
	m_ServerInfo.m_strMinVersion = (char*)ServerInfo.m_szMinVersion;
	m_ServerInfo.m_strServiceEmail = (char*)ServerInfo.m_szServiceEmail;
	m_ServerInfo.m_strServiceQQ = (char*)ServerInfo.m_szServiceQQ ;
	m_ServerInfo.m_strServiceTel = (char*)ServerInfo.m_szServiceTel ;
	m_ServerInfo.m_OpenServiceTime = ServerInfo.m_OpenServiceTime;
	m_ServerInfo.m_IsNewServer = ServerInfo.m_IsNewServer;
	m_ServerInfo.m_fMultipExp = ServerInfo.m_fMultipExp;
	m_ServerInfo.m_MinMultipExpLv = ServerInfo.m_MinMultipExpLv;
	m_ServerInfo.m_MaxMultipExpLv = ServerInfo.m_MaxMultipExpLv;
	m_ServerInfo.m_MultipExpBeginTime = ServerInfo.m_MultipExpBeginTime;
	m_ServerInfo.m_MultipExpEndTime = ServerInfo.m_MultipExpEndTime;
	if(m_ServerInfo.m_fMultipExp <= 0.000001)
	{
		m_ServerInfo.m_fMultipExp = 1.0;
	}

	UINT32 CurTime = CURRENT_TIME();

	if((m_ServerInfo.m_fMultipExp>1.000001 || m_ServerInfo.m_fMultipExp<1.000001) && CurTime < m_ServerInfo.m_MultipExpBeginTime)
	{
		//启动定时器
		this->GetTimeAxis()->SetTimer(enTimerID_MultipExp,this,( m_ServerInfo.m_MultipExpBeginTime-CurTime)*1000,"GameServer::HandleDBRetGetServerInfo[enTimerID_MultipExp]");
	}

	if( m_ServerInfo.m_OpenServiceTime == 0){
		m_ServerInfo.m_OpenServiceTime = CURRENT_TIME();

		const SConfigParam & Param = this->GetConfigParam();

		//保存
		SDB_Save_FirstOpenServerTime_Req DBReq;

		DBReq.m_ServerID = Param.m_ServerID;
		DBReq.m_PlatformID = Param.m_GamePlatform;
		DBReq.m_Time     = m_ServerInfo.m_OpenServiceTime;

		OBuffer1k obSave;
		obSave << DBReq;
		this->GetDBProxyClient()->Request(DBReq.m_ServerID, enDBCmd_Save_FirstOpenServerTime, obSave.TakeOsb(), 0, 0);
	}

	if(m_ServerInfo.m_strerverName.length()==0)
	{
		m_ServerInfo.m_strerverName = (char*)g_ConfigParam.m_szServerName;
	}
	
	for(int i=0; i< g_ConfigParam.m_vectLoginServerInfo.size(); ++i)
	{
		ILoginSrvProxy * pLoginSrvProxy = ILoginSrvProxy::CreateLoginSrvProxy();

		if(pLoginSrvProxy == 0)
		{
			TRACE("<error> %s : %d 创建登陆服代理失败 !",__FUNCTION__,__LINE__);
			return ;
		}

		SLoginServerInfo & LoginServerInfo = g_ConfigParam.m_vectLoginServerInfo[i];

		pLoginSrvProxy->Init(this,LoginServerInfo.m_szLoginSrvIp,LoginServerInfo.m_LoginSrvPort,
			LoginServerInfo.m_szGameSrvIp, LoginServerInfo.m_GameSrvPort,g_ConfigParam.m_ReportInterval);

		m_pLoginSrvProxy.push_back(pLoginSrvProxy);
	}
}

//获取平台信息返回
void GameServer::HandleDBRetGetPlatformInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("%s 获得平台信息失败 Cmd=%d",__FUNCTION__,ReqCmd);

		return ;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;

	DB_GetPlatformInfo_Rsp PlatformInfo;

	RspIb >> RspHeader >> PlatformInfo;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	/*if(m_pThirdPartProxy==0)
	{
		m_pThirdPartProxy = ::CreateThirdPartProxy(g_ConfigParam.m_GamePlatform);
	}

	if(m_pThirdPartProxy)
	{
		m_pThirdPartProxy->Init(this->m_pSocketSystem,PlatformInfo.m_szThridPartIp,PlatformInfo.m_ThridPartPort,
			PlatformInfo.m_szThridPartAppKey,PlatformInfo.m_szThridPartSecretKey,this,0,0);

		m_pThirdPartProxy->SetPlatformParam(PlatformInfo.m_szMerchantKey,PlatformInfo.m_szDesKey,PlatformInfo.m_MerchantID,PlatformInfo.m_GameID,PlatformInfo.m_ServerID);

		int nCount = RspIb.Remain()/SIZE_OF(DB_PlatformInterface());

		for(int i=0; i<nCount;i++)
		{
			DB_PlatformInterface Interface;

			RspIb >> Interface;

			if(Interface.m_InterfaceID !=0)
			{
				m_pThirdPartProxy->SetInterfaceUrl(Interface.m_InterfaceID,Interface.m_szInterfaceUrl);
			}			
		}
	}*/

}



bool    GameServer::RequestDBByActor(IActor* pActor,enDBCmd ReqCmd, OStreamBuffer & ReqOb)
{
	UINT32 Session = 0;
	ISession * pSession = pActor->GetSession();
	if(pSession==0)
	{
		Session = pSession->GetSessionID();
	}
	return m_pDBProxyClient->Request(pActor->GetCrtProp(enCrtProp_ActorUserID),ReqCmd,ReqOb,this,Session);
}


//装载玩家入内存
bool  GameServer::LoadActor(UID uidActor)
{
	Session * pSession = new Session(TSockID());

	UINT32 SessionID = pSession->GetSessionID();

	m_mapOffLineSession[SessionID] = pSession;

	pSession->LoadActor(uidActor);
	return true;
}

void  GameServer::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_OffLine:
		{
			if(m_mapOffLineSession.empty())
			{
				return;
			}
			mapSession OffLineSession = m_mapOffLineSession; //复制一份，防止跌代中移除m_mapOffLineSession会出错

			for(mapSession::iterator it = OffLineSession.begin(); it != OffLineSession.end();++it )
			{
				Session * pSession = (*it).second;
				pSession->Logout();

			}
		}
		break;
	case enTimerID_SaveToDB:
		{			
			///角色数据定时存盘间隔(单位:秒)
	        INT32 SaveToDBInterval = this->GetConfigServer()->GetGameConfigParam().m_SaveToDBInterval; 

			UINT32 nCurTime = ::time(0); 

            int onlinenum = 0;

			for(mapSession::iterator it = m_mapSession.begin(); it != m_mapSession.end(); ++it)
			{
				IActor* pActor = (*it).second->Actor();
				if(pActor )
				{
					ISession * pSession = pActor->GetSession();
					if(pSession && pSession->IsOnLine())
					{
						onlinenum++;
					}

					if( nCurTime-pActor->GetLastSaveToDBTime() >= SaveToDBInterval)
					{
						pActor->SaveData();
					}
				}
			}

			//记录下服务器的在线玩家数量
			this->SaveOnlineInfo(onlinenum);

		}
		break;
	case enTimerID_Stop:
		{
			OnServerStop();
		}
		break;
	case enTimerID_StopTip:
		{
			--m_leftMinute;

			char szTip[1024] = "\0";
			
			// fly add	20121106
			sprintf_s(szTip, sizeof(szTip), this->GetGameWorld()->GetLanguageStr(10022), m_leftMinute);
			//sprintf_s(szTip, sizeof(szTip), "尊敬的玩家，由于服务器需要停机维护，将在%d分钟后关闭，请您及时下线,以避免数据丢失！", m_leftMinute);

			//关服提示
			this->StopServiceTip(szTip);

			if ( m_leftMinute <= 1){
				
				this->GetTimeAxis()->KillTimer(enTimerID_StopTip,this);
			}
		}
		break;
	case enTimerID_MultipExp:
		{
			this->GetTimeAxis()->KillTimer(enTimerID_MultipExp,this);

			OnMultipExp();
		}
		break;
	}

	TRACE("在线玩家数[%d],断线玩家数[%d]",m_mapSession.size(),m_mapOffLineSession.size());
}

//发送给聊天服
bool GameServer::SendToChatServer(OStreamBuffer & osb)
{
	return this->m_pChatSrvProxy->Send(osb);
}

//关服提示
void GameServer::StopServiceTip(const char * pszTipContent)
{
	//给在线玩家发消息，服务器即将关闭
	this->GetGameWorld()->WorldSystemMsg(pszTipContent);

	this->GetGameWorld()->WorldNotice(0, pszTipContent);
}

void GameServer::OnMultipExp()
{
	for(mapSession::iterator it = m_mapSession.begin(); it != m_mapSession.end(); ++it)
	{
			IActor* pActor = (*it).second->Actor();
			if(pActor )
			{
				ISession * pSession = pActor->GetSession();
				if(pSession && pSession->IsOnLine())
				{
					IActivityPart * pActivityPart = pActor->GetActivityPart();

					pActivityPart->OnMultipExp();
				}
				
			}
	}

}
