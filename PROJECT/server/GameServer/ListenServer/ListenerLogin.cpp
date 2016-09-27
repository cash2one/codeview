#include "ListenerLogin.h"
#include "BclHeader.h"
#include "stdarg.h"
#include "ILogTrace.h"
#include "RandomService.h"
#include "FileConfig.h"
#include "ICryptService.h"
#include "ListenerGame.h"
#include "FileCsv.h"
#include "windows.h"
#include  <process.h>
#import "jmail.dll"
using namespace jmail;

//全局事件句柄，用于线程控制
HANDLE   g_hEvent;

std::vector<Mail_Data>  g_vectMail;

ListenerLogin::ListenerLogin()
{
	
}

ListenerLogin::~ListenerLogin()
{
	CloseHandle(g_hEvent);

	std::map<TServerID, ListenerGame *>::iterator iter = m_mapListenerGame.begin();

	for ( ; iter != m_mapListenerGame.end(); ++iter )
	{
		ListenerGame * pGame = iter->second;

		delete pGame;
	}
}

ListenerLogin * ListenerLogin::GetInstance()
{
	static ListenerLogin Listener;

	return &Listener;
}

unsigned   __stdcall   SendMail(   void*   pArguments   )
{  
	const SConfigParam & ConfigParam = ListenerLogin::GetInstance()->GetConfigParam();

	try
	{
		//   COM的初始化 
		::CoInitialize(NULL);

		jmail::IMessagePtr pMessage("JMail.Message");
		
		// 发件人邮箱
		pMessage->From = ConfigParam.m_MailAddress.c_str();

		pMessage->MailServerUserName = ConfigParam.m_MailAddress.c_str();

		pMessage->MailServerPassWord = ConfigParam.m_MailPassword.c_str();
		
		// 发件人姓名
		pMessage->FromName = "服务器监控";
		
		// 优先级设置,1-5逐次降低, 3为中级
		pMessage->Priority = 3;
		
		// 编码方式设置, 默认是iso-8859-1
		pMessage->Charset = "GB2312";
		
		while(true)
		{
			WaitForSingleObject(g_hEvent, INFINITE);

			// 添加收件人
			pMessage->AddRecipient(ConfigParam.m_MailDesAddress.c_str(), "服务器监控", "");

			for( int i = 0; i < g_vectMail.size(); ++i)
			{
				// 主题
				pMessage->Subject = g_vectMail[i].szTheme;
				
				// 正文
				pMessage->Body = g_vectMail[i].szBody;
				
				// 开始发送
				pMessage->Send(ConfigParam.m_MailServer.c_str(),VARIANT_FALSE);

				TRACE("发送客服邮件成功：%s!", ConfigParam.m_MailDesAddress.c_str());
			}

			g_vectMail.clear();

			//重新设为未重置状态
			ResetEvent(g_hEvent);
		}

		pMessage.Release();

	}
	catch (_com_error e)
	{
		TRACE("发送客服邮件失败：%s", ConfigParam.m_MailServer.c_str());
	}

	CoUninitialize();

    return   0;   
}

