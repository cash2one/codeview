
#include "IActor.h"

#include "TrainingHallPart.h"

#include "DBProtocol.h"
#include "XDateTime.h"
#include "GameSrvProtocol.h"
#include "ThingServer.h"
#include "DMsgSubAction.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IGameScene.h"
#include "IBasicService.h"

TrainingHallPart::TrainingHallPart()
{
	m_pActor = 0;

	m_TrainingHallData.m_RemainTime = 0;		//剩余时间
	m_TrainingHallData.m_LastFinishTime = 0;	//最后一次完成练功时间
	m_TrainingHallData.m_OnHookNum = 0;
	m_Status					   = enTrainingStatus_Non;

	m_VipFinishTrainNum = 0;							//今天使用VIP立即完成次数

	m_LastVipFinishTrainTime = 0;						//最后使用VIP立即完成时间

	m_TrainingTimeLong = 0;

	m_ChangeTimeLongTime = 0;

	m_BeginXiWuTime = 0;
}

TrainingHallPart::~TrainingHallPart()
{
}



//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool TrainingHallPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster==0 || pContext==0 || nLen<sizeof(SDB_Get_TrainingHallData_Rsp))
	{
		return false;
	}

	m_pActor= (IActor*)pMaster;

	const SDB_Get_TrainingHallData_Rsp * pTrainingHallData = (SDB_Get_TrainingHallData_Rsp *)pContext;

	m_TrainingHallData.m_RemainTime     = pTrainingHallData->RemainTime;

	if( m_TrainingHallData.m_RemainTime < 0 )
	{
		m_TrainingHallData.m_RemainTime  = 0;
	}

	m_TrainingHallData.m_LastFinishTime = pTrainingHallData->LastFinishTime;

	m_TrainingHallData.m_OnHookNum		= pTrainingHallData->OnHookNum;

	m_Status = (enTrainingStatus)pTrainingHallData->TrainStatus;

	m_VipFinishTrainNum = pTrainingHallData->VipFinishTrainNum;

	m_LastVipFinishTrainTime = pTrainingHallData->LastVipFinishTrainTime;

	m_TrainingTimeLong = pTrainingHallData->TrainingTimeLong;

	const SGameConfigParam & GameCnfg = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( 0 == m_TrainingTimeLong )
	{
		m_TrainingTimeLong = GameCnfg.m_MaxTrainingTime;
	}

	time_t nCurTime = CURRENT_TIME();

	//习武
	m_BeginXiWuTime = pTrainingHallData->BeginXiWuTime;
	m_uidXiWuActor = UID(pTrainingHallData->uidActorXiWu);

	if ( m_BeginXiWuTime > 0 )
	{
		if ( m_BeginXiWuTime + GameCnfg.m_TotalXiWuTime <= nCurTime )
		{
			this->EndXiWu();
		}
		else
		{
			UINT32 RemainTime = m_BeginXiWuTime + GameCnfg.m_TotalXiWuTime - nCurTime;

			//设置结束定时器
			g_pGameServer->GetTimeAxis()->SetTimer(enTrainingHallTIMER_ID_EndXiWu,this,RemainTime * 1000,"TrainingHallPart::Create");			
		}
	}

	//设置定时器,在线的玩家，晚上0点0分0秒更新次数
	tm * pTm = localtime(&nCurTime);

	UINT32 LeftTime = XDateTime::SECOND_OF_DAY - (pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec);

	g_pGameServer->GetTimeAxis()->SetTimer(enTrainingHallTIMER_ID_Night12,this,LeftTime * 1000,"TrainingHallPart::Create");

	//注册事件,得到玩家在干什么
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);
	
	m_pActor->SubscribeEvent(msgID,this,"TrainingHallPart::Create");

	return true;
}

//释放
void TrainingHallPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart TrainingHallPart::GetPartID(void)
{
	return enThingPart_Actor_TrainingHall;
}

