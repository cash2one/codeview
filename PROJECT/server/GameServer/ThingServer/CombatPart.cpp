
#include "CombatPart.h"
#include "ThingServer.h"
#include "IActor.h"
#include "IGameServer.h"
#include "IGameWorld.h"
#include "RandomService.h"
#include "ICreature.h"
#include "IMagic.h"
#include "IMonster.h"
#include "IMagicPart.h"
#include "IEquipPart.h"
#include "ITalisman.h"
#include "GodSword.h"
#include "DMsgSubAction.h"
#include "IGameScene.h"
#include "IStatusPart.h"
#include "IGoodsServer.h"
#include "DMsgSubAction.h"
#include "IBasicService.h"
#include "IActor.h"
#include "ITeamPart.h"
#include "Combat.h"
#include "ICombatServer.h"

CombatPart::CombatPart()
{
	m_pActor = 0;
	
	m_bNeedSave = false;

	m_bTeamCombat = false;

}
CombatPart::~CombatPart()
{
	
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool CombatPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	m_pActor = (ICreature*)pMaster;

	if(pMaster->GetThingClass() == enThing_Class_Actor)
	{
		if(pContext==0 || nLen<sizeof(SDBCombatData))
		{
			return false;
		}

		SDBCombatData * pDBCombatData = (SDBCombatData *)pContext;

		m_DBCombatData = * pDBCombatData;
	}

	//判断主角是否在其中
	if(m_pActor->GetThingClass() == enThing_Class_Actor)
	{
		IActor * pActor = (IActor*)m_pActor;
		if(pActor->GetMaster()==0)
		{
			//主角是否在其中
			bool bFound = false;

			for(int i=0; i<ARRAY_SIZE( m_DBCombatData.m_uidLineup); ++i)
			{
				if(m_DBCombatData.m_uidLineup[i]==pActor->GetUID())
				{
					bFound = true;
					break;
				}
			}

			if(bFound==false)
			{
				m_DBCombatData.m_uidLineup[0] = pActor->GetUID();
			}
		}	   
	}

	//注册事件,得到玩家在干什么
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);	

	m_pActor->SubscribeEvent(msgID,this,"CombatPart::Create");

	return true;
}

//释放
void CombatPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart CombatPart::GetPartID(void)
{
	return enThingPart_Crt_Combat;
}

