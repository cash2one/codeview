#include "XuanTianFuBen.h"
#include "ThingServer.h"
#include "XDateTime.h"
#include "time.h"
#include "RandomService.h"
#include "IBasicService.h"
#include "IGameScene.h"
#include "IMonster.h"
#include "ICDTimerPart.h"
#include "IFuBenPart.h"
#include <algorithm>
#include "DBProtocol.h"
#include "DSystem.h"
#include "IGoodsServer.h"

XuanTianFuBen::XuanTianFuBen()
{
	m_bOpen = false;

	m_bOver = false;
}

bool	XuanTianFuBen::Create()
{
	const SGameConfigParam & GameCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 1 < GameCnfg.m_XuanTianLevelGroup.size(); i += 2 )
	{
		VEC_XTACTORINFO vecActorInfo;

		m_vectLastDamageRank.push_back(vecActorInfo);
	}

	this->ReloadXTMonsterCnfg();

	//得到BOSS信息
	SDB_Get_XTBossInfo_Req DBBoss;

	OBuffer1k ob;
	ob << DBBoss;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Get_XTBossInfo,ob.TakeOsb(),this,0);

	//得到玄天BOSS死亡信息
	SDB_Get_XTBossDie_Req DBBossDie;

	ob.Reset();
	ob << DBBossDie;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Get_XTBossDie,ob.TakeOsb(),this,0);

	//得到玄天伤害
	SDB_Get_XTDamage_Req DBDamage;

	ob.Reset();
	ob << DBDamage;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Get_XTDamage,ob.TakeOsb(),this,0);

	return true;
}

//玄天副本是否开启
bool	XuanTianFuBen::IsOpen() const
{
	return m_bOpen;
}

void	XuanTianFuBen::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enXuanTianTimerID_Over:
		{
			this->Over_TimeXT();

			g_pGameServer->GetTimeAxis()->KillTimer(enXuanTianTimerID_Over, this);

			TRACE("玄天结束");
		}
		break;
	case enXuanTianTimerID_Start:
		{
			this->StartXuanTian();

			g_pGameServer->GetTimeAxis()->KillTimer(enXuanTianTimerID_Start, this);

			TRACE("玄天开启");
		}
		break;
	case enXuanTianTimerID_Save:
		{
			//保存BOSS数据
			std::map<UID, TMonsterID>::iterator iter = m_mapBoss.begin();
			
			for ( ; iter != m_mapBoss.end(); ++iter )
			{
				IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(iter->first);

				if ( 0 == pBoss || pBoss->GetCrtProp(enCrtProp_Blood) <= 0 )
					continue;

				this->SaveBossInfo(pBoss);
			}
		}
		break;
	}

	//time_t CurTime = CURRENT_TIME();

	//tm * pTm = localtime(&CurTime);

	//const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//bool bFlag = false;

	//for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	//{
	//	if ( (pTm->tm_hour > GameParam.m_XuanTianTime[i] || (pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min > GameParam.m_XuanTianTime[i + 1]))
	//		&& ( pTm->tm_hour < GameParam.m_XuanTianTime[i + 2] || (pTm->tm_hour == GameParam.m_XuanTianTime[i + 2] && pTm->tm_min < GameParam.m_XuanTianTime[i + 3])) )
	//	{
	//		if ( !m_bOpen ){
	//			//开启玄天
	//			this->StartXuanTian();
	//		}

	//		bFlag = true;

	//		break;
	//	}
	//}

	//if ( !bFlag )
	//{
	//	if ( m_bOpen ){
	//		//玄天结束
	//		this->Over_TimeXT();
	//	}		
	//}

	////每5分钟保存下玄天BOSS信息
	//if ( m_bOpen && pTm->tm_min / 5 == 0 )
	//{
	//	UINT32 CurTime = CURRENT_TIME();
	//	
	//	OBuffer1k ob;

	//	//BOSS信息
	//	std::map<UID, TMonsterID>::iterator iter = m_mapBoss.begin();
	//	
	//	for ( ; iter != m_mapBoss.end(); ++iter )
	//	{
	//		IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(iter->first);

	//		if ( 0 == pBoss || pBoss->GetCrtProp(enCrtProp_Blood) <= 0 )
	//			continue;

	//		SDB_Save_XTBossInfo_Req DBReq;

	//		DBReq.m_CurBlood   = pBoss->GetCrtProp(enCrtProp_Blood);
	//		DBReq.m_MonsterID  = iter->second;
	//		DBReq.m_RecordTime = CurTime;
	//		DBReq.m_UIDBoss    = iter->first.ToUint64();

	//		ob.Reset();
	//		ob << DBReq;
	//		g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Save_XTBossInfo,ob.TakeOsb(),0,0);
	//	}
	//}
}

//开启玄天
void	XuanTianFuBen::StartXuanTian(std::vector<DBCreateXTBoss> * pCreateBoss)
{
	//先删除上次玄天场景
	if ( m_BossCombatSceneID.IsValid() ){
		g_pGameServer->GetGameWorld()->DeleteGameScene(m_BossCombatSceneID);
		m_BossCombatSceneID = INVALID_SCENE_ID;
	}

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	//创建战斗场景
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(ServerParam.m_XTBossMap);

	if ( 0 == pGameScene ){
		TRACE("<error> %s : %d Line 创建玄天摆怪场景失败！！地图ID = %d", __FUNCTION__, __LINE__, ServerParam.m_XTBossMap);
		return;
	}

	//创建玄天BOSS,并进场景
	if ( !this->CreateXTBoss(pGameScene, pCreateBoss) )
	{
		TRACE("<error> %s :%d Line 创建玄天BOSS失败，玄天不能开启！！", __FUNCTION__, __LINE__);
		return;
	}

	//数据记录
	m_BossCombatSceneID = pGameScene->GetSceneID();

	m_bOpen = true;

	m_bOver = false;

	//广播
	INT32 LangID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_NoticeXuanTianStart;

	g_pGameServer->GetGameWorld()->WorldSystemMsg(g_pGameServer->GetGameWorld()->GetLanguageStr(LangID));

	//创建结束定时器
	UINT32 TimeNum = this->GetOverTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enXuanTianTimerID_Over,this,TimeNum * 1000,"XuanTianFuBen::StartXuanTian");

	//5分钟保存一次玄天BOSS数据
	g_pGameServer->GetTimeAxis()->SetTimer(enXuanTianTimerID_Save,this,ServerParam.m_SaveXuanTianTimeSpace * 1000,"XuanTianFuBen::StartXuanTian");
}

