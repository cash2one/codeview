#include "DuoBaoGroup.h"
#include "ITeamPart.h"
#include "DuoBaoWar.h"
#include "IBasicService.h"
#include "IDouFaPart.h"


DuoBaoLevelGroup::DuoBaoLevelGroup(INT32 MinLevel, INT32 MaxLevel, DuoBaoGroup * pDuoBaoGroup)
{
	m_MinLevel = MinLevel;
	m_MaxLevel = MaxLevel;
	m_pDuoBaoGroup = pDuoBaoGroup;
}

//玩家的夺宝等级是否属于该组
bool	DuoBaoLevelGroup::IsInGroup(INT32 DuoBaoLv)
{
	if ( DuoBaoLv >= m_MinLevel && (DuoBaoLv <= m_MaxLevel || m_MaxLevel == -1) )
		return true;

	return false;
}

//开始配对
void	DuoBaoLevelGroup::BeginPair(PairGroup * pGroup)
{	
	//首尾式的配对
	int begin = 0;
	int end	  = m_vecWait.size() - 1;

	for ( int i = 0; i < m_vecWait.size() / 2 && begin < end; ++i, ++begin, --end )
	{
		m_pDuoBaoGroup->PairOK(m_vecWait[begin], m_vecWait[end], pGroup);
	}

	if ( begin != end ){
		m_vecWait.clear();
		return;
	}

	//相等表示有一个没配对
	UID uidLeft = m_vecWait[begin];

	//增加此玩家的匹配次数
	m_pDuoBaoGroup->AddUserPairNum(uidLeft);

	m_vecWait.clear();

	m_vecWait.push_back(uidLeft);
}

//取出最后没配对成功的玩家
UID		DuoBaoLevelGroup::Pop_NoPairOK()
{
	if ( m_vecWait.size() > 0 )
		return m_vecWait[0];

	return UID();
}

//清空等待配对表
void	DuoBaoLevelGroup::ClearWaitPair()
{
	m_vecWait.clear();
}

//加入配对
void	DuoBaoLevelGroup::PushToPair(UID uidUser)
{
	m_vecWait.push_back(uidUser);

	//测试用的日志
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 != pActor )
	{
		TRACE("夺宝： 玩家%s加入夺宝配对！！夺宝等级最小：%d, 夺宝等级最大：%d",  pActor->GetName(), m_MinLevel, m_MaxLevel);
	}
}

//取消配对
bool	DuoBaoLevelGroup::CancelPair(IActor * pActor)
{
	std::vector<UID>::iterator iter = m_vecWait.begin();

	UID uidUser = pActor->GetUID();

	for ( ; iter != m_vecWait.end(); ++iter )
	{
		if ( *iter == uidUser ){
			m_vecWait.erase(iter);
			TRACE("玩家%s成功退出夺宝！", pActor->GetName());
			return true;
		}
	}

	return false;
}

/**********************************************************************************************/



//开始匹配
DuoBaoGroup::DuoBaoGroup(UINT8 MinLevel,UINT8 MaxLevel, bool bTeamGroup, DuoBaoWar * pDuoBaoWar)
{
	m_MinLevel = MinLevel;
	m_MaxLevel = MaxLevel;

	m_bTeamGroup = bTeamGroup;

	m_pDuoBaoWar = pDuoBaoWar;
}

bool	DuoBaoGroup::Create()
{
	//创建各等级夺宝组
	const std::vector<SDuoBaoCnfg> * pAllDuoBaoCnfg = g_pGameServer->GetConfigServer()->GetAllDuoBaoCnfg();

	if ( 0 == pAllDuoBaoCnfg )
		return false;

	for ( int i = 0; i < (*pAllDuoBaoCnfg).size(); ++i )
	{
		const SDuoBaoCnfg & DuoBaoCnfg = (*pAllDuoBaoCnfg)[i];

		DuoBaoLevelGroup LvGroup(DuoBaoCnfg.m_MinDuoBaoLevel, DuoBaoCnfg.m_MaxDuoBaoLevel, this);

		m_vecDuoBaoLvGroup.push_back(LvGroup);
	}

	return true;
}

void	DuoBaoGroup::BeginPair()
{
	PairGroup * pGroup = m_pDuoBaoWar->GetPairGroup();

	//对各夺宝等级组进行配对
	for ( int i = 0; i < m_vecDuoBaoLvGroup.size(); ++i )
	{
		DuoBaoLevelGroup & DBlevelGroup = m_vecDuoBaoLvGroup[i];

		DBlevelGroup.BeginPair(pGroup);
	}

	//进行跨级配对
	this->PairOtherLevel(pGroup);

	//开始准备倒计时
	pGroup->PairEnd();
}

