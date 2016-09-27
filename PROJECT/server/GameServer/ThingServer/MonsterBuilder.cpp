



#include "Monster.h"

#include "MonsterBuilder.h"

#include "IGameWorld.h"

#include "CombatPart.h"

#include "StatusPart.h"

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建一个怪物
	// 输  入：保存数据的缓冲区buf，len为buf的大小，怪物所在的地图ID
	// 返回值：成功返回怪物的指针，
	//         失败返回NULL
	// 备  注：销毁怪物时，调用 IMonster::Release()
	//////////////////////////////////////////////////////////////////////////
IMonster * MonsterBuilder::BuildMonster(TSceneID sceneID, const UID& uid, const char *buf, int len,UINT32 flag)
{
	if (NULL == buf || len != sizeof(SBuild_Monster))
	{
		return NULL;
	}

	SBuild_Monster *pBuildMonster = (SBuild_Monster *) buf;

	IThingPart* CrtPartArray[enThingPart_Crt_Max];
	memset(CrtPartArray, 0, sizeof(CrtPartArray));

	//可以new一块大内存，以免每次创建人物时，都要new内存
	Monster *pMonster = new Monster;
	if (NULL == pMonster)
	{
		return 0;
	}


	if (flag & THING_CREATE_FLAG_DB_DATA)
	{
	   if ( !pMonster->OnSetDBContext(buf, len) )
	  {		
		  TRACE("<error> %s : %d line 设置怪物DB数据失败",__FUNCTION__,__LINE__);
		  delete pMonster;
		return 0;
	  }
	}
	else
	{
		if (!pMonster->OnSetPrivateContext(pBuildMonster->pPrivateData, pBuildMonster->nPrivateLen) )
		{		
			TRACE("<error> %s : %d line 设置怪物私有数据失败",__FUNCTION__,__LINE__);
		  delete pMonster;

			return NULL;
		}
	}

	if(!pMonster->GetUID().IsValid())
	{
	   pMonster->SetUID(uid);
	}

	if ( !pMonster->Create())
	{	
		TRACE("<error> %s : %d line 创建物怪失败",__FUNCTION__,__LINE__);

		delete pMonster;
		return NULL;
	}

	//创建部件
	IThingPart * pThingPart = new StatusPart();

	if(pThingPart->Create(pMonster,0,0)==false)
	{
		TRACE("<error> % : %d line 创建怪物状态Part失败!",__FUNCTION__,__LINE__);
		delete pMonster;
		return 0;
	}

	pMonster->AddPart(pThingPart);


	pThingPart = new CombatPart();

	if(pThingPart->Create(pMonster,0,0)==false)
	{
		TRACE("<error> % : %d line 创建怪物战斗Part失败!",__FUNCTION__,__LINE__);

		delete pMonster;
		return 0;
	}

	pMonster->AddPart(pThingPart);


	return pMonster;
}

MonsterBuilder::MonsterBuilder()
{
}

MonsterBuilder::~MonsterBuilder()
{
}
