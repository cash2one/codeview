#include "Rank.h"
#include "ISyndicateMgr.h"
#include "IBasicService.h"
#include "RelationServer.h"
#include "ISyndicate.h"
#include "IBasicService.h"
#include "IActor.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IEventServer.h"
#include "IConfigServer.h"
#include "DMsgSubAction.h"
#include "IDouFaPart.h"


Rank::Rank()
{
	m_bInitSynRank = false;
	m_bInitSynWarRank = false;

}

Rank::~Rank()
{
}


bool Rank::Create()
{
	//更新一次排行数据
	this->FlushUserRank();
	this->FlushDuoBaoRank();

	UINT32 nFlushTimeNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_RankFlushTimeNum;

	g_pGameServer->GetTimeAxis()->SetTimer(enTimer_Flush,this, nFlushTimeNum * 1000,"Rank::Create");

	//先订阅玩家创建的消息
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return false;
	}
	pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"Rank::Create");

	//订阅创建帮派的消息
	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddSyn);

	pEnventServer->AddListener(this, msgID ,enEventSrcType_Actor, 0 ,"Rank::Create");

	//订阅解散帮派的消息
	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_JieSanSyn);

	pEnventServer->AddListener(this, msgID ,enEventSrcType_Actor, 0 ,"Rank::Create");

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Rank,this);;
}

void Rank::Close()
{
}


//收到MSG_ROOT消息
void Rank::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (Rank::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enRankCmd_Max] = 
	{
		&Rank::OpenRank,
		&Rank::ClickUser,
	};

	if(nCmd >= enFriendCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

void Rank::OnTimer(UINT32 timerID)
{
	if( timerID == enTimer_Flush)
	{
		//刷新排行数据
		this->UpdateRankData();
	}
}

// nRetCode: 取值于 enDBRetCode
void Rank::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_Get_UserRank:
		{
			HandleLoadUserRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_MyRank:
		{
			HandleClickMyRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_ChallengeRank:
		{
			HandleOpenChallengRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_MyChallengeRank:
		{
			HandleMyChallengeRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Insert_ChallengeRank:
		{
			this->HandleOnlyGetMyRankChallenge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_DuoBaoRank:
		{
			HandleLoadDuoBaoRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_MyDuoBaoRank:
		{
			HandleClickMyDuoBaoRank(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
		   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	    }
		break;
	}
}

void Rank::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_ActoreCreate);

	if( msgID == EventData.m_MsgID){
	
		SS_ActoreCreateContext * pCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == pCreateContext){
			return;
		}

		//插入到玩家排行
		if( !pCreateContext->m_bNewUser){
			return;
		}

		SDB_Insert_UserRank Req;
		Req.m_uidUser = pCreateContext->m_uidActor;

		OBuffer1k ob;
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Insert_UserRank, ob.TakeOsb(), 0, 0);

		//插入到夺宝排行
		SDB_Insert_DuoBaoRank ReqDuoBao;
		ReqDuoBao.m_uidUser = pCreateContext->m_uidActor;

		ob.Reset();
		ob << ReqDuoBao;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Insert_DuoBaoRank, ob.TakeOsb(), 0, 0);


		IActor * pCreater = g_pGameServer->GetGameWorld()->FindActor(UID(pCreateContext->m_uidActor));
		if( 0 == pCreater){
			return;
		}

		//插入到挑战排行
		SDB_Insert_ChallengeRank ReqChallenge;
		ReqChallenge.m_UidUser = pCreateContext->m_uidActor;
		strncpy(ReqChallenge.m_Name, pCreater->GetName(), sizeof(ReqChallenge.m_Name));

		ob.Reset();
		ob << ReqChallenge;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Insert_ChallengeRank, ob.TakeOsb(), this, pCreateContext->m_uidActor);
		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_AddSyn);
	if( msgID == EventData.m_MsgID){

		SS_AddSyn * pAddSyn = (SS_AddSyn *)EventData.m_pContext;
		
		if( 0 == pAddSyn || !pAddSyn->m_bCreate){
			return;
		}

		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return ;
		}

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pAddSyn->m_SynID);
		if( 0 == pSyndicate){
			return;
		}

		SynRankSynInfo SynInfo;

		SynInfo.m_Rank = m_vectSynRank.size() + 1;
		SynInfo.m_SynID = pSyndicate->GetSynID();
		SynInfo.m_SynLevel = pSyndicate->GetSynLevel();
		SynInfo.m_SynMemberNum = pSyndicate->GetSynMemberNum();
		strncpy(SynInfo.m_LeaderName, pSyndicate->GetLeaderName(), sizeof(SynInfo.m_LeaderName));
		strncpy(SynInfo.m_SynName, pSyndicate->GetSynName(), sizeof(SynInfo.m_SynName));

		m_vectSynRank.push_back(SynInfo);

		SynRankSynWarInfo SynWarInfo;

		SynWarInfo.m_Rank			= m_vectSynWarRank.size() + 1;
		SynWarInfo.m_SynID			= pSyndicate->GetSynID();
		SynWarInfo.m_SynLevel		= pSyndicate->GetSynLevel();
		SynWarInfo.m_SynMemberNum	= pSyndicate->GetSynMemberNum();
		SynWarInfo.m_SynWarAbility	= pSyndicate->GetSynWarAbility();
		SynWarInfo.m_SynWarScore	= pSyndicate->GetSynWarTotalScore();
		SynWarInfo.m_bWin			= pSyndicate->GetWinFail();
		strncpy(SynWarInfo.m_SynName, pSyndicate->GetSynName(), sizeof(SynWarInfo.m_SynName));
		strncpy(SynWarInfo.m_LeaderName, pSyndicate->GetLeaderName(), sizeof(SynWarInfo.m_LeaderName));
		strncpy(SynWarInfo.m_PreEnemySynName,pSyndicate->GetPreEnemySynName(),sizeof(SynWarInfo.m_PreEnemySynName));

		m_vectSynWarRank.push_back(SynWarInfo);
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_JieSanSyn);
	if( msgID == EventData.m_MsgID){

		SS_JieSanSyn * pJieSanSyn = (SS_JieSanSyn *)EventData.m_pContext;
		if(pJieSanSyn == 0){
			return;
		}

		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return ;
		}

		this->FlushSynRank();
		this->FlushSynWarRank();
	}
	
}

