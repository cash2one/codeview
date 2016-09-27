
#include "ActivityMgr.h"
#include "IActivityPart.h"
#include "ThingServer.h"
#include "IActor.h"
#include "IBasicService.h"
#include "EventActivity.h"
#include "TargetTimeToLevel.h"
#include "DBProtocol.h"

ActivityMgr::ActivityMgr()
{
}
	
	
ActivityMgr::~ActivityMgr()
{
}
		

bool ActivityMgr::Create()
{
	SDB_Get_ActivityCnfg_Req DBReq;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(20, enDBCmd_Get_ActivityCnfg, ob.TakeOsb(), this, 0);

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Activity,this);
}


void ActivityMgr::Close()
{
	for(int i=0; i < m_vectActivity.size(); i++)
	{
		m_vectActivity[i]->Release();
	}

	m_vectActivity.clear();

	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Activity,this);
}

		//收到MSG_ROOT消息
void ActivityMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (ActivityMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	/*
	   enActivityCmd_OpenDaily = 0  ,        //打开日常
	   enActivityCmd_OpenSignIn         ,   //打开签到
	   enActivityCmd_SignIn         ,       //签到
	   enActivityCmd_SignInAward,           //领取签到奖励	
	   enActivityCmd_OpenActivity       ,   //打开活动
	   enActivityCmd_UpdateActivity     ,   //更新活动状态
	   enActivityCmd_ActivityAward,         //领取活动奖励
	*/

	static FUNC_PROC s_funcProc[enActivityCmd_Max] = 
	{
		&ActivityMgr::OpenDaily,
		&ActivityMgr::OpenSignIn,
		&ActivityMgr::SignIn,
		&ActivityMgr::SignInAward,
		&ActivityMgr::OpenActivity,
		NULL,
		NULL,
		//&ActivityMgr::ActivityAward,
		&ActivityMgr::OnlineAwardNotic,                           //通知领取在线奖励
		&ActivityMgr::TakeOnlineAward,    //领取在线奖励
		&ActivityMgr::ChangeTicket,    //礼券兑换奖励

	};

	if(nCmd >= ARRAY_SIZE(s_funcProc) || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}



void ActivityMgr::OpenDaily(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	pActivityPart->OpenDialy();

}

void ActivityMgr::OpenSignIn(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	pActivityPart->OpenSignIn();
}


void ActivityMgr::SignIn(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	pActivityPart->SignIn();
}

void ActivityMgr::SignInAward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	CS_ActivityTakeSignInAward_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pActivityPart->TakeSignInAward(Req.m_AwardID);

}

void ActivityMgr::OpenActivity(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	pActivityPart->OpenActivity();
}

void ActivityMgr::ActivityAward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	CS_ActivityTakeActivityAward_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pActivityPart->TakeActivityAward(Req.m_ActivityID);

}

void ActivityMgr::ChangeTicket(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ActivityChangeGift_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 切磋客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	DB_ChangeTicket_Req ChangeReq;

	strncpy(ChangeReq.m_szTicketNo,Req.m_szTicketNo,sizeof(ChangeReq.m_szTicketNo)-1);
	ChangeReq.m_ServerID = g_pGameServer->GetServerID();
	ChangeReq.m_UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);

	OBuffer1k ob;

	ob << ChangeReq;

	g_pGameServer->GetDBProxyClient()->Request(ChangeReq.m_UserID,enDBCmd_ChangeTicket,ob.TakeOsb(),this,pActor->GetUID().ToUint64());

}

void ActivityMgr::TakeOnlineAward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}
	
	pActivityPart->TakeOnlineAward();
}

void ActivityMgr::OnlineAwardNotic(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();

	if(pActivityPart == 0)
	{
		return ;
	}

	pActivityPart->OnlineAwardNotic();
}

void ActivityMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_Get_ActivityCnfg:
			{
				HandleGetActivityCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   case enDBCmd_ChangeTicket:
		   {
			   HandleChangeTicket(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetActivityCnfg:
			{
				HandleGetOneActivityCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_ChangeTicketFinish:
		   {
		   }
		   break;
	   default:
			break;
	}
}


//删除活动
void	ActivityMgr::DeleteActivity(UINT16	ActivityID)
{
	std::vector<IActivity*>::iterator iter = m_vectActivity.begin();

	for ( ; iter != m_vectActivity.end(); ++iter )
	{
		IActivity * pActivity = *iter;

		if ( 0 == pActivity )
			continue;

		const  SActivityCnfg * pActivityCnfg = pActivity->GetActivityCnfg();

		if ( 0 == pActivityCnfg )
			continue;

		if ( pActivityCnfg->m_ActivityID == ActivityID )
		{
			pActivity->Release();
			m_vectActivity.erase(iter);
			break;
		}
	}

	g_pGameServer->GetConfigServer()->DelActivityCnfg(ActivityID);
}

//添加活动
void	ActivityMgr::AddActivity(UINT16	ActivityID)
{
	SDB_GetActivityCnfgReq DBReq;

	DBReq.ActivityID = ActivityID;

	OBuffer1k DBOb;
	DBOb << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(ActivityID,enDBCmd_GetActivityCnfg,DBOb.TakeOsb(),this,0);
}

//修改活动
void	ActivityMgr::ChangeActivity(UINT16 ActivityID)
{
	//先删除这活动，再从数据库中重新创建活动
	this->DeleteActivity(ActivityID);

	this->AddActivity(ActivityID);
}

void ActivityMgr::HandleGetActivityCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if (RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if (OutParam.retCode != enDBRetCode_OK){
		return;
	}

	INT32 Num = RspIb.Remain() / sizeof(SDB_ActivityCnfg);

	for ( int i = 0; i < Num; ++i)
	{
		SDB_ActivityCnfg DBActivityCnfg;

		RspIb >> DBActivityCnfg;

		if (RspIb.Error()){

			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SActivityCnfg ActivityCnfg;
		
		ActivityCnfg.m_ActivityID = DBActivityCnfg.m_ActivityID;
		ActivityCnfg.m_ActivityType = DBActivityCnfg.m_ActivityType;
		ActivityCnfg.m_AttainNum	= DBActivityCnfg.m_AttainNum;
		ActivityCnfg.m_BeginTime	= DBActivityCnfg.m_BeginTime;
		ActivityCnfg.m_bFinished	= DBActivityCnfg.m_bFinished;
		ActivityCnfg.m_EndTime		= DBActivityCnfg.m_EndTime;
		ActivityCnfg.m_EventID		= DBActivityCnfg.m_EventID;
		ActivityCnfg.m_GodStone		= DBActivityCnfg.m_GodStone;
		ActivityCnfg.m_ResID		= DBActivityCnfg.m_ResID;
		ActivityCnfg.m_Order        = DBActivityCnfg.m_Order;
		ActivityCnfg.m_strAwardDesc = (char *)DBActivityCnfg.m_szAwardDesc;
		strncpy(ActivityCnfg.m_strContent, DBActivityCnfg.m_szMailContent, sizeof(ActivityCnfg.m_strContent));
		ActivityCnfg.m_strName		= (char *)DBActivityCnfg.m_szName;
		ActivityCnfg.m_strRole		= (char *)DBActivityCnfg.m_szRole;
		//ActivityCnfg.m_strResFileUrl = (char *)DBActivityCnfg.m_szResFileUrl;
		strncpy(ActivityCnfg.m_strSubject, DBActivityCnfg.m_szMailSubject, sizeof(ActivityCnfg.m_strSubject));
		ActivityCnfg.m_Ticket		= DBActivityCnfg.m_Ticket;
		this->GetActivityParam((char *)DBActivityCnfg.m_vectGoods, ActivityCnfg.m_vectGoods);
		this->GetActivityParam((char *)DBActivityCnfg.m_vectParam, ActivityCnfg.m_vectParam);

		g_pGameServer->GetConfigServer()->Push_ActivityCnfg(ActivityCnfg);

		//创建
		this->CreateActivity(ActivityCnfg);
	}
}

void ActivityMgr::HandleChangeTicket(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
				OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	DB_TicketInfo TicketInfo;

	RspIb >> RspHeader >> OutParam >> TicketInfo;

	if (RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));

	SC_ActivityChangeGift_Rsp Rsp;

	if (OutParam.retCode == 1){
		Rsp.m_Result = enActivityRetCode_ErrTicketNo;
	}
	else if(OutParam.retCode == 2)
	{
		Rsp.m_Result = enActivityRetCode_ErrHaveChange;
	}
	else if(OutParam.retCode == 3)
	{
		Rsp.m_Result = enActivityRetCode_ErrNoActive;
	}
	else if(OutParam.retCode == 4)
	{
		Rsp.m_Result = enActivityRetCode_ErrDeadtime;
	}
	else if(OutParam.retCode == 5)
	{
		Rsp.m_Result = enActivityRetCode_ErrTaked;
	}
	else
	{
		Rsp.m_Result = enActivityRetCode_Ok;

		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(TicketInfo.m_GoodsID);

		if(pGoodsCnfg==0)
		{
			return;
		}

		//发送邮件
		SWriteSystemData EmailData;
		EmailData.m_DestUID = UID(userdata);
		EmailData.m_Money = TicketInfo.m_MoneyNum;
		EmailData.m_Stone = TicketInfo.m_StoneNum;
		EmailData.m_Ticket = TicketInfo.m_TicketNum;
		EmailData.m_PolyNimbus = TicketInfo.m_PolyNimbusNum;

		strncpy(EmailData.m_szThemeText,(const char *)TicketInfo.m_szMailTitle,sizeof(EmailData.m_szThemeText));
		//sprintf_s(EmailData.m_szContentText,sizeof(EmailData.m_szContentText),g_pGameServer->GetGameWorld()->GetLanguageStr(10086),(const char*)TicketInfo.m_szTicketNo,
			//g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID),TicketInfo.m_Number);

		strncpy(EmailData.m_szContentText, (const char *)TicketInfo.m_szMailContent, sizeof(EmailData.m_szContentText));

		std::vector<IGoods*> vectGoods = g_pGameServer->GetGameWorld()->CreateGoods(TicketInfo.m_GoodsID,TicketInfo.m_Number,true);

		g_pGameServer->GetGameWorld()->WriteSystemMail(EmailData,vectGoods);


		DB_ChangeTicketFinish_Req FinishReq;
		strncpy(FinishReq.m_szTicketNo,TicketInfo.m_szTicketNo,sizeof(FinishReq.m_szTicketNo));

		OBuffer1k DBOb;
		DBOb << FinishReq;

		g_pGameServer->GetDBProxyClient()->Request(DBUserID,enDBCmd_ChangeTicketFinish,DBOb.TakeOsb(),0,0);

	}

	if(pActor != 0)
	{
	    OBuffer1k ob;
	    ob << ActivityHeader(enActivityCmd_ChangeGift,sizeof(Rsp)) << Rsp;

		pActor->SendData(ob.TakeOsb());
	}

}

bool ActivityMgr::CreateActivity(const SActivityCnfg & ActivityCnfg)
{
	if(ActivityCnfg.m_bFinished != 0){

		return false;
	}

	IActivity * pActivity = 0;

	switch(ActivityCnfg.m_ActivityType)
	{
	case enActivityType_Event:
		{
			pActivity  = new EventActivity();
		}
		break;
	case enActivityType_TargetTimeToLevel:
		{
			pActivity  = new TargetTimeToLevel();
		}
		break;
	default:
		TRACE("<error> %s : %d 行 无效的类型！！类型ID = %d", __FUNCTION__, __LINE__, ActivityCnfg.m_ActivityType);
		return false;
	}
	
	if(pActivity == 0 || pActivity->Create(&ActivityCnfg)==false)
	{
		return false;
	}

	if(pActivity)
	{
	   m_vectActivity.push_back(pActivity);
	}

	return true;
}

//得到参数
void ActivityMgr::GetActivityParam(char * pszParam, std::vector<INT32> & vectParam)
{
	char * pszParam2 = pszParam;

	for( ; *pszParam; )
	{
		if( *pszParam == ','){
			*pszParam = '\0';

			vectParam.push_back(atoi(pszParam2));

			++pszParam;

			pszParam2 = pszParam;
		}else{
			++pszParam;
		}	
	}

	//验证最后一项
	if( atoi(pszParam2) == 0){
		return;
	}

	vectParam.push_back(atoi(pszParam2));
}

void ActivityMgr::HandleGetOneActivityCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if (RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if (OutParam.retCode != enDBRetCode_OK){
		return;
	}

	SDB_ActivityCnfg DBActivityCnfg;

	RspIb >> DBActivityCnfg;

	if (RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SActivityCnfg ActivityCnfg;
	
	ActivityCnfg.m_ActivityID = DBActivityCnfg.m_ActivityID;
	ActivityCnfg.m_ActivityType = DBActivityCnfg.m_ActivityType;
	ActivityCnfg.m_AttainNum	= DBActivityCnfg.m_AttainNum;
	ActivityCnfg.m_BeginTime	= DBActivityCnfg.m_BeginTime;
	ActivityCnfg.m_bFinished	= DBActivityCnfg.m_bFinished;
	ActivityCnfg.m_EndTime		= DBActivityCnfg.m_EndTime;
	ActivityCnfg.m_EventID		= DBActivityCnfg.m_EventID;
	ActivityCnfg.m_GodStone		= DBActivityCnfg.m_GodStone;
	ActivityCnfg.m_ResID		= DBActivityCnfg.m_ResID;
	ActivityCnfg.m_Order        = DBActivityCnfg.m_Order;
	ActivityCnfg.m_strAwardDesc = (char *)DBActivityCnfg.m_szAwardDesc;
	strncpy(ActivityCnfg.m_strContent, DBActivityCnfg.m_szMailContent, sizeof(ActivityCnfg.m_strContent));
	ActivityCnfg.m_strName		= (char *)DBActivityCnfg.m_szName;
	ActivityCnfg.m_strRole		= (char *)DBActivityCnfg.m_szRole;
	//ActivityCnfg.m_strResFileUrl = (char *)DBActivityCnfg.m_szResFileUrl;
	strncpy(ActivityCnfg.m_strSubject, DBActivityCnfg.m_szMailSubject, sizeof(ActivityCnfg.m_strSubject));
	ActivityCnfg.m_Ticket		= DBActivityCnfg.m_Ticket;
	this->GetActivityParam((char *)DBActivityCnfg.m_vectGoods, ActivityCnfg.m_vectGoods);
	this->GetActivityParam((char *)DBActivityCnfg.m_vectParam, ActivityCnfg.m_vectParam);

	g_pGameServer->GetConfigServer()->Push_ActivityCnfg(ActivityCnfg);

	//创建
	this->CreateActivity(ActivityCnfg);
}
