#include "TalismanWorldFuBen.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IGameScene.h"
#include "ITalismanPart.h"
#include "IMonster.h"
#include "XDateTime.h"
#include "IDouFaPart.h"

TalismanWorldFuBen::TalismanWorldFuBen()
{


	const std::map<UINT8,	std::vector<STalismanWorldInfo>> * pAllTalismanWorld = g_pGameServer->GetConfigServer()->GetAllTalismanWorldInfo();

	if ( 0 == pAllTalismanWorld )
		return;

	std::map<UINT8,	std::vector<STalismanWorldInfo>>::const_iterator iter = (*pAllTalismanWorld).begin();

	for ( ; iter != (*pAllTalismanWorld).end(); ++iter )
	{
		VECT_SCENEID vecScene;

		VECT_REPLACEFLOOR vecReplace;

		MAP_TWCREATUREINFO mapTWReplace;

		const std::vector<STalismanWorldInfo> & vecTWInfo = iter->second;

		for ( int i = 0; i < vecTWInfo.size(); ++i )
		{
			MAP_REPLACEINFO mapReplace;

			vecReplace.push_back(mapReplace);
		}

		m_vecScene.push_back(vecScene);

		m_vecReplaceInfo.push_back(vecReplace);

		m_TWCreatureInfo.push_back(mapTWReplace);
	}
}

bool	TalismanWorldFuBen::Create(IGameWorld * pGameWorld)
{
	//一次创建所有法宝世界场景
	const std::map<UINT8,	std::vector<STalismanWorldInfo>> * pAllTalismanWorld = g_pGameServer->GetConfigServer()->GetAllTalismanWorldInfo();

	if ( 0 == pAllTalismanWorld )
		return false;

	int index = 0;

	std::map<UINT8,	std::vector<STalismanWorldInfo>>::const_iterator iter = (*pAllTalismanWorld).begin();

	for ( ; iter != (*pAllTalismanWorld).end(); ++iter )
	{
		UINT8 level = iter->first;

		if ( m_vecScene.size() <= level )
			continue;

		VECT_SCENEID & vecScene = m_vecScene[level];

		const std::vector<STalismanWorldInfo> & vecTW = iter->second;

		MAP_TWCREATUREINFO & mapTWInfo = m_TWCreatureInfo[level];

		for ( int floor = 0; floor < vecTW.size(); ++floor )
		{
			const STalismanWorldInfo & TWInfo = vecTW[floor];

			//创建场景
			IGameScene * pGameScene = pGameWorld->CreateGameSceneByMapID(TWInfo.m_MapID);

			if ( 0 == pGameScene ){
				TRACE("<error> %s : %d Line 创建玄天摆怪场景失败！！地图ID = %d", __FUNCTION__, __LINE__, TWInfo.m_MapID);
				continue;
			}

			vecScene.push_back(pGameScene->GetSceneID());

			const std::vector<SMonsterOutput> * pvecMonstOutput = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(TWInfo.m_MapID);

			if ( 0 == pvecMonstOutput )
			{
				TRACE("<error> %s ; %d Line 得到法宝世界摆怪场景怪物地图分布失败！！MapID = %d", __FUNCTION__, __LINE__, TWInfo.m_MapID);
				return false;
			}

			//得到场景的所有怪物
			std::vector<UID> vecMonster = pGameScene->GetAllMonster();

			for ( int i = 0; i < vecMonster.size(); ++i )
			{
				IMonster * pMonster = pGameWorld->FindMonster(vecMonster[i]);

				if ( 0 == pMonster ){
					continue;
				}

				TMonsterID MonsterID= pMonster->GetCrtProp(enCrtProp_MonsterID);

				const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MonsterID);

				if ( 0 == pMonsterCnfg ){
					TRACE("<error> %s : %d Line 获取不到怪物配置信息！！怪物ID = %d", __FUNCTION__, __LINE__, MonsterID);
					continue;
				}

				//可替换的
				TWCreatureInfo TWInfo;

				TWInfo.m_uidCreature = vecMonster[i];
				TWInfo.m_Level		 = level;
				TWInfo.m_Floor		 = floor;
				TWInfo.m_bCanReplace = pMonster->GetCrtProp(enCrtProp_MonsterCanReplace);

				for ( int pos = 0; pos < (*pvecMonstOutput).size(); ++pos )
				{
					const SMonsterOutput & Output = (*pvecMonstOutput)[pos];

					if ( Output.m_MonsterID == pMonsterCnfg->m_MonsterID && Output.m_PointX == pMonster->GetCrtProp(enCrtProp_PointX) && Output.m_PointY == pMonster->GetCrtProp(enCrtProp_PointY) )
					{
						TWInfo.m_Index = Output.m_nLineup;
						break;
					}
				}

				mapTWInfo[vecMonster[i]] = TWInfo;
			}
		}
	}

	//监听玩家进场景消息
	//UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_EnterScene);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return false;
	}

	//pEnventServer->AddListener((IEventListener *)this, msgID, enEventSrcType_Actor,0,"TalismanWorldFuBen::Create");

	//增加对占领玩家卸载的否决
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	pEnventServer->AddListener((IVoteListener *)this, msgID, enEventSrcType_Actor, 0, "TalismanWorldFuBen::Create"); 

	//获取玩家的占领信息
	SDB_Get_TalismanWorld_Req DBReq;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Get_TalismanWorld,ob.TakeOsb(),this,0);

	return true;
}

bool	TalismanWorldFuBen::OnSetDBData()
{
	return true;
}