bool sortdamage(const XTActorInfo & ActorInfo1, const XTActorInfo & ActorInfo2)
{
	return ActorInfo1.m_Damage > ActorInfo2.m_Damage;
}

//玄天结束(三个BOSS都死时，提前结束)
void	XuanTianFuBen::OverXuanTian()
{
	m_bOver = true;

	//清除上次的伤害排名，放入这次的
	for ( int i = 0; i < m_vectLastDamageRank.size(); ++i )
	{
		VEC_XTACTORINFO & vecXTInfo = m_vectLastDamageRank[i];

		vecXTInfo.clear();
	}

	std::map<UID, XTActorInfo>::iterator iter = m_mapDamage.begin();

	std::vector<XTActorInfo>	vecActorInfo;

	for ( ; iter != m_mapDamage.end(); ++iter )
	{
		XTActorInfo & ActorInfo = iter->second;

		vecActorInfo.push_back(ActorInfo);
	}

	//进行排名
	std::sort(vecActorInfo.begin(), vecActorInfo.end(), sortdamage);

	for ( int i = 0; i < vecActorInfo.size(); ++i )
	{
		int index = this->GetLevelGroupIndex(vecActorInfo[i].m_Level);

		if ( -1 == index || index >= m_vectLastDamageRank.size() )
			continue;

		VEC_XTACTORINFO & vecXTActorInfo = m_vectLastDamageRank[index];

		vecXTActorInfo.push_back(vecActorInfo[i]);
	}

	//修改上一次最后一击的
	m_vecLastKillUser.clear();

	std::map<UID, BossLastKill>::iterator itKill = m_mapKillUser.begin();

	for ( ; itKill != m_mapKillUser.end(); ++itKill )
	{
		BossLastKill & KillInfo = itKill->second;

		m_vecLastKillUser.push_back(KillInfo);
	}

	//给奖励
	this->XTForward();

	//修改数据库上次玄天信息
	this->ChangeDB_LastInfo();

	//广播
	g_pGameServer->GetGameWorld()->WorldSystemMsg(g_pGameServer->GetGameWorld()->GetLanguageStr(12105));

	//删除保存BOSS信息定时器
	g_pGameServer->GetTimeAxis()->KillTimer(enXuanTianTimerID_Save,this);
	TRACE("删除定时器enXuanTianTimerID_Save");
}


//玄天时间结束
void		XuanTianFuBen::Over_TimeXT()
{
	m_bOpen = false;

	if ( !m_bOver )
	{
		this->OverXuanTian();
	}

	m_mapBoss.clear();
	m_mapKillUser.clear();
	m_mapDamage.clear();

	//清除数据库BOSS信息
	this->Clear_BossInfo();

	//创建下次开始定时器
	UINT32 TimeNum = this->GetNextOpenTimeNum();

	g_pGameServer->GetTimeAxis()->SetTimer(enXuanTianTimerID_Start,this,TimeNum * 1000,"XuanTianFuBen::Over_TimeXT");
}

