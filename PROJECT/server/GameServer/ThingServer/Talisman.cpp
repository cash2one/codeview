
#include "Talisman.h"
#include "XDateTime.h"
#include "IBasicService.h"
#include "IMagic.h"
#include "ICombatServer.h"
#include "IStatusPart.h"
#include "IEquipPart.h"
#include "ITalismanPart.h"

Talisman::Talisman()
{
	memset(m_TalismanProp,0,sizeof(m_TalismanProp));

	m_pMagic = 0;
}
Talisman::~Talisman()
{
	if(m_pMagic != 0)
	{
		m_pMagic->Release();
		m_pMagic = 0;
	}
}

//初始化，
bool Talisman::Create(void) 
{
	if(Super::Create()==false)
	{
		return false;
	}
   
	return true;
}

//开始孕育物品
bool Talisman::SpawnGoods(IActor * pActor)
{
	const SGoodsCnfg * pGoodsCnfg =  GetGoodsCnfg();

   if(pGoodsCnfg== 0 || __GetPropValue(enGoodsProp_BeginGestateTime) !=0)
   {
	   return false;
   }

   if(enTalismanType_In == pGoodsCnfg->m_SubClass)
   {
	   if(__GetPropValue(enGoodsProp_GestateGoodsID)==INVALID_GOODS_ID)
	   {
	         TGoodsID GoodsID = GestateGoodsID();
			 SetPropNum(pActor,enGoodsProp_GestateGoodsID,GoodsID);
			 SetPropNum(pActor,enGoodsProp_BeginGestateTime,CURRENT_TIME());
			 SetPropNum(pActor,enGoodsProp_QualityPoint,0);
			 SetPropNum(pActor,enGoodsProp_bUseSpawnProp,0);
	   }

	   return true;
   }

   return false;

}

	//孕育时间是否已到
bool Talisman::IsEndSpawnTime() 
{
	 //法宝孕育时间
	  INT32 SpawnTalismanTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SpawnTalismanTime;

	  //当前时间
	  time_t nCurTime = CURRENT_TIME();

	  		 //获得开始法宝孕育时间
	 INT32 BeginSpawnTime=0; 
	 GetPropNum(enGoodsProp_BeginGestateTime,BeginSpawnTime);

	  if(nCurTime-BeginSpawnTime < SpawnTalismanTime)
	  {
		  return false;
	  }

	  return true;
}

//提升品质
bool Talisman::UpgradeQuality(IActor * pActor,INT32 QualityPoint)
{
	//先记录旧值
	INT32 OldValue = 0;
	GetPropNum(enGoodsProp_QualityPoint,OldValue);

	//旧的品质等级
	UINT8 OldLevel = g_pGameServer->GetConfigServer()->GetTalismanQualityLevel(OldValue);

	int NewValue = 0;
	AddPropNum(pActor,enGoodsProp_QualityPoint,QualityPoint,&NewValue);

	//新等级
	UINT8 NewLevel = g_pGameServer->GetConfigServer()->GetTalismanQualityLevel(NewValue);

	//等级变了，需要取新的孕育物
	if(OldLevel != NewLevel)
	{
		 TGoodsID GoodsID = GestateGoodsID();
		 if(GoodsID != INVALID_GOODS_ID)
		 {
		    SetPropNum(pActor,enGoodsProp_GestateGoodsID,GoodsID);
		 }
		 else
		 {
			 //法宝级别
			 INT32 TalismanLevel = 0;
			 GetPropNum(enGoodsProp_MagicLevel,TalismanLevel);

			 TRACE("<error> %s : %d line 找不到孕育物品,QualityLevel = %d TalismanLevel = %d ", __FUNCTION__,__LINE__,NewLevel,TalismanLevel );
		 }
	}


	return true;
}

//得到是否需要更新到数据库
bool Talisman::GetNeedUpdate()
{
	return Super::GetNeedUpdate();
}

//设置是否需要更新到数据库
void Talisman::SetUpdate(bool bNeedUpdate)
{
	Super::SetUpdate(bNeedUpdate);
}


//是否在孕育物品
 bool Talisman::IsSpawnGoods()
 {
	 return __GetPropValue(enGoodsProp_BeginGestateTime) !=0;
 }

 	//该法宝是否是内法宝