//更新帮派排行榜
void Rank::FlushSynRank()
{
	m_vectSynRank.clear();

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	std::vector<ISyndicate *> vectSynRank;

	pSynMgr->GetSynRank(vectSynRank);

	for( int i = 0; i < vectSynRank.size(); ++i)
	{
		ISyndicate * pSyndicate = vectSynRank[i];
		if( 0 == pSyndicate){
			continue;
		}

		SynRankSynInfo SynInfo;

		SynInfo.m_Rank			= i + 1;
		SynInfo.m_SynID			= pSyndicate->GetSynID();
		SynInfo.m_SynLevel		= pSyndicate->GetSynLevel();
		SynInfo.m_SynMemberNum  = pSyndicate->GetSynMemberNum();
		strncpy(SynInfo.m_SynName, pSyndicate->GetSynName(), sizeof(SynInfo.m_SynName));
		strncpy(SynInfo.m_LeaderName, pSyndicate->GetLeaderName(), sizeof(SynInfo.m_LeaderName));

		m_vectSynRank.push_back(SynInfo);
	}
}


//更新帮战排行榜
void Rank::FlushSynWarRank()
{
	m_vectSynWarRank.clear();

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	std::vector<ISyndicate *> vectSynWarRank;

	pSynMgr->GetSynWarRank(vectSynWarRank);

	for( int i = 0; i < vectSynWarRank.size(); ++i)
	{
		ISyndicate * pSyndicate = vectSynWarRank[i];
		if( 0 == pSyndicate){
			continue;
		}

		SynRankSynWarInfo SynWarInfo;

		SynWarInfo.m_Rank			= i + 1;
		SynWarInfo.m_SynID			= pSyndicate->GetSynID();
		SynWarInfo.m_SynLevel		= pSyndicate->GetSynLevel();
		SynWarInfo.m_SynMemberNum	= pSyndicate->GetSynMemberNum();
		SynWarInfo.m_SynWarAbility	= pSyndicate->GetSynWarAbility();
		SynWarInfo.m_SynWarScore	= pSyndicate->GetSynWarTotalScore();
		SynWarInfo.m_bWin			= pSyndicate->GetWinFail();
		strncpy(SynWarInfo.m_SynName, pSyndicate->GetSynName(), sizeof(SynWarInfo.m_SynName));
		strncpy(SynWarInfo.m_LeaderName, pSyndicate->GetLeaderName(), sizeof(SynWarInfo.m_LeaderName));
		strncpy(SynWarInfo.m_PreEnemySynName,pSyndicate->GetPreEnemySynName(),sizeof(SynWarInfo.m_PreEnemySynName));

		m_vectSynWarRank.push_back(SynWarInfo);
		
	}

	m_bInitSynWarRank = true;
}