//攻击BOSS
void	XuanTianFuBen::AttackBoss(IActor * pActor, UID uidBoss)
{
	SC_AttackXTBoss Rsp;

	Rsp.m_RetCode = (enMainUICode)this->Check_CanAttackBoss(pActor, uidBoss);

	if ( enMainUICode_OK == Rsp.m_RetCode ){
		//可以攻击
		ICombatPart * pCombatPart = pActor->GetCombatPart();

		if ( 0 == pCombatPart )
			return;

		const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(m_BossCombatSceneID.GetMapID());

		if ( 0 == pMapConfig ){
			TRACE("<error> %s : %d Line 找不到地图的配置信息！！地图ID = %d", __FUNCTION__, __LINE__, m_BossCombatSceneID.GetMapID());
			return;
		}

		UINT64	CombatID = 0;
		
		pCombatPart->CombatWithNpc(enCombatType_XuanTian, uidBoss, CombatID, this, g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID));

		//ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();

		//if ( 0 == pCDTimerPart ){
		//	return;
		//}

		//const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

		//pCDTimerPart->RegistCDTime(ServerParam.m_XuanTianAttackCD);
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_AttackXTBoss,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//打开玄天页面
void	XuanTianFuBen::OpenXuanTiam(IActor * pActor)
{
	SC_OpenXuanTian Rsp;
	OBuffer4k ob;

	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();

	if ( 0 == pCDTimerPart )
		return;

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( !m_bOpen )
	{
		Rsp.m_XTState = enXTState_NoOpen;

		Rsp.m_OpenRemainTime = this->GetRemainOpenXT();

		ob <<  SMainUIHeader(enMainUICmd_OpenXuanTian,sizeof(Rsp)) << Rsp;
	}
	else
	{
		Rsp.m_XTState = enXTState_Open;

		Rsp.m_OpenRemainTime = this->GetRemainOverXT();

		Rsp.m_Num = m_mapBoss.size();

		ob <<  SMainUIHeader(enMainUICmd_OpenXuanTian,sizeof(Rsp) + Rsp.m_Num * sizeof(XTBossState)) << Rsp;

		std::map<UID, TMonsterID>::iterator iter = m_mapBoss.begin();

		for ( ; iter != m_mapBoss.end();  ++iter )
		{
			XTBossState BossState;

			BossState.m_uidBoss = iter->first;

			IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(iter->first);

			if ( 0 == pBoss )
			{
				BossState.m_BossBlood = 0;
			}
			else
			{
				BossState.m_BossBlood = pBoss->GetCrtProp(enCrtProp_Blood);
			}

			const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(iter->second);

			if ( 0 == pMonsterCnfg ){
				TRACE("<error> %s : %d Line 获取怪物配置文件失败！！怪物ID = %d",  __FUNCTION__, __LINE__, iter->second);
				return;
			}

			BossState.m_BossBloodUp = pMonsterCnfg->m_Blood;

			strncpy(BossState.m_BossName, pBoss->GetName(), sizeof(BossState.m_BossName));

			ob << BossState;
		}
	}

	pActor->SendData(ob.TakeOsb());
}

//显示玄天奖励
void	XuanTianFuBen::ShowXTForward(IActor * pActor, UINT32 Rank)
{
	const SXuanTianForward * pXTForward = g_pGameServer->GetConfigServer()->GetXTForward(Rank);

	if ( 0 == pXTForward )
	{
		TRACE("<error> %s : %d Line 获取玄天配置信息出错！！排名 %d", __FUNCTION__, __LINE__, Rank);
		return;
	}

	std::vector<TGoodsID> vectGoods;

	//g_pGameServer->GetGoodsServer()->GetDropGoodsID(pXTForward->m_DropID, vectGoods);

	const std::vector<SDropGoods> * pVectDropGoods = g_pGameServer->GetConfigServer()->GetDropGoodsCnfg(pXTForward->m_DropID);
	if( 0 == pVectDropGoods)
	{
		TRACE("<error> %s : %d line 获取掉落的配置文件失败!!,DropID = %d", __FUNCTION__,__LINE__,pXTForward->m_DropID);
		return;
	}

	for ( int i = 0; i < (*pVectDropGoods).size(); ++i )
	{
		const std::vector<TGoodsID> & vecGoods = (*pVectDropGoods)[i].m_vectDropGoods;

		for ( int k = 0; k + 2 < vecGoods.size(); k += 3 )
		{
			vectGoods.push_back(vecGoods[k]);
		}
	}

	SC_ShowXuanTianForward Rsp;

	OBuffer4k ob;

	for ( int i = 0; i < vectGoods.size(); ++i )
	{
		++Rsp.m_Num;
		ob << vectGoods[i];
		pActor->SendGoodsCnfg(vectGoods[i]);
	}

	OBuffer4k ob2;
	ob2 <<  SMainUIHeader(enMainUICmd_ShowXuanTianForward,sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0 )
		ob2 << ob;

	pActor->SendData(ob2.TakeOsb());
}

//使用仙石缩短玄天再次攻击时间
void	XuanTianFuBen::MoneyLessAttackTime(IActor * pActor)
{
	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	//先判断玩家有没有在CD时间
	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();

	if ( 0 == pCDTimerPart )
		return;

	SC_LessCDTime Rsp;

	if ( pCDTimerPart->IsCDTimeOK(ServerParam.m_XuanTianAttackCD) && pCDTimerPart->IsCDTimeOK(ServerParam.m_MoneyXuanTianAttackCD) ){
		Rsp.m_RetCode = enMainUICode_NoCDTime;
	} else {
		//去除之前的CD时间
		if ( !pCDTimerPart->IsCDTimeOK(ServerParam.m_XuanTianAttackCD) ){
			pCDTimerPart->UnLoadCDTime(ServerParam.m_XuanTianAttackCD);
		} else {
			pCDTimerPart->UnLoadCDTime(ServerParam.m_MoneyXuanTianAttackCD);
		}

		//注册新的CD时间
		pCDTimerPart->RegistCDTime(ServerParam.m_MoneyXuanTianAttackCD);

		Rsp.m_CDTimeNum = g_pGameServer->GetConfigServer()->GetCDTimeCnfg(ServerParam.m_MoneyXuanTianAttackCD);
	}

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_LessCDTime,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//战斗结束了
void XuanTianFuBen::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	//玄天结束时才打完的，不算
	if ( !m_bOpen || m_bOver )
		return;

	SS_OnCombat EventCombat;

	EventCombat.CombatPlace = (UINT8)enCombatPlace_XuanTian;
	EventCombat.bWin = pCombatResult->m_bWin;
	EventCombat.bIsTeam = pCombatCnt->bIsTeam;
	EventCombat.bHard = false;

	IActor * pBacker = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( 0 == pBacker )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnCombat);
	pBacker->OnEvent(msgID,&EventCombat,sizeof(EventCombat));

	if ( pCombatCnt->uidSource != pCombatCnt->uidUser )
		return;

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( pActor->GetMaster() != 0 )
		return;

	std::map<UID, TMonsterID>::iterator itBoss = m_mapBoss.find(pCombatCnt->uidEnemy);

	if ( itBoss == m_mapBoss.end() )
	{
		TRACE("<error> %s : %d Line 不是玄天BOSS的UID！！",  __FUNCTION__, __LINE__);
		return;
	}

	IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(pCombatCnt->uidEnemy);

	if ( 0 == pBoss )
	{
		TRACE("<warning> %s : %d Line Boss找不到！！", __FUNCTION__, __LINE__);
		return;
	}

	if ( pBoss->GetCrtProp(enCrtProp_Blood) > 0 )
	{
		std::map<UID, XTActorInfo>::iterator iter = m_mapDamage.find(pCombatCnt->uidUser);

		if ( iter != m_mapDamage.end() )
		{
			XTActorInfo & ActorInfo = iter->second;

			ActorInfo.m_Damage += pCombatCnt->DamageValue;

			++ActorInfo.m_AttackNum;

			ActorInfo.m_Level = pActor->GetCrtProp(enCrtProp_Level);
		}
		else
		{
			XTActorInfo ActorInfo;

			ActorInfo.m_Damage = pCombatCnt->DamageValue;

			ActorInfo.m_AttackNum = 1;

			ActorInfo.m_uidUser = pCombatCnt->uidUser;

			ActorInfo.m_Level = pActor->GetCrtProp(enCrtProp_Level);

			strncpy(ActorInfo.m_Name, pActor->GetName(), sizeof(ActorInfo.m_Name));

			m_mapDamage[pActor->GetUID()] = ActorInfo;
		}
	}

	pBoss->AddCrtPropNum(enCrtProp_Blood, -pCombatCnt->DamageValue);

	if ( pBoss->GetCrtProp(enCrtProp_Blood) <= 0 )
	{
		//BOSS死亡
		std::map<UID, BossLastKill>::iterator itDie = m_mapKillUser.find(pBoss->GetUID());

		if ( itDie != m_mapKillUser.end() )
		{	
			//BOSS已经被别人打死了
			return;
		}

		std::map<UID, XTActorInfo>::iterator it = m_mapDamage.find(pCombatCnt->uidUser);

		if ( it == m_mapDamage.end() ){
			return;
		}

		XTActorInfo & ActorInfo = it->second;

		BossLastKill BossKill;

		BossKill.m_uidUser = pActor->GetUID();
		BossKill.m_CurDamage = pCombatCnt->DamageValue;
		strncpy(BossKill.m_szBossName, pBoss->GetName(), sizeof(BossKill.m_szBossName));
		strncpy(BossKill.m_szUserName, pActor->GetName(), sizeof(BossKill.m_szUserName));
		BossKill.m_TotalDamage = ActorInfo.m_Damage;
		BossKill.m_AttackNum = ActorInfo.m_AttackNum;
		BossKill.m_UserLevel = pActor->GetCrtProp(enCrtProp_Level);

		m_mapKillUser[pBoss->GetUID()] = BossKill;

		//记入数据库
		SDB_Save_XTBossDie_Req DBReq;

		DBReq.m_UIDBoss = pBoss->GetUID().ToUint64();
		DBReq.m_AttackNum = ActorInfo.m_AttackNum;
		strncpy(DBReq.m_BossName, pBoss->GetName(), sizeof(DBReq.m_BossName));
		DBReq.m_CurDamage = pCombatCnt->DamageValue;
		DBReq.m_TotalDamage = ActorInfo.m_Damage;
		DBReq.m_UIDUser	  = pActor->GetUID().ToUint64();
		strncpy(DBReq.m_UserName, pActor->GetName(), sizeof(DBReq.m_UserName));
		DBReq.m_UserLevel = pActor->GetCrtProp(enCrtProp_Level);

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Save_XTBossDie,ob.TakeOsb(),0,0);

		//保存BOSS信息
		this->SaveBossInfo(pBoss);

		//检测玄天是否结束
		if ( this->Check_OverXT() )
		{
			this->OverXuanTian();
		}
	}
}

