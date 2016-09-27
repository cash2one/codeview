
#include "IActor.h"
#include "Syndicate.h"
#include "string.h"
#include "RelationServer.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "DMsgSubAction.h"
#include <algorithm>
#include "SyndicateMember.h"
#include "ISystemMsg.h"
#include "RandomService.h"
#include "XDateTime.h"
#include "ISession.h"

Syndicate::Syndicate()
{
}

Syndicate::Syndicate(const SyndicateInfo & SynInfo)
{
	strncpy(m_SyndicateInfo.m_szLeaderName, SynInfo.m_szLeaderName, sizeof(m_SyndicateInfo.m_szLeaderName));
	strncpy(m_SyndicateInfo.m_szSynName, SynInfo.m_szSynName, sizeof(m_SyndicateInfo.m_szSynName));
	m_SyndicateInfo.m_Exp		= SynInfo.m_Exp;
	m_SyndicateInfo.m_Level		= SynInfo.m_Level;
	m_SyndicateInfo.m_MemberNum = SynInfo.m_MemberNum;
	m_SyndicateInfo.m_SynID		= SynInfo.m_SynID;
	m_SyndicateInfo.m_uidLeader = SynInfo.m_uidLeader;
	m_SyndicateInfo.m_MaxMemberNum = SynInfo.m_MaxMemberNum;
	m_SyndicateInfo.m_SynWarScore = SynInfo.m_SynWarScore;
	m_SyndicateInfo.m_SynWarAbility = SynInfo.m_SynWarAbility;
	strncpy(m_SyndicateInfo.m_szSynMsg, SynInfo.m_szSynMsg, sizeof(m_SyndicateInfo.m_szSynMsg));
	m_SyndicateInfo.m_bWin		= SynInfo.m_bWin;
	strncpy(m_SyndicateInfo.m_PreEnemySynName,SynInfo.m_PreEnemySynName,sizeof(m_SyndicateInfo.m_PreEnemySynName));
	m_SyndicateInfo.m_SynWarTotalScore = SynInfo.m_SynWarTotalScore;

	const SSyndicateCnfg * pSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(m_SyndicateInfo.m_Level + 1);

	if( 0 == pSynCnfg){
		m_SyndicateInfo.m_NeedExp = 0;
	}else{
		m_SyndicateInfo.m_NeedExp = pSynCnfg->m_UpLevelNeedExp;
	}
}

Syndicate::~Syndicate()
{
	MAPSYNMEMBER::iterator itMem = m_SynMember.begin();

	for( ; itMem != m_SynMember.end(); ++itMem)
	{
		ISyndicateMember * pSynMember = itMem->second;
		if( 0 == pSynMember){
			continue;
		}

		delete pSynMember;
	}

	m_SynMember.clear();
}

//得到玩家获得的福利值,百分比的数值直接返回数值，比如%8返回8
UINT8		Syndicate::GetWelfareValue(enWelfare Welfare)
{
	if( enWelfare_Max <= Welfare){
		return 0;
	}

	const SSynWelfareCnfg * pSynWelfare = g_pGameServer->GetConfigServer()->GetSynWelfareCnfg(Welfare, this->GetSynLevel());
	if( 0 == pSynWelfare){
		return 0;
	}

	return pSynWelfare->m_AddValue;
}

//得到帮派的信息
const SyndicateInfo & Syndicate::GetSyndicateInfo()
{
	return m_SyndicateInfo;
}
//得到帮派ID
TSynID		Syndicate::GetSynID() const
{
	if ( INVALID_SYN_ID == m_SyndicateInfo.m_SynID )
	{
		TRACE("<error> %s : %d Line 帮派ID为无效帮派ID,帮派名字%s",__FUNCTION__,__LINE__,m_SyndicateInfo.m_szSynName);
	}
	return m_SyndicateInfo.m_SynID;
}

//得到帮主UID
const UID	Syndicate::GetLeaderUID() const
{
	return UID(m_SyndicateInfo.m_uidLeader);
}

