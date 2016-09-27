#include "Pair.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "ITeamPart.h"
#include "DuoBaoWar.h"
#include "IDouFaPart.h"


bool   Pair::Create(UID uidActor, UID uidEnemy, bool bTeamGroup, DuoBaoWar * pDuoBaoWar)
{
	if ( 0 == pDuoBaoWar )
		return false;

	m_bCombatOverDelete = true;

	m_bCombatOver = false;

	m_pDuoBaoWar = pDuoBaoWar;

	m_bInCombat = false;

	if ( !bTeamGroup )
	{
		ActorPairInfo Info;

		Info.m_uidActor = uidActor;

		m_vecUser.push_back(Info);

		Info.m_uidActor = uidEnemy;

		m_vecEenmy.push_back(Info);
	}
	else
	{
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidActor);

		IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);

		if ( 0 == pActor || 0 == pEnemy )
		{
			TRACE("<error> %s ; %d Line 获取夺宝玩家失败！！", __FUNCTION__, __LINE__);
			return false;
		}

		ITeamPart * pTeamPart = pActor->GetTeamPart();

		if ( 0 == pTeamPart )
			return false;

		IActor * pMember = pTeamPart->GetTeamMember();

		if ( 0 == pMember )
			return false;

		ActorPairInfo Info;

		Info.m_uidActor = uidActor;

		ActorPairInfo Info2;

		Info2.m_uidActor = pMember->GetUID();

		//夺宝等级大的放第一位
		if ( pMember->GetCrtProp(enCrtProp_DuoBaoLevel) > pActor->GetCrtProp(enCrtProp_DuoBaoLevel) )
		{
			m_vecUser.push_back(Info2);
			m_vecUser.push_back(Info);
		}
		else
		{
			m_vecUser.push_back(Info);
			m_vecUser.push_back(Info2);		
		}

		pTeamPart = pEnemy->GetTeamPart();

		if ( 0 == pTeamPart )
			return false;

		pMember = pTeamPart->GetTeamMember();

		if ( 0 == pMember )
			return false;

		Info.m_uidActor = uidEnemy;

		Info2.m_uidActor = pMember->GetUID();

		//夺宝等级大的放第一位
		if (  pMember->GetCrtProp(enCrtProp_DuoBaoLevel) > pEnemy->GetCrtProp(enCrtProp_DuoBaoLevel) )
		{
			m_vecEenmy.push_back(Info2);
			m_vecEenmy.push_back(Info);
		}
		else
		{
			m_vecEenmy.push_back(Info);
			m_vecEenmy.push_back(Info2);		
		}
	}

	static int id = 0;

	++id;

	TRACE("创建总配对数:%d", id);

	return true;
}

//战斗
void	Pair::Combat()
{
	if ( m_bInCombat )
		return;

	TMapID CombatMapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_DuoBaoCombatMapID;

	const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(CombatMapID);

	if ( 0 == pMapConfig )
	{
		TRACE("<error> %s : %d Line 获取地图配置信息出借！！地图ID = %d", __FUNCTION__, __LINE__, CombatMapID);
		return;
	}

	//创建战斗场景
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(CombatMapID, 0, true);

	if ( 0 == pGameScene )
	{
		TRACE("<error> %s : %d Line 创建玄天摆怪场景失败！！地图ID = %d", __FUNCTION__, __LINE__, CombatMapID);
		return;
	}

	//得到战斗的参战人员
	enJoinPlayer JoinPlayer = this->GetJoinPlayer();

	//得到战斗时的模式
	enCombatMode CombetMode = this->GetCombatMode();

	//得到一个选择参战的玩家（敌我双方各一个）
	IActor * pActor = this->GetCombatUser(false);

	IActor * pEnemy = this->GetCombatUser(true);

	if ( 0 == pActor || 0 == pEnemy )
	{
		TRACE("<error> %s : %d Line 获取夺宝玩家失败！！", __FUNCTION__, __LINE__);
		return;
	}

	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if ( 0 == pCombatPart )
		return;

	TRACE("夺宝：玩家%s和玩家%s开始夺宝战斗！！", pActor->GetName(), pEnemy->GetName());

	UINT64 CombatID;

	//通知创建敌方
	this->NoticeCreateEnemy();

	m_bInCombat = true;

	//清除玩家的夺宝数据
	this->ClearDuoBaoData();

	pCombatPart->CombatWithScene(enCombatType_DuoBao, pEnemy->GetUID(), CombatID, this, g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID), pGameScene, JoinPlayer, CombetMode);
}