//打开排行榜
void Rank::OpenRank(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_OpenRank_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	if( Req.m_BeginRank == 0){
		ClickMyRand(pActor, Req.m_Num,Req.m_RankType);
		return;
	}

	switch(Req.m_RankType)
	{
	case enRank_Type_Level:
		{
			this->OpenUserRank(pActor, Req.m_BeginRank, Req.m_Num);
		}
		break;
	case enRank_Type_Syn:
		{
			this->OpenSynRank(pActor, Req.m_BeginRank, Req.m_Num);
		}
		break;
	case enRank_Type_Challenge:
		{
			this->OpenChallengeRank(pActor, Req.m_BeginRank, Req.m_Num);
		}
		break;
	case enRank_Type_SynWar:
		{
			this->OpenSynWarRank(pActor,Req.m_BeginRank,Req.m_Num);
		}
		break;
	case enRank_Type_DuoBao:
		{
			this->OpenDuoBaoRank(pActor,Req.m_BeginRank,Req.m_Num);
		}
		break;
	default:
		{
			TRACE("<error> %s : %d 行 客户端排行类型参数有错!!,排行类型=%d", __FUNCTION__, __LINE__, Req.m_RankType);
		}
		break;
	}
}

//打开玩家排行
void Rank::OpenUserRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum)
{
	SDB_Get_UserRank Req;
	Req.m_BeginRank = nBeginRank;
	Req.m_Num		= nNum;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_UserRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
}


//打开夺宝排行
void Rank::OpenDuoBaoRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum)
{
	SDB_Get_DuoBaoRank Req;
	Req.m_BeginRank = nBeginRank;
	Req.m_Num		= nNum;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_DuoBaoRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
}

//打开帮派排行
void Rank::OpenSynRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum)
{
	if( !m_bInitSynRank){
		this->FlushSynRank();

		m_bInitSynRank = true;
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_Syn;
	Rsp.m_SynID    = pActor->GetSynID();
	Rsp.m_Num	   = nNum;
	
	if( m_vectSynRank.size() < nBeginRank){
		Rsp.m_Num = 0;
	}else{
		if( (nBeginRank + nNum - 1) > m_vectSynRank.size()){
			Rsp.m_Num = m_vectSynRank.size() - nBeginRank + 1;
		}
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(SynRankSynInfo)) << Rsp;

	for( int i = nBeginRank - 1; i < (nBeginRank + Rsp.m_Num - 1); ++i)
	{
		ob << m_vectSynRank[i];
	}

	pActor->SendData(ob.TakeOsb());
}


//打开帮战排行
void Rank::OpenSynWarRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum)
{
	if( !m_bInitSynWarRank){
		this->FlushSynWarRank();
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_SynWar;
	Rsp.m_SynID    = pActor->GetSynID();
	Rsp.m_Num	   = nNum;
	
	if( m_vectSynWarRank.size() < nBeginRank){
		Rsp.m_Num = 0;
	}else{
		if( (nBeginRank + nNum - 1) > m_vectSynWarRank.size()){
			Rsp.m_Num = m_vectSynWarRank.size() - nBeginRank + 1;
		}
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(SynRankSynWarInfo)) << Rsp;

	for( int i = nBeginRank - 1; i < (nBeginRank + Rsp.m_Num - 1); ++i)
	{
		ob << m_vectSynWarRank[i];
	}

	pActor->SendData(ob.TakeOsb());
}


//打开挑战排行榜
void Rank::OpenChallengeRank(IActor * pActor, UINT32 nBeginRank, UINT16 nNum)
{
	SDB_Get_ChallengeRank Req;
	
	Req.m_BeginPos = nBeginRank;
	Req.m_Num	   = nNum;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_ChallengeRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
}

void Rank::HandleLoadUserRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_Level;
	Rsp.m_SynID	   = pActor->GetSynID();

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_UserRankData);
	}

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(UserRankActorInfo)) << Rsp;

	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_UserRankData UserInfo;
		RspIb >> UserInfo;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		UserRankActorInfo ActorInfo;
		ActorInfo.m_Rank = UserInfo.m_Rank;
		ActorInfo.m_Exp  = UserInfo.m_Exp;
		ActorInfo.m_Layer = UserInfo.m_Layer;
		ActorInfo.m_Level = UserInfo.m_Level;
		strncpy(ActorInfo.m_Name, (char *)UserInfo.m_Name, sizeof(ActorInfo.m_Name));
		ActorInfo.m_uidUser = UID(UserInfo.m_uidUser);
		ActorInfo.m_vipLevel = UserInfo.m_vipLevel;
		
		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(UserInfo.m_uidUser));
		if( 0 == pSyndicate){
			memset(ActorInfo.m_SynName, 0, sizeof(ActorInfo.m_SynName));
		}else{
			strncpy(ActorInfo.m_SynName, pSyndicate->GetSynName(), sizeof(ActorInfo.m_SynName));
		}

		ob << ActorInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

