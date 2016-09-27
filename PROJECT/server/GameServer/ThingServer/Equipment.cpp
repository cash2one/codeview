
#include "Equipment.h"
#include "IBasicService.h"
#include "IConfigServer.h"

Equipment::Equipment() 
{
	memset(m_EquipProp,0,sizeof(m_EquipProp));
}
Equipment::~Equipment() 
{
}



//初始化，
bool Equipment::Create(void) 
{
	if(Super::Create()==false)
	{
		return false;
	}

	return true;
}

void Equipment::Release() 
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
bool Equipment::OnGetPublicContext(void * buf, int &nLen) 
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

	pPublicData->m_GoodsClass = enGoodsCategory_Equip;

	pPublicData->m_EquipProp.m_Star = __GetPropValue(enGoodsProp_StarLevel);

	pPublicData->m_EquipProp.m_GemGoodsID[0] = __GetPropValue(enGoodsProp_Gem_ID1);

	pPublicData->m_EquipProp.m_GemGoodsID[1] = __GetPropValue(enGoodsProp_Gem_ID2);

	pPublicData->m_EquipProp.m_GemGoodsID[2] = __GetPropValue(enGoodsProp_Gem_ID3);

	nLen = sizeof(SGoodsPublicData);

	return true;
}

INT32 Equipment::__GetPropValue(enGoodsProp enPropID)
{
	if(enPropID>=enGoodsProp_Equip && enPropID<enGoodsProp_EquipEnd)
	{
		return m_EquipProp[enPropID-enGoodsProp_Equip];
	}

	return Super::__GetPropValue(enPropID);
}

void Equipment::__SetPropValue(enGoodsProp enPropID,INT32 nValue)
{
	if(enPropID>=enGoodsProp_Equip && enPropID<enGoodsProp_EquipEnd)
	{
		m_EquipProp[enPropID-enGoodsProp_Equip] = nValue;
	}
	else
	{
		 Super::__SetPropValue(enPropID,nValue);
	}

	//需要保存到数据库
	this->SetUpdate(true);
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Equipment::OnGetPrivateContext(void * buf, int &nLen) 
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnGetPrivateContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;

	pPrivateData->m_GoodsClass = enGoodsCategory_Equip;

	pPrivateData->m_EquipProp.m_Star = __GetPropValue(enGoodsProp_StarLevel);

	pPrivateData->m_EquipProp.m_GemGoodsID[0] = __GetPropValue(enGoodsProp_Gem_ID1);

	pPrivateData->m_EquipProp.m_GemGoodsID[1] = __GetPropValue(enGoodsProp_Gem_ID2);

	pPrivateData->m_EquipProp.m_GemGoodsID[2] = __GetPropValue(enGoodsProp_Gem_ID3);

	pPrivateData->m_EquipProp.m_bActiveSuit1  = __GetPropValue(enGoodsProp_ActiveSuit1);

	pPrivateData->m_EquipProp.m_bActiveSuit2  = __GetPropValue(enGoodsProp_ActiveSuit2);

	nLen = sizeof(SGoodsPrivateData);

	return true;
}
bool Equipment::OnSetPrivateContext(const void * buf, int nLen) 
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnSetPrivateContext(buf,nLen)==false)
	{
		return false;
	}
		
	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;

	__SetPropValue(enGoodsProp_StarLevel,pPrivateData->m_EquipProp.m_Star);

	__SetPropValue(enGoodsProp_Gem_ID1,pPrivateData->m_EquipProp.m_GemGoodsID[0]);

	__SetPropValue(enGoodsProp_Gem_ID2,pPrivateData->m_EquipProp.m_GemGoodsID[1]);

	__SetPropValue(enGoodsProp_Gem_ID3,pPrivateData->m_EquipProp.m_GemGoodsID[2]);

	__SetPropValue(enGoodsProp_ActiveSuit1,pPrivateData->m_EquipProp.m_bActiveSuit1);

	__SetPropValue(enGoodsProp_ActiveSuit2,pPrivateData->m_EquipProp.m_bActiveSuit2);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool Equipment::OnSetDBContext(const void * buf, int nLen) 
{
	if(buf==0 || nLen<sizeof(SDBGoodsData))
	{
		return false;
	}

	if(Super::OnSetDBContext(buf,nLen)==false)
	{
		return false;
	}
			
	SDBGoodsData * pPrivateData = (SDBGoodsData*)buf;

	__SetPropValue(enGoodsProp_StarLevel,pPrivateData->m_EquipProp.m_Star);

	__SetPropValue(enGoodsProp_Gem_ID1,pPrivateData->m_EquipProp.m_GemGoodsID[0]);

	__SetPropValue(enGoodsProp_Gem_ID2,pPrivateData->m_EquipProp.m_GemGoodsID[1]);

	__SetPropValue(enGoodsProp_Gem_ID3,pPrivateData->m_EquipProp.m_GemGoodsID[2]);


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool Equipment::OnGetDBContext(void * buf, int &nLen) 
{
	if( 0 == buf || sizeof(SDB_Save_GoodsReq) < nLen)
	{
		return false;
	}

	if(Super::OnGetDBContext(buf,nLen)==false)
	{
		return false;
	}
			
	SDB_Save_GoodsReq * pDBGoods = (SDB_Save_GoodsReq *)buf;

	SDBEquipProp * pDBEquipProp = (SDBEquipProp *)&pDBGoods->GoodsData;

	pDBEquipProp->m_Star = __GetPropValue(enGoodsProp_StarLevel);

	pDBEquipProp->m_GemGoodsID[0] = __GetPropValue(enGoodsProp_Gem_ID1);

	pDBEquipProp->m_GemGoodsID[1] = __GetPropValue(enGoodsProp_Gem_ID2);

	pDBEquipProp->m_GemGoodsID[2] = __GetPropValue(enGoodsProp_Gem_ID3);

	nLen = sizeof(SDB_Save_GoodsReq);

	return true;
}


//取得物品的类别（例如：装备、消耗品）
enGoodsCategory Equipment::GetGoodsClass(void) 
{
	return enGoodsCategory_Equip;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：改变本物品数字型属性，将原来的值增加nValue
// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
// 输  出：pNewValue返回改变后的新值

//////////////////////////////////////////////////////////////////////////
bool Equipment::AddPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue) 
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
bool Equipment::SetPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue) 
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::SetPropNum(pActor,enPropID,nValue,pNewValue);
	}
	else if(enPropID >enGoodsProp_Equip && enPropID<enGoodsProp_EquipEnd)
	{
       	__SetPropValue(enPropID,nValue);

		INT32 NewValue = __GetPropValue(enPropID);


		if(pNewValue)
		{
			*pNewValue = NewValue;
		}

		if(pActor)
		{
			INT32 ActorPropID = EquipmentPropMapID[enPropID - enGoodsProp_Equip - 1];
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
bool Equipment::GetPropNum(enGoodsProp enPropID,int & nValue)
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::GetPropNum(enPropID,nValue);
	}
	else if(enPropID >enGoodsProp_Equip && enPropID<enGoodsProp_EquipEnd)
	{
       nValue =  m_EquipProp[enPropID-enGoodsProp_Equip];		
	
	   return true;
	}
	else
	{
		TRACE("%s : %d line 意外的属性ID enPropID=%d", __FUNCTION__,__LINE__,enPropID);
	}

	return false;
}