bool Talisman::IsInTalisman()
{
	const SGoodsCnfg * pGoodsCnfg =  GetGoodsCnfg();

   if(pGoodsCnfg== 0 )
   {
	   return false;
   }

   if(enTalismanType_In != pGoodsCnfg->m_SubClass)
   {
	    return false;
   }

   return true;

}

TGoodsID Talisman::GestateGoodsID()
{
	TGoodsID GoodsID = INVALID_GOODS_ID;

	const SGoodsCnfg * pGoodsCnfg =  GetGoodsCnfg();

   if(pGoodsCnfg== 0)
   {
	   return GoodsID;
   }

   if(enTalismanType_In == pGoodsCnfg->m_SubClass)
   {
	  //品质等级
	   INT32 QualityLevel = GetQualityLevel();

	   //获得法宝等级
	   INT32 TalismanLevel = 0;
	   GetPropNum(enGoodsProp_MagicLevel,TalismanLevel);

	   //法宝世界配置
	   const STalismanWorldCnfg * pWorldCnfg = g_pGameServer->GetConfigServer()->GetTalismanWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);

	   if(pWorldCnfg != 0)
	   {

		  GoodsID = g_pGameServer->GetConfigServer()->GetGestateGoodsID(QualityLevel,TalismanLevel,pWorldCnfg->m_WorldType);
	   }
		
   }

   return GoodsID;
}

//获得品质等级
UINT8 Talisman::GetQualityLevel()
{
	 //品质点
		INT32 QualityPoint = __GetPropValue(enGoodsProp_QualityPoint);

		return g_pGameServer->GetConfigServer()->GetTalismanQualityLevel(QualityPoint);
}

	//获得法术
IMagic * Talisman::GetMagic()
{
	const SGoodsCnfg * pGoodsCnfg =  GetGoodsCnfg();
	if(pGoodsCnfg == 0)
	{
		return 0;
	}

	//法术等级
	INT32 MagicLevel = 0;
	this->GetPropNum(enGoodsProp_MagicLevel,MagicLevel);

	if(m_pMagic != 0)
	{
		if(m_pMagic->GetLevel() == MagicLevel)
		{
			return m_pMagic;
		}

		m_pMagic->Release();
	}
	
	SCreateMagicCnt  CreateMagicCnt;

	CreateMagicCnt.m_MagicID = pGoodsCnfg->m_SpiritOrMagic;
	CreateMagicCnt.m_Level = MagicLevel;

	m_pMagic = g_pGameServer->GetCombatServer()->CreateMagic(CreateMagicCnt);
	return m_pMagic;
}


void Talisman::Release()
{
	delete this;
}


//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的公开现场
// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
//////////////////////////////////////////////////////////////////////////
bool Talisman::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SGoodsPublicData))
	{
		return false;
	}

	if(Super::OnGetPublicContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPublicData * pPublicData = (SGoodsPublicData *)buf;

	pPublicData->m_GoodsClass = enGoodsCategory_Talisman;

	pPublicData->m_TalismanProp.m_BeginGestateTime = __GetPropValue(enGoodsProp_BeginGestateTime);

	pPublicData->m_TalismanProp.m_EnterNum = __GetPropValue(enGoodsProp_EnterNum);

	pPublicData->m_TalismanProp.m_GestateGoodsID = __GetPropValue(enGoodsProp_GestateGoodsID);
	
	pPublicData->m_TalismanProp.m_MagicLevel = __GetPropValue(enGoodsProp_MagicLevel);

	pPublicData->m_TalismanProp.m_QualityPoint = __GetPropValue(enGoodsProp_QualityPoint);

	pPublicData->m_TalismanProp.m_GhostGoodsID = __GetPropValue(enGoodsProp_GhostGoodsID);

	pPublicData->m_TalismanProp.m_GhostLevel = __GetPropValue(enGoodsProp_GhostLevel);

	nLen = sizeof(SGoodsPublicData);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Talisman::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnGetPrivateContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData *)buf;

	pPrivateData->m_GoodsClass = enGoodsCategory_Talisman;

	pPrivateData->m_TalismanProp.m_BeginGestateTime = __GetPropValue(enGoodsProp_BeginGestateTime);

	pPrivateData->m_TalismanProp.m_EnterNum = __GetPropValue(enGoodsProp_EnterNum);

	pPrivateData->m_TalismanProp.m_GestateGoodsID = __GetPropValue(enGoodsProp_GestateGoodsID);

	//pPrivateData->m_TalismanProp.m_LastEnterTime = __GetPropValue(enGoodsProp_LastEnterTalismanTime);

	pPrivateData->m_TalismanProp.m_MagicLevel = __GetPropValue(enGoodsProp_MagicLevel);

	pPrivateData->m_TalismanProp.m_QualityPoint = __GetPropValue(enGoodsProp_QualityPoint);

	pPrivateData->m_TalismanProp.m_GhostGoodsID = __GetPropValue(enGoodsProp_GhostGoodsID);

	pPrivateData->m_TalismanProp.m_GhostLevel = __GetPropValue(enGoodsProp_GhostLevel);

	nLen = sizeof(SGoodsPrivateData);

	return true;
}


