#include "IActor.h"
#include "TeamPart.h"
#include "ThingServer.h"
#include "IFriendPart.h"
#include "TeamCmd.h"
#include "ICombatPart.h"
#include "IFuBenPart.h"
#include "IBasicService.h"


TeamPart::TeamPart()
{
	m_pActor = 0;

	m_bTeamLeader = false;

	m_bInFastTeam = false;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool TeamPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if (0 == pMaster)
		return false;

	m_pActor = (IActor *)pMaster;

	return true;
}

//释放
void TeamPart::Release(void)
{

}

//取得部件ID
enThingPart TeamPart::GetPartID(void)
{
	return enThingPart_Actor_Team;
}

//取得本身生物
IThing*		TeamPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool TeamPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void TeamPart::InitPrivateClient()
{
	this->__QuitTeam();
	this->__QuitWait();
}


//玩家下线了，需要关闭该ThingPart
void TeamPart::Close()
{
}

//保存数据
void TeamPart::SaveData()
{

}


//当前是否有组队
bool	TeamPart::IsHaveTeam()
{
	return (this->GetTeamMember() != NULL);
}

//得到队友
IActor *TeamPart::GetTeamMember()
{
	if (!m_TeamMember.IsValid())
		return 0;

	IActor * pMember = g_pGameServer->GetGameWorld()->FindActor(m_TeamMember);

	if (0 == pMember)
		m_TeamMember = UID();

	return pMember;
}

//得到队长
IActor *TeamPart::GetTeamLeader()
{
	if ( !m_TeamMember.IsValid() )
		return 0;

	if ( !m_bTeamLeader )
		return this->GetTeamMember();

	return m_pActor;
}

//邀请组队
bool	TeamPart::InviteTeam(UID uidUser, enCreateTeamType type)
{
	SC_InviteTeam Rsp;
	OBuffer1k ob;

	if ( enCreateTeamType_Point == type ){
		//只有好友才可以发起组队
		IFriendPart * pFriendPart = m_pActor->GetFriendPart();

		if ( 0 == pFriendPart ){
			return false;
		}

		if ( !pFriendPart->IsFriend(uidUser) ){
			Rsp.m_RetCode = enTeamRetCode_NotFriend;
		}
	}
	
	if ( Rsp.m_RetCode == enTeamRetCode_Ok ){
		//检测是否可以组队
		Rsp.m_RetCode = (enTeamRetCode)this->__CheckCreateTeam(uidUser);
	}

	if ( Rsp.m_RetCode == enTeamRetCode_Ok ){
		
		//邀请对方组队
		IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidUser);

		if ( 0 == pFriend ){
			
			return false;
		}

		SC_Invited	Rsp;

		Rsp.m_uidUser = m_pActor->GetUID();
		strncpy(Rsp.m_Name, m_pActor->GetName(), sizeof(Rsp.m_Name));

		ob << TeamHeader(enTeamCmd_SC_Invited, sizeof(Rsp)) << Rsp;
		pFriend->SendData(ob.TakeOsb());
	}

	ob.Reset();
	ob << TeamHeader(enTeamCmd_InviteTeam, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	return true;
}

//同意组队
void	TeamPart::AgreeTeam(UID uidUser)
{
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 == pFriend ){
		
		return;
	}

	SC_AgreeTeam Rsp;

	Rsp.m_RetCode = (enTeamRetCode)this->CreateTeam(uidUser, false);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_Ok_Invite, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//拒绝组队
void	TeamPart::RefuseTeam(UID uidUser)
{
	SC_RefuseTeam Rsp;

	Rsp.m_RetCode = enTeamRetCode_Ok;

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_No_Invite, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	//向邀请者发送拒绝邀请消息
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 == pFriend ){
		return;
	}

	Rsp.m_RetCode = enTeamRetCode_RefuseTeam;

	ob.Reset();
	ob << TeamHeader(enTeamCmd_No_Invite, sizeof(Rsp)) << Rsp;
	pFriend->SendData(ob.TakeOsb());
}

void	TeamPart::SetTeamData(UID uidUser, bool bLeader)
{
	m_TeamMember = uidUser;

	m_bTeamLeader = bLeader;
}

//退出队伍
bool	TeamPart::QuitTeam()
{
	SC_QuitTeam Rsp;

	do
	{
		if ( !m_TeamMember.IsValid() )
		{
			Rsp.m_RetCode = enTeamRetCode_NoTeam;
			break;
		}

		enActorDoing ActorDoing = m_pActor->GetActorDoing();

		if ( ActorDoing == enActorDoing_Combat )
		{
			Rsp.m_RetCode = enTeamRetCode_InCombat;
			break;		
		}

		this->__QuitTeam();
	}while(0);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_QuitTeam, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	return true;
}

