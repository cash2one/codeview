

#include "IThing.h"

#include "FuMoDongPart.h"

#include "DBProtocol.h"
#include "IActor.h"
#include "XDateTime.h"
#include "IConfigServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IGameScene.h"
#include "IGameWorld.h"
#include "IMonster.h"
#include "ICombatPart.h"
#include "DMsgSubAction.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IGoodsServer.h"
#include "ITaskPart.h"


FuMoDongPart::FuMoDongPart()
{
	m_pActor = 0;
}

FuMoDongPart::~FuMoDongPart()
{
}



//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool FuMoDongPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	SDB_Get_FuMoDongData_Rsp;
	if(pMaster==0 || pMaster->GetThingClass()!=enThing_Class_Actor)
	{
		return false;
	}

	if(pContext==0 || nLen < sizeof(SDBFuMoDongData))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	SDB_Get_FuMoDongData_Rsp * pDBFuMoDongData = (SDB_Get_FuMoDongData_Rsp *)pContext;

	m_FuMoDongData.m_AccelNumOfDay		= pDBFuMoDongData->m_AccelNumOfDay;

	m_FuMoDongData.m_EndOnHookTime		= pDBFuMoDongData->m_EndOnHookTime;

	m_FuMoDongData.m_LastAccelTime		= pDBFuMoDongData->m_LastAccelTime;

	m_FuMoDongData.m_Level				= pDBFuMoDongData->m_Level;

	m_FuMoDongData.m_LastGiveExpTime	= pDBFuMoDongData->m_LastGiveExpTime;

	m_FuMoDongData.m_GiveExp = pDBFuMoDongData->m_GiveExp;

	//做一些判断
	if(m_FuMoDongData.m_EndOnHookTime)
	{			//启动定时器
		StartGiveExpTimer();
	}

	//注册事件,得到玩家在干什么
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);	

	m_pActor->SubscribeEvent(msgID,this,"FuMoDongPart::Create");

	return true;
}

//释放
void FuMoDongPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart FuMoDongPart::GetPartID(void)
{
	return enThingPart_Actor_FuMoDong;
}

//取得本身生物
IThing*		FuMoDongPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool FuMoDongPart::OnGetDBContext(void * buf, int &nLen)
{
	if(0 == buf || nLen < sizeof(SDB_Update_FuMoDongData_Req))
	{
		return false;
	}
	SDB_Update_FuMoDongData_Req * pReq = (SDB_Update_FuMoDongData_Req *)buf;

	pReq->m_AccelNumOfDay	= GetAccelNumOfDay();

	pReq->m_EndOnHookTime	= m_FuMoDongData.m_EndOnHookTime;

	pReq->m_LastAccelTime	= m_FuMoDongData.m_LastAccelTime;

	pReq->m_Level			= m_FuMoDongData.m_Level;

	pReq->m_LastGiveExpTime = m_FuMoDongData.m_LastGiveExpTime;

	pReq->Uid_User			= m_pActor->GetUID().ToUint64();

	pReq->m_GiveExp = m_FuMoDongData.m_GiveExp;

	nLen = sizeof(SDB_Update_FuMoDongData_Req);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void FuMoDongPart::InitPrivateClient()
{
	UINT32 nCurrTime = CURRENT_TIME();

	if(m_FuMoDongData.m_EndOnHookTime)
	{
		OnTimer(enFuMoDongPartTIMER_ID_GiveExp);
		if(m_FuMoDongData.m_EndOnHookTime)
		{
			Enter();			
		}

	}
}


//玩家下线了，需要关闭该ThingPart
void FuMoDongPart::Close()
{
}


//保存数据
void FuMoDongPart::SaveData()
{
	SDB_Update_FuMoDongData_Req Req;

	int nLen = sizeof(SDB_Update_FuMoDongData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateFuMoDongInfo,ob.TakeOsb(),0,0);

}

//获得挂机余下时间
INT32 FuMoDongPart::GetOnHookRemainTime()
{
	INT32 RemainTime = 0;
	if(m_FuMoDongData.m_EndOnHookTime)
	{		
		RemainTime = m_FuMoDongData.m_EndOnHookTime - CURRENT_TIME();
		if(RemainTime<0)
		{
			RemainTime = 0;		
		}
	}

	return RemainTime;
}


//进入
void FuMoDongPart::Enter()
{
	//需要切换场景

	TSceneID SceneID;
	SceneID.From(m_pActor->GetCrtProp(enCrtProp_ActorFuMoDongSceneID));

	IGameScene * pGameScene = 0;
	if(!SceneID.IsValid())
	{
		//创建
		TMapID MapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_FuMoDongMapID;
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
			m_pActor->SetCrtProp(enCrtProp_ActorFuMoDongSceneID,SceneID.m_id);
		}
	}

	SC_EnterBuilding_Rsp Rsp;

	Rsp.m_RetCode = enBuildingRetCode_OK;

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_EnterFuMoDong,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	if (m_FuMoDongData.m_EndOnHookTime != 0){

		SyncOnHookData();
	}

	//发布进入事件
	SS_EnterFuMoDong EnterFuMoDong;
	EnterFuMoDong.bOK = true;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterFuMoDong);
	m_pActor->OnEvent(msgID,&EnterFuMoDong,sizeof(EnterFuMoDong));
}

