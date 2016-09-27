#include "IActor.h"
#include "ActivityPart.h"

#include "XDateTime.h"
#include "ActivityCmd.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IPacketPart.h"
#include "IXiuLianPart.h"
#include "ISyndicateMgr.h"
#include "ISyndicateMember.h"
#include "IFuBenPart.h"
#include "RandomService.h"
#include "GodSwordShopPart.h"
#include "GatherGodHousePart.h"
#include "IDouFaPart.h"
#include "IGoodsServer.h"
#include "ISynPart.h"
#include "ITaskPart.h"
#include "ITalismanPart.h"
#include "ISyndicate.h"

ActivityPart::ActivityPart()
{
	m_pActor = 0; 

	m_SignInNumOfMonth = 0;  //本月签到次数

	m_LastSignInTime = 0;

	m_bClientOpenActivity = 0;

	m_OnLineAwardID = 0;

	m_bCanTakeOnlineAward = 0;

	m_NextOnlineAwardTime = 0;

	m_LastOnLineAwardRestTime = 0;

	m_OffLineNum = 0;

	m_NewPlayerGuideIndex = 0;

}


ActivityPart::~ActivityPart()
{
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool ActivityPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster==0 || pContext ==0 || nLen<SIZE_OF(SDB_GetActivityPart_Rsp()))
	{
		TRACE("<error> %s : %d line 数据长度有误 nLen = %d ,期待的长度是: %d !",__FUNCTION__,__LINE__,nLen,SIZE_OF(SDB_GetActivityPart_Rsp()));
		return false;
	}
	m_pActor = (IActor*)pMaster;

	SDB_GetActivityPart_Rsp * pActivityPartRsp = (SDB_GetActivityPart_Rsp*)pContext;

	this->m_LastSignInTime = pActivityPartRsp->m_LastSignInTime;
	this->m_SignInNumOfMonth = pActivityPartRsp->m_SignInNumOfMonth;
	this->m_AwardHistory.set(pActivityPartRsp->m_AwardHistory,SIZE_OF(pActivityPartRsp->m_AwardHistory));
	this->m_OnLineAwardID = pActivityPartRsp->m_OnLineAwardID;
	this->m_LastOnLineAwardRestTime = pActivityPartRsp->m_LastOnLineAwardRestTime;
	this->m_bCanTakeOnlineAward = pActivityPartRsp->m_bCanTakeOnlineAward;
	this->m_OffLineNum = pActivityPartRsp->m_OffLineNum;
	//this->m_NewPlayerGuideIndex = pActivityPartRsp->m_NewPlayerGuideIndex;
	this->m_strGuideContext = pActivityPartRsp->m_szGuideContext;

	INT16 OffLineNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_OffLineNumOnlineAwardReset ;
	if(++this->m_OffLineNum >= OffLineNum)
	{
		this->m_OffLineNum = 0;
		m_bCanTakeOnlineAward = 0;
	}

	if(this->m_LastOnLineAwardRestTime==0)
	{
		this->m_LastOnLineAwardRestTime = CURRENT_TIME();
	}

	SDB_ActivityData * pActivityData = (SDB_ActivityData*)(pActivityPartRsp+1);

	INT32 count = (nLen - SIZE_OF(SDB_GetActivityPart_Rsp()))/SIZE_OF(SDB_ActivityData());

	for(int i=0; i<count;i++,pActivityData++)
	{
		SActivityData & ActivityData = m_mapActivity[pActivityData->m_ActivityID];

		ActivityData.m_ActivityID = pActivityData->m_ActivityID;

		ActivityData.m_bFinished = pActivityData->m_bFinished;

		ActivityData.m_bTakeAward = pActivityData->m_bTakeAward;

		ActivityData.m_ActivityProgress = pActivityData->m_ActivityProgress;
	}

	//启动在线奖励定时器
	if(m_bCanTakeOnlineAward == 0)
	{
		StartTimer();
	}

	UINT32 CurTime = CURRENT_TIME();

	//24:00更新
	INT32 Interval = (XDateTime::GetInstance().GetZeroTimeOfDay() + XDateTime::SECOND_OF_DAY)-CurTime;

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Update,this,Interval*1000,"ActivityPart::Create enTimerID_Update");	
	

	
	return true;
}

//释放
void ActivityPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart ActivityPart::GetPartID(void)
{
	return enThingPart_Actor_Activity;
}