//进行跨级配对
void	DuoBaoGroup::PairOtherLevel(PairGroup * pGroup)
{
	int lastindex = -1;

	//得到配对几次后可跨级配对
	UINT8 PairNumCanOtherLevel = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_PairNumCanOtherLevel;

	for ( int i = 0; i < m_vecDuoBaoLvGroup.size(); ++i )
	{
		DuoBaoLevelGroup & DBlevelGroup = m_vecDuoBaoLvGroup[i];


		UID uidUser = DBlevelGroup.Pop_NoPairOK();

		if ( !uidUser.IsValid() )
			continue;

		std::map<UID, UINT8>::iterator iter = m_mapPairNum.find(uidUser);

		if ( iter == m_mapPairNum.end() )
			continue;

		UINT8 PairNum = iter->second;

		if ( PairNum >= PairNumCanOtherLevel ){
			//可跨级配对
			if ( lastindex != -1 ){
				//配对
				DuoBaoLevelGroup & DBGroup = m_vecDuoBaoLvGroup[lastindex];

				this->PairOK(DBGroup.Pop_NoPairOK(), uidUser, pGroup);

				DBGroup.ClearWaitPair();

				DBlevelGroup.ClearWaitPair();
			} else {
				lastindex = i;
			}
		}
	}
}

//此两玩家配对
void	DuoBaoGroup::PairOK(UID uidUser, UID uidEnemy, PairGroup * pGroup)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);
	IActor * pEnemy = g_pGameServer->GetGameWorld()->FindActor(uidEnemy);

	if ( 0 == pActor || 0 == pEnemy )
		return;

	Pair * pPair = new Pair();

	if ( !pPair->Create(uidUser, uidEnemy, m_bTeamGroup, m_pDuoBaoWar) )
	{
		TRACE("<error> %s ; %d Line 配对失败！！", __FUNCTION__, __LINE__);
		return;
	}

	pGroup->Push_Pair(pActor, pEnemy, pPair, m_bTeamGroup);

	TRACE("夺宝：玩家%s和玩家%s配对成功！！！", pActor->GetName(), pEnemy->GetName());

	//删除配对记录
	std::map<UID, UINT8>::iterator iter = m_mapPairNum.find(uidUser);

	if ( iter != m_mapPairNum.end() )
		m_mapPairNum.erase(iter);

	iter = m_mapPairNum.find(uidEnemy);

	if ( iter != m_mapPairNum.end() )
		m_mapPairNum.erase(iter);
}

//增加玩家的匹配次数
void	DuoBaoGroup::AddUserPairNum(UID uidUser)
{
	std::map<UID, UINT8>::iterator iter = m_mapPairNum.find(uidUser);

	if ( iter == m_mapPairNum.end() )
	{
		UINT8 Num = 1;
		m_mapPairNum[uidUser] = Num;
	}
	else
	{
		++(iter->second);
	}	
}

//加入配对
UINT8	DuoBaoGroup::JoinDuoBao(IActor * pActor, INT8 & DuoBaoLvIndex)
{
	INT32 DuoBaoLv = pActor->GetCrtProp(enCrtProp_DuoBaoLevel);

	if ( m_bTeamGroup )
	{
		//组队夺宝时，以夺宝等级大为准
		ITeamPart * pTeamPart = pActor->GetTeamPart();

		if ( 0 == pTeamPart )
			return enDouFaRetCode_ErrJoinDuoBao;
		
		IActor * pMember = pTeamPart->GetTeamMember();

		if ( 0 == pMember )
			return enDouFaRetCode_ErrJoinDuoBao;

		if ( pMember->GetCrtProp(enCrtProp_DuoBaoLevel) > DuoBaoLv )
		{
			DuoBaoLv = pMember->GetCrtProp(enCrtProp_DuoBaoLevel);
		}
	}

	for ( int i = 0; i < m_vecDuoBaoLvGroup.size(); ++i )
	{
		DuoBaoLevelGroup & DBLvGroup = m_vecDuoBaoLvGroup[i];

		if ( !DBLvGroup.IsInGroup(DuoBaoLv) )
			continue;

		DBLvGroup.PushToPair(pActor->GetUID());

		DuoBaoLvIndex = i;
		
		return enDouFaRetCode_Ok;
	}

	return enDouFaRetCode_ErrJoinDuoBao;
}

//取消配对
void	DuoBaoGroup::CancelPair(UID uidUser)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 == pActor )
		return;

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	INT8 DuoBaoLvIndex = pDouFaPart->GetDuoBaoLvIndex();

	if ( DuoBaoLvIndex < 0 || DuoBaoLvIndex >= m_vecDuoBaoLvGroup.size() )
	{
		TRACE("<error> %s : %d Line 玩家的夺宝等级下标错误！！下标:%d", __FUNCTION__, __LINE__, DuoBaoLvIndex);
		return;
	}

	DuoBaoLevelGroup & DBLvGroup = m_vecDuoBaoLvGroup[DuoBaoLvIndex];

	DBLvGroup.CancelPair(pActor);

	std::map<UID, UINT8>::iterator iter = m_mapPairNum.find(uidUser);
	
	if ( iter != m_mapPairNum.end() )
	{
		m_mapPairNum.erase(iter);
	}
}