//进入法宝世界
void	TalismanWorldFuBen::EnterTalismanWorld(IActor * pActor, UINT8 floor)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	const OccupationInfo * pOccupatInfo = pTalismanPart->GetOccupationInfo();

	if ( 0 == pOccupatInfo )
		return;

	//const STalismanWorldParam *  pTWParam = g_pGameServer->GetConfigServer()->GetTalismanWorldParam(pActor->GetCrtProp(enCrtProp_Level));

	INT8 TWLevel = g_pGameServer->GetConfigServer()->GetEnterTWLevel(pActor->GetCrtProp(enCrtProp_Level));

	SC_EnterTW Rsp;

	if ( pOccupatInfo->m_uidMonster.IsValid() )
	{
		//已经有占领了，不能再法宝世界
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrOccupation;
	}
	else if ( !pTalismanPart->Check_EnterTalismanWorldNum() )
	{
		//次数已用完
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrNoEnterNum;
	}
	else if ( TWLevel == INVALID_TWLevel )
	{
		//等级不足
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrLevel;
	}
	else if ( pActor->HaveTeam() )
	{
		//组队中，无法进入
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrHaveTeam;
	}
	else if ( pActor->IsInDuoBao() )
	{
		//无法进入，正在匹配夺宝战
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrInDuoBao;
	}
	else if ( pActor->IsInFastWaitTeam() )
	{
		//等待组队中，无法操作
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrWaitTeam;
	}
	else
	{
		//UINT8  TWLevel = pTWParam->m_Level;

		Rsp.m_Level = TWLevel;

		//OK,进场景
		if ( m_vecScene.size() <= TWLevel || m_vecScene[TWLevel].size() <= floor )
		{
			TRACE("<error> %s : %d 越界！！级别%d,层数%d",  __FUNCTION__, __LINE__, TWLevel, floor);
			return;
		}

		TSceneID SceneID = m_vecScene[TWLevel][floor];

		IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

		if ( 0 == pGameScene ){
			TRACE("<error> %s : %d Line 获取法宝世界场景失败！！级别%d,层数%d", __FUNCTION__, __LINE__, TWLevel, floor);
			return;
		}

		if ( !pGameScene->EnterScene(pActor) ){
			return;
		}

		pTalismanPart->SetTWSceneID(SceneID);


		Rsp.m_TotalFloor	   = m_vecScene[TWLevel].size();

		Rsp.m_Floor			   = floor;

		//同步场景内的替换信息
		//this->SynReplaceInfoMap(pActor, pGameScene, TWLevel, floor);		
	}

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_EnterTW,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//战斗
void	TalismanWorldFuBen::Combat(IActor * pActor, UID uidEnemy, UINT8 level, UINT8 floor)
{
	std::map<UID/*挑战者*/, ChallengedInfo/*被挑战者信息*/>::iterator iter = m_InCombat.find(pActor->GetUID());

	if ( iter != m_InCombat.end() ){
		TRACE("<warning> %s : %d Line 法宝世界战斗！！挑战者已在法宝世界战斗中！！", __FUNCTION__, __LINE__);
		return;
	}

	SC_TWCombat Rsp;

	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart )
		return;

	const OccupationInfo * pOccupatInfo = pTalismanPart->GetOccupationInfo();

	if ( 0 == pOccupatInfo )
		return;

	//挑选对手，首选目标为玩家点击的
	TWCreatureInfo * pCreatureInfo = this->SelectCombatEnemy(uidEnemy, level, floor);

	if ( pOccupatInfo->m_uidMonster.IsValid() ){
		//有占领，不可以再挑战
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrOccupationNoCombat;

	} else if ( !pTalismanPart->Check_EnterTalismanWorldNum() ){
		//次数已用完
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrNoEnterNum;
	}else if ( pActor->HaveTeam() ){
		//组队中，无法进入
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrHaveTeam;
	}else if (  pActor->IsInDuoBao()  ){
		//无法进入，正在匹配夺宝战
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrInDuoBao;
	} else if ( pActor->IsInFastWaitTeam() ){
		//等待组队中，无法操作
		Rsp.m_RetCod = enTalismanWorldRetCode_ErrWaitTeam;
	}else if ( 0 == pCreatureInfo ){
		//排队等待战斗
		Rsp.m_RetCod = enTalismanWorldRetCode_WaitCombat;

		this->Push_WaitList(pActor, level, floor);
	}else {
		//进入战斗
		pCreatureInfo->m_bInCombat = true;

		uidEnemy = pCreatureInfo->m_uidOccupater.IsValid() ? pCreatureInfo->m_uidOccupater : pCreatureInfo->m_uidCreature;

		if ( !uidEnemy.IsValid() ){
			TRACE("<error> %s : %d Line 法宝世界寻找战斗对手失败！！级别%d", __FUNCTION__, __LINE__, level);
			return;
		}

		IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidEnemy);

		if ( 0 == pThing )
			return;

		this->BeginCombat(pActor, pThing, pCreatureInfo->m_Level, pCreatureInfo->m_Floor);

		//加入正在战斗队列
		ChallengedInfo Info;

		Info.m_uidUser	   = pCreatureInfo->m_uidOccupater;
		Info.m_uidMonster  = pCreatureInfo->m_uidCreature;
		Info.m_Level	   = pCreatureInfo->m_Level;
		Info.m_Floor	   = pCreatureInfo->m_Floor;

		m_InCombat[pActor->GetUID()] = Info; 
	}

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_Combat,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//战斗结束了
void TalismanWorldFuBen::OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pCombatCnt->uidUser);

	if ( 0 == pActor )
		return;

	std::map<UID/*挑战者*/, ChallengedInfo/*被挑战者信息*/>::iterator iter = m_InCombat.find(pCombatCnt->uidUser);

	if ( iter == m_InCombat.end() )
		return;

	if ( pCombatResult->m_bWin )
	{
		//扣除次数
		ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

		if ( 0 == pTalismanPart )
			return;

		pTalismanPart->AddEnterNum();
	}

	ChallengedInfo & Info = iter->second;

	if ( pCombatResult->m_bWin ){
		//挑战胜利
		UID uidEnemy = Info.m_uidUser.IsValid() ? Info.m_uidUser : Info.m_uidMonster;

		IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(pCombatCnt->uidEnemy);

		if ( 0 == pThing ){
			TRACE("<error> %s : %d Line 法宝世界战斗结束完后找不到敌人！！级别%d,层数%d", __FUNCTION__, __LINE__, Info.m_Level, Info.m_Floor);
			return;
		}

		if ( this->IsCanReplace(Info.m_uidMonster, Info.m_Level) )
		{
			if ( pThing->GetThingClass() == enThing_Class_Actor ){

				//保存战斗回放
				SDB_Update_Occupation_Info Req;
				Req.m_UidUser = pCombatCnt->uidEnemy.ToUint64();
				Req.m_UidEnemy = pCombatCnt->uidSource.ToUint64();
				Req.m_RecordBuf.m_size = pCombatCnt->ob.Size();
				Req.m_BufLen = pCombatCnt->ob.Size();

				OBuffer6k ob;
				ob << Req << pCombatCnt->ob;
				g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_UpdateOccupationInfo, ob.TakeOsb(), 0, 0);

				//清除对手玩家的占领
				g_pThingServer->GetTalismanMgr().OverOccupation((IActor *)pThing, Info.m_uidMonster, Info.m_Level, Info.m_Floor, true);



			}

			//占领
			g_pThingServer->GetTalismanMgr().Occupation(pActor, Info.m_uidMonster, Info.m_Level, Info.m_Floor);			
		}

		//胜利获得灵魄
		INT32 Soul = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_TalismanWorldGhostSoul;

		pActor->AddCrtPropNum(enCrtProp_GhostSoul, Soul);
	}

	MAP_TWCREATUREINFO & mapCreature = m_TWCreatureInfo[Info.m_Level];

	MAP_TWCREATUREINFO::iterator itTW = mapCreature.find(Info.m_uidMonster);

	if ( itTW != mapCreature.end() )
	{
		TWCreatureInfo & TWInfo = itTW->second;

		TWInfo.m_bInCombat = false;
	}

	UINT8 Level = Info.m_Level;
	UINT8 Floor = Info.m_Floor;
	UID uidMonster = Info.m_uidMonster;
	m_InCombat.erase(iter);

	//发事件
	SS_OnCombat EventCombat;

	EventCombat.CombatPlace = enCombatPlace_TalismanWorld;
	EventCombat.bWin = pCombatResult->m_bWin;
	EventCombat.bIsTeam = pCombatCnt->bIsTeam;
	EventCombat.bHard = false;

	UINT32 msgID2 = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnCombat);
	pActor->OnEvent(msgID2,&EventCombat,sizeof(EventCombat));

	//下场战斗
	this->NextCombat(uidMonster, Level, Floor);
}

