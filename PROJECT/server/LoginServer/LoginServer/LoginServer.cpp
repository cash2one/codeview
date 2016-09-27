
#include "LoginServer.h"
#include "LoginSrvProtocol.h"
#include "IDBProxyClient.h"
#include "IBasicService.h"
#include "ILogTrace.h"
#include "FileConfig.h"
#include "boost/bind.hpp"
#include "DBProtocol.h"
#include "ServerProtocol.h"
#include "GameServerProxy.h"
#include "IKeywordFilter.h"
#include "ICrashDump.h"
#include "WebCmd.h"
#include "IThirdPart.h"
#include "ICryptService.h"

SConfigParam g_ConfigParam;
IBasicService * g_pBasicService=NULL;

LoginServer & LoginServer::GetInstance()
{
	static LoginServer __Instance;

	return __Instance;
}

LoginServer::LoginServer()
{
	m_pSocketSystem = 0;
	m_pSignals_ = 0;
	m_pDBProxyClient = 0;
	m_pThirdPartProxy = 0;
}

LoginServer::~LoginServer()
{
	if(m_pDBProxyClient)
	{
		m_pDBProxyClient->Release();
		m_pDBProxyClient = 0;
	}

	if(m_pThirdPartProxy)
	{
		m_pThirdPartProxy->Release();
		m_pThirdPartProxy = 0;
	}

	if(m_pSignals_)
	{
		delete m_pSignals_;
		m_pSignals_ = 0;
	}

	if(m_pSocketSystem)
	{
		m_pSocketSystem->Release();
		m_pSocketSystem = 0;
	}
}

