#include "IActor.h"
#include "SyndicateCombat.h"
#include "RelationServer.h"
#include "IBasicService.h"
#include "XDateTime.h"
#include "ISynPart.h"
#include "IGameScene.h"



SyndicateCombat::SyndicateCombat()
{
	//帮战是否开启
	m_bOpenAttack = false;

	//帮战是否开启宣战
	m_bOpenDeclare = false;
}


bool SyndicateCombat::Create()
{
	//g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_Control,this,60 * 1000,"XuanTianFuBen::Create");

	//是否在帮战时间
	UINT32 CurTime = CURRENT_TIME();

	if ( this->IsOpenCombatTime(CurTime) )
	{
		//得到帮战数据
		SDB_Get_SynWarInfo_Req DBWar;

		OBuffer1k ob;
		ob << DBWar;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarInfo,ob.TakeOsb(),this,0);


		//清除数据库帮战宣战数据
		SDB_Delete_SynWarDeclare_Req  DBDeleteDeclare;

		ob.Reset();
		ob << DBDeleteDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);
	}
	else if ( this->IsOpenDeclareTime(CurTime) )
	{
		//得到帮战宣战数据
		SDB_Get_SynWarDeclare_Req DBDeclare;

		OBuffer1k ob;
		ob << DBDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarDeclare,ob.TakeOsb(),this,0);

		//清除数据库帮战数据
		SDB_Delete_SynWarInfo_Req  DBDeleteWar;

		ob.Reset();
		ob << DBDeleteWar;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);	
	}
	else
	{
		//清除数据库帮战数据
		SDB_Delete_SynWarDeclare_Req  DBDeclare;

		OBuffer1k ob;
		ob << DBDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);

		//清除数据库帮战宣战数据
		SDB_Delete_SynWarInfo_Req  DBCombat;

		ob.Reset();
		ob << DBCombat;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);

		//设置下次帮战开启定时器
		UINT32 TimeNum = this->GetNextOpenWarTimeNum();

		g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartSynWar,this,TimeNum * 1000,"SyndicateCombat::Create");

		TRACE("设置下次帮战开启定时器值:%d", TimeNum);

		//设置下次宣战开始定时器
		TimeNum = this->GetNextOpenDeclareTimeNum();

		g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartDeclare,this,TimeNum * 1000,"SyndicateCombat::Create");

		TRACE("设置下次宣战开启定时器值:%d", TimeNum);
	}

	
	return true;
}

void SyndicateCombat::OnEvent(XEventData & EventData)
{
	if ( 0 == EventData.m_pContext )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	if ( EventData.m_MsgID == msgID )
	{
		SS_ActoreCreateContext * pActoreCreateCnt = (SS_ActoreCreateContext *)EventData.m_pContext;

		if ( 0 == pActoreCreateCnt )
			return;

		std::set<UID>::iterator iter = m_NoTakeForward.find(UID(pActoreCreateCnt->m_uidActor));

		if ( iter != m_NoTakeForward.end() ){

			UID  uidUser = *iter;

			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

			if ( 0 == pActor )
				return;

			const SGameServerConfigParam & ServerInfo = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

			ISynPart * pSynPart = pActor->GetSynPart();

			if ( 0 == pSynPart )
				return;

			pSynPart->AddGetCredit(ServerInfo.m_WinSynWarGetCredit);
			pActor->AddCrtPropNum(enCrtProp_SynCombatLevel, ServerInfo.m_WinSynWarGetSynWarLv);

			m_NoTakeForward.erase(iter);
		}

		std::map<UID, UID>::iterator it = m_mapCombat.find(UID(pActoreCreateCnt->m_uidActor));

		if ( it != m_mapCombat.end() ){

			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(it->second);

			IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(it->first);

			if ( 0 != pEnemy && 0 != pActor ){

				this->BeginCombat(pActor, pEnemy);
			}

			m_mapCombat.erase(it);
		}


		if ( m_NoTakeForward.size() == 0 && m_mapCombat.size() == 0 ){
			//移除监听
			IEventServer * pEnventServer = g_pGameServer->GetEventServer();
			if( 0 == pEnventServer){
				return;
			}

			pEnventServer->RemoveListener((IEventListener *)this, msgID, enEventSrcType_Actor, 0);
		}

		return;
	}

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if ( EventData.m_MsgID == msgID )
	{
		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		std::map<UID, TSceneID>::iterator iter = m_mapSceneID.find(UID(pActorDoing->m_uidActor));

		if ( iter == m_mapSceneID.end() )
			return;

		TSceneID SynWarSceneID = iter->second;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(iter->first);

		if ( 0 == pActor )
			return;

		if ( SynWarSceneID.ToID() == pActor->GetCrtProp(enCrtProp_SceneID) )
			//玩家在帮战中
			pActorDoing->m_ActorDoing = enActorDoing_SynWar;

		return;
	}
}

void SyndicateCombat::OnTimer(UINT32 timerID)
{
	//time_t CurTime = CURRENT_TIME();

	//tm * pTm = localtime(&CurTime);

	//const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//if ( GameParam.m_SynCombatDeclareTime.size() < 4 || GameParam.m_SynCombatTime.size() < 4 ){
	//	TRACE("<error> %s : %d Line 配置表帮战参数个数错误！！", __FUNCTION__, __LINE__);
	//	return;
	//}

	//if ( pTm->tm_hour < GameParam.m_SynCombatDeclareTime[0]
	//	|| (pTm->tm_hour ==  GameParam.m_SynCombatDeclareTime[0] && pTm->tm_min < GameParam.m_SynCombatDeclareTime[1]) )
	//{
	//	if ( m_bOpenDeclare ){
	//		//结束帮战宣战
	//		this->OverSynWarDeclare();
	//	}
	//} else {

	//	if (  pTm->tm_hour > GameParam.m_SynCombatDeclareTime[2]
	//		|| (pTm->tm_hour ==  GameParam.m_SynCombatDeclareTime[2] && pTm->tm_min > GameParam.m_SynCombatDeclareTime[3]) )
	//	{
	//		if ( m_bOpenDeclare ){
	//			//结束帮战宣战
	//			this->OverSynWarDeclare();
	//		}
	//	} else {
	//		if ( !m_bOpenDeclare ){
	//			//开始帮战宣战
	//			this->StartSynWarDeclare();
	//		}
	//	}	
	//}


	//if ( pTm->tm_hour < GameParam.m_SynCombatTime[0]
	//	|| (pTm->tm_hour ==  GameParam.m_SynCombatTime[0] && pTm->tm_min < GameParam.m_SynCombatTime[1]) )
	//{
	//	if ( m_bOpenAttack ){
	//		//结束帮战
	//		this->OverSynCombat();
	//	}
	//} else {

	//	if (  pTm->tm_hour > GameParam.m_SynCombatTime[2]
	//		|| (pTm->tm_hour ==  GameParam.m_SynCombatTime[2] && pTm->tm_min > GameParam.m_SynCombatTime[3]) )
	//	{
	//		if ( m_bOpenAttack ){
	//			//结束帮战
	//			this->OverSynCombat();
	//		}
	//	} else {
	//		if ( !m_bOpenAttack ){
	//			//开始帮战
	//			this->StartSynCombat();
	//		}
	//	}
	//}

	switch(timerID)
	{
	case enSynWarTimer_StartSynWar:
		{
			this->StartSynCombat();

			g_pGameServer->GetTimeAxis()->KillTimer(enSynWarTimer_StartSynWar, this);
		}
		break;
	case enSynWarTimer_OverSynWar:
		{
			this->OverSynCombat();

			g_pGameServer->GetTimeAxis()->KillTimer(enSynWarTimer_OverSynWar, this);
		}
		break;
	case enSynWarTimer_StartDeclare:
		{
			this->StartSynWarDeclare();

			g_pGameServer->GetTimeAxis()->KillTimer(enSynWarTimer_StartDeclare, this);
		}
		break;
	case enSynWarTimer_OverDeclare:
		{
			this->OverSynWarDeclare();

			g_pGameServer->GetTimeAxis()->KillTimer(enSynWarTimer_OverDeclare, this);
		}
		break;
	}

	//帮战开启时，每10分钟保存一次和帮派相关的帮战信息
	//if ( m_bOpenAttack && pTm->tm_min / 10 == 0 )
	//{
	//	MAP_SYN_PAIR::iterator iter = m_SynPair.begin();

	//	for ( int nadd = 0; iter != m_SynPair.end() ; ++iter,++nadd )
	//	{
	//		SDB_Update_SynWarInfo_Req DBReq;

	//		DBReq.m_SynID = iter->first;
	//		DBReq.m_EnemySynID = iter->second;

	//		std::set<TSynID>::iterator itWin = m_FastWinSyn.find(iter->first);

	//		if ( itWin != m_FastWinSyn.end() )
	//		{
	//			DBReq.m_bWin = true;
	//		}
	//		else
	//		{
	//			DBReq.m_bWin = false;
	//		}

	//		std::hash_map<TSynID, VECT_MEMBER>::iterator itPos = m_SynMemberPos.find(iter->first);

	//		if ( itPos == m_SynMemberPos.end() )
	//			continue;

	//		UID * pUid = (UID *)&DBReq.m_MemberPos;

	//		VECT_MEMBER & vecMember = itPos->second;

	//		for ( int i = 0; i < vecMember.size() && i < MAX_SYNMEMBER_NUM; ++i, ++pUid )
	//		{
	//			(*pUid) = vecMember[i];
	//		}

	//		OBuffer1k ob;
	//		ob << DBReq;
	//		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB + nadd,enDBCmd_Update_SynWarInfo,ob.TakeOsb(),0,0);
	//	}
	//}
}

