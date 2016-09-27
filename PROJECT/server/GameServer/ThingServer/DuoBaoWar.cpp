#include "DuoBaoWar.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "XDateTime.h"
#include "IBasicService.h"
#include "ITeamPart.h"
#include "IDouFaPart.h"
#include "DuoBaoGroup.h"


DuoBaoWar::DuoBaoWar()
{
	m_bOpen = false;
}

DuoBaoWar::~DuoBaoWar()
{
	for ( int i = 0; i < m_vecSingleGroup.size(); ++i )
	{
		DuoBaoGroup * pDuoBaoGroup = m_vecSingleGroup[i];

		if ( 0 == pDuoBaoGroup )
			continue;

		delete pDuoBaoGroup;
	}

	for ( int i = 0; i < m_vecTeamGroup.size(); ++i )
	{
		DuoBaoGroup * pDuoBaoGroup = m_vecTeamGroup[i];

		if ( 0 == pDuoBaoGroup )
			continue;

		delete pDuoBaoGroup;
	}

	for ( int i = 0; i < m_vecInValidPairGroup.size(); ++i )
	{
		PairGroup * pPairGroup = m_vecInValidPairGroup[i];

		if ( 0 == pPairGroup )
			continue;

		delete pPairGroup;
	}
}

bool DuoBaoWar::Create()
{
	const SGameServerConfigParam & ServerConfig = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;
	
	//创建各等级组
	for ( int i = 0; i + 1 < ServerConfig.m_DuoBaoLevelGroup.size(); i += 2 )
	{
		DuoBaoGroup *pSingleGroup = new DuoBaoGroup(ServerConfig.m_DuoBaoLevelGroup[i], ServerConfig.m_DuoBaoLevelGroup[i + 1], false, this);

		if ( !pSingleGroup->Create() )
		{
			TRACE("<error> %s : %d Line 玩家等级组创建失败！！"__FUNCTION__, __LINE__);
			return false;
		}

		DuoBaoGroup *pTeamGroup = new DuoBaoGroup(ServerConfig.m_DuoBaoLevelGroup[i], ServerConfig.m_DuoBaoLevelGroup[i + 1], true, this);

		if ( !pTeamGroup->Create() )
		{
			TRACE("<error> %s : %d Line 玩家等级组创建失败！！"__FUNCTION__, __LINE__);
			return false;
		}

		m_vecSingleGroup.push_back(pSingleGroup);

		m_vecTeamGroup.push_back(pTeamGroup);
	}

	//检测时间，开启夺宝或者设置开启定时器
	time_t CurTime = CURRENT_TIME();

	tm *pTm = ::localtime(&CurTime);

	if ( 0 == pTm )
	{
		TRACE("<error> %s : %d Line 夺宝时间获取失败！！", __FUNCTION__, __LINE__);
		return false;
	}

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_DuoBaoTime.size() < 3 )
	{
		TRACE("<error> %s : %d Line 夺宝开启时间参数个数错误！！个数%d", __FUNCTION__, __LINE__, GameParam.m_DuoBaoTime.size());
		return false;
	}

	UINT32 OpenHour = GameParam.m_DuoBaoTime[0];

	UINT32 OpenMinute = GameParam.m_DuoBaoTime[1];

	UINT32 TimeNum = GameParam.m_DuoBaoTime[2];

	UINT32 TmpNum = 0;

	if ( pTm->tm_hour > OpenHour || ( pTm->tm_hour == OpenHour && pTm->tm_min >= OpenMinute ) )
	{
		TmpNum = (pTm->tm_hour - OpenHour) * 3600 + (pTm->tm_min - OpenMinute) * 60 + pTm->tm_sec;
	}
	else
	{
		TmpNum = (24 - OpenHour) * 3600 + (0 - OpenMinute) * 60 + pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec;
	}

	if ( TmpNum >= TimeNum )
	{
		//没开启,设置定时器
		UINT32 TimerLong = 24 * 3600 - TmpNum;

		g_pGameServer->GetTimeAxis()->SetTimer(enDuoBaoWarTimer_Begin,this,TimerLong * 1000,"DuoBaoWar::Create");
	}
	else
	{
		//开启
		this->StartDuoBao();
	}

	return true;
}

