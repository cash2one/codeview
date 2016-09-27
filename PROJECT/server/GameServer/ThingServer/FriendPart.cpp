#include "DBProtocol.h"
#include "IDBProxyClient.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IActor.h"
#include "FriendPart.h"
#include "IGameWorld.h"
#include "IBasicService.h"
#include "time.h"
#include "ISyndicate.h"
#include "ISyndicateMgr.h"
#include "DMsgSubAction.h"
#include "IEventServer.h"
#include "ViewOneCityUser.h"
#include "ThingContainer.h"
#include "ICombatPart.h"
#include "DMsgSubAction.h"
#include "IGameScene.h"
#include "MailCmd.h"
#include "IEquipPart.h"
#include "IResOutputPart.h"
#include "ISession.h"


FriendPart::FriendPart(void)
{
	m_pActor = 0;

	m_bLoadData				 = false;	//一开始不初始化好友数据，要用到时再初始化

	m_FriendInitRelationNum  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_FriendInitRelationNum;
	m_MaxFriendNum			 = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxFriendNum;
	m_MaxFriendMsgNum		 = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxFriendMsgNum;
	m_FriendEnventRecordTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_FriendEnventRecordTime;

	m_uidVisitFriend = UID();

	m_uidViewUserInfo = UID();
}

FriendPart::~FriendPart(void)
{
}

bool FriendPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(0 == pMaster || pMaster->GetThingClass() != enThing_Class_Actor){
		return false;
	}

	m_pActor = (IActor*)pMaster;

	this->LoadFriendInfo();

	return true;
}

//释放
void FriendPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart FriendPart::GetPartID(void)
{
	return enThingPart_Actor_Friend;
}

//取得本身生物
IThing*		FriendPart::GetMaster(void)
{
	return (IThing *)m_pActor;
}

//玩家下线了，需要关闭该ThingPart
void		FriendPart::Close()
{
}

//保存数据
void		FriendPart::SaveData()
{
}

bool		FriendPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

void		FriendPart::InitPrivateClient()
{
	if( m_uidVisitFriend.IsValid()){

		IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(m_uidVisitFriend);

		m_uidVisitFriend = UID();

		m_pActor->RemoveToAttentionUser(m_uidVisitFriend);

		if (pFriend){
			
			//如果正在拜访好友，得从好友场景退出，不然会挂
			TSceneID SceneID = pFriend->GetCrtProp(enCrtProp_ActorMainSceneID);

			IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
			if( 0 == pGameScene){
				TRACE("<error> %s : %d 行 得不到好友的主场景！！", __FUNCTION__, __LINE__);
				return;
			}

			pGameScene->LeaveScene(m_pActor);
		}
	}

	if( m_uidViewUserInfo.IsValid()){
		
		m_pActor->RemoveToAttentionUser(m_uidViewUserInfo);

		m_uidViewUserInfo = UID();
	}

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->RemoveAllListener(this);	
}

void		FriendPart::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__, __LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
    case enDBCmd_GetFriendListInfo:
	case enDBCmd_GetFriendEnventListInfo:
	case enDBCmd_GetFriendMsgListInfo:
	    {
			HandleLoadFriendInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
	    }
	    break;
	case enDBCmd_InsertFriendInfo:
		{
			HandleAddFriendBack(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_UpdateFriendInfo:
		{
			HandleAddRelationNum(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
    default:
	    {
			TRACE("<warning> %s : %d 行 意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",__FUNCTION__, __LINE__,ReqCmd,userID,nRetCode);
	    }
	    break;
	}
}


//加好友
bool FriendPart::AddFriend(const UID & uidFriend)
{
	SC_FriendResult_Rsp Rsp;
	Rsp.m_Result = enFriendRetCode_OK;

	OBuffer1k ob;

	do
	{
		if(m_FriendData.m_mapFriendData.size() >= m_MaxFriendNum){
			//好友列表已满
			Rsp.m_Result = enFriendRetCode_Full;
			break;
		}

		std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);

		if(iter != m_FriendData.m_mapFriendData.end()){
			//好友已经在好友列表中
			Rsp.m_Result = enFriendRetCode_Exist;
			break;
		}

		if( uidFriend == m_pActor->GetUID()){
			//好友已经在好友列表中
			Rsp.m_Result = enFriendRetCode_ErrorAddMySelf;
			break;
		}
	}while(0);		

	if( enFriendRetCode_OK != Rsp.m_Result){
		ob << FriendHeader(enFriendCmd_AddFriend, sizeof(SC_FriendResult_Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());
		return false;
	}

	SDB_Insert_FriendData_Req Req;

	Req.uid_User	= m_pActor->GetUID().m_uid;
	Req.uid_Friend	= uidFriend.m_uid;
	Req.RelationNum	= m_FriendInitRelationNum;

	ob << Req;
	
	//往数据库插入好友数据，如果对方不是你好友则也会在对方好友信息表中插入关于你的记录，如果对方已是你好友，则会删除你的好友信息表中，关于他的记录
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertFriendInfo, ob.TakeOsb(), &g_pThingServer->GetFriendMgr(), m_pActor->GetUID().m_uid);

	return true;
}

//删除好友
bool FriendPart::DeleteFriend(const UID & uidFriend)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);

	if( iter == m_FriendData.m_mapFriendData.end()){
		return false;
	}

	SDB_Delete_FriendData_Req Req;

	Req.UID_User	= m_pActor->GetUID().m_uid;
	Req.UID_Friend	= uidFriend.m_uid;

	OBuffer1k ob;
	ob << Req;

	//在好友列表里找到此玩家
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteFriendInfo, ob.TakeOsb(), NULL, 0);

	m_FriendData.m_mapFriendData.erase(iter);


	SC_DeleteFriend_Rsp Rsp;
	Rsp.m_Result = enFriendRetCode_OK;

	ob.Reset();
	ob << FriendHeader(enFriendCmd_DeleteFriend, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	this->ViewFriend();
	return true;
}

//得到与此玩家的好友度
UINT32	FriendPart::GetRelationNum(const UID & uidFriend)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);

	if( iter == m_FriendData.m_mapFriendData.end()){
		return 0;
	}
	
	SFriendBasicInfo & FriendInfo = iter->second;

	return FriendInfo.m_RelationNum;
}