//取得本身生物
IThing*		ActivityPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool ActivityPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf == 0 || nLen < SIZE_OF(SDB_UpdateActivityPart_Req()))
	{
		return false;
	}

	SDB_UpdateActivityPart_Req * pReq = (SDB_UpdateActivityPart_Req*)buf;

	pReq->m_uidUser = m_pActor->GetUID().ToUint64();

	pReq->m_LastSignInTime = this->m_LastSignInTime;
	pReq->m_SignInNumOfMonth = this->m_SignInNumOfMonth;
	memcpy((unsigned char*)pReq->m_AwardHistory,&this->m_AwardHistory,SIZE_OF(pReq->m_AwardHistory));
	pReq->m_OnLineAwardID = this->m_OnLineAwardID;
	pReq->m_LastOnLineAwardRestTime = this->m_LastOnLineAwardRestTime;
	pReq->m_bCanTakeOnlineAward = this->m_bCanTakeOnlineAward;
	pReq->m_OffLineNum = this->m_OffLineNum;
	//pReq->m_NewPlayerGuideIndex = this->m_NewPlayerGuideIndex;
	strncpy(pReq->m_szGuideContext,this->m_strGuideContext.c_str(),SIZE_OF(pReq->m_szGuideContext));

	nLen = SIZE_OF(SDB_UpdateActivityPart_Req());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void ActivityPart::InitPrivateClient()
{
	ITaskPart * pTaskPart = m_pActor->GetTaskPart();
	if(pTaskPart && pTaskPart->IsInGuide()==false)
	{
		OpenActivity();
	}

	UINT32 CurTime = CURRENT_TIME();

	const SServer_Info & ServerInfo = g_pGameServer->GetServerInfo();

	if((ServerInfo.m_fMultipExp<1.000001 || ServerInfo.m_fMultipExp>1.000001) && CurTime>ServerInfo.m_MultipExpBeginTime && CurTime<ServerInfo.m_MultipExpEndTime)
	{
		this->OnMultipExp();
	}
}


//玩家下线了，需要关闭该ThingPart
void ActivityPart::Close()
{
}

//保存数据
void ActivityPart::SaveData()
{
	SDB_UpdateActivityPart_Req Req;
	int nLen = SIZE_OF(SDB_UpdateActivityPart_Req());

	if(OnGetDBContext(&Req,nLen)==false)
	{
		TRACE("<error> %s : %d line 获取DB数据失败!",__FUNCTION__,__LINE__ );
		return ;
	}

	OBuffer1k ob;
	ob << Req ;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateActivityPartInfo,ob.TakeOsb(),0,0);

	for(MAP_ACTIVITY::iterator it = m_mapActivity.begin(); it != m_mapActivity.end(); ++it)
	{
		SActivityData & ActivityData = (*it).second;

		SDB_ActivityData_Req ActivityData_Req;

		ActivityData_Req.m_uidUser = m_pActor->GetUID().ToUint64();
		ActivityData_Req.m_ActivityID = ActivityData.m_ActivityID;
		ActivityData_Req.m_ActivityProgress = ActivityData.m_ActivityProgress;
		ActivityData_Req.m_bFinished = ActivityData.m_bFinished;
		ActivityData_Req.m_bTakeAward = ActivityData.m_bTakeAward;


		OBuffer1k ob;
		ob << ActivityData_Req ;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateActivityData,ob.TakeOsb(),0,0);

	}


}