//发送倒计时和选择战斗框
void	Pair::SendReadyCombat()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_vecUser[0].m_uidActor);

	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(m_vecEenmy[0].m_uidActor);

	if ( 0 == pActor || 0 == pEnemy )
	{
		TRACE("<error> %s : %d Line 获取夺宝玩家失败！！", __FUNCTION__, __LINE__);
		return;
	}

	SC_PopReady Rsp;
	OBuffer1k ob;

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		IActor * pTmpActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pTmpActor )
			return;

		strncpy(Rsp.m_szName, pEnemy->GetName(), sizeof(Rsp.m_szName));

		ob.Reset();
		ob <<  DouFaHeader(enDouFaCmd_SC_PopReady, sizeof(Rsp)) << Rsp;
		pTmpActor->SendData(ob.TakeOsb());
	}

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		IActor * pTmpActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pTmpActor )
			return;

		strncpy(Rsp.m_szName, pActor->GetName(), sizeof(Rsp.m_szName));

		ob.Reset();
		ob <<  DouFaHeader(enDouFaCmd_SC_PopReady, sizeof(Rsp)) << Rsp;
		pTmpActor->SendData(ob.TakeOsb());
	}
}

//战斗结束了
void	Pair::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	//发事件
	SS_OnCombat EnentCombat;

	EnentCombat.CombatPlace = enCombatPlace_DuoBao;
	EnentCombat.bWin = pCombatResult->m_bWin;
	EnentCombat.bIsTeam = pCombatCnt->bIsTeam;
	EnentCombat.bHard = false;

	IActor * pBacker = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( 0 == pBacker )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnCombat);
	pBacker->OnEvent(msgID,&EnentCombat,sizeof(EnentCombat));

	if ( pCombatCnt->uidSource != pCombatCnt->uidUser )
		return;

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidSource);

	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidEnemy);

	if ( 0 == pActor || 0 == pEnemy )
	{
		TRACE("<error> %s ; %d Line 获取不到夺宝玩家！！", __FUNCTION__, __LINE__);
		return;
	}

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		if ( pActor->GetUID() == m_vecUser[i].m_uidActor )
		{
			if ( pCombatResult->m_bWin )
			{
				this->CombatForward(m_vecEenmy, m_vecUser, pEnemy, pActor);
			}
			else
			{
				this->CombatForward(m_vecUser, m_vecEenmy, pActor,pEnemy);
			}
			return;
		}

		if ( pEnemy->GetUID() == m_vecUser[i].m_uidActor )
		{
			if ( pCombatResult->m_bWin )
			{
				this->CombatForward(m_vecEenmy, m_vecUser, pActor,pEnemy);
			}
			else
			{
				this->CombatForward(m_vecUser, m_vecEenmy, pEnemy, pActor);
			}
			return;		
		}
	}
}

//回调结束
void	Pair::BackOver()
{
	m_bCombatOver = true;

	if ( m_bCombatOverDelete )
	{
		this->Release();
	}
}

//选择退出战斗
void	Pair::QuitCombat(IActor * pActor)
{
	if ( m_bInCombat )
		return;

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		if ( m_vecUser[i].m_uidActor == pActor->GetUID() )
		{
			if ( m_vecUser[i].m_Status == enReadyStatus_No || m_vecUser[i].m_Status == enReadyStatus_Combat )
			{
				this->QuitForward(pActor->GetUID(), m_vecUser[0].m_uidActor);
			}

			m_vecUser[i].m_Status = enReadyStatus_Quit;
			
			return;
		}
	}

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		if ( m_vecEenmy[i].m_uidActor == pActor->GetUID() )
		{
			if ( m_vecEenmy[i].m_Status == enReadyStatus_No || m_vecEenmy[i].m_Status == enReadyStatus_Combat )
			{
				this->QuitForward(pActor->GetUID(), m_vecEenmy[0].m_uidActor);
			}

			m_vecEenmy[i].m_Status = enReadyStatus_Quit;

			return;
		} 
	}
}