//增加好友度	szEventDescript1是对于我的事件描述，szEventDescript2是对于好友的事件描述
void	FriendPart::AddRelationNum(const UID & uidFriend, INT32 AddRelationNum, const char * szEventDescript1, const char * szEventDescript2)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);

	if( iter == m_FriendData.m_mapFriendData.end()){
		return;
	}
	
	SFriendBasicInfo & FriendInfo = iter->second;

	FriendInfo.m_RelationNum += AddRelationNum;

	//更新数据库，如果好友也在线则同步好友的好友度
	this->UpdateFriend(uidFriend, AddRelationNum);

	//增加事件描述
	this->AddFriendEnvent(uidFriend, AddRelationNum, szEventDescript1);

	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidFriend);
	if( 0 == pFriend){

		this->__InsertFriendEvent_ToDB(uidFriend, m_pActor->GetUID(), AddRelationNum, szEventDescript2);
		return;
	}

	IFriendPart * pFriendPart = pFriend->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	//增加好友的事件描述
	pFriendPart->AddFriendEnvent(m_pActor->GetUID(), AddRelationNum, szEventDescript2);

	//发布事件
	SS_ChangeFriendRelation ChangeFriendRelation;
	ChangeFriendRelation.m_FriendRelation = FriendInfo.m_RelationNum;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ChangeFriendRelatin);
	m_pActor->OnEvent(msgID,&ChangeFriendRelation,sizeof(ChangeFriendRelation));
}

//更新好友度
void FriendPart::UpdateFriend(const UID & uidFriend, UINT32 nRelationNum)
{
	SDB_Update_FriendData_Req Req;

	Req.uid_User	= m_pActor->GetUID().m_uid;
	Req.uid_Friend	= uidFriend.m_uid;
	Req.RelationNum = nRelationNum;

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateFriendInfo, ob.TakeOsb(), &g_pThingServer->GetFriendMgr(), m_pActor->GetUID().m_uid);
}

//查看好友
void FriendPart::ViewFriend()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SViewFriendListDataRsp Rsp;
	Rsp.m_nFriendNum = m_FriendData.m_mapFriendData.size();

	OBuffer4k ob2;

	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.begin();

	for( ;iter != m_FriendData.m_mapFriendData.end(); ++iter)
	{
		SFriendBasicInfo & FriendBasicInfo = iter->second;

		SFriendListDataRsp FriendListRsp;
		
		FriendListRsp.m_uidFriend = FriendBasicInfo.m_uidFriend;

		IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor((UID)FriendListRsp.m_uidFriend);

		if( 0 != pFriend)
		{
			FriendListRsp.m_Level = pFriend->GetCrtProp(enCrtProp_Level);
			FriendListRsp.m_DuoBaoLevel = pFriend->GetCrtProp(enCrtProp_DuoBaoLevel);
			FriendListRsp.m_CombatAbility = pFriend->GetCrtProp(enCrtProp_ActorCombatAbility);
			FriendListRsp.m_SynWarLevel = pFriend->GetCrtProp(enCrtProp_SynCombatLevel);

			ISession * pSession = pFriend->GetSession();

			if ( 0 != pSession )
			{
				FriendListRsp.m_bOnLine = pFriend->GetSession()->IsOnLine();
			}
		}
		else
		{
			FriendListRsp.m_Level = FriendBasicInfo.m_Level;
			FriendListRsp.m_DuoBaoLevel = FriendBasicInfo.m_DuoBaoLevel;
			FriendListRsp.m_CombatAbility = FriendBasicInfo.m_CombatAbility;
			FriendListRsp.m_SynWarLevel = FriendBasicInfo.m_SynWarLevel;
			FriendListRsp.m_bOnLine = false;
		}

		//FriendListRsp.m_Level = FriendBasicInfo.m_Level;
		FriendListRsp.m_RelationNum	= FriendBasicInfo.m_RelationNum;
		FriendListRsp.m_Sex	  = FriendBasicInfo.m_Sex;
		FriendListRsp.m_enFriendType = FriendBasicInfo.m_enFriendType;
		FriendListRsp.m_bOneCity = FriendBasicInfo.m_bOneCity;
		FriendListRsp.m_Facade = FriendBasicInfo.m_Facade;
		FriendListRsp.m_TitleID = FriendBasicInfo.m_TitleID;
		//FriendListRsp.m_DuoBaoLevel = FriendBasicInfo.m_DuoBaoLevel;
		
		
		
		strncpy(FriendListRsp.m_szFriendName, FriendBasicInfo.m_szFriendName, sizeof(FriendListRsp.m_szFriendName));

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(FriendListRsp.m_uidFriend));
		if( 0 == pSyndicate || pSyndicate->GetSynID() == INVALID_SYN_ID )
		{
			ob2 << FriendListRsp;
			continue;
		}
		else
		{
			FriendListRsp.m_SynID = pSyndicate->GetSynID();
			ob2 << FriendListRsp;		
		}

		SSynInfo SynInfo;
		
		SynInfo.m_SynLevel = pSyndicate->GetSynLevel();
		SynInfo.m_SynMemberNum = pSyndicate->GetSynMemberNum();
		SynInfo.m_SynWarAbility = pSyndicate->GetSynWarAbility();
		strncpy(SynInfo.m_szSynLeaderName, pSyndicate->GetLeaderName(), sizeof(SynInfo.m_szSynLeaderName));
		strncpy(SynInfo.m_szSynName, pSyndicate->GetSynName(), sizeof(SynInfo.m_szSynName));

		ob2 << SynInfo;
	}

	OBuffer4k ob;
	ob << FriendHeader(enFriendCmd_ViewFriend, sizeof(SViewFriendListDataRsp) + ob2.Size()) << Rsp;

	if( ob2.Size() > 0){
		ob << ob2;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//检测是否好友
bool	FriendPart::IsFriend(const UID & uid_Actor)
{
	if(!IsInitFriendData()){
		LoadFriendInfo();
	}

	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uid_Actor);

	if(iter != m_FriendData.m_mapFriendData.end()){
		return true;
	}
	return false;
}

//加载好友信息
void	FriendPart::LoadFriendInfo()
{
	//加载所有好友度数据信息
	SDB_Get_FriendListData_Req Req;
	Req.uid_User = m_pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetFriendListInfo, ob.TakeOsb(), &g_pThingServer->GetFriendMgr(), m_pActor->GetUID().m_uid);

	//加载所有好友度改变事件数据信息,同时会删除超过一天的记录
	SDB_Get_FriendEnventListData_Req ReqFriendEnvent;
	ReqFriendEnvent.uid_User = m_pActor->GetUID().ToUint64();
	ReqFriendEnvent.time	 = time(0);

	ob.Reset();
	ob << ReqFriendEnvent;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetFriendEnventListInfo, ob.TakeOsb(), &g_pThingServer->GetFriendMgr(), m_pActor->GetUID().m_uid);

	//加载好友信息标签数据信息
	SDB_Get_FriendMsgListData_Req	ReqFriendMsg;
	ReqFriendMsg.uid_User	= m_pActor->GetUID().ToUint64();

	ob.Reset();
	ob << ReqFriendMsg;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetFriendMsgListInfo, ob.TakeOsb(), &g_pThingServer->GetFriendMgr(), m_pActor->GetUID().m_uid);

	//把是否加载过数据的标记设为true
	m_bLoadData = true;
}