//更新玩家排行榜
void Rank::FlushUserRank()
{
	SDB_Update_UserRank Req;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Update_UserRank, ob.TakeOsb(), 0, 0);
}

//从数据库加载夺宝排行
void Rank::HandleLoadDuoBaoRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_DuoBao;
	Rsp.m_SynID	   = pActor->GetSynID();

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_DuoBaoRankData);
	}

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(DuoBaoRankActorInfo)) << Rsp;

	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_DuoBaoRankData DuoBaoInfo;
		RspIb >> DuoBaoInfo;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		DuoBaoRankActorInfo ActorInfo;
		ActorInfo.m_Rank = DuoBaoInfo.m_Rank;
		ActorInfo.m_CombatAbility  = DuoBaoInfo.m_CombatAbility;
		ActorInfo.m_DuoBaoLevel = DuoBaoInfo.m_DuoBaoLevel;
		ActorInfo.m_Level = DuoBaoInfo.m_Level;
		strncpy(ActorInfo.m_Name, (char *)DuoBaoInfo.m_Name, sizeof(ActorInfo.m_Name));
		ActorInfo.m_uidUser = UID(DuoBaoInfo.m_uidUser);
		ActorInfo.m_vipLevel = DuoBaoInfo.m_VipLevel;
		
		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(DuoBaoInfo.m_uidUser));
		if( 0 == pSyndicate){
			memset(ActorInfo.m_SynName, 0, sizeof(ActorInfo.m_SynName));
		}else{
			strncpy(ActorInfo.m_SynName, pSyndicate->GetSynName(), sizeof(ActorInfo.m_SynName));
		}

		ob << ActorInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

//更新夺宝排行榜
void Rank::FlushDuoBaoRank()
{
	SDB_Update_DuoBaoRank Req;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Update_DuoBaoRank, ob.TakeOsb(), 0, 0);
}

//点击我的排行
void Rank::ClickMyRand(IActor * pActor, INT16 nNum, enRank_Type Rank_Type)
{
	if( Rank_Type == enRank_Type_Level){
		//玩家排行榜
		SDB_Get_MyRank Req;
		Req.m_uidUser = pActor->GetUID().ToUint64();
		Req.m_Num = nNum;

		OBuffer1k ob;
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_MyRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
		return;
	}else if( Rank_Type == enRank_Type_Syn){
		//帮派排行榜
		this->OpenMySynRank(pActor, nNum);
	}else if(Rank_Type == enRank_Type_SynWar){
		//帮战排行榜
		this->OpenMySynWarRank(pActor,nNum);
	
	}else if(Rank_Type == enRank_Type_DuoBao){
		//夺宝排行榜
		SDB_Get_MyDuoBaoRank Req;
		Req.m_uidUser = pActor->GetUID().ToUint64();
		Req.m_Num = nNum;

		OBuffer1k ob;
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_MyDuoBaoRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
		return;
	
	}else if( Rank_Type == enRank_Type_Challenge){
		//挑战排行榜
		SDB_Get_MyChallengeRank Req;

		Req.m_uidUser = pActor->GetUID().ToUint64();
		Req.m_Num	  = nNum;

		OBuffer1k ob;
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_MyChallengeRank, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
	}
}

