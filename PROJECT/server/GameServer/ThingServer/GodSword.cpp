

#include "GodSword.h"
#include "IBasicService.h"
#include "IConfigServer.h"
#include "ICombatServer.h"
#include "DMsgSubAction.h"


GodSword::GodSword()
{
	memset(m_GodSwordProp,0,sizeof(m_GodSwordProp));
	m_pMagic = 0;
}
GodSword::~GodSword()
{
}

//初始化，
bool GodSword::Create(void) 
{
	if(Super::Create()==false)
	{
		return false;
	}

	 SetMagicValue(0);
	 SetSwordkee(0);

	return true;
}

void GodSword::Release()
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
bool GodSword::OnGetPublicContext(void * buf, int &nLen)
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

	pPublicData->m_GoodsClass = enGoodsCategory_GodSword;

	pPublicData->m_GodSwordProp.m_Nimbus = __GetPropValue(enGoodsProp_Nimbus);

	pPublicData->m_GodSwordProp.m_SwordLevel = __GetPropValue(enGoodsProp_SwordLevel);

	pPublicData->m_GodSwordProp.m_SecretLevel = __GetPropValue(enGoodsProp_SecretLevel);

	pPublicData->m_GodSwordProp.m_MagicValue = __GetPropValue(enGoodsProp_MagicValue);

	pPublicData->m_GodSwordProp.m_SwordkeeValue = __GetPropValue(enGoodsProp_SwordkeeValue);

	pPublicData->m_GodSwordProp.m_KillNpcNum = __GetPropValue(enGoodsProp_KillNpcNum);

	pPublicData->m_GodSwordProp.m_FuBenFloor = __GetPropValue(enGoodsProp_FuBenLevel);

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
bool GodSword::OnGetPrivateContext(void * buf, int &nLen)
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

	pPrivateData->m_GoodsClass = enGoodsCategory_GodSword;

	pPrivateData->m_GodSwordProp.m_Nimbus = __GetPropValue(enGoodsProp_Nimbus);

	pPrivateData->m_GodSwordProp.m_SwordLevel = __GetPropValue(enGoodsProp_SwordLevel);

	pPrivateData->m_GodSwordProp.m_SecretLevel = __GetPropValue(enGoodsProp_SecretLevel);

	pPrivateData->m_GodSwordProp.m_MagicValue = __GetPropValue(enGoodsProp_MagicValue);

	pPrivateData->m_GodSwordProp.m_SwordkeeValue = __GetPropValue(enGoodsProp_SwordkeeValue);

	pPrivateData->m_GodSwordProp.m_KillNpcNum = __GetPropValue(enGoodsProp_KillNpcNum);

	pPrivateData->m_GodSwordProp.m_FuBenFloor = __GetPropValue(enGoodsProp_FuBenLevel);

	nLen = sizeof(SGoodsPrivateData);

	return true;
}

bool GodSword::OnSetPrivateContext(const void * buf, int nLen)
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

	__SetPropValue(enGoodsProp_Nimbus,pPrivateData->m_GodSwordProp.m_Nimbus);

	__SetPropValue(enGoodsProp_SwordLevel,pPrivateData->m_GodSwordProp.m_SwordLevel);

	__SetPropValue(enGoodsProp_SecretLevel,pPrivateData->m_GodSwordProp.m_SecretLevel);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool GodSword::OnSetDBContext(const void * buf, int nLen)
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

	__SetPropValue(enGoodsProp_Nimbus,pPrivateData->m_GodSwordProp.m_Nimbus);

	__SetPropValue(enGoodsProp_SwordLevel,pPrivateData->m_GodSwordProp.m_SwordLevel);

	__SetPropValue(enGoodsProp_SecretLevel,pPrivateData->m_GodSwordProp.m_SecretLevel);

	__SetPropValue(enGoodsProp_FuBenLevel,pPrivateData->m_GodSwordProp.m_FuBenLevel);

	__SetPropValue(enGoodsProp_KillNpcNum,pPrivateData->m_GodSwordProp.m_KillMonsterNum);

	__SetPropValue(enGoodsProp_LastEnterTime,pPrivateData->m_GodSwordProp.m_LastEnterFuBenTime);

	__SetPropValue(enGoodsProp_KillNpcNum,pPrivateData->m_GodSwordProp.m_KillMonsterNum);

	__SetPropValue(enGoodsProp_KillNpcNum,pPrivateData->m_GodSwordProp.m_KillMonsterNum);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool GodSword::OnGetDBContext(void * buf, int &nLen)
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

	SDBGodSwordProp * pDBGodSword = (SDBGodSwordProp *)&pDBGoods->GoodsData;

	pDBGodSword->m_Nimbus = __GetPropValue(enGoodsProp_Nimbus);

	pDBGodSword->m_SwordLevel = __GetPropValue(enGoodsProp_SwordLevel);

	pDBGodSword->m_SecretLevel = __GetPropValue(enGoodsProp_SecretLevel);	

	pDBGodSword->m_FuBenLevel = __GetPropValue(enGoodsProp_FuBenLevel);

	pDBGodSword->m_KillMonsterNum = __GetPropValue(enGoodsProp_KillNpcNum);

	pDBGodSword->m_LastEnterFuBenTime = __GetPropValue(enGoodsProp_LastEnterTime);


	nLen = sizeof(SDB_Save_GoodsReq);

	return true;
}


