#include "Portal.h"

#include "GameSrvProtocol.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IActor.h"
#include "IGameScene.h"
#include "DMsgSubAction.h"
#include "IResOutputPart.h"

Portal::Portal() 
{
	m_ptLoc.x = m_ptLoc.y = 0;
	m_PortalID = 0;
	m_SceneID = INVALID_SCENE_ID;
}

Portal::~Portal() 
{
}



//初始化，
bool Portal::Create(void)
{
	const SPortalCnfg * pPortalCnfg = GetPortalCnfg();
	if(pPortalCnfg == 0)
	{
		return false;
	}

	SetName(pPortalCnfg->m_szName);

	return true;
}

const SPortalCnfg * Portal::GetPortalCnfg()
{
	const SPortalCnfg * pPortalCnfg = g_pGameServer->GetConfigServer()->GetPortalCnfg(m_PortalID);

	if(pPortalCnfg == 0)
	{
		TRACE("<error> %s %d line 找不到传送门配置信息 PortalID = %d",__FUNCTION__,__LINE__,m_PortalID);
	}

	return pPortalCnfg;
}

void Portal::Release() 
{
	delete this;
}

//取得本实体对象的类型（大类，例如：玩家角色、NPC、物品等）
enThing_Class Portal::GetThingClass(void)
{
	return enThing_Class_Portal;
}


//获得位置
XPoint Portal::GetLoc() 
{
	return m_ptLoc;
}


//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的公开现场
// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
//////////////////////////////////////////////////////////////////////////
bool Portal::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen < sizeof(SPortalPublicData))
	{
		return false;
	}

	SPortalPublicData * pPrivateData = (SPortalPublicData *)buf;

	pPrivateData->m_uidPortal = GetUID();
	pPrivateData->m_PortalID  = m_PortalID;
	pPrivateData->m_SceneID   = m_SceneID;
	pPrivateData->m_ptX       = m_ptLoc.x;
	pPrivateData->m_ptY       = m_ptLoc.y;

	nLen = sizeof(SPortalPublicData);


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Portal::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen < sizeof(SPortalPrivateData))
	{
		return false;
	}

	SPortalPrivateData * pPrivateData = (SPortalPrivateData *)buf;

	pPrivateData->m_uidPortal = GetUID();
	pPrivateData->m_PortalID  = m_PortalID;
	pPrivateData->m_SceneID   = m_SceneID;
	pPrivateData->m_ptX       = m_ptLoc.x;
	pPrivateData->m_ptY       = m_ptLoc.y;

	nLen = sizeof(SPortalPrivateData);

	return true;
}

bool Portal::OnSetPrivateContext(const void * buf, int nLen) 
{
	if(buf==0 || nLen < sizeof(SPortalPrivateData))
	{
		return false;
	}

	SPortalPrivateData * pPrivateData = (SPortalPrivateData *)buf;

	SetUID(pPrivateData->m_uidPortal);
	m_PortalID = pPrivateData->m_PortalID;
	m_SceneID = pPrivateData->m_SceneID;
	m_ptLoc.x = pPrivateData->m_ptX    ;
	m_ptLoc.y = pPrivateData->m_ptY    ;


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool Portal::OnSetDBContext(const void * buf, int nLen)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool Portal::OnGetDBContext(void * buf, int &nLen) 
{
	return false;
}



//被点击
void Portal::OnClicked(IActor * pActor,TSceneID SceneID) 
{
	const SPortalCnfg * pPortalCnfg = GetPortalCnfg();
	if(pPortalCnfg == 0)
	{
		return ;
	}

	TSceneID TargetSceneID = INVALID_SCENE_ID;

	switch(pPortalCnfg->m_Target)
	{
	case enPortalTarget_MainUI:
		TargetSceneID.From(pActor->GetCrtProp(enCrtProp_ActorMainSceneID));
		break;
	case enPortalTarget_FuMeDong :
		TargetSceneID.From(pActor->GetCrtProp(enCrtProp_ActorFuMoDongSceneID));
		break;
	case enPortalTarget_HouShan:
		TargetSceneID.From(pActor->GetCrtProp(enCrtProp_ActorHouShanSceneID));
		break;
	case enPortalTarget_LastScene:
		TargetSceneID.From(pActor->GetCrtProp(enCrtProp_ActorLastSceneID));
		break;
	case enPortalTarget_Assign:
		TargetSceneID = SceneID;
		break;
	}

	SC_EnterPortal_Rsp Rsp;

	Rsp.m_Result = enGameWorldRetCode_Ok;

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(TargetSceneID);

	if(TargetSceneID == INVALID_SCENE_ID)
	{
		TRACE("<error> %s : %d line 错误的目标场景ID!",__FUNCTION__,__LINE__);
	}
	else if(pGameScene == 0)
	{
		TargetSceneID.From(pActor->GetCrtProp(enCrtProp_ActorMainSceneID));
		pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(TargetSceneID);
		if(pGameScene==0)
		{
            TRACE("<error> %s : %d line 找不到玩家的主场景!",__FUNCTION__,__LINE__);
			return;
		}
	}	
	
	if(pGameScene->EnterScene(pActor)==false)
	{
		Rsp.m_Result = 	enGameWorldRetCode_Teleport;		
	}


	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_EnterPortal,sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

}