void Rank::HandleClickMyRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;

	Rsp.m_RankType = enRank_Type_Level;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_UserRankData);
	}

	Rsp.m_SynID = pActor->GetSynID();

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(UserRankActorInfo)) << Rsp;
		
	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_UserRankData UserData;
		RspIb >> UserData;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		UserRankActorInfo ActorInfo;
		ActorInfo.m_Rank = UserData.m_Rank;
		ActorInfo.m_Exp  = UserData.m_Exp;
		ActorInfo.m_Layer = UserData.m_Layer;
		ActorInfo.m_Level = UserData.m_Level;
		strncpy(ActorInfo.m_Name, (char *)UserData.m_Name, sizeof(ActorInfo.m_Name));
		ActorInfo.m_uidUser = UID(UserData.m_uidUser);
		ActorInfo.m_vipLevel = UserData.m_vipLevel;
		
		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(UserData.m_uidUser));
		if( 0 == pSyndicate){
			memset(ActorInfo.m_SynName, 0, sizeof(ActorInfo.m_SynName));
		}else{
			strncpy(ActorInfo.m_SynName, pSyndicate->GetSynName(), sizeof(ActorInfo.m_SynName));
		}

		ob << ActorInfo;
	}

	pActor->SendData(ob.TakeOsb());
}


void Rank::HandleClickMyDuoBaoRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;

	Rsp.m_RankType = enRank_Type_DuoBao;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_DuoBaoRankData);
	}

	Rsp.m_SynID = pActor->GetSynID();

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(DuoBaoRankActorInfo)) << Rsp;
		
	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_DuoBaoRankData DuoBaoData;
		RspIb >> DuoBaoData;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		DuoBaoRankActorInfo ActorInfo;
		ActorInfo.m_Rank = DuoBaoData.m_Rank;
		ActorInfo.m_CombatAbility  = DuoBaoData.m_CombatAbility;
		ActorInfo.m_DuoBaoLevel = DuoBaoData.m_DuoBaoLevel;
		ActorInfo.m_Level = DuoBaoData.m_Level;
		strncpy(ActorInfo.m_Name, (char *)DuoBaoData.m_Name, sizeof(ActorInfo.m_Name));
		ActorInfo.m_uidUser = UID(DuoBaoData.m_uidUser);
		ActorInfo.m_vipLevel = DuoBaoData.m_VipLevel;
		
		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(DuoBaoData.m_uidUser));
		if( 0 == pSyndicate){
			memset(ActorInfo.m_SynName, 0, sizeof(ActorInfo.m_SynName));
		}else{
			strncpy(ActorInfo.m_SynName, pSyndicate->GetSynName(), sizeof(ActorInfo.m_SynName));
		}

		ob << ActorInfo;
	}

	pActor->SendData(ob.TakeOsb());
}


