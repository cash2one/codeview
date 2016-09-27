

#ifndef __THINGSERVER_GAMESCENE_H__
#define __THINGSERVER_GAMESCENE_H__

#include "IGameScene.h"
#include "IThing.h"
#include "BclHeader.h"
#include "ThingContainer.h"
#include "IActor.h"
#include "GameSrvProtocol.h"
#include  "IMonster.h"
#include "IBuilding.h"
#include "IPortal.h"

#include "ITimeAxis.h"
#include "ThingServer.h"



template<>
struct	std::hash<TSceneID>
{
	UINT32	operator()(const TSceneID & sid)const
	{
		return sid.m_id;
	}
};

class GameWorld;

//把数据广播给所有玩家，但m_pActor指定的玩家除外
class SendActorData
{
public:
	SendActorData(const char * pData,size_t Len,IActor * pActor = 0)
	{
		m_pData = pData;
		m_Len = Len;
		m_pActor = pActor;

	};

	void operator() (IThing* pThing)
	{
		if(pThing==0 || pThing->GetThingClass()!=enThing_Class_Actor
			|| pThing == (IThing*)m_pActor || m_pData==0 || m_Len==0 )
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		OStreamBuffer osb;
		osb.Write(m_pData,m_Len);

		pActor->SendData(osb);
	}

private:
	const char * m_pData;
	size_t       m_Len;
	IActor     * m_pActor; //该玩家不接收数据
};

//历遍所有生物
class VisitorThing
{
public:
	VisitorThing(IActor * pActor, TSceneID SceneID)
	{
		m_pActor = pActor;
		m_SceneID = SceneID;
	}

	void operator() (IThing* pThing)
	{
		if(pThing==0 || pThing == (IThing*)m_pActor)
		{
			return;
		}


		switch(pThing->GetThingClass())
		{
		case enThing_Class_Actor:
			{
				//m_pActor->NoticClientCreatePublicThing(pThing->GetUID());	
				NULL;
			}
			break;
		case enThing_Class_Monster:
			{
				m_pActor->NoticClientCreatePrivateThing(pThing->GetUID());	
			}
			break;
		case enThing_Class_Building:
			{
				if( m_SceneID.ToID() != m_pActor->GetCrtProp(enCrtProp_ActorMainSceneID) && m_SceneID.GetMapID() == g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MainMapID)
				{
					//在好友的主场景,不创建后山和伏魔洞
					IBuilding * pBuilding = (IBuilding *)pThing;

					if( pBuilding->GetBuildingType() != enBuildingType_FuMoDong && pBuilding->GetBuildingType() != enBuildingType_HouShan){
						m_pActor->NoticClientCreatePrivateThing(pThing->GetUID());
					}
				}
				else
				{
					m_pActor->NoticClientCreatePrivateThing(pThing->GetUID());
				}
			}
			break;
		case enThing_Class_Portal:
			{
				//不是进自己的主场景才创建传送门
				if( m_SceneID.ToID() != m_pActor->GetCrtProp(enCrtProp_ActorMainSceneID))
				{
					m_pActor->NoticClientCreatePublicThing(pThing->GetUID());
				}
			}
			break;
		default:
			return;
			break;
		}
	}

protected:
	IActor * m_pActor;

	TSceneID m_SceneID;	//要遍历的场景ID
};

class GameScene : public IGameScene
	, public ITimerSink
{
public:
	GameScene();
	virtual ~GameScene();

	//创建出一个游戏场景
	bool Create(GameWorld* pGameWorld,TSceneID sceneID,INT32 CreateNpcIndex, bool bDelNoneUser = false);

public:
	virtual void OnTimer(UINT32 timerID);

public:

	//返回自己的ID
	virtual	TSceneID	GetSceneID();
	//查询地图ID
	virtual	TMapID		GetMapID();


	//根据UID取得一个逻辑对象
	virtual IThing* GetThing(UID uid);

	//往游戏场景中添加一个thing
	virtual bool EnterScene(IThing *pThing, bool bSync=false);

	//在游戏场景里注销一个thing	
	virtual bool LeaveScene(IThing *pThing);


	//判断是否世界地图
	virtual bool    IsWorldMap();

	//强制关闭状态，无论副本内是否有人，都要关闭副本，将内部玩家传送到副本进入点
	virtual             bool    ForceCloseDown(INT32 secs);

		//获得场景内所有怪物
	virtual std::vector<UID> GetAllMonster();

	//获得场景内所有指定一类的事物
	virtual std::vector<UID> GetAllClassThing(enThing_Class type);

	//销毁场景内指定类事物
	virtual void DestroyThingClass(enThing_Class type);

private:
	
private:
	TSceneID	         m_sceneID;		//场景ID

	UINT32 m_keepLiveTime;				//一个副本没有人时，最多存活多少时间，这个来自配置文件 单位秒

	ThingContainer    m_ThingContainer; //thing 

	bool				m_bDelNoneUser;	//没有玩家时，是否删除场景
};



#endif