void  ActivityPart::GetDailyInfo( SDailyInfo & DailyInfo)
{
	/*
	enDailyID_XiuLian = 0,	 //修炼
	enDailyID_Employ     ,   //招幕
	enDailyID_MagicBook,     //法术书
	enDailyID_GoldSword,     //仙剑
	enDailyID_BaoWeiZhan,    //保卫战
	enDailyID_FuBen,         //副本
	enDailyID_DuoBao,        //夺宝
	enDailyID_GoldSwordWorld, //剑印世界
	enDailyID_XuanTian,		 //玄天
	enDailyID_TalismanWorld, //法宝世界
	enDailyID_SynWar,		 //帮战
	enDailyID_Honor,		 //荣誉
	enDailyID_Credit,		 //声望
	*/

	DailyInfo.m_RemainTimes = 0;
	DailyInfo.m_TotalTimes = 0;
	DailyInfo.m_HonorCredit = 0;
	DailyInfo.m_MaxHonorCredit = 0;
	DailyInfo.m_SynRemainTime = 0;
	DailyInfo.m_Index = 0;

	const SGameConfigParam & ConfigParam =g_pGameServer->GetConfigServer()->GetGameConfigParam();

	switch(DailyInfo.m_DailyID)
	{
	case enDailyID_XuanTian:
		{
			DailyInfo.m_RemainTimes = g_pThingServer->GetMainUIMgr().GetRemainTimeOpenXT();
			DailyInfo.m_Index = 0;
		}
		break;
	case enDailyID_FuBen:
		{
			DailyInfo.m_TotalTimes  = m_pActor->GetFuBenPart()->MaxEnterFuBenNum();
			DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - m_pActor->GetFuBenPart()->EnterFuBenNum();
			DailyInfo.m_Index = 1;
		}
		break;
	case enDailyID_BaoWeiZhan:
		{
			ISyndicateMember * pSyndicateMember = g_pGameServer->GetSyndicateMgr()->GetSyndicateMember(m_pActor->GetUID());
			if(pSyndicateMember != 0)
			{
				DailyInfo.m_TotalTimes  = ConfigParam.m_MaxEnterSynCombatNum + m_pActor->GetVipValue(enVipType_AddSynCombatNum);
				DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - m_pActor->GetFuBenPart()->GetEnterSynFuBenNum();
				
			}
			DailyInfo.m_Index = 2;
		}
		break;
	case enDailyID_GoldSwordWorld:
		{

			DailyInfo.m_TotalTimes  = m_pActor->GetFuBenPart()->GetMaxGodSwordEnterNum();
			DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - m_pActor->GetFuBenPart()->GetEnterGodSwordNum();
			DailyInfo.m_Index = 3;
		}
		break;
	case enDailyID_TalismanWorld:
		{
			const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
			DailyInfo.m_TotalTimes = GameParam.m_MaxEnterTWNum + m_pActor->GetVipValue(enVipType_AddEnterTalismanNum);
			DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - m_pActor->GetTalismanPart()->GetEnterTWNumToday() - m_pActor->GetTalismanPart()->GetVipEnterTWNumToday();
			DailyInfo.m_Index = 4;
		}
		break;
	case enDailyID_SynWar:
		{
			ISyndicate * pSyndicate = m_pActor->GetSyndicate();
			DailyInfo.m_Index = 5;
			if(pSyndicate == 0){
				return;
			}
			DailyInfo.m_TotalTimes  = m_pActor->GetSynPart()->GetMaxJoinCombatNumToday();
			DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - m_pActor->GetSynPart()->GetJoinCombatNumToday();
			DailyInfo.m_SynRemainTime  = g_pGameServer->GetSyndicateMgr()->GetRemainStartSynCombat();
			strncpy(DailyInfo.m_EnemySynName,g_pGameServer->GetSyndicateMgr()->GetVsSynName(m_pActor).c_str(),sizeof(DailyInfo.m_EnemySynName));
			strncpy(DailyInfo.m_MySynName,pSyndicate->GetSynName(),sizeof(DailyInfo.m_MySynName)); 
			
		}

		break;
	case enDailyID_Honor:
		{
			DailyInfo.m_HonorCredit  = m_pActor->GetDouFaPart()->GetGetHonorToday();
			DailyInfo.m_MaxHonorCredit = m_pActor->GetDouFaPart()->GetMaxGetHonorToday();
			DailyInfo.m_Index = 6;
		}
		break;
	case enDailyID_Credit:
		{
			SSynCombatParam SynWarParam;
			g_pGameServer->GetConfigServer()->GetSynCombatParam(m_pActor->GetCrtProp(enCrtProp_SynCombatLevel), SynWarParam);			
			DailyInfo.m_HonorCredit  = m_pActor->GetDouFaPart()->GetCreditWeek() + m_pActor->GetSynPart()->GetGetNumWeek();
			DailyInfo.m_MaxHonorCredit = m_pActor->GetDouFaPart()->GetMaxCreditWeek() + SynWarParam.m_CreditUp;
			DailyInfo.m_Index = 7;
		}
		break;
	case enDailyID_DailyTask:
		{
			DailyInfo.m_TotalTimes = m_pActor->GetTaskPart()->GetDailyTaskNum();
			DailyInfo.m_RemainTimes = m_pActor->GetTaskPart()->GetDailyTaskNotOkNum();
			DailyInfo.m_Index = 8;
		
		}
		break;
	case enDailyID_SignIn:
		{
			
			DailyInfo.m_TotalTimes = 1;
			DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - GetSignInNumOfDay();
			DailyInfo.m_Index = 9;
		}
		break;
	case enDailyID_XiuLian:
		{
			DailyInfo.m_RemainTimes = m_pActor->GetXiuLianPart()->GetAloneXiuLianRemainTime();
			DailyInfo.m_Index = 10;
		}
		break;
	case enDailyID_Employ:
		{
			DailyInfo.m_RemainTimes = m_pActor->GetGatherGodHousePart()->GetFlushEmployRemainTime();
			DailyInfo.m_Index = 11;
		}
		break;
	case enDailyID_MagicBook:
		{
			DailyInfo.m_RemainTimes = m_pActor->GetGatherGodHousePart()->GetFlushMagicBookRemainTime();
			DailyInfo.m_Index = 12;
		}
		break;
	case enDailyID_GoldSword:
		{
			DailyInfo.m_RemainTimes = m_pActor->GetGodSwordShopPart()->GetRemainFlushTime();
			DailyInfo.m_Index = 13;
		}
		break;
	case enDailyID_DuoBao:
		{
			IDouFaPart * pDouFaPart = m_pActor->GetDouFaPart();
			if( 0 != pDouFaPart){
				DailyInfo.m_TotalTimes  = pDouFaPart->MaxChallengeNum();
				DailyInfo.m_RemainTimes = DailyInfo.m_TotalTimes - pDouFaPart->GetChallengeNum();
				DailyInfo.m_Index = 14;
			}
		}
		break;

	default:
		return;
	}
}

//打开日常
void  ActivityPart::OpenDialy()
{
	SC_ActivityOpenDaily_Rsp Rsp;

	for(int i=0; i< ARRAY_SIZE(Rsp.m_DailyInfo); i++)
	{
		Rsp.m_DailyInfo[i].m_DailyID = (enDailyID)i;
		GetDailyInfo(Rsp.m_DailyInfo[i]);
	}

	OBuffer1k ob;


	ob << ActivityHeader(enActivityCmd_OpenDaily,sizeof(Rsp)) << Rsp ;

	m_pActor->SendData(ob.TakeOsb());	

}

//获得当天签到次数
UINT8 ActivityPart::GetSignInNumOfDay()
{
	if(XDateTime::GetInstance().IsSameDay(CURRENT_TIME(),m_LastSignInTime))
	{
		return 1;
	}

	m_AwardHistory.set(1,false); //设置当天未领奖

	return 0;
}