//显示最后一击
void	XuanTianFuBen::ShowLastKill(IActor * pActor)
{
	SC_ViewLastKill Rsp;

	Rsp.m_Num = m_vecLastKillUser.size();

	OBuffer1k ob;

	ob <<  SMainUIHeader(enMainUICmd_ViewLastKill,sizeof(Rsp) + Rsp.m_Num * sizeof(XTBossDie)) << Rsp;

	for ( int  i = 0; i < m_vecLastKillUser.size(); ++i )
	{
		BossLastKill & KillInfo = m_vecLastKillUser[i];

		XTBossDie BossDie;

		strncpy(BossDie.m_BossName, KillInfo.m_szBossName, sizeof(BossDie.m_BossName));
		strncpy(BossDie.m_UserNmae, KillInfo.m_szUserName, sizeof(BossDie.m_UserNmae));
		BossDie.m_DamageValue = KillInfo.m_CurDamage;
		BossDie.m_UserLevel = KillInfo.m_UserLevel;

		ob << BossDie;
	}

	pActor->SendData(ob.TakeOsb());
}

//显示伤害排行
void	XuanTianFuBen::ShowDamageRank(IActor * pActor, int index)
{
	SC_ViewXTDamageRank Rsp;

	if ( index < 0 || index >= m_vectLastDamageRank.size() )
	{
		TRACE("<error> %s : %d Line 客户端发来的索引超出！！index = %d", __FUNCTION__, __LINE__, index);
		return;
	}

	Rsp.m_index = index;

	VEC_XTACTORINFO & vecActorInfo = m_vectLastDamageRank[index];

	Rsp.m_Num = vecActorInfo.size();

	OBuffer1k ob;
	ob <<  SMainUIHeader(enMainUICmd_ViewXTDamageRank,sizeof(Rsp) + Rsp.m_Num * sizeof(XTRankName)) << Rsp;

	for ( int i = 0; i < vecActorInfo.size(); ++i )
	{
		XTActorInfo & ActorInfo = vecActorInfo[i];

		XTRankName RankName;

		strncpy(RankName.m_Name, ActorInfo.m_Name, sizeof(RankName.m_Name));
		RankName.m_DamageValue = ActorInfo.m_Damage;

		ob << RankName;
	}

	pActor->SendData(ob.TakeOsb());
}

