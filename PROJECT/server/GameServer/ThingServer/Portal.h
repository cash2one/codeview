
#ifndef __THINGSERVER_PORTAL_H__
#define __THINGSERVER_PORTAL_H__

#include "IPortal.h"
#include "ThingBase.h"
struct SPortalCnfg;

class Portal :  public TThingBase<IPortal>
{
public:
	Portal();
	virtual ~Portal();

public:
	//被点击
	virtual void OnClicked(IActor * pActor,TSceneID SceneID=INVALID_SCENE_ID);

		//初始化，
	virtual bool Create(void);

	virtual void Release();

	//取得本实体对象的类型（大类，例如：玩家角色、NPC、物品等）
	virtual enThing_Class GetThingClass(void);


	//获得位置
	virtual XPoint GetLoc();


	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen);
	virtual bool OnSetPrivateContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

private:
	const SPortalCnfg * Portal::GetPortalCnfg();

private:
	TSceneID     m_SceneID;
    XPoint       m_ptLoc;  //位置
	TPortalID    m_PortalID;


};







#endif