//创建队伍
UINT8	TeamPart::CreateTeam(UID uidUser, bool bTeamLeader)
{
	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 == pFriend ){
		
		return enTeamRetCode_OffLine;
	}

	if ( pFriend == m_pActor ){

		return enTeamRetCode_ErrTeamSelf;
	}

	SC_CreateTeam Rsp;

	enTeamRetCode RetCode = (enTeamRetCode)this->__CheckCreateTeam(uidUser);

	if ( RetCode != enTeamRetCode_Ok ){
		
		return RetCode;
	}

	if ( bTeamLeader ){
	
		Rsp.m_uidLeader = m_pActor->GetUID();
	} else {
		Rsp.m_uidLeader = uidUser;
	}
		
	OBuffer1k ob2;
	//通知对方客户端创建自已
	pFriend->NoticClientCreatePrivateThing(m_pActor->GetUID());

	ob2 << m_pActor->GetUID();
	++Rsp.m_Num;

	for ( int i = 0; i < MAX_EMPLOY_NUM; ++i )
	{
		IActor * pEmployee = m_pActor->GetEmployee(i);

		if ( 0 == pEmployee ){
			continue;
		}

		pFriend->NoticClientCreatePrivateThing(pEmployee->GetUID());

		ob2 << pEmployee->GetUID();
		++Rsp.m_Num;
	}

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SC_CreateTeam, sizeof(Rsp) + ob2.Size()) << Rsp << ob2;
	pFriend->SendData(ob.TakeOsb());

	//通知客户端创建对方
	ob.Reset();
	ob2.Reset();
	Rsp.m_Num = 0;

	m_pActor->NoticClientCreatePrivateThing(pFriend->GetUID());

	ob2 << pFriend->GetUID();
	++Rsp.m_Num;

	for ( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = pFriend->GetEmployee(i);

		if ( 0 == pEmployee ){
			continue;
		}

		m_pActor->NoticClientCreatePrivateThing(pEmployee->GetUID());

		ob2 << pEmployee->GetUID();
		++Rsp.m_Num;			
	}

	ob << TeamHeader(enTeamCmd_SC_CreateTeam, sizeof(Rsp) + ob2.Size()) << Rsp << ob2;
	m_pActor->SendData(ob.TakeOsb());

	//创建队伍
	this->__CreateTeam(pFriend, bTeamLeader);

	//如果有在快速组队等待的话，退出等待
	g_pThingServer->GetTeamMgr().QuitWait(m_pActor);
	g_pThingServer->GetTeamMgr().QuitWait(pFriend);
	
	return enTeamRetCode_Ok;
}

void	TeamPart::__QuitTeam()
{

	if (!m_TeamMember.IsValid())
		return;

	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(m_TeamMember);

	if ( 0 == pFriend ){
		return;
	}

	ITeamPart * pTeamPart = pFriend->GetTeamPart();

	if (0 != pTeamPart)
		pTeamPart->ClearTeamData(false, true);

	this->ClearTeamData(true, false);
}

//检测是否能组队
UINT8	TeamPart::__CheckCreateTeam(UID uidUser)
{
	if ( m_TeamMember.IsValid() ){
		return enTeamRetCode_MeExist;
	}

	IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(uidUser);

	if ( 0 == pFriend ){
		return enTeamRetCode_OffLine;
	}

	enActorDoing enDoing = pFriend->GetActorDoing();
	if ( enDoing != enActorDoing_None && enDoing != enActorDoing_WaitTeam ){
		return enTeamRetCode_InBusy;
	}

	ITeamPart * pTeamPart = pFriend->GetTeamPart();

	if ( 0 == pTeamPart ){
		return false;
	}

	if ( pTeamPart->IsHaveTeam() ){
		return enTeamRetCode_OtExist;
	}

	return enTeamRetCode_Ok;
}

void	TeamPart::ClearTeamData(bool bMeQuit, bool bSendMsg)
{
	//发布离队事件
	SS_QuitTeam QuitTeam;

	QuitTeam.m_bMeQuit = bMeQuit;

	QuitTeam.m_uidTeamMember = m_TeamMember;

	IActor * pLeader = this->GetTeamLeader();

	if ( 0 == pLeader )
		return;

	QuitTeam.m_uidTeamLeader = pLeader->GetUID();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitTeam);
	m_pActor->OnEvent(msgID,&QuitTeam,sizeof(QuitTeam));

	//清除数据
	m_TeamMember = UID();

	m_bTeamLeader = false;

	if ( bSendMsg )
	{
		SC_QuitTeam Rsp;

		Rsp.m_RetCode = enTeamRetCode_MemQuitTeam;

		OBuffer1k ob;
		ob << TeamHeader(enTeamCmd_QuitTeam, sizeof(Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());	
	}
}

