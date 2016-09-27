#include "Challenge.h"
#include "IActor.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IDouFaPart.h"
#include "IBasicService.h"
#include "ICombatPart.h"
#include "IGameScene.h"
#include "IConfigServer.h"
#include "RandomService.h"
#include "XDateTime.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"


//刷新我的挑战信息
void	Challenge::FlushMyChallenge(IActor * pActor)
{
	SDB_Get_ChallengeMyEnemy Req;

	Req.m_UidUser = pActor->GetUID().ToUint64();
	Req.m_Num	  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_FlushMyEnemyNum;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Get_ChallengeMyEnemy, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

//刷新我的挑战回放信息
void	Challenge::FlushMyChallengeRecord(IActor * pActor)
{
	SDB_Get_MyChallengeRecord Req;

	Req.m_uidUser = pActor->GetUID().ToUint64();
	Req.m_Num	  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ChallengeRecordNum;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Get_MyChallengeRecord, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

//查看等级组排行
void	Challenge::ViewLvGroup(IActor * pActor, enLevelGroup lvGroup, INT32 BeginPos, INT32 Num)
{
	SDB_Get_ChallengeLevelRank Req;

	Req.m_BeginPos = BeginPos;
	Req.m_Num      = Num;

	switch(lvGroup)
	{
	case enLevelGroup_30_34:
		{
			Req.m_BeginLevel = 30;
			Req.m_EndLevel	 = 34;
		}
		break;
	case enLevelGroup_35_39:
		{
			Req.m_BeginLevel = 35;
			Req.m_EndLevel	 = 39;			
		}
		break;
	case enLevelGroup_40_44:
		{
			Req.m_BeginLevel = 40;
			Req.m_EndLevel	 = 44;			
		}
		break;
	case enLevelGroup_45_49:
		{
			Req.m_BeginLevel = 45;
			Req.m_EndLevel	 = 49;
		}
		break;
	case enLevelGroup_50_54:
		{
			Req.m_BeginLevel = 50;
			Req.m_EndLevel	 = 54;			
		}
		break;
	case enLevelGroup_55_60:
		{
			Req.m_BeginLevel = 55;
			Req.m_EndLevel	 = 60;			
		}
		break;
	default:
		{
			Req.m_BeginLevel = 0;
			Req.m_EndLevel	 = 0;		
		}
		break;
	}

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Get_ChallengeLevelRank, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

//挑战玩家
void	Challenge::ChallengeUser(IActor * pActor, UID m_uidEnemy)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	UINT8 JoinChallengeNum = pDouFaPart->GetChallengeNum();
	
	SC_Challenge Rsp;

	if( JoinChallengeNum >= pDouFaPart->MaxChallengeNum())
	{
		Rsp.m_Result = enDouFaRetCode_ErrMaxChallenge;
	}
	else if ( pActor->HaveTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrHaveTeam;
	}
	else if ( pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrInDuoBao2;
	}
	else if ( pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enDouFaRetCode_ErrWaitTeam;
	}
	else
	{
		//检测是否是可挑战对手
		SDB_Check_CanChallenge CanChallenge;
		CanChallenge.m_uidMe = pActor->GetUID().ToUint64();
		CanChallenge.m_uidEnemy = m_uidEnemy.ToUint64();
		CanChallenge.m_Num	 = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_FlushMyEnemyNum;

		OBuffer1k ob;
		ob << CanChallenge;
		g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Check_CanChallenge, ob.TakeOsb(), this, pActor->GetUID().m_uid);
		return;
	}

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_Challenge, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//IDBProxyClientSink接口
void	Challenge::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_Get_ChallengeMyEnemy:
			{
				HandleFlushMyChallenge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   case enDBCmd_Get_ChallengeLevelRank:
		   {
			   HandleGetLvGroupRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Check_CanChallenge:
		   {
				HandleCheckCanChallenge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_MyChallengeRecord:
		   {
				HandleFlushMyChallengeRecord(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}
}

void	Challenge::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	if( EventData.m_MsgID == msgID)
	{
		SS_ActoreCreateContext * pActoreCreateCnt = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == pActoreCreateCnt){
			return;
		}

		std::map<UID, UID>::iterator iter = m_mapListen.find(UID(pActoreCreateCnt->m_uidActor));
		if( iter == m_mapListen.end()){
			return;
		}

		UID uidTail = (*iter).second;
		UID uidHead = (*iter).first;

		m_mapListen.erase(iter);

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidTail);
		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidHead);

		if (pActor != 0 && pEnemy != 0){
			
			this->BeginCombat(pActor, pEnemy);
		}

		if (m_mapListen.size() == 0){
			IEventServer * pEnventServer = g_pGameServer->GetEventServer();
			if( 0 == pEnventServer){
				return;
			}
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);
			pEnventServer->RemoveListener(this, msgID, enEventSrcType_Actor, 0);
		}
	}
}