void DuoBaoWar::OnTimer(UINT32 timerID)
{
	if ( enDuoBaoWarTimer_Begin == timerID )
	{
		//开启夺宝
		this->StartDuoBao();

		g_pGameServer->GetTimeAxis()->KillTimer(enDuoBaoWarTimer_Begin,this);
	}
	else if ( enDuoBaoWarTimer_Over == timerID )
	{
		//结束夺宝
		this->OverDuoBao();

		g_pGameServer->GetTimeAxis()->KillTimer(enDuoBaoWarTimer_Over,this);
	}
	else if ( enDuoBaoWarTimer_Pair == timerID )
	{
		//开始配对
		for ( int i = 0; i < m_vecSingleGroup.size(); ++i )
		{
			//单人夺宝
			DuoBaoGroup * pGroup = m_vecSingleGroup[i];

			if ( 0 == pGroup )
				continue;

			pGroup->BeginPair();
		}

		for ( int i = 0; i < m_vecTeamGroup.size(); ++i )
		{
			//组队夺宝
			DuoBaoGroup * pGroup = m_vecTeamGroup[i];

			if ( 0 == pGroup )
				continue;

			pGroup->BeginPair();
		}
	}
}

//得到玩家所属的等级组下标，-1表示没有
int	 DuoBaoWar::GetGroupUserLvIndex(UINT8 userLevel)
{
	const SGameServerConfigParam & ServerConfig = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	for ( int i = 0; i + 1 < ServerConfig.m_DuoBaoLevelGroup.size(); i += 2 )
	{
		if ( userLevel >= ServerConfig.m_DuoBaoLevelGroup[i] && (userLevel <= ServerConfig.m_DuoBaoLevelGroup[i + 1] || -1 == ServerConfig.m_DuoBaoLevelGroup[i + 1]) )
		{
			return i / 2;
		}
	}

	return -1;
}