bool Talisman::OnSetPrivateContext(const void * buf, int nLen)
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnSetPrivateContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData *)buf;

	 __SetPropValue(enGoodsProp_BeginGestateTime,pPrivateData->m_TalismanProp.m_BeginGestateTime);

	 __SetPropValue(enGoodsProp_EnterNum,pPrivateData->m_TalismanProp.m_EnterNum );

	 __SetPropValue(enGoodsProp_GestateGoodsID,pPrivateData->m_TalismanProp.m_GestateGoodsID);

	// __SetPropValue(enGoodsProp_LastEnterTalismanTime,pPrivateData->m_TalismanProp.m_LastEnterTime);

	__SetPropValue(enGoodsProp_MagicLevel,pPrivateData->m_TalismanProp.m_MagicLevel);

	__SetPropValue(enGoodsProp_QualityPoint,pPrivateData->m_TalismanProp.m_QualityPoint);

	__SetPropValue(enGoodsProp_GhostGoodsID,pPrivateData->m_TalismanProp.m_GhostGoodsID);

	__SetPropValue(enGoodsProp_GhostLevel,pPrivateData->m_TalismanProp.m_GhostLevel);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool Talisman::OnSetDBContext(const void * buf, int nLen)
{
	if(buf==0 || nLen<sizeof(SDBGoodsData))
	{
		return false;
	}

	if(Super::OnSetDBContext(buf,nLen)==false)
	{
		return false;
	}

	SDBGoodsData * pPrivateData = (SDBGoodsData *)buf;

	 __SetPropValue(enGoodsProp_BeginGestateTime,pPrivateData->m_TalismanProp.m_BeginGestateTime);

	 __SetPropValue(enGoodsProp_EnterNum,pPrivateData->m_TalismanProp.m_EnterNum );

	 __SetPropValue(enGoodsProp_GestateGoodsID,pPrivateData->m_TalismanProp.m_GestateGoodsID);

	// __SetPropValue(enGoodsProp_LastEnterTalismanTime,pPrivateData->m_TalismanProp.m_LastEnterTime);

	 if(pPrivateData->m_TalismanProp.m_MagicLevel<1)
	 {
		 pPrivateData->m_TalismanProp.m_MagicLevel = 1;
	 }

	__SetPropValue(enGoodsProp_MagicLevel,pPrivateData->m_TalismanProp.m_MagicLevel);

	__SetPropValue(enGoodsProp_QualityPoint,pPrivateData->m_TalismanProp.m_QualityPoint);

	__SetPropValue(enGoodsProp_bUseSpawnProp,pPrivateData->m_TalismanProp.m_bUseSpawnProp);

	//__SetPropValue(enGoodsProp_GhostGoodsID,pPrivateData->m_TalismanProp.m_GhostGoodsID);

	__SetPropValue(enGoodsProp_GhostGoodsID,pPrivateData->m_TalismanProp.m_GhostGoodsID);

	__SetPropValue(enGoodsProp_GhostLevel,pPrivateData->m_TalismanProp.m_GhostLevel);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool Talisman::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SDB_Save_GoodsReq))
	{
		return false;
	}

	if(Super::OnGetDBContext(buf,nLen)==false)
	{
		return false;
	}

	SDB_Save_GoodsReq * pDBGoods = (SDB_Save_GoodsReq *)buf;

	SDBTalismanProp * pDBTalisman = (SDBTalismanProp *)&pDBGoods->GoodsData;

	pDBTalisman->m_BeginGestateTime = __GetPropValue(enGoodsProp_BeginGestateTime);

	pDBTalisman->m_EnterNum = __GetPropValue(enGoodsProp_EnterNum);

	pDBTalisman->m_GestateGoodsID = __GetPropValue(enGoodsProp_GestateGoodsID);

	//pDBTalisman->m_LastEnterTime = __GetPropValue(enGoodsProp_LastEnterTalismanTime);

	pDBTalisman->m_MagicLevel = __GetPropValue(enGoodsProp_MagicLevel);

	pDBTalisman->m_QualityPoint = __GetPropValue(enGoodsProp_QualityPoint);

	pDBTalisman->m_bUseSpawnProp = __GetPropValue(enGoodsProp_bUseSpawnProp);

	//pDBTalisman->m_GhostGoodsID = __GetPropValue(enGoodsProp_GhostGoodsID);

	pDBTalisman->m_GhostGoodsID = __GetPropValue(enGoodsProp_GhostGoodsID);

	pDBTalisman->m_GhostLevel = __GetPropValue(enGoodsProp_GhostLevel);

	nLen = sizeof(SDB_Save_GoodsReq);

	return true;
}