//计算打怪可以获得的经验
INT32 FuMoDongPart::CalculateExp(TMonsterID MonsterID)
{
	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MonsterID);

	if(pMonsterCnfg == 0)
	{
		return 1;
	}

	//角色等级
	INT32 ActorLevel = m_pActor->GetCrtProp(enCrtProp_Level);

	//怪物等级
	INT32 NpcLevel = pMonsterCnfg->m_Level;

	float exp = 0;

	if(ActorLevel == NpcLevel )
	{
		//经验=（创建角色等级×2+70）
		exp = ActorLevel * 2 + 70;
	}
	else if(ActorLevel < NpcLevel)
	{
		//经验=(创建角色等级×2+70)*[1+0.05*(怪物等级-创建角色等级)]
		exp = (ActorLevel * 2 + 70)*(1+0.05*(NpcLevel-ActorLevel));
	}
	else if(ActorLevel-NpcLevel >= 10)  //高于10级不获得经验
	{
		exp =  0;
	}
	else
	{
		//经验=(创建角色等级×2+70)*[1-(创建角色等级-怪物等级)/10]
		exp = (ActorLevel * 2 + 70)*(1-(ActorLevel-NpcLevel)/(float)10);
	}
	
	exp = exp*m_pActor->GetMultipExpFactor();	

	exp += 0.99999;

	return exp;	  
}


//自动打怪
void FuMoDongPart::AutoKillMonster(CS_AutoKillMonster_Req & Req)
{
	const SFuMoDongCnfg* pFuMoDongCnfg =  g_pGameServer->GetConfigServer()->GetFuMoDongCnfg(Req.m_Level);
	if(pFuMoDongCnfg==0)
	{
		TRACE("<error> %s : %d Line 找不到伏魔洞配置信息 level = %d",__FUNCTION__,__LINE__,Req.m_Level);
		return ;
	}

	//ICombatPart * pCombatPart = m_pActor->GetCombatPart();
	//if (0 == pCombatPart){
	//	return;
	//}

	//const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(pFuMoDongCnfg->m_MonsterID);
	//if (pMonsterCnfg == 0){
	//	TRACE("<error> %s : %d Line 获取不到怪物信息！！！，怪物ID : %d",  __FUNCTION__, __LINE__, pFuMoDongCnfg->m_MonsterID);
	//	return;
	//}

	SC_AutoKillMonster_Rsp Rsp;

	if(GetOnHookRemainTime())
	{
		Rsp.m_Result = enKMRetCode_ErrOnHook;
	}
	else if(m_pActor->GetCrtProp(enCrtProp_Level)<pFuMoDongCnfg->m_ActorLevel)  	//玩家等级
	{
		Rsp.m_Result = enKMRetCode_ErrLvLimit;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		Rsp.m_Result = enKMRetCode_ErrInDuoBao;
	}
	else if ( m_pActor->HaveTeam() )
	{
		Rsp.m_Result = enKMRetCode_ErrHaveTeam;
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enKMRetCode_ErrWaitTeam;
	}
	else
	{
		//成功
		if ( !this->BeginAutoKillMonster(pFuMoDongCnfg) )
			return;
		//m_FuMoDongData.m_LastGiveExpTime = CURRENT_TIME();
		//m_FuMoDongData.m_EndOnHookTime =  m_FuMoDongData.m_LastGiveExpTime+this->GetAutoKillHourNum();
		//m_FuMoDongData.m_Level  = Req.m_Level;
		//m_FuMoDongData.m_GiveExp = pCombatPart->CalculateExp(m_pActor->GetCrtProp(enCrtProp_Level), pMonsterCnfg->m_Level);

		//StartGiveExpTimer();

		//SyncOnHookData();

		////发布事件
		//SS_OnHookFuMoDong OnHookFuMoDong;
		//OnHookFuMoDong.m_bAccel = false;
		//OnHookFuMoDong.m_bOnHook = true;
		//OnHookFuMoDong.m_DecTime = 0;

		//if( (GetAutoKillHourNum() - GetOnHookRemainTime()) >= 3600){
		//	OnHookFuMoDong.m_bOnHourMore = true;
		//}else{
		//	OnHookFuMoDong.m_bOnHourMore = false;
		//}


		//UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnHookFuMoDong);
		//m_pActor->OnEvent(msgID,&OnHookFuMoDong,sizeof(OnHookFuMoDong));
		return;

	}

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_AutoKillMonster,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

}