void	Challenge::HandleFlushMyChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}
	
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_MyChallengeRank MyChallenge;
	RspIb >> RspHeader >> MyChallenge;

	if( RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_FlushMyChallenge Rsp;

	Rsp.m_GetHonorToday = pDouFaPart->GetGetHonorToday();
	Rsp.m_MaxGetHonorToday = pDouFaPart->GetMaxGetHonorToday();
	Rsp.m_LvGroupRank	= MyChallenge.m_MyRankLevel;
	Rsp.m_MainRank		= MyChallenge.m_MyRank;
	Rsp.m_MaxChallengeNum = pDouFaPart->MaxChallengeNum();
	Rsp.m_ChallengeNum  = Rsp.m_MaxChallengeNum - pDouFaPart->GetChallengeNum();

	if( MyChallenge.m_RetCode != 0){
		Rsp.m_NumEnemy = 0;
	}else{
		Rsp.m_NumEnemy = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo);
	}

	OBuffer4k ob;
	ob <<  DouFaHeader(enDouFaCmd_FlushMyChallenge, sizeof(Rsp) + Rsp.m_NumEnemy * sizeof(EnemyInfo)) << Rsp;
	
	for( int i = 0; i < Rsp.m_NumEnemy; ++i)
	{
		SDB_ChallengeRankInfo ChallengeEnemy;
		RspIb >> ChallengeEnemy;
		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 获取客户端数据长度有误！！", __FUNCTION__, __LINE__);
			continue;
		}

		EnemyInfo Enemy;

		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(UID(ChallengeEnemy.m_uidUser));
		if( 0 == pEnemy){
			Enemy.m_Level = ChallengeEnemy.m_Level;
			Enemy.m_vipLevel = ChallengeEnemy.m_VipLevel;
		}else{
			Enemy.m_Level = pEnemy->GetCrtProp(enCrtProp_Level);
			Enemy.m_vipLevel = pEnemy->GetCrtProp(enCrtProp_TotalVipLevel);
		}

		Enemy.m_MainRank = ChallengeEnemy.m_Rank;
		strncpy(Enemy.m_Name, ChallengeEnemy.m_Name, sizeof(Enemy.m_Name));
		Enemy.m_UidEnemy = UID(ChallengeEnemy.m_uidUser);

		ob << Enemy;
	}

	pActor->SendData(ob.TakeOsb());
}

void	Challenge::HandleGetLvGroupRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());
	SDB_Get_ChallengeLevelRank Req;
	ReqIb >> Req;

	if( RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_ViewLvGroup Rsp;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo);
	}

	OBuffer4k ob;
	ob <<  DouFaHeader(enDouFaCmd_ViewLvGroup, sizeof(Rsp) + Rsp.m_Num * sizeof(LvGroupUserInfo)) << Rsp;

	for( int i = 0,nRank = Req.m_BeginPos; i < Rsp.m_Num; ++i,++nRank)
	{
		SDB_ChallengeRankInfo ChallengeInfo;
		RspIb >> ChallengeInfo;
		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 获取客户端数据长度错误！！", __FUNCTION__, __LINE__);
			continue;
		}

		LvGroupUserInfo UserInfo;

		IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(UID(ChallengeInfo.m_uidUser));
		if( 0 == pUser){
			UserInfo.m_Level = ChallengeInfo.m_Level;
			UserInfo.m_vipLevel = ChallengeInfo.m_VipLevel;
			UserInfo.m_DuoBaoLevel = ChallengeInfo.m_DuoBaoLevel;
			UserInfo.m_Layer = ChallengeInfo.m_Layer;
		}else{
			UserInfo.m_Level = pUser->GetCrtProp(enCrtProp_Level);
			UserInfo.m_vipLevel = pUser->GetCrtProp(enCrtProp_TotalVipLevel);
			UserInfo.m_DuoBaoLevel = pUser->GetCrtProp(enCrtProp_DuoBaoLevel);
			UserInfo.m_Layer = pUser->GetCrtProp(enCrtProp_ActorLayer);
		}

		UserInfo.m_UidUser = UID(ChallengeInfo.m_uidUser);
		strncpy(UserInfo.m_Name, ChallengeInfo.m_Name, sizeof(UserInfo.m_Name));
		UserInfo.m_LvGroupRank = nRank;

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(ChallengeInfo.m_uidUser));
		if( 0 == pSyndicate){
			memset(UserInfo.m_SynName, 0, sizeof(UserInfo.m_SynName));
		}else{
			strncpy(UserInfo.m_SynName, pSyndicate->GetSynName(), sizeof(UserInfo.m_SynName));
		}
		ob << UserInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