//取得物品的类别（例如：装备、消耗品）
enGoodsCategory Talisman::GetGoodsClass(void)
{
	return enGoodsCategory_Talisman;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：改变本物品数字型属性，将原来的值增加nValue
// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
// 输  出：pNewValue返回改变后的新值

//////////////////////////////////////////////////////////////////////////
bool Talisman::AddPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue)
{
	INT32 nNewValue = 0;
	INT32 nOldValue = 0;

	GetPropNum(enPropID,nOldValue);

	if( SetPropNum(pActor,enPropID,nOldValue+nValue,&nNewValue)==false)
	{
		return false;
	}

	if(pNewValue)
	{
		*pNewValue = nNewValue;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：设置本物品数字型属性，将原来的值替换为nValue
// 输  入：数字型属性enPropID，新属性值nValue，
// 输  出：pNewValue返回改变后的新值
//////////////////////////////////////////////////////////////////////////
bool Talisman::SetPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue)
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::SetPropNum(pActor,enPropID,nValue,pNewValue);
	}
	else if(enPropID >enGoodsProp_Talisman && enPropID<enGoodsProp_TalismanEnd)
	{
		//如果改变的是法术级别，并且穿在身上的，得先删效果
		if( enPropID == enGoodsProp_MagicLevel && this->IsInEquipPanel(pActor)){
			this->UnEquip(pActor);
		}

       	__SetPropValue(enPropID,nValue);

		INT32 NewValue = __GetPropValue(enPropID);

		//如果改变的是法术级别，并且穿在身上的，得重新加加效果
		if( enPropID == enGoodsProp_MagicLevel && this->IsInEquipPanel(pActor)){
			this->OnEquip(pActor);
		}

		if(pNewValue)
		{
			*pNewValue = NewValue;
		}

		if(pActor)
		{
			INT32 ActorPropID = TalismanPropMapID[enPropID - enGoodsProp_Talisman - 1];
			if( -1 != ActorPropID){
				pActor->NoticClientUpdateThing(GetUID(),ActorPropID,NewValue);
			}
		}

		return true;
	}
	else
	{
		TRACE("%s : %d line 意外的属性ID enPropID=%d", __FUNCTION__,__LINE__,enPropID);
	}


	return false;
}

//取得本物品的数字型属性
bool Talisman::GetPropNum(enGoodsProp enPropID,int & nValue)
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::GetPropNum(enPropID,nValue);
	}
	else if(enPropID >enGoodsProp_Talisman && enPropID<enGoodsProp_TalismanEnd)
	{
       nValue =  m_TalismanProp[enPropID-enGoodsProp_Talisman];		
	
	   return true;
	}
	else
	{
		TRACE("%s : %d line 意外的属性ID enPropID=%d", __FUNCTION__,__LINE__,enPropID);
	}

	return false;
}

//获得物品数量
int Talisman::GetNumber()
{
	return 1;
}


//检查生物pCreature是否能装备该项仙剑，返回TRUE表示可以装备
bool Talisman::CanEquip(ICreature *pCreature,UINT8 pos,std::string & strErr)
{
	const SGoodsCnfg * pGoodsCnfg = GetGoodsCnfg();

	//等级要求
	if(pCreature->GetCrtProp(enCrtProp_Level)<pGoodsCnfg->m_UsedLevel)
	{
		strErr = "不能装备，你的等级没达到要求!";
		return false;
	}
	
	return true;
}