bool LoginServer::Init(const char* szFileName)
{
	if(szFileName==NULL)
	{
		return false;
	}
	g_pBasicService = GetBasicService();

	if(g_pBasicService==NULL || g_pBasicService->Create()==false)
	{
		return false;
	}

	ICrashDump * pCrashDump = ::CreateCrashDump();

	if(pCrashDump != 0)
	{
		
		pCrashDump->CrtSetDbgFlag();
	}

	ILogTrace * pLogTrace = g_pBasicService->GetLogTrace();

	if(pLogTrace->Create("./LoginServer.log")==false)
	{
		return false;
	}

	FileConfig File;
	if(!File.Open(szFileName,"LoginServer"))
	{
		TRACE("<error> %s : %d 打开配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	if(File.Read(g_ConfigParam)==false)
	{
		TRACE("<error> %s : %d 读取配置参数失败!",__FUNCTION__,__LINE__);
		return false;
	}


	//创建通讯库
	m_pSocketSystem = CreateSocketSystem();
	if(m_pSocketSystem==NULL)
	{
		TRACE("<error> %s : %d创建通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	if(m_pSocketSystem->Start(g_pBasicService->GetIoService(),g_ConfigParam.m_MaxConnNum,g_ConfigParam.m_NetIoThreadNum)==false)
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

	
	if(m_pSocketSystem->Listen(g_ConfigParam.m_szIp,g_ConfigParam.m_Port,this,enPacketHeaderType_Len,g_ConfigParam.m_KeepLiveTime)==false)
	{
		TRACE("<error> %s : %d 通讯库监听失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)g_ConfigParam.m_szIp,g_ConfigParam.m_Port);
		return false;
	}

	m_pSignals_ = new boost::asio::signal_set(*g_pBasicService->GetIoService());

	m_pSignals_->add(SIGINT);
	m_pSignals_->add(SIGTERM);

#if defined(SIGQUIT)
	m_pSignals_->add(SIGQUIT);
#endif // defined(SIGQUIT)

	m_pSignals_->async_wait(boost::bind(&LoginServer::handle_stop, this));


	GetKeyWorldInfo();

	GetServerInfo();
	

	return true;
}
bool LoginServer::Run()
{
	return g_pBasicService->GetIoService()->run();
}

void LoginServer::GetKeyWorldInfo()
{
	//获取关键词
	DB_Get_Keyword_Info_Req Req;

	OBuffer1k ob;

	ob << Req;

	if(m_pDBProxyClient->Request(0,enDBCmd_GetKeywordInfo,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}
}

//获得服务器信息
void LoginServer::GetServerInfo()
{
	//获取系统信息
	DB_Get_Server_Info_Req NoticReq;

	OBuffer1k ob;

	NoticReq.m_ServerID = g_ConfigParam.m_ServerID;
	NoticReq.m_PlatformID = g_ConfigParam.m_GamePlatform;

	ob << NoticReq;

	if(m_pDBProxyClient->Request(0,enDBCmd_Get_ServerInfo,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}
}

//数据到达,
void LoginServer::OnNetDataRecv(TSockID socketid,OStreamBuffer & osb)
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

	//分发
	switch(PacketHeader.MsgCategory)
	{
	case enMsgCategory_Login:
		{
			HandleClientMsg(socketid,PacketHeader.command,ib,ob);
		}
		break;
	case enMsgCategory_GameServer:
		{
			HandleGameServerMsg(socketid,PacketHeader.command,ib,ob);
		}
		break;
	case enMsgCategory_WebServer:
		{
			HandleWebServerMsg(socketid,PacketHeader.command,ib,ob);
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 接收到的数据消息分类有误，MsgCategory = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
				PacketHeader.MsgCategory,m_pSocketSystem->GetRemote(socketid));

			m_pSocketSystem->ShutDown(socketid);
		}
		break;
	}
}

//连接关闭
void LoginServer::OnClose(TSockID sockid)
{
}


void LoginServer::handle_stop() //停止
{
	m_pDBProxyClient->Close();

	m_pSocketSystem->Stop();

	g_pBasicService->GetIoService()->stop();	

	ICrashDump * pCrashDump = ::CreateCrashDump();	
}


//前端数据返回
void LoginServer::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode,OStreamBuffer & RspOsb,
						  OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	}

	switch(ReqCmd)
	{
	case enDBCmd_GetUserInfo:
		{
			HandleDBRetGetUserInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertUserInfo:
		{
			HandleDBRetInsertUserInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetKeywordInfo:
		{
			HandleDBRetGetKeywordInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_ServerInfo:
		{
			HandleDBRetGetServerInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Insert_PointMsg:
		{
			HandleDBRetInsertPointInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
			TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		}
		break;
	}
}

void LoginServer::HandleDBRetGetUserInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
										 OBuffer4k & ReqOb,UINT64 userdata)
{
	TSockID socketid(userdata);

	if(nRetCode != enDBRetCode_OK)
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_User_Info_Record       UserInfo;
	RspIb >> RspHeader >> UserInfo;

	if(RspIb.Error())
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
		TRACE("HandleDBRetGetUserInfo DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
	}

	if(UserInfo.retCode != enUserInfoRetCode_OK)
	{
		switch(UserInfo.retCode)
		{
		case enUserInfoRetCode_NoUser:
			{
				HandleLoginFailRsp(socketid,enLoginRetCode_ErrNoUser,RspOb);
			}
			break;
		case enUserInfoRetCode_ErrorPwd:
			{
				HandleLoginFailRsp(socketid,enLoginRetCode_ErrPwd,RspOb);
			}
			break;	
		case enUserInfoRetCode_SealNo:
			{
				HandleLoginFailRsp(socketid,enLoginRetCode_ErrSealNo,RspOb);
			}
			break;
		case enUserInfoRetCode_Define:
			{
				HandleLoginFailRsp(socketid,enLoginRetCode_ErrDefend,RspOb);
			}
			break;
		default:
			{
				HandleLoginFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
				TRACE("HandleDBRetGetUserInfo DB前端应答输出参数值意外 cmd=%d userID = %u OutParam.retCode =%d",ReqCmd,DBUserID,UserInfo.retCode);
				return;
			}
			break;
		}

		return;

	}

	SC_MyServerList MyServer;

	MyServer.ServerNum = RspIb.Remain()/sizeof(DB_My_ServerInfo);  
	OBuffer4k ob;   //需要先组包，HandleLoginSuccessRsp()函数会清除RspIb的内容

	ob << LoginHeader(enLoginCmd_SC_MyServerList,sizeof(MyServer)+ RspIb.Remain()) << MyServer;

	if(MyServer.ServerNum>0)
	{
		ob.Push(RspIb.CurrentBuffer(),RspIb.Remain());
	}

	
	//应答成功
	HandleLoginSuccessRsp(socketid,UserInfo.UserID,UserInfo.IdentityStatus,UserInfo.OnlineTime,RspOb);

	//发送我的服务器列表
	m_pSocketSystem->Send(socketid,ob.TakeOsb());

	//登陆结束
	ob.Reset();

	ob << LoginHeader(enLoginCmd_SC_Finish,0);


	m_pSocketSystem->Send(socketid,ob.TakeOsb());


}


//注册DB应答
void LoginServer::HandleDBRetInsertUserInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	TSockID socketid(userdata);

	if(nRetCode != enDBRetCode_OK)
	{
		HandleRegisterFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_User_Info_Record       UserInfo;
	RspIb >> RspHeader >> UserInfo;

	if(RspIb.Error())
	{
		HandleRegisterFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
		TRACE("%s DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,ReqCmd,DBUserID,RspIb.Capacity());
	}

	if(UserInfo.retCode != enUserInfoRetCode_OK)
	{
		switch(UserInfo.retCode)
		{
		case enUserInfoRetCode_ExistUser:
			{
				HandleRegisterFailRsp(socketid,enLoginRetCode_ErrExistName,RspOb);
			}
			break;				
		default:
			{
				HandleRegisterFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
				TRACE("%s DB前端应答输出参数值意外 cmd=%d userID = %u OutParam.retCode =%d",__FUNCTION__,ReqCmd,DBUserID,UserInfo.retCode);
				return;
			}
			break;
		}

		return;

	}


	if(RspIb.Error())
	{
		HandleRegisterFailRsp(socketid,enLoginRetCode_ErrDB,RspOb);
		TRACE("%s DB前端应答长度有误 len=%d",__FUNCTION__,RspIb.Capacity());
	}

	//应答成功
	HandleRegisterSuccessRsp(socketid,UserInfo.UserID,RspOb);
}


//获得关键词
void LoginServer::HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;

	RspIb >> RspHeader;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	g_pBasicService->GetKeywordFilter()->ClearKeyword();

	INT32 nCout = RspIb.Remain()/SIZE_OF(DB_GetKeyword_Info_Rsp());

	for(int i=0; i<nCout; i++)
	{
		DB_GetKeyword_Info_Rsp Rsp;

		RspIb >> Rsp;

		const char * ptr = (char*)Rsp.m_Keyword;

		g_pBasicService->GetKeywordFilter()->AddKeyword(ptr);
	}
}

	//获取系统公告
void LoginServer::HandleDBRetGetServerInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	DBRspPacketHeader RspHeader;
	DB_GetServerInfo_Rsp Rsp;

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	RspIb >> RspHeader >> Rsp;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	this->m_strPublicNotic = (char*) Rsp.m_szPublicNotic;

	this->m_strUpdateDesc = (char*) Rsp.m_szUpdateDesc;

	this->m_strLastVersion = (char*)Rsp.m_szLastVersion;

	this->m_strMinVersion = (char*)Rsp.m_szMinVersion;

	this->m_strResUrl = (char*)Rsp.m_szInstallPackageUrl;

	this->m_strServiceTel = (char*)Rsp.m_szServiceTel;

	this->m_strServiceEmail = (char*)Rsp.m_szServiceEmail;

	this->m_strServiceQQ = (char*)Rsp.m_szServiceQQ;

	int nCount = RspIb.Remain()/SIZE_OF(DB_InstallPackageUrl());

	for(int i=0; i<nCount;i++)
	{
		DB_InstallPackageUrl PackageUrl;
		RspIb >>  PackageUrl;

		if(RspIb.Error())
	   {		
		 TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		 break;
	   }
		
		m_mapInstallPackageUrl[PackageUrl.m_PlatformID] =  PackageUrl.m_szInstallPackageUrl;
	}

}

//插入节点信息
void LoginServer::HandleDBRetInsertPointInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	TSockID socketid(userdata);

	DBRspPacketHeader RspHeader;
	DB_OutParam		  OutParam;
	SDB_BackVid		  BackVid;

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	RspIb >> RspHeader >> OutParam >> BackVid;

	if(RspIb.Error())
	{		
		TRACE("%s DB前端应答长度有误 cmd=%d  len=%d",__FUNCTION__,ReqCmd,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK )
	{
		BackVid.m_Vid = 0;
	}

	this->SendPointBack(socketid,BackVid.m_Vid);
}

void LoginServer::HandleClientMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib,OBuffer4k & ob) //处理客户端到达的消息
{

	switch(nCmd)
	{
	case enLoginCmd_CS_Login:  //登陆请求
		{
			HandleLoginReq(socketid,nCmd,ib,ob);
		}
		break;
	case enLoginCmd_CS_Register:
		{
			HandleRegisterReq(socketid,nCmd,ib,ob);
		}
		break;
	case enLoginCmd_CS_ServerState:
		{
			HandleGetServerStateReq(socketid,nCmd,ib,ob); //获取服务器状态
		}
		break;
	case enLoginCmd_CS_CheckVersion: //检查版本号
		{
			HandleCheckVersion(socketid,nCmd,ib,ob);
		}
		break;
	case enLoginCmd_CS_CheckVersionEx: //检查版本号
		{
			HandleCheckVersionEx(socketid,nCmd,ib,ob);
		}
		break;
	case enLoginCmd_CS_SendPoint:
		{
			HandleSendPoint(socketid,nCmd,ib,ob);
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 接收到客户端的数据消息命令字有误，nCmd = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
				nCmd,m_pSocketSystem->GetRemote(socketid));

			m_pSocketSystem->ShutDown(socketid);
		}
		break;
	}
}

void LoginServer::HandleGameServerMsg(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob) //处理游戏服务器到达的消息
{
	switch(nCmd)
	{
	case enServerCmd_SS_LoginLoginSrv:  //登陆请求
		{
			HandleGameServerLoginReq(socketid,nCmd,ib,ob);
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 接收到游戏服的数据消息命令字有误，nCmd = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
				nCmd,m_pSocketSystem->GetRemote(socketid));

			m_pSocketSystem->ShutDown(socketid);
		}
		break;
	}
}


void LoginServer::HandleWebServerMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob)
{
	switch(nCmd)
	{
	case enWebSrvCmd_PublicNotic:  	
	case enWebSrvCmd_UpdateDesc:		
	case enWebSrvCmd_Version:
	case enWebSrvCmd_Service:
		{
			GetServerInfo();
		}
		break;
		
	case enWebSrvCmd_KeyWorld:
		{
			GetKeyWorldInfo();
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 接收到WEB服的数据消息命令字有误，nCmd = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
				nCmd,m_pSocketSystem->GetRemote(socketid));

			m_pSocketSystem->ShutDown(socketid);
		}
		break;
	}
}