//获得当月签到次数
UINT8 ActivityPart::GetSignInNumOfMonth()
{
	if(m_LastSignInTime != 0 && XDateTime::GetInstance().IsSameMonth(CURRENT_TIME(),m_LastSignInTime) == false)
	{
		m_SignInNumOfMonth = 0;

		m_AwardHistory.zero();

		m_mapActivity.clear();
	}

	return m_SignInNumOfMonth;
}

//打开签到
void ActivityPart::OpenSignIn()
{
	SC_ActivityOpenSignIn_Rsp Rsp;

	Rsp.m_SignInNymOfMonth = GetSignInNumOfMonth();  //注意：必须先获得月签到次数，再获得日签到次数，否则可能会出现当天已签到但月签到次数为零的情况

	Rsp.m_bSignInCurDay = GetSignInNumOfDay();

	const std::map<UINT8,SSignInAwardCnfg> & mapAwardCnfg = g_pGameServer->GetConfigServer()->GetAllSignInAwardCnfg();
	const SSignInAwardCnfg * pAwardCnfg = g_pGameServer->GetConfigServer()->GetSignInAwardCnfg(1);

	Rsp.m_PolyNimbus = pAwardCnfg->m_PolyNimbus;
	Rsp.m_SignInAwardNum = mapAwardCnfg.size();

	OBuffer1k obAward;

	for(std::map<UINT8,SSignInAwardCnfg>::const_iterator it = mapAwardCnfg.begin(); it != mapAwardCnfg.end(); ++it)
	{
		const SSignInAwardCnfg & AwardCnfg = (*it).second;

		SSignInAwardInfo AwardInfo;

		AwardInfo.m_AwardID = AwardCnfg.m_AwardID;

		AwardInfo.m_SignInDayNum = AwardCnfg.m_SignInNum;

		if(Rsp.m_SignInNymOfMonth >= AwardInfo.m_SignInDayNum && (AwardInfo.m_AwardID >1 || Rsp.m_bSignInCurDay==true ))
		{			
			if(m_AwardHistory.get(AwardCnfg.m_AwardID))
			{
				AwardInfo.m_State = enSignAwardState_HaveTake;
			}
			else
			{
				AwardInfo.m_State = enSignAwardState_CanTake;
			}
		}
		else
		{
			AwardInfo.m_State = enSignAwardState_NotCanTake;
		}

		
		AwardInfo.m_AwardNum = AwardCnfg.m_GoodsAndNum.size()/2;

		if(AwardCnfg.m_NimbusStone>0)
		{
			AwardInfo.m_AwardNum++;
		}

		obAward << AwardInfo;

		SActivityAwardInfo Info;

		Info.m_AwardType = enActivityAwardType_Goods;

		for(int j=0; j< AwardCnfg.m_GoodsAndNum.size()/2; j++)
		{
			Info.m_Value = AwardCnfg.m_GoodsAndNum[j*2];
			Info.m_Pile = AwardCnfg.m_GoodsAndNum[j*2+1];

			//发送物品配置信息
			m_pActor->SendGoodsCnfg(Info.m_Value);

			obAward << Info;
		}

		if(AwardCnfg.m_NimbusStone>0)
		{
			Info.m_AwardType = enActivityAwardType_NimbusStone;
			Info.m_Value = AwardCnfg.m_NimbusStone;
			obAward << Info;
		}
	}


	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_OpenSignIn,sizeof(Rsp)+obAward.Size()) << Rsp << obAward;

	m_pActor->SendData(ob.TakeOsb());	
}

//签到
void ActivityPart::SignIn()
{
	SC_ActivitySignIn_Rsp Rsp;

	Rsp.m_Result = enActivityRetCode_Ok;
	Rsp.m_CanTakeAwardID = 0;

	if(GetSignInNumOfDay()>0)
	{
		//已签到了
		Rsp.m_Result = enActivityRetCode_ErrSignInOnce;
	}
	else
	{
		m_SignInNumOfMonth++;
		m_LastSignInTime = CURRENT_TIME();
	}

	//判断是否有新奖励可以领取
	const std::map<UINT8,SSignInAwardCnfg> & mapAwardCnfg = g_pGameServer->GetConfigServer()->GetAllSignInAwardCnfg();

	for(std::map<UINT8,SSignInAwardCnfg>::const_iterator it = mapAwardCnfg.begin(); it != mapAwardCnfg.end(); ++it)
	{
		const SSignInAwardCnfg & AwardCnfg = (*it).second;
		if(AwardCnfg.m_SignInNum == m_SignInNumOfMonth)
		{
			Rsp.m_CanTakeAwardID = AwardCnfg.m_AwardID;
			break;
		}
	}

	//获得月签到数
	Rsp.m_SignInNymOfMonth = GetSignInNumOfMonth();

	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_SignIn,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());	

	//更新签到奖励状态


}

