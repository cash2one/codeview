

#include "Monster.h"
#include "GameSrvProtocol.h"
#include "IActor.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IMagic.h"
#include "ICombatServer.h"
#include "IGameWorld.h"
#include "IGameScene.h"
#include "RandomService.h"



Monster::Monster()
{
	memset(m_MonsterProp,0,sizeof(m_MonsterProp));
}

Monster::~Monster()
{
}


//初始化，
bool Monster::Create(void)
{	
		//创建法术
	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(__GetPropValue(enCrtProp_MonsterID));

	if(pMonsterCnfg==0)
	{
		return false;
	}

	__SetPropValue(enCrtProp_ActorCrit,pMonsterCnfg->Crit);

	__SetPropValue(enCrtProp_ActorTenacity,pMonsterCnfg->Tenacity);

	__SetPropValue(enCrtProp_ActorHit,pMonsterCnfg->Hit);

	__SetPropValue(enCrtProp_ActorDodge,pMonsterCnfg->Dodge);
	
	__SetPropValue(enCrtProp_MagicCD,pMonsterCnfg->MagicCD);

	std::vector<TMagicID> vectMagic = g_pGameServer->GetConfigServer()->GetMonsterMagicCnfg(pMonsterCnfg->m_Level,pMonsterCnfg->m_MagicNum);

	if(vectMagic.empty())
	{
		return true;
	}
	
	SCreateMagicCnt CreateMagicCnt;
	
	for(int i=0; i<vectMagic.size() && i<MAX_MONSTER_MAGIC_NUM;i++)
	{
		IMagic * pMagic = 0;
		if(vectMagic[i] != INVALID_MAGIC_ID)
		{
			CreateMagicCnt.m_Level = 1;

			CreateMagicCnt.m_MagicID = vectMagic[i];

           pMagic = g_pGameServer->GetCombatServer()->CreateMagic(CreateMagicCnt);
		}
		__SetPropValue((enCrtProp)(i+enCrtProp_MonsterMagic1),(int)(void*)pMagic);
	}



	return true;
}

void Monster::Release()
{

	//场景
	TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if(pGameScene != 0)
	{
		pGameScene->LeaveScene(this);
	}

	for(int i=0; i<MAX_MONSTER_MAGIC_NUM;i++)
	{
		IMagic * pMagic = (IMagic *)(void*)__GetPropValue((enCrtProp)(i+enCrtProp_MonsterMagic1));	
		if(pMagic !=0)
		{
			pMagic->Release();
			__SetPropValue((enCrtProp)(i+enCrtProp_MonsterMagic1),0);
		}
	}

	Super::Release();
}



//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的公开现场
// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
//////////////////////////////////////////////////////////////////////////
bool Monster::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SMonsterPublicData))
	{
		return false;
	}

	SMonsterPublicData *pPublicData = (SMonsterPublicData *)buf;

	//等级
	pPublicData->m_Level = __GetPropValue(enCrtProp_Level);

	//怪物ID
	pPublicData->m_MonsterID = __GetPropValue(enCrtProp_MonsterID);

	//方向
	pPublicData->m_nDir = __GetPropValue(enCrtProp_Dir);

	//坐标
	XPoint ptLoc = GetLoc();
	pPublicData->m_ptX = ptLoc.x;
	pPublicData->m_ptY = ptLoc.y;

	//场景
	TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
	pPublicData->m_SceneID = SceneID;

	//UID
	pPublicData->m_uid = GetUID();

	//名称

	strncpy(pPublicData->m_szName,m_szName,sizeof(pPublicData->m_szName));


	nLen = sizeof(SMonsterPublicData);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Monster::OnGetPrivateContext(void * buf, int &nLen)
{
	if(0==buf || nLen < sizeof(SMonsterPrivateData))
	{
		return false;
	}

	SMonsterPrivateData * pPrivateData = ( SMonsterPrivateData * )buf;

	//境界
	pPrivateData->m_MonsterLayer     =  __GetPropValue(enCrtProp_MonsterLayer);

	//法术
	pPrivateData->m_MonsterMagic     =  __GetPropValue(enCrtProp_MonsterMagic);

	//剑气
	pPrivateData->m_MonsterSwordkee =  __GetPropValue(enCrtProp_MonsterSwordkee);

	pPrivateData->m_MonsterID        = __GetPropValue(enCrtProp_MonsterID);


	//生物共有属性

	//等级
	pPrivateData->m_Level = __GetPropValue(enCrtProp_Level) ;

	//灵力
	pPrivateData->m_Spirit = __GetPropValue(enCrtProp_Spirit);

	//护盾
	pPrivateData->m_Shield = __GetPropValue(enCrtProp_Shield);

	//气血
	pPrivateData->m_Blood = __GetPropValue(enCrtProp_Blood);

	//身法
	pPrivateData->m_Avoid = __GetPropValue(enCrtProp_Avoid);

	//方向
	pPrivateData->m_nDir = __GetPropValue(enCrtProp_Dir);

	pPrivateData->m_Lineup = __GetPropValue(enCrtProp_MonsterLineup);

	//外观
	pPrivateData->m_Facade = 0;

	//是否可被占领
	pPrivateData->m_bCanReplace = __GetPropValue(enCrtProp_MonsterCanReplace);

	
	pPrivateData->m_Crit = __GetPropValue(enCrtProp_ActorCrit);

	pPrivateData->m_Tenacity = __GetPropValue(enCrtProp_ActorTenacity);

	pPrivateData->m_Hit = __GetPropValue(enCrtProp_ActorHit);

	pPrivateData->m_Dodge = __GetPropValue(enCrtProp_ActorDodge);

	pPrivateData->m_MagicCD = __GetPropValue(enCrtProp_MagicCD);

	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(pPrivateData->m_MonsterID );

	if(pMonsterCnfg)
	{
		pPrivateData->m_Facade = pMonsterCnfg->m_Facade;
	}

	//法术
	for(int i=0; i<MAX_MONSTER_MAGIC_NUM;i++)
	{
		IMagic * pMagic = (IMagic*)this->GetCrtProp((enCrtProp)(enCrtProp_MonsterMagic1+i));
		if(pMagic==0)
		{
			break;
		}
		pPrivateData->m_Magics[i] = pMagic->GetMagicID();
	}
	

	//坐标
	XPoint ptLoc = GetLoc();
	pPrivateData->m_ptX = ptLoc.x;
	pPrivateData->m_ptY = ptLoc.y;

	//场景
	TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
	pPrivateData->m_SceneID = SceneID;

	//thing共有属性
	strncpy(pPrivateData->m_szName,m_szName,sizeof(pPrivateData->m_szName));

	pPrivateData->m_uid = GetUID();

	nLen = sizeof(SMonsterPrivateData);

	return true;
}