//游戏服登陆
void LoginServer::HandleGameServerLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	SS_Login_LoginSrv_Req Req;
	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	SGameServerInfo Info;
	Info.bCommend = Req.bCommend;
	Info.bNew = Req.bNew;
	Info.ServerID = Req.ServerID;
	strncpy(Info.ServerIp ,Req.ServerIp,MEM_SIZE(Info.ServerIp));
	Info.ServerPort = Req.ServerPort;
	strncpy(Info.szServerName, Req.szServerName,sizeof(Info.szServerName));
	Info.State = Req.State;

	TRACE("游戏服务器:%s [%s:%d]注册成功!",Req.szServerName,Info.ServerIp,Info.ServerPort);

	int index = 0;

	if(GetGameServerInfoIndex(Req.ServerID,index))
	{
		m_vectServerInfo[index] = Info;		
	}
	else
	{
		m_vectServerInfo.insert(m_vectServerInfo.begin()+index,Info);
	}

	m_pSocketSystem->SetSocketSink(socketid,new GameServerProxy(Req.ServerID,m_pSocketSystem));
	m_pSocketSystem->SetKeepLiveTime(socketid,g_ConfigParam.m_ServerKeepLiveTime);
}

bool LoginServer::GetGameServerInfoIndex(TServerID ServerID,int & index)
{
	for(index=0; index<m_vectServerInfo.size();index++)
	{
		if(m_vectServerInfo[index].ServerID == ServerID)
		{
			return true;
		}

	}

	return false;
}