//占领
bool TalismanWorldFuBen::Occupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor)
{
	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidMonster);

	if ( 0 == pMonster ){
		TRACE("<error> %s : %d Line 占领找不到怪物！！", __FUNCTION__, __LINE__);
		return false;
	}

	if ( m_vecReplaceInfo.size() <= Level || m_vecReplaceInfo[Level].size() <= floor || m_TWCreatureInfo.size() <= Level ){
		TRACE("<error> %s : %d 法宝世界级别有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, floor);
		return false;
	}

	//增加进替换图片信息
	MAP_REPLACEINFO & mapReplaceInfo = m_vecReplaceInfo[Level][floor];
	
	ReplaceInfo Info;

	Info.m_Facade = pActor->GetCrtProp(enCrtProp_ActorFacade);
	strncpy(Info.m_Name, pActor->GetName(), sizeof(Info.m_Name));
	Info.m_uidNpc = uidMonster;
	Info.m_uidUser = pActor->GetUID();
	Info.m_BeginOccupatTime = CURRENT_TIME();

	if ( !mapReplaceInfo.insert(MAP_REPLACEINFO::value_type(uidMonster, Info)).second ){
		return false;
	}

	//通知地图的其它玩家，更新图片
	this->SynReplaceInfoSingle(Info, Level, floor);

	//更新怪物信息
	MAP_TWCREATUREINFO & mapCreatureInfo = m_TWCreatureInfo[Level];

	MAP_TWCREATUREINFO::iterator iter = mapCreatureInfo.find(uidMonster);

	if ( iter == mapCreatureInfo.end() ){
		TRACE("<error> %s : %d Line 获取不到怪物！！级别%d", __FUNCTION__, __LINE__, Level);
		return false;
	}

	TWCreatureInfo & CreatureInfo = iter->second;

	CreatureInfo.m_uidOccupater = pActor->GetUID();

	//数据库保存
	SDB_Save_TalismanWorld_Req DBReq;

	DBReq.m_TLevel = CreatureInfo.m_Level;
	DBReq.m_TFloor = CreatureInfo.m_Floor;
	DBReq.m_Pos	   = CreatureInfo.m_Index;
	DBReq.m_uidOccupater = pActor->GetUID().ToUint64();
	strncpy(DBReq.m_NameOccupater, Info.m_Name, sizeof(DBReq.m_NameOccupater));
	DBReq.m_Facade = Info.m_Facade;
	DBReq.m_BeginOccupTime = Info.m_BeginOccupatTime;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Save_TalismanWorld,ob.TakeOsb(),0,0);

	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 != pTalismanPart )
	{
		pTalismanPart->SaveData();
	}

	return true;
}