//取消自动打怪
void FuMoDongPart::CancelKillMonster(CS_CancelKillMonster_Req & Req)
{
	SC_CancelKillMonster_Rsp Rsp;
	if(m_FuMoDongData.m_EndOnHookTime==0)
	{
		Rsp.m_Result = enKMRetCode_ErrNoOnHook;
	}
	else
	{
		StopOnHook();

		Rsp.m_Result = enKMRetCode_OK;
	}

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_CancelKillMonster,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//加速打怪
void FuMoDongPart::AccelKillMonster(CS_AccelKillMonster_Req & Req)
{

	SC_AccelKillMonster_Rsp Rsp;
	if(m_FuMoDongData.m_EndOnHookTime==0)
	{
		Rsp.m_Result = enKMRetCode_ErrNoOnHook;
	}
	else if(GetAccelNumOfDay()>=GetMaxAccelNumOfDay())
	{
		Rsp.m_Result = enKMRetCode_AccelNumLimit;
	}
	else if(m_pActor->GetCrtProp(enCrtProp_ActorMoney) < GetConsumeMoney() && m_pActor->GetCrtProp(enCrtProp_ActorTicket) < GetConsumeMoney())
	{
		Rsp.m_Result = enKMRetCode_NoMoney;
	}
	else
	{
		if( m_pActor->GetCrtProp(enCrtProp_ActorTicket) >= GetConsumeMoney()){
			m_pActor->AddCrtPropNum(enCrtProp_ActorTicket,-GetConsumeMoney());
		}else{
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney,-GetConsumeMoney());

			g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), GetConsumeMoney(), m_pActor->GetCrtProp(enCrtProp_ActorMoney), "伏魔洞加速打怪");
		}

		Rsp.m_Result = enKMRetCode_OK;

		//增加出战角色经验
		m_pActor->CombatActorAddExp(GetAccelExp(), true);

		//减少时间
		m_FuMoDongData.m_EndOnHookTime -= GetReduceTime();

		//发布事件
		SS_OnHookFuMoDong OnHookFuMoDong;
		OnHookFuMoDong.m_bAccel = true;
		OnHookFuMoDong.m_bOnHook = true;
		OnHookFuMoDong.m_DecTime = GetReduceTime();

		if( (GetAutoKillHourNum() - GetOnHookRemainTime()) >= 3600){
			OnHookFuMoDong.m_bOnHourMore = true;
		}else{
			OnHookFuMoDong.m_bOnHourMore = false;
		}


		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnHookFuMoDong);
		m_pActor->OnEvent(msgID,&OnHookFuMoDong,sizeof(OnHookFuMoDong));

		m_FuMoDongData.m_AccelNumOfDay++;

		if(m_FuMoDongData.m_EndOnHookTime <= CURRENT_TIME())
		{
			StopOnHook();
		}

		//同步挂机状态
		SyncOnHookData();
	}

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_AccelKillMonster,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//战斗结束了
void FuMoDongPart::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	MAP_MONSTER::iterator it = m_mapMonster.find(pCombatCnt->CombatID);

	if(it == m_mapMonster.end())
	{
		return;
	}

	SCombatMonster  Info = (*it).second;

	m_mapMonster.erase(it);

	if(pCombatCnt->bIsAutoFighte == false)
	{

		SC_KillMonster_Rsp Rsp;

		Rsp.m_Result = enKMRetCode_OK;
		Rsp.m_Level = Info.m_Level;

		OBuffer4k ob2;
		ob2 << BuildingHeader(enBuildingCmd_KillMonster,sizeof(Rsp)) << Rsp ;


		m_pActor->SendData(ob2.TakeOsb());
	}

	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(Info.m_uidMonster);

	if(pMonster== 0)
	{
		return;
	}


	//if(pCombatResult->m_GetExp>0)
	//{
	if( m_pActor->GetThingClass() == enThing_Class_Actor){
		//给出战角色经验
		((IActor *)m_pActor)->CombatActorAddExp(this->GetCombatExpNum(), true);
	}
	//}

	//杀死怪物

	if(pCombatResult->m_bWin)
	{

		//发布事件
		SS_KillMonster KillMonster;
		KillMonster.m_MonsterUID = Info.m_uidMonster.ToUint64();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_KillMonster);
		m_pActor->OnEvent(msgID,&KillMonster,sizeof(KillMonster));
	}



	g_pGameServer->GetGameWorld()->DestroyThing(Info.m_uidMonster);


	SS_AttackFuMoDong AttackFuMoDong;
	AttackFuMoDong.m_Floor = Info.m_Level;
	AttackFuMoDong.m_bWin  = pCombatResult->m_bWin;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AttackFuMoDong);
	m_pActor->OnEvent(msgID,&AttackFuMoDong,sizeof(AttackFuMoDong));

}