//加入夺宝
void DuoBaoWar::JoinDuoBao(IActor * pActor)
{
	SC_JoinDuoBao Rsp;

	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart )
		return;

	IActor * pMember = pTeamPart->GetTeamMember();

	do
	{
		if ( !m_bOpen )
		{
			Rsp.m_Result = enDouFaRetCode_NotOpenDuoBao;
			break;
		}

		IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

		if ( 0 == pDouFaPart )
			return;

		if ( pDouFaPart->IsInDuoBao() )
		{
			//已经在夺宝中，不能再次加入
			Rsp.m_Result = enDouFaRetCode_ErrInDuoBao;
			break;
		}

		if ( pActor->IsInFastWaitTeam() )
		{
			//等待组队中，无法操作
			Rsp.m_Result = enDouFaRetCode_ErrWaitTeam;
			break;			
		}

		if ( pDouFaPart->IsInRunTime() )
		{
			//正受到逃跑惩罚，不能加入夺宝
			Rsp.m_Result = enDouFaRetCode_ErrRun;
			break;
		}

		int index = this->GetGroupUserLvIndex(pActor->GetCrtProp(enCrtProp_Level));

		if ( -1 == index )
		{
			//等级不足
			Rsp.m_Result = enDouFaRetCode_ErrLevel;
			break;
		}

		IDouFaPart * pMemberDouFaPart = 0;

		INT8 DuoBaoLvIndex = INVALID_INDEX;

		if ( 0 == pMember )
		{
			//单人夺宝
			if ( index >= m_vecSingleGroup.size() )
			{
				TRACE("<error> %s : %d Line 获取玩家的夺宝组下标越界！！index=%d,m_vecSingleGroup.size()=%d", __FUNCTION__, __LINE__, index, m_vecSingleGroup.size());
				return;
			}

			Rsp.m_Result = (enDouFaRetCode)m_vecSingleGroup[index]->JoinDuoBao(pActor, DuoBaoLvIndex);
		}
		else
		{
			//组队夺宝
			int index2 = this->GetGroupUserLvIndex(pMember->GetCrtProp(enCrtProp_Level));

			if ( -1 == index2 )
			{
				//队友等级不足
				Rsp.m_Result = enDouFaRetCode_ErrMemberLevel;
				break;			
			}

			if ( pTeamPart->GetTeamLeader() != pActor )
			{
				//只有队长能选择加入
				Rsp.m_Result = enDouFaRetCode_ErrNotLeader;
				break;
			}

			pMemberDouFaPart = pMember->GetDouFaPart();

			if ( 0 == pMemberDouFaPart )
				return;

			if ( pMemberDouFaPart->IsInDuoBao() )
			{
				//队友已经在夺宝中，不能再次加入
				Rsp.m_Result = enDouFaRetCode_ErrMeberInDuoBao;
				break;
			}

			if ( pMemberDouFaPart->IsInRunTime() )
			{
				//队友正受到逃跑惩罚，不能加入夺宝
				Rsp.m_Result = enDouFaRetCode_ErrMemRun;
				break;			
			}

			if ( index < index2 )
			{
				index = index2;
			}

			//组队夺宝
			if ( index >= m_vecTeamGroup.size() )
			{
				TRACE("<error> %s : %d Line 获取玩家的夺宝组下标越界！！index=%d,m_vecTeamGroup.size()=%d", __FUNCTION__, __LINE__, index,m_vecTeamGroup.size());
				return;
			}

			Rsp.m_Result = (enDouFaRetCode)m_vecTeamGroup[index]->JoinDuoBao(pActor, DuoBaoLvIndex);
		}

		if ( INVALID_INDEX == DuoBaoLvIndex )
		{
			TRACE("<error> %s : %d 参加夺宝失败，玩家的夺宝等级组没有！！", __FUNCTION__, __LINE__);
			return;
		}

		if ( enDouFaRetCode_Ok == Rsp.m_Result )
		{
			pDouFaPart->SetUserLvIndex(index);
			pDouFaPart->SetDuoBaoLvIndex(DuoBaoLvIndex);

			if ( 0 != pMemberDouFaPart )
			{
				pMemberDouFaPart->SetUserLvIndex(index);
				pMemberDouFaPart->SetIsTeamDuoBao(true);
				pMemberDouFaPart->SetDuoBaoLvIndex(DuoBaoLvIndex);
				pDouFaPart->SetIsTeamDuoBao(true);
			}
			else
			{
				pDouFaPart->SetIsTeamDuoBao(false);
			}
		}

	}while(0);

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_JoinDuoBao, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	if ( pMember != 0 && Rsp.m_Result == enDouFaRetCode_Ok )
	{
		ob.Reset();
		ob <<  DouFaHeader(enDouFaCmd_JoinDuoBao, sizeof(Rsp)) << Rsp;
		pMember->SendData(ob.TakeOsb());
	}
}