//取得本身生物
IThing*		CombatPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool CombatPart::OnGetDBContext(void * buf, int &nLen)
{
	if(m_pActor->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	IActor * pActor = (IActor*)m_pActor;

	if(pActor->GetMaster()!=0)
	{
		return true;
	}


	SDB_Update_CombatData_Req CombatData_Req ;
	CombatData_Req.Uid_User = m_pActor->GetUID().ToUint64();
	CombatData_Req.uidLineup1 = m_DBCombatData.m_uidLineup[0].ToUint64();;
	CombatData_Req.uidLineup2 = m_DBCombatData.m_uidLineup[1].ToUint64();
	CombatData_Req.uidLineup3 = m_DBCombatData.m_uidLineup[2].ToUint64();

	OBuffer1k ob;
	ob << CombatData_Req;

	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateCombatInfo,ob.TakeOsb(),0,0);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void CombatPart::InitPrivateClient()
{
	for( int i = 0; i < ARRAY_SIZE(m_DBCombatData.m_uidLineup); ++i)
	{
		if( !m_DBCombatData.m_uidLineup[i].IsValid()){
			continue;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_DBCombatData.m_uidLineup[i]);
		if( 0 == pActor){
			m_DBCombatData.m_uidLineup[i] = UID();
		}
	}
	

	LineupSync();

	if ( m_pActor->GetThingClass() == enThing_Class_Actor )
	{
		IActor * pActor = (IActor *)m_pActor;

		if ( pActor->GetMaster() == 0 )
		{
			if ( !m_bTeamCombat ){
				//设置默认组队参战状态
				m_bTeamCombat = true;

				int num = 1;

				for ( int i = 0; i < ARRAY_SIZE(m_DBCombatData.m_uidLineup) && num < MAX_TEAMCOMBAT_NUM; ++i )
				{
					IActor * pEmployee = g_pGameServer->GetGameWorld()->FindActor(m_DBCombatData.m_uidLineup[i]);

					if ( 0 == pEmployee || pActor == pEmployee )
						continue;

					ICombatPart * pCombatPart = pEmployee->GetCombatPart();

					if ( 0 != pCombatPart ){
						pCombatPart->SetTeamCombatStatus(true, false);
						++num;
					}
				}
			}

			this->SycTeamCombatState();
		}
	}
}

void CombatPart::LineupSync()
{
	if(m_pActor->GetThingClass() !=enThing_Class_Actor )
	{
		return;
	}

	if(((IActor*)m_pActor)->GetMaster() != 0)
	{
		return;
	}

	SC_EquipLineup_Sync Rsp;

	memcpy(Rsp.m_uidLineup,m_DBCombatData.m_uidLineup,sizeof(Rsp.m_uidLineup));


	OBuffer4k ob;

	ob << Equip_Header(enEquipCmd_LineupSync,sizeof(Rsp)) << Rsp;

	if(m_pActor->GetThingClass() == enThing_Class_Actor)
	{
		((IActor*)m_pActor)->SendData(ob.TakeOsb());
	}
}
//玩家下线了，需要关闭该ThingPart
void CombatPart::Close()
{
}

//保存数据
void CombatPart::SaveData()
{
	if(m_pActor->GetThingClass() != enThing_Class_Actor || m_bNeedSave == false)
	{
		return ;
	}

	IActor * pActor = (IActor*)m_pActor;

	if(pActor->GetMaster()!=0)
	{
		return ;
	}


	SDB_Update_CombatData_Req CombatData_Req ;
	CombatData_Req.Uid_User = m_pActor->GetUID().ToUint64();
	CombatData_Req.uidLineup1 = m_DBCombatData.m_uidLineup[0].ToUint64();;
	CombatData_Req.uidLineup2 = m_DBCombatData.m_uidLineup[1].ToUint64();
	CombatData_Req.uidLineup3 = m_DBCombatData.m_uidLineup[2].ToUint64();
	CombatData_Req.uidLineup4 = m_DBCombatData.m_uidLineup[3].ToUint64();
	CombatData_Req.uidLineup5 = m_DBCombatData.m_uidLineup[4].ToUint64();
	CombatData_Req.uidLineup6 = m_DBCombatData.m_uidLineup[5].ToUint64();
	CombatData_Req.uidLineup7 = m_DBCombatData.m_uidLineup[6].ToUint64();
	CombatData_Req.uidLineup8 = m_DBCombatData.m_uidLineup[7].ToUint64();
	CombatData_Req.uidLineup9 = m_DBCombatData.m_uidLineup[8].ToUint64();

	OBuffer1k ob;
	ob << CombatData_Req;

	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateCombatInfo,ob.TakeOsb(),0,0);

	m_bNeedSave = false;

}

void CombatPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if (msgID == EventData.m_MsgID){

		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		if (0 == pActorDoing)
			return;	
	}
}

//指定地点战斗
bool CombatPart::CombatWithScene(enCombatType CombatType,UID uidEnemy,UINT64 & CombatID,ICombatObserver * pCombatObserver, const char* szSceneName,
								 IGameScene * pGameScene, enJoinPlayer JoinPlayer,enCombatMode CombatMode ,
								 UINT8 ChiefIndex,UINT16 DropID,UINT32 Param)
{
   Combat * pCombat = new Combat(Param);
	
	if(pCombat->CombatWithScene(CombatType,(IActor*)m_pActor,uidEnemy,pCombatObserver,szSceneName,pGameScene,JoinPlayer,CombatMode,ChiefIndex,DropID)==false)
	{
		delete pCombat;
		return false;
	}


	CombatID = pCombat->GetCombatID();

	g_pGameServer->GetCombatServer()->AddCombat(pCombat);

	return true;
}