//选择战斗
void	Pair::SelectCombat(IActor * pActor)
{
	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		if ( m_vecUser[i].m_uidActor == pActor->GetUID() )
		{
			m_vecUser[i].m_Status = enReadyStatus_Combat;

			if ( !this->Check_Combat() )
			{
				//通知弹出等待框
				this->NoticeWaitTip(pActor, true);
			}

			return;
		}
	}

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		if ( m_vecEenmy[i].m_uidActor == pActor->GetUID() )
		{
			m_vecEenmy[i].m_Status = enReadyStatus_Combat;

			if ( !this->Check_Combat() )
			{
				//通知弹出等待框
				this->NoticeWaitTip(pActor, true);
			}
			return;
		}
	}
}

//检测是否能战斗
bool	Pair::Check_CanCombat()
{
	bool bOk1 = false,bOk2 = false;

	//每方都必须有一个参战才可战斗
	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		if ( Info.m_Status == enReadyStatus_No )
		{
			//惩罚
			this->QuitForward(Info.m_uidActor, m_vecEenmy[0].m_uidActor);
		}
		else if ( Info.m_Status == enReadyStatus_Combat )
		{
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

			if ( 0 == pActor )
				continue;

			this->NoticeWaitTip(pActor, false);
			bOk1 = true;
		}
	}

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		if ( Info.m_Status == enReadyStatus_No )
		{
			//惩罚
			this->QuitForward(Info.m_uidActor, m_vecUser[0].m_uidActor);
		}
		else if ( Info.m_Status == enReadyStatus_Combat )
		{
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

			if ( 0 == pActor )
				continue;

			this->NoticeWaitTip(pActor, false);
			bOk2 = true;
		}
	}

	if ( bOk1 && bOk2 )
		return true;

	this->ClearDuoBaoData();

	return false;
}

//得到是否战斗结束就删除
bool	Pair::GetIsCombatOverDelete()
{
	return m_bCombatOverDelete;
}

//设置是否战斗结束就删除
void	Pair::SetIsCombatOverDelete(bool bCombatOverDelete)
{
	m_bCombatOverDelete = bCombatOverDelete;
}

//得到战斗是否结束
bool	Pair::GetCombatIsOver()
{
	return m_bCombatOver;
}

//得到参战人员
enJoinPlayer Pair::GetJoinPlayer()
{
	enJoinPlayer JoinPlayer1 = m_vecUser.size() > 1 ? enJoinPlayer_OneselfTeam : enJoinPlayer_Oneself;

	if ( JoinPlayer1 == enJoinPlayer_OneselfTeam )
	{
		//组队时，判断是否有玩家不参战
		for ( int i = 0; i < m_vecUser.size(); ++i )
		{
			ActorPairInfo & Info = m_vecUser[i];

			if ( Info.m_Status != enReadyStatus_Combat )
			{
				JoinPlayer1 = enJoinPlayer_Oneself;
				break;
			}
		}
	}

	enJoinPlayer JoinPlayer2 = m_vecEenmy.size() > 1 ? enJoinPlayer_EnemyTeam : enJoinPlayer_OnlyEnemy;

	if ( JoinPlayer2 == enJoinPlayer_EnemyTeam )
	{
		//组队时，判断是否有玩家不参战
		for ( int i = 0; i < m_vecEenmy.size(); ++i )
		{
			ActorPairInfo & Info = m_vecEenmy[i];

			if ( Info.m_Status != enReadyStatus_Combat )
			{
				JoinPlayer2 = enJoinPlayer_OnlyEnemy;
				break;
			} 
		}	
	}

	return enJoinPlayer(JoinPlayer1 | JoinPlayer2);
}

//得到战斗模式
enCombatMode Pair::GetCombatMode()
{
	enCombatMode CombatMode1 = m_vecUser.size() > 1 ? enCombatMode_OneselfTeam : enCombatMode_Oneself;

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		if ( Info.m_Status != enReadyStatus_Combat )
		{
			CombatMode1 = enCombatMode_Oneself;
		} 
	}

	enCombatMode CombatMode2 = m_vecEenmy.size() > 1 ? enCombatMode_EnemyTeam : enCombatMode_OnlyEnemy;

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		if ( Info.m_Status != enReadyStatus_Combat )
		{
			CombatMode2 = enCombatMode_OnlyEnemy;
		} 
	}

	return enCombatMode(CombatMode1 | CombatMode2);
}