//设置帮主UID
void Syndicate::SetLeaderUID(UINT64 LeaderUID, bool bUpdate)
{
	m_SyndicateInfo.m_uidLeader = LeaderUID;

	if(bUpdate){
		this->UpdateSyndicate();
	}
}

//等到帮主名字
const char * Syndicate::GetLeaderName() const
{
	return m_SyndicateInfo.m_szLeaderName;
}

//设置帮主名字
void		Syndicate::SetLeaderName(const char * szLeaderName, bool bUpdate)
{
	strncpy(m_SyndicateInfo.m_szLeaderName, szLeaderName, sizeof(m_SyndicateInfo.m_szLeaderName));

	if( bUpdate){
		this->UpdateSyndicate();
	}
}

//昨到帮派名字
const char * Syndicate::GetSynName() const
{
	return m_SyndicateInfo.m_szSynName;
}

//得到帮派等级
UINT8		Syndicate::GetSynLevel() const
{
	return m_SyndicateInfo.m_Level;
}

//设置帮派等级
bool		Syndicate::SetSynLevel(UINT8 Level, bool bUpdate)
{
	const SSyndicateCnfg * pSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(Level);
	if( 0 == pSynCnfg){
		TRACE("<error> %s : %d 行 获取帮派配置信息出错！！，帮派等级 = %d", __FUNCTION__, __LINE__, Level);
		return false;
	}

	m_SyndicateInfo.m_Level = Level;
	m_SyndicateInfo.m_MaxMemberNum = pSynCnfg->m_MaxMemberNum;

	//下级帮派配置信息
	const SSyndicateCnfg * pNextSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(Level + 1);
	if( 0 == pNextSynCnfg){
		m_SyndicateInfo.m_NeedExp = 0;
	}else{
		m_SyndicateInfo.m_NeedExp = pNextSynCnfg->m_UpLevelNeedExp;
	}
	
	if( true == bUpdate){
		this->UpdateSyndicate();
	}

	return true;
}

//得到帮派经验
INT32		Syndicate::GetSynExp() const
{
	return m_SyndicateInfo.m_Exp;
}

//增加帮派经验值
void		Syndicate::AddSynExp(INT32 Exp, bool bUpdate)
{
	if( 0 == m_SyndicateInfo.m_NeedExp){
		//已达最高级
		return;
	}

	if( (m_SyndicateInfo.m_Exp + Exp) >= m_SyndicateInfo.m_NeedExp){
		//即将要获得的帮派等级配置信息
		const SSyndicateCnfg * pSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(m_SyndicateInfo.m_Level + 1);
		if( 0 == pSynCnfg){
			TRACE("<error> %s : %d 行 获取不到当然的帮派配置信息！！,帮派等级=%d", __FUNCTION__, __LINE__, m_SyndicateInfo.m_Level + 1);
			return;
		}

		m_SyndicateInfo.m_MaxMemberNum = pSynCnfg->m_MaxMemberNum;

		//经验大于帮派升级所需经验时，帮派升级
		++m_SyndicateInfo.m_Level;
		
		//下级帮派配置信息
		const SSyndicateCnfg * pNextSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(m_SyndicateInfo.m_Level + 1);
		if( 0 == pNextSynCnfg){
			//到最高级,经验直接置0
			m_SyndicateInfo.m_Exp = 0;
			m_SyndicateInfo.m_NeedExp = 0;
		}else{
			m_SyndicateInfo.m_NeedExp = pNextSynCnfg->m_UpLevelNeedExp;
			m_SyndicateInfo.m_Exp = m_SyndicateInfo.m_Exp + Exp - pSynCnfg->m_UpLevelNeedExp;
		}
	}else{
		m_SyndicateInfo.m_Exp = m_SyndicateInfo.m_Exp + Exp;
	}
	
	if( bUpdate){
		this->UpdateSyndicate();
	}
}

//得到帮派人数
UINT16		Syndicate::GetSynMemberNum() const
{
	return m_SyndicateInfo.m_MemberNum;
}