//领取签到奖励
void ActivityPart::TakeSignInAward(UINT8  AwardID)
{
	SC_ActivityTakeSignInAward_Rsp Rsp;
	Rsp.m_AwardID = AwardID;
	Rsp.m_Result = enActivityRetCode_Ok;

	//获得月签到数
	UINT8 SignInNymOfMonth = GetSignInNumOfMonth();

	if(m_AwardHistory.get(AwardID))
	{
		Rsp.m_Result = enActivityRetCode_ErrHaveTakeAward;
	}
	else
	{
		const SSignInAwardCnfg * pAwardCnfg = g_pGameServer->GetConfigServer()->GetSignInAwardCnfg(AwardID);

		if(pAwardCnfg == 0)
		{
			Rsp.m_Result = enActivityRetCode_ErrAwardID;
		}
		else if(pAwardCnfg->m_SignInNum > SignInNymOfMonth
			   || (AwardID==1 && GetSignInNumOfDay()==0))  //领的是当天签到奖，需要特别判断，当天是否已签到了
		{
			Rsp.m_Result = enActivityRetCode_ErrSignInNum;
		}
		else
		{
			IPacketPart * pPacketPart = m_pActor->GetPacketPart();

			//判断背包空间
			if(pPacketPart->CanAddGoods(pAwardCnfg->m_GoodsAndNum)==false)
			{
				Rsp.m_Result = enActivityRetCode_ErrNoSpace;
			}
			else
			{
				for(int i=0; i<pAwardCnfg->m_GoodsAndNum.size()/2; i++)
				{
					if(pPacketPart->AddGoods(pAwardCnfg->m_GoodsAndNum[i*2],pAwardCnfg->m_GoodsAndNum[i*2+1], true)==false)
					{
						TRACE("<error> %s : %d line 增加物品到背包失败!",__FUNCTION__,__LINE__);
						continue;
					}

					g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Activity,pAwardCnfg->m_GoodsAndNum[i*2],UID(),pAwardCnfg->m_GoodsAndNum[i*2+1],"领取签到奖励");
				}

				if(pAwardCnfg->m_NimbusStone>0)
				{
					m_pActor->AddCrtPropNum(enCrtProp_ActorStone,pAwardCnfg->m_NimbusStone);
				}

				//聚灵气
				if(pAwardCnfg->m_PolyNimbus>0)
				{
					m_pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus,pAwardCnfg->m_PolyNimbus);
				}

				//声望
				if(pAwardCnfg->m_Credit>0)
				{
					m_pActor->AddCrtPropNum(enCrtProp_ActorCredit,pAwardCnfg->m_Credit);
				}

				//可以领取奖励
				m_AwardHistory.set(AwardID,true);
			}

		}
	}

	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_SignInAward,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//打开活动
void ActivityPart::OpenActivity()
{
	SC_ActivityOpenActivity_Rsp Rsp;
	Rsp.m_ActivityNum = 0;

	const std::map< UINT16, SActivityCnfg> & mapActivity = g_pGameServer->GetConfigServer()->GetAllActivityCnfg();

	OBuffer4k obActivity;

	UINT32 nCurTime = CURRENT_TIME();

	for(std::map< UINT16, SActivityCnfg>::const_iterator it = mapActivity.begin() ; it != mapActivity.end(); ++it )
	{
		const SActivityCnfg & ActivityCnfg = (*it).second;
		//if( XDateTime::GetInstance().FormatTime(ActivityCnfg.m_EndTime.c_str()) < nCurTime)
		if( ActivityCnfg.m_EndTime < nCurTime || ActivityCnfg.m_BeginTime > nCurTime)
		{
			continue;
		}

		SActivityInfo Info;

		Info.m_ActivityID = ActivityCnfg.m_ActivityID;

		Info.m_Order = ActivityCnfg.m_Order;

		SActivityData * pActivityData = GetActivityData(ActivityCnfg.m_ActivityID);

		if(pActivityData == 0 && (bool)ActivityCnfg.m_bFinished == true)
		{
			pActivityData = &m_mapActivity[ActivityCnfg.m_ActivityID];
			pActivityData->m_ActivityID = ActivityCnfg.m_ActivityID;
			pActivityData->m_ActivityProgress = ActivityCnfg.m_AttainNum;
			pActivityData->m_bFinished = true;
			pActivityData->m_bTakeAward = false;
		}

		if(pActivityData != 0)
		{
			Info.m_ActivityProgress = pActivityData->m_ActivityProgress;
			Info.m_bFinished        = pActivityData->m_bFinished;
			Info.m_bTakeAward       = pActivityData->m_bTakeAward;
		}
		

		if(ActivityCnfg.m_Ticket > 0)
		{
			Info.m_AwardNum++;
		}

		if(ActivityCnfg.m_GodStone > 0)
		{
			Info.m_AwardNum++;
		}

		if(ActivityCnfg.m_vectGoods.size() > 0)
		{
			Info.m_AwardNum += ActivityCnfg.m_vectGoods.size()/2;
		}

		obActivity << Info;

		if(ActivityCnfg.m_Ticket > 0)
		{
			SActivityAwardInfo AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_Ticket;
			AwardInfo.m_Value     = ActivityCnfg.m_Ticket;

			obActivity << AwardInfo;
		}

		if(ActivityCnfg.m_GodStone > 0)
		{
			SActivityAwardInfo AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_GodStone;
			AwardInfo.m_Value     = ActivityCnfg.m_GodStone;

			obActivity << AwardInfo;
		}

		if(ActivityCnfg.m_vectGoods.size() > 0)
		{
			for( int i = 0; i + 1 < ActivityCnfg.m_vectGoods.size(); i += 2)
			{
				SActivityAwardInfo AwardInfo;
			   AwardInfo.m_AwardType = enActivityAwardType_Goods;
			   AwardInfo.m_Value     = ActivityCnfg.m_vectGoods[i];
			   AwardInfo.m_Pile      = ActivityCnfg.m_vectGoods[i + 1];

			   m_pActor->SendGoodsCnfg(ActivityCnfg.m_vectGoods[i]);

				obActivity << AwardInfo;

				this->m_pActor->SendGoodsCnfg( AwardInfo.m_Value );
			}
		}

		Rsp.m_ActivityNum ++;	

		//char          m_szActivityName[]; //活动名称
		obActivity.Push(ActivityCnfg.m_strName.c_str(),ActivityCnfg.m_strName.length()+1);
		//char          m_szActivityRole[]; //活动规则
		obActivity.Push(ActivityCnfg.m_strRole.c_str(),ActivityCnfg.m_strRole.length()+1);
		//char          m_szActiivityAward; //活动奖励
		obActivity.Push(ActivityCnfg.m_strAwardDesc.c_str(),ActivityCnfg.m_strAwardDesc.length()+1);

		//char          m_strResFileUrl; //活动资源
		//obActivity.Push(ActivityCnfg.m_strResFileUrl.c_str(),ActivityCnfg.m_strResFileUrl.length()+1);

		

	}

	m_bClientOpenActivity = true;

	OBuffer4k ob;

	ob << ActivityHeader(enActivityCmd_OpenActivity,sizeof(Rsp)+obActivity.Size()) << Rsp << obActivity;

	m_pActor->SendData(ob.TakeOsb());


}