//检查生物pCreature是否能装备该项装备，返回TRUE表示可以装备
bool Equipment::CanEquip(ICreature *pCreature, UINT8 pos,std::string & strErr) 
{
	const SGoodsCnfg * pGoodsCnfg = GetGoodsCnfg();

	if(pGoodsCnfg->m_SubClass != pos)
	{
		strErr = "不能装备，部位要求不符合!";

		return false;
	}

	//等级要求
	if(pCreature->GetCrtProp(enCrtProp_Level)<pGoodsCnfg->m_UsedLevel)
	{
		strErr = "不能装备，你的等级没达到要求!";
		return false;
	}
	
	return true;
}

//获得强化增加的基础属性百分比
INT32 Equipment::GetStrongAddRate()
{
	//强化增加的属性
	INT32 nStarLevel = 0;
	GetPropNum(enGoodsProp_StarLevel,nStarLevel);

	//强化增加的基础属性百分比
	INT32 Rate = 0;

	if(nStarLevel > 0)
	{
		const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
		if(nStarLevel>ConfigParam.m_vectEquipStrongPropRate.size())
		{
			TRACE("<error> %s : line 行找不到强化增加基础属性比例信息 nStarLevel = %d", __FUNCTION__,__LINE__,nStarLevel);
		}
		else
		{
			Rate = ConfigParam.m_vectEquipStrongPropRate[nStarLevel-1];
		}
	}

	return Rate;
}

//获得镶嵌宝石增加的属性ID,及属性值,参数nHole孔号，从零开始
bool Equipment::GetGemProp(INT32 nHole, enEquipProp & PropID ,INT32 & value)
{
	PropID = enEquipProp_Max;
	value = 0;

	if(nHole<0 || nHole>=MAX_INLAY_NUM)
	{
		TRACE("<error> %s : %d line 错误的宝石镶嵌孔号 nHole=%d",__FUNCTION__,__LINE__,nHole);
		return false;
	}

    TGoodsID GemGoodsID = __GetPropValue((enGoodsProp)(enGoodsProp_Gem_ID1+nHole));

	if(GemGoodsID != INVALID_GOODS_ID)
	{
		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GemGoodsID);
		if(pGoodsCnfg==0)
		{
			TRACE("<error> %s : %d line 找不到宝石配置信息 GemGoodsID=%d",__FUNCTION__,__LINE__,GemGoodsID);
			return false;
		}

		PropID = (enEquipProp)pGoodsCnfg->m_BloodOrSwordkee;
		value = pGoodsCnfg->m_AvoidOrSwordLvMax;
	}

	return true;
}