//增加或减少帮派从数
void		Syndicate::AddSynMemberNum(INT16 AddMemberNum, bool bUpdate)
{
	m_SyndicateInfo.m_MemberNum += AddMemberNum;

	if( true == bUpdate){
		this->UpdateSyndicate();
	}
}

//得到帮派升级需要经验
UINT32		Syndicate::GetUpLevelNeedExp() const
{
	return m_SyndicateInfo.m_NeedExp;
}

//设置帮派升级需要经验
void		Syndicate::SetUpLevelNeedExp(UINT32 Exp)
{
	m_SyndicateInfo.m_NeedExp = Exp;
}

//得到帮派最多可容纳人数
UINT16		Syndicate::GetMaxMemberNum() const
{
	return m_SyndicateInfo.m_MaxMemberNum;
}

//设置帮派最多可容纳人数
void		Syndicate::SetMaxMemberNum(UINT16 MaxMemberNum)
{
	m_SyndicateInfo.m_MaxMemberNum = MaxMemberNum;
}

//保存帮派的信息
void		Syndicate::UpdateSyndicate()
{
	SDB_Updata_Syndicate_Req Req;

	Req.Exp			= m_SyndicateInfo.m_Exp;
	strncpy(Req.LeaderName, m_SyndicateInfo.m_szLeaderName, sizeof(Req.LeaderName));
	Req.Level		= m_SyndicateInfo.m_Level;
	Req.MemberNum	= m_SyndicateInfo.m_MemberNum;
	Req.SynID		= m_SyndicateInfo.m_SynID;
	strncpy(Req.SynName, m_SyndicateInfo.m_szSynName, sizeof(Req.SynName));
	Req.Uid_Leader	= m_SyndicateInfo.m_uidLeader;
	Req.SynWarScore = m_SyndicateInfo.m_SynWarScore;
	Req.SynWarAbility = m_SyndicateInfo.m_SynWarAbility;
	Req.SynWarTotalScore = m_SyndicateInfo.m_SynWarTotalScore;
	strncpy(Req.szSynMsg, m_SyndicateInfo.m_szSynMsg, sizeof(Req.szSynMsg));

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_UpdateSyndicateInfo, ob.TakeOsb(), 0, 0);
}

//加入帮派
bool		Syndicate::JoinSyndicate(const SyndicateMemberInfo & MemberInfo)
{
	ISyndicateMember * pSynMember = new SyndicateMember(MemberInfo);
	if( 0 == pSynMember){
		return false;
	}

	m_SynMember[MemberInfo.m_uidUser] = pSynMember;

	//帮派人数加1
	this->AddSynMemberNum(1);

	//插入帮派成员数据
	SDB_Insert_SyndicateMember_Req Req;

	Req.SynID		= MemberInfo.m_SynID;
	Req.uid_User	= MemberInfo.m_uidUser.m_uid;
	Req.UserLevel	= MemberInfo.m_Userlevel;
	Req.VipLevel	= MemberInfo.m_VipLevel;
	Req.LastOnlineTime = MemberInfo.m_LastOnlineTime;
	strncpy(Req.UserName, MemberInfo.m_szUserName, sizeof(Req.UserName));

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_InsertSynMemberInfo, ob.TakeOsb(), 0, 0);

	//发布事件
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Req.uid_User));
	if( 0 == pActor){
		return true;
	}

	SS_AddSyn AddSyn;
	AddSyn.m_bCreate = this->GetSynMemberNum() == 1 ? true : false;
	AddSyn.m_SynID   = this->GetSynID();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddSyn);
	pActor->OnEvent(msgID,&AddSyn,sizeof(AddSyn));

	return true;
}

//获得帮派成员
ISyndicateMember * Syndicate::GetSynMember(UID uidUser)
{
	MAPSYNMEMBER::iterator iter = m_SynMember.find(uidUser);
	if( iter == m_SynMember.end()){
		return 0;
	}

	return iter->second;
}

