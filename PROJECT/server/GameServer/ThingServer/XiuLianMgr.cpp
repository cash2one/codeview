
#include "XiuLianMgr.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IXiuLianPart.h"
#include "IActor.h"
#include "XDateTime.h"
#include "IGameWorld.h"
#include "IMagicPart.h"
#include "IFriendPart.h"
#include "IConfigServer.h"
#include "DMsgSubAction.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"

XiuLianMgr::XiuLianMgr()
{
}

XiuLianMgr::~XiuLianMgr()
{
}

bool XiuLianMgr::Create()
{
	if( g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_XiuLian,this)==false)
	{
		return false;
	}
	//获取修炼每几分钟更新一次
	UINT16 UpdateMinuteNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_UpdateMinuteNum;

	g_pGameServer->GetTimeAxis()->SetTimer(enXiuLianTimerID,this, UpdateMinuteNum * 60 * 1000,"XiuLianMgr::Create[enXiuLianTimerID]");

	return true;
}

void XiuLianMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_XiuLian,this);
}


// 前置机回调
// nRetCode: 取值于 enDBRetCode
void XiuLianMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
						 OStreamBuffer & ReqOsb,UINT64 userdata) 
{
	if(nRetCode != enDBRetCode_OK){
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0){
		return ;
	}

	pXiuLianPart->OnDBRet(userID,ReqCmd,nRetCode,RspOsb,ReqOsb,userdata);
}


//收到MSG_ROOT消息
void XiuLianMgr::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	typedef  void (XiuLianMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enXiuLianCmd_Max]=
	{		
		& XiuLianMgr::OpenXiuLian,
		NULL,	
		& XiuLianMgr::AskAloneXiuLian,
		& XiuLianMgr::CancelAloneXiuLian,

		NULL,
		NULL,
		& XiuLianMgr::AskTwoXiuLian,
		& XiuLianMgr::CancelTwoXiuLian,
		NULL,

		NULL,
		NULL,
		NULL,
		NULL,
		NULL,

		NULL,
		NULL,
		NULL,

		& XiuLianMgr::GetEffectXLElement,
		& XiuLianMgr::GetInXiuLianActorUID,

	};

	if(nCmd>=enXiuLianCmd_Max || 0==s_funcProc[nCmd])
	{
		TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}

//打开
void  XiuLianMgr::OpenXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{	
	CS_OpenXiuLian_Req Req;

	ib >> Req;

	if( ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if( 0 == pXiuLianPart)
	{
		return ;
	}

	pXiuLianPart->Open(Req);	
}

//请求独自修炼
void  XiuLianMgr::AskAloneXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_AlongXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	const UID* pActorUid = (const UID*)ib.CurrentBuffer();

	if(Req.m_ActorNum * sizeof(UID)>ib.Remain())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->AloneXiuLian(&Req,pActorUid);	
}

//取消独自修炼
void  XiuLianMgr::CancelAloneXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{	
	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->CancelAloneXiuLian();	
}

//请求双修
void  XiuLianMgr::AskTwoXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_TwoXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->TwoXiuLian(Req);	
}

//取消双修
void  XiuLianMgr::CancelTwoXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->CancelTwoXiuLian();	
}

//请求修炼法术
void  XiuLianMgr::AskMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_MagicXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->MagicXiuLian(Req);	
}

//接受修炼法术
void  XiuLianMgr::AcceptMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_AcceptMagicXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->AcceptMagicXiuLian(Req);	
}

//拒绝修炼法术
void  XiuLianMgr::RejectMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_RejectMagicXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->RejectMagicXiuLian(Req);	
}

//取消修炼法术
void  XiuLianMgr::CancelMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_CancelMagicXiuLian_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->CancelMagicXiuLian(Req);	
}

//查看请求数据
void XiuLianMgr::ViewAskXiuLianData(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	//CS_ViewXiuLianAskData_Req Req;

	//ib >> Req;

	//if(ib.Error())
	//{
	//	TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
	//	return ;
	//}

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if(pXiuLianPart==0)
	{
		return ;
	}

	pXiuLianPart->ViewAskData(/*Req*/);	
}

//取消修炼请求
void	XiuLianMgr::CancelAskSeq(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if( 0 == pXiuLianPart){
		return;
	}

	CS_CancelAskSeq_Req Req;
	ib >> Req;

	STwoXiuLianData * pTwoXiuLianData = this->GetAskXiuLianData(Req.m_AskSeqID);
	if( 0 == pTwoXiuLianData){
		return;
	}

	if( pTwoXiuLianData->m_XiuLianType == enXiuLianType_Magic){
		pXiuLianPart->CancelMagicAsk();
	}

	SC_CancelAskSeq_Rsp Rsp;
	Rsp.m_Result = enXiuLianRetCode_OK;

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_CancelAskData,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());
}