//得到一个参战人员
IActor * Pair::GetCombatUser(bool bEnemy)
{
	if ( bEnemy )
	{
		for ( int i = 0; i < m_vecEenmy.size(); ++i )
		{
			ActorPairInfo & Info = m_vecEenmy[i];

			if ( Info.m_Status == enReadyStatus_Combat )
			{
				return g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);
			}
		}
	}
	else
	{
		for ( int i = 0; i < m_vecUser.size(); ++i )
		{
			ActorPairInfo & Info = m_vecUser[i];

			if ( Info.m_Status == enReadyStatus_Combat )
			{
				return g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);
			}
		}	
	}

	return 0;
}

//失败惩罚
void	Pair::QuitForward(UID uidFailer, UID uidWin)
{
	IActor * pFailer = g_pGameServer->GetGameWorld()->FindActor(uidFailer);

	IActor * pWiner = g_pGameServer->GetGameWorld()->FindActor(uidWin);

	if ( 0 == pFailer || 0 == pWiner )
	{
		TRACE("<error> %s : %d Line 获取不到夺宝玩家！！", __FUNCTION__, __LINE__);
		return;
	}

	//减少夺宝等级=失败夺宝等级系数×（失败方夺宝等级/胜利方夺宝等级）
	const SDuoBaoCnfg * pFailCnfg = g_pGameServer->GetConfigServer()->GetDuoBaoCnfg(pFailer->GetCrtProp(enCrtProp_DuoBaoLevel));

	if ( 0 == pFailCnfg )
	{
		TRACE("<error> %s : %d Line 获取失败惩罚配置信息失败！！夺宝等级%d", __FUNCTION__, __LINE__, pFailer->GetCrtProp(enCrtProp_DuoBaoLevel));
		return;
	}

	bool bTeamGroup = (m_vecUser.size() > 1); 

	//减少的夺宝等级
	INT32 DecDuoBaoLv = 0;

	float DuoBapParam = pow(10.0f,(float)((pWiner->GetCrtProp(enCrtProp_DuoBaoLevel) - pFailer->GetCrtProp(enCrtProp_DuoBaoLevel)) / 400.0f )) + 1.0f;

	if ( DuoBapParam != 0 )
	{
		if ( bTeamGroup )
		{
			DecDuoBaoLv = pFailCnfg->m_TeamR * (  pFailCnfg->m_FailRes - (1.0f / DuoBapParam) ) + 0.9999999;
		}
		else
		{
			DecDuoBaoLv = pFailCnfg->m_SingleR * (  pFailCnfg->m_FailRes - (1.0f / DuoBapParam) ) + 0.9999999;
		}
	}

	if ( DecDuoBaoLv != 0 )
	{
		if ( (pFailer->GetCrtProp(enCrtProp_DuoBaoLevel) + DecDuoBaoLv) < 0 )
		{
			pFailer->SetCrtProp(enCrtProp_DuoBaoLevel, 0);
		}
		else
		{
			pFailer->AddCrtPropNum(enCrtProp_DuoBaoLevel, DecDuoBaoLv);
		}
	}

	//增加的声望值
	IDouFaPart * pDouFaPart = pFailer->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	if ( bTeamGroup )
	{
		if ( pFailCnfg->m_TeamRunGetCredit != 0 )
		{
			pDouFaPart->AddCreditDuoBao(pFailCnfg->m_TeamFailGetCredit);
		}
	}
	else
	{
		if ( pFailCnfg->m_SingleRunGetCredit != 0 )
		{
			pDouFaPart->AddCreditDuoBao(pFailCnfg->m_SingleFailGetCredit);
		}
	}

	//开启逃跑惩罚
	pDouFaPart->StartRunTime();

	pDouFaPart->ClearDuoBaoData();
}


