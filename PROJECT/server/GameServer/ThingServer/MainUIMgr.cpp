
#include "MainUIMgr.h"
#include "IFuBenPart.h"
#include "ThingServer.h"
#include "IActor.h"
#include "IBasicService.h"
#include "GameSrvProtocol.h"
#include "ITalismanPart.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "IGodSword.h"
#include "IConfigServer.h"
#include "IActorBasicPart.h"
#include "IFuBenPart.h"
#include "ResetGodSwordFuBen.h"
#include "XDateTime.h"
#include "IGoodsServer.h"
#include "IActivityPart.h"
#include <set>
#include "ITeamPart.h"

MainUIMgr::MainUIMgr()
{
}

MainUIMgr::~MainUIMgr()
{
}

bool MainUIMgr::Create()
{
	if ( !m_XuanTianFuBen.Create() ){
		return false;
	}

	//设置定时器,晚上0点0分0秒更新仙剑副本进度
	time_t nCurTime = CURRENT_TIME();
	tm * pTm = localtime(&nCurTime);

	UINT32 LeftTime = XDateTime::SECOND_OF_DAY - (pTm->tm_hour * 3600 + pTm->tm_min * 60 + pTm->tm_sec); //XDateTime::SECOND_OF_DAY;

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_ResetGodSwordFuBen,this,LeftTime * 1000,"MainUIMgr::Create");

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_MainUI,this);
}

void MainUIMgr::Close()
{
}


//收到MSG_ROOT消息
void MainUIMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
		typedef  void (MainUIMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enMainUICmd_Max] = 
	{
		&MainUIMgr::OnOpenFuBen,
		&MainUIMgr::OnChallengeFuBen,
		&MainUIMgr::OnResetFuBen,
		&MainUIMgr::OnSpawnTalismanGoods,
		&MainUIMgr::OnTakeTalismanGoods,
		&MainUIMgr::OnUpgradeTalismanQuality,
		&MainUIMgr::OnUpgradeSword,
		&MainUIMgr::OnEnterSwordFuBen,
		&MainUIMgr::OpenGodSword,
		NULL,
		&MainUIMgr::OnIdentity,
		&MainUIMgr::NewPlayerGuideIndex,
		&MainUIMgr::NewPlayerGuideConfirm,
		&MainUIMgr::ViewFuBenDropGoods,
		&MainUIMgr::CommonFuBenAutoCombat,
		&MainUIMgr::OpenViewDrop,
		NULL,
		NULL,
		&MainUIMgr::SelectSynProgress,
		&MainUIMgr::OpenXuanTian,
		&MainUIMgr::ShowXTForward,
		&MainUIMgr::AttackXTBoss,
		&MainUIMgr::MoneyLessAttackTime,
		&MainUIMgr::TeamChallgeFuBen,
		&MainUIMgr::ViewXTDamageRank,
		&MainUIMgr::ViewLastKill,



	};

	
	 if(nCmd >= enMainUICmd_Max || 0 == s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}

void MainUIMgr::OnTimer(UINT32 timerID)
{
	if( enTimerID_ResetGodSwordFuBen == timerID){
		
		ResetGodSwordFuBen GodSwordFuBen;

		g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, GodSwordFuBen);

		static bool bFirst = true;

		if( true == bFirst){
			//第一次要删除旧定时器,重新设置定时器,因为第一次设置的定时器时间是从开服时间到0点剩余时间
			g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_ResetGodSwordFuBen, this);

			g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_ResetGodSwordFuBen, this, XDateTime::SECOND_OF_DAY * 1000, "MainUIMgr::OnTimer");

			bFirst = false;
		}
	}	
}

//打开副本
void MainUIMgr::OnOpenFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	pFuBenPart->OpenFuBen();
}

//挑战副本
void MainUIMgr::OnChallengeFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_MainUIChallenge_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pFuBenPart->ChallengeFuBen(Req.m_FuBenID);
}

//重置副本
void MainUIMgr::OnResetFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_MainUIReset_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pFuBenPart->ResetFuBen(Req.m_FuBenID);
}

//领取法宝孕育物
void MainUIMgr::OnTakeTalismanGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();
	if(pTalismanPart==0)
	{
		return ;
	}

	CS_TakeTalismanGoods_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pTalismanPart->TakeTalismanSpawnGoods(Req.m_uidTalisman);
}


	//孕育法宝物