//取得本身生物
IThing*		TrainingHallPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool TrainingHallPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen < sizeof(SDB_Update_TrainingHallData_Req))
	{
		return false;
	}

	SDB_Update_TrainingHallData_Req * pTrainingHallData = (SDB_Update_TrainingHallData_Req *)buf;

	pTrainingHallData->RemainTime	  = m_TrainingHallData.m_RemainTime;		//剩余时间

	pTrainingHallData->LastFinishTime =  m_TrainingHallData.m_LastFinishTime;	//上次完成时间
	
	pTrainingHallData->OnHookNum	  = m_TrainingHallData.m_OnHookNum;			//已挂机次数

	pTrainingHallData->Uid_User		  = m_pActor->GetUID().ToUint64();

	//pTrainingHallData->GetExp		  = m_TrainingHallData.m_GetExp;			//已获得经验

	pTrainingHallData->TrainStatus	  = m_Status;

	pTrainingHallData->VipFinishTrainNum = m_VipFinishTrainNum;

	pTrainingHallData->LastVipFinishTrainTime = m_LastVipFinishTrainTime;

	pTrainingHallData->TrainingTimeLong = m_TrainingTimeLong;

	pTrainingHallData->BeginXiWuTime = m_BeginXiWuTime;

	pTrainingHallData->uidActorXiWu  = m_uidXiWuActor.ToUint64();

	nLen = sizeof(SDB_Update_TrainingHallData_Req);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void TrainingHallPart::InitPrivateClient() 
{
	this->GetTrainingTimeLong();

	time_t nCurTime = CURRENT_TIME();

	if(m_TrainingHallData.m_RemainTime != 0 && m_Status == enTrainingStatus_Do){

		this->Enter_OnLine();
	}
}


//玩家下线了，需要关闭该ThingPart
void TrainingHallPart::Close()
{
	g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_Night12,this);
}

//保存数据
void TrainingHallPart::SaveData()
{
	SDB_Update_TrainingHallData_Req Req;

	int nLen = sizeof(SDB_Update_TrainingHallData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateTrainingHallInfo,ob.TakeOsb(),0,0);
}

//获得已挂机次数
INT16 TrainingHallPart::GetCurOnhookNum()
{
	time_t nCurTime = CURRENT_TIME();

	if(XDateTime::GetInstance().IsSameDay(nCurTime,m_TrainingHallData.m_LastFinishTime)==false)
	{
         this->m_TrainingHallData.m_OnHookNum = 0;
	}

	return this->m_TrainingHallData.m_OnHookNum;
}