//给生物pCreature装备该装备，返回TRUE表示装备成功
bool Equipment::OnEquip(ICreature *pCreature) 
{
	//强化增加百分比
	INT32 StrongAddRate = GetStrongAddRate();

	TGoodsID GoodsID = GetGoodsID();
	//把属性加给玩家
	for(int i = 0 ; i<enEquipProp_Max; i++)
	{
		INT32 Value = g_pThingServer->GetEquipProp(GoodsID,(enEquipProp)i);
		if(Value >0)
		{
			Value += Value* (float)StrongAddRate/100 + 0.9999;

             pCreature->AddCrtPropNum(s_EquipPropMapActorProp[i],Value);
		}
	}

	//镶嵌属性
	for(int i=0; i<MAX_INLAY_NUM; ++i)
	{
		enEquipProp  PropID = enEquipProp_Max;
		INT32  Value = 0;
		GetGemProp(i,PropID ,Value);

		if(PropID != enEquipProp_Max)
		{
			 pCreature->AddCrtPropNum(s_EquipPropMapActorProp[PropID],Value);
		}
	}

	return true;
}

//给生物pCreature卸载该装备，返回TRUE表示卸载成功
bool Equipment::UnEquip(ICreature *pCreature) 
{
	//把属性移除

	//强化增加百分比
	INT32 StrongAddRate = GetStrongAddRate();

	TGoodsID GoodsID = GetGoodsID();
	//把属性加给玩家
	for(int i = 0 ; i<enEquipProp_Max; i++)
	{
		INT32 Value = g_pThingServer->GetEquipProp(GoodsID,(enEquipProp)i);
		if(Value >0)
		{
			Value += Value* (float)StrongAddRate/100 + 0.9999;

             pCreature->AddCrtPropNum(s_EquipPropMapActorProp[i],-Value);
		}
	}

	//镶嵌属性
	for(int i=0; i<MAX_INLAY_NUM; ++i)
	{
		enEquipProp  PropID = enEquipProp_Max;
		INT32  Value = 0;
		GetGemProp(i,PropID ,Value);

		if(PropID != enEquipProp_Max && PropID<ARRAY_SIZE(s_EquipPropMapActorProp))
		{
			 pCreature->AddCrtPropNum(s_EquipPropMapActorProp[PropID],-Value);
		}
	}

	//设置成要更新
	this->SetUpdate(true);

	return true;
}

//获得套装属性ID及属性值
bool Equipment::GetSuitProp(INT32 nIndex, enEquipProp & PropID ,INT32 & value)
{
	PropID = enEquipProp_Max;
	value = 0;

	UINT16 SuitID = this->GetGoodsCnfg()->m_SuitIDOrSwordSecretID;
	if( SuitID == 0 )
	{
		return false;
	}

	const SSuitCnfg* pSuitCnfg = g_pGameServer->GetConfigServer()->GetSuitCnfg(SuitID);

	if(pSuitCnfg == 0)
	{
		TRACE("<error>  %s : %d line 找不到套装配置信息 SuitID = %d ", __FUNCTION__,__LINE__,SuitID);
		return false;
	}

	switch(nIndex)
	{
	case 0:
		{
			PropID = (enEquipProp)pSuitCnfg->m_SuitPropID1;
			value = pSuitCnfg->m_SuitPropValue1;
		}
		break;
	case 1:
		{
			PropID = (enEquipProp)pSuitCnfg->m_SuitPropID2;
			value = pSuitCnfg->m_SuitPropValue2;
		}
		break;
	default:
		return false;
		break;
	}

	return true;
}


//激活套装属性
void Equipment::ActiveSuitProp(IActor* pActor,enGoodsProp PropID)
{
	enEquipProp  EquipPropID;

	INT32 Value = 0;
	
	GetSuitProp(PropID-enGoodsProp_ActiveSuit1,EquipPropID,Value);

	if(EquipPropID != enEquipProp_Max && EquipPropID<ARRAY_SIZE(s_EquipPropMapActorProp) && Value>0)
	{
		 pActor->AddCrtPropNum(s_EquipPropMapActorProp[EquipPropID],Value);
		 this->SetPropNum(pActor,PropID,true);
	}
}

//不激活套装属性
void Equipment::InActiveSuitProp(IActor* pActor,enGoodsProp PropID)
{	
	enEquipProp  EquipPropID;

	INT32 Value = 0;
	
	GetSuitProp(PropID-enGoodsProp_ActiveSuit1,EquipPropID,Value);

	if(EquipPropID != enEquipProp_Max && EquipPropID<ARRAY_SIZE(s_EquipPropMapActorProp) && Value>0)
	{
		 pActor->AddCrtPropNum(s_EquipPropMapActorProp[EquipPropID],-Value);
		 this->SetPropNum(pActor,PropID,false);
	}
}




//得到是否需要更新到数据库
bool Equipment::GetNeedUpdate()
{
	return Super::GetNeedUpdate();
}

//设置是否需要更新到数据库
void Equipment::SetUpdate(bool bNeedUpdate)
{
	Super::SetUpdate(bNeedUpdate);
}