//当前是否可以战斗
bool CombatPart::CanCombat()
{
	return true;
}

    //获得战斗怪及战位信息
  std::vector<SCreatureLineupInfo>  CombatPart::GetCombatMapMonster(UINT8 ChiefIndex,IGameScene * pGameScene)
  {
	  std::vector<SCreatureLineupInfo> vect;

	  return vect;
  }

//创建战斗怪
bool CombatPart::CreateCombatMapMonster(IGameScene * pGameScene,const SCombatMapMonster * pCombatMapMonster,TMonsterID MonsterID,INT32 nBlood)
{

	SCreateMonsterContext MonsterCnt;

	MonsterCnt.m_DropID = 0;
	MonsterCnt.m_nLineup = 0;
	MonsterCnt.MonsterID = 0;
	MonsterCnt.nDir = 0;
	MonsterCnt.ptLoc.x = MonsterCnt.ptLoc.y = 0;
	MonsterCnt.SceneID = pGameScene->GetSceneID();

	IMonster * pMonster = 0;

	if(pCombatMapMonster->m_MonsterID1 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 0;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID1;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID2 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 1;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID2;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID3 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup =2;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID3;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID4 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 3;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID4;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID5 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 4;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID5;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID6 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 5;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID6;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	if(pCombatMapMonster->m_MonsterID7 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 6;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID7;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}


	if(pCombatMapMonster->m_MonsterID8 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 7;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID8;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}


	if(pCombatMapMonster->m_MonsterID9 != INVALID_MONSTER_ID)
	{
		MonsterCnt.m_nLineup = 8;
		MonsterCnt.MonsterID = pCombatMapMonster->m_MonsterID9;
		pMonster = g_pGameServer->GetGameWorld()->CreateMonster(MonsterCnt);	
		if(MonsterID==MonsterCnt.MonsterID && nBlood!=0)
		{
			pMonster->SetCrtProp(enCrtProp_Blood,nBlood);
		}
	}

	return true;
}

//和怪物战斗  bOpenHardType为是否开启困难模式
bool  CombatPart::CombatWithNpc(enCombatType CombatType,UID  uidNpc, UINT64 & CombatID,ICombatObserver * pCombatObserver, const char* szSceneName,
								enCombatIndexMode CombatIndexMode,enJoinPlayer JoinPlayer,enCombatMode CombatMode, UINT32 Param)
{

	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidNpc);

	if(pMonster == 0)
	{
		return false;
	}


	  TSceneID  SceneID;
	   SceneID.From(pMonster->GetCrtProp(enCrtProp_SceneID));
	//获取地图掉落
		const SMapConfigInfo * pMapInfo = g_pGameServer->GetConfigServer()->GetMapConfigInfo(SceneID.GetMapID());

	UINT32 CombatIndex = 0;

	UINT32 DropID = 0;

	if ( enCombatIndexMode_Com == CombatIndexMode )
	{
		//普通战斗模式
		CombatIndex = pMonster->GetCrtProp(enCrtProp_MonsterCombatIndex);
		DropID = pMonster->GetCrtProp(enCrtProp_MonsterDropID);
		if(DropID==0 && pMapInfo)
		{
			DropID = pMapInfo->m_DropID;
		}
	}
	else if ( enCombatIndexMode_Hard == CombatIndexMode )
	{
		//困难战斗模式
		CombatIndex = pMonster->GetCrtProp(enCrtProp_MonsterCombatIndexHard);
		DropID = pMonster->GetCrtProp(enCrtProp_MonsterDropIDHard);
		if(DropID==0 && pMapInfo)
		{
			DropID = pMapInfo->m_DropIDHard;
		}
	}
	else if ( enCombatIndexMode_Team == CombatIndexMode )
	{
		//组队战斗模式
		CombatIndex = pMonster->GetCrtProp(enCrtProp_MonsterCombatIndexTeam);
		DropID = pMonster->GetCrtProp(enCrtProp_MonsterDropIDTeam);
		if(DropID==0 && pMapInfo)
		{
			DropID = pMapInfo->m_DropIDHard;
		}
	}
	else
	{
		TRACE("<error> %s : %d Line 战斗索引模式错误！！模式%d", __FUNCTION__, __LINE__, CombatIndexMode);
		return false;
	}

	//动态创建怪物
	const SCombatMapMonster * pCombatMapMonster  = g_pGameServer->GetConfigServer()->GetCombatMapMonster(CombatIndex);

	if(pCombatMapMonster == 0)
	{
		return false;
	}
	
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(pCombatMapMonster->m_CombatMapID,0,true);

	if(pGameScene == 0)
	{
		return false;
	}	

	if(CombatType == enCombatType_XuanTian)
	{
		CreateCombatMapMonster(pGameScene,pCombatMapMonster,pMonster->GetCrtProp(enCrtProp_MonsterID),pMonster->GetCrtProp(enCrtProp_Blood));
	}
	else
	{
		CreateCombatMapMonster(pGameScene,pCombatMapMonster);
	}


	return CombatWithScene(CombatType,uidNpc,CombatID,pCombatObserver,szSceneName,pGameScene, JoinPlayer,CombatMode,pCombatMapMonster->m_MainMonsterIndex,DropID,Param);

}