//增加好友度(包括减少),根据增加方式来决定增加多少好友度
void	FriendPart::AddRelationNum(const UID & uidFriend, const char * szFriendName, enAddRelationNumType AddRelationNumType, INT8 nHours/*需要时间的用，单位：小时*/)
{
	if( !IsFriend(uidFriend)){
		return;
	}

	INT32 nRelationNum = GetRelationNum(uidFriend);

	char szDescriptEnvent[1024];
	char szDescriptFriendEnvent[1024];
	memset(szDescriptEnvent, 0, 1024);
	memset(szDescriptFriendEnvent, 0, 1024);

	INT32 nAddRelation = 0;


	// fly add	20121106

	switch(AddRelationNumType)
	{
	case enHelpStone:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationHelpStone;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10025),szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10026), m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您帮%s收取灵石，好友度+%d",szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s帮您收取灵石，好友度+%d", m_pActor->GetName(), nAddRelation);
		break;
	case enGuardOneHour:
		nAddRelation  = nHours * g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationGuardOneHour;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10027), szFriendName, nHours, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10028), m_pActor->GetName(),nHours, nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您守护%s总共%d小时，好友度+%d", szFriendName, nHours, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s守护您总共%d小时，好友度+%d", m_pActor->GetName(),nHours, nAddRelation);
		break;
	case enXiuLianOneHour:
		nAddRelation  = nHours * g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationXiuLianOneHour;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10029), szFriendName, nHours, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024,g_pGameServer->GetGameWorld()->GetLanguageStr(10030),m_pActor->GetName(), nHours, nAddRelation); 
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您与%s一起修炼%d小时，好友度+%d", szFriendName, nHours, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s与您一起修炼%d小时，好友度+%d",m_pActor->GetName(), nHours, nAddRelation); 
		break;
	case enXiuLianMagic:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationXiuLianMagic;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10031), szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10032),m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您与%s成功修炼法术，好友度+%d", szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%d与您成功修炼法术，好友度+%d",m_pActor->GetName(), nAddRelation);
		break;
	case enJoinFaBaoComplete:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationJoinFaBaoComplete;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10033), szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10034), m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您进入%s的法宝世界，好友度+%d", szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s进入您的法宝世界，好友度+%d", m_pActor->GetName(), nAddRelation);
		break;
	case enStealStone:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationStealStone;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10035), szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10036), m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您窃取%s灵石，好友度%d", szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s窃取您灵石，好友度%d", m_pActor->GetName(), nAddRelation);
		break;
	case enCancelBeGuard:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationCancelBeGuard;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10037), szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10038), m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您取消被%s守护，好友度%d", szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s取消被您守护，好友度%d", m_pActor->GetName(), nAddRelation);
		break;
	case enCancelGuard:
		nAddRelation  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AddRelationCancelGuard;
		nRelationNum += nAddRelation;
		sprintf_s(szDescriptEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10039), szFriendName, nAddRelation);
		sprintf_s(szDescriptFriendEnvent, 1024, g_pGameServer->GetGameWorld()->GetLanguageStr(10040), m_pActor->GetName(), nAddRelation);
		//sprintf_s(szDescriptEnvent, DESCRIPT_LEN_100, "您取消守护%s，好友度%d", szFriendName, nAddRelation);
		//sprintf_s(szDescriptFriendEnvent, DESCRIPT_LEN_100, "%s取消守护您，好友度%d", m_pActor->GetName(), nAddRelation);
		break;
	}
	if(nRelationNum < 0){
		nRelationNum = 0;
	}

	UpdateFriend(uidFriend, nAddRelation);
	
	//事件记录
	this->AddFriendEnvent(uidFriend, nRelationNum, szDescriptEnvent);

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidFriend);
	if( 0 == pActor){
		
		this->__InsertFriendEvent_ToDB(uidFriend, m_pActor->GetUID(), nAddRelation, szDescriptFriendEnvent);
		return;
	}

	IFriendPart * pFriendPart = pActor->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}
	pFriendPart->AddFriendEnvent(m_pActor->GetUID(), nRelationNum, szDescriptFriendEnvent);
}

//检测好友数据是否初始化过
bool	FriendPart::IsInitFriendData()
{
	return m_bLoadData;
}

//把初始化过的标志设成true
void	FriendPart::SetInitTure()
{
	m_bLoadData = true;
}

//从好友信息列表中删除
void	FriendPart::__DeleteFriendMsgList(const UID & uidSrcUser)
{
	std::map<UID, SFriendMsgInfo>::iterator iter = m_FriendData.m_mapFriendMsg.find(uidSrcUser);

	if(iter != m_FriendData.m_mapFriendMsg.end()){
		m_FriendData.m_mapFriendMsg.erase(iter);
	}
}