bool Monster::OnSetPrivateContext(const void * buf, int nLen)
{		
	if(0==buf || nLen < sizeof(SMonsterPrivateData))
	{
		return false;
	}

	const SMonsterPrivateData * pPrivateData = (const SMonsterPrivateData * )buf;


	//境界
	__SetPropValue(enCrtProp_MonsterLayer,  pPrivateData->m_MonsterLayer) ;

	//法术
	__SetPropValue(enCrtProp_MonsterMagic,  pPrivateData->m_MonsterMagic) ;

	//剑气
	__SetPropValue(enCrtProp_MonsterSwordkee,  pPrivateData->m_MonsterSwordkee);

	__SetPropValue(enCrtProp_MonsterID,  pPrivateData->m_MonsterID ) ;


	//生物共有属性

	//等级
	__SetPropValue(enCrtProp_Level, pPrivateData->m_Level);

	//灵力
	__SetPropValue(enCrtProp_Spirit, pPrivateData->m_Spirit);

	//护盾
	__SetPropValue(enCrtProp_Shield, pPrivateData->m_Shield);

	//气血
	__SetPropValue(enCrtProp_Blood, pPrivateData->m_Blood);

	//身法
	__SetPropValue(enCrtProp_Avoid,pPrivateData->m_Avoid);

	//方向	
	__SetPropValue(enCrtProp_Dir,pPrivateData->m_nDir);

	//坐标
	__SetPropValue(enCrtProp_PointX,pPrivateData->m_ptX);
	__SetPropValue(enCrtProp_PointY,pPrivateData->m_ptY);

	__SetPropValue(enCrtProp_SceneID,pPrivateData->m_ptY);

	__SetPropValue(enCrtProp_ActorCrit,pPrivateData->m_Crit);

	__SetPropValue(enCrtProp_ActorTenacity,pPrivateData->m_Tenacity);

	__SetPropValue(enCrtProp_ActorHit,pPrivateData->m_Hit);

	__SetPropValue(enCrtProp_ActorDodge,pPrivateData->m_Dodge);

	__SetPropValue(enCrtProp_MagicCD,pPrivateData->m_MagicCD);
	//thing共有属性
	strncpy(m_szName,pPrivateData->m_szName,sizeof(m_szName));

	SetUID(UID(pPrivateData->m_uid));

	return true;
}



//////////////////////////////////////////////////////////////////////////
// 描  述：改变本生物的数字型属性，将原来的值增加nValue
// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
//         属性同步标志enFlag
// 返回值：成功pNewValue返回改变后的新值，
//////////////////////////////////////////////////////////////////////////
bool Monster::AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue,IActor* pActor)
{
	if(enPropID > enCrtProp_Monster && enPropID< enCrtProp_Monster_End)
	{
		m_MonsterProp[enPropID-enCrtProp_Monster] += nValue;


		INT32 NewValue = m_MonsterProp[enPropID-enCrtProp_Monster] ;

		if(pNewValue)
		{
			*pNewValue = NewValue ;
		}

		//得到对应于私有属性的排序编号
		INT32 MonsterPropID = ActorPropMapID[enPropID - enCrtProp_Monster - (enCrtProp_MonsterSwordkee - enCrtProp_Monster)];
		if( -1 == MonsterPropID){
			return true;
		}

		if(pActor)
		{
			pActor->NoticClientUpdateThing(GetUID(),MonsterPropID,NewValue);
		}

		return true;
	}	

	return Super::AddCrtPropNum(enPropID,nValue,pNewValue,pActor);
}

