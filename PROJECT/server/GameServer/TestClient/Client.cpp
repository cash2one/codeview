#include "Client.h"
#include "string.h"
#include "windows.h"
#include "GameSrvProtocol.h"
#include "CmdPacket.h"
#include "CmdShopMall.h"
#include "RandomService.h"
#include "CmdTalk.h"
#include "CmdEquip.h"
#include "FuMoDong.h"
#include "IThing.h"


Client::Client(TUserID UserID, const char * pszTicket)
{
	m_UserID = UserID;

	memset(m_szTicket, 0, sizeof(m_szTicket));

	strncpy(m_szTicket, pszTicket, sizeof(m_szTicket));

	m_bWaitTimer = false;

	m_bFirst = true;

	this->Init();
}

Client::~Client()
{
	TestClient::GetInstance()->GetTimeAxis()->KillTimer(enTimerID_Msg, this);

	std::hash_map<UINT8/*服消息分类*/, ICmdBase *>::iterator iter = m_mapCmd.begin();

	while( iter != m_mapCmd.end()){
		delete iter->second;
		++iter;
	}
}

bool Client::Init()
{
	ICmdBase * pCmdBase = 0;

	//背包
	 pCmdBase = new CmdPacket(this);
	m_mapCmd[(UINT8)enMsgCategory_Packet] = pCmdBase;

    //商城
	pCmdBase = new CmdShopMall(this);
	m_mapCmd[(UINT8)enMsgCategory_ShopMall] = pCmdBase;

	//聊天
	pCmdBase = new CmdTalk(this);
	m_mapCmd[(UINT8)enMsgCategory_Talk] = pCmdBase;

	//装备
	pCmdBase = new CmdEquip(this);
	m_mapCmd[(UINT8)enMsgCategory_Equip] = pCmdBase;

	//挑战伏魔洞
	pCmdBase = new CmdFuMoDong(this);
	m_mapCmd[(UINT8)enMsgCategory_Building] = pCmdBase;

	return true;
}

//进入游戏
void Client::EnterGame(const char* szIP,UINT16 port)
{
	m_SockID = TestClient::GetInstance()->GetSocketSystem()->Connect(szIP, port, this, enPacketHeaderType_Len,TestClient::GetInstance()->GetConfigParam()->m_KeepLiveTime);

	if( m_SockID.IsInvalid()){
		TRACE("<error> %s : %d 通讯库连接服务器失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)szIP, port);
	}
}

//是否已进入游戏服
bool Client::IsLogin()
{
	return m_SockID.IsInvalid();
}

//发送数据
bool Client::SendData(OStreamBuffer & osb)
{
	return TestClient::GetInstance()->GetSocketSystem()->Send(m_SockID, osb);
}

//随机执行命令
void Client::RandomExecutionCmd()
{
	if( 0 == m_mapCmd.size())
	{
		return;
	}

	if ( m_bFirst )
	{
		//第一条命令用来执行GM命令
		m_bFirst = false;

		std::hash_map<UINT8/*服消息分类*/, ICmdBase *>::iterator it = m_mapCmd.find((UINT8)enMsgCategory_Talk);

		if ( it == m_mapCmd.end() )
			return;

		ICmdBase * pCmdBase = it->second;

		if ( 0 == pCmdBase )
			return;

		pCmdBase->SendCmd();
		return;
	}

	std::hash_map<UINT8/*服消息分类*/, ICmdBase *>::iterator iter = m_mapCmd.begin();

	int nRandom = RandomService::GetRandom() % m_mapCmd.size();

	for( int i = 0; i < nRandom; ++i){
		++iter;
	}

	if( iter == m_mapCmd.end()){
		return;
	}

	ICmdBase * pCmdBase = iter->second;
	if( 0 == pCmdBase){
		return;
	}

	pCmdBase->SendCmd();
}

void Client::OnTimer(UINT32 timerID)
{
	this->RandomExecutionCmd();
}

//获得玩家的UID值
const UID & Client::GetUserUID()
{
	return m_uidUser;
}

//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void Client::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
	TRACE("连接游戏服成功!!");

	//进入游戏服
	CS_CreateActor Req;
	Req.cityID = 11;
	Req.facade = 1;
	Req.sex    = 1;
	TestClient::GetInstance()->RandomGetName(Req.szName, sizeof(Req.szName));
	Req.UserID = m_UserID;

	OBuffer1k ob;
	ob << GameFrameHeader(enGameFrameCmd_CS_CreateActor, sizeof(Req)) << Req;
	TestClient::GetInstance()->GetSocketSystem()->Send(socketid, ob.TakeOsb());
}

//数据到达,
void Client::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	OBuffer4k ob(osb);
	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader Header;
	ib >> Header;

	//消息分发
	std::hash_map<UINT8/*服消息分类*/, ICmdBase *>::iterator iter = m_mapCmd.find((UINT8)Header.MsgCategory);

	if( iter == m_mapCmd.end()){

		if( Header.MsgCategory == enMsgCategory_GameFrame){
			//进游戏服消息特别处理，因为只进一次
			this->__HandleEneterGame(Header.command, ib);
		}else if( Header.MsgCategory == enMsgCategory_GameWorld){
			//游戏世界的消息操作
			this->__HandleGameWorld(Header.command, ib);
		}
		//不处理的消息,返回
		return;
	}

	(iter->second)->OnNetDataRecv(Header.command, ib);
}

//连接关闭
void Client::OnClose(TSockID sockid)
{
	TRACE("连接关闭!!!!!");

	m_SockID = TSockID();
}

//进游戏服消息操作
void Client::__HandleEneterGame(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enGameFrameCmd_SC_Enter:
		{
			SC_EnterGame_Rsp Rsp;
			ib >> Rsp;
			if( Rsp.Result == enEnterGameRetCode_Ok){
				TRACE("成功进入服务器!!");
				this->__Start();
			}
		}
		break;
	}
}

//游戏世界的消息操作
void Client::__HandleGameWorld(UINT8 nCmd, IBuffer & ib)
{
	switch(nCmd)
	{
	case enGameWorldCmd_SC_Private_Context:
		{
			SC_CreateThing RspThing;
			ib >> RspThing;

			if( ib.Error()){
				TRACE("<error> %s:%d商城消息读取错误,错误命令ID:%d", __FUNCTION__, __LINE__, nCmd);
			}

			if(RspThing.enThingClass == enThing_Class_Actor)
			{

				SActorPrivateData Data;
				ib >> Data;

				if( ib.Error()){
					TRACE("<error> %s:%d商城消息读取错误,错误命令ID:%d", __FUNCTION__, __LINE__, nCmd);
				}

				if( !Data.m_uidMaster.IsValid()){
					//获得玩家的UID
					m_uidUser = Data.m_uid;
				}
			}
		}
		break;
	}
}

//开始
void Client::__Start()
{
	//进入服务器后,设置定时器,定时发送消息
	UINT32 nTimeNum = TestClient::GetInstance()->GetConfigParam()->m_TimeNumSendMsg;
	TestClient::GetInstance()->GetTimeAxis()->SetTimer(enTimerID_Msg, this, nTimeNum, "Client::__Start");
}