//访问好友处理流程
bool	FriendPart::BeginVisitFriend(IActor * pFriend)
{
	SC_VisitFriend_Rsp Rsp;
	VisitFriendInfo FriendInfo;
	OBuffer1k ob;

	//拜访好友地图
	TSceneID SceneID = pFriend->GetCrtProp(enCrtProp_ActorMainSceneID);

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
	if( 0 == pGameScene){
		m_uidVisitFriend = UID();
		return false;
	}

	IResOutputPart * pResPart = m_pActor->GetResOutputPart();
	if( 0 == pResPart){
		return false;
	}

	//好友出战角色
	ICombatPart * pCombatPart = pFriend->GetCombatPart();
	if( 0 == pCombatPart){
		m_uidVisitFriend = UID();
		return false;
	}

	FriendInfo.m_uidFriend = pFriend->GetUID();

	std::vector<SCreatureLineupInfo> vectCreature = pCombatPart->GetJoinBattleActor();

	FriendInfo.m_Num = vectCreature.size();

	Rsp.m_TotalCollectToday = pResPart->GetTotalCollectToday();

	Rsp.m_MaxCollectToday = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_BuildingResFilchMax;

	ISyndicate * pSyn = m_pActor->GetSyndicate();
	if( 0 != pSyn){
		Rsp.m_MaxCollectToday = Rsp.m_MaxCollectToday + Rsp.m_MaxCollectToday * pSyn->GetWelfareValue(enWelfare_MaxFilchCollectionOther) / 100.0f + 0.99999;
	}

	ob << FriendHeader(enFriendCmd_VisitFriend, sizeof(SC_VisitFriend_Rsp) + sizeof(VisitFriendInfo) + FriendInfo.m_Num * sizeof(CombatActorInfo)) << Rsp << FriendInfo;

	for( int i = 0; i < vectCreature.size() && i < MAX_COMBAT_ACTOR_NUM; ++i)
	{
		ICreature * pCreature = vectCreature[i].m_pCreature;
		if( 0 == pCreature){
			continue;
		}
		
		if( pCreature->GetThingClass() != enThing_Class_Actor){
			continue;
		}

		IActor * pActor = (IActor *)pCreature;

		CombatActorInfo CbActorInfo;

		CbActorInfo.m_Facade = pActor->GetCrtProp(enCrtProp_ActorFacade);
		CbActorInfo.m_Level	 = pActor->GetCrtProp(enCrtProp_Level);
		CbActorInfo.m_uidActor = pActor->GetUID();
		strncpy(CbActorInfo.m_Name, pActor->GetName(), sizeof(CbActorInfo.m_Name));

		ob << CbActorInfo;
	}
	
	//自己进场景
	if( pGameScene->EnterScene(m_pActor)){

		IResOutputPart * pFriendResPart = pFriend->GetResOutputPart();
		if( 0 == pFriendResPart){
			return false;
		}

		//同步下建筑数据
		pFriendResPart->SendSynResBuildInfo(m_pActor);

		//监听玩家离开场景消息
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

		m_pActor->SubscribeEvent(msgID,this,"FriendPart::BeginVisitFriend");
	}else{
		Rsp.m_Result = enFriendRetCode_ErrEnterScene;

		ob.Reset();
		ob << FriendHeader(enFriendCmd_VisitFriend, sizeof(SC_VisitFriend_Rsp)) << Rsp;
	}

	m_pActor->SendData(ob.TakeOsb());

	if( Rsp.m_Result == enFriendRetCode_OK){
		return true;
	}

	return false;
}

//加载玩家的好友信息
void	FriendPart::HandleLoadFriendInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_GetFriendListInfo:
		{		
			for(int i = 0; i < OutParam.retCode; ++i){
				SDB_Get_FriendListData_Rsp       FriendListData_Rsp;
				RspIb >> FriendListData_Rsp;

				if ( RspIb.Error()){

					TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
					return;
				}

				SFriendBasicInfo FriendData;
				FriendData.m_uidFriend		= FriendListData_Rsp.uid_Friend;
				FriendData.m_bOneCity		= FriendListData_Rsp.bOneCity;
				FriendData.m_enFriendType	= (enFriendType)FriendListData_Rsp.Flag;
				FriendData.m_Level			= FriendListData_Rsp.FriendLevel;
				FriendData.m_RelationNum	= FriendListData_Rsp.RelationNum;
				FriendData.m_Sex			= FriendListData_Rsp.FriendSex;
				FriendData.m_Facade			= FriendListData_Rsp.FriendFacade;
				FriendData.m_TitleID		= FriendListData_Rsp.TitleID;
				FriendData.m_vipLevel		= FriendListData_Rsp.vipLevel;
				FriendData.m_DuoBaoLevel    = FriendListData_Rsp.DuoBaoLevel;
				FriendData.m_CombatAbility  = FriendListData_Rsp.CombatAbility;
				FriendData.m_SynWarLevel	= FriendListData_Rsp.SynWarLevel;
				strncpy(FriendData.m_szFriendName, FriendListData_Rsp.FriendName, sizeof(FriendData.m_szFriendName));
				strncpy(FriendData.m_szSynName, FriendListData_Rsp.SynName, sizeof(FriendData.m_szSynName));

				m_FriendData.m_mapFriendData[UID(FriendListData_Rsp.uid_Friend)] = FriendData;
			}
		}
		break;
	case enDBCmd_GetFriendEnventListInfo:
		{
			for(int i = 0; i < OutParam.retCode; ++i){
				SDB_Get_FriendEnventListData_Rsp       FriendEnventListData_Rsp;
				RspIb >> FriendEnventListData_Rsp;

				SFriendEnventData FriendEnventData;
				strncpy(FriendEnventData.szDescript, FriendEnventListData_Rsp.Descript, sizeof(FriendEnventData.szDescript));
				FriendEnventData.uTime		= FriendEnventListData_Rsp.Time;

				std::map<UINT64/*好友的UID值*/,	VECT_FRIENDEVENT>::iterator iter = m_FriendData.m_mapFriendEnvent.find(FriendEnventListData_Rsp.uid_Friend);

				if( iter == m_FriendData.m_mapFriendEnvent.end()){
					VECT_FRIENDEVENT vectEvent;
					vectEvent.push_back(FriendEnventData);
					m_FriendData.m_mapFriendEnvent[FriendEnventListData_Rsp.uid_Friend] = vectEvent;
				}else{
					VECT_FRIENDEVENT & vectEvent = iter->second;
					vectEvent.push_back(FriendEnventData);
				}
			}
		}
		break;
	case enDBCmd_GetFriendMsgListInfo:
		{
			for(int i = 0; i < OutParam.retCode; ++i){
				SDB_Get_FrientMsgListData_Rsp		   FriendMsgListData_Rsp;
				RspIb >> FriendMsgListData_Rsp;

				SFriendMsgInfo FriendInfo;

				FriendInfo.m_Level		= FriendMsgListData_Rsp.SrcUserLevel;
				FriendInfo.m_Sex		= FriendMsgListData_Rsp.SrcUserSex;
				strncpy(FriendInfo.m_szSrcUserName, FriendMsgListData_Rsp.SrcUserName, sizeof(FriendInfo.m_szSrcUserName));
				strncpy(FriendInfo.m_szSrcUserSynName, FriendMsgListData_Rsp.SrcUserSynName,sizeof(FriendInfo.m_szSrcUserSynName));
				FriendInfo.m_bOneCity	= FriendMsgListData_Rsp.bOneCity;
				FriendInfo.m_Facade		= FriendMsgListData_Rsp.SrcUserFacade;
				FriendInfo.m_TitleID	= FriendMsgListData_Rsp.TitleID;
				FriendInfo.m_DuoBaoLevel = FriendMsgListData_Rsp.DuoBaoLevel;
				FriendInfo.m_SynWarLevel = FriendMsgListData_Rsp.SynWarLevel;
				FriendInfo.m_CombatAbility = FriendMsgListData_Rsp.CombatAbility;
				m_FriendData.m_mapFriendMsg[UID(FriendMsgListData_Rsp.uid_SrcUser)] = FriendInfo;
			}
		}
		break;
	}

}