enKMRetCode FuMoDongPart::CombatWithNpc(UINT8 level )
{
	//创建怪物
	const SFuMoDongCnfg* pFuMoDongCnfg =  g_pGameServer->GetConfigServer()->GetFuMoDongCnfg(level);
	if(pFuMoDongCnfg==0)
	{
		TRACE("<error> %s : %d Line 找不到伏魔洞配置信息 level = %d",__FUNCTION__,__LINE__,level);
		return enKMRetCode_ErrCombat;
	}

	enKMRetCode Result = enKMRetCode_OK;

	TSceneID sceneID;
	sceneID.From(m_pActor->GetCrtProp(enCrtProp_ActorFuMoDongSceneID));


	SCreateMonsterContext  MonsterCnt;

	MonsterCnt.MonsterID = pFuMoDongCnfg->m_MonsterID;
	MonsterCnt.nDir      = 0;
	MonsterCnt.SceneID   = INVALID_SCENE_ID;
	MonsterCnt.ptLoc.x   = 0;
	MonsterCnt.ptLoc.y   = 0;

	MonsterCnt.m_nLineup   = 0;
	MonsterCnt.m_CombatIndex   = pFuMoDongCnfg->m_CombatIndex;
	MonsterCnt.m_DropID    = pFuMoDongCnfg->m_DropID;

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();

	char szTemp[1024]={0};		

	strncpy(szTemp,g_pGameServer->GetGameWorld()->GetLanguageStr(pFuMoDongCnfg->m_LanguageID),sizeof(szTemp));
	//if( level == 0){
	//	sprintf(szTemp,g_pGameServer->GetGameWorld()->GetLanguageStr(10006));
	//}else{

	//	sprintf(szTemp,g_pGameServer->GetGameWorld()->GetLanguageStr(10043),level);
	//}

	UINT64 CombatID;



	IMonster * pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);
	if(pMonster==0)
	{
		Result = enKMRetCode_ErrCreateMonsterFail;
	}
	else
	{	

		if(pCombatPart->CombatWithNpc(enCombatType_FuMoDong,pMonster->GetUID(),CombatID,this,szTemp)==false)
		{
			Result = enKMRetCode_ErrCombat;
		}		
	}


	if(Result != enKMRetCode_OK)
	{
		SC_KillMonster_Rsp Rsp;

		Rsp.m_Result = Result;
		Rsp.m_Level = level;

		OBuffer4k ob2;
		ob2 << BuildingHeader(enBuildingCmd_KillMonster,sizeof(Rsp)) << Rsp ;	

		m_pActor->SendData(ob2.TakeOsb());
	}
	else
	{
		SCombatMonster Info;
		Info.m_Level = level;
		Info.m_uidMonster = pMonster->GetUID();
		m_mapMonster[CombatID] = Info;
	}

	return Result;
}