//得到影响修炼的元素
void XiuLianMgr::GetEffectXLElement(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	SC_EffectXLElement Rsp;

	ISyndicate * pSyndicate = pActor->GetSyndicate();

	if ( 0 != pSyndicate){
		
		Rsp.m_AddAloneXLPosNum = pSyndicate->GetWelfareValue(enWelfare_AloneXiuLianAddXiuLianActor);

		Rsp.m_SynWelfareHour   = pSyndicate->GetWelfareValue(enWelfare_XiuLianTime);

		Rsp.m_DesNum		   = pSyndicate->GetWelfareValue(enWelfare_ReduceXiuLianMoney);
	}

	Rsp.m_VipAddPosNum = pActor->GetVipValue(enVipType_AddXiuLianPos);

	Rsp.m_VipTakeResAdd = pActor->GetVipValue(enVipType_AddXLNimbus);

	OBuffer1k ob;
	ob << XiuLian_Header(enXiuLianCmd_EffectXLElement,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//得到正在修炼中的角色UID
void XiuLianMgr::GetInXiuLianActorUID(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if( 0 == pXiuLianPart){
		return;
	}

	pXiuLianPart->GetInXiuLianActorUID();
}

//增加一个修炼请求
bool  XiuLianMgr::AddAskXiuLianData(STwoXiuLianData & XiuLianData)
{
	m_AskXiuLianData[XiuLianData.m_AskSeq] = XiuLianData;
	return true;
}


//删除一个修炼请求
bool  XiuLianMgr::DelAskXiuLianData(UINT32 AskSeq)
{
	m_AskXiuLianData.erase(AskSeq);
	return true;
}

//根据序列号获得请求数据
STwoXiuLianData * XiuLianMgr::GetAskXiuLianData(UINT32 AskSeq)
{
	MAP_ASK_DATA::iterator it = m_AskXiuLianData.find(AskSeq);
	if(it != m_AskXiuLianData.end())
	{
		return &(*it).second;
	}
	return 0;
}

 //玩家上线时，把未加到玩家身上的请求加给玩家
std::vector<UINT32> XiuLianMgr::Check_AddToAskList(IActor * pActor)
{
	std::vector<UINT32> vectAskSeq;

	IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();
	if( 0 == pXiuLianPart){
		return vectAskSeq;
	}

	UID uid_User = pActor->GetUID();

	for(MAP_ASK_DATA::iterator iter = m_AskXiuLianData.begin(); iter != m_AskXiuLianData.end(); ++iter)
	{
		STwoXiuLianData & TwoXiuLianData = iter->second;

		if( (TwoXiuLianData.m_uidSource == uid_User || TwoXiuLianData.m_uidFriend == uid_User) && !pXiuLianPart->IsInAskList(TwoXiuLianData.m_AskSeq)){
			vectAskSeq.push_back(iter->first);
		}
	}

	return vectAskSeq;
}

void XiuLianMgr::MagicXiuLianOnTimer(STwoXiuLianData * pAskXiuLianData)
{

	time_t nCurTime = CURRENT_TIME();
	if(pAskXiuLianData->m_XiuLianState == enXiuLianState_Doing)
	{			
		if(nCurTime >= pAskXiuLianData->m_EndTime)
		{
			//结束了
			pAskXiuLianData->m_XiuLianState = enXiuLianState_Finish;

			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pAskXiuLianData->m_uidSource);
			IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(pAskXiuLianData->m_uidFriend);

			pAskXiuLianData->m_bStudyMagic = true;

			pAskXiuLianData->m_bUpdate = true;

			if(pActor)
			{
				IXiuLianPart * pXiuLianPart = pActor->GetXiuLianPart();

				pXiuLianPart->StopMagicXiuLian(); 

				if(pAskXiuLianData->m_Mode == enXiuLianMode_Study)
				{
					IMagicPart * pMagicPart = pActor->GetMagicPart();
					if(pMagicPart)
					{
						pMagicPart->StudyMagic(pAskXiuLianData->m_MagicID);
						pAskXiuLianData->m_bStudyMagic = false;
					}
				}
			}

			if(pFriend)
			{
				IXiuLianPart * pXiuLianPart = pFriend->GetXiuLianPart();

				pXiuLianPart->StopMagicXiuLian(); 

				if(pAskXiuLianData->m_Mode == enXiuLianMode_Teach)
				{
					IMagicPart * pMagicPart = pFriend->GetMagicPart();
					if(pMagicPart)
					{
						pMagicPart->StudyMagic(pAskXiuLianData->m_MagicID);
						pAskXiuLianData->m_bStudyMagic = false;
					}
				}
			}

			////修炼完成，发布事件
			//SS_XiuLianFinish XiuLianFinish;
			//XiuLianFinish.m_XiuLianType = enXiuLianType_Magic;

			//UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_XiuLian);

			//if( 0 != pActor){
			//	pActor->OnEvent(msgID,&XiuLianFinish,sizeof(XiuLianFinish));	
			//}
			//if( 0 != pFriend){
			//	pFriend->OnEvent(msgID,&XiuLianFinish,sizeof(XiuLianFinish));
			//}
		}
	}	
}

void  XiuLianMgr::OnTimer(UINT32 timerID)
{
	time_t nCurTime = CURRENT_TIME();

	if(enXiuLianTimerID == timerID)
	{
		for(MAP_ASK_DATA::iterator it = m_AskXiuLianData.begin(); it != m_AskXiuLianData.end(); )
		{
			STwoXiuLianData * pAskXiuLianData = &(*it).second;

			if(pAskXiuLianData->m_UnloadTime>0)
			{
				if(nCurTime > pAskXiuLianData->m_UnloadTime)
				{					
			          IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pAskXiuLianData->m_uidSource);
			          IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(pAskXiuLianData->m_uidFriend);
					  if(pActor !=0 || pFriend!=0)
					  {
						  pAskXiuLianData->m_UnloadTime = 0;
					  }
					  else
					  {
					     m_AskXiuLianData.erase(it++);
						 continue;
					  }
				}
			}
			else
			{
				if(pAskXiuLianData->m_XiuLianType == enXiuLianType_Magic)
				{
					MagicXiuLianOnTimer(pAskXiuLianData);
				}
			}

			++it;

		}
	}
}

