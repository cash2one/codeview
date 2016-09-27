
#include "IActor.h"

#include "SyndicateMember.h"
#include "string.h"
#include "DBProtocol.h"
#include "RelationServer.h"
#include "IDBProxyClient.h"
#include "XDateTime.h"

#include "TBuffer.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IGameScene.h"
#include "IMonster.h"
#include "ICombatPart.h"
#include "time.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "DMsgSubAction.h"
#include "ISession.h"

SyndicateMember::SyndicateMember()
{
	memset(&m_SyndicateMemberInfo,0,sizeof(m_SyndicateMemberInfo));
	m_SyndicateMemberInfo.m_uidUser = UID();

	m_SceneID		= INVALID_SCENE_ID;
	m_CombatSceneID = INVALID_SCENE_ID;
	m_nCurMode		= 0;
}
SyndicateMember::SyndicateMember(const SyndicateMemberInfo & MemberInfo)
{
	strncpy(m_SyndicateMemberInfo.m_szUserName, MemberInfo.m_szUserName, sizeof(m_SyndicateMemberInfo.m_szUserName));
	m_SyndicateMemberInfo.m_uidUser  = MemberInfo.m_uidUser;
	m_SyndicateMemberInfo.m_SynID	 = MemberInfo.m_SynID;
	m_SyndicateMemberInfo.m_Position = MemberInfo.m_Position;
	m_SyndicateMemberInfo.m_Contribution = MemberInfo.m_Contribution;
	m_SyndicateMemberInfo.m_Userlevel	 = MemberInfo.m_Userlevel;
	m_SyndicateMemberInfo.m_VipLevel	 = MemberInfo.m_VipLevel;

	m_SyndicateMemberInfo.m_CombatAbility = MemberInfo.m_CombatAbility;
	m_SyndicateMemberInfo.m_ActorFacade   = MemberInfo.m_ActorFacade;
	m_SyndicateMemberInfo.m_Score		  = MemberInfo.m_Score;
	m_SyndicateMemberInfo.m_SynWarLv	  = MemberInfo.m_SynWarLv;
	m_SyndicateMemberInfo.m_LastOnlineTime = MemberInfo.m_LastOnlineTime;

	m_SceneID		= INVALID_SCENE_ID;
	m_nCurMode		= 0;
}

SyndicateMember::~SyndicateMember()
{
}

//获取帮派成员信息
const SyndicateMemberInfo & SyndicateMember::GetSynMemberInfo()
{
	return m_SyndicateMemberInfo;
}

//获取所在帮派的ID
TSynID		SyndicateMember::GetSynID() const
{
	return m_SyndicateMemberInfo.m_SynID;
}

//获取玩家的UID
UID			SyndicateMember::GetSynMemberUID() const
{
	return m_SyndicateMemberInfo.m_uidUser;
}

//获取玩家的名字
const char * SyndicateMember::GetSynMemberName() const
{
	return m_SyndicateMemberInfo.m_szUserName;
}

//获取玩家的职位
enumSynPosition SyndicateMember::GetPosition() const
{
	return m_SyndicateMemberInfo.m_Position;
}

//设置玩家的职位
void	SyndicateMember::SetPosition(enumSynPosition Position,bool bUpdate)
{
	m_SyndicateMemberInfo.m_Position = Position;

	if( bUpdate){
		this->UpdateSynMemberData();
	}
}

//获取玩家的帮派贡献
INT32	SyndicateMember::GetContribution() const
{
	return m_SyndicateMemberInfo.m_Contribution;
}

//增加或减少玩家的帮派贡献
void	SyndicateMember::AddContribution(INT32 Contribution)
{
	IActor * pActor = GetActor();
	if( 0 == pActor){
		return;
	}

	if( Contribution < 0 && m_SyndicateMemberInfo.m_Contribution < -Contribution){
		m_SyndicateMemberInfo.m_Contribution = 0;
	}else{
		m_SyndicateMemberInfo.m_Contribution += Contribution;
	}

	SC_UpdateContribution UpdateContribution;
	UpdateContribution.m_UserUID = pActor->GetUID();
	UpdateContribution.m_Contribution = m_SyndicateMemberInfo.m_Contribution;

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_UpdateContribution, sizeof(UpdateContribution)) << UpdateContribution; 
	pActor->SendData(ob.TakeOsb());	
}