//解散帮派
bool	Syndicate::JieSanSyndicate()
{
	ISystemMsg * pSysMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
	if( 0 == pSysMsg){
		return false;
	}

	//数据库删除帮派
	SDB_Delete_Syndicate_Req DeleteSyn_Req;
	DeleteSyn_Req.SynID = this->GetSynID();

	OBuffer1k ob;
	ob << DeleteSyn_Req;
	g_pGameServer->GetDBProxyClient()->Request(DeleteSyn_Req.SynID, enDBCmd_DeleteSyn, ob.TakeOsb(), 0, 0);

	//删除帮派成员
	MAPSYNMEMBER::iterator itMem = m_SynMember.begin();

	for( ; itMem != m_SynMember.end(); ++itMem)
	{
		ISyndicateMember * pSynMember = itMem->second;
		if( 0 == pSynMember){
			continue;
		}

		UID uidUser = pSynMember->GetSynMemberUID();

		bool bLeader = (pSynMember->GetPosition() == enumSynPosition_Leader);

		delete pSynMember;

		if( bLeader){
			continue;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);
		if( 0 == pActor){
			continue;
		}

		//解散消息，让还在帮派界面操作的关闭界面
		SC_CloseSynWindow Rsp;
		Rsp.m_CloseType = enCloseSynWindow_JieSan;

		ob.Reset();
		ob << SyndicateHeader(enSyndicateCmd_SC_CloseSynWindow, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());

		//给帮众发公告，告诉解散帮派
		SInsertMsg Msg;

		// fly add	20121106
	
		strncpy(Msg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10018), sizeof(Msg.szMsgBody));
		//strncpy(Msg.szMsgBody, "您的帮派已被解散！！！！", sizeof(Msg.szMsgBody));

		pSysMsg->ViewMsg(pActor, enMsgType_Insert, &Msg);

		//发布退帮事件
		SS_QuitSyn QuitSyn;
		QuitSyn.uidUser = pActor->GetUID().ToUint64();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitSyn);
		pActor->OnEvent(msgID,&QuitSyn,sizeof(QuitSyn));
	}

	m_SynMember.clear();

	return true;
}

//离开帮派
bool	Syndicate::RemoveSyndicate(UID uidUser)
{
	MAPSYNMEMBER::iterator iter = m_SynMember.find(uidUser);
	if( iter == m_SynMember.end()){
		return false;
	}

	ISyndicateMember * pSynMember = iter->second;
	if( 0 == pSynMember){
		return false;
	}

	this->AddSynMemberNum(-1);

	m_SynMember.erase(iter);

	delete pSynMember;
	pSynMember = 0;

	//数据库删除
	SDB_Delete_SyndicateMember_Req Req;
	Req.SynID = this->GetSynID();
	Req.uid_QuitUser = uidUser.ToUint64();

	OBuffer1k ob;
	ob << Req;

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);
	if( 0 == pActor){
		g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_DeleteSynMemberInfo, ob.TakeOsb(), 0, 0);
		return true;	
	}

	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteSynMemberInfo, ob.TakeOsb(), 0, 0);

	//如果玩家在线，则发布退帮事件
	SS_QuitSyn QuitSyn;
	QuitSyn.uidUser = uidUser.ToUint64();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_QuitSyn);
	pActor->OnEvent(msgID,&QuitSyn,sizeof(QuitSyn));

	return true;
}

//遍历帮派成员
void	Syndicate::VisitAllSynMember(IVisitSynMember & visit)
{
	MAPSYNMEMBER::iterator itMem = m_SynMember.begin();
	
	for( ; itMem != m_SynMember.end(); ++itMem)
	{
		ISyndicateMember * pSynMember = itMem->second;

		UID Uid_Actor = pSynMember->GetSynMemberUID();

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Uid_Actor);
		if( 0 == pActor){
			continue;
		}

		visit.VisitMember(pActor);
	}
}