//战斗奖励
void	Pair::CombatForward(std::vector<ActorPairInfo> & vecFailer, std::vector<ActorPairInfo> & vecWiner, IActor * pFailer, IActor * pWiner)
{
	//增加夺宝等级=胜利夺宝等级系数×（失败方夺宝等级/胜利方夺宝等级）
	const SDuoBaoCnfg * pWinCnfg = g_pGameServer->GetConfigServer()->GetDuoBaoCnfg(pWiner->GetCrtProp(enCrtProp_DuoBaoLevel));

	if ( 0 == pWinCnfg )
	{
		TRACE("<error> %s : %d Line 获取失败惩罚配置信息失败！！夺宝等级%d", __FUNCTION__, __LINE__, pWiner->GetCrtProp(enCrtProp_DuoBaoLevel));
		return;
	}

	//减少夺宝等级=失败夺宝等级系数×（失败方夺宝等级/胜利方夺宝等级）
	const SDuoBaoCnfg * pFailCnfg = g_pGameServer->GetConfigServer()->GetDuoBaoCnfg(pFailer->GetCrtProp(enCrtProp_DuoBaoLevel));

	if ( 0 == pFailCnfg )
	{
		TRACE("<error> %s : %d Line 获取失败惩罚配置信息失败！！夺宝等级%d", __FUNCTION__, __LINE__, pFailer->GetCrtProp(enCrtProp_DuoBaoLevel));
		return;
	}

	INT32 FailDuoBaoLv = pFailer->GetCrtProp(enCrtProp_DuoBaoLevel);

	INT32 WinDuoBaoLv = pWiner->GetCrtProp(enCrtProp_DuoBaoLevel);

	bool bTeamGroup = (m_vecUser.size() > 1);

	SC_CombatResult Rsp;
	OBuffer1k ob;

	//胜利方增加
	for ( int i = 0; i < vecWiner.size(); ++i )
	{
		if ( vecWiner[i].m_Status != enReadyStatus_Combat )
			continue;

		IActor * pUserWin = g_pGameServer->GetGameWorld()->FindActor(vecWiner[i].m_uidActor);

		if ( 0 == pUserWin )
			continue;

		DuoBaoCombatResult Result;

		strncpy(Result.m_szName, pUserWin->GetName(), sizeof(Result.m_szName));
		Result.m_Level = pUserWin->GetCrtProp(enCrtProp_Level);
		Result.m_CombatAbility = pUserWin->GetCrtProp(enCrtProp_ActorCombatAbility);
		Result.m_NenLiType = (UINT8)pUserWin->GetNenLi();

		//胜利方增加的夺宝等级
		INT32 AddDuoBaoLv = 0;

		float DuoBapParam = pow(10.0f,(float)((FailDuoBaoLv - pUserWin->GetCrtProp(enCrtProp_DuoBaoLevel)) / 400.0f )) + 1.0f;

		if ( DuoBapParam != 0 )
		{
			if ( bTeamGroup )
			{
				AddDuoBaoLv = pWinCnfg->m_TeamR * (  pWinCnfg->m_WinRes - (1.0f / DuoBapParam) ) + 0.9999999;
			}
			else
			{
				AddDuoBaoLv = pWinCnfg->m_SingleR * (  pWinCnfg->m_WinRes - (1.0f / DuoBapParam) ) + 0.9999999;
			}
		}

		if ( AddDuoBaoLv > 0 )
		{
			if ( (WinDuoBaoLv + AddDuoBaoLv) < 0 )
			{
				pUserWin->SetCrtProp(enCrtProp_DuoBaoLevel, 0);
			}
			else
			{
				pUserWin->AddCrtPropNum(enCrtProp_DuoBaoLevel, AddDuoBaoLv);
			}
		}

		Result.m_AddDuoBaoLevel = AddDuoBaoLv;

		ob << Result;

		++Rsp.m_Num;

		//胜利方增加的声望
		IDouFaPart * pDouFaPart = pUserWin->GetDouFaPart();

		if ( 0 == pDouFaPart )
			return;
		
		if ( bTeamGroup )
		{
			pDouFaPart->AddCreditDuoBao(pWinCnfg->m_TeamWinGetCredit);
		}
		else
		{
			pDouFaPart->AddCreditDuoBao(pWinCnfg->m_SingleWinGetCredit);
		}
	}

	//失败方减少或者减少
	for ( int i = 0; i < vecFailer.size(); ++i )
	{
		if ( vecFailer[i].m_Status != enReadyStatus_Combat )
			continue;

		IActor * pUserFail = g_pGameServer->GetGameWorld()->FindActor(vecFailer[i].m_uidActor);

		if ( 0 == pUserFail )
			continue;

		DuoBaoCombatResult Result;

		strncpy(Result.m_szName, pUserFail->GetName(), sizeof(Result.m_szName));
		Result.m_Level = pUserFail->GetCrtProp(enCrtProp_Level);
		Result.m_CombatAbility = pUserFail->GetCrtProp(enCrtProp_ActorCombatAbility);
		Result.m_NenLiType = (UINT8)pUserFail->GetNenLi();

		//失败方减少的夺宝等级
		INT32 DecDuoBaoLv = 0;

		float DuoBapParam = pow(10.0f,(float)((WinDuoBaoLv - pUserFail->GetCrtProp(enCrtProp_DuoBaoLevel)) / 400.0f )) + 1.0f;

		if ( DuoBapParam != 0 )
		{
			if ( bTeamGroup )
			{
				DecDuoBaoLv = pFailCnfg->m_TeamR * (  pFailCnfg->m_FailRes - (1.0f / DuoBapParam) ) + 0.9999999;
			}
			else
			{
				DecDuoBaoLv = pFailCnfg->m_SingleR * (  pFailCnfg->m_FailRes - (1.0f / DuoBapParam) ) + 0.9999999;
			}
		}

		if ( DecDuoBaoLv != 0 )
		{
			if ( (FailDuoBaoLv + DecDuoBaoLv) < 0 )
			{
				DecDuoBaoLv = -FailDuoBaoLv;
				pUserFail->SetCrtProp(enCrtProp_DuoBaoLevel, 0);
			}
			else
			{
				pUserFail->AddCrtPropNum(enCrtProp_DuoBaoLevel, DecDuoBaoLv);
			}
		}

		Result.m_AddDuoBaoLevel = DecDuoBaoLv;

		ob << Result;
		++Rsp.m_Num;
		
		//失败方增加的声望值
		IDouFaPart * pFailerDouFaPart = pUserFail->GetDouFaPart();

		if ( 0 == pFailerDouFaPart )
			return;

		if ( bTeamGroup )
		{
			pFailerDouFaPart->AddCreditDuoBao(pFailCnfg->m_TeamRunGetCredit);
		}
		else
		{
			pFailerDouFaPart->AddCreditDuoBao(pFailCnfg->m_SingleRunGetCredit);
		}
	}

	//给该配对所有玩家发送战斗结果信息
	OBuffer1k ob2;

	for ( int i = 0; i < vecWiner.size(); ++i )
	{
		if ( vecWiner[i].m_Status != enReadyStatus_Combat )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(vecWiner[i].m_uidActor);

		if ( 0 == pActor )
			continue;

		ob2.Reset();
		ob2 <<  DouFaHeader(enDouFaCmd_CombatResult, sizeof(Rsp) + ob.Size()) << Rsp;

		if ( ob.Size() > 0 )
		{
			ob2 << ob;
		}

		pActor->SendData(ob2.TakeOsb());
	}

	for ( int i = 0; i < vecFailer.size(); ++i )
	{
		if ( vecFailer[i].m_Status != enReadyStatus_Combat )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(vecFailer[i].m_uidActor);

		if ( 0 == pActor )
			continue;

		ob2.Reset();
		ob2 <<  DouFaHeader(enDouFaCmd_CombatResult, sizeof(Rsp) + ob.Size()) << Rsp;

		if ( ob.Size() > 0 )
		{
			ob2 << ob;
		}

		pActor->SendData(ob2.TakeOsb());
	}
}