INT16 TrainingHallPart::GetTotalOnhookNum()
{
	ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(m_pActor->GetUID());

	INT32 TotalOnhookNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxOnhookNumOfDay;;

	if(pSyndicate != 0)
	{
		TotalOnhookNum += pSyndicate->GetWelfareValue(enWelfare_OnHookNumOneDay);
	}

	TotalOnhookNum += m_pActor->GetVipValue(enVipType_AddTrainingNum);

	return TotalOnhookNum;
}
//同步练功数据
void TrainingHallPart::SyncTrainingData(INT32 nPerExp)
{
	SC_TrainingData_Sync Rsp;

	Rsp.m_CurOnHookNum = GetCurOnhookNum();

	switch(m_Status)
	{
	case enTrainingStatus_Non:
		{
			Rsp.m_RemainTime = 0;
		}
		break;
	case enTrainingStatus_Do:
		{
			Rsp.m_RemainTime = m_TrainingHallData.m_RemainTime - (CURRENT_TIME() - m_TrainingHallData.m_LastFinishTime);
		}
		break;
	case enTrainingStatus_Pause:
		{
			Rsp.m_RemainTime =m_TrainingHallData.m_RemainTime;
		}
		break;
	}
	//if(m_TrainingHallData.m_RemainTime > 0){
	//	Rsp.m_RemainTime = m_TrainingHallData.m_RemainTime - (time(0) - m_TrainingHallData.m_LastFinishTime);
	//}else{
	//	Rsp.m_RemainTime = 0;
	//}
	Rsp.m_GetExp = nPerExp;
	Rsp.m_TotalOnHookNum = GetTotalOnhookNum();
	Rsp.m_Status = m_Status;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	Rsp.m_Price		= GameParam.m_TrainingFinishNowPrice;

	Rsp.m_Charge	= GetConsumeSpiritStone();

	Rsp.m_TotalTime	= GetTrainingTimeLong();

	Rsp.m_VipFinishTrainNum = GetVipFinishTrainNum();
	Rsp.m_DecTrainingTime   = GameParam.m_MaxTrainingTime - m_TrainingTimeLong;
	
	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SyncTrainingData,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

void TrainingHallPart::Enter()
{
	SC_EnterTrainingHall_Rsp Rsp;

	//if( m_TrainingHallData.m_RemainTime==0
	//	&& m_TrainingHallData.m_OnHookNum >= this->GetTotalOnhookNum() && XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_TrainingHallData.m_LastFinishTime))
	//{
	//	Rsp.m_Result = enTrainingRetCode_TrainingNumLimit;		
	//}
	//else
	//{

	//先发返回，客户端要求
	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_EnterTrainingHall,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	this->SyncTrainingData();

	this->SycXiWuData();
//		Rsp.m_Result = enTrainingRetCode_OK;
//		Rsp.m_Charge = GetConsumeSpiritStone();
//		INT32 RemainNum = GetRemainNum();
//	    Rsp.m_CurOnHookNum = GetCurOnhookNum();
////	    Rsp.m_RemainTime = RemainNum * GetTrainingTimeSpace();
//		Rsp.m_RemainTime = m_TrainingHallData.m_RemainTime - (time(0) - m_TrainingHallData.m_LastFinishTime) % this->GetTrainingTimeSpace();;
////		Rsp.m_bTraining = (m_TrainingHallData.m_BeginTime!=0);
//		Rsp.m_bTraining = (m_TrainingHallData.m_RemainTime !=0 );
//		Rsp.m_GetExp = m_TrainingHallData.m_GetExp;
//		Rsp.m_TotalOnHookNum = GetTotalOnhookNum();
//		Rsp.m_OnHookNum = m_TrainingHallData.m_OnHookNum;
	//}



}

//开始练功
void TrainingHallPart::StartTraining()
{
	SC_StartTraining_Rsp Rsp;

	Rsp.m_Result = enTrainingRetCode_OK;

	time_t nCurTime = CURRENT_TIME();

	if( !XDateTime::GetInstance().IsSameDay(nCurTime,m_TrainingHallData.m_LastFinishTime)){
		//和进度不同一天
		m_TrainingHallData.m_OnHookNum = 0;
	}

	if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enTrainingRetCode_ErrInDuoBao;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enTrainingRetCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enTrainingRetCode_ErrWaitTeam;
	}
	else if( 0 < m_TrainingHallData.m_RemainTime)
	{
		//已在挂机,继续练功
		this->ContinueTraining();
	}
	else if( 0 >= m_TrainingHallData.m_RemainTime && this->GetCurOnhookNum() >= GetTotalOnhookNum())
	{
		//次数限制
		Rsp.m_Result = enTrainingRetCode_TrainingNumLimit;
	}
	else if ( m_pActor->GetCrtProp(enCrtProp_ActorStone) < GetConsumeSpiritStone() )
	{
		//没灵石
		Rsp.m_Result = enTrainingRetCode_NoStone;
	}
	else
	{
		m_pActor->AddCrtPropNum(enCrtProp_ActorStone,-GetConsumeSpiritStone());

		++m_TrainingHallData.m_OnHookNum;

		m_TrainingHallData.m_RemainTime = this->GetTrainingTimeLong();

		m_TrainingHallData.m_LastFinishTime = nCurTime;

		m_Status = enTrainingStatus_Do;

		SyncTrainingData();

		StartTimer(); //启动定时器

		//发布事件
		SS_Training Training;
		Training.m_bAutoStop = false;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Training);
		m_pActor->OnEvent(msgID,&Training,sizeof(Training));

		return;		 	  
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_StartTraining,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//停止练功
void TrainingHallPart::StopTraining() 
{
	SC_StopTraining_Rsp Rsp;
	Rsp.m_Result = enTrainingRetCode_OK;

	if( 0 == m_TrainingHallData.m_RemainTime)
	{
		Rsp.m_Result = enTrainingRetCode_ErrNoOnHook;
	}
	else
	{
		m_TrainingHallData.m_RemainTime -= (CURRENT_TIME() - m_TrainingHallData.m_LastFinishTime);
		m_Status	 = enTrainingStatus_Pause;
		g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_GiveExp,this);	
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_StopTraining,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}


//获得每次练功的时长
INT32 TrainingHallPart::GetTrainingTimeSpace()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_TrainingTimeSpace;
}