void MainUIMgr::OnSpawnTalismanGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();
	if(pTalismanPart==0)
	{
		return ;
	}

	CS_SpawnTalismanGoods_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pTalismanPart->SpawnTalismanSpawnGoods(Req.m_uidTalisman);
}

	//提升法宝品质
void MainUIMgr::OnUpgradeTalismanQuality(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();
	if(pTalismanPart==0)
	{
		return ;
	}

	CS_UpgradeTalismanQuality_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}


	pTalismanPart->UpgradeTalismanQuality(Req.m_uidTalisman,Req.m_gidProp);
}


//仙剑升级
void MainUIMgr::OnUpgradeSword(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_UpgradeSword_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}
	
	IGoodsServer  * pGoodsServer = g_pGameServer->GetGoodsServer();
	if( 0 == pGoodsServer){
		return;
	}

	IGoods * pGoods = pGoodsServer->GetGoodsFromPacketOrEquipPanel(pActor, Req.m_uidGoods);

	CS_UpgradeSword_Rsp Rsp;
	Rsp.m_uidGoods = Req.m_uidGoods;
	Rsp.m_Result = enMainUICode_OK;

	IGodSword * pGodSword = (IGodSword *)pGoods;

	if(pGoods == 0)
	{
		Rsp.m_Result = enMainUICode_ErrNoGoods;
	}
	else if(pGoods->GetGoodsClass() != enGoodsCategory_GodSword)
	{
		Rsp.m_Result = enMainUICode_ErrNotGodSword;
	}
	else
	{
		//当前等级
		INT32 nCurLevel = 0;

		pGodSword->GetPropNum(enGoodsProp_SwordLevel,nCurLevel);

		//上限
		INT32 nLevelUp = pGodSword->GetGoodsCnfg()->m_AvoidOrSwordLvMax;
		
		const SGodSwordLevelCnfg * pSwordLevelCnfg = g_pGameServer->GetConfigServer()->GetGodSwordLevelCnfg(nCurLevel+1);

		//当前仙剑灵气
		INT32 GodSwordNimbus = 0;
		GodSwordNimbus = pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus);

		if(nCurLevel >= nLevelUp)
		{
			Rsp.m_Result = enMainUICode_ErrLevelLimit;
		}
		else if(pSwordLevelCnfg == 0)
		{
			TRACE("<error> %s : %d line 找不到仙剑级别配置信息 level = %d ",__FUNCTION__,__LINE__,nCurLevel+1);
			Rsp.m_Result = enMainUICode_ErrLevelLimit;
		}
		else if(pSwordLevelCnfg->m_vectParam.size() < 2)
		{
			TRACE("<error> %s : %d line 仙剑级别配置信息参数有错！！level = %d,参数个数 = %d", __FUNCTION__,__LINE__,nCurLevel+1,pSwordLevelCnfg->m_vectParam.size());
			return;
		}
		else if( !pPacketPart->HaveGoods(pSwordLevelCnfg->m_vectParam[0], pSwordLevelCnfg->m_vectParam[1]))
		{
			Rsp.m_Result = enMainUICode_ErrGoods;
		}
		else if(GodSwordNimbus < pSwordLevelCnfg->m_NeedNimbus)
		{
			Rsp.m_Result = enMainUICode_ErrNotNimbus;
		}
		else
		{
			pActor->AddCrtPropNum(enCrtProp_ActorGodSwordNimbus, -pSwordLevelCnfg->m_NeedNimbus);

			pPacketPart->DestroyGoods(pSwordLevelCnfg->m_vectParam[0], pSwordLevelCnfg->m_vectParam[1]);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,pSwordLevelCnfg->m_vectParam[0],UID(),pSwordLevelCnfg->m_vectParam[1],"仙剑升级消耗材料");

			if(pGodSword->UpgradeSword(pActor)==false)
			{
				Rsp.m_Result = enMainUICode_ErrUpgradeFail;
			}
			else
			{
				//设为绑定
				int bBind = 0;

				pGodSword->GetPropNum(enGoodsProp_Bind, bBind);

				if ( !bBind )
				{
					pGodSword->SetPropNum(pActor, enGoodsProp_Bind, 1);
				}		
			}
		}
	}

	 OBuffer1k ob;

	 ob << SMainUIHeader(enMainUICmd_UpgradeSword,SIZE_OF(Rsp)) << Rsp;

	 pActor->SendData(ob.TakeOsb());

}


//进入仙剑副本
void  MainUIMgr::OnEnterSwordFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_EnterGodSwordFuBen_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pFuBenPart->OnEnterSwordFuBen(Req.m_uidGodSword,Req.m_Level);
}

