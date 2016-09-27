#include "FriendMgr.h"
#include "ThingServer.h"
#include "DBProtocol.h"
#include "GameSrvProtocol.h"
#include "IActor.h"
#include "IBasicService.h"
#include "ISession.h"
#include "IFriendPart.h"
#include "IGameWorld.h"
#include "IEquipPart.h"
#include "ICombatPart.h"

FriendMgr::FriendMgr(void)
{
}

FriendMgr::~FriendMgr(void)
{
}

bool FriendMgr::Create(void)
{
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Relation,this);
}

void FriendMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Relation,this);
}

//收到MSG_ROOT消息
void FriendMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (FriendMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enFriendCmd_Max] = 
	{
		&FriendMgr::AddFriend,
		&FriendMgr::DeleteFriend,
		&FriendMgr::ViewFriend,
		&FriendMgr::ViewFriendEnvent,
		&FriendMgr::ViewFriendMsg,
		&FriendMgr::ViewOneCityOnlineUser,
		&FriendMgr::VisitFriend,
		&FriendMgr::LeaveVisitFriend,
		&FriendMgr::ViewUserInfo,
		&FriendMgr::LeaveViewUserInfo,
		&FriendMgr::DeleteFriendMsg,
		
	};

	if(nCmd >= enFriendCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

//添加新好友
void	FriendMgr::AddFriend(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_AddFriend_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端消息长度有误!! nCmd = %d", __FUNCTION__, __LINE__, nCmd);
		return;
	}

	SDB_Get_ActorUIDByName_Req GetActorIDByName_Req;
	strncpy(GetActorIDByName_Req.ActorName, Req.m_szFriendName, sizeof(GetActorIDByName_Req.ActorName));

	OBuffer1k ob;
	ob << GetActorIDByName_Req;

	//先请求，用玩家的名字得到玩家的ID
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetActorUIDByName1, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}


//IDBProxyClientSink接口
void FriendMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	if( ReqCmd == enDBCmd_GetFriendListInfo || ReqCmd == enDBCmd_GetFriendEnventListInfo ||
			ReqCmd == enDBCmd_GetFriendMsgListInfo || ReqCmd == enDBCmd_InsertFriendInfo ||
			ReqCmd == enDBCmd_UpdateFriendInfo)
	{
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
		if( 0 == pActor){
			return;
		}
		IFriendPart * pFriendPart = pActor->GetFriendPart();
		if( 0 == pFriendPart){
			return;
		}

		pFriendPart->OnDBRet(userID, ReqCmd, nRetCode, RspOsb, ReqOsb, userdata);

		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_GetActorUIDByName1:
			{
				HandleAddFriend(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   case enDBCmd_GetActorUIDByName2:
		   {
			   HandleDeleteFriend(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}
}

void	FriendMgr::OnEvent(XEventData & EventData)
{
	//创建玩家的消息ID
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	if( EventData.m_MsgID == msgID)
	{
		SS_ActoreCreateContext * ActoreCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == ActoreCreateContext){
			return;
		}

		MAP_VIEWUSERINFO::iterator iter = m_mapViewUserInfo.find(UID(ActoreCreateContext->m_uidActor));
		if( iter == m_mapViewUserInfo.end()){
			//不是要关注的玩家
			return;
		}

		IActor * pRecvActor = g_pGameServer->GetGameWorld()->FindActor(iter->second);

		IActor * pSendActor = g_pGameServer->GetGameWorld()->FindActor(iter->first);

		if( 0 != pRecvActor || 0 != pSendActor){
			this->SendUserInfo(pRecvActor, pSendActor);
		}

		m_mapViewUserInfo.erase(iter);
	}
}

///////
void	FriendMgr::HandleAddFriend(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ActorUIDByName_Rsp       ActorIDByName_Rsp;
	RspIb >> RspHeader >> OutParam >> ActorIDByName_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK)
	{
		SC_FriendResult_Rsp Rsp;
		Rsp.m_Result = enFriendRetCode_NoExistUser;

		OBuffer1k ob;
		ob << FriendHeader(enFriendCmd_AddFriend, sizeof(SC_FriendResult_Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	pFriendPart->AddFriend(UID(ActorIDByName_Rsp.Uid_Actor));
}


//删除好友
void	FriendMgr::DeleteFriend(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_DeleteFriend_Req Req;

	ib >> Req;

	IFriendPart * pFriendPart = pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->DeleteFriend(UID(Req.m_uidFriend));
	//SDB_Get_ActorUIDByName_Req GetActorIDByName_Req;
	//strncpy(GetActorIDByName_Req.ActorName, Req.m_szFriendName, sizeof(GetActorIDByName_Req.ActorName));

	//OBuffer1k ob;
	//ob << GetActorIDByName_Req;

	////先请求，用玩家的名字得到玩家的ID
	//g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetActorUIDByName2, ob.TakeOsb(), this, pActor->GetUID().m_uid);	
}

//
void	FriendMgr::HandleDeleteFriend(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ActorUIDByName_Rsp       ActorIDByName_Rsp;
	RspIb >> RspHeader >> OutParam >> ActorIDByName_Rsp;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}

	if(nRetCode != enDBRetCode_OK)
	{
		return;
	}
	else if(OutParam.retCode != enDBRetCode_OK)
	{
		return;
	}

	pFriendPart->DeleteFriend(UID(ActorIDByName_Rsp.Uid_Actor));
}

//查看好友
void	 FriendMgr::ViewFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 != pFriendPart){
		pFriendPart->ViewFriend();
	}
}

//查看好友度改变事件
void	FriendMgr::ViewFriendEnvent(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewFriendEvent_Req Req;
	ib >> Req;
	if(ib.Error()){
		TRACE("<error> %s : %d 行 获取客户端数据长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 != pFriendPart){
		pFriendPart->ViewFriendEnvent(Req.m_uidFriend);
	}
}

//查看好友信息标签里的信息
void	FriendMgr::ViewFriendMsg(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 != pFriendPart){
		pFriendPart->ViewFriendMsg();
	}
}

//查看同城的在线玩家
void	FriendMgr::ViewOneCityOnlineUser(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 != pFriendPart){
		pFriendPart->ViewOneCityOnlineUser();
	}
}