//保存数据到数据库
void XiuLianMgr::SaveToDB(UINT32 AskSeq)
{
	return SaveToDB(GetAskXiuLianData(AskSeq));
}

//延时卸载修炼数据
void XiuLianMgr::UnloadXiuLianRecord(UINT32 AskSeq)
{
	STwoXiuLianData * pAskXiuLianData = GetAskXiuLianData(AskSeq);

	if(pAskXiuLianData==0)
	{
		return;
	}

	SaveToDB(pAskXiuLianData);

	UINT16 BackUnloadXiuLianRecordTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_BackUnloadXiuLianRecordTime;

	pAskXiuLianData->m_UnloadTime = CURRENT_TIME()+BackUnloadXiuLianRecordTime;
}

void XiuLianMgr::SaveToDB(STwoXiuLianData * pAskXiuLianData)
{
	if(pAskXiuLianData==0 || pAskXiuLianData->m_bUpdate==false)
	{
		return;
	}

	SDB_Update_XiuLianData_Record_Req XiuLianData_Record;

	XiuLianData_Record.AskSeq = pAskXiuLianData->m_AskSeq;

	XiuLianData_Record.AskSeq = pAskXiuLianData->m_AskSeq;
	XiuLianData_Record.AskTime = pAskXiuLianData->m_AskTime;
	XiuLianData_Record.EndTime = pAskXiuLianData->m_EndTime;
	strncpy(XiuLianData_Record.FriendName, pAskXiuLianData->m_szFriendName,SIZE_OF(XiuLianData_Record.FriendName));
	XiuLianData_Record.Hours = pAskXiuLianData->m_Hours;
	XiuLianData_Record.LastGetNimbusTime = pAskXiuLianData->m_lastGetNimbusTime;
	XiuLianData_Record.m_bStudyMagic = pAskXiuLianData->m_bStudyMagic;
	XiuLianData_Record.m_FriendNotTakeNimbus = pAskXiuLianData->m_FriendNotTakeNimbus;
	XiuLianData_Record.m_FriendTotalNimbus = pAskXiuLianData->m_FriendTotalNimbus;
	XiuLianData_Record.MagicID = pAskXiuLianData->m_MagicID;
	XiuLianData_Record.m_SourceNotTakeNimbus = pAskXiuLianData->m_SourceNotTakeNimbus;
	XiuLianData_Record.Mode = pAskXiuLianData->m_Mode;
	strncpy(XiuLianData_Record.SourceName, pAskXiuLianData->m_szSourceName,SIZE_OF(XiuLianData_Record.SourceName));
	XiuLianData_Record.TotalNimbus = pAskXiuLianData->m_TotalNimbus;
	XiuLianData_Record.UidFriend = pAskXiuLianData->m_uidFriend.ToUint64();
	XiuLianData_Record.UidFriendActor = pAskXiuLianData->m_uidFriendActor.ToUint64();
	XiuLianData_Record.UidSource = pAskXiuLianData->m_uidSource.ToUint64();
	XiuLianData_Record.UidSourceActor = pAskXiuLianData->m_uidSourceActor.ToUint64();
	XiuLianData_Record.XiuLianState = pAskXiuLianData->m_XiuLianState;
	XiuLianData_Record.XiuLianType = pAskXiuLianData->m_XiuLianType;
	XiuLianData_Record.m_FriendLastGetNimbusTime = pAskXiuLianData->m_FriendLastGetNimbusTime;

	XiuLianData_Record.m_SourceLayer = pAskXiuLianData->m_SourceLayer;
	XiuLianData_Record.m_SourceNimbusSpeed = pAskXiuLianData->m_SourceNimbusSpeed;
	XiuLianData_Record.m_FriendLayer = pAskXiuLianData->m_FriendLayer;
	XiuLianData_Record.m_FriendNimbusSpeed = pAskXiuLianData->m_FriendNimbusSpeed;

	OBuffer1k ob;
	ob << XiuLianData_Record;

	g_pGameServer->GetDBProxyClient()->Request(0,enDBCmd_UpdateXiuLianRecord,ob.TakeOsb(),0,0);

}