//领取活动奖励
void ActivityPart::TakeActivityAward(UINT16  ActivityID)
{
	SC_ActivityTakeActivityAward_Rsp Rsp;

	Rsp.m_Result = enActivityRetCode_Ok;

	Rsp.m_ActivityID = ActivityID;

	SActivityData * pActivityData = GetActivityData(ActivityID);

	if(pActivityData == 0 || pActivityData->m_bFinished == false)
	{
		Rsp.m_Result = enActivityRetCode_ErrNotFinish;
	}
	else if(pActivityData->m_bTakeAward)
	{
		Rsp.m_Result = enActivityRetCode_ErrHaveTakeActivityAward;
	}
	else
	{		
		const SActivityCnfg * pActivityCnfg   = g_pGameServer->GetConfigServer()->GetActivityCnfg(ActivityID);
		if( pActivityCnfg == 0)
		{
			Rsp.m_Result = enActivityRetCode_ErrActivityEnd;
		}
		else
		{
					//可以领奖
			SWriteSystemData SysMailData;
			SysMailData.m_DestUID = m_pActor->GetUID();
			SysMailData.m_Stone = pActivityCnfg->m_GodStone;
			SysMailData.m_Ticket = pActivityCnfg->m_Ticket;
			strncpy(SysMailData.m_szThemeText, pActivityCnfg->m_strSubject, sizeof(SysMailData.m_szThemeText));
			strncpy(SysMailData.m_szContentText, pActivityCnfg->m_strContent, sizeof(SysMailData.m_szContentText));

			std::vector<IGoods *> vecGoods;

			for( int i = 0; i + 1 < pActivityCnfg->m_vectGoods.size(); i += 2)
			{
				SCreateGoodsContext GoodsCnt;

				GoodsCnt.m_Binded = true;
				GoodsCnt.m_GoodsID = pActivityCnfg->m_vectGoods[i];
				GoodsCnt.m_Number  = pActivityCnfg->m_vectGoods[i + 1];

				IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsCnt);
				if( 0 == pGoods){
					TRACE("<error> %s : %d 行 创建活动物品奖励失败！！,物品ID = %d, 物品数量 = %d", __FUNCTION__, __LINE__, GoodsCnt.m_GoodsID, GoodsCnt.m_Number);
					continue;
				}

				vecGoods.push_back(pGoods);
			}

			g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData, vecGoods);

			pActivityData->m_bTakeAward = true;
		}

	}

	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_ActivityAward,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}


SActivityData * ActivityPart::GetActivityData(UINT16 ActivityID)
{
	MAP_ACTIVITY::iterator it = m_mapActivity.find(ActivityID);

	if(it == m_mapActivity.end())
	{
		return 0;
	}

	return &(*it).second;
}

//推进进度
void ActivityPart::AdvanceProgress(UINT16  ActivityID ) 
{
	const SActivityCnfg * pActivityCnfg   = g_pGameServer->GetConfigServer()->GetActivityCnfg(ActivityID);

	if(pActivityCnfg == 0)
	{
		return ;
	}

	SActivityData * pActivityData =  &m_mapActivity[ActivityID];

	if(pActivityData->m_bFinished == true)
	{
		return;
	}

	pActivityData->m_ActivityID = ActivityID;

	pActivityData->m_ActivityProgress++;

	if(pActivityData->m_ActivityProgress >= pActivityCnfg->m_AttainNum)
	{
		//完成
		pActivityData->m_bFinished = true;

		//领取活动奖励
		this->TakeActivityAward(ActivityID);
	}

	if(m_bClientOpenActivity == false)
	{
		return;
	}

	SC_ActivityUpdateActivity Rsp;

	Rsp.m_ActivityID = ActivityID;
	Rsp.m_ActivityProgress = pActivityData->m_ActivityProgress;
	Rsp.m_bFinished = pActivityData->m_bFinished;

	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_SC_UpdateActivity,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

}