//加好友时的回调，用于调整双方的好友度相同
void	FriendPart::HandleAddFriendBack(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Insert_FriendData_Rsp Rsp;
	RspIb >> RspHeader >> OutParam >> Rsp;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode == 2){
		return;
	}

	SFriendBasicInfo FriendInfo;
	
	FriendInfo.m_uidFriend	  = Rsp.uid_Friend;
	FriendInfo.m_bOneCity	  = Rsp.bOneCity;
	FriendInfo.m_enFriendType = (enFriendType)Rsp.Flag;
	FriendInfo.m_Level		  = Rsp.FriendLevel;
	FriendInfo.m_RelationNum  = Rsp.RelationNum;
	FriendInfo.m_Sex		  = Rsp.FriendSex;
	FriendInfo.m_Facade		  = Rsp.FriendFacade;
	FriendInfo.m_TitleID	  = Rsp.FriendTitleID;
	//FriendInfo.m_vipLevel	  = Rsp.vipLevel;
	FriendInfo.m_DuoBaoLevel  = Rsp.DuoBaoLevel;
	FriendInfo.m_CombatAbility = Rsp.CombatAbility;
	FriendInfo.m_SynWarLevel = Rsp.SynWarLevel;
	strncpy(FriendInfo.m_szFriendName, Rsp.FriendName, sizeof(FriendInfo.m_szFriendName));
	strncpy(FriendInfo.m_szSynName, Rsp.SynName, sizeof(FriendInfo.m_szSynName));

	m_FriendData.m_mapFriendData[UID(Rsp.uid_Friend)] = FriendInfo;

	this->__DeleteFriendMsgList(UID(Rsp.uid_Friend)); //如果好友信息里有对方的，则删除

	SC_FriendResult_Rsp RspFriend;
	OBuffer1k ob;

	RspFriend.m_Result = enFriendRetCode_OK;
	ob <<  FriendHeader(enFriendCmd_AddFriend, sizeof(SC_FriendResult_Rsp)) << RspFriend;
	m_pActor->SendData(ob.TakeOsb());

	//同步刷新显示
	this->ViewFriend();

	//如果对方在线要更新数据
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Rsp.uid_Friend));
	if( 0 != pActor){
		IFriendPart * pFriendPart = pActor->GetFriendPart();
		if( 0 == pFriendPart){
			return;
		}

		if( !pFriendPart->IsFriend(m_pActor->GetUID())){
			//对方不是你好友时，会往对方的好友信息列表中添加一条信息
			pFriendPart->AddToFriendMsgList(m_pActor->GetUID());
		}else{
			//对方是你好友时，则要同步下数据
			if( Rsp.Flag >= enFriendType_Max){
				return;
			}

			pFriendPart->SynFriendInfo(UID(Rsp.uid_Friend), (enFriendType)Rsp.Flag, Rsp.RelationNum);
		}
	}

	//发事件
	SS_AddFriend AddFriend;

	if( m_pActor->GetCrtProp(enCrtProp_ActorSex) == (INT32)Rsp.FriendSex){
		AddFriend.m_bYiXing = false;
	}else{
		AddFriend.m_bYiXing = true;
	}

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddFriend);
	m_pActor->OnEvent(msgID,&AddFriend,sizeof(AddFriend));
}

//删除改变好友度事件(只保存一天记录)
void	FriendPart::DeleteFriendEnvent()
{
	if(!IsInitFriendData()){
		LoadFriendInfo();
	}

	time_t CurTime = time(0);

	SDB_Delete_FriendEnventData_Req Req;
	Req.Time	   = CurTime;

	OBuffer1k ob;
	ob << Req;

	//超过一天的全删除
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteFriendEnventInfo, ob.TakeOsb(), 0, 0);

	std::map<UINT64/*好友的UID值*/,	VECT_FRIENDEVENT>::iterator iter = m_FriendData.m_mapFriendEnvent.begin();

	for( ; iter != m_FriendData.m_mapFriendEnvent.end(); ++iter)
	{
		VECT_FRIENDEVENT & vectEvent = iter->second;

		VECT_FRIENDEVENT::iterator itDel = vectEvent.begin();
		for( ; itDel != vectEvent.end();)
		{
			SFriendEnventData & FriendEnventData = *itDel;
			
			if((CurTime - FriendEnventData.uTime) >= m_FriendEnventRecordTime){
				//超过一天，删除
				vectEvent.erase(itDel++);
				continue;
			}

			 ++itDel;
		}
	}
}

//加改变好友度事件,szDescript为事件描述
void	FriendPart::AddFriendEnvent(const UID & uidFriend, UINT32 uAddRelationNum, const char * szDescript)
{
	if(!IsInitFriendData()){
		LoadFriendInfo();
	}

 	SDB_Insert_FriendEnventData_Req	 Req;
	Req.uid_User		= m_pActor->GetUID().m_uid;
	Req.uid_Friend		= uidFriend.m_uid;
	Req.AddRelationNum  = uAddRelationNum;
	strncpy(Req.Descript, szDescript, sizeof(Req.Descript));
	Req.Time			= time(0);

	OBuffer1k ob;
	ob << Req;

	if(g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertFriendEnventInfo, ob.TakeOsb(), 0, 0)){
		SFriendEnventData FriendEnventData;
		FriendEnventData.uid_friend = uidFriend.ToUint64();
		FriendEnventData.uTime		= time(0);
		strncpy(FriendEnventData.szDescript, szDescript, sizeof(FriendEnventData.szDescript));

		std::map<UINT64/*好友的UID值*/,	VECT_FRIENDEVENT>::iterator iter = m_FriendData.m_mapFriendEnvent.find(FriendEnventData.uid_friend);

		if( iter == m_FriendData.m_mapFriendEnvent.end()){
			VECT_FRIENDEVENT vectEvent;
			vectEvent.push_back(FriendEnventData);
			m_FriendData.m_mapFriendEnvent[FriendEnventData.uid_friend] = vectEvent;
		}else{
			VECT_FRIENDEVENT & vectEvent = iter->second;
			vectEvent.push_back(FriendEnventData);
		}
	}
}