//打开帮战界面
void SyndicateCombat::OpenSynCombat(IActor * pActor)
{
	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 == pSyn )
		return;

	ISyndicateMember * pSynMember = pSyn->GetSynMember(pActor->GetUID());

	if ( 0 == pSynMember )
	{
		TRACE("<error> %s ; %d Line 找不到帮派成员中！！玩家%s", __FUNCTION__, __LINE__, pActor->GetUID().ToString());
		return;
	}

	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	SC_OpenSynWar Rsp;
	OBuffer1k ob;

	SSynCombatParam Param;

	g_pGameServer->GetConfigServer()->GetSynCombatParam(pActor->GetCrtProp(enCrtProp_SynCombatLevel), Param);

	Rsp.m_CreditUp = Param.m_CreditUp + pActor->GetVipValue(enVipType_AddSynWarCreditUp);

	Rsp.m_GetCreditWeak = pSynPart->GetGetNumWeek();

	Rsp.m_SynWarLevel = pSynMember->GetSynWarLv();

	Rsp.m_bOpenCombat = m_bOpenAttack;

	if ( pSynMember->GetPosition() != enumSynPosition_Leader )
	{
		Rsp.m_bOpenDeclare = false;
	}
	else
	{
		Rsp.m_bOpenDeclare = m_bOpenDeclare;
	}
	

	MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyn->GetSynID());

	if ( iter != m_SynPair.end() ){
		//有配对帮派
		Rsp.m_bHaveVsSyn = true;
		
		ISyndicate * pVsSyn = g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSyndicate(iter->second);

		if ( 0 == pVsSyn )
			return;

		VsSynName SynName;

		strncpy(SynName.m_VsSynName, pVsSyn->GetSynName(), sizeof(SynName.m_VsSynName));

		ob << SyndicateHeader(enSyndicateCmd_OpenSynWar, sizeof(Rsp) + sizeof(SynName)) << Rsp << SynName;
	} else {
		ob << SyndicateHeader(enSyndicateCmd_OpenSynWar, sizeof(Rsp)) << Rsp;
	}

	pActor->SendData(ob.TakeOsb());
}

//打开宣战页面
void SyndicateCombat::OpenDeclareWar(IActor * pActor)
{
	ISyndicate * pMeSyn = pActor->GetSyndicate();

	if ( 0 == pMeSyn )
		return;

	SC_OpenDeclareWar Rsp;

	std::vector<ISyndicate *> vectSynRank;

	g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSynRank(vectSynRank);

	OBuffer4k ob;

	for ( int i = 0; i < vectSynRank.size(); ++i )
	{
		ISyndicate * pSyn = vectSynRank[i];

		if ( 0 == pSyn || pSyn == pMeSyn )
			continue;

		MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyn->GetSynID());

		if ( iter != m_SynPair.end() )
			continue;

		VsSynInfo SynInfo;

		SynInfo.m_SynID = pSyn->GetSynID();
		SynInfo.m_SynLevel = pSyn->GetSynLevel();
		SynInfo.m_MemberNum = pSyn->GetSynMemberNum();
		SynInfo.m_Rank	= i + 1;
		SynInfo.m_SynWarAbility = pSyn->GetSynWarAbility();
		strncpy(SynInfo.m_Name, pSyn->GetSynName(), sizeof(SynInfo.m_Name));

		ob << SynInfo;

		++Rsp.m_SynNum;
	}

	MAP_SYN_APPLY::iterator iter = m_SynApply.find(pMeSyn->GetSynID());

	for ( ; iter != m_SynApply.end(); ++iter )
	{
		VECT_SYNID & vecSynID = iter->second;

		for ( int i = 0; i < vecSynID.size(); ++i )
		{
			ISyndicate * pSyn = g_pGameServer->GetSyndicateMgr()->GetSyndicate(vecSynID[i]);

			if ( 0 == pSyn || pSyn == pMeSyn )
				continue;

			ApplySynInfo SynInfo;

			SynInfo.m_SynID = vecSynID[i];
			SynInfo.m_SynWarAbility = pSyn->GetSynWarAbility();
			strncpy(SynInfo.m_Name, pSyn->GetSynName(), sizeof(SynInfo.m_Name));

			ob << SynInfo;

			++Rsp.m_ApplySynNum;
		}
	}

	OBuffer4k ob2;
	ob2 << SyndicateHeader(enSyndicateCmd_OpenDeclareWar, sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0 )
		ob2 << ob;

	pActor->SendData(ob2.TakeOsb());
}

