#include "IActor.h"
#include "TeamMgr.h"
#include "ITeamPart.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "ICombatPart.h"
#include "IFuBenPart.h"
#include "ISyndicate.h"

TeamMgr::TeamMgr()
{

}

bool TeamMgr::Create()
{
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Team,this);
}

void TeamMgr::Close()
{

}

//收到MSG_ROOT消息
void TeamMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (TeamMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enTeamCmd_Max] = 
	{
		&TeamMgr::InviteTeam,
		NULL,
		&TeamMgr::AgreeTeam,
		&TeamMgr::RefuseTeam,
		NULL,
		&TeamMgr::QuitTeam,
		NULL,
		&TeamMgr::QuitWaitOrTimeEnd,
		&TeamMgr::SetTeamLineup,
		NULL,
		&TeamMgr::OpenTeamInfo,
		&TeamMgr::SetTeamCombatState,
		NULL,
	};

	if(nCmd >= ARRAY_SIZE(s_funcProc) || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

//取消等待
void	TeamMgr::QuitWait(IActor * pActor)
{
	UID uidUser = pActor->GetUID();

	MAP_FASTTEAM::iterator iter = m_WaitFastTeam.begin();

	for ( ; iter != m_WaitFastTeam.end(); ++iter ){
		
		if ( uidUser == iter->second ){
			
			m_WaitFastTeam.erase(iter);

			//把玩家设成不在等待状态
			ITeamPart * pTeamPart = pActor->GetTeamPart();

			if ( 0 == pTeamPart )
				break;

			pTeamPart->SetIsInFastTeam(false);
			break;
		}
	}
}

//邀请组队
void	TeamMgr::InviteTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	CS_InviteTeam Req;

	ib >> Req;

	if ( ib.Error() ){

		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTeamPart->InviteTeam(Req.m_uidFriend, enCreateTeamType_Point);	
}

//同意组队
void	TeamMgr::AgreeTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	CS_AgreeTeam Req;

	ib >> Req;

	if ( ib.Error() ){

		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTeamPart->AgreeTeam(Req.m_uidUser);
}

//拒绝组队
void	TeamMgr::RefuseTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	CS_RefuseTeam Req;

	ib >> Req;

	if ( ib.Error() ){

		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pTeamPart->RefuseTeam(Req.m_uidUser);
}

//退出队伍
void	TeamMgr::QuitTeam(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	pTeamPart->QuitTeam();	
}

//副本快速组队
void	TeamMgr::FuBenFastCreateTeam(IActor *pActor, TFuBenID FuBenID)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	SC_FuBen_FastTeam Rsp;
	
	do
	{
		MAP_FASTTEAM::iterator iter = m_WaitFastTeam.begin();

		for ( ; iter != m_WaitFastTeam.end(); ++iter )
		{
			if ( pActor->GetUID() == iter->second && FuBenID != iter->first)
			{
				//只能快速组队一个副本
				Rsp.m_RetCode = enTeamRetCode_NotTwoFast;
				break;
			}
		}

		if ( Rsp.m_RetCode != enTeamRetCode_Ok )
		{
			break;
		}

		iter = m_WaitFastTeam.find(FuBenID);

		if ( iter == m_WaitFastTeam.end() )
		{
			//进入等待
			Rsp.m_RetCode = enTeamRetCode_Wait;

			m_WaitFastTeam[FuBenID] = pActor->GetUID();

			pTeamPart->SetIsInFastTeam(true);

			break;
		}

		UID uidUser = iter->second;

		if ( uidUser == pActor->GetUID() )
		{
			//已经操作了快速组队
			Rsp.m_RetCode = enTeamRetCode_ErrHaveFast;
			break;
		}

		Rsp.m_RetCode = (enTeamRetCode)pTeamPart->CreateTeam(uidUser, true);

		if ( Rsp.m_RetCode == enTeamRetCode_Ok )
		{
			//进入组队副本
			IFuBenPart * pFuBenPart = pActor->GetFuBenPart();

			if ( 0 == pFuBenPart )
				return;

			pFuBenPart->TeamChallengeFuBen(FuBenID);
		}
		else if ( Rsp.m_RetCode == enTeamRetCode_OffLine || Rsp.m_RetCode == enTeamRetCode_InBusy 
			|| Rsp.m_RetCode == enTeamRetCode_OtExist)
		{
			//进入等待
			Rsp.m_RetCode = enTeamRetCode_Wait;

			m_WaitFastTeam[FuBenID] = pActor->GetUID();

			IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(uidUser);

			if ( 0 != pUser )
			{
				ITeamPart * pUserTeamPart = pUser->GetTeamPart();

				if ( 0 != pUserTeamPart )
				{
					pUserTeamPart->SetIsInFastTeam(false);
				}
			}
			
			pTeamPart->SetIsInFastTeam(true);
		}

	}while(0);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SC_FuBen_FastTeam, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//退出等待或者等待时间到
void	TeamMgr::QuitWaitOrTimeEnd(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	SC_QuitWaitOrTimeEnd Rsp;

	this->QuitWait(pActor);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_QuitWaitOrTimeEnd, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//设置组队阵形
void	TeamMgr::SetTeamLineup(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if ( 0 == pCombatPart ){
		
		return;
	}

	CS_SetTeamLineup Req;

	ib >> Req;

	if ( ib.Error() ){

		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_SetTeamLineup Rsp;

	Rsp.m_RetCode = (enTeamRetCode)pCombatPart->SetTeamLineup(Req, true);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SetTeamLineup, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//打开队伍信息
void	TeamMgr::OpenTeamInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	SC_OpenTeamInfo Rsp;

	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		
		return;
	}

	IActor * pMember = pTeamPart->GetTeamMember();

	if ( 0 == pMember ){
		
		Rsp.m_RetCode = enTeamRetCode_NoTeam;
	} else {
		
		SFuBenNum FuBenNum;

		IFuBenPart * pFuBenPart = pActor->GetFuBenPart();

		if ( 0 == pFuBenPart ){
			
			return;
		}

		//ICombatPart * pCombatPart = pActor->GetCombatPart();

		//if ( 0 == pCombatPart )
		//	return;

		//pCombatPart->SycTeamLineup();

		pFuBenPart->GetFuBenNum(FuBenNum);

		Rsp.m_FreeNum	= FuBenNum.m_FreeNum;
		Rsp.m_StoneNum	= FuBenNum.m_StoneNum;
		Rsp.m_SynWelNum = FuBenNum.m_SynWelNum;
		Rsp.m_VipNum	= FuBenNum.m_VipNum;
		//Rsp.m_MaxSynWelNum 

		ISyndicate * pSyn = pActor->GetSyndicate();

		if ( 0 != pSyn ){
			Rsp.m_MaxSynWelNum = pSyn->GetWelfareValue(enWelfare_EnterFuBenNum);
		}

		pFuBenPart = pMember->GetFuBenPart();

		if ( 0 == pFuBenPart ){
			
			return;
		}

		pFuBenPart->GetFuBenNum(FuBenNum);

		Rsp.m_MemFreeNum	= FuBenNum.m_FreeNum;
		Rsp.m_MemStoneNum	= FuBenNum.m_StoneNum;
		Rsp.m_MemSynWelNum	= FuBenNum.m_SynWelNum;
		Rsp.m_MemVipNum		= FuBenNum.m_VipNum;

		pSyn = pMember->GetSyndicate();

		if ( 0 != pSyn ){
			Rsp.m_MaxMemSynWelNum = pSyn->GetWelfareValue(enWelfare_EnterFuBenNum);
		}
	}

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_OpenTeamInfo, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//设置组队参战状态
void	TeamMgr::SetTeamCombatState(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_SetTeamCombatState Req;

	ib >> Req;

	if ( ib.Error() ){

		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IActor * pTarget = pActor;
	
	if ( pActor->GetUID() != Req.m_uidActor ){
		
		pTarget = pActor->GetEmployee(Req.m_uidActor);
	}

	if ( 0 == pTarget )
		return;

	ICombatPart * pCombatPart = pTarget->GetCombatPart();

	if ( 0 == pCombatPart ){
		return;
	}

	SC_SetTeamCombatState Rsp;

	Rsp.m_RetCode = (enTeamRetCode)pCombatPart->SetTeamCombatStatus(Req.m_bTeamCombat);

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SetTeamCombatState, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}