//查看改变好友度事件
void	FriendPart::ViewFriendEnvent(UID & uidFriend)
{
	UINT32 nCurTime = time(0);

	SC_ViewFriendEnvent_Rsp Rsp;

	std::map<UINT64/*好友的UID值*/,	VECT_FRIENDEVENT>::iterator iter = m_FriendData.m_mapFriendEnvent.find(uidFriend.ToUint64());

	OBuffer1k ob;

	if( iter == m_FriendData.m_mapFriendEnvent.end()){
		Rsp.m_nFriendEnventNum = 0;

		ob << FriendHeader(enFriendCmd_ViewFriendEnvent, sizeof(SC_ViewFriendEnvent_Rsp)) << Rsp;
	}else{
		VECT_FRIENDEVENT & vectEvent = iter->second;

		Rsp.m_nFriendEnventNum = vectEvent.size();

		ob << FriendHeader(enFriendCmd_ViewFriendEnvent, sizeof(SC_ViewFriendEnvent_Rsp) + Rsp.m_nFriendEnventNum * sizeof(SFriendEnventDataRsp)) << Rsp;

		for(int i = 0; i < vectEvent.size(); ++i)
		{
			SFriendEnventDataRsp FriendEnventList;
			FriendEnventList.m_uTime = nCurTime;
			strncpy(FriendEnventList.m_szDescript, vectEvent[i].szDescript, sizeof(FriendEnventList.m_szDescript));

			ob << FriendEnventList;
		}
	}

	m_pActor->SendData(ob.TakeOsb());
}

//加到好友信息标签里
void	FriendPart::AddToFriendMsgList(const UID & uidSrcUser)
{
	if(m_FriendData.m_mapFriendMsg.size() >= m_MaxFriendMsgNum){
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidSrcUser);
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SFriendMsgInfo FriendMsgInfo;
	
	if( pActor->GetCrtProp(enCrtProp_ActorCityID) == m_pActor->GetCrtProp(enCrtProp_ActorCityID)){
		FriendMsgInfo.m_bOneCity = true;
	}else{
		FriendMsgInfo.m_bOneCity = false;
	}

	FriendMsgInfo.m_Level = pActor->GetCrtProp(enCrtProp_Level);
	FriendMsgInfo.m_Sex	  = pActor->GetCrtProp(enCrtProp_ActorSex);
	FriendMsgInfo.m_Facade = pActor->GetCrtProp(enCrtProp_ActorFacade);
	strncpy(FriendMsgInfo.m_szSrcUserName, pActor->GetName(), sizeof(FriendMsgInfo.m_szSrcUserName));
	
	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(uidSrcUser);
	if( 0 != pSyndicate){
		strncpy(FriendMsgInfo.m_szSrcUserSynName, pSyndicate->GetSynName(), sizeof(FriendMsgInfo.m_szSrcUserSynName));
	}

	m_FriendData.m_mapFriendMsg[uidSrcUser] = FriendMsgInfo;
}

//查看好友信息标签里的信息
void	FriendPart::ViewFriendMsg()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0== pSynMgr){
		return;
	}

	SViewFriendMsgListDataRsp Rsp;
	Rsp.m_nFriendMsgNum = m_FriendData.m_mapFriendMsg.size();

	OBuffer4k ob2;

	std::map<UID, SFriendMsgInfo>::iterator iter = m_FriendData.m_mapFriendMsg.begin();

	for( ;iter != m_FriendData.m_mapFriendMsg.end() ; ++iter)
	{
		SFriendMsgInfo & FriendInfo = iter->second;

		SFriendMsgDataRsp FriendMsgRsp;

		FriendMsgRsp.m_uidSrcUser = iter->first;
		IActor * pSrcUser = g_pGameServer->GetGameWorld()->FindActor((UID)FriendMsgRsp.m_uidSrcUser);
		if( 0 != pSrcUser  &&  pSrcUser->GetIsSelfOnline()){
			FriendMsgRsp.m_Level = pSrcUser->GetCrtProp(enCrtProp_Level);
			FriendMsgRsp.m_DuoBaoLevel = pSrcUser->GetCrtProp(enCrtProp_DuoBaoLevel);
			FriendMsgRsp.m_SynWarLevel = pSrcUser->GetCrtProp(enCrtProp_SynCombatLevel);
			FriendMsgRsp.m_CombatAbility = pSrcUser->GetCrtProp(enCrtProp_ActorCombatAbility);
			FriendMsgRsp.m_bOnLine = true;
		}else{
			FriendMsgRsp.m_Level = FriendInfo.m_Level;
			FriendMsgRsp.m_DuoBaoLevel = FriendInfo.m_DuoBaoLevel;
			FriendMsgRsp.m_SynWarLevel = FriendInfo.m_SynWarLevel;
			FriendMsgRsp.m_CombatAbility = FriendInfo.m_CombatAbility;
			FriendMsgRsp.m_bOnLine = false;		
		}

		//FriendMsgRsp.m_Level = FriendInfo.m_Level;
		FriendMsgRsp.m_Sex	 = FriendInfo.m_Sex;
		strncpy(FriendMsgRsp.m_szName, FriendInfo.m_szSrcUserName, sizeof(FriendMsgRsp.m_szName));
		FriendMsgRsp.m_bOneCity = FriendInfo.m_bOneCity;
		FriendMsgRsp.m_Facade = FriendInfo.m_Facade;
		FriendMsgRsp.m_TitleID = FriendInfo.m_TitleID;


		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(iter->first);
		if( 0 == pSyndicate || pSyndicate->GetSynID() == INVALID_SYN_ID )
		{
			ob2 << FriendMsgRsp;
			continue;
		}
		else
		{
			FriendMsgRsp.m_SynID = pSyndicate->GetSynID();
			ob2 << FriendMsgRsp;
		}

		SSynInfo SynInfo;
		
		SynInfo.m_SynLevel = pSyndicate->GetSynLevel();
		SynInfo.m_SynMemberNum = pSyndicate->GetSynMemberNum();
		SynInfo.m_SynWarAbility = pSyndicate->GetSynWarAbility();
		strncpy(SynInfo.m_szSynLeaderName, pSyndicate->GetLeaderName(), sizeof(SynInfo.m_szSynLeaderName));
		strncpy(SynInfo.m_szSynName, pSyndicate->GetSynName(), sizeof(SynInfo.m_szSynName));

		ob2 << SynInfo;
	}

	OBuffer4k ob;
	ob << FriendHeader(enFriendCmd_ViewFriendMsg, sizeof(SViewFriendMsgListDataRsp) + ob2.Size()) << Rsp;

	if( ob2.Size() > 0){
		ob << ob2;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//设置好友对自己的好友度数值
void	FriendPart::SetRelationNum(const UID & uidFriend, UINT32 uRelationNum)
{
	if(IsInitFriendData()){	
		//加载过数据的才需要加载
		std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);

		if(iter == m_FriendData.m_mapFriendData.end()){
			return;
		}

		SFriendBasicInfo & FriendInfo = iter->second;

		FriendInfo.m_RelationNum = uRelationNum;
	}
}