//游戏服断开连接
void LoginServer::OnGameServerClose(TServerID ServerID)
{
	int index = 0;

	if(GetGameServerInfoIndex(ServerID,index)==false)
	{
		return;
	}

	//移除
	m_vectServerInfo.erase(m_vectServerInfo.begin()+index);

}

//游戏服汇报状态
void LoginServer::HandleReportStateReq(TServerID ServerID,enServerState State)
{
	int index = 0;

	if(GetGameServerInfoIndex(ServerID,index)==false)
	{
		return;
	}

	m_vectServerInfo[index].State = State;
}

//具体的消息请求

//登陆请求
void  LoginServer::HandleLoginReq(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob)
{

	CS_Login_Req LoginReq;

	ib >>  LoginReq;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的登陆请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	//验证版本号
	/*if(LoginReq.ClientVersion < 1)
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrVersion,ob);
		return ;
	}*/


	LoginReq.szUserName[sizeof(LoginReq.szUserName)-2]=0;
	LoginReq.szPassword[sizeof(LoginReq.szPassword)-2]=0;
	LoginReq.szAccessSecret[sizeof(LoginReq.szAccessSecret)-2] = 0;

	TRACE("帐户[%s]，密码[%s] 渠道号[%d]请求登陆了!",LoginReq.szUserName,LoginReq.szPassword,LoginReq.ChanelType);

	if(this->m_pThirdPartProxy != 0)
	{
		char * ptr = new char[SIZE_OF(SThirdPartReq())+SIZE_OF(LoginReq)];
		SThirdPartReq *pThirdPartReq = (SThirdPartReq *)ptr;
		pThirdPartReq->m_nCmd = nCmd;
		pThirdPartReq->m_SockID = socketid;
		memcpy(pThirdPartReq+1,&LoginReq,SIZE_OF(LoginReq));

		m_pThirdPartProxy->UserLogin(LoginReq.szUserName,LoginReq.szPassword,(UINT32)pThirdPartReq);

	}
	else
	{
		LoginReqToDB(socketid,LoginReq);
	}	
}