//当杀死了怪物
void  CombatPart::OnKillMonster(UID uidNpc)
{
	IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(uidNpc);

	if(pMonster == 0)
	{
		return ;
	}

	//杀怪给予经验
	INT32 exp = CalculateExp(pMonster);
	if(exp>0)
	{
		if( m_pActor->GetThingClass() == enThing_Class_Actor){
			//给出战角色经验
			((IActor *)m_pActor)->CombatActorAddExp(exp, true);
		}
	}

	//杀死怪物

	g_pGameServer->GetGameWorld()->DestroyThing(uidNpc);

	//发布事件
	SS_KillMonster KillMonster;
	KillMonster.m_MonsterUID = uidNpc.ToUint64();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_KillMonster);
	m_pActor->OnEvent(msgID,&KillMonster,sizeof(KillMonster));
}

//计算打怪可以获得的经验
INT32 CombatPart::CalculateExp(ICreature * pMonster)
{
	//角色等级
	INT32 ActorLevel = m_pActor->GetCrtProp(enCrtProp_Level);

	//怪物等级
	INT32 NpcLevel = pMonster->GetCrtProp(enCrtProp_Level);

	return this->CalculateExp(ActorLevel, NpcLevel);
}

//计算打怪可以获得的经验
INT32 CombatPart::CalculateExp(UINT8 ActorLevel, UINT8 NpcLevel)
{
	float exp = 0;

	INT32 expParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_CombatExpParam;

	if(ActorLevel == NpcLevel )
	{
		//经验=（创建角色等级×2+70）
		exp = ActorLevel * 2 + expParam;
	}
	else if(ActorLevel < NpcLevel)
	{
		//经验=(创建角色等级×2+70)*[1+0.05*(怪物等级-创建角色等级)]
		exp = (ActorLevel * 2 + expParam)*(1+0.05*std::min(NpcLevel-ActorLevel,4)) ;
	}
	else if(ActorLevel-NpcLevel >= 10)  //高于10级不获得经验
	{
		exp =  0;
	}
	else
	{
		//经验=(创建角色等级×2+70)*[1-(创建角色等级-怪物等级)/10]
		exp = (ActorLevel * 2 + expParam)*(1-(ActorLevel-NpcLevel)/(float)10 ) ;
	}

	if ( m_pActor->GetThingClass() == enThing_Class_Actor){

		IActor * pActor = (IActor *)m_pActor;
		exp = exp + exp * pActor->GetVipValue(enVipType_AutoKillAddExp) / 100.0f;

		exp = exp*((IActor *)m_pActor)->GetMultipExpFactor(); //多倍经验
	}

	exp += 0.99999;

	return exp;	 
}