//获得每天可练功次数
INT32  TrainingHallPart::GetMaxTrainingNum()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxTrainingNum;
}

//每次挂机需要消防灵石
INT32  TrainingHallPart::GetConsumeSpiritStone()
{
	float rate = 1;

	ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(m_pActor->GetUID());

	if(pSyndicate)
	{
		rate =	(100-pSyndicate->GetWelfareValue(enWelfare_ReduceTrainingHallMoney))/(float)100;
	}

	//消耗灵石的数量=5000*帮派福利(练功减免)
	INT32 num = 5000 * rate+0.99999;
	return num;
}


//获得每次练功可获得经验
INT32 TrainingHallPart::GetExpOfPerTraining()
{
	//角色等级
	INT32 ActorLevel = m_pActor->GetCrtProp(enCrtProp_Level);

	//每次经验=21000*(角色等级^1/2) *物品加成

	INT32 TrainingExpFactor = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_TrainingExpFactor;

	INT32 exp = TrainingExpFactor*pow(ActorLevel,0.5);

	return exp;
}

//获得剩余练功次数
INT32 TrainingHallPart::GetRemainNum()
{	
	if(XDateTime::GetInstance().IsSameDay(m_TrainingHallData.m_LastFinishTime,CURRENT_TIME())==false)
	{
		return 1;
	}

	INT32 SpaceTime = GetTrainingTimeSpace();
	return (m_TrainingHallData.m_RemainTime+SpaceTime-1) / SpaceTime;
}


void TrainingHallPart::OnTimer(UINT32 timerID)
{
     switch(timerID)
	 {
	 case enTrainingHallTIMER_ID_GiveExp:
		 {
			this->EndTraining();
		 }
		 break;
	 case enTrainingHallTIMER_ID_Night12:
		 {
			this->m_TrainingHallData.m_OnHookNum = 0;

			g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_Night12,this);

			g_pGameServer->GetTimeAxis()->SetTimer(enTrainingHallTIMER_ID_Night12,this,XDateTime::SECOND_OF_DAY * 1000,"TrainingHallPart::OnTimer");
		 }
		 break;
	 case enTrainingHallTIMER_ID_EndXiWu:
		 {
			this->EndXiWu();
		 }
		 break;
	 default:
		 break;
	 }
}