//删除
void	Pair::Release()
{
	static int id = 0;

	++id;

	TRACE("删除总配对数:%d", id);

	delete this;
}

//通知创建敌方
void	Pair::NoticeCreateEnemy()
{
	//得到甲方参战人员
	std::vector<UID>	vecCombat1;

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		if ( Info.m_Status == enReadyStatus_Quit )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		ICombatPart * pCombatPart = pActor->GetCombatPart();

		if ( 0 == pCombatPart )
			continue;

		std::vector<SCreatureLineupInfo> vecCreature = pCombatPart->GetJoinBattleActor((m_vecUser.size() > 1));

		for ( int index = 0; index < vecCreature.size(); ++index )
		{
			if ( vecCreature[index].m_pCreature == 0 )
				continue;

			vecCombat1.push_back(vecCreature[index].m_pCreature->GetUID());
		}
	}

	//得到乙方参战人员
	std::vector<UID>	vecCombat2;

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		if ( Info.m_Status == enReadyStatus_Quit )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		ICombatPart * pCombatPart = pActor->GetCombatPart();

		if ( 0 == pCombatPart )
			continue;

		std::vector<SCreatureLineupInfo> vecCreature = pCombatPart->GetJoinBattleActor((m_vecUser.size() > 1));

		for ( int index = 0; index < vecCreature.size(); ++index )
		{
			if ( vecCreature[index].m_pCreature == 0 )
				continue;

			vecCombat2.push_back(vecCreature[index].m_pCreature->GetUID());
		}	
	}

	//通知甲方创建乙方参战人员
	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		if ( Info.m_Status == enReadyStatus_Quit )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		for ( int index = 0; index < vecCombat2.size(); ++index )
		{
			pActor->NoticClientCreatePrivateThing(vecCombat2[index]);
		}
	}

	//通知乙方创建甲方参战人员
	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		if ( Info.m_Status == enReadyStatus_Quit )
			continue;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		for ( int index = 0; index < vecCombat1.size(); ++index )
		{
			pActor->NoticClientCreatePrivateThing(vecCombat1[index]);
		}
	}
}