//打开仙剑面板
void  MainUIMgr::OpenGodSword(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if( 0 == pFuBenPart){
		return;
	}

	SC_OpenGodSword_Rsp  Rsp;

	Rsp.m_GodSwordNimbus	= pActor->GetCrtProp(enCrtProp_ActorGodSwordNimbus);
	Rsp.m_MaxGodSwordNimbus = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;
	Rsp.m_EnterGodSwordFuBenNum = pFuBenPart->GetEnterGodSwordNum();
	Rsp.m_MaxVipEnterNum    = pActor->GetVipValue(enVipType_AddGodSwordWorldNum);

	 OBuffer1k ob;
	 ob << SMainUIHeader(enMainUICmd_OpenGodSword,SIZE_OF(Rsp)) << Rsp;
	 pActor->SendData(ob.TakeOsb());
}

//认证通过了
void MainUIMgr::OnIdentity(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
}

void MainUIMgr::NewPlayerGuideConfirm(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	 OBuffer1k ob;
	 ob << SMainUIHeader(enMainUICmd_GuideConfirm,0);
	 pActor->SendData(ob.TakeOsb());
}

//新手引导步骤
void MainUIMgr::NewPlayerGuideIndex(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IActivityPart * pActivityPart = pActor->GetActivityPart();
	if(pActivityPart==0)
	{
		return ;
	}

	//CS_NewPlayerGuide Req;

	std::string strContext;

	ib >> strContext;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pActivityPart->SetNewPlayerGuideContext(strContext.c_str());

}

//查看普通副本掉落物品
void MainUIMgr::ViewFuBenDropGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewCommonFuBenDrop Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_ViewCommonFuBenDrop Rsp;

	OBuffer4k ob;

	UINT16 DropID = g_pGameServer->GetConfigServer()->GetMapBossDropID(Req.m_MapID, Req.m_bOpenHardType);
		
	const std::vector<SDropGoods> * pvectGoods = g_pGameServer->GetConfigServer()->GetDropGoodsCnfg(DropID);

	if ( 0 != pvectGoods){

		std::set<TGoodsID> setGoodsID;

		for(int index = 0; index < (*pvectGoods).size(); ++index)
		{
			const SDropGoods & DropGoods = (*pvectGoods)[index];

			for( int k = 0; k + 2 < DropGoods.m_vectDropGoods.size(); k += 3)
			{
				setGoodsID.insert(DropGoods.m_vectDropGoods[k]);
			}
		}

		std::set<TGoodsID>::iterator iter = setGoodsID.begin();

		for ( ; iter != setGoodsID.end(); ++iter)
		{
			const TGoodsID & GoodsID = *iter;

			ob << GoodsID;

			pActor->SendGoodsCnfg(GoodsID);

			++Rsp.m_Num;
		}		
	}


	OBuffer4k ob2;
	ob2 << SMainUIHeader(enMainUICmd_ViewDropGoods,SIZE_OF(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){

		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}

//普通副本自动快速打怪
void MainUIMgr::CommonFuBenAutoCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_CommonFuBenAutoCombat Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pFuBenPart->CommonFuBenAutoCombat(Req.m_FuBenID, Req.m_bOpenHardType);	
}