//点击玩家，弹出玩家帮派信息
void Rank::ClickUser(IActor * pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ClickUser_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 获取客户端数据长度有误！", __FUNCTION__, __LINE__);
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_ClickUser_Rsp Rsp;
	OBuffer1k ob;

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(Req.m_uidUser));
	if( 0 == pSyndicate){
		Rsp.m_SynID = INVALID_SYN_ID;
		ob << RankHeader(enRankCmd_ClickUser, sizeof(Rsp)) << Rsp;
	}else{
		Rsp.m_SynID = pSyndicate->GetSynID();
		
		UserSynInfo SynInfo;
		strncpy(SynInfo.m_szSynName, pSyndicate->GetSynName(), sizeof(SynInfo.m_szSynName));
		SynInfo.m_SynMemberNum = pSyndicate->GetSynMemberNum();
		strncpy(SynInfo.m_szSynLeaderName, pSyndicate->GetLeaderName(), sizeof(SynInfo.m_szSynLeaderName));
		SynInfo.m_SynLevel = pSyndicate->GetSynLevel();
		SynInfo.m_SynWarAbility = pSyndicate->GetSynWarAbility();
		
		ob << RankHeader(enRankCmd_ClickUser, sizeof(Rsp) + sizeof(SynInfo)) << Rsp << SynInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

void Rank::HandleOpenChallengRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_Challenge;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo);
	}

	Rsp.m_SynID = pActor->GetSynID();

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(ChallengeRankInfo)) << Rsp;

	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_ChallengeRankInfo DBInfo;
		RspIb >> DBInfo;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		ChallengeRankInfo RankInfo;

		RankInfo.m_Rank = DBInfo.m_Rank;

		RankInfo.m_uidUser = UID(DBInfo.m_uidUser);

		IActor * pTmpUser = g_pGameServer->GetGameWorld()->FindActor(UID(DBInfo.m_uidUser));
		if( 0 == pTmpUser){
			RankInfo.m_Layer = DBInfo.m_Layer;
			RankInfo.m_Level = DBInfo.m_Level;
			RankInfo.m_ChallengeNum = DBInfo.m_ChallengeNum;
			RankInfo.m_vipLevel		= DBInfo.m_VipLevel;
		}else{
			RankInfo.m_Layer = pTmpUser->GetCrtProp(enCrtProp_ActorLayer);
			RankInfo.m_Level = pTmpUser->GetCrtProp(enCrtProp_Level);
			RankInfo.m_vipLevel = pTmpUser->GetCrtProp(enCrtProp_TotalVipLevel);

			IDouFaPart * pDouFaPart = pTmpUser->GetDouFaPart();
			if( 0 == pDouFaPart){
				return;
			}

			RankInfo.m_ChallengeNum = pDouFaPart->GetChallengeNum();
		}

		strncpy(RankInfo.m_Name, DBInfo.m_Name, sizeof(RankInfo.m_Name));

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(DBInfo.m_uidUser));
		if( 0 == pSyndicate){
			memset(RankInfo.m_SynName, 0, sizeof(RankInfo.m_SynName));
		}else{
			strncpy(RankInfo.m_SynName, pSyndicate->GetSynName(), sizeof(RankInfo.m_SynName));
		}

		ob << RankInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

//打开我的帮派排行榜
void Rank::OpenMySynRank(IActor * pActor, INT16 nNum)
{
	//帮派排行榜
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	if( !m_bInitSynRank){
		this->FlushSynRank();

		m_bInitSynRank = true;
	}


	UINT32 mySynRank = 0;

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pActor->GetUID());
	if( 0 != pSyndicate){
	{
		TSynID SynID = pSyndicate->GetSynID();
		for( int i = 0; i < m_vectSynRank.size(); ++i)
	
			if( SynID == m_vectSynRank[i].m_SynID){
				mySynRank = i + 1;
				break;
			}
		}
	}

	INT32 nBeginRank = mySynRank - nNum / 2;

	if( nBeginRank <= 0){
		nBeginRank = 1;
	}

	if( (nBeginRank + nNum - 1) > m_vectSynRank.size()){
		nNum = m_vectSynRank.size() - nBeginRank + 1;
	}
	
	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_Syn;
	Rsp.m_SynID	   = pActor->GetSynID();
	Rsp.m_Num      = nNum;

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + nNum * sizeof(SynRankSynInfo)) << Rsp;

	for( int i = nBeginRank - 1; i < (nBeginRank + nNum - 1); ++i)
	{
		ob << m_vectSynRank[i];
	}

	pActor->SendData(ob.TakeOsb());	
}


