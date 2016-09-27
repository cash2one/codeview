
#include "IKeywordFilter.h"

#include "ChatServer.h"
#include "IBasicService.h"
#include "FileConfig.h"
#include "boost/bind.hpp"
#include "DBProtocol.h"
#include "ServerProtocol.h"
#include "GameServerProxy.h"



SConfigParam g_ConfigParam;
IBasicService * g_pBasicService=NULL;

ChatServer & ChatServer::GetInstance()
{
	static ChatServer __Instance;

	return __Instance;
}

ChatServer::ChatServer()
{
	m_pSocketSystem = 0;
	m_pSignals_ = 0;
	m_pDBProxyClient = 0;
}

ChatServer::~ChatServer()
{
	if(m_pDBProxyClient)
	{
		m_pDBProxyClient->Release();
		m_pDBProxyClient = 0;
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

bool ChatServer::Init(const char* szFileName)
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

	ILogTrace * pLogTrace = g_pBasicService->GetLogTrace();

	if(pLogTrace->Create("./ChatServer.log")==false)
	{
		return false;
	}

	FileConfig File;
	if(!File.Open(szFileName,"ChatServer"))
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

	if(m_pSocketSystem->Start(g_pBasicService->GetIoService(),g_ConfigParam.m_MaxConnNum,g_ConfigParam.m_NetIoThreadNum,g_ConfigParam.m_KeepLiveTime)==false)
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

	m_pSignals_ = new boost::asio::signal_set(*g_pBasicService->GetIoService());

	m_pSignals_->add(SIGINT);
	m_pSignals_->add(SIGTERM);

#if defined(SIGQUIT)
	m_pSignals_->add(SIGQUIT);
#endif // defined(SIGQUIT)

	m_pSignals_->async_wait(boost::bind(&ChatServer::handle_stop, this));


	//获取关键词
	DB_Get_Keyword_Info_Req Req;

	OBuffer1k ob;

	ob << Req;

	if(m_pDBProxyClient->Request(0,enDBCmd_GetKeywordInfo,ob.TakeOsb(),this,0)==false)
	{
		TRACE("<error> %s :%d line 发起DB请求失败!");
	}

	return true;
}
bool ChatServer::Run()
{
	return g_pBasicService->GetIoService()->run();
}


//数据到达,
void ChatServer::OnNetDataRecv(TSockID socketid,OStreamBuffer & osb)
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
void ChatServer::OnClose(TSockID sockid)
{
}


void ChatServer::handle_stop() //停止
{
	m_pDBProxyClient->Close();

	m_pSocketSystem->Stop();

	g_pBasicService->GetIoService()->stop();	
}


//前端数据返回
void ChatServer::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode,OStreamBuffer & RspOsb,
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
	case enDBCmd_GetKeywordInfo:
		{
			HandleDBRetGetKeywordInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
			TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		}
		break;
	}
}

//获得关键词
void ChatServer::HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
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

	INT32 nCout = RspIb.Remain()/SIZE_OF(DB_GetKeyword_Info_Rsp());

	for(int i=0; i<nCout; i++)
	{
		DB_GetKeyword_Info_Rsp Rsp;

		RspIb >> Rsp;

		const char * ptr = (char*)Rsp.m_Keyword;

		g_pBasicService->GetKeywordFilter()->AddKeyword(ptr);
	}
}

void ChatServer::HandleClientMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib,OBuffer4k & ob) //处理客户端到达的消息
{

	switch(nCmd)
	{
	case enChatSrvCmd_CS_Enter:  //登陆请求
		{
			HandleLoginReq(socketid,nCmd,ib,ob);
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

void ChatServer::HandleGameServerMsg(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob) //处理游戏服务器到达的消息
{
	switch(nCmd)
	{
	case enServerCmd_SS_LoginChatSrv:  //登陆请求
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


//游戏服登陆
void ChatServer::HandleGameServerLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob)
{
	SS_Login_ChatSrv_Req Req;
	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	TRACE("游戏服务器:%s 注册成功!",Req.szServerName);

	GameServerProxy * pGameServerProxy = GetGameServerProxy(Req.ServerID);

	if(0==pGameServerProxy)
	{
		pGameServerProxy = new GameServerProxy(Req.ServerID,m_pSocketSystem,Req.szServerName);
		m_mapServer[Req.ServerID] = pGameServerProxy; 
	}

	pGameServerProxy->SetSockID(socketid);

	m_pSocketSystem->SetSocketSink(socketid,pGameServerProxy);
	m_pSocketSystem->SetKeepLiveTime(socketid,g_ConfigParam.m_ServerKeepLiveTime);
}

GameServerProxy * ChatServer::GetGameServerProxy(TServerID ServerID)
{
	std::hash_map<UINT16,GameServerProxy*>::iterator it = m_mapServer.find(ServerID);

	if(it == m_mapServer.end())
	{
		return 0;
	}

	return it->second;
}


//游戏服断开连接
void ChatServer::OnGameServerClose(TServerID ServerID)
{	
	//移除	

}


//具体的消息请求

//登陆请求
void  ChatServer::HandleLoginReq(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob)
{

	CS_EnterChatSrv_Req Req;

	ib >>  Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d 接收到的登陆请求数据长度有误，len = %u,remote_ip=%u!",__FUNCTION__,__LINE__,
			ob.Size(),m_pSocketSystem->GetRemote(socketid));
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	GameServerProxy * pGameServerProxy = GetGameServerProxy(Req.ServerID);

	SC_EnterChatSrv_Rsp Rsp;
	Rsp.m_Result = enEnterChatSrvRetCode_Ok;

	if(pGameServerProxy==0)
	{
		Rsp.m_Result = enEnterChatSrvRetCode_ServerNotOpen;
	}
	else
	{
		pGameServerProxy->ActorEnter(socketid,Req);
	}

	ob.Reset();

	ob << enChatSrvCmdHeader(enChatSrvCmd_SC_Enter,SIZE_OF(Rsp)) << Rsp;

	m_pSocketSystem->Send(socketid,ob.TakeOsb());

}



