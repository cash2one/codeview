
#include "IActor.h"
#include "SyndicateMgr.h"
#include "IBasicService.h"
#include "string.h"

#include "RelationServer.h"
#include "IConfigServer.h"
#include <vector>
#include <algorithm>
#include "IMailPart.h"
#include <string>
#include "DMsgSubAction.h"
#include "IKeywordFilter.h"
#include "IFuBenPart.h"
#include "XDateTime.h"
#include "ISession.h"
#include "ICDTimerPart.h"

SyndicateMgr::SyndicateMgr()
{
}

SyndicateMgr::~SyndicateMgr()
{
	
}

bool	SyndicateMgr::Create()
{
	LoadSynInfo();

	if( false == m_SyndicateShop.Create(this)){
		return false;
	}

	if( false == m_SynMagic.Create(this)){
		return false;
	}

	if ( false == m_SynWar.Create()){
		return false;
	}

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Syndicate,this);
}

void	SyndicateMgr::Close()
{
	//释放帮派
	std::hash_map<TSynID, ISyndicate *>::iterator itSyn = m_mapSyndicate.begin();

	for(; itSyn != m_mapSyndicate.end(); ++itSyn)
	{
		delete itSyn->second;
	}
}

//收到MSG_ROOT消息
void	SyndicateMgr::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	typedef  void (SyndicateMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enSyndicateCmd_Max] = 
	{
		&SyndicateMgr::CreateSyndicate,
		&SyndicateMgr::ApplyJoinSyn,
		&SyndicateMgr::ViewSynList,
		&SyndicateMgr::ViewSynMemberList,
		&SyndicateMgr::ViewSynApplyList,
		&SyndicateMgr::RenMianUser,
		&SyndicateMgr::InviteUser,
		&SyndicateMgr::AllowUser,
		&SyndicateMgr::NotAllowUser,
		&SyndicateMgr::QuitSyn,
		&SyndicateMgr::ViewSynShop,
		&SyndicateMgr::BuySynGoods,
		&SyndicateMgr::ViewSynMagic,
		&SyndicateMgr::LearnSynMagic,
		&SyndicateMgr::EnterSyn,

		&SyndicateMgr::OpenSynCombat,
		&SyndicateMgr::EnterSynCombat,
		NULL,
		&SyndicateMgr::ZhaoMuSynMember,
		NULL,
		&SyndicateMgr::ViewSynWelfare,
		&SyndicateMgr::ResetSynCombat,
		&SyndicateMgr::ViewSynFuBenDropGoods,
		NULL,
		&SyndicateMgr::SynFuBenAutoCombat,
		&SyndicateMgr::OpenSynWar,
		&SyndicateMgr::OpenDeclareWar,
		&SyndicateMgr::DeclareWar,
		&SyndicateMgr::AcceptDeclareWar,
		&SyndicateMgr::RefuseDeclareWar,
		&SyndicateMgr::EnterSynWar,
		&SyndicateMgr::BuyCombatNum,
		&SyndicateMgr::SynWarCombat,
		&SyndicateMgr::ChangeSynMsg,
		&SyndicateMgr::GetSynWarData,
		&SyndicateMgr::ClickSynWarbWin,

		&SyndicateMgr::GetSynFuBenBossInfo,
	};

	if( nCmd >= enSyndicateCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

// nRetCode: 取值于 enDBRetCode
void	SyndicateMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if( nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_InsertSyndicateInfo:
		{
			HandleCreateSyndicate(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_LoadSyndicateListInfo:
		{
			HandleLoadSynListInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_LoadSyndicateMemberListInfo:
		{
			HandleLoadSynMemberListInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_LoadSyndicateApplyInfo:
		{
			HandleLoadSynApplyListInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetActorUIDByName1:
		{
			HandleInviteUser(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
	   {
		   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	   }
	   break;
	}
}

//得到玩家的帮派ID
TSynID			SyndicateMgr::GetUserSynID(const UID & uid_User)
{
	std::hash_map<UID, TSynID, std::hash<UID>, std::equal_to<UID>>::iterator iter = m_mapSynID.find(uid_User);
	if( iter == m_mapSynID.end()){
		return INVALID_SYN_ID;
	}

	return iter->second;
}

//获得玩家的帮派
ISyndicate *	SyndicateMgr::GetSyndicate(const UID & uid_User)
{
	TSynID SynID = this->GetUserSynID(uid_User);

	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.find(SynID);
	if( iter == m_mapSyndicate.end()){
		return 0;
	}

	return iter->second;
}

//获得玩家的帮派
ISyndicate *	SyndicateMgr::GetSyndicate(TSynID SynID)
{
	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.find(SynID);
	if( iter == m_mapSyndicate.end()){
		return 0;
	}

	return iter->second;
}

//获得玩家个人的帮派信息
ISyndicateMember *	 SyndicateMgr::GetSyndicateMember(const UID & uid_User)
{
	ISyndicate * pSyn = this->GetSyndicate(uid_User);
	if( 0 == pSyn){
		return 0;
	}

	return pSyn->GetSynMember(uid_User);
}

//通过邮件加入帮派
bool	SyndicateMgr::MailJoinSyndicate(IActor * pActor, TSynID SynID, enMailRetCode & MailRetCode)
{
	ISyndicate * pSyndicate = this->GetSyndicate(SynID);
	if( 0 == pSyndicate){
		MailRetCode = enMailRetCode_NoSyn;
		return false;
	}

	ISyndicateMember * pSynMember = pSyndicate->GetSynMember(pActor->GetUID());
	if( 0 != pSynMember){
		//已经有帮派
		MailRetCode = enMailRetCode_ExistSyn;
		return false;
	}
	
	if( pSyndicate->GetSynMemberNum() >= pSyndicate->GetMaxMemberNum()){
		//帮派成员已满
		MailRetCode = enMailRetCode_MemberFull;
		return false;
	}

	SyndicateMemberInfo MemberInfo;

	MemberInfo.m_Position = enumSynPosition_General;
	MemberInfo.m_SynID = SynID;
	strncpy(MemberInfo.m_szUserName, pActor->GetName(), sizeof(MemberInfo.m_szUserName));
	MemberInfo.m_uidUser = pActor->GetUID();
	MemberInfo.m_Userlevel = pActor->GetCrtProp(enCrtProp_Level);

	if( !this->AddToSyndicate(MemberInfo)){
		MailRetCode = enMailRetCode_ErrAddSyn;
		return false;
	}

	MailRetCode = enMailRetCode_OK;
	return true;
}

//加入帮派
bool	SyndicateMgr::AddToSyndicate(const SyndicateMemberInfo & SynMemberInfo)
{
	ISyndicate * pSyndicate = this->GetSyndicate(SynMemberInfo.m_SynID);
	if( 0 == pSyndicate){
		return false;
	}

	if( !pSyndicate->JoinSyndicate(SynMemberInfo)){
		return false;
	}

	m_mapSynID[SynMemberInfo.m_uidUser] = SynMemberInfo.m_SynID;

	return true;
}

//遍历帮派成员
void	SyndicateMgr::VisitAllSynMember(TSynID SynID, IVisitSynMember & visit)
{
	ISyndicate * pSyndicate = this->GetSyndicate(SynID);
	if( 0 == pSyndicate){
		return;
	}

	pSyndicate->VisitAllSynMember(visit);
}

//帮派排序
bool	sortsyn(const ISyndicate * pSyn1, const ISyndicate * pSyn2)
{
	//先按帮派等级排
	INT32 Level1 = pSyn1->GetSynLevel();
	INT32 Level2 = pSyn2->GetSynLevel();
	
	if( Level2 < Level1){
		return true;
	}

	if(Level2 > Level1){
		return false;
	}

	////再按帮派人数排
	
	return  pSyn2->GetSynMemberNum() < pSyn1->GetSynMemberNum();
}


//帮战排序
bool	SortSynWar(const ISyndicate * pSyn1, const ISyndicate * pSyn2)
{
	////按帮战积分排	
	return  pSyn2->GetSynWarTotalScore() < pSyn1->GetSynWarTotalScore();
}

//得到帮派排名榜
void	SyndicateMgr:: GetSynRank(std::vector<ISyndicate *> & vectSynRank)
{
	vectSynRank.clear();

	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();

	for( ; iter != m_mapSyndicate.end(); ++iter)
	{
		ISyndicate * pSyn = iter->second;

		if ( 0 == pSyn )
			continue;

		vectSynRank.push_back(pSyn);
	}

	//对帮派进行排序
   std::sort(vectSynRank.begin(), vectSynRank.end(), sortsyn);
}


//得到帮战排名榜
void	SyndicateMgr:: GetSynWarRank(std::vector<ISyndicate *> & vectSynWarRank)
{
	vectSynWarRank.clear();

	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();

	for( ; iter != m_mapSyndicate.end(); ++iter)
	{
		if( 0 == (iter->second)){
			continue;
		}

		vectSynWarRank.push_back(iter->second);
	}

	//对帮派进行排序
   std::sort(vectSynWarRank.begin(), vectSynWarRank.end(), SortSynWar);
}

//创建帮派
void	SyndicateMgr::CreateSyndicate(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_CreateSyndicate_Req Req;
	ib >>  Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_CreateSyndicate_Rsp RspRetcode;
	OBuffer1k ob;

	Req.m_szCreateSynName[THING_NAME_LEN - 1] = '\0';

	if(false == this->CanCreateSyndicate(pActor, Req.m_szCreateSynName, RspRetcode.m_RetCode)){
		
		ob << SyndicateHeader(enSyndicateCmd_CreateSyn, sizeof(SC_CreateSyndicate_Rsp)) << RspRetcode;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	
	//插入数据库，并返回帮派ID到回调涵数处理
	SDB_Insert_Syndicate_Req DBReq;
	DBReq.uid_Leader = pActor->GetUID().m_uid;
	strncpy(DBReq.szLeaderName, pActor->GetName(), sizeof(DBReq.szLeaderName));
	strncpy(DBReq.szSynName, Req.m_szCreateSynName, sizeof(DBReq.szSynName));

	ob << DBReq;

	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertSyndicateInfo, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

//申请加入帮派
void	SyndicateMgr::ApplyJoinSyn(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_ApplyJoinSyn_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误!!", __FUNCTION__, __LINE__);
		return;
	}

	SC_Syndicate_Rsp Rsp;

	OBuffer1k ob;

	if( this->CheckCanApplyJoinSyn(pActor, Req.m_SynID, Rsp.m_SynRetCode)){

		SDB_Insert_SyndicateApply_Req ReqApply;

		ReqApply.ApplyUserLayer = pActor->GetCrtProp(enCrtProp_ActorLayer);
		ReqApply.ApplyUserLevel = pActor->GetCrtProp(enCrtProp_Level);
		strncpy(ReqApply.ApplyUserName, pActor->GetName(), sizeof(ReqApply.ApplyUserName));
		ReqApply.SynID			= Req.m_SynID;
		ReqApply.Uid_ApplyUser  = pActor->GetUID().m_uid;
		ReqApply.LastOnlineTime = CURRENT_TIME();

		ob << ReqApply;
		g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertSynApplyInfo, ob.TakeOsb(), 0, 0);
		
		SyndicateApply SynApply;

		SynApply.m_ApplyUserLayer = ReqApply.ApplyUserLayer;
		SynApply.m_ApplyUserLevel =	ReqApply.ApplyUserLevel;
		SynApply.m_SynID		  = ReqApply.SynID;
		strncpy(SynApply.m_szApplyUserName, ReqApply.ApplyUserName, sizeof(SynApply.m_szApplyUserName));
		SynApply.m_uidApplyUser   = UID(ReqApply.Uid_ApplyUser);
		SynApply.m_LastOnlineTime = ReqApply.LastOnlineTime;

		this->AddToSynApplyList(SynApply);	//加入到帮派申请者集合中
	}

	ob.Reset();
	ob << SyndicateHeader(enSyndicateCmd_ApplySyn, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}


//查看帮派列表
void	SyndicateMgr::ViewSynList(IActor * pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_ViewList_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	std::vector<ISyndicate *>  vectSyn;
	
	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();
	for( ; iter != m_mapSyndicate.end(); ++iter)
	{
		vectSyn.push_back( iter->second);
	}

	//进行帮派排序，先按等级排再按帮派人数排
	std::sort(vectSyn.begin(), vectSyn.end(), sortsyn);

	SC_ViewListNum_Rsp RspNum;
	
	if(Req.m_RankNumBegin > vectSyn.size()){
		RspNum.m_SynNum = 0;
	}else{
		if(Req.m_RankNumEnd > vectSyn.size()){
			Req.m_RankNumEnd = vectSyn.size();
		}

		RspNum.m_SynNum = Req.m_RankNumEnd - Req.m_RankNumBegin;
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_ViewSynList, sizeof(RspNum) + RspNum.m_SynNum * sizeof(ViewSynList_Rsp)) << RspNum;

	if( RspNum.m_SynNum != 0)
	{
		for(int i = Req.m_RankNumBegin; i < Req.m_RankNumEnd; ++i)
		{
			ViewSynList_Rsp Rsp;
			Rsp.m_SynID = vectSyn[i]->GetSynID();
			Rsp.m_Level = vectSyn[i]->GetSynLevel();
			Rsp.m_MemberNum = vectSyn[i]->GetSynMemberNum();
			Rsp.m_nRank		= i + 1;
			strncpy(Rsp.m_szLeaderName, vectSyn[i]->GetLeaderName(), sizeof(Rsp.m_szLeaderName));
			strncpy(Rsp.m_szSynName, vectSyn[i]->GetSynName(), sizeof(Rsp.m_szSynName));
			
			ob << Rsp;
		}	
	}

	pActor->SendData(ob.TakeOsb());
}

//对帮派成员进行排序
//bool	sortsynmember(const ISyndicateMember * pSynMember1, const ISyndicateMember * pSynMember2)
//{
//	//先按职位排
//	if(pSynMember1->GetPosition() < pSynMember2->GetPosition()){
//		return true;
//	}
//	if(pSynMember1->GetPosition() > pSynMember2->GetPosition()){
//		return false;
//	}
//
//	//再按帮派贡献排
//	if(pSynMember1->GetContribution() < pSynMember2->GetPosition()){
//		return false;
//	}
//	return true;
//}

//查看帮派成员列表
void	SyndicateMgr::ViewSynMemberList(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());
	if( 0 == pSyndicate){
		TRACE("<error> %s : %d 行 获取玩家帮派失败!!,userID = %d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_ActorUserID));
		return;
	}

	pSyndicate->ViewSynMemberList(pActor);
}

//查看帮派加入申请者列表
void	SyndicateMgr::ViewSynApplyList(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	this->__ViewSynApplyList(pActor);
}

//任免
void	SyndicateMgr::RenMianUser(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_RenMianSynMember_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());
	if( 0 == pSyndicate){
		return;
	}

	TSynID SynID = pSyndicate->GetSynID();

	ISyndicateMember * pTargetSynMember = pSyndicate->GetSynMember(UID(Req.m_TargetUserUID));
	if( 0 == pTargetSynMember){
		return;
	}

	ISyndicateMember * pSynMember = pSyndicate->GetSynMember(pActor->GetUID());
	if( 0 == pSynMember){
		return;
	}

	SC_RenMianSynMember_Rsp	Rsp;

	if( pActor->GetUID() == Req.m_TargetUserUID){
		Rsp.m_SynRetCode = enSynRetCode_ErrSelf;
	}else if( enSynRetCode_OK == (Rsp.m_SynRetCode = this->CanRenMian(pSynMember, pTargetSynMember->GetPosition(), Req.m_RenMianType))){

		if( enRenMianType_LetOut == Req.m_RenMianType){
			//踢出操作
			this->LetOutOfSyndicate(SynID, UID(Req.m_TargetUserUID));
		}else{
			pTargetSynMember->SetPosition((enumSynPosition)Req.m_RenMianType);

			if((enumSynPosition)Req.m_RenMianType == enumSynPosition_Leader){
				pSyndicate->SetLeaderUID(Req.m_TargetUserUID.ToUint64());
				//任免别人为帮主,则自己降为弟子
				pSyndicate->SetLeaderName(pTargetSynMember->GetSynMemberName());

				pSynMember->SetPosition(enumSynPosition_General);
			}
		}

		Rsp.m_SynMemberNum = pSyndicate->GetSynMemberNum();
	}

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_RenMian, sizeof(Rsp))<< Rsp;
	pActor->SendData(ob.TakeOsb());
}

//邀请玩家加入帮派
void	SyndicateMgr::InviteUser(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	ISyndicateMember * pSynMember = this->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSynMember){
		return;
	}

	ISyndicate * pSyn = this->GetSyndicate(pSynMember->GetSynID());
	if( 0 == pSyn){
		return;
	}

	SC_Syndicate_Rsp Rsp;

	if( pSynMember->GetPosition() != enumSynPosition_Leader && pSynMember->GetPosition() != enumSynPosition_SecondLeader){
		//没权限
		Rsp.m_SynRetCode = enSynRetCode_NoPermission;
	}else if( pSyn->GetSynMemberNum() >= pSyn->GetMaxMemberNum()){
		//帮派人数已满，不能再邀请玩家加入
		Rsp.m_SynRetCode = enSynRetCode_MemberFull;
	}

	if( Rsp.m_SynRetCode != enSynRetCode_OK){

		OBuffer1k ob;
		ob << SyndicateHeader(enSyndicateCmd_Invite, sizeof(Rsp)) << Rsp; 
		pActor->SendData(ob.TakeOsb());
		return;
	}

	CS_InviteUser_Req Req;
	ib >> Req;

	//先请求，用玩家的名字得到玩家的ID
	SDB_Get_ActorUIDByName_Req GetActorDataByName_Req;
	strncpy(GetActorDataByName_Req.ActorName, Req.m_szDestUserName, sizeof(GetActorDataByName_Req.ActorName));

	OBuffer1k ob;
	ob << GetActorDataByName_Req;

	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetActorUIDByName1, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

//批准玩家加入帮派
void	SyndicateMgr::AllowUser(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_AllowAdd_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误!!", __FUNCTION__, __LINE__);
		return;
	}

	ISyndicateMember * pSynMember = this->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSynMember){
		return;
	}

	ISyndicate * pSyndicate = this->GetSyndicate(pSynMember->GetSynID());
	if( 0 == pSyndicate){
		return;
	}

	SC_AllowRetCode_Rsp Rsp;

	OBuffer1k ob;

	enumSynPosition Position = pSynMember->GetPosition();
	
	if( Position != enumSynPosition_Leader && Position != enumSynPosition_SecondLeader){
		//没权限
		Rsp.m_SynRetCode = enSynRetCode_NoPermission;
	}else if( pSyndicate->GetSynMemberNum() >= pSyndicate->GetMaxMemberNum()){
		//帮派人数已满
		Rsp.m_SynRetCode = enSynRetCode_MemberFull;
	}else{
		//OK
		for( int i = 0; i < Req.m_num; ++i)
		{
			UID uidApplyUser;
			ib >> uidApplyUser;

			if( ib.Error()){
				TRACE("<error> %s : %d 行 客户端数据长度有误!!", __FUNCTION__, __LINE__);
				return;
			}

			if( pSyndicate->GetSynMemberNum() >= pSyndicate->GetMaxMemberNum()){
				break;
			}

			Rsp.m_SynRetCode = (enSynRetCode)this->AllowUserJoin(pActor, uidApplyUser);
		}

		if( Req.m_num > 1){
			Rsp.m_SynRetCode = enSynRetCode_OK;
		}

		Rsp.m_SynMemberNum = pSyndicate->GetSynMemberNum();

		//刷新下
		this->__ViewSynApplyList(pActor);
	}

	ob.Reset();
	ob << SyndicateHeader(enSyndicateCmd_Allow, sizeof(Rsp)) << Rsp; 
	pActor->SendData(ob.TakeOsb());
}

//不批准玩家加入帮派
void	SyndicateMgr::NotAllowUser(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_RefuseAdd_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误!!", __FUNCTION__, __LINE__);
		return;
	}

	ISyndicateMember * pSyndicateMember = this->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSyndicateMember){
		return;
	}

	SC_RefuseAdd_Rsp Rsp;
	OBuffer1k ob;

	enumSynPosition Position = pSyndicateMember->GetPosition();

	if( Position != enumSynPosition_Leader && Position != enumSynPosition_SecondLeader){
		//没权限
		Rsp.m_SynRetCode = enSynRetCode_NoPermission;
	}else{
		for( int i = 0; i < Req.m_num; ++i)
		{
			UID uidApplyUser;
			ib >> uidApplyUser;
			
			if( ib.Error()){
				TRACE("<error> %s : %d 行 客户端数据长度有误!!", __FUNCTION__, __LINE__);
				return;		
			}

			//删除申请记录
			this->RefuseUserJoin(pActor, uidApplyUser);
		}
	}

	ob.Reset();
	ob << SyndicateHeader(enSyndicateCmd_NotAllow, sizeof(Rsp)) << Rsp; 
	pActor->SendData(ob.TakeOsb());
}

//玩家确认退出帮派
void	SyndicateMgr::QuitSyn(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	ISyndicateMember * pSyndicateMember = this->GetSyndicateMember(pActor->GetUID());
	if(  0 == pSyndicateMember){
		return;
	}

	SC_QuitSynRetCode_Rsp Rsp;
	Rsp.m_SynRetCode = enSynRetCode_OK;
	OBuffer1k ob;

	if( enumSynPosition_Leader == pSyndicateMember->GetPosition()){
		//帮主为解散
		this->JieSanSyn(pActor, Rsp.m_SynRetCode);
	}else{
		this->RemoveSynMember(this->GetUserSynID(pActor->GetUID()), pActor->GetUID());
	}

	ob.Reset();
	ob << SyndicateHeader(enSyndicateCmd_QuitSyn, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//查看帮派商铺
void	SyndicateMgr::ViewSynShop(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_ViewSynShop_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_SyndicateShop.ViewSynShop(pActor, Req.m_SynShopLabel);
}

//购买帮派物品
void	SyndicateMgr::BuySynGoods(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_BuySynGoods_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_SyndicateShop.BuySynGoods(pActor, Req.m_GoodsID);
}

//查看帮派技能
void	SyndicateMgr::ViewSynMagic(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	m_SynMagic.ViewSynMagic(pActor);
}

//学习帮派技能
void	SyndicateMgr::LearnSynMagic(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_LearnSynMagic_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_SynMagic.LearnSynMagic(pActor, Req.m_SynMagicID);
}

//进入帮派界面
void	SyndicateMgr::EnterSyn(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	SC_EnterSyn_Rsp Rsp;
	OBuffer1k ob;

	ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());
	
	if( 0 == pSyndicate){
		//玩家没有加入帮派
		Rsp.bHaveSyn = false;
		ob << SyndicateHeader(enSyndicateCmd_EnterSyn, sizeof(Rsp)) << Rsp;
	}else{
		ISyndicateMember * pSynMember = pSyndicate->GetSynMember(pActor->GetUID());
		if( 0 == pSynMember){
			TRACE("<error> %s : %d 行 玩家获取帮派成员失败！！,玩家ID = %d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_ActorUserID));
			return;
		}

		//玩家有帮派
		Rsp.bHaveSyn = true;
		EnterSynData SynData;
		strncpy(SynData.LeaderName, pSyndicate->GetLeaderName(), sizeof(SynData.LeaderName));
		SynData.Level		 = pSyndicate->GetSynLevel();
		SynData.SynID		 = pSyndicate->GetSynID();
		SynData.SynMemberNum = pSyndicate->GetSynMemberNum();
		SynData.Rank		 = this->GetSynRank(pSyndicate->GetSynID());
		strncpy(SynData.SynName, pSyndicate->GetSynName(), sizeof(SynData.SynName));
		SynData.SynExp		 = pSyndicate->GetSynExp();
		SynData.UpNeedExp	 = pSyndicate->GetUpLevelNeedExp();
		SynData.Contribution = pSynMember->GetContribution();
		SynData.MaxMemberNum = pSyndicate->GetMaxMemberNum();
		SynData.SynWarAbility = pSyndicate->GetSynWarAbility();
		SynData.SynWarScore  = pSyndicate->GetSynWarScore();
		strncpy(SynData.SynMsg, pSyndicate->GetSynMsg(), sizeof(SynData.SynMsg));

		if ( pSynMember->GetPosition() == enumSynPosition_Leader || pSynMember->GetPosition() == enumSynPosition_SecondLeader)
			SynData.bCanChangeMsg = true;
		else
			SynData.bCanChangeMsg = false;
		

		ob << SyndicateHeader(enSyndicateCmd_EnterSyn, sizeof(Rsp) + sizeof(SynData)) << Rsp << SynData;
	}

	pActor->SendData(ob.TakeOsb());
}

//验证是否可以创建帮派
bool	SyndicateMgr::CanCreateSyndicate(IActor * pActor, const char * szSynName, enSynRetCode & SynRetCode)
{
	//检测玩家是否已有帮派
	ISyndicateMember * pSynMeber = this->GetSyndicateMember(pActor->GetUID());
	if( 0 != pSynMeber){
		SynRetCode = enSynRetCode_ExistSyn;
		return false;	
	}

	//检测帮派名称是否有相同的
	for( std::hash_map<TSynID, ISyndicate *>::iterator itSyn = m_mapSyndicate.begin(); itSyn != m_mapSyndicate.end();++itSyn)
	{
		ISyndicate * pSyn = itSyn->second;

		if( 0 == memcmp(pSyn->GetSynName(), szSynName, THING_NAME_LEN)){
			SynRetCode = enSynRetCode_ExistName;
			return false;
		}		
	}

	if( g_pGameServer->GetKeywordFilter()->IsValidName(szSynName) == false){
		//不能使用非法字符
		SynRetCode = enSynRetCode__ErrKeyword;
		return false;
	}

	//等级是否高于最低创建帮派要求
	UINT8 minLevel = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MinLevelCreateSyn;
	
	if( minLevel > pActor->GetCrtProp(enCrtProp_Level)){
		SynRetCode = enSynRetCode_ErrLevel;
		return false;
	}

	//创建的帮派名字是否不为空
	if( 0 == strcmp(szSynName, "")){
		SynRetCode = enSynRetCode_ErrSynName;
		return false;
	}


	return true;
}

//创建帮派(数据库回调涵数调用)
void	SyndicateMgr::HandleCreateSyndicate(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam				 OutParam;
	SDB_Get_SynID_Rsp		 SynID_Rsp;
	RspIb >> RspHeader >> OutParam >> SynID_Rsp;

	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());
	SDB_Insert_Syndicate_Req CreateSynInfo;
	ReqIb >> CreateSynInfo;

	if(RspIb.Error() || ReqIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK )
		return;

	if ( SynID_Rsp.SynID == INVALID_SYN_ID )
	{
		TRACE("<error> %s : %d Line 创建帮派时，数据库返回的帮派ID为0",__FUNCTION__,__LINE__);
		return;
	}

	//添加到帮派集合中去
	SyndicateInfo SynInfo;
	SynInfo.m_Exp			= 0;
	SynInfo.m_Level			= 1;
	SynInfo.m_MemberNum		= 0;
	SynInfo.m_NeedExp		= g_pGameServer->GetConfigServer()->GetSyndicateCnfg(SynInfo.m_Level)->m_UpLevelNeedExp;
	SynInfo.m_SynID			= SynID_Rsp.SynID;
	strncpy(SynInfo.m_szLeaderName, CreateSynInfo.szLeaderName, sizeof(SynInfo.m_szLeaderName));
	strncpy(SynInfo.m_szSynName, CreateSynInfo.szSynName, sizeof(SynInfo.m_szSynName));
	SynInfo.m_uidLeader		= userdata;
	SynInfo.m_MaxMemberNum  = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(SynInfo.m_Level)->m_MaxMemberNum;
	SynInfo.m_SynWarScore	= 0;
	SynInfo.m_SynWarTotalScore = 0;
	SynInfo.m_SynWarAbility =  pActor->GetCrtProp(enCrtProp_ActorCombatAbility) /g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_SynWarAbilityParam ;

	ISyndicate * pSyn = new Syndicate(SynInfo);
	m_mapSyndicate[SynID_Rsp.SynID] = pSyn;

	//添加到帮派成员集合中去
	SyndicateMemberInfo  SynMemberInfo;
	SynMemberInfo.m_Position = enumSynPosition_Leader;
	SynMemberInfo.m_SynID    = SynID_Rsp.SynID;
	SynMemberInfo.m_uidUser	 = UID(userdata);
	strncpy(SynMemberInfo.m_szUserName, CreateSynInfo.szLeaderName, sizeof(SynMemberInfo.m_szUserName));
	SynMemberInfo.m_Userlevel = pActor->GetCrtProp(enCrtProp_Level);
	SynMemberInfo.m_CombatAbility = pActor->GetCrtProp(enCrtProp_ActorCombatAbility);
	SynMemberInfo.m_ActorFacade = pActor->GetCrtProp(enCrtProp_ActorFacade);
	SynMemberInfo.m_Score = 0;	
	SynMemberInfo.m_SynWarLv = 0;

	this->AddToSyndicate(SynMemberInfo);

	pSyn->UpdateSynCombatAbility();

	SC_CreateSyndicate_Rsp Rsp;
	Rsp.m_RetCode = enSynRetCode_OK;

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_CreateSyn, sizeof(SC_CreateSyndicate_Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//加载帮派的所有数据
void	SyndicateMgr::LoadSynInfo()
{
	SDB_Get_SyndicateList_Req Req;

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_LoadSyndicateListInfo, ob.TakeOsb(), this, 0);

	SDB_Get_SyndicateMemberList_Req mReq;

	ob.Reset();
	ob << mReq;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID, enDBCmd_LoadSyndicateMemberListInfo, ob.TakeOsb(), this, 0);

	SDB_Get_SyndicateApplyList_Req aReq;

	ob.Reset();
	ob << aReq;

	g_pGameServer->GetDBProxyClient()->Request(SYNDICATEID + 1, enDBCmd_LoadSyndicateApplyInfo, ob.TakeOsb(), this, 0);
}

//加入到帮派成员集合中
void	SyndicateMgr::AddToSynMemberList(ISyndicateMember * pSynMember)
{
	ISyndicate * pSyndicate = this->GetSyndicate(pSynMember->GetSynID());
	if( 0 == pSyndicate){
		return;
	}

	pSyndicate->AddToMemberList(pSynMember);
}

//加入到帮派申请者集合中
void	SyndicateMgr::AddToSynApplyList(const SyndicateApply SynApply)
{
	std::hash_map<TSynID, MAPSYNAPPLY>::iterator iter = m_mapSyndicateApply.find(SynApply.m_SynID);

	if( iter == m_mapSyndicateApply.end()){
		MAPSYNAPPLY mapSynApply;
		mapSynApply[SynApply.m_uidApplyUser] = SynApply;
		m_mapSyndicateApply[SynApply.m_SynID] = mapSynApply;
	}else{
		MAPSYNAPPLY & mapSynApply = iter->second;
		mapSynApply[SynApply.m_uidApplyUser] = SynApply;
	}
}

//检测是否可以申请加入此帮派
bool	SyndicateMgr::CheckCanApplyJoinSyn(IActor * pActor, TSynID SynID, enSynRetCode & enRetCode)
{
	std::hash_map<TSynID, ISyndicate *>::iterator itSyn  = m_mapSyndicate.find(SynID);

	if( itSyn == m_mapSyndicate.end()){
		enRetCode = enSynRetCode_NoSyn;
		return false;
	}

	ISyndicate * pSyn = itSyn->second;
	if( 0 == pSyn){
		enRetCode = enSynRetCode_NoSyn;
		return false;		
	}

	if( pSyn->GetSynMemberNum() >= pSyn->GetMaxMemberNum()){
		enRetCode = enSynRetCode_MemberFull;
		return false;
	}

	ISyndicateMember * pSyndicateMember = this->GetSyndicateMember(pActor->GetUID());
	if( 0 != pSyndicateMember){
		enRetCode = enSynRetCode_ExistSyn;
		return false;
	}

	std::hash_map<TSynID, MAPSYNAPPLY>::iterator iter = m_mapSyndicateApply.find(SynID);
	if( iter != m_mapSyndicateApply.end()){

		MAPSYNAPPLY & mapApply = iter->second;

		MAPSYNAPPLY::iterator it = mapApply.find(pActor->GetUID());
		if( it != mapApply.end()){
			//玩家已经在该帮派的加入名单中，不能再申请
			enRetCode = enSynRetCode_ExistSynApply;
			return false;
		}
	}

	//加入帮派最低等级
	UINT8 MinLevel = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MinLevelJoinSyn;
	if( pActor->GetCrtProp(enCrtProp_Level) < MinLevel){
		enRetCode = enSynRetCode_ErrLevelJoin;
		return false;
	}

	enRetCode = enSynRetCode_OK;
	return true;
}

//检测是否可任免
enSynRetCode SyndicateMgr::CanRenMian(ISyndicateMember * pSynMember, enumSynPosition BeforeRenMian, enRenMianType AfterRenMian)
{
	enumSynPosition	 Position  =  pSynMember->GetPosition();

	if( Position != enumSynPosition_Leader && Position != enumSynPosition_SecondLeader){
		return enSynRetCode_NoPermission;
	}

	if( AfterRenMian == enRenMianType_LetOut){
		//踢出操作
		if( Position == enumSynPosition_Leader &&  BeforeRenMian == enumSynPosition_Leader){
			return enSynRetCode_NoPermission;
		}

		if( Position == enumSynPosition_SecondLeader && (BeforeRenMian == enumSynPosition_Leader || BeforeRenMian == enumSynPosition_SecondLeader)){
			return enSynRetCode_NoPermission;
		}
	}else{
		//其它任免操作
		if( Position == enumSynPosition_SecondLeader && (AfterRenMian == enumSynPosition_Leader || AfterRenMian == enumSynPosition_SecondLeader ||
				BeforeRenMian == enumSynPosition_SecondLeader || BeforeRenMian == enumSynPosition_Leader))
		{
			return enSynRetCode_NoPermission;
		}
	}
	
	return enSynRetCode_OK;
}


//踢出帮派
void	SyndicateMgr::LetOutOfSyndicate(TSynID SynID, const UID & uid_TargetUser)
{
	if( !this->RemoveSynMember(SynID, uid_TargetUser)){
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uid_TargetUser);
	if( 0 == pActor){
		return;
	}

	ISystemMsg * pSystemMsg = g_pGameServer->GetRelationServer()->GetSystemMsg();
	if( 0 == pSystemMsg){
		return;
	}

	//被踢出，如果还在帮派界面操作的关闭界面
	SC_CloseSynWindow Rsp;
	Rsp.m_CloseType = enCloseSynWindow_TiChu;

	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_SC_CloseSynWindow, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	//给玩家发公告
	SInsertMsg Msg;

	// fly add	20121106

	strncpy(Msg.szMsgBody, g_pGameServer->GetGameWorld()->GetLanguageStr(10019), sizeof(Msg.szMsgBody));
	//strncpy(Msg.szMsgBody, "您已被踢出帮派！！！！", sizeof(Msg.szMsgBody));

	pSystemMsg->ViewMsg(pActor, enMsgType_Insert, &Msg);
}

//获得帮派排名
UINT8	SyndicateMgr::GetSynRank(TSynID SynID)
{
	std::vector<ISyndicate *>  vectSyn;
	
	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();
	for( ; iter != m_mapSyndicate.end(); ++iter)
	{
		ISyndicate * pSyn = iter->second;
		if( 0 == pSyn){
			continue;
		}

		vectSyn.push_back( pSyn);
	}
	//进行帮派排序，先按等级排再按帮派人数排
	std::sort(vectSyn.begin(), vectSyn.end(), sortsyn);

	//获得排名
	for( int nRank = 0; nRank < vectSyn.size(); ++nRank)
	{
		ISyndicate * pSyn = vectSyn[nRank];

		if( pSyn->GetSynID() == SynID){
			return nRank + 1;
		}
	}

	return 0;
}

//加载数据(数据库回调涵数调用)
void	SyndicateMgr::HandleLoadSynListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	for(int i = 0; i < OutParam.retCode; ++i)
	{
		SDB_Get_SyndicateRsp Rsp;

		RspIb >> Rsp;
		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SyndicateInfo SynInfo;
		SynInfo.m_Exp			= Rsp.Exp;
		SynInfo.m_Level			= Rsp.Level;

		const SSyndicateCnfg * pSynCnfg = g_pGameServer->GetConfigServer()->GetSyndicateCnfg(Rsp.Level);
		if( 0 == pSynCnfg){
			TRACE("<warning> %s : %d 行 获取帮派配置信息错误!!,帮派等级=%d", __FUNCTION__, __LINE__, Rsp.Level);
			continue;
		}

		SynInfo.m_MaxMemberNum	= pSynCnfg->m_MaxMemberNum;
		SynInfo.m_MemberNum		= Rsp.MemberNum;
		SynInfo.m_NeedExp		= pSynCnfg->m_UpLevelNeedExp;
		SynInfo.m_SynID			= Rsp.SynID;
		strncpy(SynInfo.m_szLeaderName, Rsp.LeaderName, sizeof(SynInfo.m_szLeaderName));
		strncpy(SynInfo.m_szSynName, Rsp.SynName, sizeof(SynInfo.m_szSynName));
		SynInfo.m_uidLeader		= Rsp.Uid_Leader;
		SynInfo.m_SynWarScore   = Rsp.SynWarScore;
		SynInfo.m_SynWarAbility = Rsp.SynWarAbility;
		strncpy(SynInfo.m_szSynMsg, Rsp.szSynMsg, sizeof(SynInfo.m_szSynMsg));
		SynInfo.m_bWin			= (enumSynWarbWin)Rsp.bWin;
		strncpy(SynInfo.m_PreEnemySynName,Rsp.PreEnemySynName,sizeof(SynInfo.m_PreEnemySynName));
		SynInfo.m_SynWarTotalScore = Rsp.SynWarTotalScore;
		
		ISyndicate * pSyn = new Syndicate(SynInfo);

		m_mapSyndicate[Rsp.SynID] = pSyn;
	}
}

//加载数据(数据库回调涵数调用)
void	SyndicateMgr::HandleLoadSynMemberListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("HandleLoadSynMemberListInfo DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	for(int i = 0; i < OutParam.retCode; ++i)
	{
		SDB_Get_SyndicateMember_Rsp Rsp;

		RspIb >> Rsp;
		
		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 客户端数据长度有误！！", __FUNCTION__, __LINE__);
			break;
		}

		SyndicateMemberInfo SynMemInfo;

		SynMemInfo.m_Contribution = Rsp.Contribution;
		SynMemInfo.m_Position	  = (enumSynPosition)Rsp.Position;
		SynMemInfo.m_SynID		  = Rsp.SynID;
		strncpy(SynMemInfo.m_szUserName, Rsp.MemberName, sizeof(SynMemInfo.m_szUserName));
		SynMemInfo.m_Userlevel	  = Rsp.MemberLevel;
		SynMemInfo.m_uidUser	  = UID(Rsp.Uid_Member);
		SynMemInfo.m_VipLevel	  = Rsp.VipLevel;
		SynMemInfo.m_CombatAbility  = Rsp.CombatAbility;
		SynMemInfo.m_ActorFacade  = Rsp.ActorFacade;
		SynMemInfo.m_Score		  = Rsp.Score;
		SynMemInfo.m_SynWarLv	  = Rsp.SynWarLv;
		SynMemInfo.m_LastOnlineTime = Rsp.LastOnlineTime;

		ISyndicateMember * pSyndicateMember = new SyndicateMember(SynMemInfo);
		
		this->AddToSynMemberList(pSyndicateMember);

		m_mapSynID[UID(Rsp.Uid_Member)] = Rsp.SynID;
	}

	//更新帮派战力
	std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();

	for ( ; iter != m_mapSyndicate.end(); ++iter )
	{
		ISyndicate * pSyn = iter->second;

		if ( 0 != pSyn )
		{
			pSyn->UpdateSynCombatAbility();
		}
	}
}

//加载数据(数据库回调涵数调用)
void	SyndicateMgr::HandleLoadSynApplyListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("HandleLoadSynApplyListInfo DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	for(int i = 0; i < OutParam.retCode; ++i)
	{
		SDB_Get_SyndicateApply_Rsp Rsp;

		RspIb >> Rsp;

		if(RspIb.Error()){
			TRACE("HandleLoadSynApplyListInfo DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SyndicateApply SynApply;

		SynApply.m_ApplyUserLayer = Rsp.ApplyUserLayer;
		SynApply.m_ApplyUserLevel = Rsp.ApplyUserLevel;
		SynApply.m_SynID		  = Rsp.SynID;
		strncpy(SynApply.m_szApplyUserName, Rsp.ApplyUserName, sizeof(SynApply.m_szApplyUserName));
		SynApply.m_uidApplyUser	  = UID(Rsp.Uid_ApplyUser);
		SynApply.m_LastOnlineTime = Rsp.LastOnlineTime;

		this->AddToSynApplyList(SynApply);
	}
}

//邀请玩家加入帮派的数据库回调函数
void	SyndicateMgr::HandleInviteUser(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ActorUIDByName_Rsp       ActorIDByName_Rsp;
	RspIb >> RspHeader >> OutParam >> ActorIDByName_Rsp;

	if(RspIb.Error())
	{
		TRACE("HandleInviteUser DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_Syndicate_Rsp Rsp;
	Rsp.m_SynRetCode = enSynRetCode_OK;

	OBuffer1k ob;

	if(enDBMailRetCode_NoUser == OutParam.retCode){
		//不存在此用户
		Rsp.m_SynRetCode = enSynRetCode_NoUser;
	}else if( INVALID_SYN_ID != this->GetUserSynID(UID(ActorIDByName_Rsp.Uid_Actor))){
		//该玩家已经存在帮派了
		Rsp.m_SynRetCode = enSynRetCode_ExistSyn;	
	}else{
		//写帮派邮件
		IMailPart * pMailPart = pActor->GetMailPart();
		if( 0 == pMailPart){
			return;
		}

		SWriteData WriteData;

		ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());
		if( 0 == pSyndicate){
			return;
		}

		// fly add	20121106

		sprintf(WriteData.m_szContentText, "%s%s", pSyndicate->GetSynName(), g_pGameServer->GetGameWorld()->GetLanguageStr(10020));

		strncpy(WriteData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10021), sizeof(WriteData.m_szThemeText));
		//sprintf(WriteData.m_szContentText, "%s%s", pSyndicate->GetSynName(), "帮派邀请您加入");

		//strncpy(WriteData.m_szThemeText, "邀请加入", sizeof(WriteData.m_szThemeText));

		WriteData.m_UidDestUser = UID(ActorIDByName_Rsp.Uid_Actor);
		
		WriteData.m_MailType = enMailType_Syn;	//帮派邮件

		pMailPart->WriteMail(WriteData);
	}

	ob << SyndicateHeader(enSyndicateCmd_Invite, sizeof(SC_Syndicate_Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}


///////////////////////////////////////////////////////////////////////////
	//打开帮派保卫战面板
	void		SyndicateMgr::OpenSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
		if( 0 ==  pFuBenPart){
			return;
		}

		pFuBenPart->OpenSynFuBen();
	}

	//进入帮派保卫战
	void		SyndicateMgr::EnterSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		ISyndicateMember * pSyndicateMember = this->GetSyndicateMember(pActor->GetUID());

		if(pSyndicateMember == 0)
		{
			return ;
		}

		CS_EnterSynCombat_Req Req;

		ib >> Req;

		if(ib.Error())
		{
			TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return;
		}

		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
		if( 0 == pFuBenPart){
			return;
		}

		pFuBenPart->ChallengeSynFuBen();

		//pSyndicateMember->EnterSynCombat(Req.m_Mode);
	}

	//挑战怪物
	void		SyndicateMgr::SynCombatWithNpc(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		//ISyndicateMember * pSyndicateMember = this->GetSyndicateMember(pActor->GetUID());

		//if(pSyndicateMember == 0)
		//{
		//	return ;
		//}

		//CS_SynCombatNpc_Req Req;

		//ib >> Req;

		//if(ib.Error())
		//{
		//	TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		//	return;
		//}

		//pSyndicateMember->CombatWithNpc(Req.m_FuBenID, Req.m_uidNpc);
	}

	//招募帮派成员
	void		SyndicateMgr::ZhaoMuSynMember(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		IRelationServer * pRelationServer = g_pGameServer->GetRelationServer();
		if( 0 == pRelationServer){
			return;
		}

		ISyndicateMember * pSynMember = this->GetSyndicateMember(pActor->GetUID());
		if( 0 == pSynMember){
			return;
		}

		SC_ZhaoMuSynMemberRet_Rsp Rsp;

		enumSynPosition Position = pSynMember->GetPosition();

		ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();

		if ( 0 == pCDTimerPart )
			return;

		const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

		if( Position != enumSynPosition_Leader && Position != enumSynPosition_SecondLeader){
			//没权限
			Rsp.m_SynRetCode = enSynRetCode_NoPermission;
		}
		else if ( !pCDTimerPart->IsCDTimeOK(ServerParam.m_ZhaoMuSynMemberCDTimer) )
		{
			Rsp.m_SynRetCode = enSynRetCode_InCDTime;
		}
		else
		{
			pRelationServer->ZhaoMuSynMember(pActor);

			pCDTimerPart->RegistCDTime(ServerParam.m_ZhaoMuSynMemberCDTimer);
		}

		OBuffer1k ob;
		ob << SyndicateHeader(enSyndicateCmd_ZhaoMuSynMember, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}

	//查看帮派福利
	void		SyndicateMgr::ViewSynWelfare(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		const std::vector<SSynWelfareCnfg> & mapSynWelfare = g_pGameServer->GetConfigServer()->GetAllSynWelfareCnfg();

		SC_SynWelfare_Rsp Rsp;
		Rsp.m_Num = mapSynWelfare.size();

		OBuffer4k ob;
		ob << SyndicateHeader(enSyndicateCmd_OpenSynWelfare, sizeof(Rsp) + Rsp.m_Num * sizeof(SynWelfareData)) << Rsp;

		for( int i = 0; i < mapSynWelfare.size(); ++i)
		{
			const SSynWelfareCnfg & SynWelfareCnfg = mapSynWelfare[i];

			SynWelfareData Data;

			Data.m_SynWelfareID = SynWelfareCnfg.m_SynWelfareID;
			Data.m_NeedSynLevel = SynWelfareCnfg.m_NeedSynLevel;
			Data.m_ResID		   = SynWelfareCnfg.m_ResID;

			// fly add	20121106
			strncpy(Data.m_SynWelfareName, g_pGameServer->GetGameWorld()->GetLanguageStr(SynWelfareCnfg.m_WelfareNameLangID), sizeof(Data.m_SynWelfareName));
			strncpy(Data.m_Descript, g_pGameServer->GetGameWorld()->GetLanguageStr(SynWelfareCnfg.m_WelfareDescLangID), sizeof(Data.m_Descript));
			//strncpy(Data.m_SynWelfareName, SynWelfareCnfg.m_WelfareName.c_str(), sizeof(Data.m_SynWelfareName));
			//strncpy(Data.m_Descript, SynWelfareCnfg.m_WelfareDesc.c_str(), sizeof(Data.m_Descript));

			ob.Push(&Data, sizeof(Data));
		}

		pActor->SendData(ob.TakeOsb());
	}

	//重置帮派保卫战
	void		SyndicateMgr::ResetSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib)
	{
		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
		if( 0 == pFuBenPart){
			return;
		}

		pFuBenPart->ResetSynFuBen();
	}

	//查看帮派副本掉落物品
	void		SyndicateMgr::ViewSynFuBenDropGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_ViewSynFuBenForwardGoods Req;

		ib >> Req;

		if(ib.Error())
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		std::vector<TGoodsID> vectGoods;

		const std::vector<SDropGoods> * pVectDropGoods = g_pGameServer->GetConfigServer()->GetDropGoodsCnfg(Req.DropID);

		if( 0 == pVectDropGoods)
		{
			TRACE("<error> %s : %d line 获取掉落的配置文件失败!!,DropID = %d", __FUNCTION__,__LINE__,Req.DropID);
			return;
		}

		for ( int i = 0; i < (*pVectDropGoods).size(); ++i )
		{
			const std::vector<TGoodsID> & vecGoods = (*pVectDropGoods)[i].m_vectDropGoods;

			for ( int k = 0; k + 2 < vecGoods.size(); k +=3 )
			{
				vectGoods.push_back(vecGoods[k]);
			}
		}

		SC_ViewSynFuBenForwardGoods Rsp;

		Rsp.m_Num = vectGoods.size();

		OBuffer4k ob;

		ob << SyndicateHeader(enSyndicateCmd_ViewSynFuBenDropGoods,sizeof(Rsp) + Rsp.m_Num * sizeof(TGoodsID)) << Rsp;

		for ( int i = 0; i < vectGoods.size(); ++i )
		{
			ob << vectGoods[i];
			pActor->SendGoodsCnfg(vectGoods[i]);
		}

		pActor->SendData(ob.TakeOsb());



		//const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfgByMode(Req.SynMode);

		//if ( 0 == pSynCombatCnfg )
		//{
		//	TRACE("<error> %s : %d Line 获取帮派副本配置错误！！模式%d", __FUNCTION__, __LINE__, Req.SynMode);
		//	return;
		//}

		//SC_ViewSynFuBenForwardGoods Rsp;

		//Rsp.m_Num = 0;
		//OBuffer4k ob;

		//const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pSynCombatCnfg->m_SynFuBenID);
		//if( 0 != pFuBenCnfg){

		//	for( int i = 0; i < pFuBenCnfg->m_MapID.size(); ++i)
		//	{
		//		UINT16 DropID = g_pGameServer->GetConfigServer()->GetMapBossDropID(pFuBenCnfg->m_MapID[i]);
		//		
		//		const std::vector<SDropGoods> * pvectGoods = g_pGameServer->GetConfigServer()->GetDropGoodsCnfg(DropID);
		//		if( 0 == pvectGoods){
		//			continue;
		//		}

		//		for(int index = 0; index < (*pvectGoods).size(); ++index)
		//		{
		//			const SDropGoods & DropGoods = (*pvectGoods)[index];

		//			for( int k = 0; k + 2 < DropGoods.m_vectDropGoods.size(); k += 3)
		//			{
		//				ob << DropGoods.m_vectDropGoods[k];

		//				pActor->SendGoodsCnfg(DropGoods.m_vectDropGoods[k]);

		//				++Rsp.m_Num;
		//			}
		//		}
		//	}
		//}

		// OBuffer4k ob2;
		// ob2 << SyndicateHeader(enSyndicateCmd_ViewSynFuBenDropGoods,SIZE_OF(Rsp) + ob.Size()) << Rsp;

		// if( ob.Size() > 0){
		//	ob2 << ob;
		// }

		// pActor->SendData(ob2.TakeOsb());
	}

	//允许玩家加入帮派
	UINT8		SyndicateMgr::AllowUserJoin(IActor * pActor, UID uidApplyUser)
	{
		TSynID SynID = this->GetUserSynID(pActor->GetUID());

		if( this->GetUserSynID(uidApplyUser) != 0){

			this->DeleteApply(SynID, uidApplyUser);
			return enSynRetCode_ExistSyn;
		}

		std::hash_map<TSynID, MAPSYNAPPLY>::iterator iter = m_mapSyndicateApply.find(SynID);
		if( iter == m_mapSyndicateApply.end()){
			return enSynRetCode_NoSyn;
		}

		MAPSYNAPPLY & mapApply = iter->second;

		MAPSYNAPPLY::iterator it = mapApply.find(uidApplyUser);
		if( it == mapApply.end()){
			return enSynRetCode_NotExistApply;
		}

		SyndicateApply & SynApply = it->second;

		SyndicateMemberInfo SynMemberInfo;

		SynMemberInfo.m_Position = enumSynPosition_General;
		SynMemberInfo.m_SynID  = SynID;
		strncpy(SynMemberInfo.m_szUserName, SynApply.m_szApplyUserName,sizeof(SynMemberInfo.m_szUserName));
		SynMemberInfo.m_uidUser = SynApply.m_uidApplyUser;
		SynMemberInfo.m_Userlevel = SynApply.m_ApplyUserLevel;

		if ( pActor->GetSession()->IsOnLine() )
		{
			SynMemberInfo.m_LastOnlineTime = CURRENT_TIME();
		}
		else
		{
			SynMemberInfo.m_LastOnlineTime = SynApply.m_LastOnlineTime;
		}
		

		this->AddToSyndicate(SynMemberInfo);

		//移除
		mapApply.erase(it);

		iter = m_mapSyndicateApply.begin();
		for( ; iter != m_mapSyndicateApply.end(); ++iter)
		{
			MAPSYNAPPLY & mapSynApply = iter->second;

			MAPSYNAPPLY::iterator itDel = mapSynApply.find(uidApplyUser);
			if( itDel != mapSynApply.end()){
				mapSynApply.erase(itDel);
			}
		}

		return enSynRetCode_OK;
	}

	//拒绝玩家加入帮
	void		SyndicateMgr::RefuseUserJoin(IActor * pActor, UID uidApplyUser)
	{
		TSynID SynID = this->GetUserSynID(pActor->GetUID());

		//从申请列表中删除
		this->DeleteApply(SynID, uidApplyUser);
	}

	//查看帮派加入申请者列表
	void		SyndicateMgr::__ViewSynApplyList(IActor * pActor)
	{
		SC_ViewListNum_Rsp RspNum;
		OBuffer1k ob;

		TSynID SynID = this->GetUserSynID(pActor->GetUID());

		std::hash_map<TSynID, MAPSYNAPPLY>::iterator iter = m_mapSyndicateApply.find(SynID);

		if( iter == m_mapSyndicateApply.end()){
			RspNum.m_SynNum = 0;
			ob << SyndicateHeader(enSyndicateCmd_ViewSynApplyList, sizeof(RspNum)) << RspNum;
			pActor->SendData(ob.TakeOsb());
			return;
		}

		MAPSYNAPPLY & mapSynApply = iter->second;
		
		RspNum.m_SynNum = mapSynApply.size();
		
		ob << SyndicateHeader(enSyndicateCmd_ViewSynApplyList, sizeof(RspNum) + RspNum.m_SynNum * sizeof(SC_ViewSynApplyList_Rsp)) << RspNum;

		MAPSYNAPPLY::iterator it = mapSynApply.begin();

		for( ; it != mapSynApply.end(); ++it)
		{
			SyndicateApply & SynApply = it->second;

			SC_ViewSynApplyList_Rsp	   Rsp;
			Rsp.m_uidApplyUser  = SynApply.m_uidApplyUser.m_uid;
			Rsp.m_Layer			= SynApply.m_ApplyUserLayer;
			Rsp.m_Level			= SynApply.m_ApplyUserLevel;
			strncpy(Rsp.m_szApplyUserName, SynApply.m_szApplyUserName, sizeof(Rsp.m_szApplyUserName));
			
			ob << Rsp;
		}

		pActor->SendData(ob.TakeOsb());
	}

	
	//解散帮派
	void		SyndicateMgr::JieSanSyn(IActor * pActor, enSynRetCode & SynRetCode)
	{
		ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());
		if( 0 == pSyndicate){
			TRACE("<error> %s : %d 行 解散帮派时，寻找不到该帮派!!", __FUNCTION__, __LINE__); 
			return;
		}


		UID uidLeader = pSyndicate->GetLeaderUID();
		IActor * pLeader = g_pGameServer->GetGameWorld()->FindActor(uidLeader);
		if( 0 == pLeader){
			return ;
		}

		const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

		if( pSyndicate->GetSynMemberNum() >= GameParam.m_CanJieSanMemberNum){
			//帮派人数大于10人，无法解散
			SynRetCode = enSynRetCode_ErrJieSan;
			return;
		}

		//解散
		pSyndicate->JieSanSyndicate();

		TSynID SynID = pSyndicate->GetSynID();

		std::hash_map<UID, TSynID, std::hash<UID>, std::equal_to<UID>>::iterator iter = m_mapSynID.begin();
		for( ; iter !=  m_mapSynID.end(); )
		{
			if( SynID == iter->second){
				m_mapSynID.erase(iter++);
				continue;
			}

			++iter;
		}

		//删除加入申请者
		std::hash_map<TSynID, MAPSYNAPPLY>::iterator itApply = m_mapSyndicateApply.find(SynID);
		if( itApply != m_mapSyndicateApply.end()){
			m_mapSyndicateApply.erase(itApply);
		}

		//删除帮派
		std::hash_map<TSynID, ISyndicate *>::iterator itSyn = m_mapSyndicate.find(SynID);
		if( itSyn != m_mapSyndicate.end()){
			m_mapSyndicate.erase(itSyn);
		}

		delete pSyndicate;

		//发布解散帮派事件
		SS_JieSanSyn JieSanSyn;
		JieSanSyn.m_SynID = SynID;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_JieSanSyn);
		pLeader->OnEvent(msgID,&JieSanSyn,sizeof(JieSanSyn));
	}

	//移除帮派成员
	bool		SyndicateMgr::RemoveSynMember(TSynID SynID, const UID & uid_TargetUser)
	{
		ISyndicate * pSyndicate = this->GetSyndicate(SynID);
		if( 0 == pSyndicate){
			TRACE("<error> %s : %d 行 没找到此帮派!!帮派ID=%d", __FUNCTION__, __LINE__, SynID);
			return false;
		}

		if( !pSyndicate->RemoveSyndicate(uid_TargetUser)){
			return false;
		}

		std::hash_map<UID, TSynID, std::hash<UID>, std::equal_to<UID>>::iterator itDel = m_mapSynID.find(uid_TargetUser);
		if( itDel != m_mapSynID.end()){
			m_mapSynID.erase(itDel);
		}

		return true;
	}

	//从帮派申请中删除
	void		SyndicateMgr::DeleteApply(TSynID SynID, UID uidApplyUser)
	{
		std::hash_map<TSynID, MAPSYNAPPLY>::iterator iter = m_mapSyndicateApply.find(SynID);
		if( iter == m_mapSyndicateApply.end()){
			return;
		}

		MAPSYNAPPLY & mapApply = iter->second;

		MAPSYNAPPLY::iterator it = mapApply.find(uidApplyUser);
		if( it == mapApply.end()){
			return;
		}

		//移除
		mapApply.erase(it);

		//删除申请记录
		SDB_Delete_SyndicateApply_Req ReqDel;
		ReqDel.SynID = SynID;
		ReqDel.uid_ApplyUser = uidApplyUser.ToUint64();

		OBuffer1k ob;
		ob << ReqDel;
		g_pGameServer->GetDBProxyClient()->Request(ReqDel.SynID, enDBCmd_DeleteSynApplyInfo, ob.TakeOsb(), 0, 0);	
	}

	//更新帮派战力
	void		SyndicateMgr::UpdateSynCombatAbility()
	{
		std::hash_map<TSynID, ISyndicate *>::iterator iter = m_mapSyndicate.begin();

		for ( ; iter != m_mapSyndicate.end(); ++iter )
		{
			ISyndicate * pSyn = iter->second;

			if ( 0 == pSyn )
				continue;

			pSyn->UpdateSynCombatAbility();
		}
	}

	//自动快速攻打帮派副本
	void		SyndicateMgr::SynFuBenAutoCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
		if( 0 == pFuBenPart){
			return;
		}

		pFuBenPart->SynFuBenAutoCombat();	
	}

	//打开帮战
	void		SyndicateMgr::OpenSynWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		m_SynWar.OpenSynCombat(pActor);
	}

	//打开宣战页面
	void		SyndicateMgr::OpenDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		m_SynWar.OpenDeclareWar(pActor);
	}

	//帮战宣战
	void		SyndicateMgr::DeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_DeclareWar Req;

		ib >> Req;

		if ( ib.Error() )
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		m_SynWar.DeclareWar(pActor, Req.m_SynID);
	}

	//接受宣战
	void		SyndicateMgr::AcceptDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_AcceptDeclareWar Req;

		ib >> Req;

		if ( ib.Error() )
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		m_SynWar.AcceptDeclareWar(pActor, Req.m_SynID);
	}

	//拒绝宣战
	void		SyndicateMgr::RefuseDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_RefuseDeclareWar Req;

		ib >> Req;

		if ( ib.Error() )
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		m_SynWar.RefuseDeclareWar(pActor, Req.m_SynID);
	}

	//进入帮战
	void		SyndicateMgr::EnterSynWar(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		m_SynWar.EnterSynCombatScene(pActor);
	}

	//购买战斗次数
	void		SyndicateMgr::BuyCombatNum(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		m_SynWar.BuyCombatNum(pActor);
	}

	
	//帮战战斗
	void		SyndicateMgr::SynWarCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_SynWarCombat Req;

		ib >> Req;

		if ( ib.Error() )
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		m_SynWar.Combat(pActor, Req.uidEnemy);
	}

	//修改帮派公告
	void		SyndicateMgr::ChangeSynMsg(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		CS_ChangeSynMsg Req;

		ib >> Req;

		if ( ib.Error() )
		{
			TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return ;
		}

		ISyndicate * pSyndicate = this->GetSyndicate(pActor->GetUID());

		if ( 0 == pSyndicate )
			return;

		ISyndicateMember * pSynMember = pSyndicate->GetSynMember(pActor->GetUID());

		if ( 0 == pSynMember )
			return;

		SC_ChangeSynMsg Rsp;

		if ( pSynMember->GetPosition() != enumSynPosition_Leader && pSynMember->GetPosition() != enumSynPosition_SecondLeader ){
			//权限不足
			Rsp.m_Result = enSynRetCode_NoPermission;
		} else {
			//OK
			Rsp.m_Result = enSynRetCode_OK;
			pSyndicate->SetSynMsg(Req.m_szSynMsg);
		}

		strncpy(Rsp.m_szSynMsg, pSyndicate->GetSynMsg(), sizeof(Rsp.m_szSynMsg));

		OBuffer1k ob;
		ob << SyndicateHeader(enSyndicateCmd_ChangeSynMsg, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}

	//得到帮战数据
	void		SyndicateMgr::GetSynWarData(IActor *pActor,UINT8 nCmd, IBuffer & ib)
	{
		m_SynWar.GetSynWarData(pActor);
	}

//确认帮派胜负弹出框
void	SyndicateMgr::ClickSynWarbWin(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	//传回主场景
	pActor->ComeBackMainScene();
}

//得到帮派副本BOSS信息
void	SyndicateMgr::GetSynFuBenBossInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_SynFuBenBossInfo Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	const SSynCombatCnfg * pSynCombatCnfg = g_pGameServer->GetConfigServer()->GetSynCombatCnfgByMode(Req.SynMode);

	if ( 0 == pSynCombatCnfg )
	{
		TRACE("<error> %s : %d Line 获取帮派副本配置错误！！模式%d", __FUNCTION__, __LINE__, Req.SynMode);
		return;
	}

	SC_SynFuBenBossInfo Rsp;

	Rsp.m_Num = 0;
	OBuffer1k ob;

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(pSynCombatCnfg->m_SynFuBenID);

	if( 0 != pFuBenCnfg)
	{
		for( int i = 0; i < pFuBenCnfg->m_MapID.size(); ++i)
		{
			TMapID	MapID = pFuBenCnfg->m_MapID[i];

			const std::vector<SMonsterOutput> * pvecMonster = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);

			if ( 0 == pvecMonster)
			{
				continue;
			}

			for ( int i = 0; i < (*pvecMonster).size(); ++i)
			{
				const SMonsterOutput & MonsterOutput =(*pvecMonster)[i];

				if ( MonsterOutput.m_MonsterType != enMonsterType_Boss)
				{
					continue;
				}

					
				TMonsterID BossID = MonsterOutput.m_MonsterID;

				const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(BossID);

				if ( 0 == pMonsterCnfg)
				{
					TRACE("<error> %s : %d LINE 获取怪物配置信息出错！！怪物ID = %d", __FUNCTION__, __LINE__, BossID);
					continue;
				}

				SynFuBenBoss BSInfo;

				BSInfo.DropID = g_pGameServer->GetConfigServer()->GetMapBossDropID(MapID);
				strncpy(BSInfo.m_BossName, pMonsterCnfg->m_szName, sizeof(BSInfo.m_BossName));

				++Rsp.m_Num;

				ob << BSInfo;
			}
		}
	}

	 OBuffer1k ob2;
	 ob2 << SyndicateHeader(enSyndicateCmd_SynFuBenBossInfo,SIZE_OF(Rsp) + ob.Size()) << Rsp;

	 if( ob.Size() > 0){
		ob2 << ob;
	 }

	 pActor->SendData(ob2.TakeOsb());
}

//得到帮战开始剩余时间
UINT32	SyndicateMgr::GetRemainStartSynCombat()
{
	return m_SynWar.GetNextOpenWarTimeNum();
}

//得到敌对帮派ID
TSynID SyndicateMgr::GetVsSynID(IActor * pActor)
{
	return m_SynWar.GetVsSynID(pActor);
}

//得到敌对帮派名
std::string SyndicateMgr::GetVsSynName(IActor * pActor) 
{
	return m_SynWar.GetVsSynName(pActor);
}