//宣战
void SyndicateCombat::DeclareWar(IActor * pActor, TSynID SynID)
{
	SC_DeclareWar Rsp;

	do
	{
		//是否在宣战时间
		if ( !this->IsInDeclareWarTime() ){
			Rsp.m_Result = enSynRetCode_ErrNotInTime;
			break;
		}

		ISyndicate * pSyn = pActor->GetSyndicate();
		ISyndicateMember * pSynMenber = pSyn->GetSynMember(pActor->GetUID());

		if ( 0 == pSyn || 0 == pSynMenber)
			return;

		if (pSynMenber->GetPosition() != enumSynPosition_Leader)//( pSyn->GetLeaderUID() != pActor->GetUID() )
		{
			Rsp.m_Result = enSynRetCode_ErrNotLeaderDeclare;
			break;
		}

		ISyndicate * pSyn2 = g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSyndicate(SynID);

		if ( 0 == pSyn2 )
			return;

		//检测是否可以配对
		Rsp.m_Result = (enSynRetCode)this->CanPairJoin(pSyn, pSyn2);

		if ( enSynRetCode_OK != Rsp.m_Result )
			break;

		//是否已在向此帮派宣战过
		MAP_SYN_APPLY::iterator iter = m_SynApply.find(SynID);

		if ( iter != m_SynApply.end() ){

			VECT_SYNID & vecSynID = iter->second;

			for ( int i = 0; i < vecSynID.size(); ++i )
			{
				if ( vecSynID[i] == pSyn->GetSynID() ){
					Rsp.m_Result = enSynRetCode_ExistDeclareThis;
					break;
				}
			}
		}

		if ( enSynRetCode_OK != Rsp.m_Result )
			break;

		//OK,可以宣战
		this->OkDeclareWar(pSyn->GetSynID(), pSyn2->GetSynID());

	}while(0);

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_DeclareWar, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//接受宣战
void SyndicateCombat::AcceptDeclareWar(IActor * pActor, TSynID SynID)
{
	SC_AcceptDeclareWar Rsp;

	ISyndicate * pSyn1 = pActor->GetSyndicate();

	if ( 0 == pSyn1 )
		return;

	ISyndicate * pSyn2 = g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSyndicate(SynID);

	if ( 0 == pSyn2 )
		return;

	//检测是否可以配对
	Rsp.m_Result = (enSynRetCode)this->CanPairJoin(pSyn1, pSyn2);

	if ( enSynRetCode_OK == Rsp.m_Result ){
		//开始配对
		this->PairSynCombat(pSyn1, pSyn2);
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_AcceptDeclareWar, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//拒绝宣战
void SyndicateCombat::RefuseDeclareWar(IActor * pActor, TSynID SynID)
{
	SC_RefuseDeclareWar Rsp;

	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 == pSyn )
		return;

	MAP_SYN_APPLY::iterator iter = m_SynApply.find(pSyn->GetSynID());

	if ( iter != m_SynApply.end() ){
		
		VECT_SYNID & vec = iter->second;

		VECT_SYNID::iterator it = vec.begin();

		for ( ; it != vec.end() ; ++it )
		{
			if ( SynID == *it ){
				vec.erase(it);
				break;
			}
		}
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_RefuseDeclareWar, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//开始系统宣战
void SyndicateCombat::BeginSystemDeclareWar()
{
	std::vector<ISyndicate *> vecSyndicate;

	g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSynRank(vecSyndicate);

	//按名次顺序配对
	ISyndicate * pSynTmp = 0;

	for ( int i = 0; i < vecSyndicate.size(); ++i )
	{
		ISyndicate * pSyn = vecSyndicate[i];

		if ( 0 == pSyn )
			continue;

		if ( enSynRetCode_OK != this->CanJoinSynCombat(pSyn) )
			continue;

		if ( 0 == pSynTmp ){
			pSynTmp = pSyn;
			continue;
		} else {
			//配对
			this->PairSynCombat(pSyn, pSynTmp);
			pSynTmp = 0;
		}
	}
}

//进入帮战场景
void SyndicateCombat::EnterSynCombatScene(IActor * pActor)
{
	SC_EnterSynWar Rsp;
	OBuffer4k ob;

	do
	{
		if ( !m_bOpenAttack ){
			Rsp.m_Result = enSynRetCode_ErrNotInAttackTime;
			break;
		}

		ISyndicate * pSyndicate = pActor->GetSyndicate();

		if ( 0 == pSyndicate )
			return;

		MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyndicate->GetSynID());

		if ( iter == m_SynPair.end() ){
			Rsp.m_Result = enSynRetCode_ErrNoEnemySyn;
			break;
		}

		ISynPart * pSynPart = pActor->GetSynPart();

		if ( 0 == pSynPart )
			return;

		//if ( pSynPart->GetMaxJoinCombatNumToday() <= pSynPart->GetJoinCombatNumToday() ){
		//	Rsp.m_Result = enSynRetCode_ErrNoCombatNum;
		//	break;
		//}

		ISyndicate * pEnemySyn = g_pGameServer->GetSyndicateMgr()->GetSyndicate(iter->second);

		if ( 0 == pEnemySyn ){
			return;
		}

		if ( pActor->IsInDuoBao() )
		{
			Rsp.m_Result = enSynRetCode_ErrInDuoBao;
			break;			
		}

		if ( pActor->HaveTeam() )
		{
			Rsp.m_Result = enSynRetCode_ErrHaveTeam;
			break;
		}

		if ( pActor->IsInFastWaitTeam() )
		{
			Rsp.m_Result = enSynRetCode_ErrWaitTeam;
			break;			
		}

		IGameScene * pGameScene = 0;

		//得到场景
		std::map<UID, TSceneID>::iterator itSce = m_mapSceneID.find(pActor->GetUID());

		if ( itSce != m_mapSceneID.end() )
		{
			TSceneID SceneID = itSce->second;

			pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
		}

		if ( 0 == pGameScene )
		{
			const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

			TMapID SynWarMapID = ServerParam.m_SynWarMapID;

			pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(SynWarMapID);

			if ( 0 == pGameScene )
			{
				TRACE("<error> %s : %d Line 创建玄天摆怪场景失败！！地图ID = %d", __FUNCTION__, __LINE__, SynWarMapID);
				return;
			}

			m_mapSceneID[pActor->GetUID()] = pGameScene->GetSceneID();
		}

		if ( !pGameScene->EnterScene(pActor) ){
			Rsp.m_Result = (enSynRetCode)enSyndicateCmd_Max;
			g_pGameServer->GetGameWorld()->DeleteGameScene(pGameScene);
			break;
		}

		EnterSynWarInfo SynInfo;

		SynInfo.m_SynWarLevel = pActor->GetCrtProp(enCrtProp_SynCombatLevel);
		SynInfo.m_CanJoinCombatNum = pSynPart->GetMaxJoinCombatNumToday() - pSynPart->GetJoinCombatNumToday();
		SynInfo.m_MaxJoinCombatNum = pSynPart->GetMaxJoinCombatNumToday();
		SynInfo.m_SynScore		   = pSyndicate->GetSynWarScore();
		SynInfo.m_EnemySynScore	   = pEnemySyn->GetSynWarScore();
		strncpy(SynInfo.m_EnemySynName, pEnemySyn->GetSynName(), sizeof(SynInfo.m_EnemySynName));
		SynInfo.m_CombatWinNum = 0;

		std::map<UID, UserSynWarInfo>::iterator it = m_KillHistory.find(pActor->GetUID());

		bool bNoKill = true;

		if ( it != m_KillHistory.end() ){
			bNoKill = false;
			//TRACE("<error> %s : %d Line 获取不到玩家帮战数据！！", __FUNCTION__, __LINE__);
			//return;
		}

		//UserSynWarInfo & UserInfo = it->second;

		std::hash_map<TSynID, VECT_MEMBER>::iterator itMem = m_SynMemberPos.find(pEnemySyn->GetSynID());

		if ( itMem == m_SynMemberPos.end() ){
			TRACE("<error> %s : %d Line 获取不到帮派的帮战的分布信息！！帮派ID = %d", __FUNCTION__,  __LINE__, pEnemySyn->GetSynID());
			return;
		}

		VECT_MEMBER & vecMember = itMem->second;
		
		SynInfo.m_Num = 0;

		OBuffer4k ob3;

		for ( int i = 0; i < vecMember.size(); ++i )
		{
			ISyndicateMember * pSynMember = pEnemySyn->GetSynMember(vecMember[i]);

			if ( 0 == pSynMember )
				continue;

			SynWarUserInfo SynUser;

			SynUser.m_Facade = pSynMember->GetFacade();
			SynUser.m_Level  = pSynMember->GetLevel();
			strncpy(SynUser.m_Name, pSynMember->GetSynMemberName(), sizeof(SynUser.m_Name));
			SynUser.m_uidUser = pSynMember->GetSynMemberUID();

			if ( bNoKill ){
				SynUser.m_bCombat = true;
			} else {
				UserSynWarInfo & UserInfo = it->second;

				if ( UserInfo.m_SynWarHitroy.get(i) ){
					SynUser.m_bCombat = false;
					++SynInfo.m_CombatWinNum;
				} else {
					SynUser.m_bCombat = true;
				}
			}

			++SynInfo.m_Num;

			ob3 << SynUser;
		}

		ob << SynInfo << ob3;

	}while(0);

	OBuffer4k ob2;
	ob2 << SyndicateHeader(enSyndicateCmd_EnterSynWar, sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0 )
		ob2 << ob;

	pActor->SendData(ob2.TakeOsb());
}

//购买战斗次数
void SyndicateCombat::BuyCombatNum(IActor * pActor)
{
	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	SC_BuyCombatNum Rsp;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( pActor->GetCrtProp(enCrtProp_ActorMoney) < GameParam.m_BuyCombatNumGodStone ){
		Rsp.m_Result = enSynRetCode_ErrNoGodStone;
	} else {
		pSynPart->SetMaxJoinCombatNumToday(pSynPart->GetMaxJoinCombatNumToday() + GameParam.m_BuyCanAddCombatNum);

		Rsp.m_MaxCombatNum = pSynPart->GetMaxJoinCombatNumToday();

		Rsp.m_CanCombatNum = pSynPart->GetMaxJoinCombatNumToday() - pSynPart->GetJoinCombatNumToday();

		pActor->AddCrtPropNum(enCrtProp_ActorMoney, -GameParam.m_BuyCombatNumGodStone);
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_BuyCombatNum, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//战斗
void SyndicateCombat::Combat(IActor * pActor, UID uidEnemy)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if ( 0 == pCombatPart )
		return;

	SC_SynWarCombat Rsp;

	//检测次数
	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	if ( pSynPart->GetMaxJoinCombatNumToday() <= pSynPart->GetJoinCombatNumToday() )
	{
		Rsp.m_Result = enSynRetCode_ErrNoCombatNum;
	}
	else if ( pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enSynRetCode_ErrInDuoBao;		
	}
	else if ( pActor->HaveTeam() )
	{
		Rsp.m_Result = enSynRetCode_ErrHaveTeam;
	}
	else if ( pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enSynRetCode_ErrWaitTeam;		
	}
	else
	{
		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);

		if ( 0 == pEnemy ){
			//装载
			if ( m_mapCombat.insert(std::map<UID, UID>::value_type(uidEnemy, pActor->GetUID())).second ){

				g_pGameServer->LoadActor(uidEnemy);

				if (  m_NoTakeForward.size() == 0 && m_mapCombat.size() == 1 ){
					//监听创建消息
					UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

					IEventServer * pEnventServer = g_pGameServer->GetEventServer();
					if( 0 == pEnventServer){
						return;
					}

					pEnventServer->AddListener((IEventListener *)this, msgID, enEventSrcType_Actor,0,"SyndicateCombat::Combat");	
				}
			}

			return;
		} else {
			//开始战斗
			this->BeginCombat(pActor, pEnemy);
		}	
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_SynWarCombat, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());	
}

//战斗结束了
void SyndicateCombat::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	SS_OnCombat EventCombat;

	EventCombat.CombatPlace = enCombatPlace_SynWar;
	EventCombat.bWin = pCombatResult->m_bWin;
	EventCombat.bIsTeam = pCombatCnt->bIsTeam;
	EventCombat.bHard = false;

	IActor * pBacker = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( 0 == pBacker )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnCombat);
	pBacker->OnEvent(msgID,&EventCombat,sizeof(EventCombat));

	//避免计算两次
	if ( pCombatCnt->uidSource != pCombatCnt->uidUser )
		return;

	//得到胜利者和失败者
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidSource);
	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidEnemy);

	if ( !pCombatResult->m_bWin )
	{
		//失败奖励
		this->CombatForward(pActor, pEnemy,false, pCombatResult);
	}
	else
	{
		//胜利奖励
		this->CombatForward(pActor, pEnemy,true, pCombatResult);

		//杀敌记录
		ISyndicate * pEnemySyn = pEnemy->GetSyndicate();

		if ( 0 == pEnemySyn )
			return;

		std::hash_map<TSynID, VECT_MEMBER>::iterator itPos = m_SynMemberPos.find(pEnemySyn->GetSynID());

		if ( itPos == m_SynMemberPos.end() )
			return;

		VECT_MEMBER & vecMember = itPos->second;

		int pos = -1;

		for ( int i = 0; i < vecMember.size(); ++i )
		{
			if ( vecMember[i] == pEnemy->GetUID() )
			{
				pos = i;
				break;
			}
		}

		if ( pos >= 0 )
		{
			std::map<UID, UserSynWarInfo>::iterator iter = m_KillHistory.find(pActor->GetUID());

			if ( iter == m_KillHistory.end() )
			{
				UserSynWarInfo Info;
				Info.m_SynWarHitroy.set(pos, true);
				m_KillHistory[pActor->GetUID()] = Info;
			}
			else
			{
				UserSynWarInfo & Info = iter->second;
				Info.m_SynWarHitroy.set(pos, true);

				//检测下是否战胜所有对手
				bool bAll = true;

				for ( int index = 0; index < vecMember.size() && index < MAX_SYN_USERNUM; ++index )
				{
					if ( Info.m_SynWarHitroy.get(index) == false )
					{
						bAll = false;
						break;
					}
				}

				if ( bAll )
				{
					//清除所有
					Info.m_SynWarHitroy.zero();
				}
			}
		}
	}
}

