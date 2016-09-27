
#ifndef __BCL_THING_SERVER_ITHING_H__
#define __BCL_THING_SERVER_ITHING_H__

#include "IEventServer.h"
#include "stdio.h"
#include <hash_map>
#include "UniqueIDGenerator.h"
#include "DSystem.h"

enum enThing_Class VC_PACKED_ONE
{
	enThing_Class_Actor = 0,	//0 = 玩家角色
	enThing_Class_Monster=1,		//1 = NPC
	enThing_Class_Goods=2,		//2 = 物品
	enThing_Class_Building = 3, //建筑
	enThing_Class_Portal   = 4, //传送门

	enThing_Class_Max			//最大值
} PACKED_ONE;


const static char * g_ThingClassName[enThing_Class_Max]=
{
    "玩家",
    "NPC",
    "物品",   
	"建筑",
	"传送门",
};




struct IThing : public IEventSource
{
		//初始化，
	virtual bool Create(void) = 0;

	virtual void Release() = 0;

	//设置本实体对象的UID
	virtual void SetUID(UID uid) = 0;

	//取得本实体对象的UID
	virtual UID GetUID(void) = 0;

	//取得本实体对象的名字
	virtual const char * GetName(void) = 0;

	//取得本实体对象的类型（大类，例如：玩家角色、NPC、物品等）
	virtual enThing_Class GetThingClass(void) = 0;


	//获得位置
	virtual XPoint GetLoc() = 0;


	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen) = 0;
	virtual bool OnSetPrivateContext(const void * buf, int nLen) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：发送消息给本实体
	// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
	// 返回值：
	// 备  注：在此函数内，可能会发FireVote和FireAction
	//////////////////////////////////////////////////////////////////////////
	virtual void OnMessage( UINT32 dwMsg, const void * buf, int nLen) = 0;

	//设置是否需要存盘
	virtual void SetSaveFlag(bool bSave) = 0;

	virtual bool GetSaveFlag() = 0;


protected:
	virtual ~IThing(){}


};


#endif