//立即完成练功
void  TrainingHallPart::TrainingFinishNow()
{
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	SC_TrainingFinishNow Rsp;

	//是否使用VIP立即完成
	bool bVipFinish = (this->GetVipFinishTrainNum() < m_pActor->GetVipValue(enVipType_FinishTrainingFreeNum));

	if ( m_TrainingHallData.m_RemainTime == 0){

		Rsp.m_Result = enTrainingRetCode_ErrNoOnHook;

	}else if ( !bVipFinish && m_pActor->GetCrtProp(enCrtProp_ActorMoney) < GameParam.m_TrainingFinishNowPrice && m_pActor->GetCrtProp(enCrtProp_ActorTicket) < GameParam.m_TrainingFinishNowPrice){

		Rsp.m_Result = enTrainingRetCode_NoMoneyOrTicket;

	}else{
		
		if ( bVipFinish){

			m_LastVipFinishTrainTime = CURRENT_TIME();
			++m_VipFinishTrainNum;

		}else{
			
			//先扣仙石或礼卷
			if( m_pActor->GetCrtProp(enCrtProp_ActorTicket) >= GameParam.m_TrainingFinishNowPrice){
				m_pActor->AddCrtPropNum(enCrtProp_ActorTicket, -GameParam.m_TrainingFinishNowPrice);
			}else{
				m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, -GameParam.m_TrainingFinishNowPrice);

				g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), GameParam.m_TrainingFinishNowPrice, m_pActor->GetCrtProp(enCrtProp_ActorMoney), "练功堂立即完成");
			}
		}

		INT32 OldExp = m_pActor->GetCrtProp(enCrtProp_ActorExp);

		//INT32 exp = this->GetRemainNum() * GetExpOfPerTraining()*m_pActor->GetMultipExpFactor()+0.999999;
		//练功现在只在完成时给一次经验
		INT32 exp = this->GetExpOfPerTraining()*m_pActor->GetMultipExpFactor()+0.999999;

		INT32 NewExp = 0;

		//享受多倍经验
		m_pActor->CombatActorAddExp(exp, true, &NewExp);

		//m_TrainingHallData.m_GetExp += (NewExp - OldExp);

		m_TrainingHallData.m_RemainTime = 0;

		m_TrainingHallData.m_LastFinishTime = CURRENT_TIME();

		g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_GiveExp,this);

		m_Status = enTrainingStatus_Non;

		this->SyncTrainingData(NewExp - OldExp);

		//聊天框提示练功获得经验
		char szTip[DESCRIPT_LEN_50] = "\0";
		sprintf_s(szTip,sizeof(szTip),g_pGameServer->GetGameWorld()->GetLanguageStr(10089),NewExp - OldExp);
		g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip,m_pActor,enTalkMsgType_UpLevel);
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_TrainingFinishNow,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//练功结束,给予经验
UINT32  TrainingHallPart::GiveExp()
{
	INT32 OldExp = m_pActor->GetCrtProp(enCrtProp_ActorExp);

	INT32 exp = GetExpOfPerTraining()*m_pActor->GetMultipExpFactor()+0.999999;

	INT32 NewExp = 0;

	//享受多倍经验
	m_pActor->CombatActorAddExp(exp, true, &NewExp);

	return NewExp - OldExp;
}


//启动定时器
void TrainingHallPart::StartTimer()
{
	if ( 0 == m_TrainingHallData.m_RemainTime )
		return;

	//if( !m_bContinue){
	//	m_TrainingTimeSpace = GetTrainingTimeSpace();

	//}else{
	//	m_TrainingTimeSpace = m_TrainingHallData.m_RemainTime % this->GetTrainingTimeSpace();		
	//	
	//}

	//if(m_TrainingTimeSpace == 0)
	//{
	//	m_TrainingTimeSpace = GetTrainingTimeSpace();
	//}
	////上线如果已在练功，则创建的第一次定时器取剩下的时间


	g_pGameServer->GetTimeAxis()->SetTimer(enTrainingHallTIMER_ID_GiveExp,this,m_TrainingHallData.m_RemainTime *1000,"TrainingHallPart::StartTimer[enTrainingHallTIMER_ID_GiveExp]");
}

//上线如果还在练功中,则进入
void  TrainingHallPart::Enter_OnLine()
{
	//需要切换场景
	TSceneID SceneID;
	SceneID.From(m_pActor->GetCrtProp(enCrtProp_ActorHouShanSceneID));

	IGameScene * pGameScene = 0;
	if(!SceneID.IsValid())
	{
		//创建
		TMapID MapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_HouShanMapID;
		pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
		SceneID =pGameScene->GetSceneID();
	}
	else
	{
		pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
	}

	if(pGameScene)
	{
		if(pGameScene->EnterScene(m_pActor))
		{
			m_pActor->SetCrtProp(enCrtProp_ActorHouShanSceneID,SceneID.m_id);
		}
	}

	SC_EnterBuilding_Rsp Rsp;

	Rsp.m_RetCode = enBuildingRetCode_OK;

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_EnterTrainingHall,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	//已在挂机,继续练功
	this->ContinueTraining();

	this->SycXiWuData();
}