//得到帮战数据
void SyndicateCombat::GetSynWarData(IActor * pActor)
{
	SC_GetSynWarData Rsp;

	ISyndicate * pSyndicate = pActor->GetSyndicate();

	if ( 0 == pSyndicate )
		return;

	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyndicate->GetSynID());

	if ( iter == m_SynPair.end() ){
		return;
	}

	ISyndicate * pEnemySyn = g_pGameServer->GetSyndicateMgr()->GetSyndicate(iter->second);

	if ( 0 == pEnemySyn ){
		return;
	}

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	Rsp.m_SynWarLevel = pActor->GetCrtProp(enCrtProp_SynCombatLevel);
	Rsp.m_CanJoinCombatNum = pSynPart->GetMaxJoinCombatNumToday() - pSynPart->GetJoinCombatNumToday();
	Rsp.m_MaxJoinCombatNum = pSynPart->GetMaxJoinCombatNumToday();
	Rsp.m_SynScore		   = pSyndicate->GetSynWarScore();
	Rsp.m_EnemySynScore	   = pEnemySyn->GetSynWarScore();
	strncpy(Rsp.m_EnemySynName, pEnemySyn->GetSynName(), sizeof(Rsp.m_EnemySynName));
	Rsp.m_CombatWinNum = 0;

	std::map<UID, UserSynWarInfo>::iterator it = m_KillHistory.find(pActor->GetUID());

	bool bNoKill = true;

	if ( it != m_KillHistory.end() ){
		bNoKill = false;
	}

	std::hash_map<TSynID, VECT_MEMBER>::iterator itMem = m_SynMemberPos.find(pEnemySyn->GetSynID());

	if ( itMem == m_SynMemberPos.end() ){
		TRACE("<error> %s : %d Line 获取不到帮派的帮战的分布信息！！帮派ID = %d", __FUNCTION__,  __LINE__, pEnemySyn->GetSynID());
		return;
	}

	VECT_MEMBER & vecMember = itMem->second;
	
	Rsp.m_Num = 0;

	OBuffer4k ob;

	for ( int i = 0; i < vecMember.size(); ++i )
	{
		ISyndicateMember * pSynMember = pEnemySyn->GetSynMember(vecMember[i]);

		if ( 0 == pSynMember )
			continue;

		SynWarUserInfo SynUser;

		SynUser.m_Facade = pSynMember->GetFacade();
		SynUser.m_Level  = pSynMember->GetLevel();
		strncpy(SynUser.m_Name, pSynMember->GetSynMemberName(), sizeof(SynUser.m_Name));
		SynUser.m_uidUser = pSynMember->GetSynMemberUID();

		if ( bNoKill ){
			SynUser.m_bCombat = true;
		} else {
			UserSynWarInfo & UserInfo = it->second;

			if ( UserInfo.m_SynWarHitroy.get(i) ){
				SynUser.m_bCombat = false;
				++Rsp.m_CombatWinNum;
			} else {
				SynUser.m_bCombat = true;
			}
		}

		++Rsp.m_Num;

		ob << SynUser;
	}

	OBuffer4k ob2;
	ob2 << SyndicateHeader(enSyndicateCmd_GetSynWarData, sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0 )
		ob2 << ob;

	pActor->SendData(ob2.TakeOsb());
}

//开始帮战
void SyndicateCombat::StartSynCombat()
{
	m_bOpenAttack = true;

	m_SynApply.clear();

	m_KillHistory.clear();

	//系统配对
	this->BeginSystemDeclareWar();

	//随机分配各帮派成员在场景的分布位置
	this->RandomSetSynMemberPos();

	//玩家卸载时保存下玩家的帮战数据
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->AddListener((IVoteListener *)this, msgID, enEventSrcType_Actor, 0, "SyndicateCombat::StartSynCombat");

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	pEnventServer->AddListener((IEventListener *)this, msgID, enEventSrcType_Actor, 0, "SyndicateCombat::StartSynCombat");

	//清除数据库宣战信息
	SDB_Delete_SynWarDeclare_Req  DBReq;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);

	//广播
	INT32 LangID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_NoticeSynWarStart;

	g_pGameServer->GetGameWorld()->WorldSystemMsg(g_pGameServer->GetGameWorld()->GetLanguageStr(LangID));

	//设置帮战结束定时器
	UINT32 TimeNum = this->GetOverWarTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_OverSynWar,this,TimeNum * 1000,"SyndicateCombat::StartSynCombat");

	TRACE("帮战开启,帮战结束定时器值:%d", TimeNum);
}

//结束帮战
void SyndicateCombat::OverSynCombat()
{
	IRelationServer * pRelationServer = g_pGameServer->GetRelationServer();
	m_bOpenAttack = false;

	//奖励
	this->SynWarOverForward();

	//刷新帮战排行
	if(pRelationServer != 0){
		pRelationServer->WarEndFlushRank();
	}


	//把帮派积分重新设为0
	MAP_SYN_PAIR::iterator iter = m_SynPair.begin();

	for ( ; iter != m_SynPair.end() ; ++iter )
	{
		ISyndicate * pSyn = g_pGameServer->GetSyndicateMgr()->GetSyndicate(iter->first);

		if ( 0 == pSyn )
			continue;

		pSyn->SetSynWarScore(0);
	}

	//清数据
	m_SynPair.clear();

	//取消卸载监听
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->RemoveListener((IVoteListener *)this, msgID, enEventSrcType_Actor, 0);

	//清除数据库帮战数据
	SDB_Delete_SynWarInfo_Req  DBDeleteWar;

	OBuffer1k ob;
	ob << DBDeleteWar;
	g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);

	//设置下次帮战开启定时器
	UINT32 TimeNum = this->GetNextOpenWarTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartSynWar,this,TimeNum * 1000,"SyndicateCombat::OverSynCombat");

	TRACE("帮战结束,下次开启定时器值:%d", TimeNum);
}

//开始帮战宣战
void SyndicateCombat::StartSynWarDeclare()
{
	m_bOpenDeclare = true;

	//设置宣战结束定时器
	UINT32 TimeNum = this->GetOverDeclareTime();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_OverDeclare,this,TimeNum * 1000,"SyndicateCombat::StartSynWarDeclare");

	TRACE("宣战开启,宣战结束定时器值:%d", TimeNum);
}

//结束帮战宣战
void SyndicateCombat::OverSynWarDeclare()
{
	m_bOpenDeclare = false;

	//设置下次宣战开始定时器
	UINT32 TimeNum = this->GetNextOpenDeclareTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartDeclare,this,TimeNum * 1000,"SyndicateCombat::OverSynWarDeclare");

	TRACE("宣战结束,宣战下次开启定时器值:%d", TimeNum);
}

//是否在宣战时间
bool SyndicateCombat::IsInDeclareWarTime()
{
	return m_bOpenDeclare;
}

void SyndicateCombat::OkDeclareWar(TSynID SynID, TSynID SynID2)
{
	MAP_SYN_APPLY::iterator  iter = m_SynApply.find(SynID2);

	if ( iter != m_SynApply.end() ){

		VECT_SYNID & vecSynID = iter->second;

		vecSynID.push_back(SynID);
	} else {
		
		VECT_SYNID vecSynID;

		vecSynID.push_back(SynID);

		m_SynApply[SynID2] = vecSynID;
	}
}

void SyndicateCombat::PairSynCombat(ISyndicate * pSyn1, ISyndicate * pSyn2)
{
	TSynID SynID1 = pSyn1->GetSynID();
	TSynID SynID2 = pSyn2->GetSynID();

	if ( !m_SynPair.insert(MAP_SYN_PAIR::value_type(SynID1, SynID2)).second )
		return;

	if ( !m_SynPair.insert(MAP_SYN_PAIR::value_type(SynID2, SynID1)).second ){
		m_SynPair.erase(SynID1);
		return;
	}

	m_SynApply.erase(SynID1);

	m_SynApply.erase(SynID2);

	pSyn1->SetSynWarScore(0);
	pSyn2->SetSynWarScore(0);
}

//检测两个帮派是否可以配对
UINT8 SyndicateCombat::CanPairJoin(ISyndicate * pSyn1, ISyndicate * pSyn2)
{
	enSynRetCode RetCode = (enSynRetCode)this->CanJoinSynCombat(pSyn1);

	if ( enSynRetCode_OK != RetCode )
		return RetCode;

	RetCode = (enSynRetCode)this->CanJoinSynCombat(pSyn2);

	if ( enSynRetCode_OK != RetCode )
		return RetCode;

	if ( pSyn1 == pSyn2 )
		return enSynRetCode_DeclareMySelf;

	return enSynRetCode_OK;
}

//检测一个帮派是否可以参加帮战
UINT8 SyndicateCombat::CanJoinSynCombat(ISyndicate * pSyn)
{
	MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyn->GetSynID());

	if ( iter != m_SynPair.end() )
		return enSynRetCode_ErrDeclared;

	return pSyn->Check_JoinSynCombat();	
}