void		XuanTianFuBen::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
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
		case enDBCmd_Get_XTBossInfo:
			{
				this->HandleGetXTBossInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_Get_XTBossDie:
			{
				this->HandleGetXTBossDie(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_Get_XTDamage:
			{
				this->HandleGetXTDamage(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_GetMonsterCnfg:
			{
				this->HandleGetXTBossCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		default:
			{
				TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
			}
			break;
	}
}

//保存玩家的伤害数据
void	XuanTianFuBen::SaveUserDamage(IActor * pActor)
{
	std::map<UID, XTActorInfo>::iterator iter = m_mapDamage.find(pActor->GetUID());

	if ( iter == m_mapDamage.end() )
		return;

	XTActorInfo & Info = iter->second;

	SDB_Update_XTDamage_Req DBReq;

	DBReq.m_AttackNum = Info.m_AttackNum;
	DBReq.m_Damage	  = Info.m_Damage;
	DBReq.m_Level	  = Info.m_Level;
	DBReq.m_UIDUser	  = Info.m_uidUser.ToUint64();
	strncpy(DBReq.m_UserName, Info.m_Name, sizeof(DBReq.m_UserName));
	DBReq.m_bOver	  = m_bOver;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_Damage,enDBCmd_Update_XTDamage,ob.TakeOsb(),0,0);
}

//重新加载玄天怪物配置信息
void	XuanTianFuBen::ReloadXTMonsterCnfg()
{
	//获取BOSS配置
	SDB_GetMonsterCnfgReq DBBossCnfg;

	OBuffer1k ob;
	ob << DBBossCnfg;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_GetMonsterCnfg,ob.TakeOsb(),this,0);
}

//随机产生一个玄天BOSS的ID
XTBossInfo	XuanTianFuBen::RandomGetXTBossID()
{
	XTBossInfo BossInfo;

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	int nTotalRandom = 0;

	for ( int i = 0; i + 2 < ServerParam.m_XuanTianBoss.size(); i += 3 )
	{
		nTotalRandom += ServerParam.m_XuanTianBoss[i + 2];
	}

	int nRandom = RandomService::GetRandom() % nTotalRandom;

	int Tmp = 0;

	for ( int i = 0; i + 2 < ServerParam.m_XuanTianBoss.size(); i += 4 )
	{
		Tmp += ServerParam.m_XuanTianBoss[i + 2];

		if ( Tmp > nRandom ){
			BossInfo.m_MonsterID = ServerParam.m_XuanTianBoss[i];
			BossInfo.m_CombatMap = ServerParam.m_XuanTianBoss[i + 1];
			BossInfo.m_CombatIndex = ServerParam.m_XuanTianBoss[i + 2];
			return BossInfo;
		}
	}

	TRACE("<error> %s : %d Line 随机抽取不到玄天BOSS",  __FUNCTION__, __LINE__);
	return BossInfo;
}

//是否能攻击玄天BOSS
UINT8		XuanTianFuBen::Check_CanAttackBoss(IActor * pActor, UID uidBoss)
{
	if ( !m_bOpen ){
		return enMainUICode_NoOpen;
	}

	std::map<UID, TMonsterID>::iterator iter = m_mapBoss.find(uidBoss);

	if ( iter == m_mapBoss.end() )
	{
		TRACE("<error> %s : %d Line 客户端传来的BOSS的UID错误！！", __FUNCTION__, __LINE__);
		return enMainUICode_Err;
	}

	IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(uidBoss);

	if ( 0 == pBoss ){
		TRACE("<error> %s : %d Line Boss找不到！！", __FUNCTION__, __LINE__);
		return enMainUICode_NoBossDie;
	}

	if ( pBoss ->GetCrtProp(enCrtProp_Blood) <= 0 ){
		return enMainUICode_NoBossDie;
	}

	UINT8 MinLevel = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_EnterMinLevel;

	if ( MinLevel > pActor->GetCrtProp(enCrtProp_Level) )
	{
		return enMainUICode_ErrLevelEnterXT;
	}
	else if ( pActor->HaveTeam() )
	{
		return enMainUICode_ErrHaveTeam;
	}
	else if ( pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		return enMainUICode_ErrInDuoBao;
	}
	else if ( pActor->IsInFastWaitTeam() )
	{
		return enMainUICode_ErrWaitTeam;
	}

	return enMainUICode_OK;
}

//玄天奖励
void		XuanTianFuBen::XTForward()
{
	//给最后一击奖励    排名0表示最后一击
	for ( int i = 0; i < m_vecLastKillUser.size(); ++i )
	{
		BossLastKill & killInfo = m_vecLastKillUser[i];

		this->GetRankForward(killInfo.m_uidUser, 0, killInfo.m_AttackNum, killInfo.m_TotalDamage, killInfo.m_szBossName);
	}
	
	//给伤害排名奖励
	for ( int i = 0; i < m_vectLastDamageRank.size(); ++i )
	{
		VEC_XTACTORINFO & vecActorInfo = m_vectLastDamageRank[i];

		for ( int rank = 0; rank < vecActorInfo.size(); ++rank )
		{
			XTActorInfo & Info = vecActorInfo[rank];

			this->GetRankForward(Info.m_uidUser, rank + 1, Info.m_AttackNum, Info.m_Damage, "");
		}
	}
}

//按等级给奖励
void		XuanTianFuBen::GetRankForward(UID uidUser, INT32 Rank, INT32 AttackNum, INT32 Damage, const char * pBossName)
{
	SWriteSystemData MailData;

	MailData.m_DestUID = uidUser;
	strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10081), sizeof(MailData.m_szThemeText));

	const SXuanTianForward * pXTForward = g_pGameServer->GetConfigServer()->GetXTForward(Rank);

	if ( 0 == pXTForward ){
		//没有配置的获得奖励为：攻击次数*聚灵气
		const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

		MailData.m_PolyNimbus = AttackNum * ServerParam.m_XTGetPolyNimbusParam;

		if ( MailData.m_PolyNimbus > ServerParam.m_MaxXTGetPolyNimbusNoRank )
		{
			MailData.m_PolyNimbus = ServerParam.m_MaxXTGetPolyNimbusNoRank;
		}

		//恭喜你与玄天BOSS战斗%d次，请接受我们的奖励吧
		sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10082), AttackNum);

		g_pGameServer->GetGameWorld()->WriteSystemMail(MailData);

		TRACE("%s : %s", uidUser.ToString(), MailData.m_szContentText);
	} else {

		MailData.m_PolyNimbus = pXTForward->m_PolyNimbus;

		if ( 0 == Rank ){
			//恭喜你对%s造成了最后一击，请接受我们的奖励吧
			sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10084), pBossName);

		} else {
			//恭喜你对玄天BOSS造成了%d伤害，排名%d，请接受我们的奖励吧
			sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10083), Damage, Rank);
		}

		TRACE("%s : %s",uidUser.ToString(),  MailData.m_szContentText);

		//随机获得物品
		std::vector<TGoodsID> vectGoodsID;

		g_pGameServer->GetGoodsServer()->GetDropGoodsID(pXTForward->m_DropID, vectGoodsID);

		std::vector<IGoods *> vecGoods;

		for ( int i = 0; i < vectGoodsID.size(); ++i )
		{
			SCreateGoodsContext GoodsContext;

			GoodsContext.m_GoodsID = vectGoodsID[i];
			GoodsContext.m_Number  = 1;
			GoodsContext.m_Binded  = false;

			IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsContext);
			if ( 0 == pGoods )
			{
				TRACE("<error> %s : %d Line 创建物品失败，物品ID = %d", __FUNCTION__, __LINE__, vectGoodsID[i]);
				break;
			}

			vecGoods.push_back(pGoods);
		}

		//std::vector<IGoods *> vecGoods = this->RandomGetGoodsForward(pXTForward);

		g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);
	}
}