//////////////////////////////////////////////////////////////////////////
// 描  述：设置本生物的属性，替换原来的值
// 输  入：数字型属性enPropID，属性值nValue
//         属性同步标志enFlag
// 返回值：成功pNewValue返回改变后的新值
//////////////////////////////////////////////////////////////////////////
bool Monster::SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue,IActor* pActor)
{
	if(enPropID>enCrtProp_Monster && enPropID< enCrtProp_Monster_End)
	{
		m_MonsterProp[enPropID-enCrtProp_Monster] = nValue;


		INT32 NewValue = m_MonsterProp[enPropID-enCrtProp_Monster] ;

		if(pNewValue)
		{
			*pNewValue = NewValue ;
		}

		//得到对应于私有属性的排序编号
		INT32 MonsterPropID = ActorPropMapID[enPropID - enCrtProp_Monster - (enCrtProp_MonsterSwordkee - enCrtProp_Monster)];
		if( -1 == MonsterPropID){
			return true;
		}

		if(pActor)
		{
			pActor->NoticClientUpdateThing(GetUID(),MonsterPropID,NewValue);
		}
		return true;
	}	


	if(enPropID == enCrtProp_Blood )
	{
		INT32 BloodUp = GetBloodUp();
		//判断血上限
		if(nValue>BloodUp)
		{
			nValue = BloodUp;
		}

	}

	return Super::SetCrtProp(enPropID,nValue,pNewValue,pActor);
}

//获得血上限
INT32 Monster::GetBloodUp()
{
	const SMonsterCnfg * pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(__GetPropValue(enCrtProp_MonsterID));
	if(pMonsterCnfg==0)
	{
		return 0;
	}

	return pMonsterCnfg->m_Blood;
}

//恢复气血
void Monster::RecoverBlood(IActor * pActor) 
{
	INT32 BloodUp = GetBloodUp();

	if(BloodUp != __GetPropValue(enCrtProp_Blood))
	{
		SetCrtProp(enCrtProp_Blood,BloodUp,0,pActor);
	}
}

//取得本生物的属性
INT32 Monster::GetCrtProp(enCrtProp enPropID)
{
	if(enPropID>enCrtProp_Monster && enPropID< enCrtProp_Monster_End)
	{
		return m_MonsterProp[enPropID-enCrtProp_Monster];	  
	}	

	return Super::GetCrtProp(enPropID);
}

//获得物理伤害
INT32   Monster::GetPhysicalDamage()
{
	
	//怪物物理伤害=（灵力+身法+剑气*300%）

	//灵力
	INT32 spirit = GetCrtProp(enCrtProp_Spirit);

	//身法
	INT32 Avoid = GetCrtProp(enCrtProp_Avoid);

	//剑气
	INT32 Swordkee = GetCrtProp(enCrtProp_MonsterSwordkee);

	return (spirit + Avoid + Swordkee*3)+0.9999;       

}

//获得法术伤害
INT32   Monster::GetMagicDamage()
{
	//怪物法术伤害=（灵力+法术*300%）

	//灵力
	INT32 spirit = GetCrtProp(enCrtProp_Spirit);

	//剑气
	INT32 Magic = GetCrtProp(enCrtProp_MonsterMagic);

	return (spirit+Magic*3.0f)+0.9999;       
}

//获得防御值
INT32   Monster::GetDefense()
{
	//怪物防御=(护盾*2+身法*2)

	//灵力
	INT32 Shield = GetCrtProp(enCrtProp_Shield);

	//身法
	INT32 Avoid = GetCrtProp(enCrtProp_Avoid);

	return (Shield*2.0f + Avoid*2.0f)+0.9999;
}

INT32 Monster::__GetPropValue(enCrtProp enPropID)
{
	if(enPropID>enCrtProp_Monster && enPropID< enCrtProp_Monster_End)
	{
		return m_MonsterProp[enPropID-enCrtProp_Monster];	  
	}	

	return Super::__GetPropValue(enPropID);
}

bool Monster::__SetPropValue(enCrtProp enPropID,int nValue)
{
	if(enPropID>enCrtProp_Monster && enPropID< enCrtProp_Monster_End)
	{
		m_MonsterProp[enPropID-enCrtProp_Monster] = nValue;
	}	

	return Super::__SetPropValue(enPropID,nValue);
}