//随机分配各帮派成员在场景中的位置
void  SyndicateCombat::RandomSetSynMemberPos()
{
	m_SynMemberPos.clear();

	MAP_SYN_PAIR::iterator iter = m_SynPair.begin();

	for ( int nadd = 0; iter != m_SynPair.end(); ++iter, ++nadd )
	{
		ISyndicate * pSyn = g_pGameServer->GetSyndicateMgr()->GetSyndicate(iter->first);

		if ( 0 == pSyn ){
			TRACE("<error> %s ; %d Line 配对过的帮派，现在找不到帮派！！帮派ID = %d", __FUNCTION__, __LINE__, iter->first);
			continue;
		}

		//得到随机分布位置
		VECT_MEMBER vecMember;

		pSyn->RandomSynMemberPos(vecMember);

		m_SynMemberPos[pSyn->GetSynID()] = vecMember;

		//入库
		SDB_Update_SynWarInfo_Req DBReq;

		DBReq.m_SynID = iter->first;
		DBReq.m_EnemySynID = iter->second;
		DBReq.m_bWin = false;

		UID * pUid = (UID *)&DBReq.m_MemberPos;

		for ( int i = 0; i < vecMember.size() && i < MAX_SYNMEMBER_NUM; ++i, ++pUid )
		{
			(*pUid) = vecMember[i];
		}

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB + nadd,enDBCmd_Update_SynWarInfo,ob.TakeOsb(),0,0);
	}
}

//战斗胜利奖励
void  SyndicateCombat::CombatForward(IActor * pWiner, IActor * pFailer)
{
	//ISynPart * pWinSynPart = pWiner->GetSynPart();

	//if ( 0 == pWinSynPart )
	//	return;

	//ISynPart * pFailSynPart = pFailer->GetSynPart();

	//if ( 0 == pFailSynPart )
	//	return;

	//const SGameServerConfigParam & ServerInfo = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	////获得帮战积分
	//INT32 GetSynScore = 0;

	//if ( pWiner->GetCrtProp(enCrtProp_SynCombatLevel) != -1 ){
	//	GetSynScore = (1 + pFailer->GetCrtProp(enCrtProp_SynCombatLevel)) / (1 + pWiner->GetCrtProp(enCrtProp_SynCombatLevel)) * ServerInfo.m_WinSynWarCombatParam + 0.99999;
	//}

	//ISyndicate * pSyn = pWiner->GetSyndicate();

	//if ( 0 == pSyn )
	//	return;

	//
	//pSyn->AddSynWarScore(GetSynScore);

	////胜利方获得声望
	//INT32 GetCredit = ServerInfo.m_WinSynWarCombatCredit;

	//if ( pWinSynPart->GetGetNumWeek() + GetCredit > this->GetCreditUp(pWiner) ){
	//	GetCredit = this->GetCreditUp(pWiner) - pWinSynPart->GetGetNumWeek();
	//}

	//pWinSynPart->AddGetCredit(GetCredit);

	////失败方获得声望
	//GetCredit = ServerInfo.m_FailSynWarCombatCredit;

	//if ( pFailSynPart->GetGetNumWeek() + GetCredit > this->GetCreditUp(pFailer) ){
	//	GetCredit = this->GetCreditUp(pFailer) - pFailSynPart->GetGetNumWeek();
	//}

	//pFailSynPart->AddGetCredit(GetCredit);

	////获得帮战等级
	//SSynCombatParam WinParam;

	//g_pGameServer->GetConfigServer()->GetSynCombatParam(pWiner->GetCrtProp(enCrtProp_SynCombatLevel), WinParam);

	//SSynCombatParam FailParam;

	//g_pGameServer->GetConfigServer()->GetSynCombatParam(pFailer->GetCrtProp(enCrtProp_SynCombatLevel), FailParam);

	////胜利方增加的帮战等级
	//INT32 AddWinerCombatLevel = 0;

	//if ( -1 != pWiner->GetCrtProp(enCrtProp_SynCombatLevel) ){
	//	AddWinerCombatLevel = WinParam.m_WinParam * ((1 + pFailer->GetCrtProp(enCrtProp_SynCombatLevel)) / (1 + pWiner->GetCrtProp(enCrtProp_SynCombatLevel))) + 0.99999;
	//}

	////失败方减少的帮战等级
	//INT32 DesFailCombatLevel = 0;

	//if ( -1 != pWiner->GetCrtProp(enCrtProp_SynCombatLevel) ){
	//	DesFailCombatLevel = FailParam.m_FailParam * ((1 + pFailer->GetCrtProp(enCrtProp_SynCombatLevel)) / (1 + pWiner->GetCrtProp(enCrtProp_SynCombatLevel))) + 0.99999;
	//}

	//pWiner->AddCrtPropNum(enCrtProp_SynCombatLevel, AddWinerCombatLevel);

	//if ( pFailer->GetCrtProp(enCrtProp_SynCombatLevel) < DesFailCombatLevel ){
	//	DesFailCombatLevel = pFailer->GetCrtProp(enCrtProp_SynCombatLevel);
	//}

	//pFailer->AddCrtPropNum(enCrtProp_SynCombatLevel, -DesFailCombatLevel);
}

//战斗胜利奖励
void  SyndicateCombat::CombatForward(IActor * pActor, IActor * pEnemy, bool bWin, CombatCombatOver * pCombatResult)
{
	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 == pSyn )
		return;

	const SSynWarForward * pSynWarForward = g_pGameServer->GetConfigServer()->GetSynWarForward(pActor->GetCrtProp(enCrtProp_SynCombatLevel));

	if ( 0 == pSynWarForward )
	{
		TRACE("<error> %s : %d Line 获取帮战奖励配置信息出错！！帮战等级:%d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_SynCombatLevel));
		return;
	}

	const SGameServerConfigParam & ServerInfo = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	float SynWarParam = pow(10.0f,(float)((pEnemy->GetCrtProp(enCrtProp_SynCombatLevel) - pActor->GetCrtProp(enCrtProp_SynCombatLevel)) / 400.0f )) + 1.0f;

	if ( bWin )
	{
		//获得帮战积分
		if ( SynWarParam != 0 )
		{
			INT32 AddSynScore = pSynWarForward->m_SynWarScoreR * (pSynWarForward->m_SynWarScoreWinRes - ( 1.0f / SynWarParam)) + 0.999999;

			if ( AddSynScore < ServerInfo.m_MinGetScoreSynWar )
			{
				AddSynScore = ServerInfo.m_MinGetScoreSynWar;
			}
			else if ( AddSynScore > ServerInfo.m_MaxGetScoreSynWar )
			{
				AddSynScore = ServerInfo.m_MaxGetScoreSynWar;
			}

			if ( AddSynScore != 0 )
			{
				pCombatResult->m_SynScore = AddSynScore;


				pSyn->AddSynWarScore(AddSynScore);

				ISyndicateMember * pSynMember = pSyn->GetSynMember(pActor->GetUID());

				if ( 0 != pSynMember )
					pSynMember->AddSynScore(AddSynScore);

				//检测下这个帮派的帮战是否结束
				if ( pSyn->GetSynWarScore() >= ServerInfo.m_SynWarWinScore )
				{
					//帮战胜利
					this->SynWinForward(pSyn->GetSynID());

					m_FastWinSyn.insert(pSyn->GetSynID());
				}
			}
		}

		//胜利方获得声望
		INT32 GetCredit = ServerInfo.m_WinSynWarCombatCredit;

		if ( GetCredit != 0 )
		{
			if ( pSynPart->GetGetNumWeek() + GetCredit + pActor->GetVipValue(enVipType_AddGetCredit) > this->GetCreditUp(pActor) )
			{
				GetCredit = this->GetCreditUp(pActor) - (pSynPart->GetGetNumWeek() + pActor->GetVipValue(enVipType_AddGetCredit));
			}

			pSynPart->AddGetCredit(GetCredit);

			pCombatResult->m_Credit = GetCredit + pActor->GetVipValue(enVipType_AddGetCredit);		
		}

		//获得帮战等级
		if ( SynWarParam != 0 )
		{
			INT32 AddSynWarLv = pSynWarForward->m_SynWarLvR * ( pSynWarForward->m_SynWarLvWinRes - ( 1.0f / SynWarParam)) + 0.999999;

			if ( AddSynWarLv != 0 )
			{
				if ( pActor->GetCrtProp(enCrtProp_SynCombatLevel) + AddSynWarLv < 0 )
				{
					AddSynWarLv = -pActor->GetCrtProp(enCrtProp_SynCombatLevel);
				}

				pActor->AddCrtPropNum(enCrtProp_SynCombatLevel, AddSynWarLv);

				pCombatResult->m_SynWarLevel = AddSynWarLv;			
			}
		}
	}
	else
	{
		//失败方获得声望
		INT32 GetCredit = ServerInfo.m_FailSynWarCombatCredit;

		if ( GetCredit != 0 )
		{
			if ( pSynPart->GetGetNumWeek() + GetCredit > this->GetCreditUp(pActor) ){
				GetCredit = this->GetCreditUp(pActor) - pSynPart->GetGetNumWeek();
			}

			pCombatResult->m_Credit = GetCredit + pActor->GetVipValue(enVipType_AddGetCredit);

			if ( 0 != pCombatResult->m_Credit )
			{
				pSynPart->AddGetCredit(GetCredit);
			}	
		}


		//减少帮战等级
		if ( SynWarParam != 0 )
		{
			INT32 DecSynWarLv = pSynWarForward->m_SynWarLvR * ( pSynWarForward->m_SynWarLvFailRes - ( 1.0f / SynWarParam)) + 0.999999;

			if ( DecSynWarLv != 0 )
			{
				if ( pActor->GetCrtProp(enCrtProp_SynCombatLevel) + DecSynWarLv < 0 )
				{
					DecSynWarLv = -pActor->GetCrtProp(enCrtProp_SynCombatLevel);
				}

				pActor->AddCrtPropNum(enCrtProp_SynCombatLevel, DecSynWarLv);

				pCombatResult->m_SynWarLevel = DecSynWarLv;			
			}
		}
	}
	
}