//登陆请求数据库
void LoginServer::LoginReqToDB(TSockID socketid,CS_Login_Req & LoginReq)
{
	//请求数据库
	DB_Get_User_Info_Req DBReq;
	strncpy(DBReq.szUserName,LoginReq.szUserName,sizeof(DBReq.szUserName));
	strncpy(DBReq.szPassword,LoginReq.szPassword,sizeof(DBReq.szPassword));	
	strncpy(DBReq.szAccessSecret,LoginReq.szAccessSecret,sizeof(DBReq.szAccessSecret));	
	strncpy(DBReq.szIp,IpToStr(m_pSocketSystem->GetRemote(socketid)),ARRAY_SIZE(DBReq.szIp));
	DBReq.Chanel = LoginReq.ChanelType; 
	
	OBuffer4k ob;

	ob << DBReq;

	if(m_pDBProxyClient->Request(socketid.slot,enDBCmd_GetUserInfo,ob.TakeOsb(),this,socketid.m_id)==false)
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrDB,ob);
		return;
	}
}


//请求检查版本号
void LoginServer::HandleCheckVersion(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	CS_CheckVersion_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的登陆请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	Req.ClientVersion[ARRAY_SIZE(Req.ClientVersion)-1] = 0;

	CS_CheckVersionEx_Req ReqEx;

	memcpy(ReqEx.ClientVersion,Req.ClientVersion,sizeof(ReqEx.ClientVersion));

	//旧协议，默认是当乐平台
	ReqEx.ChanelType = enChanelType_DangLe;

	
	HandleCheckVersion_i(socketid,nCmd,ReqEx,ob);    
}


//请求检查版本号
void LoginServer::HandleCheckVersionEx(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	CS_CheckVersionEx_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的登陆请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	Req.ClientVersion[ARRAY_SIZE(Req.ClientVersion)-1] = 0;

	HandleCheckVersion_i(socketid,nCmd,Req,ob);
    
}

//获得安装包路径
std::string LoginServer::GetInstallPackageUrl(UINT32 PaltformID)
{
	MAP_INSTALLPACKAGE::iterator it = m_mapInstallPackageUrl.find(PaltformID);

	if(it == m_mapInstallPackageUrl.end())
	{
		TRACE("<error> %s : %d line 无法获得平台[%d]安装包路径!",__FUNCTION__,__LINE__,PaltformID);
		return "";
	}

	return (*it).second;
		
}

//请求检查版本号
void LoginServer::HandleCheckVersion_i(TSockID socketid,UINT8 nCmd,CS_CheckVersionEx_Req & Req,OBuffer4k & ob)
{

	Req.ClientVersion[ARRAY_SIZE(Req.ClientVersion)-1] = 0;

	SC_CheckVersion_Rsp Rsp;

	int ret = strcmp(Req.ClientVersion,this->m_strLastVersion.c_str());

	ob.Reset();

	std::string strInstallPackage;

	if(ret==0)
	{
		Rsp.m_Result = enCheckVersionCode_OK;		
	}
	else 
	{		
		strInstallPackage = GetInstallPackageUrl(Req.ChanelType);

		ret = strcmp(Req.ClientVersion,this->m_strMinVersion.c_str());

		if(ret>=0)
		{
			Rsp.m_Result = enCheckVersionCode_Can;
		}
		else
		{
			Rsp.m_Result = enCheckVersionCode_Must;
		}

		
	}

	int len = m_strResUrl.length()+m_strPublicNotic.length()+m_strUpdateDesc.length()+strInstallPackage.length()+m_strServiceTel.length()+m_strServiceEmail.length()+m_strServiceQQ.length()+7;

 	ob << LoginHeader(enLoginCmd_SC_CheckVersion,sizeof(Rsp)+len) << Rsp << m_strPublicNotic<<m_strUpdateDesc<<strInstallPackage<<m_strServiceTel <<m_strServiceEmail<<m_strServiceQQ << m_strResUrl;


	m_pSocketSystem->Send(socketid,ob.TakeOsb()); 

    
}