//给生物pCreature装备该法宝，返回TRUE表示装备成功
bool Talisman::OnEquip(ICreature *pCreature)
{
	if(pCreature->GetThingClass() != enThing_Class_Actor)
	{
		TRACE("<error> %s : %d 不能给非玩家装备法宝",__FUNCTION__,__LINE__);
		return false;
	}

	//把法宝上灵件的属性加给玩家
	enGhostProp  PropID = enGhostProp_Max;
	INT32  Value = 0;
    TGoodsID GhostGoodsID = __GetPropValue((enGoodsProp)enGoodsProp_GhostGoodsID);
	int Level = __GetPropValue((enGoodsProp)enGoodsProp_GhostLevel);

	if(GhostGoodsID != INVALID_GOODS_ID)
	{
		const SUpGhostCnfg * pGhostCnfg = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(GhostGoodsID,Level);
		if(pGhostCnfg==0)
		{
			TRACE("<error> %s : %d line 找不到灵件配置信息 GhostGoodsID=%d",__FUNCTION__,__LINE__,GhostGoodsID);
			return false;
		}

		PropID = (enGhostProp)pGhostCnfg->m_Type;
		Value = pGhostCnfg->m_Addprop;

		if(PropID != enGhostProp_Max)
		{
			 pCreature->AddCrtPropNum(s_GhostPropMapActorProp[PropID],Value);
		}
	}

	IMagic * pMagic = GetMagic();
	if(pMagic)
	{
		pMagic->OnEquip((IActor*)pCreature);
	}

	return true;
}

//给生物pCreature卸载该仙剑，返回TRUE表示卸载成功
bool Talisman::UnEquip(ICreature *pCreature)
{
	if(pCreature->GetThingClass() != enThing_Class_Actor)
	{
		TRACE("<error> %s : %d 不能给非玩家取下法宝",__FUNCTION__,__LINE__);
		return false;
	}

	//把法宝上灵件的属性从玩家身上扣除
	enGhostProp  PropID = enGhostProp_Max;
	INT32  Value = 0;
    TGoodsID GhostGoodsID = __GetPropValue((enGoodsProp)enGoodsProp_GhostGoodsID);
	int Level = __GetPropValue((enGoodsProp)enGoodsProp_GhostLevel);

	if(GhostGoodsID != INVALID_GOODS_ID)
	{
		const SUpGhostCnfg * pGhostCnfg = g_pGameServer->GetConfigServer()->GetUpGhostCnfg(GhostGoodsID,Level);
		if(pGhostCnfg==0)
		{
			TRACE("<error> %s : %d line 找不到灵件配置信息 GhostGoodsID=%d",__FUNCTION__,__LINE__,GhostGoodsID);
			return false;
		}

		PropID = (enGhostProp)pGhostCnfg->m_Type;
		Value = pGhostCnfg->m_Addprop;

		if(PropID != enGhostProp_Max && PropID < ARRAY_SIZE(s_GhostPropMapActorProp))
		{
			 pCreature->AddCrtPropNum(s_GhostPropMapActorProp[PropID],- Value);
		}
	}

	IMagic * pMagic = GetMagic();
	if(pMagic)
	{
		pMagic->OnUnEquip((IActor*)pCreature);
	}

	return true;
}


INT32 Talisman::__GetPropValue(enGoodsProp enPropID)
{
	if(enPropID>=enGoodsProp_Talisman && enPropID<enGoodsProp_TalismanEnd)
	{
		return m_TalismanProp[enPropID-enGoodsProp_Talisman];
	}

	return Super::__GetPropValue(enPropID);
}

void Talisman::__SetPropValue(enGoodsProp enPropID,INT32 nValue)
{
	if(enPropID>=enGoodsProp_Talisman && enPropID<enGoodsProp_TalismanEnd)
	{
		m_TalismanProp[enPropID-enGoodsProp_Talisman] = nValue;
	}
	else
	{
		 Super::__SetPropValue(enPropID,nValue);
	}

	//属性有变动,需要保存
	this->SetUpdate(true);
}

//法宝的法术级别改变
bool Talisman::IsInEquipPanel(IActor * pActor)
{
	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return false;
	}

	return (pEquipPart->GetEquipment(this->GetUID()) != 0);
}