//帮战奖励
void  SyndicateCombat::SynWarOverForward()
{
	std::set<TSynID>	setSyn;

	MAP_SYN_PAIR::iterator iter = m_SynPair.begin();

	for ( ; iter != m_SynPair.end(); ++iter )
	{
		TSynID SynID1 = iter->first;
		TSynID SynID2 = iter->second;

		std::set<TSynID>::iterator itWin = m_FastWinSyn.find(SynID1);

		if ( itWin != m_FastWinSyn.end() )
		{
			//已经胜利的，给过奖励了
			continue;
		}

		itWin = m_FastWinSyn.find(SynID2);

		if ( itWin != m_FastWinSyn.end() )
		{
			//已经胜利的，给过奖励了
			continue;
		}

		std::set<TSynID>::iterator itbOK = setSyn.find(SynID1);

		if ( itbOK != setSyn.end() )
			continue;

		setSyn.insert(SynID1);
		setSyn.insert(SynID2);

		ISyndicate * pSyn1 = g_pGameServer->GetSyndicateMgr()->GetSyndicate(SynID1);
		ISyndicate * pSyn2 = g_pGameServer->GetSyndicateMgr()->GetSyndicate(SynID2);

		if ( 0 == pSyn1 || 0 == pSyn2 )
			continue;

		if ( pSyn1->GetSynWarScore() == pSyn2->GetSynWarScore() )
		{
			//平局
			pSyn1->SetPreSynWarData(pSyn2->GetSynName(),enumSynWarbWin_Draw);
			pSyn2->SetPreSynWarData(pSyn1->GetSynName(),enumSynWarbWin_Draw);
			UpdatePreSynWarData(SynID1,enumSynWarbWin_Draw,pSyn2->GetSynName());
			UpdatePreSynWarData(SynID2,enumSynWarbWin_Draw,pSyn1->GetSynName());

			//发出系统公告
			char m_szSysMsg[DESCRIPT_LEN_100] = "\0";

			sprintf_s(m_szSysMsg, sizeof(m_szSysMsg), g_pGameServer->GetGameWorld()->GetLanguageStr(12104), pSyn1->GetSynName(), pSyn2->GetSynName());

			g_pGameServer->GetGameWorld()->WorldSystemMsg(m_szSysMsg);

			this->SendSynAllResult(pSyn1, enumSynWarbWin_Draw);
			this->SendSynAllResult(pSyn2, enumSynWarbWin_Draw);

			continue;
		}
		else if ( pSyn1->GetSynWarScore() > pSyn2->GetSynWarScore() )
		{
			this->SynWinForward(SynID1);
		}
		else
		{
			this->SynWinForward(SynID2);
		}
	}
}

//获得声望上限
INT32 SyndicateCombat::GetCreditUp(IActor * pActor)
{
	const SSynWarForward * pSynWarForward = g_pGameServer->GetConfigServer()->GetSynWarForward(pActor->GetCrtProp(enCrtProp_SynCombatLevel));

	if ( 0 == pSynWarForward )
	{
		TRACE("<error> %s : %d Line 获取帮战奖励配置信息出错！！帮战等级:%d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_SynCombatLevel));
		return 0;
	}	

	return pSynWarForward->m_CreditUp + pActor->GetVipValue(enVipType_AddSynWarCreditUp);
}

//开始战斗
void  SyndicateCombat::BeginCombat(IActor * pActor, IActor * pEnemy)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if ( 0 == pCombatPart )
		return;

	ISynPart * pSynPart = pActor->GetSynPart();

	if ( 0 == pSynPart )
		return;

	pSynPart->AddCombatNumToday(1);

	TMapID CombatMapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_SynWarCombatMapID;

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(CombatMapID);

	if ( 0 == pMapConfig ){
		TRACE("<error> %s : %d Line 找不到地图的配置信息！！地图ID = %d", __FUNCTION__, __LINE__, CombatMapID);
		return;
	}

	//创建战斗场景
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(CombatMapID, true);
	if ( 0 == pGameScene ){
		TRACE("<error> %s : %d Line 创建帮派战斗场景失败！！地图ID = %d", __FUNCTION__, __LINE__, CombatMapID);
		return;
	}

	UINT64 CombatID = 0;

	pCombatPart->CombatWithScene(enCombatType_SynChallenge,pEnemy->GetUID(), CombatID, this, g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID), pGameScene);
}

//得到帮战是否开启
bool SyndicateCombat::GetbOpenAttack()
{
	return m_bOpenAttack;
}

//得到敌对帮派
TSynID SyndicateCombat::GetVsSynID(IActor * pActor)
{
	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 == pSyn ){
		return 0;
	}

	MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyn->GetSynID());

	if ( iter != m_SynPair.end() ){
		
		ISyndicate * pVsSyn = g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSyndicate(iter->second);
		//得到敌对帮派名字名字
		//char szName[THING_NAME_LEN] = {'\0'};
		if ( 0 != pVsSyn ){
			//strncpy(szName, pVsSyn->GetSynName(), THING_NAME_LEN);
			return pVsSyn->GetSynID();
		}
	}

	return 0;
}

void SyndicateCombat::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_Get_LastRecordTime:
		{
			this->HandleGetSynWarLastTime(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_SynWarDeclare:
		{
			this->HandleGetSynWarDeclare(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_SynWarInfo:
		{
			this->HandleGetSynWarData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_SynWarKillHistory:
		{
			this->HandleGetSynWarKill(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;

	default:
		{
			TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		}
		break;
	}
}

bool  SyndicateCombat::OnVote(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	if (  EventData.m_MsgID == msgID )
	{
		//卸载玩家时，保存玩家的帮战数据
		SS_UnloadActorContext * pUnloadActorContext = (SS_UnloadActorContext *)EventData.m_pContext;

		if( 0 == pUnloadActorContext)
		{
			return true;
		}

		std::map<UID, UserSynWarInfo>::iterator iter = m_KillHistory.find(pUnloadActorContext->m_uidActor);

		if ( iter != m_KillHistory.end() )
		{
			UserSynWarInfo & Info = iter->second;

			SDB_Update_SynWarKillHistory_Req DBReq;

			DBReq.m_uidUser = (iter->first).ToUint64();

			memcpy(&DBReq.m_KillHistroy, &Info.m_SynWarHitroy, sizeof(DBReq.m_KillHistroy));

			OBuffer1k ob;
			ob << DBReq;
			g_pGameServer->GetDBProxyClient()->Request(DBReq.m_uidUser,enDBCmd_Update_SynWarKillHistory,ob.TakeOsb(),0,0);
		}

		//清除玩家的帮战场景
		std::map<UID, TSceneID>::iterator itSce = m_mapSceneID.find(pUnloadActorContext->m_uidActor);

		if ( itSce != m_mapSceneID.end() )
		{
			g_pGameServer->GetGameWorld()->DeleteGameScene(itSce->second);

			m_mapSceneID.erase(itSce);		
		}
	}

	return true;
}

//该时间是否开启帮战战斗
bool  SyndicateCombat::IsOpenCombatTime(UINT32 time)
{
	time_t ttime = time;

	tm * pTm = localtime(&ttime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatDeclareTime.size() < 4 || GameParam.m_SynCombatTime.size() < 4 )
	{
		TRACE("<error> %s : %d Line 配置表帮战参数个数错误！！", __FUNCTION__, __LINE__);
		return false;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatTime[0]
		|| (pTm->tm_hour ==  GameParam.m_SynCombatTime[0] && pTm->tm_min < GameParam.m_SynCombatTime[1]) )
	{
		return false;
	}
	else
	{
		if (  pTm->tm_hour > GameParam.m_SynCombatTime[2]
			|| (pTm->tm_hour ==  GameParam.m_SynCombatTime[2] && pTm->tm_min > GameParam.m_SynCombatTime[3]) )
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;
}

//该时间是否开启帮战宣战
bool		SyndicateCombat::IsOpenDeclareTime(UINT32 time)
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatDeclareTime.size() < 4 || GameParam.m_SynCombatTime.size() < 4 ){
		TRACE("<error> %s : %d Line 配置表帮战参数个数错误！！", __FUNCTION__, __LINE__);
		return false;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatDeclareTime[0]
		|| (pTm->tm_hour ==  GameParam.m_SynCombatDeclareTime[0] && pTm->tm_min < GameParam.m_SynCombatDeclareTime[1]) )
	{
		return false;
	}
	else
	{
		if (  pTm->tm_hour > GameParam.m_SynCombatDeclareTime[2]
			|| (pTm->tm_hour ==  GameParam.m_SynCombatDeclareTime[2] && pTm->tm_min > GameParam.m_SynCombatDeclareTime[3]) )
		{
			return false;
		}
		else
		{
			return true;
		}	
	}

	return false;
}

//得到帮战最后记录时间
void		SyndicateCombat::HandleGetSynWarLastTime(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader	RspHeader;
	DB_OutParam			OutParam;
	SDB_LastRecordTime	LastTime;
	RspIb >> RspHeader >> OutParam >> LastTime;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	UINT32 CurTime = CURRENT_TIME();

	bool bSameToday = XDateTime::GetInstance().IsSameDay(CurTime,LastTime.m_LastRecordTime);

	bool bCombatTime = this->IsOpenCombatTime(CurTime);

	bool bDeclareTime = this->IsOpenDeclareTime(CurTime);

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		if ( bCombatTime )
		{
			this->StartSynCombat();
		}
		else if ( bDeclareTime )
		{
			this->StartSynWarDeclare();
		}

		return;
	}

	if ( !bSameToday )
	{
		//删除宣战信息
		SDB_Delete_SynWarDeclare_Req  DBDeclare;

		OBuffer1k ob;
		ob << DBDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);

		//删除帮战信息
		SDB_Delete_SynWarInfo_Req  DBCombat;

		ob.Reset();
		ob << DBCombat;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);
	}
	else if ( bCombatTime )
	{
		if ( this->IsOpenCombatTime(LastTime.m_LastRecordTime) )
		{
			m_bOpenAttack = true;

			//得到帮战数据
			SDB_Get_SynWarInfo_Req DBWar;

			OBuffer1k ob;
			ob << DBWar;
			g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarInfo,ob.TakeOsb(),this,0);

			//得到帮战杀敌记录
			SDB_Get_SynWarKillHistory_Req DBKill;

			ob.Reset();
			ob << DBWar;
			g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarKillHistory,ob.TakeOsb(),this,0);
		}
		else
		{
			//直接开启帮战
			this->StartSynCombat();
		}

		//删除宣战信息
		SDB_Delete_SynWarDeclare_Req  DBDeclare;

		OBuffer1k ob;
		ob << DBDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);
	}
	else if (  bDeclareTime )
	{
		if ( this->IsOpenDeclareTime(LastTime.m_LastRecordTime) )
		{
			m_bOpenDeclare = true;

			//得到帮战宣战数据
			SDB_Get_SynWarDeclare_Req DBDeclare;

			OBuffer1k ob;
			ob << DBDeclare;
			g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarDeclare,ob.TakeOsb(),this,0);
		}
		else
		{
			//直接开启宣战
			this->StartSynWarDeclare();
		}

		//删除帮战信息
		SDB_Delete_SynWarInfo_Req  DBCombat;

		OBuffer1k ob;
		ob << DBCombat;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);
	}
	else
	{
		TRACE("<error> %s : %d Line 帮战错误！！", __FUNCTION__, __LINE__);
	}
}