//清除练功堂次数限制
void TrainingHallPart::ClearTrainNum()
{
	m_TrainingHallData.m_LastFinishTime = 0;
	m_TrainingHallData.m_OnHookNum      = 0;
}

//掉线，下线要做的一些事
void  TrainingHallPart::LeaveDoSomeThing()
{
	this->GetTrainingTimeLong();

	if( m_Status == enTrainingStatus_Do){
		//停止练功
		m_TrainingHallData.m_RemainTime -= (CURRENT_TIME() - m_TrainingHallData.m_LastFinishTime);

		m_Status = enTrainingStatus_Pause;

		g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_GiveExp,this);
	}
}

//继续练功
void  TrainingHallPart::ContinueTraining()
{
	m_Status = enTrainingStatus_Do;

	m_TrainingHallData.m_LastFinishTime = CURRENT_TIME();

	this->SyncTrainingData();

	StartTimer(); //启动定时器
}

//得到使用VIP立即完成次数
UINT8 TrainingHallPart::GetVipFinishTrainNum()
{
	if ( !XDateTime::GetInstance().IsSameDay(m_LastVipFinishTrainTime,CURRENT_TIME())){

		m_VipFinishTrainNum = 0;
	}

	return m_VipFinishTrainNum;
}

void TrainingHallPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if (msgID == EventData.m_MsgID){
		
		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		if (0 == pActorDoing)
			return;

		if (enTrainingStatus_Do == m_Status)
			//玩家在练功
			pActorDoing->m_ActorDoing = enActorDoing_Training;
	}
}