//得到玩家的等级
UINT8	SyndicateMember::GetLevel()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_SyndicateMemberInfo.m_uidUser);

	if ( 0 != pActor )
	{
		//更新下
		m_SyndicateMemberInfo.m_Userlevel = pActor->GetCrtProp(enCrtProp_Level);
	}

	return m_SyndicateMemberInfo.m_Userlevel;
}

//保存玩家帮派成员数据，在玩家退出游戏时调用
void	SyndicateMember::UpdateSynMemberData()
{
	SDB_Update_SyndicateMember_Req Req;

	Req.Uid_Member	 = m_SyndicateMemberInfo.m_uidUser.m_uid;
	Req.Contribution = m_SyndicateMemberInfo.m_Contribution;
	Req.Position	 = m_SyndicateMemberInfo.m_Position;
	Req.CombatAbility= m_SyndicateMemberInfo.m_CombatAbility;
	Req.Score		 = m_SyndicateMemberInfo.m_Score;

	IActor * pActor = this->GetActor();

	if ( 0 != pActor && pActor->GetIsSelfOnline() )
	{
		m_SyndicateMemberInfo.m_LastOnlineTime = CURRENT_TIME();
		Req.LastOnlineTime = m_SyndicateMemberInfo.m_LastOnlineTime;
	}
	else
	{
		Req.LastOnlineTime = m_SyndicateMemberInfo.m_LastOnlineTime;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_UpdateSynMemberInfo, ob.TakeOsb(), 0, 0);
}

IActor *  SyndicateMember::GetActor()
{
	return g_pGameServer->GetGameWorld()->FindActor(m_SyndicateMemberInfo.m_uidUser);

}

//得到vip等级
UINT8			SyndicateMember::GetVipLevel()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_SyndicateMemberInfo.m_uidUser);

	if ( 0 != pActor )
	{
		//更新下
		m_SyndicateMemberInfo.m_VipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
	}

	return m_SyndicateMemberInfo.m_VipLevel;
}

//得到战斗力
INT32			SyndicateMember::GetCombatAbility()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_SyndicateMemberInfo.m_uidUser);

	if ( 0 != pActor )
	{
		//更新下
		m_SyndicateMemberInfo.m_CombatAbility = pActor->GetCrtProp(enCrtProp_ActorCombatAbility);
	}

	return m_SyndicateMemberInfo.m_CombatAbility;
}

//和玩家同步下数据
void			SyndicateMember::SycUserData(INT32 CombatAbility, UINT8 Level, UINT8 VipLevel, INT32 SynWarLv,INT32 ActorFacade)
{
	m_SyndicateMemberInfo.m_CombatAbility = CombatAbility;

	m_SyndicateMemberInfo.m_Userlevel = Level;

	m_SyndicateMemberInfo.m_VipLevel  = VipLevel;

	m_SyndicateMemberInfo.m_SynWarLv  = SynWarLv;

	m_SyndicateMemberInfo.m_ActorFacade = ActorFacade;
}

//获得外观
UINT16			SyndicateMember::GetFacade()
{
	return m_SyndicateMemberInfo.m_ActorFacade;
}

//获得帮派功勋
INT32			SyndicateMember::GetSynMemberScore()
{
	return m_SyndicateMemberInfo.m_Score;
}


//获得帮战等级
INT32			SyndicateMember::GetSynWarLv()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_SyndicateMemberInfo.m_uidUser);

	if ( 0 != pActor )
	{
		//更新下
		m_SyndicateMemberInfo.m_SynWarLv = pActor->GetCrtProp(enCrtProp_SynCombatLevel);
	}

	return m_SyndicateMemberInfo.m_SynWarLv;
}

//增加帮派功勋
void			SyndicateMember::AddSynScore(INT32 ScoreNum)
{
	m_SyndicateMemberInfo.m_Score += ScoreNum;

	IActor * pActor = this->GetActor();

	if ( 0 != pActor && ScoreNum > 0 )
	{
		SS_GetSynWarScore ScoreEvent;

		ScoreEvent.GetNum = ScoreNum;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_GetSynWarScore);
		pActor->OnEvent(msgID,&ScoreEvent,sizeof(ScoreEvent));
	}
}

//得到最后在线时间
UINT32			SyndicateMember::GetLastOnlineTime()
{
	return m_SyndicateMemberInfo.m_LastOnlineTime;
}