//处理客户端发上来的节点问题
void LoginServer::HandleSendPoint(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	CS_SendPoint_Login Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的登陆请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		return;
	}

	switch(Req.m_Operator)
	{
	case enEnterGameOperator_CheckVersion:
		{
			if ( Req.m_Vid != 0 )
			{
				//已经有虚拟ID了，不用再插入这个节点了，已经有了
				this->SendPointBack(socketid,Req.m_Vid);
			}
			else
			{
				//没有虚拟ID,需要向数据库插入，向返回虚拟ID
				SDB_Insert_PointMsg_Req DBReq;

				DBReq.m_Vid = Req.m_Vid;
				DBReq.m_UserID = Req.m_UserID;
				DBReq.m_Operator = (UINT8)enEnterGameOperator_CheckVersion;

				OBuffer1k ob2;

				ob2 << DBReq;

				if(m_pDBProxyClient->Request(socketid.slot,enDBCmd_Insert_PointMsg,ob2.TakeOsb(),this,socketid.m_id)==false)
				{
					this->SendPointBack(socketid,0);
					return;
				}
			}
		}
		break;
	case enEnterGameOperator_EnterSelectServer:
		{
			SDB_Insert_PointMsg_Req DBReq;

			DBReq.m_Vid = Req.m_Vid;
			DBReq.m_UserID = Req.m_UserID;
			DBReq.m_Operator = (UINT8)enEnterGameOperator_EnterSelectServer;

			OBuffer1k ob2;

			ob2 << DBReq;

			m_pDBProxyClient->Request(socketid.slot,enDBCmd_Insert_PointMsg,ob2.TakeOsb(),0,0);

			this->SendPointBack(socketid,Req.m_Vid);
		}
		break;
	//case enEnterGameOperator_EnterCreateActor:
	//	{
	//		SDB_Insert_PointMsg_Req DBReq;

	//		DBReq.m_Vid = Req.m_Vid;
	//		DBReq.m_UserID = Req.m_UserID;
	//		DBReq.m_Operator = (UINT8)enEnterGameOperator_EnterCreateActor;

	//		OBuffer1k ob2;

	//		ob2 << DBReq;

	//		m_pDBProxyClient->Request(socketid.slot,enDBCmd_Insert_PointMsg,ob2.TakeOsb(),0,0);

	//		this->SendPointBack(socketid,Req.m_Vid);	
	//	}
	//	break;
	}
}

//登陆成功应答
void LoginServer::HandleLoginSuccessRsp(TSockID socketid,TUserID UserID,UINT8 IdentityStatus,INT32 OnlineTime,OBuffer4k & ob)
{

	ob.Reset();
	//成功应答
	SC_Login_Success_Rsp Rsp;
	Rsp.UserID = UserID;
	Rsp.ServerTime = std::clock();
	SS_UserPrivateData * pUserPrivateData = (SS_UserPrivateData*)Rsp.szTicket;
	pUserPrivateData->m_UserID = UserID;
	pUserPrivateData->m_IdentityStatus = IdentityStatus;
	pUserPrivateData->m_OnlineTime = OnlineTime;

	
	ob << LoginHeader(enLoginCmd_SC_LoginSuccess,sizeof(Rsp)) << Rsp;

	m_pSocketSystem->Send(socketid,ob.TakeOsb());  //注意多个包不能连在一起发


	//返回游戏列表
	PushGameServerList(socketid,ob);

}

//Push游戏服务器列表到OBuffer
void  LoginServer::PushGameServerList(TSockID socketid,OBuffer4k & ob)
{
	ob.Reset();

	SC_Server_List_Syn ServerList;
	ServerList.ServerNum = this->m_vectServerInfo.size();

	UINT32 length = sizeof(SC_Server_List_Syn)+sizeof(SGameServerInfo)*ServerList.ServerNum;

	ob << LoginHeader(enLoginCmd_SC_ServerList,length) << ServerList;

	if(ServerList.ServerNum)
	{
		ob.Push(&m_vectServerInfo[0],sizeof(SGameServerInfo)*m_vectServerInfo.size());
	}
	
	//一次性发送
	m_pSocketSystem->Send(socketid,ob.TakeOsb());

}

//登陆失败应答
void LoginServer::HandleLoginFailRsp(TSockID socketid,UINT8 retCode,OBuffer4k & ob)
{
	ob.Reset();
	SC_Login_Faile_Rsp Rsp;
	Rsp.m_Result = (enLoginRetCode)retCode;

	ob << LoginHeader(enLoginCmd_SC_LoginFaile,sizeof(SC_Login_Faile_Rsp)) << Rsp;

	m_pSocketSystem->Send(socketid,ob.TakeOsb());

}