//通知领取在线奖励
void ActivityPart::OnlineAwardNotic()
{
	SC_ActivityOnlineAwardNotic Rsp;

	Rsp.m_AwardNum = 0;
	OBuffer1k obAward;
	const SOnlineAwardCnfg * pAwardCnfg = g_pGameServer->GetConfigServer()->GetOnlineAwardCnfg(m_OnLineAwardID+1);
	if(pAwardCnfg==0)
	{
		Rsp.m_RemainTime = -1; //没有奖项了
	}
	else
	{
		Rsp.m_RemainTime = m_NextOnlineAwardTime - CURRENT_TIME();
		if( Rsp.m_RemainTime<0)
		{
			Rsp.m_RemainTime = 0;
		}
	

		//OBuffer1k obAward;
		SActivityAwardInfo  AwardInfo;
		AwardInfo.m_AwardType = enActivityAwardType_Goods;

		for(int j=0; j<pAwardCnfg->m_GoodsAndNum.size()/2; ++j)
		{

			AwardInfo.m_Value = pAwardCnfg->m_GoodsAndNum[j*2];
			AwardInfo.m_Pile  = pAwardCnfg->m_GoodsAndNum[j*2+1];

			//发送物品配置信息
			m_pActor->SendGoodsCnfg(AwardInfo.m_Value);
			obAward << AwardInfo ;
			Rsp.m_AwardNum++;

		}


		if(pAwardCnfg->m_NimbusStone > 0)
		{
			SActivityAwardInfo  AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_NimbusStone;
			AwardInfo.m_Value = pAwardCnfg->m_NimbusStone;

			obAward << AwardInfo ;
			Rsp.m_AwardNum++;

		}

		if(pAwardCnfg->m_Ticket > 0)
		{
			SActivityAwardInfo  AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_Ticket;
			AwardInfo.m_Value = pAwardCnfg->m_Ticket;

			obAward << AwardInfo ;
			Rsp.m_AwardNum++;

		}

		//聚灵气
		if(pAwardCnfg->m_PolyNimbus > 0)
		{
			SActivityAwardInfo  AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_PolyNimbus;
			AwardInfo.m_Value = pAwardCnfg->m_PolyNimbus;

			obAward << AwardInfo ;
			Rsp.m_AwardNum++;

		}

		//声望
		if(pAwardCnfg->m_Credit > 0)
		{
			SActivityAwardInfo  AwardInfo;
			AwardInfo.m_AwardType = enActivityAwardType_Credit;
			AwardInfo.m_Value = pAwardCnfg->m_Credit;

			obAward << AwardInfo ;
			Rsp.m_AwardNum++;
		}

	}
	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_OnlineAwardNotic,sizeof(Rsp)+obAward.Size()) << Rsp << obAward;

	m_pActor->SendData(ob.TakeOsb());
}


void ActivityPart::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
     case enTimerID_Online:
	 {
		 m_bCanTakeOnlineAward = true;

	     StopTimer();
	 }
	 break;
    case enTimerID_Update:
		{
			 StopTimer();		
			 StartTimer();
			 OnlineAwardNotic();
			 OpenSignIn(); //更新签到状态
		}
	 break;
	}


}

//启动在线奖励定时
void ActivityPart::StartTimer()
{
	m_bCanTakeOnlineAward = false;

	m_OffLineNum = 0;

	UINT32 nCurTime = CURRENT_TIME();

	if(XDateTime::GetInstance().IsSameDay(this->m_LastOnLineAwardRestTime,nCurTime) == false)
	{
		m_OnLineAwardID = 0;
		m_LastOnLineAwardRestTime = nCurTime;
	}

	const SOnlineAwardCnfg * pAwardCnfg = g_pGameServer->GetConfigServer()->GetOnlineAwardCnfg(m_OnLineAwardID+1);

	if(pAwardCnfg==0)
	{
		return;
	}


	//单位是分种，需要*60
	UINT32 interval = pAwardCnfg->m_OnlineTime * 60 ;

	m_NextOnlineAwardTime = nCurTime + interval;

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Online,this,interval* 1000,"ActivityPart::StartTimer");

}

//停止定时器
void ActivityPart::StopTimer()
{
	g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Online,this);
}