//结束占领
bool TalismanWorldFuBen::OverOccupation(IActor * pActor, UID uidMonster, UINT8 Level, UINT8 Floor)
{
	if ( m_vecReplaceInfo.size() <= Level || m_vecReplaceInfo[Level].size() <= Floor ){
		TRACE("<error> %s : %d Line 法宝世界的级别有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return false;
	}

	//删除替换图片信息
	MAP_REPLACEINFO & mapReplaceInfo = m_vecReplaceInfo[Level][Floor];

	MAP_REPLACEINFO::iterator iter = mapReplaceInfo.find(uidMonster);

	if ( iter == mapReplaceInfo.end() ){
		TRACE("<error> %s : %d Line 结束占领找不到占领信息！！怪物UID=%s", __FUNCTION__, __LINE__, uidMonster.ToString());
		return false;
	}

	//通知场景内所有玩家取消替换图片
	this->CancelReplace(uidMonster, Level, Floor);

	mapReplaceInfo.erase(iter);

	//更新怪物信息
	MAP_TWCREATUREINFO & mapCreatureInfo = m_TWCreatureInfo[Level];

	MAP_TWCREATUREINFO::iterator it = mapCreatureInfo.find(uidMonster);

	if ( it == mapCreatureInfo.end() ){
		TRACE("<error> %s : %d Line 获取不到怪物！！级别%d", __FUNCTION__, __LINE__, Level);
		return false;
	}

	TWCreatureInfo & CreatureInfo = it->second;

	CreatureInfo.m_uidOccupater = UID();

	//删除数据库替换
	SDB_Delete_TalismanWorld_Req DBReq;

	DBReq.m_Level = CreatureInfo.m_Level;
	DBReq.m_Floor = CreatureInfo.m_Floor;
	DBReq.m_Pos	  = CreatureInfo.m_Index;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Delete_TalismanWorld,ob.TakeOsb(),0,0);

	return true;
}