//取得物品的类别（例如：装备、消耗品）
enGoodsCategory GodSword::GetGoodsClass(void)
{
	return enGoodsCategory_GodSword;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：改变本物品数字型属性，将原来的值增加nValue
// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
// 输  出：pNewValue返回改变后的新值

//////////////////////////////////////////////////////////////////////////
bool GodSword::AddPropNum(IActor* pActor,enGoodsProp enPropID, int nValue,int * pNewValue)
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
bool GodSword::SetPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue)
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::SetPropNum(pActor,enPropID,nValue,pNewValue);
	}
	else if(enPropID >enGoodsProp_GodSword && enPropID<enGoodsProp_GodSwordEnd)
	{
		__SetPropValue(enPropID,nValue);

		INT32 NewValue = __GetPropValue(enPropID);

		if(pNewValue)
		{
			*pNewValue = NewValue;
		}
       
		if(pActor)
		{
			INT32 ActorProp = GodSwordPropMapID[enPropID - enGoodsProp_GodSword - 1];
			if( -1 != ActorProp){
				pActor->NoticClientUpdateThing(GetUID(),ActorProp,NewValue);
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
bool GodSword::GetPropNum(enGoodsProp enPropID,int & nValue)
{
	if(enPropID<enGoodsProp_BaseEnd)
	{
		return Super::GetPropNum(enPropID,nValue);
	}
	else if(enPropID >enGoodsProp_GodSword && enPropID<enGoodsProp_GodSwordEnd)
	{
       nValue =  m_GodSwordProp[enPropID-enGoodsProp_GodSword];		
	
	   return true;
	}
	else
	{
		TRACE("%s : %d line 意外的属性ID enPropID=%d", __FUNCTION__,__LINE__,enPropID);
	}

	return false;
}

//获得物品数量
int GodSword::GetNumber()
{
	return 1;
}


//获得剑气值
int  GodSword::GetSwordkee()
{
	return __GetPropValue(enGoodsProp_SwordkeeValue);	
}

//重新设置剑气值
void  GodSword::SetSwordkee(IActor * pActor)
{	

	//级别
	int SwordLevel = 0;
		
	GetPropNum(enGoodsProp_SwordLevel,SwordLevel);

	INT32 nCurValue = GetSwordkeeUpLevel(SwordLevel);

	 SetPropNum(pActor,enGoodsProp_SwordkeeValue,nCurValue);
}

//获得法术值
int  GodSword::GetMagicValue()
{
	return __GetPropValue(enGoodsProp_MagicValue);
}

//重新设置法术值
void  GodSword::SetMagicValue(IActor * pActor)
{
		
	//级别
	int SwordLevel = 0;
		
	GetPropNum(enGoodsProp_SwordLevel,SwordLevel);

	INT32 nCurValue = GetMagicUpLevel(SwordLevel);

	 SetPropNum(pActor,enGoodsProp_MagicValue,nCurValue);
}

//提升仙剑级别(value为负时降级)
 bool GodSword::UpgradeSword(IActor * pActor,INT8 value) 
 {
	 if(AddPropNum(pActor,enGoodsProp_SwordLevel,value)==false)
	 {
		 return false;
	 }

	 SetMagicValue(pActor);
	 SetSwordkee(pActor);

	 pActor->RecalculateProp(); //重新计算伤害

/*	//发事件
	 SS_SwordKee SwordKee;
	 GetPropNum(enGoodsProp_SwordLevel, SwordKee.m_SwordLevel);

	 UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Swordkee);
	 pActor->OnEvent(msgID,&SwordKee,sizeof(SwordKee));*/
	 return true;
 }

//得到是否需要更新到数据库
bool GodSword::GetNeedUpdate()
{
	return Super::GetNeedUpdate();
}

//设置是否需要更新到数据库
void GodSword::SetUpdate(bool bNeedUpdate)
{
	Super::SetUpdate(bNeedUpdate);
}

 //获得法术
IMagic * GodSword::GetMagic()
{
	IMagic * pMagic = 0;

	//先判断是否有剑诀
	 const SGodSwordWorldCnfg * pSwordWorldCnfg = g_pGameServer->GetConfigServer()->GetGoldSwordWorldCnfg(this->GetGoodsCnfg()->m_SuitIDOrSwordSecretID);
	 if(pSwordWorldCnfg)
	 {
		 //获取剑诀等级
		 INT32 SecretLevel = __GetPropValue(enGoodsProp_SecretLevel);
		 if(SecretLevel>0)
		 {
			 SCreateMagicCnt MagicCnt ;
			 MagicCnt.m_MagicID = pSwordWorldCnfg->m_MagicID;
			 MagicCnt.m_Level = SecretLevel;
			 pMagic = g_pGameServer->GetCombatServer()->CreateMagic(MagicCnt);
		 }
	 }
	return pMagic;
}

//获得提升一级剑气值
int  GodSword::GetSwordkeeUpLevel(int Level)
{
	const SGoodsCnfg * pGoodsCnfg = GetGoodsCnfg();

	int nCurValue = pGoodsCnfg->m_BloodOrSwordkee;
    //提升的法术值=仙剑法术基础值*5%
	return (float)nCurValue*(100+5*(Level-1))/100+0.9999;
}

//获得提升一级法术值
int  GodSword::GetMagicUpLevel(int Level)
{
	const SGoodsCnfg * pGoodsCnfg = GetGoodsCnfg();

	int nCurValue = pGoodsCnfg->m_SpiritOrMagic;

	 //提升的法术值=仙剑法术基础值*5%

	return (float)nCurValue*(100 + 5 *(Level-1)) /100 + 0.9999;
}

//检查生物pCreature是否能装备该项仙剑，返回TRUE表示可以装备
bool GodSword::CanEquip(ICreature *pCreature,UINT8 pos,std::string & strErr)
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

//给生物pCreature装备该仙剑，返回TRUE表示装备成功
bool GodSword::OnEquip(ICreature *pCreature)
{
	if(pCreature->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	IActor * pActor = (IActor*)pCreature;
	
	pActor->RecalculateProp();

	return true;
}

//给生物pCreature卸载该仙剑，返回TRUE表示卸载成功
bool GodSword::UnEquip(ICreature *pCreature)
{
	if(pCreature->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	IActor * pActor = (IActor*)pCreature;
	
	pActor->RecalculateProp();

	return true;
}

INT32 GodSword::__GetPropValue(enGoodsProp enPropID)
{
	if(enPropID>=enGoodsProp_GodSword && enPropID<enGoodsProp_GodSwordEnd)
	{
		return m_GodSwordProp[enPropID-enGoodsProp_GodSword];
	}

	return Super::__GetPropValue(enPropID);
}

void GodSword::__SetPropValue(enGoodsProp enPropID,INT32 nValue)
{
	if(enPropID>=enGoodsProp_GodSword && enPropID<enGoodsProp_GodSwordEnd)
	{
		INT32 MaxValue = GetMaxValue(enPropID);
		if(nValue>MaxValue)
		{
			nValue = MaxValue;
		}
		m_GodSwordProp[enPropID-enGoodsProp_GodSword] = nValue;
	}
	else
	{
		 Super::__SetPropValue(enPropID,nValue);
	}

	//有变更,需要保存到数据库
	this->SetUpdate(true);
}

//获得最大值
int GodSword::GetMaxValue(enGoodsProp enPropID)
{
	switch(enPropID)
	{
	case enGoodsProp_Nimbus:
		return g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGodSwordNimbus;
		break;
	default:
		break;
	}

	return 0x7fffffff;
}
