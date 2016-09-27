#include "TestClient.h"
#include "BclHeader.h"
#include "stdarg.h"
#include "ILogTrace.h"
#include "FileConfig.h"
#include "RandomService.h"
#include "LoginSrvProtocol.h"
#include "Client.h"
#include "windows.h"
#include "ICryptService.h"

TestClient::TestClient()
{
	m_SockID = TSockID();

	m_ClientNum = 0;

	m_pSocketSystem = 0;

	m_pBasicService = 0;
	m_pTimeAxis		= 0;
	m_pSignals_		= 0;
}

TestClient::~TestClient()
{
	std::vector<Client *>::iterator iter = m_vectClient.begin();

	while( iter != m_vectClient.end()){
		delete *iter;
		++iter;
	}
}

TestClient * TestClient::GetInstance()
{
	static TestClient __Instance;

	return &__Instance;
}

bool TestClient::Init(const char* szFileName, int nCLientNum)
{
	m_ClientNum		= nCLientNum;

	m_pBasicService = ::GetBasicService();

	if( 0 == m_pBasicService || false == m_pBasicService->Create()){
		return false;
	}

	ILogTrace * pLogTrace = m_pBasicService->GetLogTrace();
	if( false == pLogTrace->Create("./TestClient.log")){
		return false;
	}

	FileConfig File;
	if( !File.Open(szFileName,"TestClient")){
		TRACE("<error> %s : %d 打开配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	if( !File.Read(m_ConfigParam)){
		TRACE("<error> %s : %d 读取配置参数失败!",__FUNCTION__,__LINE__);
		return false;
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

	//连接登录服
	m_SockID = m_pSocketSystem->Connect(m_ConfigParam.m_szLoginSrvIp,m_ConfigParam.m_LoginSrvPort,this,enPacketHeaderType_Len, m_ConfigParam.m_KeepLiveTime);
	if( m_SockID.IsInvalid()){
		TRACE("<error> %s : %d 通讯库连接登录服失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)m_ConfigParam.m_szLoginSrvIp,m_ConfigParam.m_LoginSrvPort);
		return false;
	}

	return true;
}

//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void TestClient::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	TRACE("连接登录服成功!");

	//注册m_ClientNum个玩家
	CS_Register_Req Req;
	//Req.ClientVersion = 1;
	strncpy(Req.ClientVersion, "1.0.2", sizeof(Req.ClientVersion));
	strncpy(Req.szPassword, this->GetCryptService()->CalculateMD5((const UINT8 *)((const char *)"123"), 4).toString().c_str(), sizeof(Req.szPassword));
	//strncpy(Req.szPassword, "123", sizeof(Req.szPassword));
	
	OBuffer1k ob;

	for( int i = 0; i < m_ClientNum; ++i)
	{
		this->RandomGetName(Req.szUserName, sizeof(Req.szUserName));
		
		ob.Reset();
		ob << LoginHeader(enLoginCmd_CS_Register, sizeof(Req)) << Req;
		m_pSocketSystem->Send(socketid, ob.TakeOsb());
	}
}

//数据到达,
void TestClient::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	static int nFaileClientNum = 0;

	static int nSucceessClientNum = 0;

	OBuffer4k ob(osb);
	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader Header;
	ib >> Header;

	static int loginNum = 0;

	switch( Header.command)
	{
	case enLoginCmd_SC_RegisterSuccess:
		{
			++nSucceessClientNum;

			SC_Register_Success_Rsp Rsp;
			ib >> Rsp;
			TRACE("注册成功,用户ID:%d", Rsp.UserID);

			//创建一个模拟客户端
			Client * pClient = new Client(Rsp.UserID, Rsp.szTicket);

			pClient->EnterGame(m_ConfigParam.m_szGameSrvIp,m_ConfigParam.m_GameSrvPort);
			m_vectClient.push_back(pClient);			
		}
		break;
	case enLoginCmd_SC_RegisterFaile:
		{		
			++nFaileClientNum;
			SC_Register_Faile_Rsp Rsp;
			ib >> Rsp;
			TRACE("注册失败,注册返回码%d", Rsp.m_Result);
		}
		break;
	case enLoginCmd_SC_ServerList:
		{		
				
		}
		break;
	case enLoginCmd_SC_Finish:
		{
			TRACE("第%d名用户登陆成功!",nSucceessClientNum);
		}
		break;
	}
}

//连接关闭
void TestClient::OnClose(TSockID sockid)
{
	TRACE("连接关闭OK");
}

ISocketSystem  * TestClient::GetSocketSystem()
{
	return m_pSocketSystem;
}

IBasicService  * TestClient::GetBasicServer()
{
	return m_pBasicService;
}

ITimeAxis      * TestClient::GetTimeAxis()
{
	return m_pTimeAxis;
}

const SConfigParam   * TestClient::GetConfigParam()
{
	return &m_ConfigParam;
}

ICryptService * TestClient::GetCryptService()
{
	if(m_pBasicService)
	{
		return m_pBasicService->GetCryptService();
	}

	return 0;
}

//随机获取一个10字节名字
void  TestClient::RandomGetName(char * szName, int nLen)
{
	if( 10 > nLen){
		return;
	}

	//97到122是ASCII的字母对应码
	int nRandom = 0;
	
	for( int i = 0; i < 10; ++i,++szName){
		nRandom = RandomService::GetRandom() % 26;
		*szName = nRandom + 97;
	}

	*szName = '\0';
}

//进入游戏
void  TestClient::__EnterGame()
{

}