//随机获得物品奖励
std::vector<IGoods *> XuanTianFuBen::RandomGetGoodsForward(const SXuanTianForward * pXTForward)
{
	std::vector<IGoods *> vecGoods;

	//if ( 0 == pXTForward && pXTForward->m_vecGoods.size() < 1 && pXTForward->m_TotalRandom == 0)
	//	return vecGoods;

	////物品数量
	//int GoodsNum  = pXTForward->m_vecGoods[0];

	//int TotalRandom = pXTForward->m_TotalRandom;

	//std::vector<int> vecIndex;

	//for ( int num = 0; num < GoodsNum; ++num )
	//{
	//	INT32 nRandom = RandomService::GetRandom() % TotalRandom;

	//	int Tmp = 0;

	//	for ( int i = 1; i + 1 < pXTForward->m_vecGoods.size(); i += 2 )
	//	{
	//		Tmp += pXTForward->m_vecGoods[i + 1];

	//		if ( Tmp > nRandom ){

	//			bool bContinue = false;

	//			for ( int k = 0; k < vecIndex.size(); ++k )
	//			{
	//				if ( vecIndex[k] == i ){
	//					bContinue = true;
	//					continue;
	//				}
	//			}

	//			if ( bContinue )
	//				continue;
	//			
	//			TGoodsID GoodsID = pXTForward->m_vecGoods[i];

	//			SCreateGoodsContext GoodsContext;

	//			GoodsContext.m_GoodsID = GoodsID;
	//			GoodsContext.m_Number  = 1;
	//			GoodsContext.m_Binded  = true;

	//			IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsContext);
	//			if ( 0 == pGoods ){
	//				TRACE("<error> %s : %d Line 创建物品失败，物品ID = %d", __FUNCTION__, __LINE__, GoodsID);
	//				break;
	//			}

	//			vecGoods.push_back(pGoods);

	//			TotalRandom -= pXTForward->m_vecGoods[i + 1];

	//			vecIndex.push_back(i);
	//			break;
	//		}
	//	}	
	//}

	return vecGoods;
}

//得到玄天开放剩余时间
UINT32		XuanTianFuBen::GetRemainOpenXT()
{
	if ( m_bOpen )
		return 0;

	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	{
		if ( pTm->tm_hour < GameParam.m_XuanTianTime[i]
			|| (pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min < GameParam.m_XuanTianTime[i + 1]) )
		{
			return (GameParam.m_XuanTianTime[i] - pTm->tm_hour) * 3600 + (GameParam.m_XuanTianTime[i + 1] - pTm->tm_min) * 60;
		}
	}

	return (24 - pTm->tm_hour) * 3600 + (0 - pTm->tm_min) * 60 + GameParam.m_XuanTianTime[0] * 3600 + GameParam.m_XuanTianTime[1] * 60;
}

//得到玄天结束剩余时间
UINT32		XuanTianFuBen::GetRemainOverXT()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	{
		if ( (pTm->tm_hour > GameParam.m_XuanTianTime[i] || (pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min > GameParam.m_XuanTianTime[i + 1]) )
			&& (pTm->tm_hour < GameParam.m_XuanTianTime[i + 2] || (pTm->tm_hour == GameParam.m_XuanTianTime[i + 2] && pTm->tm_min < GameParam.m_XuanTianTime[i + 3])) )
		{
			return (GameParam.m_XuanTianTime[i + 2] - pTm->tm_hour) * 3600 + (GameParam.m_XuanTianTime[i + 3] - pTm->tm_min) * 60;
		}
	}

	return 0;
}

//创建玄天BOSS
bool		XuanTianFuBen::CreateXTBoss(IGameScene * pGameScene, std::vector<DBCreateXTBoss> * pCreateBoss)
{
	m_mapBoss.clear();

	UINT32 CurTime = CURRENT_TIME();

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	for ( int i = 0; i + 1 < ServerParam.m_XuanTianBoss.size(); i += 2 )
	{
		SCreateMonsterContext  MonsterCnt;

		MonsterCnt.MonsterID = ServerParam.m_XuanTianBoss[i];
		MonsterCnt.SceneID   = INVALID_SCENE_ID;
		MonsterCnt.m_CombatIndex = ServerParam.m_XuanTianBoss[i + 1];

		INT32 CurBlood = 0;

		if ( 0 != pCreateBoss )
		{
			for ( int i = 0; i < (*pCreateBoss).size(); ++i )
			{
				if ( MonsterCnt.MonsterID == (*pCreateBoss)[i].m_MonsterID )
				{
					MonsterCnt.uid = (*pCreateBoss)[i].m_uidBoss;
					CurBlood = (*pCreateBoss)[i].m_CurBlood;
					break;
				}
			}
		}

		//创建战斗场景BOSS
		IMonster * pBoss = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);

		if ( 0 == pBoss )
		{
			TRACE("<error> %s : %d Line 创建玄天BOSS失败！！BOSS的ID:%d,战斗索引:%d", __FUNCTION__, __LINE__, MonsterCnt.MonsterID, MonsterCnt.m_CombatIndex);
			continue;
		}

		if (  0 != pCreateBoss )
		{
			pBoss->SetCrtProp(enCrtProp_Blood, CurBlood);
		}

		m_mapBoss[pBoss->GetUID()] = MonsterCnt.MonsterID;

		pGameScene->EnterScene(pBoss);

		//入库
		this->SaveBossInfo(pBoss);
	}

	return true;
}

//得到玩家的级别组下标
int			XuanTianFuBen::GetLevelGroupIndex(UINT8 Level)
{
	const SGameConfigParam & GameCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 1 < GameCnfg.m_XuanTianLevelGroup.size(); i += 2)
	{
		if ( Level >= GameCnfg.m_XuanTianLevelGroup[i] && Level <=  GameCnfg.m_XuanTianLevelGroup[i + 1] )
		{
			return i / 2;
			TRACE("i : %d, index : %d, Level", i, i / 2, Level);
		}
	}

	return -1;
}