//手动打怪
void FuMoDongPart::KillMonster(CS_KillMonster_Req & Req)
{
	bool bWin = false;

	OBuffer4k ob;

	SC_KillMonster_Rsp Rsp;

	Rsp.m_Result = enKMRetCode_OK;
	Rsp.m_Level = Req.m_Level;

	const SFuMoDongCnfg* pFuMoDong = g_pGameServer->GetConfigServer()->GetFuMoDongCnfg(Req.m_Level);
	if( 0 == pFuMoDong){
		TRACE("<error> %s : %d 行,找不到此层的伏魔洞配置文件!!,层数=%d", __FUNCTION__, __LINE__, Req.m_Level);
		return;
	}

	if( m_pActor->GetCrtProp(enCrtProp_Level) < pFuMoDong->m_ActorLevel)
	{
		//级别不足，不能进入
		Rsp.m_Result = enDKRetCode_ErrLevelLimit;
	}
	else if ( m_pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		Rsp.m_Result = enKMRetCode_ErrInDuoBao;
	}
	else if ( m_pActor->HaveTeam())
	{
		//组队中，无法进入
		Rsp.m_Result = enKMRetCode_ErrHaveTeam;		
	}
	else if ( m_pActor->IsInFastWaitTeam() )
	{
		Rsp.m_Result = enKMRetCode_ErrWaitTeam;
	}
	else if ( m_pActor->GetIsInCombat() )
	{
		//防止客户端快速按多次，直接return
		return;
	}

	ob << BuildingHeader(enBuildingCmd_KillMonster,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if ( Rsp.m_Result == enKMRetCode_OK )
	{
		CombatWithNpc(Req.m_Level);	
	}
}


//清除今天挂机加速的使用次数
void FuMoDongPart::ClearAccellNum()
{
	m_FuMoDongData.m_AccelNumOfDay = 0;
	m_FuMoDongData.m_LastAccelTime = CURRENT_TIME();
}

void FuMoDongPart::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enFuMoDongPartTIMER_ID_GiveExp:
		{
			this->GiveAutoForward();
		}
		break;
	default:
		break;
	}
}

//定时给予经验
INT32 FuMoDongPart::GiveExp()
{
	INT32 nCurTime = CURRENT_TIME();

	if(nCurTime> m_FuMoDongData.m_EndOnHookTime)
	{
		nCurTime =  m_FuMoDongData.m_EndOnHookTime;
	}
	INT32 nDiffTime = nCurTime - m_FuMoDongData.m_LastGiveExpTime;
	INT32 TimeSpace = GetGiveExpTimeSpace();

	INT32 OldExp = m_pActor->GetCrtProp(enCrtProp_ActorExp);
	INT32 NewExp = 0;

	INT32 exp = 0;

	if(nDiffTime>=TimeSpace) //时间到，或者不足一分钟
	{		
		INT32 interval = (nDiffTime/*+TimeSpace-1*/)/TimeSpace;

		exp = interval * CalculateExpPerTimes();

		//增加出战角色经验
		m_pActor->CombatActorAddExp(exp, true, &NewExp);

		m_FuMoDongData.m_LastGiveExpTime += interval*TimeSpace;
	}
	/*
	//3小时增加聚灵气
	if()
	{
	const SPolyNimbusCnfg * pPolyNimbusConfig = g_pGameServer->GetConfigServer()->GetPolyNimbusCnfg(1);
	if( 0 == pPolyNimbusConfig){
	TRACE("<error> %s ; %d 行 获取语言类型配置数据出错!!语言ID = 1", __FUNCTION__, __LINE__);
	return;
	}
	pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus,nAddExp, pNewExp);
	}
	*/
	if(nCurTime >= m_FuMoDongData.m_EndOnHookTime)
	{
		StopOnHook();
	}

	return NewExp - OldExp;
}

//定时给予掉落
void FuMoDongPart::GiveDropGoods(std::vector<TGoodsID> & vectGoods)
{
	const SFuMoDongCnfg* pFuMoDonCnfg = g_pGameServer->GetConfigServer()->GetFuMoDongCnfg(m_FuMoDongData.m_Level);
	if( 0 == pFuMoDonCnfg){
		TRACE("<error> %s : %d 行 获取不到伏魔洞的配置信息!!!层数=%d", __FUNCTION__, __LINE__, m_FuMoDongData.m_Level);
		return;
	}

	//掉落
	if( pFuMoDonCnfg->m_DropID != 0)
	{
		g_pGameServer->GetGoodsServer()->GiveUserDropGoods(m_pActor,pFuMoDonCnfg->m_DropID,vectGoods);
	}

	for( int i = 0; i < vectGoods.size(); ++i)
	{
		m_pActor->SendGoodsCnfg(vectGoods[i]);
	}
}