//得到帮战数据
void		SyndicateCombat::HandleGetSynWarData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader	RspHeader;
	DB_OutParam			OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		//清除数据库帮战数据
		SDB_Delete_SynWarDeclare_Req  DBDeclare;

		OBuffer1k ob;
		ob << DBDeclare;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarDeclare,ob.TakeOsb(),0,0);

		//开启帮战
		this->StartSynCombat();

		//设置下次宣战开始定时器
		UINT32 TimeNum = this->GetNextOpenDeclareTimeNum();

		g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartDeclare,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarData");

		TRACE("设置下次宣战开启定时器值:%d", TimeNum);

		return;
	}

	m_bOpenAttack = true;

	//玩家卸载时保存下玩家的帮战数据
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->AddListener((IVoteListener *)this, msgID, enEventSrcType_Actor, 0, "SyndicateCombat::StartSynCombat"); 

	int num = RspIb.Remain() / sizeof(SDB_SynWarInfo);

	for ( int i = 0; i < num; ++i )
	{
		SDB_SynWarInfo Info;

		RspIb >> Info;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		m_SynPair[Info.m_SynID] = Info.m_EnemySynID;

		if ( Info.m_bWin )
		{
			m_FastWinSyn.insert(Info.m_SynID);
		}

		VECT_MEMBER vecMember;

		UID * pUid = (UID *)&Info.m_MemberPos;

		for ( int index = 0; index < MAX_SYNMEMBER_NUM; ++index,++pUid )
		{
			if ( !(*pUid).IsValid() )
			{
				break;
			}

			vecMember.push_back((*pUid));
		}

		m_SynMemberPos[Info.m_SynID] = vecMember;
	}

	//得到帮战杀敌记录
	SDB_Get_SynWarKillHistory_Req DBKill;

	OBuffer1k ob;
	ob << DBKill;
	g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Get_SynWarKillHistory,ob.TakeOsb(),this,0);

	//设置帮战结束定时器
	UINT32 TimeNum = this->GetOverWarTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_OverSynWar,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarData");

	TRACE("帮战开启,帮战结束定时器值:%d", TimeNum);

	//设置下次宣战开始定时器
	TimeNum = this->GetNextOpenDeclareTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartDeclare,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarData");

	TRACE("设置下次宣战开启定时器值:%d", TimeNum);
}

//得到帮战杀敌记录
void		SyndicateCombat::HandleGetSynWarKill(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader	RspHeader;
	DB_OutParam			OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
		return;

	int num = RspIb.Remain() / sizeof(SDB_SynWarKillHistroy);

	for ( int i = 0; i < num ; ++i )
	{
		SDB_SynWarKillHistroy Info;

		RspIb >> Info;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		UserSynWarInfo KillInfo;

		memcpy(&KillInfo.m_SynWarHitroy, &Info.m_KillHistroy, sizeof(KillInfo.m_SynWarHitroy));

		m_KillHistory[UID(Info.m_uidUser)] = KillInfo;
	}
}

//得到帮战宣战数据
void		SyndicateCombat::HandleGetSynWarDeclare(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader	RspHeader;
	DB_OutParam			OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		//清除数据库帮战宣战数据
		SDB_Delete_SynWarInfo_Req  DBCombat;

		OBuffer1k ob;
		ob << DBCombat;
		g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Delete_SynWarInfo,ob.TakeOsb(),0,0);

		//开启宣战
		this->StartSynWarDeclare();

		//设置下次帮战开启定时器
		UINT32 TimeNum = this->GetNextOpenWarTimeNum();

		g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartSynWar,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarDeclare");

		TRACE("设置下次帮战开启定时器值:%d", TimeNum);

		return;
	}

	//开启宣战
	m_bOpenDeclare = true;

	int num = RspIb.Remain() / sizeof(SDB_SynWarDeclare);

	for ( int i = 0; i < num; ++i )
	{
		SDB_SynWarDeclare Info;

		RspIb >> Info;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		MAP_SYN_APPLY::iterator iter = m_SynApply.find(Info.m_SynID);

		if ( iter != m_SynApply.end() )
		{
			VECT_SYNID & vecSyn = iter->second;

			vecSyn.push_back(Info.m_DeclareSynID);
		}
		else
		{
			VECT_SYNID vecSyn;

			vecSyn.push_back(Info.m_DeclareSynID);

			m_SynApply[Info.m_SynID] = vecSyn;
		}
	}

	//设置宣战结束定时器
	UINT32 TimeNum = this->GetOverDeclareTime();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_OverDeclare,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarDeclare");

	TRACE("宣战开启,宣战结束定时器值:%d", TimeNum);

	//设置下次帮战开启定时器
	TimeNum = this->GetNextOpenWarTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enSynWarTimer_StartSynWar,this,TimeNum * 1000,"SyndicateCombat::HandleGetSynWarDeclare");

	TRACE("设置下次帮战开启定时器值:%d", TimeNum);
}

//保存帮派的帮战信息
void		SyndicateCombat::SaveSynSynWar(TSynID SynID, bool bWin)
{
	MAP_SYN_PAIR::iterator iter = m_SynPair.find(SynID);

	if ( iter == m_SynPair.end() )
		return;

	SDB_Update_SynWarInfo_Req DBReq;

	DBReq.m_SynID = SynID;
	DBReq.m_EnemySynID = iter->second;
	DBReq.m_bWin = bWin;

	std::hash_map<TSynID, VECT_MEMBER>::iterator itPos = m_SynMemberPos.find(SynID);

	if ( itPos == m_SynMemberPos.end() )
		return;

	UID * pUid = (UID *)&DBReq.m_MemberPos;

	VECT_MEMBER & vecMember = itPos->second;

	for ( int i = 0; i < vecMember.size() && i < MAX_SYNMEMBER_NUM; ++i, ++pUid )
	{
		(*pUid) = vecMember[i];
	}

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(SYN_WAR_DB,enDBCmd_Update_SynWarInfo,ob.TakeOsb(),0,0);	
}