//同步好友对自己的好友数据
void	FriendPart::SynFriendInfo(const UID & uidFriend, enFriendType FriendType, UINT8 RelationNum)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);
	if( iter == m_FriendData.m_mapFriendData.end()){
		return;
	}

	SFriendBasicInfo & FriendInfo = iter->second;
	FriendInfo.m_enFriendType = FriendType;
	FriendInfo.m_RelationNum  = RelationNum;
}

//造访好友
void	FriendPart::VisitFriend(const UID & uidFriend)
{
	SC_VisitFriend_Rsp Rsp;

	if ( uidFriend == m_uidVisitFriend )
	{
		Rsp.m_Result = enFriendRetCode_OK;
	}
	//else if ( m_uidVisitFriend.IsValid() )
	//{
	//	//一次只能访问一个，防止用户快速点多下
	//	Rsp.m_Result = enFriendRetCode_enHoldOn;
	//}
	else
	{
		//先取消上一次的拜访
		this->CancelVisitFriend(false);

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidFriend);

		if( 0 == pActor)
		{
			//先订阅玩家创建的消息
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

			IEventServer * pEnventServer = g_pGameServer->GetEventServer();
			if( 0 == pEnventServer)
			{
				return;
			}

			pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"VisitFriend");

			//记录要访问的好友UID
			m_uidVisitFriend = uidFriend;

			//加载玩家进内存
			g_pGameServer->LoadActor(uidFriend);

			return;
		}
		
		//开始真正访问好友
		if( this->BeginVisitFriend(pActor))
		{
			//记录要访问的好友UID
			m_uidVisitFriend = uidFriend;

			//关注此玩家
			m_pActor->AddToAttentionUser(pActor->GetUID());
		}

		return;
	}

	OBuffer1k ob;
	ob << FriendHeader(enFriendCmd_VisitFriend, sizeof(SC_VisitFriend_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

void	FriendPart::OnEvent(XEventData & EventData)
{
	//创建玩家的消息ID
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	if( EventData.m_MsgID == msgID)
	{
		SS_ActoreCreateContext * ActoreCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == ActoreCreateContext){
			return;
		}

		if( ActoreCreateContext->m_uidActor == m_uidVisitFriend.ToUint64()){
			//拜访好友的事件
			this->OnEvent_VisitFriend(m_uidVisitFriend);
			return;
		}

		if( ActoreCreateContext->m_uidActor == m_uidViewUserInfo.ToUint64()){
			//查看人物信息事件
			this->OnEvent_ViewUserInfo(m_uidViewUserInfo);
			return;
		}
	}

	//进场景消息
	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

	if( EventData.m_MsgID == msgID)
	{
		if( m_uidVisitFriend.IsValid()){
			SS_EnterScene * pEnterScene = (SS_EnterScene *)EventData.m_pContext;
			if( 0 == pEnterScene){
				return;
			}

			TSceneID OldSceneID = pEnterScene->m_OldSceneID;
			
			if( OldSceneID.GetMapID() == g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MainMapID
				 && OldSceneID.ToID() != m_pActor->GetCrtProp(enCrtProp_ActorMainSceneID))
			{
				//从好友的主场景中回来
				m_pActor->RemoveToAttentionUser(m_uidVisitFriend);
				m_uidVisitFriend = UID();

				//取消监听
				m_pActor->UnsubscribeEvent(msgID, this);
			}	
		}else{
			//取消监听
			m_pActor->UnsubscribeEvent(msgID, this);	
		}
	}
}

//结束拜访好友
void	FriendPart::EndVisitFriend()
{
	if( m_uidVisitFriend.IsValid()){
		m_uidVisitFriend = UID();
		
		//要取消监听的消息ID
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
		if( 0 == pEnventServer){
			return;
		}

		pEnventServer->RemoveListener(this, msgID,enEventSrcType_Actor,0);
	}
}

//查看同城的在线玩家
void	FriendPart::ViewOneCityOnlineUser()
{
	ViewOneCityUser CityUser(m_pActor);

	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, CityUser);

	CityUser.SendViewMsg();
}

//得到好友名字
const char * FriendPart::GetFriendName(UID uidFriend)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);
	if( iter == m_FriendData.m_mapFriendData.end()){
		return 0;
	}

	return (iter->second).m_szFriendName;
}

//得到好友信息
const SFriendBasicInfo * FriendPart::GetFriendInfo(UID uidFriend)
{
	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.find(uidFriend);
	if( iter == m_FriendData.m_mapFriendData.end()){
		return 0;
	}

	return &(iter->second);
}

//增加好友度(包括减少),的数据库回调
void	FriendPart::HandleAddRelationNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());
	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_Update_FriendData_Rsp  Rsp; 
	RspIb >> RspHeader >> Rsp;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}
	SDB_Update_FriendData_Req Req;
	ReqIb >> Req;

	UINT32 nMeRelation = this->GetRelationNum(UID(Req.uid_Friend));

	//好友在线则更新好友列表里的好友度
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Req.uid_Friend));
	if( 0 != pActor){
		IFriendPart * pFriendPart = pActor->GetFriendPart();
		if( 0 != pFriendPart){
			UINT32 nFriendRelation = pFriendPart->GetRelationNum(m_pActor->GetUID());
			//因为好友度是与好友的共同属性，为了防止修改的同时，好友也在修改，取出三者的最大值写入
			UINT32 nMaxRelation = 0;

			if( nMeRelation > nFriendRelation){
				nMaxRelation = nMeRelation;
			}else{
				nMaxRelation = nFriendRelation;
			}
			if( Rsp.RelationNum > nMaxRelation){
				nMaxRelation = Rsp.RelationNum;
			}

			this->SetRelationNum(UID(Req.uid_Friend), nMaxRelation);
			pFriendPart->SetRelationNum(m_pActor->GetUID(), nMaxRelation);
		}
	}else{
		if( nMeRelation < Rsp.RelationNum){
			this->SetRelationNum(UID(Req.uid_Friend), Rsp.RelationNum);
		}
	}
}