//定时给予自动打怪奖励
void FuMoDongPart::GiveAutoForward()
{
	SC_AutoKillMonsterForward Rsp;

	//经验
	Rsp.m_GetExp = this->GiveExp();

	//掉落
	std::vector<TGoodsID> vectGoods;
	this->GiveDropGoods(vectGoods);

	Rsp.m_Num = vectGoods.size();

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SC_GiveAutoKillForword,sizeof(Rsp) + Rsp.m_Num * sizeof(TGoodsID)) << Rsp;

	for( int i = 0; i < vectGoods.size(); ++i)
	{
		ob << vectGoods[i];
	}

	m_pActor->SendData(ob.TakeOsb());
}

//启动给予经验定时器
void FuMoDongPart::StartGiveExpTimer()
{
	//启动定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enFuMoDongPartTIMER_ID_GiveExp,this,GetGiveExpTimeSpace()*1000,"FuMoDongPart::StartGiveExpTimer[enFuMoDongPartTIMER_ID_GiveExp]");	
}

//计算每次可以获得的经验
INT32 FuMoDongPart::CalculateExpPerTimes()
{
	INT32 Exp = m_FuMoDongData.m_GiveExp;

	//	Exp += (Exp * m_pActor->GetVipValue(enVipType_AutoKillAddExp) / 100.0f + 0.99999);

	return Exp * m_pActor->GetWallowFactor();
}

//获得给予经验时间间隔
INT32 FuMoDongPart::GetGiveExpTimeSpace()
{
	return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_FuMoDongGetExpTimeSpace;
}

//获得每天最大加速次数
INT32 FuMoDongPart::GetMaxAccelNumOfDay()
{
	return 5 + m_pActor->GetVipValue(enVipType_AccelKillAddHour);
}

//获得每次加速需要消耗的仙石
INT32 FuMoDongPart::GetConsumeMoney()
{
	INT32 nNeedMoney = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_AccelKilNeedMoney;

	nNeedMoney -= m_pActor->GetVipValue(enVipType_AccelKillDesMoney);

	if ( nNeedMoney < 0){

		nNeedMoney = 0;
	}

	return nNeedMoney;
}

//获得每次加速获得的经验
INT32 FuMoDongPart::GetAccelExp()
{
	return CalculateExpPerTimes() * 70;
}

//获得每次加速减少的挂机时间
INT32 FuMoDongPart::GetReduceTime()
{
	return 3600;
}


//获得当天加速次数
INT32 FuMoDongPart::GetAccelNumOfDay()
{
	INT32 nCurTime = CURRENT_TIME();
	if(!XDateTime::GetInstance().IsSameDay(nCurTime,m_FuMoDongData.m_LastAccelTime))
	{
		m_FuMoDongData.m_AccelNumOfDay = 0;
		m_FuMoDongData.m_LastAccelTime = nCurTime;
	}

	return m_FuMoDongData.m_AccelNumOfDay;
}

//同步挂机数据
void FuMoDongPart::SyncOnHookData()
{

	SC_OnHookData_Sync Rsp;

	Rsp.m_RemainingTime = GetOnHookRemainTime();

	Rsp.m_AccelNum = GetMaxAccelNumOfDay()-GetAccelNumOfDay();

	Rsp.m_Price = GetConsumeMoney();

	Rsp.m_Level = m_FuMoDongData.m_Level;

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SyncOnHookData,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//结束挂机状态
void FuMoDongPart::StopOnHook()
{
	//发布事件
	SS_OnHookFuMoDong OnHookFuMoDong;
	OnHookFuMoDong.m_bAccel = true;
	OnHookFuMoDong.m_bOnHook = true;
	OnHookFuMoDong.m_DecTime = 0;

	if( (GetAutoKillHourNum() - GetOnHookRemainTime()) >= 3600){
		OnHookFuMoDong.m_bOnHourMore = true;
	}else{
		OnHookFuMoDong.m_bOnHourMore = false;
	}

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnHookFuMoDong);
	m_pActor->OnEvent(msgID,&OnHookFuMoDong,sizeof(OnHookFuMoDong));

	//////////////////////////////////////////////////

	m_FuMoDongData.m_EndOnHookTime = 0;
	m_FuMoDongData.m_LastGiveExpTime = 0;
	m_FuMoDongData.m_Level = 0;
	g_pGameServer->GetTimeAxis()->KillTimer(enFuMoDongPartTIMER_ID_GiveExp,this);
}