//拜访好友
void	FriendMgr::VisitFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_VisitFriend_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 拜访好友客户端参数错误", __FUNCTION__, __LINE__);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 != pFriendPart){
		pFriendPart->VisitFriend(Req.m_uidFriend);
	}
}

//查看人物信息
void	FriendMgr::ViewUserInfo(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewUserInfo Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 查看人物信息客户端参数错误", __FUNCTION__, __LINE__);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->ViewUserInfo(Req.m_uidUser);

	//IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidUser);
	//if( 0 == pUser){

	//	if( 0 == m_mapViewUserInfo.size()){
	//		//先订阅玩家创建的消息
	//		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	//		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	//		if( 0 == pEnventServer){
	//			return;
	//		}

	//		pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"FriendMgr::ViewUserInfo");	
	//	}

	//	m_mapViewUserInfo[Req.m_uidUser] = pActor->GetUID();

	//	g_pGameServer->LoadActor(Req.m_uidUser);
	//}else{
	//	this->SendUserInfo(pActor, pUser);
	//}
}

//离开查看其它玩家人物信息界面
void	FriendMgr::LeaveViewUserInfo(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_LeaveViewUserInfo Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 拜访好友客户端参数错误", __FUNCTION__, __LINE__);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->LeaveViewUserInfo(Req.m_uidUser);

	//pActor->RemoveToAttentionUser(Req.m_uidUser);

	//SC_LeaveViewUserInfo Rsp;
	//Rsp.m_Result = enFriendRetCode_OK;

	//OBuffer1k ob;
	//ob << FriendHeader(enFriendCmd_LeaveViewUserInfo, sizeof(SC_LeaveViewUserInfo)) << Rsp;
	//pActor->SendData(ob.TakeOsb());
}

void	FriendMgr::SendUserInfo(IActor * pRecvActor, IActor * pSendActor)
{
	//主角的角色信息和装备栏信息
	pRecvActor->NoticClientCreatePrivateThing(pSendActor->GetUID());

	IEquipPart * pEquipPart = pSendActor->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	pEquipPart->SendEquipPanelData(pRecvActor);

	//招募角色的角色信息和装备栏信息
	for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = pSendActor->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		IEquipPart * pEmpEquipPart = pEmployee->GetEquipPart();
		if( 0 == pEmpEquipPart){
			continue;
		}

		pRecvActor->NoticClientCreatePrivateThing(pEmployee->GetUID());

		//发送装备栏数据
		pEmpEquipPart->SendEquipPanelData(pRecvActor);
	}

	//添加关注
	pRecvActor->AddToAttentionUser(pSendActor->GetUID());

	SC_ViewUserInfo Rsp;

	Rsp.m_Result = enFriendRetCode_OK;
	Rsp.m_uidUser = pSendActor->GetUID();

	ICombatPart * pCombatPart = pSendActor->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}

	std::vector<SCreatureLineupInfo> vectLineup =  pCombatPart->GetJoinBattleActor();

	for( int i = 0; i < vectLineup.size(); ++i)
	{
		ICreature * m_pCreature = vectLineup[i].m_pCreature;
		if( 0 == m_pCreature){
			continue;
		}

		Rsp.m_uidLineup[i] = m_pCreature->GetUID();
	}

	OBuffer1k ob;
	ob << FriendHeader(enFriendCmd_ViewUserInfo, sizeof(SC_ViewUserInfo)) << Rsp;
	pRecvActor->SendData(ob.TakeOsb());
}

//离开拜访好友界面
void	FriendMgr::LeaveVisitFriend(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	
}

//删除好友信息
void	FriendMgr::DeleteFriendMsg(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_DeleteFriendMsg Rsp;

	ib >> Rsp;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 拜访好友客户端参数错误", __FUNCTION__, __LINE__);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();

	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->DeleteFriendMsg(Rsp.m_uidUser);
}