void	Challenge::HandleCheckCanChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());
	SDB_Check_CanChallenge Req;
	ReqIb >> Req;

	if( RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_Challenge Rsp;

	if( OutParam.retCode != 0){
		Rsp.m_Result = enDouFaRetCode_ErrRankChange;
	}else{
		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(UID(Req.m_uidEnemy));
		if( 0 != pEnemy){
			this->BeginCombat(pActor, pEnemy);
		}else{
			//OK，加载玩家
			if( m_mapListen.size() == 0){
				//先订阅玩家创建的消息
				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

				IEventServer * pEnventServer = g_pGameServer->GetEventServer();
				if( 0 == pEnventServer){
					return;
				}

				pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"Challenge::HandleCheckCanChallenge");		
			}

			g_pGameServer->LoadActor(UID(Req.m_uidEnemy));

			m_mapListen[UID(Req.m_uidEnemy)] = pActor->GetUID();
		}
	}
}


void	Challenge::HandleFlushMyChallengeRecord(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}
	


	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;


	if( RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_FlushMyChallengeRecord Rsp;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_MyChallengeRecordInfo);
	}

	OBuffer4k ob;
	ob <<  DouFaHeader(enDouFaCmd_FlushChallengeRecord, sizeof(Rsp) + Rsp.m_Num * sizeof(MyChallengeRecordInfo) ) << Rsp;

	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_MyChallengeRecordInfo MyChallengeRecord;
		RspIb >> MyChallengeRecord;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}
		MyChallengeRecordInfo RecordInfo;

		RecordInfo.m_bActive = MyChallengeRecord.m_bActive;
		RecordInfo.m_bWin = MyChallengeRecord.m_bWin;
		RecordInfo.m_EnemyLevel = MyChallengeRecord.m_EnemyLevel;
		RecordInfo.m_VipLevel = MyChallengeRecord.m_VipLevel;
		RecordInfo.m_RecordBufIndex = MyChallengeRecord.m_Rank;
		RecordInfo.m_EnemyFacade = MyChallengeRecord.m_EnemyFacade;
		strncpy(RecordInfo.m_EnemyName, MyChallengeRecord.m_EnemyName, sizeof(RecordInfo.m_EnemyName));
		

		ob << RecordInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

//开始战斗
void	Challenge::BeginCombat(IActor * pActor, IActor * pEnemy)
{
	if( 0 == pActor || 0 == pEnemy){
		return;
	}

	ICombatPart * pCombatPart = pActor->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ChallengeCombatMap;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	
	if( 0 == pGameScene){
		return ;
	}

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(MapID);
	if( 0 == pMapConfig){
		TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, MapID);
		return;
	}


	UINT64 CombatID = 0;

	pCombatPart->CombatWithScene(enCombatType_Challenge,pEnemy->GetUID(), CombatID,this,g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID),pGameScene, enJoinPlayer_Oneself);

	m_setCombat.insert(CombatID);

	pDouFaPart->JoinChallenge();

}

//进入挑战场景
bool	Challenge::EnterChallengeScene(IActor * pActor, IActor * pEnemy)
{
	TMapID MapID   = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ChallengeCombatMap;

	//创建
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
	
	if( 0 == pGameScene){
		return false;
	}

	TSceneID SceneID =pGameScene->GetSceneID();

	pGameScene->EnterScene(pActor);

	SC_EnterChallengeScene Rsp;
	Rsp.m_Result = enDouFaRetCode_Ok;

	OBuffer1k ob;
	ob << DouFaHeader(enDouFaCmd_SC_EnterChallenge,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	return true;
}

//战斗奖励
void	Challenge::Forward(IActor * pActor, IActor * pEnemy, bool bWin,const OBuffer4k & ob)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();
	if( 0 == pDouFaPart){
		return;
	}

	SC_Challenge Rsp;

	Rsp.m_Result = enDouFaRetCode_Ok;

	ChallengeData data;

	if( bWin){
		INT32 GetHonor = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ChallengeWinGetHonorNum;

		GetHonor += pActor->GetVipValue(enVipType_AddGetHonor);

		INT32 MaxGetHonor = pDouFaPart->GetMaxGetHonorToday();

		if( (GetHonor + pDouFaPart->GetGetHonorToday()) > MaxGetHonor){
			GetHonor = MaxGetHonor - pDouFaPart->GetGetHonorToday();
		}

		data.m_GetHonor = GetHonor;

		//胜利交换排名
		SDB_Change_ChallengeRank DBReq;
		DBReq.m_UidUser1 = pActor->GetUID().ToUint64();
		DBReq.m_UidUser2 = pEnemy->GetUID().ToUint64();

		OBuffer1k obDB;
		obDB << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Change_ChallengeRank, obDB.TakeOsb(), 0, 0);
	}
	
	data.m_NowHonor = pDouFaPart->GetGetHonorToday();

	pDouFaPart->AddHonor(data.m_GetHonor);

	OBuffer4k ob2;
	ob2 << DouFaHeader(enDouFaCmd_Challenge,sizeof(Rsp) + sizeof(data) ) << Rsp << data ;
	pActor->SendData(ob2.TakeOsb());
}