//对帮派成员进行排序
bool	sortsynmember(const ISyndicateMember * pSynMember1, const ISyndicateMember * pSynMember2)
{
	//先按职位排
//	return pSynMember1->GetPosition() < pSynMember2->GetPosition();
	INT32 MemberPosition1 = pSynMember1->GetPosition();
	INT32 MemberPosition2 = pSynMember2->GetPosition();
	if( MemberPosition1< MemberPosition2){
		return true;
	}
	if(MemberPosition1 > MemberPosition2){
		return false;
	}

	//再按帮派贡献排
	INT32 MemberContribution1 = pSynMember1->GetContribution();
	INT32 MemberContribution2 = pSynMember2->GetContribution();
	
	return MemberContribution2 < MemberContribution1;
}

//查看帮派成员列表
void	Syndicate::ViewSynMemberList(IActor * pActor)
{
	std::vector<ISyndicateMember *>  vectSynMember;

	MAPSYNMEMBER::iterator iter = m_SynMember.begin();

	for(; iter != m_SynMember.end(); ++iter)
	{
		vectSynMember.push_back(iter->second);
	}
	
	//排名
	std::sort(vectSynMember.begin(), vectSynMember.end(), sortsynmember);

	SC_ViewListNum_Rsp RspNum;
	
	RspNum.m_SynNum = vectSynMember.size();

	UINT32 CurTime = CURRENT_TIME();

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_ViewSynMemberList, sizeof(RspNum) + RspNum.m_SynNum * sizeof(SC_ViewSynMemberList_Rsp)) << RspNum;

	for( int i = 0; i < vectSynMember.size(); ++i)
	{
		SC_ViewSynMemberList_Rsp Rsp;

		ISyndicateMember * pSynMember = vectSynMember[i];

		if ( 0 == pSynMember )
			continue;

		Rsp.m_uidSynMember = pSynMember->GetSynMemberUID();
		Rsp.m_Contribution = pSynMember->GetContribution();
		Rsp.m_Position	   = pSynMember->GetPosition();
		Rsp.m_Level		   = pSynMember->GetLevel();
		Rsp.m_vipLevel	   = pSynMember->GetVipLevel();
		Rsp.m_SynWarLv	   = pSynMember->GetSynWarLv();
		Rsp.m_SynScore	   = pSynMember->GetSynMemberScore();
		Rsp.m_bIsValid	   = true;

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pSynMember->GetSynMemberUID());

		if ( 0 == pActor || !pActor->GetSession()->IsOnLine() )
		{
			INT32 LastOnlineTime = pSynMember->GetLastOnlineTime();

			if ( LastOnlineTime == 0 )
			{
				Rsp.m_bIsValid = false;
				Rsp.m_LastOnLineTime = 0;
			}
			else
			{
				Rsp.m_LastOnLineTime  = CurTime - LastOnlineTime;
			}
		}
		else
		{
			Rsp.m_LastOnLineTime  = 0;
		}

		strncpy(Rsp.m_szSynMemberName, pSynMember->GetSynMemberName(), sizeof(Rsp.m_szSynMemberName));		

		ob << Rsp;
	}	

	pActor->SendData(ob.TakeOsb());	
}


//加入到帮派成员集合中
void	Syndicate::AddToMemberList(ISyndicateMember * pSynMember)
{
	m_SynMember[pSynMember->GetSynMemberUID()] = pSynMember;
}

//是否能参加帮战
UINT8	Syndicate::Check_JoinSynCombat()
{
	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	if ( m_SynMember.size() < ServerParam.m_JoinSynCombatUserNumLow ){
		return enSynRetCode_NoMember;
	}

	//可参战人数
	int Num = 0;

	MAPSYNMEMBER::iterator iter = m_SynMember.begin();

	for ( ; iter != m_SynMember.end(); ++iter )
	{
		ISyndicateMember * pSynMember = iter->second;

		if ( 0 == pSynMember )
			continue;

		if ( pSynMember->GetLevel() >= ServerParam.m_JoinSynCombatUserNumLow )
			++Num;

		if ( Num >= ServerParam.m_JoinSynCombatUserNumLow )
			break;
	}

	if ( 0 == Num )
		return enSynRetCode_ErrLowJoinLeve;

	if ( Num < ServerParam.m_JoinSynCombatUserNumLow )
		return enSynRetCode_NoMember;

	return enSynRetCode_OK;
}