//打开查看掉落界面
void MainUIMgr::OpenViewDrop(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_OpenViewDrop Req;

	ib >> Req;

	if ( ib.Error()){
		
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	const SFuBenCnfg * pFuBenCnfg = g_pGameServer->GetConfigServer()->GetFuBenCnfg(Req.m_FuBenID);

	if ( 0 == pFuBenCnfg){

		TRACE("<error> %s : %d 行 找不到副本配置信息!!副本ID = %d", __FUNCTION__, __LINE__, Req.m_FuBenID);
		return;
	}

	SC_OpenViewDrop Rsp;

	if (  pFuBenCnfg->m_Type == enFuBenType_Common && pFuBenCnfg->m_EnterLevel >= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_LvNumCommonAndHardFB){
		
		Rsp.m_bCanOpenHardType = true;
	}

	OBuffer1k ob;

	for ( int i = 0; i < pFuBenCnfg->m_MapID.size(); ++i)
	{
		TMapID	MapID = pFuBenCnfg->m_MapID[i];

		const std::vector<SMonsterOutput> * pvecMonster = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(MapID);

		if ( 0 == pvecMonster){

			continue;
		}

		for ( int i = 0; i < (*pvecMonster).size(); ++i)
		{
			const SMonsterOutput & MonsterOutput =(*pvecMonster)[i];

			if ( MonsterOutput.m_MonsterType != enMonsterType_Boss){

				continue;
			}

				
			TMonsterID BossID = MonsterOutput.m_MonsterID;

			const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(BossID);

			if ( 0 == pMonsterCnfg){
				
				TRACE("<error> %s : %d LINE 获取怪物配置信息出错！！怪物ID = %d", __FUNCTION__, __LINE__, BossID);
				continue;
			}

			BossInfo BSInfo;

			BSInfo.m_MapID = MapID;
			strncpy(BSInfo.m_Name, pMonsterCnfg->m_szName, sizeof(BSInfo.m_Name));

			++Rsp.m_Num;

			ob << BSInfo;
		}
	}

	OBuffer1k ob2;

	ob2 << SMainUIHeader(enMainUICmd_OpenViewDrop,SIZE_OF(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){

		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}

//选择是否同步进度
void MainUIMgr::SelectSynProgress(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		return;
	}

	IActor * pMember = pTeamPart->GetTeamMember();

	if ( 0 == pMember ){
		return;
	}

	CS_SelectSycRet Req;

	ib >> Req;

	if ( ib.Error()){
		
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	SC_SelectSycRet Rsp;

	if ( Req.m_bOk ){
		//选择同步进度,挑战副本
		IActor * pLeader = pTeamPart->GetTeamLeader();

		if ( 0 == pLeader ){
			return;
		}

		IFuBenPart * pFuBenPart = pLeader->GetFuBenPart();

		if ( 0 == pFuBenPart ){
			return;
		}

		pFuBenPart->TeamChallengeFuBen(Req.m_FuBenID, true);
		
	} else {
		//发给队友，说明不同步
		Rsp.m_RetCode = enMainUICode_NoSycProgress;

		OBuffer1k ob;
		ob << SMainUIHeader(enMainUICmd_SelectSycRet,sizeof(Rsp)) << Rsp;
		pMember->SendData(ob.TakeOsb());
	}

	//发送给自己OK
	Rsp.m_RetCode = enMainUICode_OK;

	OBuffer1k ob;
	ob << SMainUIHeader(enMainUICmd_SelectSycRet,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	//通知对友取消等待框
	SC_PopWaitSyc RspPop;

	RspPop.m_bOpen = false;

	ob.Reset();
	ob << SMainUIHeader(enMainUICmd_PopWaitSyc,sizeof(Rsp)) << Rsp;
	pMember->SendData(ob.TakeOsb());
}

//打开玄天页面
void MainUIMgr::OpenXuanTian(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_XuanTianFuBen.OpenXuanTiam(pActor);
}

//显示玄天奖励
void MainUIMgr::ShowXTForward(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ShowXuanTianForward Req;

	ib >> Req;

	if ( ib.Error()){
		
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_XuanTianFuBen.ShowXTForward(pActor, Req.m_Rank);
}

//攻击玄天BOSS
void MainUIMgr::AttackXTBoss(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_AttackXTBoss Req;

	ib >> Req;

	if ( ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_XuanTianFuBen.AttackBoss(pActor, Req.m_uidBoss);
}

//使用仙石缩短玄天再次攻击时间
void MainUIMgr::MoneyLessAttackTime(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_XuanTianFuBen.MoneyLessAttackTime(pActor);
}

//获取玄天开启剩余时间
INT32 MainUIMgr::GetRemainTimeOpenXT()
{
	return m_XuanTianFuBen.GetRemainOpenXT();
}

//组队挑战副本
void MainUIMgr::TeamChallgeFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuBenPart * pFuBenPart = pActor->GetFuBenPart();
	if(pFuBenPart==0)
	{
		return ;
	}

	CS_TeamChallengeFuBen Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	
	pFuBenPart->TeamChallengeFuBen(Req.m_FuBenID);
}

//显示玄天伤害排行
void MainUIMgr::ViewXTDamageRank(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewXTDamageRank Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	m_XuanTianFuBen.ShowDamageRank(pActor, Req.m_index);
}

//显示最后一击
void MainUIMgr::ViewLastKill(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	m_XuanTianFuBen.ShowLastKill(pActor);
}

//玩家保存玄天数据
void MainUIMgr::SaveXTData(IActor * pActor)
{
	m_XuanTianFuBen.SaveUserDamage(pActor);
}

//重新加载玄天怪物信息
void MainUIMgr::ReloadXTMonsterCnfg()
{
	m_XuanTianFuBen.ReloadXTMonsterCnfg();
}