bool ListenerLogin::Init(const char* szFileName)
{
	m_pBasicService = ::GetBasicService();

	if( 0 == m_pBasicService || false == m_pBasicService->Create()){
		return false;
	}

	ILogTrace * pLogTrace = m_pBasicService->GetLogTrace();
	if( false == pLogTrace->Create("./ListenServer.log")){
		return false;
	}

	FileConfig File;
	if( !File.Open(szFileName,"ListenServer")){
		TRACE("<error> %s : %d 打开配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	if( !File.Read(m_ConfigParam)){
		TRACE("<error> %s : %d 读取配置参数失败!表名：%s",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	//获取服务器列表
	FileCSV FileCS;
	if(FileCS.Open("GameServerList.csv")==false)
	{
		TRACE("打开配置文件[%s]失败!", "GameServerList.csv");
		return false;
	}

	std::vector<GameServerCnfg> vectGameCnfg;

	if( FileCS.GetTable(vectGameCnfg) == false)
	{
		TRACE("<error> %s : %d Line 读取配置参数失败！！表名：%s", __FUNCTION__, __LINE__, "GameServerList.csv");
		return false;
	}

	for ( int i = 0; i < vectGameCnfg.size(); ++i )
	{
		GameServerCnfg & GameCnfg = vectGameCnfg[i];

		GameServerInfo Info;

		Info.m_bInServerList = false;
		Info.m_ServerID = GameCnfg.m_ServerID;
		strncpy(Info.m_szServerName, GameCnfg.m_szServerName, sizeof(Info.m_szServerName));

		m_mapGameInfo[Info.m_ServerID] = Info;

		ListenerGame *pGame = new ListenerGame;

		if ( !pGame->Create(Info.m_ServerID, Info.m_szServerName) )
		{
			TRACE("<error> %s : %d Line 创建游戏服务器监听器失败！！服务器ID:%d,名称:%s", __FUNCTION__, __LINE__, Info.m_ServerID, Info.m_szServerName);
			delete pGame;
			continue;
		}

		m_mapListenerGame[Info.m_ServerID] = pGame;
	}

	//创建定时器
	m_pTimeAxis = m_pBasicService->GetTimeAxis();
	if( false == m_pTimeAxis->Create(m_pBasicService->GetIoService())){
		TRACE("<error> %s : %d创建时间轴库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	//创建通讯库
	m_pSocketSystem = CreateSocketSystem();
	if( NULL == m_pSocketSystem){
		TRACE("<error> %s : %d创建通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	if( false == m_pSocketSystem->Start(m_pBasicService->GetIoService(), m_ConfigParam.m_MaxConnNum, m_ConfigParam.m_NetIoThreadNum, m_ConfigParam.m_KeepLiveTime)){
		TRACE("<error> %s : %d 初始化通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	//先检测一次
	this->CheckServer();

	//设置定时器，监听服务器
	m_pTimeAxis->SetTimer(enListenerTimer_Control, this, m_ConfigParam.m_ListenTimeSpace * 1000, "ListenerLogin::Init");

	//创建人工事件对象
	g_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE hThread;

	//创建发邮件线程
	hThread = (HANDLE)_beginthreadex(   NULL,   0,   &SendMail,   0,   0,   0);

	CloseHandle(hThread);

	return true;
}

//对登录服和游戏服做检测
void	ListenerLogin::CheckServer()
{
	m_SockID = m_pSocketSystem->Connect(m_ConfigParam.m_szLoginSrvIp,m_ConfigParam.m_LoginSrvPort,this,enPacketHeaderType_Len, m_ConfigParam.m_KeepLiveTime);

	if( m_SockID.IsInvalid())
	{
		TRACE("<error> %s : %d 通讯库连接登录服失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)m_ConfigParam.m_szLoginSrvIp,m_ConfigParam.m_LoginSrvPort);
	    this->SendMailNoticeNoOpen(0, true, true);
		return;
	}
}

//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void ListenerLogin::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	TRACE("连接登录服成功!");

	//登录
	CS_Login_Req Req;

	Req.ChanelType = (enChanelType)7;
	strncpy(Req.ClientVersion, m_ConfigParam.m_ClientVersion, sizeof(Req.ClientVersion));
	strncpy(Req.szAccessSecret, "", sizeof(Req.szAccessSecret));
	strncpy(Req.szPassword, this->GetCryptService()->CalculateMD5((const UINT8 *)((const char *)m_ConfigParam.m_UserPassword), sizeof(m_ConfigParam.m_UserPassword)).toString().c_str(), sizeof(Req.szPassword));
	strncpy(Req.szUserName, m_ConfigParam.m_UserName, sizeof(Req.szUserName));

	OBuffer1k ob;
	ob << LoginHeader(enLoginCmd_CS_Login, sizeof(Req)) << Req;
	m_pSocketSystem->Send(socketid, ob.TakeOsb());
}

//数据到达,
void ListenerLogin::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	OBuffer4k ob(osb);
	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader Header;
	ib >> Header;

	switch( Header.command)
	{
	case enLoginCmd_SC_LoginSuccess:
		{
			TRACE("玩家登录成功！！");

			SC_Login_Success_Rsp Rsp;

			ib >> Rsp;

			if ( ib.Error() )
			{
				TRACE("<error> %s : %d Line 包长度有误", __FUNCTION__, __LINE__);
				break;
			}

			m_UserID = Rsp.UserID;

			memcpy(m_szTicket, Rsp.szTicket,sizeof(m_szTicket));
		}
		break;
	case enLoginCmd_SC_LoginFaile:
		{
			TRACE("玩家登录失败！！");
		}
		break;
	case enLoginCmd_SC_ServerList:
		{
			SC_Server_List_Syn Req;

			ib >> Req;

			for ( int i = 0; i < Req.ServerNum; ++i )
			{
				SGameServerInfo Info;

				ib >> Info;

				TRACE("服务器%d : %s", i, Info.szServerName);

				std::map<TServerID,GameServerInfo>::iterator iter = m_mapGameInfo.find(Info.ServerID);

				if ( iter != m_mapGameInfo.end() )
				{
					GameServerInfo & GameInfo = iter->second;

					GameInfo.m_bInServerList = true;
				}

				std::map<TServerID, ListenerGame *>::iterator itGame = m_mapListenerGame.find(Info.ServerID);

				if ( itGame == m_mapListenerGame.end() )
				{
					TRACE("警告！！！！ 该服没开启监控！！服务器ID:%d", Info.ServerID);
					continue;
				}

				ListenerGame *pGame = itGame->second;

				if ( 0 == pGame )
				{
					TRACE("<error> %s : %d Line  创建游戏服监控器失败！！服务器ID:%d", __FUNCTION__, __LINE__, Info.ServerID);
					continue;
				}

				if ( !pGame->Reset(Info.ServerIp, Info.ServerPort, m_UserID, m_szTicket) )
				{
					TRACE("<error> %s : %d Line 创建游戏服务器监听器失败！！服务器ID:%d,名称:%s,IP:%s,端口:%s", __FUNCTION__, __LINE__, Info.ServerID, Info.szServerName, Info.ServerIp, Info.ServerPort);
					delete pGame;
					continue;
				}

				m_vecListenGame.push_back(pGame);
			}

			//不在服务器列表的 表示服务器挂掉了
			std::map<TServerID,GameServerInfo>::iterator it = m_mapGameInfo.begin();

			for ( ; it != m_mapGameInfo.end(); ++it )
			{
				GameServerInfo & GameInfo = it->second;

				if ( !GameInfo.m_bInServerList )
				{
					TRACE("警告！！！！服务器%s未开启",GameInfo.m_szServerName);
					this->SendMailNoticeNoOpen(GameInfo.m_ServerID, true, false);
					continue;
				}

				GameInfo.m_bInServerList = false;
			}
		}
		break;
	case enLoginCmd_SC_Finish:
		{
			TRACE("登录结束！！");
		}
		break;
	}
}

//连接关闭
void ListenerLogin::OnClose(TSockID sockid)
{
}

IBasicService  *		ListenerLogin::GetBasicServer()
{
	return m_pBasicService;
}

ICryptService * ListenerLogin::GetCryptService()
{
	if(m_pBasicService)
	{
		return m_pBasicService->GetCryptService();
	}

	return 0;
}

ISocketSystem  *		ListenerLogin::GetSocketSystem()
{
	return m_pSocketSystem;
}

const SConfigParam &	ListenerLogin::GetConfigParam()
{
	return m_ConfigParam;
}

//通知网管服务器没开启
void					ListenerLogin::SendMailNoticeNoOpen(TServerID ServerID, bool bNoOpen, bool bLogin)
{
	Mail_Data KFData;

	if ( bLogin )
	{
		//登录服
		strncpy(KFData.szTheme, "登录服未开启", sizeof(KFData.szTheme));
		strncpy(KFData.szBody, "登录服未开启！！！", sizeof(KFData.szBody));
	}
	else
	{
		//游戏服
		std::map<TServerID,GameServerInfo>::iterator iter = m_mapGameInfo.find(ServerID);

		if ( iter == m_mapGameInfo.end() )
		{
			TRACE("<error> %s : %d 配置中找不到该服务器的配置！！服务器ID = %d", __FUNCTION__, __LINE__, ServerID);
		}

		if ( bNoOpen )
		{
			strncpy(KFData.szTheme, "服务器未开启", sizeof(KFData.szTheme));

			if ( iter != m_mapGameInfo.end() )
			{
				sprintf_s(KFData.szBody, sizeof(KFData.szBody), "服务器未开启！！！服务器ID:%d,服务器名字:%s", ServerID, (iter->second).m_szServerName);
			}
			else
			{
				sprintf_s(KFData.szBody, sizeof(KFData.szBody), "服务器未开启！！！服务器ID:%d", ServerID);
			}
		}
		else
		{
			strncpy(KFData.szTheme, "服务器连接不上", sizeof(KFData.szTheme));

			if ( iter != m_mapGameInfo.end() )
			{
				sprintf_s(KFData.szBody, sizeof(KFData.szBody), "服务器连接不上！！！服务器ID:%d,服务器名字:%s", ServerID, (iter->second).m_szServerName);
			}
			else
			{
				sprintf_s(KFData.szBody, sizeof(KFData.szBody), "服务器连接不上！！！服务器ID:%d", ServerID);
			}
		}
	}

	g_vectMail.push_back(KFData);

	//触发事件对象
	SetEvent(g_hEvent);
}

void		ListenerLogin::OnTimer(UINT32 timerID)
{
	if ( enListenerTimer_Control == timerID )
	{
		//开始检测
		this->CheckServer();
	}
}
