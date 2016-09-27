#ifndef __THINGSERVER_ACTOR_BUILDER_H__
#define __THINGSERVER_ACTOR_BUILDER_H__

#include "Actor.h"

class ActorBuilder
{
public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建一个角色
	// 输  入：保存数据的缓冲区buf，len为buf的大小，角色所在的地图ID
	// 返回值：成功返回角色的指针，
	//         失败返回NULL
	// 备  注：销毁角色时，调用 Actor::Release()
	//////////////////////////////////////////////////////////////////////////
	Actor* BuildActor(TSceneID sceneID, const UID& uid, const char *buf, int len,UINT32 flag);

	ActorBuilder();

	~ActorBuilder();

};



#endif