//领取在线奖励
void ActivityPart::TakeOnlineAward()
{
	SC_ActivityOnlineAward_Rsp Rsp;
	Rsp.m_AwardNum = 0;
	Rsp.m_Result = enActivityRetCode_Ok;

	const SOnlineAwardCnfg * pAwardCnfg = 0;

	OBuffer1k obAward;

	if(m_bCanTakeOnlineAward==false)
	{
		TRACE("<error> %s : %d line 在线时间没够，不能领取奖励!",__FUNCTION__,__LINE__);
		Rsp.m_Result = enActivityRetCode_ErrNotTime;
	}
	else if(0==(pAwardCnfg=g_pGameServer->GetConfigServer()->GetOnlineAwardCnfg(m_OnLineAwardID+1)))
	{
		Rsp.m_Result = enActivityRetCode_ErrHaveTakeAward;
	}
	else
	{		

		SActivityAwardInfo  AwardInfo;
		AwardInfo.m_AwardType = enActivityAwardType_Goods;

		for(int j=0; j<pAwardCnfg->m_GoodsAndNum.size()/2; ++j)
		{

			AwardInfo.m_Value = pAwardCnfg->m_GoodsAndNum[j*2];
			AwardInfo.m_Pile  = pAwardCnfg->m_GoodsAndNum[j*2+1];

			//发送物品配置信息
			m_pActor->SendGoodsCnfg(AwardInfo.m_Value);

			if(m_pActor->GetPacketPart()->AddGoods(AwardInfo.m_Value,AwardInfo.m_Pile, true)==false)
			{
				Rsp.m_Result = enActivityRetCode_ErrNoSpace;
			}
			else
			{
				g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Activity,AwardInfo.m_Value,UID(),AwardInfo.m_Pile,"领取在线奖励");

				obAward << AwardInfo ;

				Rsp.m_AwardNum++;
			}

		}

		if(Rsp.m_Result == enActivityRetCode_Ok)
		{
			if(pAwardCnfg->m_NimbusStone > 0)
			{
				SActivityAwardInfo  AwardInfo;
				AwardInfo.m_AwardType = enActivityAwardType_NimbusStone;
				AwardInfo.m_Value = pAwardCnfg->m_NimbusStone;

				m_pActor->AddCrtPropNum(enCrtProp_ActorStone,pAwardCnfg->m_NimbusStone);

				obAward << AwardInfo ;
				Rsp.m_AwardNum++;
			}

			if(pAwardCnfg->m_Ticket > 0)
			{
				SActivityAwardInfo  AwardInfo;
				AwardInfo.m_AwardType = enActivityAwardType_Ticket;
				AwardInfo.m_Value = pAwardCnfg->m_Ticket;

				m_pActor->AddCrtPropNum(enCrtProp_ActorTicket,pAwardCnfg->m_Ticket);

				obAward << AwardInfo ;

				Rsp.m_AwardNum++;
			}

			//聚灵气
			if(pAwardCnfg->m_PolyNimbus > 0)
			{
				SActivityAwardInfo  AwardInfo;
				AwardInfo.m_AwardType = enActivityAwardType_PolyNimbus;
				AwardInfo.m_Value = pAwardCnfg->m_PolyNimbus;

				m_pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus,pAwardCnfg->m_PolyNimbus);

				obAward << AwardInfo ;

				Rsp.m_AwardNum++;
			}

			//声望
			if(pAwardCnfg->m_Credit > 0)
			{
				SActivityAwardInfo  AwardInfo;
				AwardInfo.m_AwardType = enActivityAwardType_Credit;
				AwardInfo.m_Value = pAwardCnfg->m_Credit;

				m_pActor->AddCrtPropNum(enCrtProp_ActorCredit,pAwardCnfg->m_Credit);

				obAward << AwardInfo ;

				Rsp.m_AwardNum++;
			}

			m_OnLineAwardID++;
		}
	}

	OBuffer1k ob;

	ob << ActivityHeader(enActivityCmd_OnlineAward,sizeof(Rsp)+obAward.Size()) << Rsp << obAward;

	m_pActor->SendData(ob.TakeOsb());

	if(Rsp.m_Result == enActivityRetCode_Ok)
	{
		StartTimer(); //启动下一级奖定时器

		OnlineAwardNotic();
	}


}

	 //获得新手指引的步骤
INT16 ActivityPart::GetNewPlayerGuideIndex()
{
	return m_NewPlayerGuideIndex;
}

	 //设置新手指引步骤
void ActivityPart::SetNewPlayerGuideIndex(INT16 Index)
{
	m_NewPlayerGuideIndex = Index;
}

	 	 //获得新手指引上下文
const char* ActivityPart::GetNewPlayerGuideContext()
{
	return m_strGuideContext.c_str();
}

	 //设置新手指引上下文
void ActivityPart::SetNewPlayerGuideContext(const char * pContext)
{
	m_strGuideContext = pContext;
}

	 //多倍经验
void ActivityPart::OnMultipExp() 
{
        SC_ActivityMultipExp MultipExpRsp;

		const SServer_Info & ServerInfo = g_pGameServer->GetServerInfo();

		UINT32 CurTime = CURRENT_TIME();

		if(ServerInfo.m_MultipExpEndTime<CurTime)
		{
			return;
		}

		MultipExpRsp.m_MultipExpEndTime = ServerInfo.m_MultipExpEndTime-CurTime;
		
		char szBuffer[1024] = {0};

		sprintf_s(szBuffer,sizeof(szBuffer),g_pGameServer->GetGameWorld()->GetLanguageStr(10087),ServerInfo.m_fMultipExp);

		OBuffer4k ob;
		ob << ActivityHeader(enActivityCmd_MultipExp,sizeof(MultipExpRsp)+strlen(szBuffer)+1) << MultipExpRsp ;
		ob.Push(szBuffer,strlen(szBuffer)+1);

		m_pActor->SendData(ob.TakeOsb());
}