//在玄天开放的第几个时期，0表示还没开放并且今天也没开放过
UINT8		XuanTianFuBen::InOpenTimeSpaceIndex(UINT32 time)
{
	time_t CurTime = time;

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	{
		if ( (pTm->tm_hour > GameParam.m_XuanTianTime[i] || (pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min > GameParam.m_XuanTianTime[i + 1]))
			&& ( pTm->tm_hour < GameParam.m_XuanTianTime[i + 2] || (pTm->tm_hour == GameParam.m_XuanTianTime[i + 2] && pTm->tm_min < GameParam.m_XuanTianTime[i + 3])) )
		{
			return i / 4 + 1;
		}
	}

	return 0;
}

//得到玄天BOSS信息
void		XuanTianFuBen::HandleGetXTBossInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//检测当前是否开放玄天
	UINT8 index = this->InOpenTimeSpaceIndex(CURRENT_TIME());

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		if ( index > 0 )
		{
			this->StartXuanTian();
		}
		else
		{
			this->SetStartTimer();
		}

		return;
	}

	std::vector<DBCreateXTBoss> vecDBCreateBoss;

	bool bFirst = true;

	bool bChangeData = true;

	int num = RspIb.Remain() / sizeof(SDB_XTBossInfo);

	bool bOver = true;

	for ( int i = 0; i < num; ++i )
	{
		SDB_XTBossInfo BossInfo;

		RspIb >> BossInfo;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		if ( bFirst )
		{
			UINT32 RecordTime = BossInfo.m_RecordTime;

			UINT8 index2 = this->InOpenTimeSpaceIndex(RecordTime);

			if ( index2 > 0 )
			{
				//关服时是在玄天开放时
				UINT32 Curtime = CURRENT_TIME();
				if ( index == index2 && XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),RecordTime))
				{
					//开服务器还在继续这次的玄天,所有数据不变
					bChangeData = false;
				}
			}
			else
			{
				bChangeData = false;
			}

			bFirst = false;
		}

		if ( !bChangeData && index > 0 )
		{
			//使用数据库的数据创建BOSS
			DBCreateXTBoss DBBoss;
			
			DBBoss.m_MonsterID = BossInfo.m_MonsterID;
			DBBoss.m_uidBoss   = UID(BossInfo.m_UIDBoss);
			DBBoss.m_CurBlood  = BossInfo.m_CurBlood;

			vecDBCreateBoss.push_back(DBBoss);

			if ( DBBoss.m_CurBlood > 0 )
			{
				bOver = false;
			}
		}
	}

	if ( bChangeData )
	{
		//上次玄天结束，修改数据库
		this->Clear_BossInfo();

		this->ChangeDB_LastInfo();
	}

	if ( index > 0 )
	{
		//开启玄天
		if ( vecDBCreateBoss.size() > 0 )
		{
			//使用数据库数据创建BOSS
			this->StartXuanTian(&vecDBCreateBoss);

			m_bOver = bOver;
		}
		else
		{
			this->StartXuanTian();
		}
	}
	else
	{
		this->SetStartTimer();
	}
}

//得到玄天伤害数据
void		XuanTianFuBen::HandleGetXTDamage(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
		return;

	int num = RspIb.Remain() / sizeof(SDB_XTDamage);

	for ( int i = 0; i < num; ++i )
	{
		SDB_XTDamage Damage;

		RspIb >> Damage;

		if( RspIb.Error() )
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		XTActorInfo Info;

		Info.m_AttackNum = Damage.m_AttackNum;
		Info.m_Damage	 = Damage.m_Damage;
		Info.m_Level	 = Damage.m_Level;
		strncpy(Info.m_Name, Damage.m_UserName, sizeof(Info.m_Name));
		Info.m_uidUser	 = UID(Damage.m_UIDUser);

		if ( Damage.m_IsCurrent )
		{
			//上次伤害
			int index = this->GetLevelGroupIndex(Info.m_Level);

			if ( -1 == index || index >= m_vectLastDamageRank.size() )
				continue;

			VEC_XTACTORINFO & vecXTActorInfo = m_vectLastDamageRank[index];

			vecXTActorInfo.push_back(Info);		
		}
		else
		{
			//当前伤害
			m_mapDamage[Info.m_uidUser] = Info;
		}
	}

	for ( int i = 0; i < m_vectLastDamageRank.size(); ++i )
	{
		VEC_XTACTORINFO & vecXTActorInfo = m_vectLastDamageRank[i];

		for ( int k = 0; k < vecXTActorInfo.size(); ++k )
		{
			std::sort(vecXTActorInfo.begin(), vecXTActorInfo.end(), sortdamage);
		}
	}
}

//得到玄天BOSS杀死数据
void		XuanTianFuBen::HandleGetXTBossDie(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
		return;

	int num = RspIb.Remain() / sizeof(SDB_XTBossDie);

	for ( int i = 0; i < num; ++i )
	{
		SDB_XTBossDie BossDie;

		RspIb >> BossDie;

		if( RspIb.Error() )
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		BossLastKill Info;

		Info.m_AttackNum = BossDie.m_AttackNum;
		Info.m_CurDamage = BossDie.m_CurDamage;
		strncpy(Info.m_szBossName, BossDie.m_BossName, sizeof(Info.m_szBossName));
		strncpy(Info.m_szUserName, BossDie.m_UserName, sizeof(Info.m_szUserName));
		Info.m_TotalDamage = BossDie.m_TotalDamage;
		Info.m_uidUser   = UID(BossDie.m_UIDUser);
		Info.m_UserLevel = BossDie.m_UserLevel;

		if ( BossDie.m_UIDBoss != UID().ToUint64() )
		{
			//当前玄天
			m_mapKillUser[UID(BossDie.m_UIDBoss)] = Info;
		}
		else
		{
			//上一次玄天
			m_vecLastKillUser.push_back(Info);
		}
	}
}

//清除数据库的BOSS信息
void		XuanTianFuBen::Clear_BossInfo()
{
	//清除数据库BOSS信息
	SDB_Delete_XTBossInfo_Req DBBossInfo;

	OBuffer1k ob;
	ob << DBBossInfo;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Delete_XTBossInfo,ob.TakeOsb(),0,0);
}