void	FriendPart::OnEvent_VisitFriend(UID uidFriend)
{
	//好友已创建成功，获取好友指针
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidFriend);
	if( 0 == pFriend){
		//如果玩家没取到
		m_uidVisitFriend = UID();
	}else{
		//开始真正访问好友
		if( this->BeginVisitFriend(pFriend)){
		
			//关注此玩家
			m_pActor->AddToAttentionUser(pFriend->GetUID());
		}		
	}

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	//要取消监听的消息ID
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	pEnventServer->RemoveListener(this, msgID,enEventSrcType_Actor,0);
}

//邮件点击选择收信人按钮
void  FriendPart::ClickMailFriend()
{
	SC_ClickFriend_Rsp Rsp;

	Rsp.m_Num = m_FriendData.m_mapFriendData.size();

	OBuffer1k ob;
	ob << MailHeader(enMailCmd_ClickFriend, sizeof(SC_ClickFriend_Rsp) + Rsp.m_Num * sizeof(FriendName)) << Rsp;

	std::map<UID/*好友的UID值*/, SFriendBasicInfo>::iterator iter = m_FriendData.m_mapFriendData.begin();
	
	for( ; iter != m_FriendData.m_mapFriendData.end(); ++iter)
	{
		SFriendBasicInfo & FriendBasicInfo = iter->second;

		FriendName Name;

		strncpy(Name.m_szFriendName, FriendBasicInfo.m_szFriendName, sizeof(Name.m_szFriendName));

		ob << Name;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//掉线，下线要做的一些事
void  FriendPart::LeaveDoSomeThing()
{
	//取消拜访好友
	this->CancelVisitFriend();

	if( m_uidViewUserInfo.IsValid())
	{
		m_pActor->RemoveToAttentionUser(m_uidViewUserInfo);

		m_uidViewUserInfo = UID();
	}

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->RemoveAllListener(this);
}

//查看人物信息
void  FriendPart::ViewUserInfo(UID uidUser)
{
	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);
	if( 0 == pTargetActor){

		//先订阅玩家创建的消息
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
		if( 0 == pEnventServer){
			return;
		}

		pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"FriendPart::ViewUserInfo");	

		m_uidViewUserInfo = uidUser;

		g_pGameServer->LoadActor(uidUser);
	}else{
		this->SendUserInfo(pTargetActor);
	}
}

void  FriendPart::SendUserInfo(IActor * pTargetActor)
{
	//主角的角色信息和装备栏信息
	m_pActor->NoticClientCreatePrivateThing(pTargetActor->GetUID());

	IEquipPart * pEquipPart = pTargetActor->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	pEquipPart->SendEquipPanelData(m_pActor);

	//招募角色的角色信息和装备栏信息
	for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = pTargetActor->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		IEquipPart * pEmpEquipPart = pEmployee->GetEquipPart();
		if( 0 == pEmpEquipPart){
			continue;
		}

		m_pActor->NoticClientCreatePrivateThing(pEmployee->GetUID());

		//发送装备栏数据
		pEmpEquipPart->SendEquipPanelData(m_pActor);
	}

	//添加关注
	m_pActor->AddToAttentionUser(pTargetActor->GetUID());

	SC_ViewUserInfo Rsp;

	Rsp.m_Result = enFriendRetCode_OK;
	Rsp.m_uidUser = pTargetActor->GetUID();

	ICombatPart * pCombatPart = pTargetActor->GetCombatPart();
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
	m_pActor->SendData(ob.TakeOsb());
}

void	FriendPart::__InsertFriendEvent_ToDB(const UID & uidUser, const UID & uidFriend, UINT32 uAddRelationNum, const char * szDescript)
{
 	SDB_Insert_FriendEnventData_Req	 Req;
	Req.uid_User		= uidUser.ToUint64();
	Req.uid_Friend		= uidFriend.ToUint64();
	Req.AddRelationNum  = uAddRelationNum;
	strncpy(Req.Descript, szDescript, sizeof(Req.Descript));
	Req.Time			= time(0);

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(Req.AddRelationNum, enDBCmd_InsertFriendEnventInfo, ob.TakeOsb(), 0, 0);	
}

void	FriendPart::OnEvent_ViewUserInfo(UID uidUser)
{
	IActor * pTargetActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if( 0 != pTargetActor){
		this->SendUserInfo(pTargetActor);
	}

	m_uidViewUserInfo = UID();

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	//要取消监听的消息ID
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	pEnventServer->RemoveListener(this, msgID,enEventSrcType_Actor,0);
}

//离开查看其它玩家人物信息界面
void	FriendPart::LeaveViewUserInfo(UID uidUser)
{
	m_pActor->RemoveToAttentionUser(uidUser);

	SC_LeaveViewUserInfo Rsp;
	Rsp.m_Result = enFriendRetCode_OK;

	OBuffer1k ob;
	ob << FriendHeader(enFriendCmd_LeaveViewUserInfo, sizeof(SC_LeaveViewUserInfo)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//删除好友信息
void	FriendPart::DeleteFriendMsg(UID uidUser)
{
	SC_DeleteFriendMsg Rsp;
	OBuffer1k ob;

	std::map<UID, SFriendMsgInfo>::iterator iter = m_FriendData.m_mapFriendMsg.find(uidUser);

	if(iter != m_FriendData.m_mapFriendMsg.end())
	{
		m_FriendData.m_mapFriendMsg.erase(iter);

		SDB_Delete_FriendMsgData_Req DBReq;

		DBReq.uid_User = m_pActor->GetUID().ToUint64();
		DBReq.uid_SrcUser = uidUser.ToUint64();

		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteFriendMsgInfo, ob.TakeOsb(), 0, 0);
		ob.Reset();
	}
	else
	{
		Rsp.m_Result = enFriendRetCode_NoFriendMsg;
	}

	ob << FriendHeader(enFriendCmd_DeleteFriendMsg, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//取消拜访,bBackMainScene为是否退回主场景
void	FriendPart::CancelVisitFriend(bool bBackMainScene)
{
	if ( m_uidVisitFriend.IsValid() )
	{
		//先退回主场景
		if ( bBackMainScene )
		{
			m_pActor->ComeBackMainScene();
		}

		//取消关注
		m_pActor->RemoveToAttentionUser(m_uidVisitFriend);

		m_uidVisitFriend = UID();	
	}
}