//设置组队阵形
UINT8 CombatPart::SetTeamLineup(CS_SetTeamLineup & Req, bool bSycMember)
{
	if ( m_pActor->GetThingClass() != enThing_Class_Actor || 0 != ((IActor *)m_pActor)->GetMaster() ){

		return enTeamRetCode_Max;
	}

	IActor * pActor = (IActor *)m_pActor;

	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){

		return enTeamRetCode_Max;
	}

	IActor * pTeamMember = pTeamPart->GetTeamMember();

	if ( 0 == pTeamMember ){

		return enTeamRetCode_NoTeam;
	}

	if ( bSycMember )
	{
		//自己主角是否在阵形中
		bool bHaveMaster = false;

		//队友主角是否在阵形中
		bool bHaveMemberMaster = false;

		for ( int i = 0; i < MAX_LINEUP_POS_NUM; ++i )
		{
			if ( !Req.m_uidLineup[i].IsValid()){

				continue;
			}

			if ( g_pGameServer->GetGameWorld()->FindActor(Req.m_uidLineup[i]) == 0 ){

				return enTeamRetCode_ErrActor;
			}

			if ( pActor->GetUID() == Req.m_uidLineup[i] ){

				bHaveMaster = true;
			}

			if ( pTeamMember->GetUID() == Req.m_uidLineup[i] ){

				bHaveMemberMaster = true;
			}
		}

		if ( !bHaveMemberMaster || !bHaveMaster ){

			return enTeamRetCode_NoHaveUuser;
		}	
	}

	memcpy(&m_TeamLineup, &Req.m_uidLineup, sizeof(m_TeamLineup));

	//同步
	this->SycTeamLineup();

	if ( bSycMember )
	{
		//队友也设置下
		ICombatPart * pMemberCombatPart = pTeamMember->GetCombatPart();

		if ( 0 != pMemberCombatPart )
		{
			pMemberCombatPart->SetTeamLineup(Req, false);
		}
	}

	return enTeamRetCode_Ok;
}


//获取参战人员
std::vector<SCreatureLineupInfo> CombatPart::GetJoinBattleActor(bool bTeam)
{
	std::vector<SCreatureLineupInfo> vect;


	if ( bTeam )
	{
		IActor * pActor = (IActor *)m_pActor;

		ITeamPart * pTeamPart = pActor->GetTeamPart();

		if ( 0 == pTeamPart )
			return vect;

		IActor * pLeader = pTeamPart->GetTeamLeader();

		if ( 0 == pLeader )
			return vect;

		if ( m_pActor != pLeader )
		{
			ICombatPart * pCombatPart = pLeader->GetCombatPart();

			if ( 0 == pCombatPart )
				return vect;

			return pCombatPart->GetJoinBattleActor(true);
		}

		std::vector<SCreatureLineupInfo> MyVect;
		std::vector<SCreatureLineupInfo> MemberVect;

		if ( m_pActor->GetThingClass() != enThing_Class_Actor )
			return vect;

		IActor * pMember= pTeamPart->GetTeamMember();

		if ( 0 == pMember )
			return vect;

		for ( int i = 0; i < ARRAY_SIZE(m_TeamLineup); ++i )
		{
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_TeamLineup[i]);

			if ( 0 == pActor )
			{
				continue;
			}

			SCreatureLineupInfo Info;
			Info.m_pCreature = pActor;
			Info.m_Pos = i;

			if ( pActor->GetMaster() == 0 && pActor->GetUID() == m_pActor->GetUID() )
			{
				vect.insert(vect.begin(),Info);
			} 
			else if ( pActor->GetMaster() == 0 && pActor->GetUID() == pMember->GetUID() )
			{
				MemberVect.insert(MemberVect.begin(), Info);
			}
			else if ( pActor->GetMaster()->GetUID() == m_pActor->GetUID() )
			{
				MyVect.push_back(Info);
			}
			else
			{
				MemberVect.push_back(Info);
			}
		}

		vect.insert(vect.end(), MyVect.begin(), MyVect.end());
		vect.insert(vect.end(), MemberVect.begin(), MemberVect.end());
	}
	else
	{
		for(int i=0; i<ARRAY_SIZE(m_DBCombatData.m_uidLineup);i++)
		{
			IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_DBCombatData.m_uidLineup[i]);
			if(pActor)
			{
				SCreatureLineupInfo Info;
				Info.m_pCreature = pActor;
				Info.m_Pos = i;

				if(pActor->GetMaster()==0)
				{
					//队长主角放在第一位了
					vect.insert(vect.begin(),Info);
					
				}
				else
				{			
					vect.push_back(Info);
				}
			}
		}
	}

	return vect;
}