//更改数据库上一次玄天信息
void		XuanTianFuBen::ChangeDB_LastInfo()
{
	//数据库删除上一次玄天BOSS死亡信息，并把BOSS死亡信息转为上一次BOSS死亡信息
	SDB_Change_XTBossDie_Req DBBossDie;

	OBuffer1k ob;
	ob << DBBossDie;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Change_XTBossDie,ob.TakeOsb(),0,0);

	//数据库删除上一次伤害，并把当前伤害转为上一次伤害
	SDB_Change_XTLastDamage_Req DBDamage;

	ob.Reset();
	ob << DBDamage;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Change_XTLastDamage,ob.TakeOsb(),0,0);
}

//保存BOSS数据
void		XuanTianFuBen::SaveBossInfo(IMonster * pBoss)
{
	SDB_Save_XTBossInfo_Req DBReq;

	DBReq.m_CurBlood   = pBoss->GetCrtProp(enCrtProp_Blood);
	DBReq.m_MonsterID  = pBoss->GetCrtProp(enCrtProp_MonsterID);
	DBReq.m_RecordTime = CURRENT_TIME();
	DBReq.m_UIDBoss    = pBoss->GetUID().ToUint64();

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(XUAN_TIAN_DB,enDBCmd_Save_XTBossInfo,ob.TakeOsb(),0,0);
}

//检测玄天是否结束
bool		XuanTianFuBen::Check_OverXT()
{
	if ( !m_bOpen || m_bOver )
		return true;

	std::map<UID, TMonsterID>::iterator iter = m_mapBoss.begin();

	for ( ; iter != m_mapBoss.end() ; ++iter )
	{
		IMonster * pBoss = g_pGameServer->GetGameWorld()->FindMonster(iter->first);

		if ( 0 == pBoss )
			continue;

		if ( pBoss->GetCrtProp(enCrtProp_Blood) > 0 )
		{
			return false;
		}
	}

	return true;
}

//距离下次开启玄天的秒数
UINT32	XuanTianFuBen::GetNextOpenTimeNum()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	{
		if ( pTm->tm_hour < GameParam.m_XuanTianTime[i] || ( pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min < GameParam.m_XuanTianTime[i + 1] ) )
		{
			return (GameParam.m_XuanTianTime[i] - pTm->tm_hour) * 3600 + (GameParam.m_XuanTianTime[i + 1] - pTm->tm_min ) * 60 - pTm->tm_sec;
		}

		if ( pTm->tm_hour > GameParam.m_XuanTianTime[i + 2] || ( pTm->tm_hour == GameParam.m_XuanTianTime[i + 2] && pTm->tm_min >= GameParam.m_XuanTianTime[i + 3] ) )
		{
			continue;
		}

		//正在开启时间段
		return 0;
	}

	//今天的玄天已全部结束，返回现在到明天第一场的秒数
	return (24 - pTm->tm_hour) * 3600 - pTm->tm_min * 60 - pTm->tm_sec + GameParam.m_XuanTianTime[0] * 3600 + GameParam.m_XuanTianTime[1] * 60;
}

//距离玄天结束的秒数
UINT32	XuanTianFuBen::GetOverTimeNum()
{
	time_t CurTime = CURRENT_TIME();

	tm * pTm = localtime(&CurTime);

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	for ( int i = 0; i + 3 < GameParam.m_XuanTianTime.size(); i += 4 )
	{
		if ( pTm->tm_hour < GameParam.m_XuanTianTime[i] || ( pTm->tm_hour == GameParam.m_XuanTianTime[i] && pTm->tm_min < GameParam.m_XuanTianTime[i + 1] ) )
		{
			return 0;
		}

		if ( pTm->tm_hour > GameParam.m_XuanTianTime[i + 2] || ( pTm->tm_hour == GameParam.m_XuanTianTime[i + 2] && pTm->tm_min > GameParam.m_XuanTianTime[i + 3] ) )
		{
			continue;
		}

		//正在开启时间段
		return (GameParam.m_XuanTianTime[i + 2] - pTm->tm_hour) * 3600 + (GameParam.m_XuanTianTime[i + 3] - pTm->tm_min) * 60 - pTm->tm_sec;
	}

	//今天的玄天已全部结束
	return 0;
}

//设置开启定时器
void	XuanTianFuBen::SetStartTimer()
{
	UINT32 timeNum = this->GetNextOpenTimeNum();

	if ( timeNum > 0 )
	{
		//创建下次开始定时器
		g_pGameServer->GetTimeAxis()->SetTimer(enXuanTianTimerID_Start,this,timeNum * 1000,"XuanTianFuBen::SetStartTimer");
	}
}

//得到玄天BOSS配置信息
void	XuanTianFuBen::HandleGetXTBossCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
		return;

	int num = RspIb.Remain() / sizeof(SDB_MonsterCnfg);

	for ( int i = 0; i < num; ++i )
	{
		SDB_MonsterCnfg Data;

		RspIb >> Data;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SMonsterCnfg MonsterCnfg;

		MonsterCnfg.m_MonsterID = Data.m_MonsterID;
		strncpy((char *)MonsterCnfg.m_szName, (const char *)Data.m_szName, sizeof(MonsterCnfg.m_szName));
		MonsterCnfg.m_Level = Data.m_Level;
		MonsterCnfg.m_Spirit = Data.m_Spirit;
		MonsterCnfg.m_Shield = Data.m_Shield;
		MonsterCnfg.m_Blood = Data.m_Blood;
		MonsterCnfg.m_Avoid = Data.m_Avoid;
		MonsterCnfg.m_MagicValue = Data.m_MagicValue;
		MonsterCnfg.m_Swordkee = Data.m_Swordkee;
		MonsterCnfg.m_Facade = Data.m_Facade;
		MonsterCnfg.m_MagicNum = Data.m_MagicNum;
		MonsterCnfg.m_MonsterNameLangID = Data.m_MonsterNameLangID;
		MonsterCnfg.Crit = Data.Crit;
		MonsterCnfg.Tenacity = Data.Tenacity;
		MonsterCnfg.Hit = Data.Hit;
		MonsterCnfg.Dodge = Data.Dodge;
		MonsterCnfg.MagicCD = Data.MagicCD;

		g_pGameServer->GetConfigServer()->Push_MonsterCnfg(MonsterCnfg);
	}
}
