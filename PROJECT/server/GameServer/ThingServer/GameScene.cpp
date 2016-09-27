
#include "IActor.h"
#include "GameScene.h"
#include "GameWorld.h"
#include "ThingServer.h"
#include "GameSrvProtocol.h"
#include "ISession.h"
#include "IConfigServer.h"
#include "DSystem.h"
#include "VisitScene.h"
#include "IResOutputPart.h"


GameScene::GameScene()
{
	m_sceneID = INVALID_SCENE_ID;
	m_keepLiveTime = UINT_MAX;
	m_bDelNoneUser = false;
}
GameScene::~GameScene()
{
	m_ThingContainer.Visit(VisitScene());
}

bool GameScene::Create(GameWorld* pGameWorld,TSceneID sceneID,INT32 CreateNpcIndex, bool bDelNoneUser)
{
	const SMapConfigInfo * pMapConfigInfo = g_pGameServer->GetConfigServer()->GetMapConfigInfo(sceneID.GetMapID());

	if(pMapConfigInfo==0)
	{
		TRACE("%s : %d Line 不能获得地图配置信息 mapid=%d",__FUNCTION__,__LINE__,sceneID.GetMapID());
		return false;
	}

	m_bDelNoneUser = bDelNoneUser;

	m_sceneID = sceneID;
	m_keepLiveTime = pMapConfigInfo->m_keepliveTime;

	//由GameScene自己调

	pGameWorld->AddGameScene(this);

	const std::vector<SMonsterOutput> * pVectMonster = g_pGameServer->GetConfigServer()->GetMapMonsterCnfg(sceneID.GetMapID());
	if(pVectMonster)
	{
		//创建怪物
		for(int i=CreateNpcIndex; i < pVectMonster->size(); ++i)
		{
			const SMonsterOutput & Output = (*pVectMonster)[i];

			if(Output.m_MonsterID == INVALID_MONSTER_ID)
			{
				continue;
			}
			
		
			SCreateMonsterContext  MonsterCnt;

			MonsterCnt.MonsterID = Output.m_MonsterID;
			MonsterCnt.nDir      = Output.m_nDir;
			MonsterCnt.SceneID   = sceneID;
			MonsterCnt.ptLoc.x   = Output.m_PointX;
			MonsterCnt.ptLoc.y   = Output.m_PointY;

			MonsterCnt.m_nLineup   = Output.m_nLineup;
			MonsterCnt.m_CombatIndex   = Output.m_CombatIndex;
			MonsterCnt.m_DropID    = Output.m_DropID;
			MonsterCnt.m_DropIDHard = Output.m_DropIDHard;
			MonsterCnt.m_DropIDTeam = Output.m_DropIDTeam;
			MonsterCnt.m_CombatIndexHard = Output.m_CombatIndexHard;
			MonsterCnt.m_CombatIndexTeam = Output.m_CombatIndexTeam;
			MonsterCnt.m_bCanReplace = Output.m_bCanReplace;

			IMonster * pMonster = pGameWorld->CreateMonster(MonsterCnt);

			if(pMonster)
			{
				if(EnterScene(pMonster)==false)
				{
					pGameWorld->DestroyThing(pMonster->GetUID());
				}
			}
		}

	}


	//创建建筑
	INT32 size = pMapConfigInfo->m_vectBuildingType.size();
	for(int i=0; i< size;i++)
	{
		SCreateBuildingContext BuildingCnt;
		BuildingCnt.m_BuildingType = (enBuildingType)pMapConfigInfo->m_vectBuildingType[i];
		BuildingCnt.SceneID = sceneID;
		BuildingCnt.m_uidOwner = UID();

		IBuilding * pBuilding = pGameWorld->CreateBuilding(BuildingCnt);
	}

	//创建传送门
	SCreatePortalContext PortalCnt;
	PortalCnt.m_SceneID = sceneID;

	for(int i=0; i< pMapConfigInfo->m_vectPortal.size()/3;i++)
	{
		PortalCnt.m_PortalID = pMapConfigInfo->m_vectPortal[i*3];
		PortalCnt.m_ptX = pMapConfigInfo->m_vectPortal[i*3+1];
		PortalCnt.m_ptY = pMapConfigInfo->m_vectPortal[i*3+2];

		IPortal * pPortal = pGameWorld->CreatePortal(PortalCnt);

		if(pPortal)
		{
			EnterScene(pPortal);
		}

	}

	return true;	
}

void GameScene::OnTimer(UINT32 timerID)
{
}


//返回自己的ID
TSceneID	GameScene::GetSceneID()
{
	return m_sceneID;
}

//查询地图ID
TMapID		GameScene::GetMapID()
{
	return m_sceneID.GetMapID();

}


