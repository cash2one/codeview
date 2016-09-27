

#ifndef __THINGSERVER_IGAMESCENE_H__
#define __THINGSERVER_IGAMESCENE_H__

#include "DSystem.h"
#include "IEventServer.h"
#include <vector>

struct IThing;
struct UID;

struct IGameScene
{
	virtual ~IGameScene() {};

	//返回自己的ID
	virtual	TSceneID	GetSceneID()=0;
	//查询地图ID
	virtual	TMapID		GetMapID()=0;


		//根据UID取得一个逻辑对象
	virtual IThing* GetThing(UID uid) =	0;

		//往游戏场景中添加一个thing
	virtual bool EnterScene(IThing *pThing,  bool bSync=false) = 0;


		//在游戏场景里注销一个thing	
	virtual bool LeaveScene(IThing *pThing) = 0;


	   //判断是否世界地图
    virtual bool    IsWorldMap()=0;


	    //强制关闭状态，无论副本内是否有人，都要关闭副本，将内部玩家传送到副本进入点
    virtual      bool    ForceCloseDown(int secs)=0;

	
	//获得场景内所有怪物
	virtual std::vector<UID> GetAllMonster() = 0;

	//获得场景内所有指定一类的事物
	virtual std::vector<UID> GetAllClassThing(enThing_Class type) = 0;

	//销毁场景内指定类事物
	virtual void DestroyThingClass(enThing_Class type) = 0;
};



#endif