//注册请求
void LoginServer::HandleRegisterReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	CS_Register_Req RegisterReq;

	ib >>  RegisterReq;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的注册请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	//验证版本号
	/*if(RegisterReq.ClientVersion < 1)
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrVersion,ob);
		return ;
	}*/


	RegisterReq.szUserName[sizeof(RegisterReq.szUserName)-2] = 0;
	RegisterReq.szPassword[sizeof(RegisterReq.szPassword)-2] = 0;

	//TRACE("帐户[%s]，密码[%s] 请求注册了!",RegisterReq.szUserName,RegisterReq.szPassword);

	//过滤关键词
	if(g_pBasicService->GetKeywordFilter()->IsValidName(RegisterReq.szUserName)==false)
	{
		HandleLoginFailRsp(socketid,enLoginRetCode_ErrKeyName,ob);
	}
	else
	{
		if(this->m_pThirdPartProxy != 0)
		{
			char * ptr = new char[SIZE_OF(SThirdPartReq())+SIZE_OF(RegisterReq)];
		   SThirdPartReq *pThirdPartReq = (SThirdPartReq *)ptr;
		   pThirdPartReq->m_nCmd = nCmd;
		   pThirdPartReq->m_SockID = socketid;
		   memcpy(pThirdPartReq+1,&RegisterReq,SIZE_OF(RegisterReq));

		   m_pThirdPartProxy->UserRegister(RegisterReq.szUserName,RegisterReq.szPassword,(UINT32)pThirdPartReq);
		}
		else
		{
		   //请求数据库
		   RegisterToDB(socketid,RegisterReq);
		}	
	}
}

//向数据库注册
void LoginServer::RegisterToDB(TSockID socketid,CS_Register_Req & RegisterReq)
{
		DB_Insert_User_Info_Req DBReq;
		strncpy(DBReq.szUserName,RegisterReq.szUserName,sizeof(DBReq.szUserName));

	/*	if(RegisterReq.m_CryptType == enCryptType_No)
		{
			TSHA256 sha256 = g_pBasicService->GetCryptService()->CalculateSHA256((unsigned char*)(char*)DBReq.szPassword,strlen(DBReq.szPassword));
			 strncpy(DBReq.szPassword,sha256.toString().c_str(),sizeof(DBReq.szPassword));
		}
		else
		{
		   strncpy(DBReq.szPassword,RegisterReq.szPassword,sizeof(DBReq.szPassword));	
		}*/

		 strncpy(DBReq.szPassword,RegisterReq.szPassword,sizeof(DBReq.szPassword));


		strncpy(DBReq.szIp,IpToStr(m_pSocketSystem->GetRemote(socketid)),ARRAY_SIZE(DBReq.szIp));
		DBReq.Chanel = g_ConfigParam.m_GamePlatform;
		
		OBuffer4k ob;

		ob << DBReq;

		if(m_pDBProxyClient->Request(socketid.slot,enDBCmd_InsertUserInfo,ob.TakeOsb(),this,socketid.m_id)==false)
		{
			HandleLoginFailRsp(socketid,enLoginRetCode_ErrDB,ob);
			return;
		}
}

//注册成功应答
void LoginServer::HandleRegisterSuccessRsp(TSockID socketid,TUserID UserID,OBuffer4k & ob)
{
	ob.Reset();
	//成功应答
	SC_Register_Success_Rsp Rsp;
	Rsp.UserID = UserID;
	Rsp.ServerTime = std::clock();
	
	SS_UserPrivateData * pUserPrivateData = (SS_UserPrivateData*)Rsp.szTicket;
	pUserPrivateData->m_UserID = UserID;
	pUserPrivateData->m_IdentityStatus = 0;
	pUserPrivateData->m_OnlineTime = 0;

	ob << LoginHeader(enLoginCmd_SC_RegisterSuccess,sizeof(Rsp)) << Rsp;

	//一次性发送
	m_pSocketSystem->Send(socketid,ob.TakeOsb());

	//返回游戏列表
	PushGameServerList(socketid,ob);

	//登陆结束
	ob.Reset();
	ob << LoginHeader(enLoginCmd_SC_Finish,0);

	//一次性发送
	m_pSocketSystem->Send(socketid,ob.TakeOsb());

}

//注册失败应答
void LoginServer::HandleRegisterFailRsp(TSockID socketid,UINT8 retCode,OBuffer4k & ob)
{
	ob.Reset();
	SC_Register_Faile_Rsp Rsp;
	Rsp.m_Result = (enLoginRetCode)retCode;	

	ob << LoginHeader(enLoginCmd_SC_RegisterFaile,sizeof(Rsp)) << Rsp;

	m_pSocketSystem->Send(socketid,ob.TakeOsb());

}



	 	//登陆返回
void LoginServer::OnLoginRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata)
{
	char * ptr = (char*)(void*)userdata;
	 SThirdPartReq *pThirdPartReq = (SThirdPartReq *)ptr;
	  if(pThirdPartReq==0)
	  {
		  return;
	  }

	  if(RetCode == enThirdPartRetCode_OK)
	  {
		  CS_Login_Req * pLoginReq = ( CS_Login_Req *)(pThirdPartReq+1);

		  LoginReqToDB(pThirdPartReq->m_SockID,*pLoginReq);
	  }
	  else
	  {
             enLoginRetCode LoginRetCode = GetRetCode(RetCode);

			 OBuffer4k ob;
			 this->HandleLoginFailRsp(pThirdPartReq->m_SockID,LoginRetCode,ob);
	  }

	  delete [] ptr;
}