//同步整张场景的替换信息给指定玩家
void	TalismanWorldFuBen::SynReplaceInfoMap(IActor * pActor, UINT8 Level, UINT8 Floor)
{
	if ( m_vecReplaceInfo.size() <= Level || m_vecReplaceInfo[Level].size() <= Floor 
		|| m_vecScene.size() <= Level || m_vecScene[Level].size() <= Floor ){
		TRACE("<error> %s : %d Line 法宝世界级别有错！！级别%d, 层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	TSceneID SceneID = m_vecScene[Level][Floor];

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if ( 0 == pGameScene ){
		TRACE("<error> %s : %d Line 获取法宝世界场景失败！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	MAP_REPLACEINFO & mapReplace = m_vecReplaceInfo[Level][Floor];

	SC_ChangePhoto Rsp;

	Rsp.m_Num = mapReplace.size();

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_ChangePhoto,sizeof(Rsp) + Rsp.m_Num * sizeof(ReplaceInfo)) << Rsp;

	MAP_REPLACEINFO::iterator iter = mapReplace.begin();

	for ( ; iter != mapReplace.end(); ++iter )
	{
		ob << iter->second;

		TRACE("法宝世界替换玩家UID:%s", (iter->second).m_uidUser.ToString());
	}

	pActor->SendData(ob.TakeOsb());
}

//同步单个替换信息给场景内所有玩家
void	TalismanWorldFuBen::SynReplaceInfoSingle(const ReplaceInfo & Info, UINT8 Level, UINT8 Floor)
{
	if ( m_vecReplaceInfo.size() <= Level || m_vecReplaceInfo[Level].size() <= Floor 
		|| m_vecScene.size() <= Level || m_vecScene[Level].size() <= Floor )
	{
		TRACE("<error> %s : %d Line 法宝世界级别有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	TSceneID SceneID = m_vecScene[Level][Floor];

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if ( 0 == pGameScene ){
		TRACE("<error> %s : %d Line 法宝世界获取不到场景信息！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	std::vector<UID> vecActor = pGameScene->GetAllClassThing(enThing_Class_Actor);

	if ( vecActor.size() <= 0 )
		return;

	SC_ChangePhoto Rsp;
	OBuffer1k ob;

	Rsp.m_Num = 1;

	for ( int i = 0; i < vecActor.size(); ++i )
	{
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(vecActor[i]);

		if ( 0 == pActor )
			continue;

		if ( pActor->GetMaster() != 0 )
			continue;

		ob.Reset();
		ob << TalismanWorldHeader(enTalismanWorldCmd_ChangePhoto,sizeof(Rsp) + Rsp.m_Num * sizeof(ReplaceInfo)) << Rsp << Info;
		pActor->SendData(ob.TakeOsb());
	}
}

//通知场景内所有玩家取消替换图片
void	TalismanWorldFuBen::CancelReplace(UID uidMonster,UINT8 Level, UINT8 Floor)
{
	if ( m_vecScene.size() <= Level || m_vecScene[Level].size() <= Floor )
	{
		TRACE("<error> %s : %d Line 法宝世界级别有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	TSceneID SceneID = m_vecScene[Level][Floor];

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if ( 0 == pGameScene ){
		TRACE("<error> %s : %d Line 法宝世界获取不到场景信息！！级别%d,层数%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	std::vector<UID> vecActor = pGameScene->GetAllClassThing(enThing_Class_Actor);

	if ( vecActor.size() <= 0 )
		return;

	SC_CancelReplace Rsp;
	OBuffer1k ob;

	Rsp.uidMonster = uidMonster;

	for ( int i = 0; i < vecActor.size(); ++i )
	{
		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(vecActor[i]);

		if ( 0 == pActor )
			continue;

		if ( pActor->GetMaster() != 0 )
			continue;

		ob.Reset();
		ob << TalismanWorldHeader(enTalismanWorldCmd_CancelReplace,sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
	}
}

//取消等待进攻
void	TalismanWorldFuBen::Cancel_WaitCombat(UID uidUser)
{
	std::list<TWWaitUser>::iterator iter = m_listAttacker.begin();

	for ( ; iter != m_listAttacker.end(); ++iter )
	{
		TWWaitUser & WaitUser = *iter;

		if ( WaitUser.m_uidUser == uidUser )
		{
			m_listAttacker.erase(iter);

			SC_CancelWait Rsp;

			Rsp.m_RetCod = enTalismanWorldRetCode_OK;

			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(uidUser);

			if ( 0 != pActor )
			{
				OBuffer1k ob;
				ob << TalismanWorldHeader(enTalismanWorldCmd_CancelWait,sizeof(Rsp)) << Rsp;
				pActor->SendData(ob.TakeOsb());			
			}

			return;
		}
	}
}

void	TalismanWorldFuBen::OnEvent(XEventData & EventData)
{
	//UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS, enMsgID_EnterScene);

	//if( EventData.m_MsgID == msgID){

	//	SS_EnterScene * pEnterScene = (SS_EnterScene *)EventData.m_pContext;

	//	if( 0 == pEnterScene)
	//		return;

	//	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(pEnterScene->m_uidUser));

	//	if ( 0 == pActor )
	//		return;

	//	TSceneID EnterSceneID = pEnterScene->m_NewSceneID;

	//	for ( int i = 0; i < m_vecScene.size(); ++i )
	//	{
	//		VECT_SCENEID & vecSceneID = m_vecScene[i];

	//		for ( int n = 0; n < vecSceneID.size(); ++n )
	//		{
	//			TSceneID SceneID = vecSceneID[n];

	//			if ( SceneID == EnterSceneID ){
	//				//同步场景内的替换信息
	//				this->SynReplaceInfoMap(pActor, i, n);	
	//				return;
	//			}
	//		}
	//	}
	//}
}

bool	TalismanWorldFuBen::OnVote(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

	if ( msgID == EventData.m_MsgID )
	{
		SS_UnloadActorContext * pUnloadActorContext = (SS_UnloadActorContext *)EventData.m_pContext;

		if( 0 == pUnloadActorContext)
		{
			return true;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(pUnloadActorContext->m_uidActor);

		if ( 0 == pActor )
			return true;

		ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

		if ( 0 == pTalismanPart )
			return true;

		const OccupationInfo * pOccupation = pTalismanPart->GetOccupationInfo();

		if ( 0 == pOccupation )
			return true;

		if ( pOccupation->m_uidMonster.IsValid() )
			return false;
	}

	return true;
}

//开始战斗
bool	TalismanWorldFuBen::BeginCombat(IActor * pActor, IThing * pEnemy, UINT8 level, UINT8 floor)
{
	ICombatPart * pCombatPart = pActor->GetCombatPart();

	if ( 0 == pCombatPart )
		return false;

	const STalismanWorldInfo * pTWInfo = g_pGameServer->GetConfigServer()->GetTalismanWorldInfo(level, floor);

	if ( 0 == pTWInfo ){
		TRACE("<error> %s ; %d Line 获取法宝世界配置出错！！级别%d,层数%d", __FUNCTION__, __LINE__, level, floor);
		return false;
	}

	const STalismanWorldParam * pTWParam = g_pGameServer->GetConfigServer()->GetTalismanWorldParam(floor);

	if ( 0 == pTWParam )
	{
		TRACE("<error> %s : %d Line 获取法宝世界配置出错！！层数%d", __FUNCTION__, __LINE__, floor);
		return false;
	}

	if ( enThing_Class_Actor == pEnemy->GetThingClass() )
	{
		//和玩家战斗
		const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(pTWInfo->m_CombatMapID);

		if ( 0 == pMapConfig ){
			TRACE("<error> %s : %d Line 获取地图配置信息出错！！地图ID = %d", __FUNCTION__, __LINE__, pTWInfo->m_CombatMapID);
			return false;
		}

		//创建战斗场景
		IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(pTWInfo->m_CombatMapID);

		if ( 0 == pGameScene ){
			TRACE("<error> %s : %d Line 创建法宝世界战斗场景失败！！地图ID = %d", __FUNCTION__, __LINE__, pTWInfo->m_CombatMapID);
			return false;
		}

		UINT64 CombatID;

		//开始战斗
		pCombatPart->CombatWithScene(enCombatType_Talisman, pEnemy->GetUID(), CombatID, this, g_pGameServer->GetGameWorld()->GetLanguageStr(pTWParam->m_LangID), pGameScene);
	}
	else if ( enThing_Class_Monster == pEnemy->GetThingClass() )
	{
		//与怪物战斗
		UINT64 CombatID;

		//开始战斗
		pCombatPart->CombatWithNpc(enCombatType_Talisman, pEnemy->GetUID(), CombatID, this, g_pGameServer->GetGameWorld()->GetLanguageStr(pTWParam->m_LangID));
	}
	else
	{
		TRACE("<error> %s : %d Line 法宝世界战斗错误！！对手的事物分类为%d", __FUNCTION__, __LINE__, pEnemy->GetThingClass());
		return false;
	}

	return true;
}

//挑选一个和玩家打,首选玩家点击的那只，若在战斗则选别的
TWCreatureInfo * TalismanWorldFuBen::SelectCombatEnemy(UID uidEnemy, UINT8 Level, UINT8 Floor)
{
	MAP_TWCREATUREINFO & mapTWCreatureInfo = m_TWCreatureInfo[Level];

	MAP_TWCREATUREINFO::iterator iter = mapTWCreatureInfo.find(uidEnemy);

	if ( iter == mapTWCreatureInfo.end() ){
		TRACE("<error> %s ; %d Line 法宝世界中找不到玩家点击的那个可替换怪物！！UID:%s,级别%d", __FUNCTION__, __LINE__, uidEnemy.ToString(),Level);
		return 0;
	}

	TWCreatureInfo & Info = iter->second;

	if ( !Info.m_bCanReplace ){
		//不可替换的
		return &Info;
	}

	if ( !Info.m_bInCombat ){
		//可以战斗
		return &Info;
	}

	//在战斗中，则选择一个不在战斗的其它怪物和玩家战斗
	iter = mapTWCreatureInfo.begin();

	for ( ; iter != mapTWCreatureInfo.end(); ++iter )
	{
		TWCreatureInfo & CreatureInfo = iter->second;

		if ( !CreatureInfo.m_bInCombat && CreatureInfo.m_bCanReplace && g_pGameServer->GetConfigServer()->GetTalismanWorldParam(CreatureInfo.m_Floor) == g_pGameServer->GetConfigServer()->GetTalismanWorldParam(Floor) )
			return &CreatureInfo;
	}

	return 0;
}

//加入等待进攻队列
void			TalismanWorldFuBen::Push_WaitList(IActor * pActor,UINT8 Level, UINT8 Floor)
{
	SC_PopWaitCombat Rsp;

	std::list<TWWaitUser>::iterator iter = m_listAttacker.begin();
	
	bool bOk = true;

	for (; iter != m_listAttacker.end(); ++iter )
	{
		TWWaitUser & WaitUser = *iter;

		if ( WaitUser.m_uidUser == pActor->GetUID() )
		{
			bOk = false;
			break;
		}
	}

	if ( bOk )
	{
		TWWaitUser WaitUser;

		WaitUser.m_uidUser = pActor->GetUID();
		WaitUser.m_Level = Level;
		WaitUser.m_Floor = Floor;

		m_listAttacker.push_back(WaitUser);

		Rsp.m_Pos = m_listAttacker.size();	
	}

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_SC_PopWaitCombat,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//取出等待进攻队列的队首玩家
UID				TalismanWorldFuBen::Pop_WaitList(UINT8 Level, UINT8 Floor)
{
	if ( m_listAttacker.size() <= 0 )
		return UID();

	std::list<TWWaitUser>::iterator iter = m_listAttacker.begin();

	for ( ; iter != m_listAttacker.end(); ++iter )
	{
		TWWaitUser & WaitUser = *iter;

		if ( WaitUser.m_Level == Level && g_pGameServer->GetConfigServer()->GetTalismanWorldParam(WaitUser.m_Floor) == g_pGameServer->GetConfigServer()->GetTalismanWorldParam(Floor) )
		{
			UID uidUser = WaitUser.m_uidUser;
			m_listAttacker.erase(iter);
			return uidUser;
		}
	}

	return UID();
}

//得到被玩家占领的怪物UID
UID				TalismanWorldFuBen::GetMonsterOccupated(IActor * pActor)
{
	ITalismanPart * pTalismanPart = pActor->GetTalismanPart();

	if ( 0 == pTalismanPart ){
		return UID();
	}

	const OccupationInfo * pOccupatInfo = pTalismanPart->GetOccupationInfo();

	if ( 0 == pOccupatInfo ){
		return UID();
	}

	return pOccupatInfo->m_uidMonster;
}

//进行下一场战斗，从进攻队列中选择进攻者
void			TalismanWorldFuBen::NextCombat(UID uidMonster, UINT8 level, UINT8 floor)
{
	//查看进攻队列，是否有下场战斗
	UID uidAttacker = this->Pop_WaitList(level, floor);

	if ( !uidAttacker.IsValid() )
		return;

	//进入战斗
	IActor * pAttacker = g_pGameServer->GetGameWorld()->FindActor(uidAttacker);

	if ( 0 == pAttacker )
		//下线了
		return;

	MAP_TWCREATUREINFO & mapCreatureInfo = m_TWCreatureInfo[level];

	MAP_TWCREATUREINFO::iterator iter = mapCreatureInfo.find(uidMonster);

	if ( iter == mapCreatureInfo.end() ){
		TRACE("<error> %s : %d Line 找不到法宝世界怪物！！级别%d,层数%d", __FUNCTION__, __LINE__, level, floor);
		return;
	}

	TWCreatureInfo & Info = iter->second;

	UID uidEnemy = Info.m_uidOccupater.IsValid() ? Info.m_uidOccupater : Info.m_uidCreature;

	IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidEnemy);

	if ( 0 == pThing )
		return;

	ChallengedInfo ChaInfo;

	ChaInfo.m_Floor = Info.m_Floor;
	ChaInfo.m_Level = Info.m_Level;
	ChaInfo.m_uidMonster = Info.m_uidCreature;
	ChaInfo.m_uidUser = Info.m_uidOccupater;

	m_InCombat[pAttacker->GetUID()] = ChaInfo;

	this->BeginCombat(pAttacker, pThing, level, floor);
}


//保存
//void		TalismanWorldFuBen::SaveData()
//{
//	//先删除先前数据
//	SDB_Delete_TalismanWorld_Req DBReq;
//
//	OBuffer1k ob;
//	ob << DBReq;
//	g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Delete_TalismanWorld,ob.TakeOsb(),0,0);
//
//	//保存当前数据
//	for ( int i = 0; i < m_TWCreatureInfo.size(); ++i )
//	{
//		MAP_TWCREATUREINFO & mapCreature = m_TWCreatureInfo[i];
//
//		MAP_TWCREATUREINFO::iterator iter = mapCreature.begin();
//
//		for ( ; iter != mapCreature.end(); ++iter )
//		{
//			TWCreatureInfo & Info = iter->second;
//
//			if ( Info.m_Level >= m_vecReplaceInfo.size() || Info.m_Floor >= m_vecReplaceInfo[Info.m_Level].size() )
//			{
//				TRACE("<error> %s ; %d Line 法宝世界级别有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Info.m_Level, Info.m_Floor);
//				continue;
//			}
//
//			SDB_Save_TalismanWorld_Req DBSaveReq;
//
//			DBSaveReq.m_TLevel = Info.m_Level;
//			DBSaveReq.m_TFloor = Info.m_Floor;
//			DBSaveReq.m_Pos	   = Info.m_Index;
//
//			MAP_REPLACEINFO & mapReplace = m_vecReplaceInfo[Info.m_Level][Info.m_Floor];
//
//			MAP_REPLACEINFO::iterator it = mapReplace.find(Info.m_uidCreature);
//
//			if ( it != mapReplace.end() )
//			{
//				ReplaceInfo & RepInfo = it->second;
//
//				DBSaveReq.m_uidOccupater = RepInfo.m_uidUser.ToUint64();
//				strncpy(DBSaveReq.m_NameOccupater, RepInfo.m_Name, sizeof(DBSaveReq.m_NameOccupater));
//				DBSaveReq.m_Facade = RepInfo.m_Facade;
//				DBSaveReq.m_BeginOccupTime = RepInfo.m_BeginOccupatTime;
//			}
//			else
//			{
//				DBSaveReq.m_uidOccupater = UID().ToUint64();
//				MEM_ZERO(&DBSaveReq.m_NameOccupater);
//				DBSaveReq.m_Facade = 0;
//				DBSaveReq.m_BeginOccupTime = 0;				
//			}
//
//			OBuffer1k ob;
//			ob << DBReq;
//			g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Save_TalismanWorld,ob.TakeOsb(),0,0);
//		}
//	}
//}

void		TalismanWorldFuBen::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
		case enDBCmd_Get_TalismanWorld:
			{
				this->HandleGetTalismanWorldInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
		case enDBCmd_Get_ChallengeRecordBuf:
			{
				this->HandleGetTalismanWorldRecordBuf(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break; 
		case enDBCmd_GetOccupationInfo:
			{
				this->HandleGetOccupationInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break; 
		default:
			{
				TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
			}
			break;
	}
}

//关服后，玩家上线获取玩家的占领信息
void		TalismanWorldFuBen::GetUserOccupation(UID uidUser, UINT8 Level, UINT8 Floor, UID & uidMonster, UINT32 & OccupatTime)
{
	if ( Level >= m_vecReplaceInfo.size() ||  Floor >= m_vecReplaceInfo[Level].size() )
	{
		TRACE("<error> %s : %d 玩家的占领数据有错！！,级别：%d, 层数：%d", __FUNCTION__, __LINE__, Level, Floor);
		return;
	}

	MAP_REPLACEINFO & mapReplace = m_vecReplaceInfo[Level][Floor];

	MAP_REPLACEINFO::iterator iter = mapReplace.begin();

	for ( ; iter != mapReplace.end(); ++iter )
	{
		ReplaceInfo & Info = iter->second;

		if ( Info.m_uidUser == uidUser )
		{
			uidMonster = Info.m_uidNpc;
			OccupatTime = Info.m_BeginOccupatTime;
			return;
		}
	}
}

//得到一个怪物是否是可替换的
bool		TalismanWorldFuBen::IsCanReplace(UID uidMonster, UINT8 Level)
{
	if ( Level >= m_TWCreatureInfo.size() )
	{
		TRACE("<error> %s ; %d Line 法宝世界级别有错！！级别%d", __FUNCTION__, __LINE__, Level);
		return false;
	}

	MAP_TWCREATUREINFO & mapCreature = m_TWCreatureInfo[Level];

	MAP_TWCREATUREINFO::iterator iter = mapCreature.find(uidMonster);

	if ( iter == mapCreature.end() )
	{
		TRACE("<error> %s : %d Line 获取法宝世界怪物错误！！", __FUNCTION__, __LINE__);
		return false;
	}

	TWCreatureInfo & CreatureInfo = iter->second;

	return CreatureInfo.m_bCanReplace;
}

//得到法宝世界数据库信息
void		TalismanWorldFuBen::HandleGetTalismanWorldInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
		return;

	UINT32 CurTime = CURRENT_TIME();

	int num = RspIb.Remain() / sizeof(SDB_TalismanWorld);

	for ( int i = 0; i < num; ++i )
	{
		SDB_TalismanWorld Data;

		RspIb >> Data;

		if(RspIb.Error())
		{
			TRACE("<error> %s : %d DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		if ( Data.m_TLevel >= m_TWCreatureInfo.size() )
		{
			TRACE("<error> %s : %d Line 从数据库得到的数据错误！！法宝世界级别%d", __FUNCTION__, __LINE__, Data.m_TLevel);
			return;
		}

		UINT32 TimeNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_OccupationTimeNum;

		if ( Data.m_BeginOccupTime + TimeNum <= CurTime )
		{
			//占领已结束
			//删除数据库替换
			SDB_Delete_TalismanWorld_Req DBReq;

			DBReq.m_Level = Data.m_TLevel;
			DBReq.m_Floor = Data.m_TFloor;
			DBReq.m_Pos	  = Data.m_Pos;

			OBuffer1k ob;
			ob << DBReq;
			g_pGameServer->GetDBProxyClient()->Request(TALISMAN_WORLD_DB,enDBCmd_Delete_TalismanWorld,ob.TakeOsb(),0,0);
			continue;
		}

		MAP_TWCREATUREINFO & mapCreate = m_TWCreatureInfo[Data.m_TLevel];

		MAP_TWCREATUREINFO::iterator iter = mapCreate.begin();

		for ( ; iter != mapCreate.end(); ++iter )
		{
			TWCreatureInfo & Info = iter->second;

			if ( Info.m_Floor != Data.m_TFloor )
				continue;

			if ( Info.m_Index != Data.m_Pos )
			{
				continue;
			}

			Info.m_uidOccupater = UID(Data.m_uidOccupater);

			ReplaceInfo RepInfo;

			RepInfo.m_Facade = Data.m_Facade;
			strncpy(RepInfo.m_Name, Data.m_NameOccupater, sizeof(RepInfo.m_Name));
			RepInfo.m_uidNpc = UID(Info.m_uidCreature);
			RepInfo.m_uidUser = UID(Data.m_uidOccupater);
			RepInfo.m_BeginOccupatTime = Data.m_BeginOccupTime;

			if ( Info.m_Level >= m_vecReplaceInfo.size() || Info.m_Floor >= m_vecReplaceInfo[Info.m_Level].size() )
			{
				TRACE("<error> %s ; %d Line 法宝世界级别或者层数有错！！级别%d,层数%d", __FUNCTION__, __LINE__, Info.m_Level, Info.m_Floor);
				return;
			}

			MAP_REPLACEINFO & mapReplace = m_vecReplaceInfo[Info.m_Level][Info.m_Floor];

			mapReplace[RepInfo.m_uidNpc] = RepInfo;

			//占领者加载进内存
			g_pGameServer->LoadActor(RepInfo.m_uidUser);
		}
	}
}

//得到法宝世界占领时被击败的战斗回放
void	TalismanWorldFuBen::HandleGetTalismanWorldRecordBuf(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_ViewRec Rsp;
	OBuffer6k ob;
	
	if( OutParam.retCode != 0){
		Rsp.m_Result = enTalismanWorldRetCode_ErrNoRec;
		ob <<  TalismanWorldHeader(enTalismanWorldCmd_ViewRec, sizeof(Rsp)) << Rsp;
	}else{
		SDB_ChallengeRecordBuf DBChallengeRecordBuf;
		RspIb >> DBChallengeRecordBuf;
		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}
		
		ob.Push((const unsigned char*)DBChallengeRecordBuf.m_RecordBuf,DBChallengeRecordBuf.m_BufLen);		
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	
	pActor->SendData(ob.TakeOsb());
}

//得到法宝世界占领时被击败的信息
void TalismanWorldFuBen::HandleGetOccupationInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if( RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}



	SDB_OccupationInfo OccupationInfo;
	RspIb >> OccupationInfo;
	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}


	SC_OccupationInfo Rsp;
	Rsp.m_GhostSoul = OccupationInfo.m_GhostSoul;
	Rsp.m_RecIndex = OccupationInfo.m_Rank;
	strncpy(Rsp.m_EnemyName,OccupationInfo.m_EnemyName,sizeof(Rsp.m_EnemyName));
	Rsp.m_VipLevel = OccupationInfo.m_VipLevel;

	OBuffer1k ob;
	ob <<  TalismanWorldHeader(enTalismanWorldCmd_GetOccupationInfo, sizeof(Rsp) ) << Rsp;

	pActor->SendData(ob.TakeOsb());
	
}

//查看回放
void TalismanWorldFuBen::ViewRec(IActor * pActor,UINT32 m_Rank)
{
	SDB_Get_ChallengeRecordBuf DBReq;
	DBReq.m_Rank = m_Rank;


	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_Get_ChallengeRecordBuf, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
}

//从数据库获取占领被打败的信息
void TalismanWorldFuBen::GetPreOccupationInfo(IActor * pActor)
{
	SDB_Get_OccupationInfo DBReq;
	DBReq.m_uidUser = pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_GetOccupationInfo, ob.TakeOsb(), this, pActor->GetUID().m_uid);
}