//打开我的帮战排行榜
void Rank::OpenMySynWarRank(IActor * pActor, INT16 nNum)
{
	//帮战排行榜
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	if( !m_bInitSynWarRank){
		this->FlushSynWarRank();

		m_bInitSynWarRank = true;
	}


	UINT32 mySynWarRank = 0;

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pActor->GetUID());
	if( 0 != pSyndicate){
	{
		TSynID SynID = pSyndicate->GetSynID();
		for( int i = 0; i < m_vectSynWarRank.size(); ++i)
	
			if( SynID == m_vectSynWarRank[i].m_SynID){
				mySynWarRank = i + 1;
				break;
			}
		}
	}

	INT32 nBeginRank = mySynWarRank - nNum / 2;

	if( nBeginRank <= 0){
		nBeginRank = 1;
	}

	if( (nBeginRank + nNum - 1) > m_vectSynWarRank.size()){
		nNum = m_vectSynWarRank.size() - nBeginRank + 1;
	}
	
	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_SynWar;
	Rsp.m_SynID	   = pActor->GetSynID();
	Rsp.m_Num      = nNum;

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + nNum * sizeof(SynRankSynWarInfo)) << Rsp;

	for( int i = nBeginRank - 1; i < (nBeginRank + nNum - 1); ++i)
	{
		ob << m_vectSynWarRank[i];
	}

	pActor->SendData(ob.TakeOsb());	
}

//更新排行数据
void Rank::UpdateRankData()
{
	//刷新
	this->FlushUserRank();

	//先更新下帮派战力
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();

	if( 0 == pSynMgr)
	{
		return;
	}

	pSynMgr->UpdateSynCombatAbility();

	this->FlushSynRank();

	this->FlushSynWarRank();

	this->FlushDuoBaoRank();
}

void Rank::HandleMyChallengeRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return;
	}

	SC_OpenRank_Rsp Rsp;
	Rsp.m_RankType = enRank_Type_Challenge;

	if( OutParam.retCode != 0){
		Rsp.m_Num = 0;
	}else{
		Rsp.m_Num = RspIb.Remain() / sizeof(SDB_ChallengeRankInfo);
	}

	Rsp.m_SynID = pActor->GetSynID();

	OBuffer4k ob;
	ob << RankHeader(enRankCmd_OpenRank, sizeof(Rsp) + Rsp.m_Num * sizeof(ChallengeRankInfo)) << Rsp;

	for( int i = 0; i < Rsp.m_Num; ++i)
	{
		SDB_ChallengeRankInfo DBInfo;
		RspIb >> DBInfo;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		ChallengeRankInfo RankInfo;

		RankInfo.m_Rank = DBInfo.m_Rank;
		RankInfo.m_uidUser = UID(DBInfo.m_uidUser);

		IActor * pTmpUser = g_pGameServer->GetGameWorld()->FindActor(UID(DBInfo.m_uidUser));
		if( 0 == pTmpUser){
			RankInfo.m_Layer = DBInfo.m_Layer;
			RankInfo.m_Level = DBInfo.m_Level;
			RankInfo.m_ChallengeNum = DBInfo.m_ChallengeNum;
			RankInfo.m_vipLevel		= DBInfo.m_VipLevel;
		}else{
			RankInfo.m_Layer = pTmpUser->GetCrtProp(enCrtProp_ActorLayer);
			RankInfo.m_Level = pTmpUser->GetCrtProp(enCrtProp_Level);
			RankInfo.m_vipLevel	= pTmpUser->GetCrtProp(enCrtProp_TotalVipLevel);

			IDouFaPart * pDouFaPart = pTmpUser->GetDouFaPart();
			if( 0 == pDouFaPart){
				return;
			}

			RankInfo.m_ChallengeNum = pDouFaPart->GetChallengeNum();
		}

		strncpy(RankInfo.m_Name, DBInfo.m_Name, sizeof(RankInfo.m_Name));

		ISyndicate * pSyndicate = pSynMgr->GetSyndicate(UID(DBInfo.m_uidUser));
		if( 0 == pSyndicate){
			memset(RankInfo.m_SynName, 0, sizeof(RankInfo.m_SynName));
		}else{
			strncpy(RankInfo.m_SynName, pSyndicate->GetSynName(), sizeof(RankInfo.m_SynName));
		}

		ob << RankInfo;
	}

	pActor->SendData(ob.TakeOsb());
}

//仅得到我的排行
void Rank::HandleOnlyGetMyRankChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if ( 0 == pActor){
		
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_MyRank  MyRank;
	RspIb >> RspHeader >> OutParam >> MyRank;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != 0){
		return;
	}

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart){
		
		return;
	}

	INT32	MaxChallengeNum = g_pGameServer->GetConfigServer()->GetMaxChallengeNum(MyRank.m_Rank);

	pDouFaPart->SetMaxChallengeToday(MaxChallengeNum);
}