//排名前五及前5%的奖励
void	Challenge::LvGroupRankForward(UID uidUser, enLevelGroup lvGroup, UINT8 nRank)
{
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	const SChallengeForward * pChallengeForward ;
	if(nRank > GameParam.m_ChallengeRankGetForward){

		pChallengeForward = g_pGameServer->GetConfigServer()->GetChallengeForward(lvGroup, GameParam.m_ChallengeRankGetForward + 1);

	}else{

		pChallengeForward = g_pGameServer->GetConfigServer()->GetChallengeForward(lvGroup, nRank);

	}
	if( 0 == pChallengeForward){
		return;
	}

	if( pChallengeForward->m_vectGoods.size() < 1){
		SWriteSystemData SysMailData;

		SysMailData.m_DestUID = uidUser;
		SysMailData.m_PolyNimbus = pChallengeForward->m_PolyNimbus;


		strncpy(SysMailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10008), sizeof(SysMailData.m_szThemeText));
		//strncpy(SysMailData.m_szThemeText, "挑战奖励", sizeof(SysMailData.m_szThemeText));

		switch(lvGroup)
		{
		case enLevelGroup_30_34:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 30, 34, nRank);
			//sprintf(SysMailData.m_szContentText, "恭喜你获得%d-%d等级组本周的第%d名，感谢您的参与", 30, 34, nRank);
			break;
		case enLevelGroup_35_39:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 35, 39, nRank);

			break;
		case enLevelGroup_40_44:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 40, 44, nRank);

			break;
		case enLevelGroup_45_49:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 45, 49, nRank);

			break;
		case enLevelGroup_50_54:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 50, 54, nRank);

			break;
		case enLevelGroup_55_60:
			
			sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 55, 60, nRank);

			break;
		default:
			TRACE("<error> %s : %d 行 没有此等级组奖励！！等级组 %d", __FUNCTION__, __LINE__, lvGroup);
			return;
		}

		g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData);
		return;
	}


	INT32 nTotalRandom = 0;

	//要抽取的物品数量
	INT32 nCountGoods = pChallengeForward->m_vectGoods[0];

	for( int i = 1; i + 2 < pChallengeForward->m_vectGoods.size(); i += 3)
	{
		nTotalRandom += pChallengeForward->m_vectGoods[i + 2];
	}

	//保存得到的物品
	std::vector<UINT16> vectGoods;	//vectGoods[0] = goodsID,vectGoods[1] = num,....

	for( int i = 0; i < nCountGoods; ++i)
	{
		INT32 nRandom = RandomService::GetRandom() % nTotalRandom;

		INT32 nTemp = 0;

		for( int k = 1; k + 2 < pChallengeForward->m_vectGoods.size(); k += 3)
		{
			//先检测是否已经在得到的物品列表里面
			bool bExist = false;
			for( int n = 0; n + 1 < vectGoods.size(); n += 2)
			{
				if( pChallengeForward->m_vectGoods[k] == vectGoods[n]){
					bExist = true;
					continue;
				}
			}

			if( bExist){
				//已经在得到的物品列表里面
				continue;
			}

			nTemp += pChallengeForward->m_vectGoods[k + 2];

			if( nTemp > nRandom){
				vectGoods.push_back(pChallengeForward->m_vectGoods[k]);
				vectGoods.push_back(pChallengeForward->m_vectGoods[k + 1]);

				nTotalRandom -= pChallengeForward->m_vectGoods[k + 2];
				break;
			}
		}
	}

	SWriteSystemData SysMailData;

	SysMailData.m_DestUID = uidUser;
	SysMailData.m_PolyNimbus = pChallengeForward->m_PolyNimbus;

	// fly add	20121106
	strncpy(SysMailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10008), sizeof(SysMailData.m_szThemeText));
	//strncpy(SysMailData.m_szThemeText, "挑战奖励", sizeof(SysMailData.m_szThemeText));

	switch(lvGroup)
	{
	case enLevelGroup_30_34:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 30, 34, nRank);
		//sprintf(SysMailData.m_szContentText, "恭喜你获得%d-%d等级组本周的第%d名，感谢您的参与", 30, 34, nRank);
		break;
	case enLevelGroup_35_39:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 35, 39, nRank);

		break;
	case enLevelGroup_40_44:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 40, 44, nRank);

		break;
	case enLevelGroup_45_49:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 45, 49, nRank);

		break;
	case enLevelGroup_50_54:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 50, 54, nRank);

		break;
	case enLevelGroup_55_60:
		
		sprintf(SysMailData.m_szContentText, g_pGameServer->GetGameWorld()->GetLanguageStr(10076), 55, 60, nRank);

		break;
	default:
		TRACE("<error> %s : %d 行 没有此等级组奖励！！等级组 %d", __FUNCTION__, __LINE__, lvGroup);
		return;
	}

	std::vector<IGoods *> vecGoods;

	for( int i = 0; i + 1 < vectGoods.size() && i < MAX_MAIL_GOODS_NUM; i += 2)
	{
		SCreateGoodsContext GoodsCnt;

		GoodsCnt.m_Binded = false;
		GoodsCnt.m_GoodsID = vectGoods[i];
		GoodsCnt.m_Number  = vectGoods[i + 1];

		IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsCnt);
		if( 0 == pGoods){
			TRACE("<error> %s : %d 行 创建挑战物品奖励失败！！,物品ID = %d, 物品数量 = %d", __FUNCTION__, __LINE__, GoodsCnt.m_GoodsID, GoodsCnt.m_Number);
			continue;
		}

		vecGoods.push_back(pGoods);
	}

	g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData, vecGoods);
}