//玩家退出或者离队时，检测是否结束
bool	Pair::Check_Over()
{
	bool bOver = true;

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		if ( m_vecUser[i].m_Status == enReadyStatus_No || m_vecUser[i].m_Status == enReadyStatus_Combat )
		{
			bOver = false;
		}
	}

	if ( bOver )
	{
		SC_ViewRetCode Rsp;
		OBuffer1k ob;

		Rsp.m_Result = enDouFaRetCode_EnemyQuit;

		for ( int i = 0; i < m_vecEenmy.size(); ++i )
		{
			//通知对方退出，战斗胜利
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_vecEenmy[i].m_uidActor);

			if ( 0 == pActor )
				continue;

			ob.Reset();
			ob <<  DouFaHeader(enDouFaCmd_ViewRetCode, sizeof(Rsp)) << Rsp;
			pActor->SendData(ob.TakeOsb());

			//在选择战斗中的，关闭等待页面
			if ( m_vecEenmy[i].m_Status == enReadyStatus_Combat || m_vecEenmy[i].m_Status == enReadyStatus_No )
			{
				this->NoticeWaitTip(pActor, false);
			}
		}

		this->ClearDuoBaoData();

		m_bCombatOver = true;

		return true;
	}

	bOver = true;

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		if ( m_vecEenmy[i].m_Status == enReadyStatus_No || m_vecEenmy[i].m_Status == enReadyStatus_Combat )
		{
			bOver = false;
		}
	}

	if ( bOver )
	{
		SC_ViewRetCode Rsp;
		OBuffer1k ob;

		Rsp.m_Result = enDouFaRetCode_EnemyQuit;

		for ( int i = 0; i < m_vecUser.size(); ++i )
		{
			//通知对方退出，战斗胜利
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_vecUser[i].m_uidActor);

			if ( 0 == pActor )
				continue;

			ob.Reset();
			ob <<  DouFaHeader(enDouFaCmd_ViewRetCode, sizeof(Rsp)) << Rsp;
			pActor->SendData(ob.TakeOsb());

			//在选择战斗中的，关闭等待页面
			if ( m_vecUser[i].m_Status == enReadyStatus_Combat )
			{
				this->NoticeWaitTip(pActor, false);
			}
		}

		this->ClearDuoBaoData();

		m_bCombatOver = true;

		return true;	
	}

	return false;
}

//玩家退出或者离队或者选择战斗时，检测是否开始战斗
bool	Pair::Check_Combat()
{
	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		if ( m_vecUser[i].m_Status == enReadyStatus_No )
		{
			return false;
		}
	}

	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		if ( m_vecEenmy[i].m_Status == enReadyStatus_No )
		{
			return false;
		}
	}

	//开始战斗
	m_bCombatOverDelete = false;
	this->Combat();	

	return true;
}

//清除玩家的夺宝数据
void	Pair::ClearDuoBaoData()
{
	for ( int i = 0; i < m_vecEenmy.size(); ++i )
	{
		ActorPairInfo & Info = m_vecEenmy[i];

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

		if ( 0 == pDouFaPart )
			continue;

		pDouFaPart->ClearDuoBaoData();
	}

	for ( int i = 0; i < m_vecUser.size(); ++i )
	{
		ActorPairInfo & Info = m_vecUser[i];

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Info.m_uidActor);

		if ( 0 == pActor )
			continue;

		IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

		if ( 0 == pDouFaPart )
			continue;

		pDouFaPart->ClearDuoBaoData();
	}
}

//通知弹出或者关闭等待框
void	Pair::NoticeWaitTip(IActor * pActor, bool bOpen)
{
	//通知弹出等待框
	SC_WaitSelect Rsp;

	Rsp.m_bOpen = bOpen;

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_SC_WaitSelect, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}
