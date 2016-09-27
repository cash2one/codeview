

#ifndef __THINGSERVER_MONSTER_BUILDER_H__
#define __THINGSERVER_MONSTER_BUILDER_H__

#include "DSystem.h"
#include "BclHeader.h"
#include "IMonster.h"

class MonsterBuilder
{
public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建一个怪物
	// 输  入：保存数据的缓冲区buf，len为buf的大小，怪物所在的地图ID
	// 返回值：成功返回怪物的指针，
	//         失败返回NULL
	// 备  注：销毁怪物时，调用 IMonster::Release()
	//////////////////////////////////////////////////////////////////////////
	IMonster * BuildMonster(TSceneID sceneID, const UID& uid, const char *buf, int len,UINT32 flag);

	MonsterBuilder();

	~MonsterBuilder();

};



#endif