enLoginRetCode LoginServer::GetRetCode(enThirdPartRetCode RetCode)
{
	enLoginRetCode LoginRetCode = enLoginRetCode_ErrUnknow ;
	switch(RetCode)
	{
	case enThirdPartRetCode_OK:
		LoginRetCode =	enLoginRetCode_OK;
		break;
	case enThirdPartRetCode_ErrUnknow:
		LoginRetCode =	enLoginRetCode_ErrUnknow;
		break;

	case enThirdPartRetCode_UserNotExist:
		LoginRetCode =	enLoginRetCode_ErrNoUser;
		break;

	case enThirdPartRetCode_ErrPwd:
		LoginRetCode =	enLoginRetCode_ErrPwd;
		break;

	case enThirdPartRetCode_UserExit:
		LoginRetCode =	enLoginRetCode_ErrExistName;
		break;

	case enThirdPartRetCode_Close:
		LoginRetCode =	enLoginRetCode_ErrDB;
		break;
	}

	return LoginRetCode;
}


	//注册返回
void LoginServer::OnRegisterRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata)
{
	char * ptr = (char*)(void*)userdata;
	 SThirdPartReq *pThirdPartReq = (SThirdPartReq *)ptr;
	  if(pThirdPartReq==0)
	  {
		  return;
	  }

	  if(RetCode == enThirdPartRetCode_OK)
	  {
		  CS_Register_Req * pLoginReq = ( CS_Register_Req *)(pThirdPartReq+1);

		  RegisterToDB(pThirdPartReq->m_SockID,*pLoginReq);
	  }
	  else
	  {
             enLoginRetCode LoginRetCode = GetRetCode(RetCode);

			 OBuffer4k ob;
			 this->HandleLoginFailRsp(pThirdPartReq->m_SockID,LoginRetCode,ob);
	  }

	  delete [] ptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//获取服务器状态
void LoginServer::HandleGetServerStateReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	CS_Server_State_Req StateReq;

	ib >>  StateReq;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的服务器状态请求数据长度有误，len = %u,remote_ip=%s!",__FUNCTION__,__LINE__,
			ob.Size(),IpToStr(m_pSocketSystem->GetRemote(socketid)));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	ob.Reset();

	SC_Server_State_Rsp Rsp;
	Rsp.ServerNum = m_vectServerInfo.size();

	int length = sizeof(SC_Server_State_Rsp) + sizeof(SServerStateInfo)*Rsp.ServerNum;

	ob << LoginHeader(enLoginCmd_SC_ServerState,sizeof(Rsp)) << Rsp;

	if(Rsp.ServerNum)
	{
		SServerStateInfo StateInfo;
		for(int i=0; i<Rsp.ServerNum;++i)
		{
			StateInfo.ServerID = m_vectServerInfo[i].ServerID;

			StateInfo.State = m_vectServerInfo[i].State;

			ob << StateInfo;
		}


	}

	//一次性发送
	m_pSocketSystem->Send(socketid,ob.TakeOsb());


}

//不是线程安全
const  char* LoginServer::IpToStr(UINT32 Ip)
{
	static  char szIp[20]={0}; //必须为静态的
	const unsigned char * ptr = (const unsigned char*)&Ip;
	sprintf_s(szIp,sizeof(szIp),"%d.%d.%d.%d",ptr[3],ptr[2],ptr[1],ptr[0]);

	return szIp;
}

UINT32 LoginServer::IpToUINT(const char* szIp)
{
	UINT32 ip = 0;

	char szTemp[20]={0};

	const char * ptr1 = szIp;
	const char * ptr2 = 0;

	while((ptr2=strchr(ptr1,'.')) != 0)
	{
		memmove(szTemp,ptr1,ptr2-ptr1);
		ip += atoi(szTemp);
		ip <<= 8;

		ptr1 = ptr2+1;
	}

	if(ptr1 && *ptr1)
	{
		ip += atoi(ptr1);
	}

	return ip;
}

//发送节点的返回
void LoginServer::SendPointBack(TSockID socketid,UINT32 Vid)
{
	SC_SendPoint Rsp;

	Rsp.m_Vid = Vid;

	OBuffer1k ob;
	ob << LoginHeader(enLoginCmd_SC_SendPoint,sizeof(Rsp)) << Rsp;
	m_pSocketSystem->Send(socketid,ob.TakeOsb());
}