//得到帮派成员的随机分布位置
void	Syndicate::RandomSynMemberPos(std::vector<UID> & vecMember)
{
	MAPSYNMEMBER::iterator iter = m_SynMember.begin();

	for ( ; iter != m_SynMember.end(); ++iter )
	{
		vecMember.push_back(iter->first);	
	}

	int count = vecMember.size();

	//实现随机分布位置
	for ( int i = 0; i < count / 2; ++i )
	{
		int RandomPos = RandomService::GetRandom() % count;

		if ( i == count )
			continue;

		//进行互换位置
		UID uidTmp = vecMember[i];

		vecMember[i] = vecMember[RandomPos];

		vecMember[RandomPos] = uidTmp;
	}
}

//得到帮派积分
INT32	Syndicate::GetSynWarScore() const
{
	return m_SyndicateInfo.m_SynWarScore;
}

//设置帮派积分
void	Syndicate::SetSynWarScore(INT32 SynWarScore)
{
	m_SyndicateInfo.m_SynWarScore = SynWarScore;

	this->UpdateSyndicate();
}

//得到帮战总积分
INT32	Syndicate::GetSynWarTotalScore() const
{
	return m_SyndicateInfo.m_SynWarTotalScore;
}

//增加帮战积分
void	Syndicate::AddSynWarScore(INT32 SynWarScore)
{
	m_SyndicateInfo.m_SynWarScore += SynWarScore;
	m_SyndicateInfo.m_SynWarTotalScore += SynWarScore;

	this->UpdateSyndicate();
	
}

//得到帮战实力
INT32	Syndicate::GetSynWarAbility() const
{
	return m_SyndicateInfo.m_SynWarAbility;
}

//设置帮战实力
void	Syndicate::SetSynWarAbility(INT32 SynWarAbility)
{
	m_SyndicateInfo.m_SynWarAbility = SynWarAbility;
}

//得到帮派公告
const char *	Syndicate::GetSynMsg()
{
	return m_SyndicateInfo.m_szSynMsg;
}

//设置帮派公告
void			Syndicate::SetSynMsg(const char * szSynMsg)
{
	strncpy(m_SyndicateInfo.m_szSynMsg, szSynMsg, sizeof(m_SyndicateInfo.m_szSynMsg));
}


//得到上一场帮战敌对帮派名字
const char * Syndicate::GetPreEnemySynName() const
{
	return m_SyndicateInfo.m_PreEnemySynName;
}

//得到上一场帮战是否胜利
enumSynWarbWin Syndicate::GetWinFail() const
{
	return m_SyndicateInfo.m_bWin;
}


//设置上一场帮战数据
void Syndicate::SetPreSynWarData(const char * szSynName,enumSynWarbWin bWin)
{
	m_SyndicateInfo.m_bWin = bWin;
	strncpy(m_SyndicateInfo.m_PreEnemySynName,szSynName,sizeof(m_SyndicateInfo.m_PreEnemySynName));
}

//更新帮派战力
void	Syndicate::UpdateSynCombatAbility()
{
	INT32 SynCombatAbilityParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_SynCombatAbilityParam;

	INT32 TotalCombatAbility = 0;

	MAPSYNMEMBER::iterator iter = m_SynMember.begin();

	for ( ; iter != m_SynMember.end(); ++iter )
	{
		ISyndicateMember * pSynMember = iter->second;

		if ( 0 != pSynMember )
		{
			TotalCombatAbility += pSynMember->GetCombatAbility();
		}
	}

	if ( SynCombatAbilityParam != 0 )
	{
		m_SyndicateInfo.m_SynWarAbility = TotalCombatAbility / SynCombatAbilityParam;
	}
}