//开始习武
void TrainingHallPart::StartXiWu(UID uidActor)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidActor);

	SC_StartXiWu Rsp;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if ( 0 == pActor )
	{
		Rsp.m_RetCode = enTrainingRetCode_NoActor;
	}
	else if ( m_uidXiWuActor.IsValid() )
	{
		Rsp.m_RetCode = enTrainingRetCode_HaveXiWu;
	}
	else if ( uidActor == m_pActor->GetUID() )
	{
		Rsp.m_RetCode = enTrainingRetCode_NotMaster;
	}
	else if ( m_pActor->GetCrtProp(enCrtProp_ActorStone) < GameParam.m_XiWuNeedStoneNum )
	{
		Rsp.m_RetCode = enTrainingRetCode_NoStone;
	}
	else if ( m_pActor->GetCrtProp(enCrtProp_Level) < pActor->GetCrtProp(enCrtProp_Level)
		|| ( (m_pActor->GetCrtProp(enCrtProp_Level) == pActor->GetCrtProp(enCrtProp_Level) && m_pActor->GetCrtProp(enCrtProp_ActorExp) <= pActor->GetCrtProp(enCrtProp_ActorExp)) ) )
	{
		Rsp.m_RetCode = enTrainingRetCode_NoMoreMaster;
	}
	else
	{
		if ( pActor->GetMaster() != m_pActor )
		{
			TRACE("<error> %s : %d Line 客户端发送上来的角色UID(%s)不是主角(%s)的招募角色！！", __FUNCTION__, __LINE__, uidActor.ToString(), m_pActor->GetName());
			return;
		}

		m_uidXiWuActor = uidActor;
		
		//开始习武，马上获得经验
		UINT32 GetExp = this->GetXiWuExp();
		INT32 nNewExp = 0;
		UINT32 RealGetExp = 0;
		UINT32 nOldExp = pActor->GetCrtProp(enCrtProp_ActorExp);
		INT16 nOldLevel = pActor->GetCrtProp(enCrtProp_Level);
		char szMsg[DESCRIPT_LEN_100] = "\0";

		pActor->AddCrtPropNum(enCrtProp_ActorMultipExp,GetExp,&nNewExp);
		RealGetExp = nNewExp - nOldExp;

		//招募角色经验大于主角经验时，直接设置为和主角相同经验
		if ( m_pActor->GetCrtProp(enCrtProp_Level) < pActor->GetCrtProp(enCrtProp_Level)
		|| ( (m_pActor->GetCrtProp(enCrtProp_Level) == pActor->GetCrtProp(enCrtProp_Level) && m_pActor->GetCrtProp(enCrtProp_ActorExp) <= pActor->GetCrtProp(enCrtProp_ActorExp)) ) )
		{
			pActor->SetCrtProp(enCrtProp_Level,m_pActor->GetCrtProp(enCrtProp_Level));
			pActor->SetCrtProp(enCrtProp_ActorExp,m_pActor->GetCrtProp(enCrtProp_ActorExp));
			RealGetExp = g_pGameServer->GetConfigServer()->AcotorRealGetExp(nOldLevel,nOldExp,pActor->GetCrtProp(enCrtProp_Level),pActor->GetCrtProp(enCrtProp_ActorExp));
		}

		//聊天框提示习武获得的经验
		sprintf(szMsg, g_pGameServer->GetGameWorld()->GetLanguageStr(10091), pActor->GetName(),RealGetExp);
		g_pGameServer->GetGameWorld()->WorldSystemMsg(szMsg, m_pActor,enTalkMsgType_UpLevel);

		//开始习武
		m_BeginXiWuTime = CURRENT_TIME();

		m_uidXiWuActor	= uidActor;

		//设置习武结束定时器
		g_pGameServer->GetTimeAxis()->SetTimer(enTrainingHallTIMER_ID_EndXiWu,this,GameParam.m_TotalXiWuTime * 1000,"TrainingHallPart::BeginXiWu");



		//同步
		this->SycXiWuData();
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_StartXiWu,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//同步习武
void	TrainingHallPart::SycXiWuData()
{
	SC_Syc_XiWu Rsp;

	if ( m_BeginXiWuTime > 0 )
	{
		if ( m_BeginXiWuTime + g_pGameServer->GetConfigServer()->GetGameConfigParam().m_TotalXiWuTime < CURRENT_TIME() )
		{
			//结束
			this->EndXiWu();
		}
		else
		{
			Rsp.m_LeftTime = m_BeginXiWuTime + g_pGameServer->GetConfigServer()->GetGameConfigParam().m_TotalXiWuTime - CURRENT_TIME();

			Rsp.m_uidActor = m_uidXiWuActor;		
		}
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SC_Syc_XiWu,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//取消习武
void TrainingHallPart::CancelXiWu()
{
	SC_CancelXiWu Rsp;

	this->EndXiWu();

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_CancelXiWu,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//得到可练功时长
UINT32 TrainingHallPart::GetTrainingTimeLong()
{
	if ( !m_pActor->GetIsSelfOnline() )
		return m_TrainingTimeLong;

	if ( XDateTime::GetInstance().IsSameDay(m_ChangeTimeLongTime, CURRENT_TIME()) )
		return m_TrainingTimeLong;

	if ( 0 == m_ChangeTimeLongTime )
	{
		m_ChangeTimeLongTime = m_pActor->GetCrtProp(enCrtProp_LastOnlineTime);
	}

	UINT32 CurTime = CURRENT_TIME();

	if ( 0 != m_ChangeTimeLongTime )
	{
		const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

		//获得上次上线时间离现在的天数
		INT32 OnlineSpaceDay = CurTime / XDateTime::SECOND_OF_DAY  - m_ChangeTimeLongTime / XDateTime::SECOND_OF_DAY;

		if ( OnlineSpaceDay == 1 )
		{
			//连续上线，减少练功时间
			if ( m_TrainingTimeLong < GameParam.m_ServerConfigParam.m_DecTraingTime ||  m_TrainingTimeLong - GameParam.m_ServerConfigParam.m_DecTraingTime <= GameParam.m_ServerConfigParam.m_MinTrainingTime )
			{
				m_TrainingTimeLong = GameParam.m_ServerConfigParam.m_MinTrainingTime;
			}
			else
			{
				m_TrainingTimeLong -= GameParam.m_ServerConfigParam.m_DecTraingTime;
			}
		}
		else if ( OnlineSpaceDay > 1 )
		{
			//间隔天数上线，增加练功时间
			if ( m_TrainingTimeLong + GameParam.m_ServerConfigParam.m_AddTraingTime >= GameParam.m_MaxTrainingTime )
			{
				m_TrainingTimeLong = GameParam.m_MaxTrainingTime;
			}
			else
			{
				m_TrainingTimeLong += GameParam.m_ServerConfigParam.m_AddTraingTime;
			}
		}
	}

	m_ChangeTimeLongTime = CurTime;

	return m_TrainingTimeLong;
}

//得到习武经验
UINT32 TrainingHallPart::GetXiWuExp()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_uidXiWuActor);

	if ( 0 == pActor )
	{
		TRACE("<error> %s : %d Line 找不到习武角色(UID:%s)！！", __FUNCTION__, __LINE__, m_uidXiWuActor.ToString());
		m_BeginXiWuTime = 0;
		m_uidXiWuActor = UID();
		return 0;
	}
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//UINT32 CurTime = CURRENT_TIME();

	//习武完成，经验式子：30000*(角色等级^1/2)
	UINT32 GetExp = GameParam.m_ServerConfigParam.m_XiWuExpParam * pow(pActor->GetCrtProp(enCrtProp_Level),0.5) + 0.99999;

	//if ( m_BeginXiWuTime + GameParam.m_TotalXiWuTime  > CurTime )
	//{
	//	//提前结束，经验式子：30000*(角色等级^1/2)*（实际习武时间/总时间）*0.7
	//	GetExp = GameParam.m_ServerConfigParam.m_XiWuExpParam * pow(pActor->GetCrtProp(enCrtProp_Level),0.5) * ( (m_BeginXiWuTime + GameParam.m_TotalXiWuTime - CurTime) / GameParam.m_TotalXiWuTime) * GameParam.m_ServerConfigParam.m_CancelDecExpRand / 100.0f + 0.99999;
	//}
	//else
	//{
	//	//习武完成，经验式子：30000*(角色等级^1/2)
	//	GetExp = GameParam.m_ServerConfigParam.m_XiWuExpParam * pow(pActor->GetCrtProp(enCrtProp_Level),0.5) + 0.99999;
	//}

	return GetExp;
}

//习武结束
void	  TrainingHallPart::EndXiWu()
{

	g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_EndXiWu,this);

	m_BeginXiWuTime = 0;
	m_uidXiWuActor = UID();

	//同步下
	this->SycXiWuData();
}

//练功结束
void	 TrainingHallPart::EndTraining()
{

	//得到经验
	UINT32 GetExp = this->GiveExp();

	m_TrainingHallData.m_RemainTime = 0;
	m_Status = enTrainingStatus_Non;
	//m_TrainingHallData.m_LastFinishTime = 0;  //这里不能清零，该变量在判断当天是否练过功时，需要用到。

	g_pGameServer->GetTimeAxis()->KillTimer(enTrainingHallTIMER_ID_GiveExp,this);

	//同步
	this->SyncTrainingData(GetExp);

	//聊天框提示练功获得经验
	char szTip[DESCRIPT_LEN_50] = "\0";
	sprintf_s(szTip,sizeof(szTip),g_pGameServer->GetGameWorld()->GetLanguageStr(10089),GetExp);
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip,m_pActor,enTalkMsgType_UpLevel);


}