//掉线，下线要做的一些事
void  TeamPart::LeaveDoSomeThing()
{
	this->__QuitTeam();
	this->__QuitWait();
}

//设置是否在快速组队
void	TeamPart::SetIsInFastTeam(bool bInFastTeam)
{
	m_bInFastTeam = bInFastTeam;
}

//得到是否在快速组队
bool	TeamPart::GetIsInFastTeam()
{
	return m_bInFastTeam;
}

//创建队伍
void			TeamPart::__CreateTeam(IActor * pFriend, bool bLeader)
{
	ITeamPart * pTeamPart = pFriend->GetTeamPart();

	if ( 0 == pTeamPart ){

		return;
	}

	m_TeamMember = pFriend->GetUID();

	m_bTeamLeader = bLeader;

	pTeamPart->SetTeamData(m_pActor->GetUID(), !bLeader);

	if ( bLeader ){
		
		this->__SetDefaultTeamLineup(m_pActor->GetUID(), pFriend->GetUID());
	} else {
		
		this->__SetDefaultTeamLineup(pFriend->GetUID(), m_pActor->GetUID());
	}
}

//取消等待
void			TeamPart::__QuitWait()
{
	g_pThingServer->GetTeamMgr().QuitWait(m_pActor);
}

//设置默认队伍阵形
bool			TeamPart::__SetDefaultTeamLineup(UID uidLeader, UID uidMember)
{
	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	if ( 0 == pCombatPart ){
		
		return false;
	}

	if ( MAX_LINEUP_POS_NUM < 2 ){
		
		return false;
	}

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( ServerParam.m_TeamMemberLineupPos.size() < MAX_TEAMCOMBAT_NUM
		|| ServerParam.m_TeamLeaderLineupPos.size() < MAX_TEAMCOMBAT_NUM )
	{
		TRACE("<error> %s ; %d Line 组队阵形配置参数个数错误！！个数等于%d和%d", __FUNCTION__, __LINE__, ServerParam.m_TeamLeaderLineupPos.size(), ServerParam.m_TeamMemberLineupPos.size());
		return false;
	}

	int pos = 0;

	//先设置队长角色在阵形中的位置
	CS_SetTeamLineup Lineup;

	Lineup.m_uidLineup[ServerParam.m_TeamLeaderLineupPos[pos]] = uidLeader;

	++pos;

	IActor * pLeader = g_pGameServer->GetGameWorld()->FindActor(uidLeader);

	if ( 0 == pLeader )
		return false;

	for ( int i = 0; i < MAX_EMPLOY_NUM && pos < MAX_TEAMCOMBAT_NUM; ++i )
	{
		IActor * pEmployee = pLeader->GetEmployee(i);

		if ( 0 == pEmployee )
			continue;

		ICombatPart * pEmCombatPart = pEmployee->GetCombatPart();

		if ( 0 == pEmCombatPart )
			continue;

		if ( pEmCombatPart->GetIsJoinTeamCombat() ){
			Lineup.m_uidLineup[ServerParam.m_TeamLeaderLineupPos[pos]] = pEmployee->GetUID();
			++pos;
		}
	}

	//设置队员角色在阵形中的位置
	pos = 0;

	Lineup.m_uidLineup[ServerParam.m_TeamMemberLineupPos[pos]] = uidMember;

	++pos;

	IActor * pMember = g_pGameServer->GetGameWorld()->FindActor(uidMember);

	if ( 0 == pMember )
		return false;

	for ( int i = 0; i < MAX_EMPLOY_NUM && pos < MAX_TEAMCOMBAT_NUM; ++i )
	{
		IActor * pEmployee = pMember->GetEmployee(i);

		if ( 0 == pEmployee )
			continue;

		ICombatPart * pEmCombatPart = pEmployee->GetCombatPart();

		if ( 0 == pEmCombatPart )
			continue;

		if ( pEmCombatPart->GetIsJoinTeamCombat() ){
			Lineup.m_uidLineup[ServerParam.m_TeamMemberLineupPos[pos]] = pEmployee->GetUID();
			++pos;
		}
	}

	if ( enTeamRetCode_Ok != pCombatPart->SetTeamLineup(Lineup) ){
		
		return false;
	}

	IActor * pOther = pLeader;

	if ( pLeader == m_pActor )
		pOther = pMember;

	ICombatPart * pOtCombatPart =  pOther->GetCombatPart();

	if ( 0 == pOtCombatPart )
		return false;

	if ( enTeamRetCode_Ok != pOtCombatPart->SetTeamLineup(Lineup) )
		return false;

	return true;
}