//设置参战
bool CombatPart::SetJoinBattleActor(CS_EquipJoinBattle_Req & Req)
{
	if(m_pActor->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	SC_EquipJoinBattle_Rsp Rsp;
	Rsp.m_Result = enEquipRetCode_OK;

	IActor * pActor = (IActor*)m_pActor;

	//主角是否在内
	bool bHaveMaster = false; 

	//出战角色数
	UINT8 CombatActorNum = 0;

	for(int i = 0; i < ARRAY_SIZE(Req.m_uidLineup) && i < MAX_LINEUP_POS_NUM; i++)
	{
		if(!Req.m_uidLineup[i].IsValid())
		{
			continue;
		}

		if(pActor->GetUID()==Req.m_uidLineup[i])
		{
			bHaveMaster = true;
		}
		else if(pActor->GetEmployee(Req.m_uidLineup[i])==0)
		{
			Rsp.m_Result = enEquipRetCode_ErrActor;
			break;
		}

		++CombatActorNum;
	}

	if(bHaveMaster == false)
	{
		Rsp.m_Result = enEquipRetCode_ErrNotRemoveMaster;
	}

	if( CombatActorNum > MAX_COMBAT_ACTOR_NUM){
		Rsp.m_Result = enEquipRetCode_OverMaxCombatActorNum;
	}

	if(Rsp.m_Result == enEquipRetCode_OK)
	{
		memcpy(m_DBCombatData.m_uidLineup,Req.m_uidLineup,MEM_SIZE(m_DBCombatData.m_uidLineup));
		m_bNeedSave = true;
	}

	LineupSync();	

	OBuffer4k ob;

	ob << Equip_Header(enEquipCmd_JoinBattle,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

	if( Rsp.m_Result == enEquipRetCode_OK){
		return true;
	}

	return false;
}

//移除参战
bool  CombatPart::RemoveJoinBattleActor(UID uidActor)
{
	if(m_pActor->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}
	if(m_pActor->GetUID() == uidActor)
	{
		return false;
	}

	for(int i=0; i<ARRAY_SIZE(m_DBCombatData.m_uidLineup); i++)
	{
		if(m_DBCombatData.m_uidLineup[i]==uidActor)
		{
			m_DBCombatData.m_uidLineup[i] = UID();
			break;
		}
	}

	LineupSync();

	return true;
}


//尝试设置参战，如果参战位已满，则返回false失败
bool CombatPart::TrySetJoinBattle(UID uidActor)
{
	for(int i=0; i<ARRAY_SIZE(m_DBCombatData.m_uidLineup); i++)
	{
		if( !m_DBCombatData.m_uidLineup[i].IsValid())
		{
			m_DBCombatData.m_uidLineup[i] = uidActor;

			m_bNeedSave = true;

			this->LineupSync();
			return true;
		}
	}

	return false;
}



//同步组队阵形
void	CombatPart::SycTeamLineup()
{
	if ( m_pActor->GetThingClass() !=enThing_Class_Actor )
	{
		return;
	}

	if ( ((IActor*)m_pActor)->GetMaster() != 0 ){

		return;
	}

	SC_SynTeamLineup Rsp;

	memcpy(&Rsp.m_uidLineup, &m_TeamLineup, sizeof(m_TeamLineup));

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SC_SycTeamLineup, sizeof(Rsp)) << Rsp;
	((IActor*)m_pActor)->SendData(ob.TakeOsb());
}

//设置组队参战状态
UINT8	CombatPart::SetTeamCombatStatus(bool bJoinCombat, bool bSyc)
{
	if ( bJoinCombat == m_bTeamCombat ){
		return enTeamRetCode_Ok;
	}

	if ( m_pActor->GetThingClass() != enThing_Class_Actor ){
		return enTeamCmd_Max;
	}

	IActor * pActor = (IActor *)m_pActor;

	IActor * pMaster = pActor;

	if ( pActor->GetMaster() != 0 ){
		pMaster = pActor->GetMaster();
	}

	if ( pMaster == pActor && !bJoinCombat){
		return enTeamRetCode_ErrMasterCombat;
	}
	
	ITeamPart * pTeamPart = pMaster->GetTeamPart();

	if ( 0 == pTeamPart ){
		return enTeamCmd_Max;
	}

	if ( pTeamPart->IsHaveTeam() ){
		return enTeamRetCode_ErrHaveTeam;
	}

	//获取参战数量
	int TeamCombatNum = 1;

	for ( int i = 0; i < MAX_EMPLOY_NUM; ++i )
	{
		IActor * pEmployee = pMaster->GetEmployee(i);

		if ( 0 == pEmployee ){
			continue;
		}

		ICombatPart * pCombatPart = pEmployee->GetCombatPart();

		if ( 0 == pCombatPart ){
			continue;
		}

		if ( pCombatPart->GetIsJoinTeamCombat() ){
			++TeamCombatNum; 
		}
	}

	if ( bJoinCombat && MAX_TEAMCOMBAT_NUM <= TeamCombatNum ){
		return enTeamRetCode_ErrTeamCombatNum;
	}

	m_bTeamCombat = bJoinCombat;

	if ( bSyc ){
		//同步下
		ICombatPart * pCombatPart = pMaster->GetCombatPart();

		if ( 0 == pCombatPart )
			return enTeamCmd_Max;

		pCombatPart->SycTeamCombatState();	
	}

	return enTeamRetCode_Ok;
}

//得到是否参加组队战斗
bool	CombatPart::GetIsJoinTeamCombat()
{
	return m_bTeamCombat;
}

//同步组队参战状态
void	CombatPart::SycTeamCombatState()
{
	if ( m_pActor->GetThingClass() != enThing_Class_Actor ){
		return;
	}

	IActor * pActor = (IActor *)m_pActor;

	if ( pActor->GetMaster() != 0 ){
		return;
	}

	SC_SycTeamCombatState Rsp;

	Rsp.m_uidCombat[0] = pActor->GetUID();

	int pos = 1;

	for ( int i = 0; i < MAX_EMPLOY_NUM && pos < MAX_TEAMCOMBAT_NUM; ++i )
	{
		IActor * pEmployee = pActor->GetEmployee(i);

		if ( 0 == pEmployee ){
			continue;
		}

		ICombatPart * pCombatPart = pEmployee->GetCombatPart();

		if ( 0 == pCombatPart ){
			continue;
		}

		if ( pCombatPart->GetIsJoinTeamCombat() ){
			Rsp.m_uidCombat[pos] = pEmployee->GetUID();
			++pos;
		}
	}

	OBuffer1k ob;
	ob << TeamHeader(enTeamCmd_SC_SycTeamCombatState, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