//帮派胜利的奖励
void		SyndicateCombat::SynWinForward(TSynID SynID)
{
	const SGameServerConfigParam & ServerInfo = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	std::hash_map<TSynID, VECT_MEMBER>::iterator iter = m_SynMemberPos.find(SynID);

	if ( iter == m_SynMemberPos.end() )
	{
		TRACE("<error> %s : %d Line 获取不到帮派的帮战信息！！帮派%d", __FUNCTION__, __LINE__, SynID);
		return;
	}

	VECT_MEMBER & vecMember = iter->second;

	for ( int i = 0; i < vecMember.size(); ++i )
	{
		UID uidUser = vecMember[i];

		IActor * pMember = g_pGameServer->GetGameWorld()->FindActor(uidUser);

		if ( 0 != pMember )
		{
			this->SendSynResult(pMember, enumSynWarbWin_Win);
		}

		std::map<UID, UserSynWarInfo>::iterator itWar = m_KillHistory.find(uidUser);

		if ( itWar == m_KillHistory.end() )
			continue;

		UserSynWarInfo & WarInfo = itWar->second;

		for ( int n = 0; n < MAX_SYN_USERNUM; ++n )
		{
			if ( WarInfo.m_SynWarHitroy.get(n) )
			{
				//至少得胜过一场，才给奖励
				IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

				if ( 0 == pActor )
				{
					if ( m_NoTakeForward.insert(uidUser).second ){
						//装载玩家
						g_pGameServer->LoadActor(uidUser);

						if ( m_NoTakeForward.size() == 1 && m_mapCombat.size() == 0 ){

							UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

							IEventServer * pEnventServer = g_pGameServer->GetEventServer();
							if( 0 == pEnventServer){
								return;
							}

							pEnventServer->AddListener((IEventListener *)this, msgID, enEventSrcType_Actor,0,"SyndicateCombat::SynWarForward");	
						}
					}
					continue;
				} 

				ISynPart * pSynPart = pActor->GetSynPart();

				if ( 0 == pSynPart )
					return;

				INT32 GetCredit = ServerInfo.m_WinSynWarGetCredit;

				if ( pSynPart->GetGetNumWeek() + GetCredit > this->GetCreditUp(pActor) ){
					GetCredit = this->GetCreditUp(pActor) - pSynPart->GetGetNumWeek();
				}

				pSynPart->AddGetCredit(GetCredit);
				pActor->AddCrtPropNum(enCrtProp_SynCombatLevel, ServerInfo.m_WinSynWarGetSynWarLv);
				
				break;
			}
		}	
	}

	this->SaveSynSynWar(SynID, true);

	MAP_SYN_PAIR::iterator itPair = m_SynPair.find(SynID);

	ISyndicate * pSynWin = g_pGameServer->GetSyndicateMgr()->GetSyndicate(SynID);
	ISyndicate * pSynFail = g_pGameServer->GetSyndicateMgr()->GetSyndicate(itPair->second);

	if(pSynFail == 0 || pSynWin == 0)
	{
		return;
	}

	UpdatePreSynWarData(SynID,enumSynWarbWin_Win,pSynFail->GetSynName());
	UpdatePreSynWarData(itPair->second,enumSynWarbWin_Fail,pSynWin->GetSynName());
	
	pSynWin->SetPreSynWarData(pSynFail->GetSynName(),enumSynWarbWin_Win);
	pSynFail->SetPreSynWarData(pSynWin->GetSynName(),enumSynWarbWin_Fail);	

	//发出系统公告
	char m_szSysMsg[DESCRIPT_LEN_100] = "\0";

	sprintf_s(m_szSysMsg, sizeof(m_szSysMsg), g_pGameServer->GetGameWorld()->GetLanguageStr(12103), pSynWin->GetSynName(), pSynFail->GetSynName());

	g_pGameServer->GetGameWorld()->WorldSystemMsg(m_szSysMsg);

	//给失败帮派成员发送帮战结果框
	this->SendSynAllResult(pSynFail, enumSynWarbWin_Fail);
	
}

//更新上一场帮战数据
void SyndicateCombat::UpdatePreSynWarData(TSynID SynID,enumSynWarbWin bWin,const char * SynName)
{
	SDB_Update_PreSynWarInfo Req;

	Req.m_SynID = SynID;
	Req.m_bWin = bWin;
	strncpy(Req.SynName,SynName,sizeof(Req.SynName));

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID,enDBCmd_Update_PreSynWarInfo,ob.TakeOsb(),0,0);
}

//弹出帮战胜利/失败弹出框
void		SyndicateCombat::SendSynResult(IActor * pActor, enumSynWarbWin enbWin)
{
	SC_PopSynResult Rsp;

	Rsp.m_SynWarbWin = enbWin;

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_PopSynResult, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());	
	
}

//给帮派成员发送帮战结果框
void		SyndicateCombat::SendSynAllResult(ISyndicate * pSyn, enumSynWarbWin enbWin)
{
	std::hash_map<TSynID, VECT_MEMBER>::iterator iter = m_SynMemberPos.find(pSyn->GetSynID());

	if ( iter == m_SynMemberPos.end() )
		return;

	VECT_MEMBER & vecMember = iter->second;

	for ( int i = 0; i < vecMember.size(); ++i )
	{
		IActor * pMember = g_pGameServer->GetGameWorld()->FindActor(vecMember[i]);

		if ( 0 == pMember )
			continue;

		this->SendSynResult(pMember, enbWin);
	}
}

//多久开启下次帮战
UINT32		SyndicateCombat::GetNextOpenWarTimeNum()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatTime.size() < 4 )
	{
		TRACE("<error> %s : %d Line 帮战开启结束时间配置参数个数有错！！个数%d", __FUNCTION__, __LINE__, GameParam.m_SynCombatTime.size());
		return 0;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatTime[0] || ( pTm->tm_hour == GameParam.m_SynCombatTime[0] && pTm->tm_min < GameParam.m_SynCombatTime[1] ) )
	{
		return (GameParam.m_SynCombatTime[0] - pTm->tm_hour) * 3600 + (GameParam.m_SynCombatTime[1] - pTm->tm_min) * 60 - pTm->tm_sec;
	}

	if ( pTm->tm_hour > GameParam.m_SynCombatTime[2] || ( pTm->tm_hour == GameParam.m_SynCombatTime[2] && pTm->tm_min >= GameParam.m_SynCombatTime[3] ) )
	{
		return (24 - pTm->tm_hour) * 3600 - pTm->tm_min * 60 - pTm->tm_sec + GameParam.m_SynCombatTime[0] * 3600 + GameParam.m_SynCombatTime[1] * 60;
	}

	return 0;
}

//多久结束帮战
UINT32		SyndicateCombat::GetOverWarTimeNum()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatTime.size() < 4 )
	{
		TRACE("<error> %s : %d Line 帮战开启结束时间配置参数个数有错！！个数%d", __FUNCTION__, __LINE__, GameParam.m_SynCombatTime.size());
		return 0;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatTime[0] || ( pTm->tm_hour == GameParam.m_SynCombatTime[0] && pTm->tm_min < GameParam.m_SynCombatTime[1] ) )
	{
		return 0;
	}

	if ( pTm->tm_hour > GameParam.m_SynCombatTime[2] || ( pTm->tm_hour == GameParam.m_SynCombatTime[2] && pTm->tm_min >= GameParam.m_SynCombatTime[3] ) )
	{
		return 0;
	}

	return (GameParam.m_SynCombatTime[2] - pTm->tm_hour) * 3600 + (GameParam.m_SynCombatTime[3] - pTm->tm_min) * 60 - pTm->tm_sec;
}

//多久开启下次帮战宣战
UINT32		SyndicateCombat::GetNextOpenDeclareTimeNum()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatDeclareTime.size() < 4 )
	{
		TRACE("<error> %s : %d Line 帮战宣战开启结束时间配置参数个数有错！！个数%d", __FUNCTION__, __LINE__, GameParam.m_SynCombatTime.size());
		return 0;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatDeclareTime[0] || ( pTm->tm_hour == GameParam.m_SynCombatDeclareTime[0] && pTm->tm_min < GameParam.m_SynCombatDeclareTime[1] ) )
	{
		return (GameParam.m_SynCombatDeclareTime[0] - pTm->tm_hour) * 3600 + (GameParam.m_SynCombatDeclareTime[1] - pTm->tm_min) * 60 - pTm->tm_sec;
	}

	if ( pTm->tm_hour > GameParam.m_SynCombatDeclareTime[2] || ( pTm->tm_hour == GameParam.m_SynCombatDeclareTime[2] && pTm->tm_min >= GameParam.m_SynCombatDeclareTime[3] ) )
	{
		return (24 - pTm->tm_hour) * 3600 - pTm->tm_min * 60 - pTm->tm_sec + GameParam.m_SynCombatDeclareTime[0] * 3600 + GameParam.m_SynCombatDeclareTime[1] * 60;
	}

	return 0;
}

//多久结束帮战宣战
UINT32		SyndicateCombat::GetOverDeclareTime()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_SynCombatDeclareTime.size() < 4 )
	{
		TRACE("<error> %s : %d Line 帮战宣战开启结束时间配置参数个数有错！！个数%d", __FUNCTION__, __LINE__, GameParam.m_SynCombatTime.size());
		return 0;
	}

	if ( pTm->tm_hour < GameParam.m_SynCombatDeclareTime[0] || ( pTm->tm_hour == GameParam.m_SynCombatDeclareTime[0] && pTm->tm_min < GameParam.m_SynCombatDeclareTime[1] ) )
	{
		return 0;
	}

	if ( pTm->tm_hour > GameParam.m_SynCombatDeclareTime[2] || ( pTm->tm_hour == GameParam.m_SynCombatDeclareTime[2] && pTm->tm_min >= GameParam.m_SynCombatDeclareTime[3] ) )
	{
		return 0;
	}

	return (GameParam.m_SynCombatDeclareTime[2] - pTm->tm_hour) * 3600 + (GameParam.m_SynCombatDeclareTime[3] - pTm->tm_min) * 60 - pTm->tm_sec;
}


//得到敌对帮派名
std::string SyndicateCombat::GetVsSynName(IActor * pActor)
{
	ISyndicate * pSyn = pActor->GetSyndicate();

	if ( 0 == pSyn ){
		return std::string();
	}

	MAP_SYN_PAIR::iterator iter = m_SynPair.find(pSyn->GetSynID());

	if ( iter != m_SynPair.end() ){
		
		ISyndicate * pVsSyn = g_pGameServer->GetRelationServer()->GetSyndicateMgr()->GetSyndicate(iter->second);
		//得到敌对帮派名字

		if ( 0 != pVsSyn ){
			std::string strSynName = pVsSyn->GetSynName();
			return strSynName;
		}
	}

	return std::string();
}