//战斗结束了
void Challenge::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{

	std::hash_set<UINT64>::iterator it = m_setCombat.find(pCombatCnt->CombatID);

	if(it == m_setCombat.end())
	{
		return;
	}

	m_setCombat.erase(it);

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidSource);
	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidEnemy);

	if(pActor==0 || pEnemy==0)
	{
		return;
	}

	//保存战斗回放
	SDB_Insert_MyChallengeRecord Req;
	Req.m_UidUser = pCombatCnt->uidSource.ToUint64();
	Req.m_UidEnemy = pCombatCnt->uidEnemy.ToUint64();
	Req.m_bActive = true;
	Req.m_bWin = pCombatResult->m_bWin;
	Req.m_RecordBuf.m_size = pCombatCnt->ob.Size();
	Req.m_BufLen = pCombatCnt->ob.Size();

	OBuffer6k ob;
	ob << Req << pCombatCnt->ob;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Insert_MyChallengeRecord, ob.TakeOsb(), 0, 0);

	SDB_Insert_MyChallengeRecord Req1;
	Req1.m_UidUser = pCombatCnt->uidEnemy.ToUint64();
	Req1.m_UidEnemy = pCombatCnt->uidSource.ToUint64();
	Req1.m_bActive = false;
	Req1.m_bWin = !pCombatResult->m_bWin;
	Req1.m_RecordBuf.m_size = pCombatCnt->ob.Size();
	Req1.m_BufLen = pCombatCnt->ob.Size();

	OBuffer6k ob1;
	ob1 << Req1 << pCombatCnt->ob;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Insert_MyChallengeRecord, ob1.TakeOsb(), 0, 0);

	if( pCombatResult->m_bWin){
		this->Forward(pActor, pEnemy, true, pCombatCnt->ob);
	}else{
		this->Forward(pActor, pEnemy, false, pCombatCnt->ob);
	}
}

//购买战斗次数
void Challenge::BuyCombatNum(IActor * pActor)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	SC_BuyChallengeNum Rsp;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( pActor->GetCrtProp(enCrtProp_ActorMoney) < GameParam.m_BuyChallengeNumGodStone ){
		Rsp.m_Result = enDouFaRetCode_ErrNoGodStone;
	} else {
		pDouFaPart->SetMaxChallengeToday(pDouFaPart->MaxChallengeNum() + GameParam.m_BuyCanAddChellengeNum);

		Rsp.m_MaxCombatNum = pDouFaPart->MaxChallengeNum();

		Rsp.m_CanCombatNum = pDouFaPart->MaxChallengeNum() - pDouFaPart->GetChallengeNum();

		pActor->AddCrtPropNum(enCrtProp_ActorMoney, -GameParam.m_BuyChallengeNumGodStone);
	}

	OBuffer1k ob;
	ob << DouFaHeader(enDouFaCmd_BuyCombatNum, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