//配对组

bool	PairGroup::Create(DuoBaoWar * pDuoBaoWar, TGroupID GroupID)
{
	m_GroupID = GroupID;

	m_pDuoBaoWar = pDuoBaoWar;

	return true;
}

void	PairGroup::ClearData()
{
	m_vecPair.clear();

	m_mapPair.clear();
}

void	PairGroup::Push_Pair(IActor * pActor, IActor * pEnemy, Pair * pPair, bool bTeam)
{
	m_vecPair.push_back(pPair);

	m_mapPair[pActor->GetUID()] = pPair;

	m_mapPair[pEnemy->GetUID()] = pPair;

	IDouFaPart * pDouFaPart = pActor->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	pDouFaPart->SetGroupID(m_GroupID);

	pDouFaPart = pEnemy->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return;

	pDouFaPart->SetGroupID(m_GroupID);

	if ( bTeam )
	{
		ITeamPart * pTeamPart = pActor->GetTeamPart();

		if ( 0 != pTeamPart )
		{
			IActor * pMember = pTeamPart->GetTeamMember();

			if ( 0 != pMember )
				m_mapPair[pMember->GetUID()] = pPair;

			pDouFaPart = pMember->GetDouFaPart();

			if ( 0 == pDouFaPart )
				return;

			pDouFaPart->SetGroupID(m_GroupID);
		}

		pTeamPart = pEnemy->GetTeamPart();

		if ( 0 != pTeamPart )
		{
			IActor * pMember = pTeamPart->GetTeamMember();

			if ( 0 != pMember )
				m_mapPair[pMember->GetUID()] = pPair;

			pDouFaPart = pMember->GetDouFaPart();

			if ( 0 == pDouFaPart )
				return;

			pDouFaPart->SetGroupID(m_GroupID);
		}
	}
}

void PairGroup::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enPairGroup_Ready:
		{
			//进入战斗
			for ( int i = 0; i < m_vecPair.size(); ++i )
			{
				Pair * pPair = m_vecPair[i];

				if ( 0 == pPair )
					continue;

				if ( pPair->GetCombatIsOver() )
				{
					//战斗已经结束,删除
					pPair->Release();	
				}
				else if ( !pPair->GetIsCombatOverDelete() )
				{
					//战斗已经开始但还没结束，重新设成战斗结束删除
					pPair->SetIsCombatOverDelete(true);
				}
				else if ( !pPair->Check_CanCombat() )
				{
					//不符合进行战斗条件，删除
					pPair->Release();
				}
				else
				{
					//开始战斗
					pPair->Combat();
				}
			}

			//接下来，Pair的生命期交给自己
			g_pGameServer->GetTimeAxis()->KillTimer(enPairGroup_Ready,this);

			//使命结束，放入空间配对组池,供下次使用
			m_pDuoBaoWar->InValid_PairGroup(m_GroupID);
		}
		break;
	}
}

TGroupID PairGroup::GetGroupID()
{
	return m_GroupID;
}

//配对结束
void	PairGroup::PairEnd()
{
	if ( m_vecPair.size() <= 0 )
	{
		//配对组中没有配对，放入空闲配对组中
		m_pDuoBaoWar->InValid_PairGroup(m_GroupID);
		return;
	}

	for ( int i = 0; i < m_vecPair.size(); ++i )
	{
		Pair * pPair = m_vecPair[i];

		if ( 0 == pPair )
			continue;

		//发送倒计时和选择战斗框
		pPair->SendReadyCombat();
	}

	//开始战斗倒计时
	UINT32 TimerSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_DuoBaoReadyTimeNum;

	g_pGameServer->GetTimeAxis()->SetTimer(enPairGroup_Ready,this,TimerSpace * 1000,"PairGroup::PairEnd");
}

//选择退出战斗
void	PairGroup::QuitCombat(IActor * pActor)
{
	std::map<UID, Pair *>::iterator iter = m_mapPair.find(pActor->GetUID());

	if ( iter == m_mapPair.end() )
		return;

	Pair * pPair = iter->second;

	if ( 0 == pPair )
		return;

	pPair->QuitCombat(pActor);

	//检测是否结束
	if ( !pPair->Check_Over() )
	{
		//没结束则检测剩下的是否可进入战斗
		pPair->Check_Combat();
	}

	TRACE("夺宝：玩家%s选择退出夺宝战斗！！", pActor->GetName());
}

//选择战斗
void	PairGroup::SelectCombat(IActor * pActor)
{
	std::map<UID, Pair *>::iterator iter = m_mapPair.find(pActor->GetUID());

	if ( iter == m_mapPair.end() )
		return;

	Pair * pPair = iter->second;

	if ( 0 == pPair )
		return;

	pPair->SelectCombat(pActor);

	TRACE("夺宝：玩家%s选择进入夺宝战斗！！", pActor->GetName());
}