//根据UID取得一个逻辑对象
IThing* GameScene::GetThing(UID uid)
{
	return m_ThingContainer.GetThing(uid);
}

//往游戏场景中添加一个thing
bool GameScene::EnterScene(IThing *pThing, bool bSync)
{	

	if(pThing->GetThingClass()==enThing_Class_Actor)
	{
		if(m_ThingContainer.GetThing(pThing->GetUID())==0)
		{
			if(m_ThingContainer.AddThing(pThing)==false)
		   {
			  return false;
		   }
		}
	

		IActor * pActor = (IActor*)pThing;

		TSceneID SceneID;
		SceneID.From(pActor->GetCrtProp(enCrtProp_SceneID));

		if(SceneID != m_sceneID)
		{
			//离开旧场景
			IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
			if(pGameScene)
			{
				pGameScene->LeaveScene(pThing);
				
				pActor->SetCrtProp(enCrtProp_ActorLastSceneID,SceneID);
			}

			pActor->SetCrtProp(enCrtProp_SceneID,m_sceneID);
		}


		//通知客户端装载地图
		OBuffer1k ob;

		SC_EnterScene EnterScene;
		EnterScene.m_MapID = m_sceneID.GetMapID();
		EnterScene.m_SceneID = m_sceneID;

		ob << GameWorldHeader(enGameWorldCmd_SC_Enter_Scene,sizeof(SC_EnterScene))<<EnterScene;

		pActor->SendData(ob.TakeOsb());

	
		VisitorThing Visitor(pActor, m_sceneID);

		m_ThingContainer.Visit(Visitor);


		OBuffer1k ob2;
		ob2 << GameWorldHeader(enGameWorldCmd_SC_Enter_SceneFinish,0);

		pActor->SendData(ob2.TakeOsb());

		if( m_sceneID.ToID() == pActor->GetCrtProp(enCrtProp_ActorMainSceneID)){
			//切换回主场景，要同步下建筑时间
			IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
			if( 0 != pResOutputPart){
				pResOutputPart->SynResBuildInfo();
			}
		}

		//进场景成功，发事件
		SS_EnterScene  Scene;
		Scene.m_uidUser		  = pActor->GetUID();
		Scene.m_NewSceneID	  = m_sceneID;
		Scene.m_OldSceneID	  = SceneID;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);
		pActor->OnEvent(msgID,&Scene,sizeof(Scene));

	}
	else
	{
		if(pThing->GetThingClass() == enThing_Class_Monster)
		{
			IMonster * pMonster = (IMonster *)pThing;
			pMonster->SetCrtProp(enCrtProp_SceneID,m_sceneID);
		}
		
		if(m_ThingContainer.AddThing(pThing)==false)
		{
			return false;
		}
	}

	return true;
}

//在游戏场景里注销一个thing	
bool GameScene::LeaveScene(IThing *pThing)
{
	if(pThing==0)
	{
		return false;
	}

	m_ThingContainer.RemoveThing(pThing->GetUID());

	if ( m_bDelNoneUser )
	{
		 if ( m_ThingContainer.GetThingCount(enThing_Class_Actor) <= 0 )
		 {
			//放入垃圾回收里，等待回收
			 g_pGameServer->GetGameWorld()->Push_InvalidScene(m_sceneID);
		 }
	}

	return true;
}


//判断是否世界地图
bool    GameScene::IsWorldMap()
{
	return m_sceneID.IsWorldMap();
}

//强制关闭状态，无论副本内是否有人，都要关闭副本，将内部玩家传送到副本进入点
bool    GameScene::ForceCloseDown(INT32 secs)
{
	return true;
}



		//获得场景内所有怪物
std::vector<UID> GameScene::GetAllMonster()
{
	struct 
	{
		std::vector<UID> m_vect;
		void operator() (IThing * pThing)
		{
			m_vect.push_back(pThing->GetUID());
		}
	} Visit;
	
	m_ThingContainer.Visit(enThing_Class_Monster,Visit);

	return Visit.m_vect;
}

//获得场景内所有指定一类的事物
std::vector<UID> GameScene::GetAllClassThing(enThing_Class type)
{
	struct
	{
		std::vector<UID> m_vect;
		void operator() (IThing * pThing)
		{
			m_vect.push_back(pThing->GetUID());
		}
	}Visit;

	m_ThingContainer.Visit(type, Visit);

	return Visit.m_vect;
}

//销毁场景内指定类事物
void GameScene::DestroyThingClass(enThing_Class type)
{
	std::vector<UID> vecUID = this->GetAllClassThing(type);

	//先从场景移除
	m_ThingContainer.RemoveThingClass(type);

	//从世界删除
	for ( int i = 0; i < vecUID.size(); ++i )
	{
		g_pGameServer->GetGameWorld()->DestroyThing(vecUID[i]);
	}
}