//点击战斗准备框按钮
void DuoBaoWar::ReadySelect(IActor *pActor, bool bEnterCombat)
{
	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	TGroupID groupID= pDouFaPart->GetGroupID();

	if ( INVALID_GROUPID == groupID )
	{
		TRACE("<error> %s ; %d Line %s的组ID无郊！！组ID = %d", __FUNCTION__, __LINE__, pActor->GetName(), groupID);
		return;
	}

	std::map<TGroupID, PairGroup *>::iterator iter = m_mapPairGroup.find(groupID);

	if ( iter == m_mapPairGroup.end() )
	{
		TRACE("<error> %s ; %d Line 没找到该配对组！！组ID = %d", __FUNCTION__, __LINE__, groupID);
		return;
	}

	PairGroup * pGroup = iter->second;

	if ( 0 == pGroup )
	{
		TRACE("<error> %s : %d Line 配对组指针为空！！组ID:%d", __FUNCTION__, __LINE__, pGroup);
		return;
	}

	if ( bEnterCombat )
	{
		pGroup->SelectCombat(pActor);
	}
	else
	{
		pGroup->QuitCombat(pActor);
	}

	SC_ReadySelect Rsp;

	OBuffer1k ob;
	ob <<  DouFaHeader(enDouFaCmd_ReadySelect, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//退出或掉线或顶号或离队
void DuoBaoWar::QuitDuoBao(IActor * pQuiter)
{
	if ( !m_bOpen )
		//未开放
		return;

	IDouFaPart * pDouFaPart = pQuiter->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	TGroupID groupID= pDouFaPart->GetGroupID();

	int index = pDouFaPart->GetUserLvIndex();

	if ( INVALID_GROUPID != groupID )
	{
		//准备战斗期间，设成退出状态
		std::map<TGroupID, PairGroup *>::iterator iter = m_mapPairGroup.find(groupID);

		if ( iter == m_mapPairGroup.end() )
		{
			//已经在战斗中了
			pDouFaPart->ClearDuoBaoData();
		}
		else
		{
			PairGroup * pGroup = iter->second;

			pGroup->QuitCombat(pQuiter);		
		}
	}
	else if (  INVALID_INDEX != index  )
	{
		//等待配对期，直接整队退出配对
		if ( index >= m_vecSingleGroup.size() || index >= m_vecSingleGroup.size() )
		{
			TRACE("<error> %s : %d Line 玩家夺宝组索引有错！！索引%d", __FUNCTION__, __LINE__, index);
			return;
		}

		SC_CancelDuoBao Rsp;

		OBuffer1k ob;

		if ( !pDouFaPart->IsTeamDuoBao() )
		{
			DuoBaoGroup * pDuoBaoGroup = m_vecSingleGroup[index];

			if ( 0 != pDuoBaoGroup )
			{
				pDuoBaoGroup->CancelPair(pQuiter->GetUID());
			}
		}
		else
		{
			ITeamPart * pTeamPart = pQuiter->GetTeamPart();

			if ( 0 == pTeamPart )
			{
				return;
			}

			IActor * pLeader = pTeamPart->GetTeamLeader();

			DuoBaoGroup * pDuoBaoGroup = 0;

			if ( 0 != pLeader )
			{
				pDuoBaoGroup = m_vecTeamGroup[index];

				if ( 0 != pDuoBaoGroup )
				{
					pDuoBaoGroup->CancelPair(pLeader->GetUID());
				}

				IActor * pMember = pTeamPart->GetTeamMember();

				if ( 0 != pMember )
				{
					IDouFaPart * pMemberDouFaPart = pMember->GetDouFaPart();

					if ( 0 != pMemberDouFaPart )
					{
						pMemberDouFaPart->ClearDuoBaoData();

						//通知队友取消
						ob <<  DouFaHeader(enDouFaCmd_CancelDuoBao, sizeof(Rsp)) << Rsp;
						pMember->SendData(ob.TakeOsb());
					}
				}
			}
		}

		ob.Reset();
		ob <<  DouFaHeader(enDouFaCmd_CancelDuoBao, sizeof(Rsp)) << Rsp;
		pQuiter->SendData(ob.TakeOsb());

		pDouFaPart->ClearDuoBaoData();
	}
	else
	{
		SC_CancelDuoBao Rsp;

		OBuffer1k ob;
		ob <<  DouFaHeader(enDouFaCmd_CancelDuoBao, sizeof(Rsp)) << Rsp;
		pQuiter->SendData(ob.TakeOsb());
	}
}

//创建一个配对成功组
PairGroup * DuoBaoWar::GetPairGroup()
{
	static TGroupID id = 0;

	if ( m_vecInValidPairGroup.size() <= 0 )
	{
		//若没有空闲的配对组，则新建
		PairGroup *pGroup = new PairGroup;

		++id;

		if ( !pGroup->Create(this, id) )
		{
			TRACE("<error> %s : %d Line 创建配对组失败！！id = %d", __FUNCTION__, __LINE__, id);
			return 0;
		}

		TRACE("创建配对组成功！！id = %d", id);

		m_mapPairGroup[pGroup->GetGroupID()] = pGroup;

		return pGroup;
	}

	PairGroup *pGroup = m_vecInValidPairGroup[m_vecInValidPairGroup.size() - 1];

	m_vecInValidPairGroup.pop_back();

	if ( 0 == pGroup )
		return 0;

	m_mapPairGroup[pGroup->GetGroupID()] = pGroup;

	return pGroup;
}

//配对组的使命结束，放入空间配对组池,供下次使用
void	DuoBaoWar::InValid_PairGroup(TGroupID GroupID)
{
	std::map<TGroupID, PairGroup*>::iterator iter = m_mapPairGroup.find(GroupID);

	if ( iter == m_mapPairGroup.end() )
	{
		return;
	}

	PairGroup* pGroup = iter->second;

	pGroup->ClearData();

	m_mapPairGroup.erase(iter);

	//放入空闲配对组池
	m_vecInValidPairGroup.push_back(pGroup);
}

//是否开启
bool	DuoBaoWar::IsOpen()
{
	return m_bOpen;
}

//设置结束定时器
void DuoBaoWar::SetOverTimer()
{
	time_t CurTime = CURRENT_TIME();

	tm *pTm = ::localtime(&CurTime);

	if ( 0 == pTm )
	{
		TRACE("<error> %s : %d Line 夺宝时间获取失败！！", __FUNCTION__, __LINE__);
		return;
	}

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( GameParam.m_DuoBaoTime.size() < 3 )
	{
		TRACE("<error> %s : %d Line 夺宝开启时间参数个数错误！！个数%d", __FUNCTION__, __LINE__, GameParam.m_DuoBaoTime.size());
		return;
	}

	UINT32 OpenHour = GameParam.m_DuoBaoTime[0];

	UINT32 OpenMinute = GameParam.m_DuoBaoTime[1];

	UINT32 TimeNum = GameParam.m_DuoBaoTime[2];

	//得到已经开启多久了
	UINT32 StartLong = 0;
	
	if ( pTm->tm_hour > OpenHour || ( pTm->tm_hour == OpenHour && pTm->tm_min >= OpenMinute ) )
	{
		StartLong = (pTm->tm_hour - OpenHour) * 3600 + (pTm->tm_min - OpenMinute) * 60 + pTm->tm_sec;
	}
	else
	{
		StartLong = (24 - OpenHour) * 3600 + (0 - OpenMinute) * 60 + pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec;
	}

	UINT32 TimerLong = 0;

	if ( TimeNum < StartLong )
	{
		TRACE("<error> %s : %d Line 夺宝时间错误！！", __FUNCTION__, __LINE__);
		TimerLong = 1;
	}
	else
	{
		TimerLong = TimeNum - StartLong;

		g_pGameServer->GetTimeAxis()->SetTimer(enDuoBaoWarTimer_Over,this,TimerLong * 1000,"DuoBaoWar::SetOverTimer");
	}
}

//开启夺宝
void DuoBaoWar::StartDuoBao()
{
	m_bOpen = true;

	//设置结束定时器
	this->SetOverTimer();

	//开启配对定时器
	UINT32 PairSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_DuoBaoPairSpace;
	
	g_pGameServer->GetTimeAxis()->SetTimer(enDuoBaoWarTimer_Pair,this,PairSpace * 1000,"DuoBaoWar::StartDuoBao");

	//广播
	INT32 LangID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_NoticeDuoBaoStart;

	IGameWorld * pGameWorld = g_pGameServer->GetGameWorld();

	if ( 0 != pGameWorld )
	{
		pGameWorld->WorldSystemMsg(pGameWorld->GetLanguageStr(LangID));
	}
	//g_pGameServer->GetGameWorld()->WorldSystemMsg(g_pGameServer->GetGameWorld()->GetLanguageStr(LangID));
}

//结束夺宝
void DuoBaoWar::OverDuoBao()
{
	m_bOpen = false;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//设置下次开启定时器
	UINT32 TimerLong = 24 * 3600 - GameParam.m_DuoBaoTime[2];

	g_pGameServer->GetTimeAxis()->SetTimer(enDuoBaoWarTimer_Begin,this,TimerLong * 1000,"DuoBaoWar::OverDuoBao");

	//关闭配对定时器
	g_pGameServer->GetTimeAxis()->KillTimer(enDuoBaoWarTimer_Pair, this);

	//广播
	g_pGameServer->GetGameWorld()->WorldSystemMsg(g_pGameServer->GetGameWorld()->GetLanguageStr(12106));
}