//得到伏魔洞可挂机时长
UINT32	FuMoDongPart::GetAutoKillHourNum()
{
	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return ConfigParam.m_FuMoDongOnHookTimeSpace;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(m_pActor->GetUID());
	if( 0 == pSyndicate){
		return ConfigParam.m_FuMoDongOnHookTimeSpace;
	}

	return (pSyndicate->GetWelfareValue(enWelfare_OnHookTime) * 3600) > 0 ? (pSyndicate->GetWelfareValue(enWelfare_OnHookTime) * 3600) : ConfigParam.m_FuMoDongOnHookTimeSpace;
}

void FuMoDongPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if (msgID == EventData.m_MsgID){

		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		if (0 == pActorDoing)
			return;

		if (m_FuMoDongData.m_EndOnHookTime != 0)
			//玩家在挂机
			pActorDoing->m_ActorDoing = enActorDoing_OnHook;
	}
}

//掉线，下线要做的一些事
void  FuMoDongPart::LeaveDoSomeThing()
{
	if ( 0 != m_FuMoDongData.m_EndOnHookTime )
		return;

	ITaskPart * pTaskPart = m_pActor->GetTaskPart();

	if ( 0 == pTaskPart )
		return;

	if ( pTaskPart->IsInGuide() )
		return;

	const SFuMoDongCnfg* pFuMoDongCnfg = g_pGameServer->GetConfigServer()->GetFuMoDongCnfgByUserLv(m_pActor->GetCrtProp(enCrtProp_Level));

	if ( 0 == pFuMoDongCnfg ){
		TRACE("<error> %s : %d Line 获取伏魔洞配置信息出错！！玩家等级 %d", __FUNCTION__, __LINE__, m_pActor->GetCrtProp(enCrtProp_Level));
		return;
	}

	this->BeginAutoKillMonster(pFuMoDongCnfg);
}

//自动打怪
bool FuMoDongPart::BeginAutoKillMonster(const SFuMoDongCnfg* pFuMoDongCnfg)
{
	if ( 0 == pFuMoDongCnfg )
		return false;

	ICombatPart * pCombatPart = m_pActor->GetCombatPart();
	if (0 == pCombatPart){
		return false;
	}

	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(pFuMoDongCnfg->m_MonsterID);
	if (pMonsterCnfg == 0){
		TRACE("<error> %s : %d Line 获取不到怪物信息！！！，怪物ID : %d",  __FUNCTION__, __LINE__, pFuMoDongCnfg->m_MonsterID);
		return false;
	}

	m_FuMoDongData.m_LastGiveExpTime = CURRENT_TIME();
	m_FuMoDongData.m_EndOnHookTime =  m_FuMoDongData.m_LastGiveExpTime+this->GetAutoKillHourNum();
	m_FuMoDongData.m_Level  = pFuMoDongCnfg->m_Level;
	m_FuMoDongData.m_GiveExp = pCombatPart->CalculateExp(m_pActor->GetCrtProp(enCrtProp_Level), pMonsterCnfg->m_Level);

	StartGiveExpTimer();

	SyncOnHookData();

	//发布事件
	SS_OnHookFuMoDong OnHookFuMoDong;
	OnHookFuMoDong.m_bAccel = false;
	OnHookFuMoDong.m_bOnHook = true;
	OnHookFuMoDong.m_DecTime = 0;

	if( (GetAutoKillHourNum() - GetOnHookRemainTime()) >= 3600){
		OnHookFuMoDong.m_bOnHourMore = true;
	}else{
		OnHookFuMoDong.m_bOnHourMore = false;
	}

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnHookFuMoDong);
	m_pActor->OnEvent(msgID,&OnHookFuMoDong,sizeof(OnHookFuMoDong));

	return true;
}

//打怪获得经验数
UINT32 FuMoDongPart::GetCombatExpNum()
{
	double exp = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_FuMoDongCombatGetExp*m_pActor->GetMultipExpFactor() + 0.99999;


	return exp;
}
