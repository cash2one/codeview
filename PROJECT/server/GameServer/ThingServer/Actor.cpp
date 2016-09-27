

#include "Actor.h"
#include "IThingPart.h"
#include "DBProtocol.h"
#include "ThingServer.h"
#include "IDBProxyClient.h"
#include "GameSrvProtocol.h"
#include "ISession.h"
#include "IResOutputPart.h"
#include "IFuMoDongPart.h"
#include "ITrainingHallPart.h"
#include "IGameWorld.h"
#include "IEquipPart.h"
#include "IGatherGodHousePart.h"
#include "IXiuLianPart.h"
#include "IFriendPart.h"
#include "IMailPart.h"
#include "IMagicPart.h"
#include "ICombatPart.h"
#include "IFuBenPart.h"
#include "ISynMagicPart.h"
#include "ISyndicateMember.h"
#include "ISyndicateMgr.h"
#include "ITalismanPart.h"
#include "IGodSword.h"
#include "IActorBasicPart.h"
#include "IConfigServer.h"
#include "IGameScene.h"
#include "ITaskPart.h"
#include "IChengJiuPart.h"
#include "IActivityPart.h"
#include "IMonster.h"
#include "IMagic.h"
#include "XDateTime.h"
#include "ICDTime.h"
#include "IStatusPart.h"
#include "IDouFaPart.h"
#include "ISynPart.h"
#include "ITeamPart.h"
#include "StatusPart.h"
#include "ITalisman.h"
#include "ISynPart.h"
#include "ITalismanPart.h"


Actor::Actor()
{
	memset(m_szNameOld,0,sizeof(m_szNameOld));
	m_pSession = 0;
	memset(m_pThingParts,0,sizeof(m_pThingParts));
	memset(m_ActorProp,0,sizeof(m_ActorProp));
	for(int i=0; i<sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0]);++i)
	{
		m_uidEmploy[i] = UID();
	}

	m_pMaster = 0;

	m_LastSaveToDBTime = 0;

	m_bNeedSave = false;

	m_SourceActor = 0;

	for( int i = 0; i < (int)enUseFlag_Max; ++i)
	{
		bool bOk = this->GetUseFlag((enUseFlag)i);
		bOk = false;
	}

	m_bInCombat = false;

	m_bSelfOnline = false;
}

Actor::~Actor()
{

}


//初始化，
bool Actor::Create(void)
{	
	LevelOrAptitudeChange();
	return true;
}

void Actor::Release()
{
	if(this->IsClone()==false)
	{
		//离开场景
		TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
		IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

		if(pGameScene != 0)
		{
			pGameScene->LeaveScene(this);
		}


		if( 0 == m_pMaster)
		{
			//删除主场景
			g_pGameServer->GetGameWorld()->DeleteGameScene(TSceneID(__GetPropValue(enCrtProp_ActorMainSceneID)));

			//删除伏魔洞场景
			g_pGameServer->GetGameWorld()->DeleteGameScene(TSceneID(__GetPropValue(enCrtProp_ActorFuMoDongSceneID)));

			//删除后山场景
			g_pGameServer->GetGameWorld()->DeleteGameScene(TSceneID(__GetPropValue(enCrtProp_ActorHouShanSceneID)));

			//删除副本所有场景
			IFuBenPart * pFuBenPart = this->GetFuBenPart();
			if( 0 != pFuBenPart)
			{
				pFuBenPart->ReleaseScene();
			}
		}

		for(int i=0; i<ARRAY_SIZE(m_pThingParts);i++)
		{
			if(m_pThingParts[i] != 0)
			{
				m_pThingParts[i]->Release();
				m_pThingParts[i] = 0;
			}
		}

		//卸载与玩家有关的所有冷却时间
		ICDTimeMgr * pCDTimeMgr = g_pGameServer->GetCDTimeMgr();
		if( 0 != pCDTimeMgr){
			pCDTimeMgr->UnLoadCDTime(this);
		}

		//先卸载招募角色
		for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
		{
			IActor * pEmployee = this->GetEmployee(i);
			if( 0 != pEmployee){
				g_pGameServer->GetGameWorld()->DestroyThing(pEmployee->GetUID());
			}
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
bool Actor::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SActorPublicData))
	{
		return false;
	}


	SActorPublicData * pBasicData = (SActorPublicData*)(unsigned char*)buf;

	//经验
	pBasicData->m_ActorExp = __GetPropValue(enCrtProp_ActorExp);

	//境界
	pBasicData->m_ActorLayer = __GetPropValue(enCrtProp_ActorLayer) ;

	//灵气
	pBasicData->m_ActorNimbus = __GetPropValue(enCrtProp_ActorNimbus);

	//资质
	pBasicData->m_ActorAptitude = __GetPropValue(enCrtProp_ActorAptitude);

	//性别
	pBasicData->m_ActorSex = __GetPropValue(enCrtProp_ActorSex);

	//UserID
	pBasicData->m_ActorUserID = __GetPropValue(enCrtProp_ActorUserID);

	//外观
	pBasicData->m_ActorFacade = __GetPropValue(enCrtProp_ActorFacade);

	//物理伤害
	pBasicData->m_ActorPhysics = __GetPropValue(enCrtProp_ActorPhysics);

	//法术伤害
	pBasicData->m_ActorMagic  = __GetPropValue(enCrtProp_ActorMagic);

	//防御
	pBasicData->m_ActorDefend = __GetPropValue(enCrtProp_ActorDefend);

	//灵气速率
	pBasicData->m_ActorNimbusSpeed = __GetPropValue(enCrtProp_ActorNimbusSpeed);

	if(m_pMaster)
	{
		pBasicData->m_uidMaster = m_pMaster->GetUID();
	}
	else
	{
		pBasicData->m_uidMaster = UID();
	}

	memcpy(pBasicData->m_uidEmploy ,m_uidEmploy,sizeof(pBasicData->m_uidEmploy));

	//场景
	TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
	pBasicData->m_SceneID = SceneID;


	//生物共有属性

	//等级
	pBasicData->m_Level = __GetPropValue(enCrtProp_Level) ;

	//灵力
	pBasicData->m_Spirit	  = __GetPropValue(enCrtProp_ActorSpiritBasic);
	pBasicData->m_SpiritEquip = __GetPropValue(enCrtProp_ActorSpiritEqup);

	//护盾
	pBasicData->m_Shield	  = __GetPropValue(enCrtProp_ActorShieldBasic);
	pBasicData->m_ShieldEquip = __GetPropValue(enCrtProp_ActorShieldEqup);

	//气血
	pBasicData->m_BloodUp	   = __GetPropValue(enCrtProp_ActorBloodUpBasic);
	pBasicData->m_BloodUpEquip = __GetPropValue(enCrtProp_ActorBloodEqup);

	//身法
	pBasicData->m_Avoid		 = __GetPropValue(enCrtProp_ActorAvoidBasic);
	pBasicData->m_AvoidEquip = __GetPropValue(enCrtProp_ActorAvoidEqup);

	//方向
	pBasicData->m_nDir = __GetPropValue(enCrtProp_Dir);

	//能力
	pBasicData->m_NenLi = this->GetNenLi();

	//thing共有属性
	strncpy(pBasicData->m_szName,m_szName,sizeof(pBasicData->m_szName));

	pBasicData->m_uid = GetUID();

	nLen = sizeof(SActorPrivateData);


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Actor::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SActorPrivateData))
	{
		return false;
	}


	SActorPrivateData * pBasicData = (SActorPrivateData*)(unsigned char*)buf;

	//经验
	pBasicData->m_ActorExp = __GetPropValue(enCrtProp_ActorExp);

	//境界
	pBasicData->m_ActorLayer = __GetPropValue(enCrtProp_ActorLayer) ;

	//灵气
	pBasicData->m_ActorNimbus = __GetPropValue(enCrtProp_ActorNimbus);

	//资质
	pBasicData->m_ActorAptitude = __GetPropValue(enCrtProp_ActorAptitude);

	//性别
	pBasicData->m_ActorSex = __GetPropValue(enCrtProp_ActorSex);

	//UserID
	pBasicData->m_ActorUserID = __GetPropValue(enCrtProp_ActorUserID);

	//游戏币
	pBasicData->m_ActorMoney  = __GetPropValue(enCrtProp_ActorMoney);

	//礼券
	pBasicData->m_ActorTicket  = __GetPropValue(enCrtProp_ActorTicket);

	//灵石
	pBasicData->m_ActorStone  = __GetPropValue(enCrtProp_ActorStone);

	//外观
	pBasicData->m_ActorFacade = __GetPropValue(enCrtProp_ActorFacade);

	//气血上限
	//	pBasicData->m_ActorBloodUp = __GetPropValue(enCrtProp_ActorBloodUp);

	//物理伤害
	pBasicData->m_ActorPhysics = __GetPropValue(enCrtProp_ActorPhysics);

	//法术伤害
	pBasicData->m_ActorMagic  = __GetPropValue(enCrtProp_ActorMagic);

	//防御
	pBasicData->m_ActorDefend = __GetPropValue(enCrtProp_ActorDefend);

	//灵气速率
	pBasicData->m_ActorNimbusSpeed = __GetPropValue(enCrtProp_ActorNimbusSpeed);

	//城市编号
	pBasicData->m_ActorCityID = __GetPropValue(enCrtProp_ActorCityID);

	//荣誉
	pBasicData->m_ActorHonor  = __GetPropValue(enCrtProp_ActorHonor);

	//声望
	pBasicData->m_ActorCredit = __GetPropValue(enCrtProp_ActorCredit);

	//能力
	pBasicData->m_NenLi = this->GetNenLi();

	//仙剑灵气
	pBasicData->m_GodSwordNimbus = __GetPropValue(enCrtProp_ActorGodSwordNimbus);

	//聚灵气	
	pBasicData->m_ActorPolyNimbus = __GetPropValue(enCrtProp_ActorPolyNimbus);


	//爆击	
	pBasicData->m_Crit = __GetPropValue(enCrtProp_ActorCrit);

	//坚韧	
	pBasicData->m_Tenacity = __GetPropValue(enCrtProp_ActorTenacity);

	//命中	
	pBasicData->m_Hit = __GetPropValue(enCrtProp_ActorHit);

	//回避	
	pBasicData->m_Dodge = __GetPropValue(enCrtProp_ActorDodge);
	//法术回复	
	pBasicData->m_MagicCD = __GetPropValue(enCrtProp_MagicCD);


	//爆击等级	
	pBasicData->m_CritLv = __GetPropValue(enCrtProp_ActorCritLv);

	//坚韧等级	
	pBasicData->m_TenacityLv = __GetPropValue(enCrtProp_ActorTenacityLv);

	//命中等级	
	pBasicData->m_HitLv = __GetPropValue(enCrtProp_ActorHitLv);

	//回避等级	
	pBasicData->m_DodgeLv = __GetPropValue(enCrtProp_ActorDodgeLv);

	//法术回复等级	
	pBasicData->m_MagicCDLv = __GetPropValue(enCrtProp_MagicCDLv);

	//历史充值总额
	pBasicData->m_Recharge = __GetPropValue(enCrtProp_Recharge);


	//战斗力	
	pBasicData->m_CombatAbility = __GetPropValue(enCrtProp_ActorCombatAbility);

	//金剑诀伤害等级	
	pBasicData->m_GoldDamageLv = __GetPropValue(enCrtProp_GoldDamageLv);

	//木剑诀伤害等级	
	pBasicData->m_WoodDamageLv = __GetPropValue(enCrtProp_WoodDamageLv);

	//水剑诀伤害等级	
	pBasicData->m_WaterDamageLv = __GetPropValue(enCrtProp_WaterDamageLv);

	//火剑诀伤害等级	
	pBasicData->m_FireDamageLv = __GetPropValue(enCrtProp_FireDamageLv);

	//土剑诀伤害等级	
	pBasicData->m_SoilDamageLv = __GetPropValue(enCrtProp_SoilDamageLv);

	//金剑诀伤害	
	pBasicData->m_GoldDamage = __GetPropValue(enCrtProp_GoldDamage);

	//木剑诀伤害	
	pBasicData->m_WoodDamage = __GetPropValue(enCrtProp_WoodDamage);

	//水剑诀伤害	
	pBasicData->m_WaterDamage = __GetPropValue(enCrtProp_WaterDamage);

	//火剑诀伤害	
	pBasicData->m_FireDamage = __GetPropValue(enCrtProp_FireDamage);

	//土剑诀伤害	
	pBasicData->m_SoilDamage = __GetPropValue(enCrtProp_SoilDamage);

	//灵魄	
	pBasicData->m_GhostSoul = __GetPropValue(enCrtProp_GhostSoul);

	//夺宝等级
	pBasicData->m_DuoBaoLevel = __GetPropValue(enCrtProp_DuoBaoLevel);

	if(m_pMaster)
	{
		pBasicData->m_uidMaster = m_pMaster->GetUID();
	}
	else
	{
		pBasicData->m_uidMaster = UID();
	}

	memcpy(pBasicData->m_uidEmploy ,m_uidEmploy,sizeof(pBasicData->m_uidEmploy));

	//场景
	TSceneID SceneID(__GetPropValue(enCrtProp_SceneID));
	pBasicData->m_SceneID = SceneID;

	//VIP等级
	pBasicData->m_VipLevel = __GetPropValue(enCrtProp_TotalVipLevel);

	//生物共有属性

	//等级
	pBasicData->m_Level = __GetPropValue(enCrtProp_Level) ;

	//灵力
	pBasicData->m_Spirit = __GetPropValue(enCrtProp_ActorSpiritBasic);
	pBasicData->m_SpiritEquip = __GetPropValue(enCrtProp_ActorSpiritEqup);

	//护盾
	pBasicData->m_Shield = __GetPropValue(enCrtProp_ActorShieldBasic);
	pBasicData->m_ShieldEquip = __GetPropValue(enCrtProp_ActorShieldEqup);

	//气血上限
	pBasicData->m_BloodUp = __GetPropValue(enCrtProp_ActorBloodUpBasic);
	pBasicData->m_BloodUpEquip = __GetPropValue(enCrtProp_ActorBloodEqup);

	//身法
	pBasicData->m_Avoid = __GetPropValue(enCrtProp_ActorAvoidBasic);
	pBasicData->m_AvoidEquip = __GetPropValue(enCrtProp_ActorAvoidEqup);

	//方向
	pBasicData->m_nDir = __GetPropValue(enCrtProp_Dir);

	//thing共有属性
	strncpy(pBasicData->m_szName,m_szName,sizeof(pBasicData->m_szName));

	pBasicData->m_uid = GetUID();

	nLen = sizeof(SActorPrivateData);


	return true;
}

bool Actor::OnSetPrivateContext(const void * buf, int nLen)
{
	if(buf==0 || nLen<sizeof(SActorPrivateData))
	{
		return false;
	}

	SActorPrivateData * pBasicData = (SActorPrivateData*)(unsigned char*)buf;

	//经验
	__SetPropValue(enCrtProp_ActorExp, pBasicData->m_ActorExp);

	//境界
	__SetPropValue(enCrtProp_ActorLayer,pBasicData->m_ActorLayer) ;

	//灵气
	__SetPropValue(enCrtProp_ActorNimbus, pBasicData->m_ActorNimbus);

	//资质
	__SetPropValue(enCrtProp_ActorAptitude,pBasicData->m_ActorAptitude) ;

	//能力
	__SetPropValue(enCrtProp_ActorNenLi, (int)this->GetNenLi());

	//性别
	__SetPropValue(enCrtProp_ActorSex,pBasicData->m_ActorSex);

	//UserID
	__SetPropValue(enCrtProp_ActorUserID,pBasicData->m_ActorUserID);

	//游戏币
	__SetPropValue(enCrtProp_ActorMoney,pBasicData->m_ActorMoney);

	//礼券
	__SetPropValue(enCrtProp_ActorTicket,pBasicData->m_ActorTicket );

	//灵石
	__SetPropValue(enCrtProp_ActorStone, pBasicData->m_ActorStone);

	//外观
	__SetPropValue(enCrtProp_ActorFacade,pBasicData->m_ActorFacade);

	//灵气速率
	__SetPropValue(enCrtProp_ActorNimbusSpeed,pBasicData->m_ActorNimbusSpeed);

	//气血上限
	__SetPropValue(enCrtProp_ActorBloodUp,pBasicData->m_BloodUp);
	__SetPropValue(enCrtProp_Blood,pBasicData->m_BloodUp);  //气血恢复到上限

	//城市编号
	__SetPropValue(enCrtProp_ActorCityID, pBasicData->m_ActorCityID);

	//荣誉
	__SetPropValue(enCrtProp_ActorHonor, pBasicData->m_ActorHonor);

	//声望
	__SetPropValue(enCrtProp_ActorCredit, pBasicData->m_ActorCredit);

	//仙剑灵气
	__SetPropValue(enCrtProp_ActorGodSwordNimbus, pBasicData->m_GodSwordNimbus);

	//聚灵气	
	__SetPropValue(enCrtProp_ActorPolyNimbus, pBasicData->m_ActorPolyNimbus);

	//爆击	
	__SetPropValue(enCrtProp_ActorCrit, pBasicData->m_Crit);

	//坚韧	
	__SetPropValue(enCrtProp_ActorTenacity, pBasicData->m_Tenacity);

	//命中	
	__SetPropValue(enCrtProp_ActorHit, pBasicData->m_Hit);

	//回避	
	__SetPropValue(enCrtProp_ActorDodge, pBasicData->m_Dodge);

	//法术回复	
	__SetPropValue(enCrtProp_MagicCD, pBasicData->m_MagicCD);


	//爆击等级	
	__SetPropValue(enCrtProp_ActorCritLv, pBasicData->m_CritLv);

	//坚韧等级		
	__SetPropValue(enCrtProp_ActorTenacityLv, pBasicData->m_TenacityLv);

	//命中等级		
	__SetPropValue(enCrtProp_ActorHitLv, pBasicData->m_HitLv);

	//回避等级		
	__SetPropValue(enCrtProp_ActorDodgeLv, pBasicData->m_DodgeLv);

	//法术回复等级		
	__SetPropValue(enCrtProp_MagicCDLv, pBasicData->m_MagicCDLv);


	//金剑诀伤害等级	
	__SetPropValue(enCrtProp_GoldDamageLv, pBasicData->m_GoldDamageLv);

	//木剑诀伤害等级	
	__SetPropValue(enCrtProp_WoodDamageLv, pBasicData->m_WoodDamageLv);

	//水剑诀伤害等级	
	__SetPropValue(enCrtProp_WaterDamageLv, pBasicData->m_WaterDamageLv);

	//火剑诀伤害等级	
	__SetPropValue(enCrtProp_FireDamageLv, pBasicData->m_FireDamageLv);

	//土剑诀伤害等级	
	__SetPropValue(enCrtProp_SoilDamageLv, pBasicData->m_SoilDamageLv);


	//金剑诀伤害	
	__SetPropValue(enCrtProp_GoldDamage, pBasicData->m_GoldDamage);

	//木剑诀伤害	
	__SetPropValue(enCrtProp_WoodDamage, pBasicData->m_WoodDamage);

	//水剑诀伤害	
	__SetPropValue(enCrtProp_WaterDamage, pBasicData->m_WaterDamage);

	//火剑诀伤害	
	__SetPropValue(enCrtProp_FireDamage, pBasicData->m_FireDamage);

	//土剑诀伤害	
	__SetPropValue(enCrtProp_SoilDamage, pBasicData->m_SoilDamage);	

	//灵魄	
	__SetPropValue(enCrtProp_GhostSoul, pBasicData->m_GhostSoul);

	if(pBasicData->m_uidMaster.IsValid())
	{
		m_pMaster = g_pGameServer->GetGameWorld()->FindActor(pBasicData->m_uidMaster);
	}

	memcpy(m_uidEmploy,pBasicData->m_uidEmploy,sizeof(m_uidEmploy));

	//生物共有属性

	//等级
	__SetPropValue(enCrtProp_Level,pBasicData->m_Level) ;

	//灵力
	__SetPropValue(enCrtProp_ActorSpiritBasic,pBasicData->m_Spirit);
	__SetPropValue(enCrtProp_ActorSpiritEqup,pBasicData->m_SpiritEquip);

	//护盾
	__SetPropValue(enCrtProp_ActorShieldBasic,pBasicData->m_Shield);
	__SetPropValue(enCrtProp_ActorShieldEqup,pBasicData->m_ShieldEquip);

	//气血
	__SetPropValue(enCrtProp_ActorBloodUpBasic,pBasicData->m_BloodUp);
	__SetPropValue(enCrtProp_ActorBloodEqup,pBasicData->m_BloodUpEquip);

	//身法
	__SetPropValue(enCrtProp_ActorAvoidBasic,pBasicData->m_Avoid);
	__SetPropValue(enCrtProp_ActorAvoidEqup,pBasicData->m_AvoidEquip);

	//方向
	__SetPropValue(enCrtProp_Dir,pBasicData->m_nDir);

	//坐标
	XPoint ptLoc = GetLoc();
	ptLoc.x = pBasicData->m_ptX;
	ptLoc.y = pBasicData->m_ptY;


	//thing共有属性
	strncpy(m_szName,pBasicData->m_szName,sizeof(m_szName));

	strncpy(m_szNameOld,pBasicData->m_szName,sizeof(m_szNameOld));


	SetUID(pBasicData->m_uid);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool Actor::OnSetDBContext(const void * buf, int nLen)
{
	if( 0 == buf || nLen < sizeof(SDB_Get_ActorBasicData_Rsp)){
		return false;
	}

	SDB_Get_ActorBasicData_Rsp * pActorData = (SDB_Get_ActorBasicData_Rsp *)buf;

	//经验
	__SetPropValue(enCrtProp_ActorExp, pActorData->ActorExp);

	//境界
	__SetPropValue(enCrtProp_ActorLayer, pActorData->ActorLayer);

	//境界配置信息
	const SActorLayerCnfg * pLayerCnfoOld = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(pActorData->ActorLayer);

	if ( 0 == pLayerCnfoOld){

		TRACE("<error> %s : %d Line 获取境界配置信息出错！！境界等级=%d,UserID = %d", __FUNCTION__, __LINE__, pActorData->ActorLayer, pActorData->UserID);
		return false;
	}

	//灵气
	__SetPropValue(enCrtProp_ActorNimbus, pActorData->ActorNimbus);

	//资质
	__SetPropValue(enCrtProp_ForeverAptitude, pActorData->ActorAptitude);

	INT32 nTotalAptitude = pActorData->ActorAptitude + pLayerCnfoOld->m_AddAptitude;

	__SetPropValue(enCrtProp_ActorAptitude, nTotalAptitude);

	//能力
	__SetPropValue(enCrtProp_ActorNenLi, (int)this->GetNenLi());

	//性别
	__SetPropValue(enCrtProp_ActorSex, pActorData->ActorSex);

	//UserID
	__SetPropValue(enCrtProp_ActorUserID, pActorData->UserID);

	//游戏币
	__SetPropValue(enCrtProp_ActorMoney, pActorData->ActorMoney);

	//礼券
	__SetPropValue(enCrtProp_ActorTicket, pActorData->ActorTicket);

	//灵石
	__SetPropValue(enCrtProp_ActorStone, pActorData->ActorStone);

	//外观
	__SetPropValue(enCrtProp_ActorFacade, pActorData->ActorFacade);



	//气血上限
	__SetPropValue(enCrtProp_ForeverBloodUp,pActorData->ActorBloodUp);

	INT32 BasicBloodUp = RecalculateBoold(nTotalAptitude / 1000.0f,pActorData->Level , pActorData->ActorBloodUp , pLayerCnfoOld->m_AddBlood);

	__SetPropValue(enCrtProp_ActorBloodUpBasic, BasicBloodUp);
	__SetPropValue(enCrtProp_ActorBloodUp, BasicBloodUp);

	//城市编号
	__SetPropValue(enCrtProp_ActorCityID, pActorData->CityID);

	//荣誉
	__SetPropValue(enCrtProp_ActorHonor, pActorData->Honor);

	//声望
	__SetPropValue(enCrtProp_ActorCredit, pActorData->Credit);

	//灵气速率
	__SetPropValue(enCrtProp_ForeverActorNimbusSpeed, pActorData->ActorNimbusSpeed);
	__SetPropValue(enCrtProp_ActorNimbusSpeed, pActorData->ActorNimbusSpeed + pLayerCnfoOld->m_NimbusSpeed);

	//仙剑灵气
	__SetPropValue(enCrtProp_ActorGodSwordNimbus, pActorData->GodSwordNimbus);

	//聚灵气	
	__SetPropValue(enCrtProp_ActorPolyNimbus, pActorData->ActorPolyNimbus);

	//金剑诀伤害等级	
	__SetPropValue(enCrtProp_GoldDamageLv, pActorData->GoldDamageLv);

	//木剑诀伤害等级	
	__SetPropValue(enCrtProp_WoodDamageLv, pActorData->WoodDamageLv);

	//水剑诀伤害等级	
	__SetPropValue(enCrtProp_WaterDamageLv, pActorData->WaterDamageLv);

	//火剑诀伤害等级	
	__SetPropValue(enCrtProp_FireDamageLv, pActorData->FireDamageLv);

	//土剑诀伤害等级	
	__SetPropValue(enCrtProp_SoilDamageLv, pActorData->SoilDamageLv);

	//金剑诀伤害	
	__SetPropValue(enCrtProp_GoldDamage, pActorData->GoldDamage);

	//木剑诀伤害	
	__SetPropValue(enCrtProp_WoodDamage, pActorData->WoodDamage);

	//水剑诀伤害	
	__SetPropValue(enCrtProp_WaterDamage, pActorData->WaterDamage);

	//火剑诀伤害	
	__SetPropValue(enCrtProp_FireDamage, pActorData->FireDamage);

	//土剑诀伤害	
	__SetPropValue(enCrtProp_SoilDamage, pActorData->SoilDamage);

	//爆击	
	__SetPropValue(enCrtProp_ActorCrit, pActorData->Crit);

	//坚韧	
	__SetPropValue(enCrtProp_ActorTenacity, pActorData->Tenacity);

	//命中	
	__SetPropValue(enCrtProp_ActorHit, pActorData->Hit);

	//回避	
	__SetPropValue(enCrtProp_ActorDodge, pActorData->Dodge);

	//法术回复	
	__SetPropValue(enCrtProp_MagicCD, pActorData->MagicCD);

	//爆击等级	
	__SetPropValue(enCrtProp_ActorCritLv, pActorData->CritLv);

	//坚韧等级		
	__SetPropValue(enCrtProp_ActorTenacityLv, pActorData->TenacityLv);

	//命中等级		
	__SetPropValue(enCrtProp_ActorHitLv, pActorData->HitLv);

	//回避等级		
	__SetPropValue(enCrtProp_ActorDodgeLv, pActorData->DodgeLv);

	//法术回复等级		
	__SetPropValue(enCrtProp_MagicCDLv, pActorData->MagicCDLv);

	//灵魄	
	__SetPropValue(enCrtProp_GhostSoul, pActorData->GhostSoul);


	//帮战等级
	__SetPropValue(enCrtProp_SynCombatLevel, pActorData->SynCombatLevel);

	//夺宝等级
	__SetPropValue(enCrtProp_DuoBaoLevel, pActorData->DuoBaoLevel);


	//上次登录最后在线时间
	__SetPropValue(enCrtProp_LastOnlineTime, pActorData->LastOnlineTime);


	const SCreateActorCnfg & CreateActorCnfg = g_pGameServer->GetConfigServer()->GetCreateActorCnfg();
	//爆击
	if(__GetPropValue(enCrtProp_ActorCrit)<CreateActorCnfg.m_Crit)
	{
		__SetPropValue(enCrtProp_ActorCrit,CreateActorCnfg.m_Crit);
	}

	//坚韧
	if(__GetPropValue(enCrtProp_ActorTenacity)<CreateActorCnfg.m_Tenacity)
	{
		__SetPropValue(enCrtProp_ActorTenacity,CreateActorCnfg.m_Tenacity);
	}

	//命中
	if(__GetPropValue(enCrtProp_ActorHit)<CreateActorCnfg.m_Hit)
	{
		__SetPropValue(enCrtProp_ActorHit,CreateActorCnfg.m_Hit);
	}

	//回避

	if(__GetPropValue(enCrtProp_ActorDodge)<CreateActorCnfg.m_Dodge)
	{
		__SetPropValue(enCrtProp_ActorDodge,CreateActorCnfg.m_Dodge);
	}

	//法术回复
	if(__GetPropValue(enCrtProp_MagicCD)<CreateActorCnfg.m_MagicCD)
	{
		__SetPropValue(enCrtProp_MagicCD,CreateActorCnfg.m_MagicCD);
	}

	//金剑诀伤害
	if(__GetPropValue(enCrtProp_GoldDamage)<CreateActorCnfg.m_GoldDamage)
	{
		__SetPropValue(enCrtProp_GoldDamage,CreateActorCnfg.m_GoldDamage);
	}

	//木剑诀伤害
	if(__GetPropValue(enCrtProp_WoodDamage)<CreateActorCnfg.m_WoodDamage)
	{
		__SetPropValue(enCrtProp_WoodDamage,CreateActorCnfg.m_WoodDamage);
	}

	//水剑诀伤害
	if(__GetPropValue(enCrtProp_WaterDamage)<CreateActorCnfg.m_WaterDamage)
	{
		__SetPropValue(enCrtProp_WaterDamage,CreateActorCnfg.m_WaterDamage);
	}

	//火剑诀伤害

	if(__GetPropValue(enCrtProp_FireDamage)<CreateActorCnfg.m_FireDamage)
	{
		__SetPropValue(enCrtProp_FireDamage,CreateActorCnfg.m_FireDamage);
	}

	//土剑诀伤害
	if(__GetPropValue(enCrtProp_SoilDamage)<CreateActorCnfg.m_SoilDamage)
	{
		__SetPropValue(enCrtProp_SoilDamage,CreateActorCnfg.m_SoilDamage);
	}

	//生物共有属性

	//等级
	__SetPropValue(enCrtProp_Level, pActorData->Level);

	//灵力
	__SetPropValue(enCrtProp_ForeverSpirit, pActorData->Spirit);

	INT32 BasicSpirit = RecalculateSpiri(nTotalAptitude/ 1000.0f, pActorData->Level,pActorData->Spirit , pLayerCnfoOld->m_AddSpirit);

	__SetPropValue(enCrtProp_ActorSpiritBasic,BasicSpirit);
	__SetPropValue(enCrtProp_Spirit, BasicSpirit);

	//护盾
	__SetPropValue(enCrtProp_ForeverShield, pActorData->Shield);

	INT32 BasicShiel = RecalculateShield(nTotalAptitude / 1000.0f , pActorData->Level , pActorData->Shield , pLayerCnfoOld->m_AddShield);

	__SetPropValue(enCrtProp_ActorShieldBasic,BasicShiel);
	__SetPropValue(enCrtProp_Shield,BasicShiel);

	//身法
	__SetPropValue(enCrtProp_ForeverAvoid, pActorData->Avoid);

	INT32 BasicAvoid = RecalculateAvoid(nTotalAptitude / 1000.0f, pActorData->Level , pActorData->Avoid , pLayerCnfoOld->m_AddAvoid);

	__SetPropValue(enCrtProp_ActorAvoidBasic, BasicAvoid);
	__SetPropValue(enCrtProp_Avoid, BasicAvoid);

	//方向
	__SetPropValue(enCrtProp_Dir, pActorData->Dir);

	//坐标
	__SetPropValue(enCrtProp_PointX, pActorData->ptX);
	__SetPropValue(enCrtProp_PointY, pActorData->ptY);


	//初始化m_TaskHistory
	m_UseFlag.set((unsigned char*)pActorData->m_UseFlag,SIZE_OF(pActorData->m_UseFlag));

	//VIP等级
	__SetPropValue(enCrtProp_VipLevel, pActorData->VipLevel);

	__SetPropValue(enCrtProp_TotalVipLevel, pActorData->VipLevel);

	//历史充值数量
	__SetPropValue(enCrtProp_Recharge, pActorData->Recharge);

	//thing共有属性
	strncpy(m_szName,pActorData->Name, sizeof(m_szName));

	strncpy(m_szNameOld,pActorData->Name,sizeof(m_szNameOld));

	SetUID(UID(pActorData->uid));

	if(UID(pActorData->uidMaster).IsValid())
	{
		m_pMaster = g_pGameServer->GetGameWorld()->FindActor(UID(pActorData->uidMaster));
		if( 0 == m_pMaster)
		{
			return false;
		}
	}

	return true;

}

bool Actor::SetEmployUID(const void * buf,  int nLen)
{
	if( 0 == buf || nLen < sizeof(SBuild_Actor))
	{
		return false;
	}

	SBuild_Actor * pBuild_Actor = (SBuild_Actor *)buf;

	for( int i = 0; i < pBuild_Actor->m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
	{
		UID UidEmploy =  UID(pBuild_Actor->m_EmployBuildData[i]->m_pActorBasicData->uid);
		m_uidEmploy[i] = UidEmploy;
	}

	//其它的把它设为-1表示无效值
	for( int k = pBuild_Actor->m_nEmployNum; k < MAX_EMPLOY_NUM; ++k)
	{
		m_uidEmploy[k] = UID();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool Actor::OnGetDBContext(void * buf, int &nLen)
{

	if(buf==0 || nLen<sizeof(SDB_Update_ActorBasicData_Req))
	{
		return false;
	}

	SDB_Update_ActorBasicData_Req * pReq = (SDB_Update_ActorBasicData_Req*)buf;

	//经验
	pReq->ActorExp			= __GetPropValue(enCrtProp_ActorExp);

	//境界
	pReq->ActorLayer		= __GetPropValue(enCrtProp_ActorLayer) ;

	//灵气
	pReq->ActorNimbus		= __GetPropValue(enCrtProp_ActorNimbus);

	//资质
	pReq->ActorAptitude		= __GetPropValue(enCrtProp_ForeverAptitude) ;

	//性别
	pReq->ActorSex			= __GetPropValue(enCrtProp_ActorSex);

	//UserID
	pReq->UserID			= __GetPropValue(enCrtProp_ActorUserID);

	//游戏币
	pReq->ActorMoney		= __GetPropValue(enCrtProp_ActorMoney);

	//礼券
	pReq->ActorTicket		= __GetPropValue(enCrtProp_ActorTicket);

	//灵石
	pReq->ActorStone		= __GetPropValue(enCrtProp_ActorStone);

	//外观
	pReq->ActorFacade		= __GetPropValue(enCrtProp_ActorFacade);

	//灵气速率
	pReq->ActorNimbusSpeed	= __GetPropValue(enCrtProp_ForeverActorNimbusSpeed);

	//气血上限
	pReq->ActorBloodUp		= __GetPropValue(enCrtProp_ForeverBloodUp);

	//城市编号
	pReq->CityID			= __GetPropValue(enCrtProp_ActorCityID);

	//荣誉
	pReq->Honor				= __GetPropValue(enCrtProp_ActorHonor);

	//声望
	pReq->Credit            = __GetPropValue(enCrtProp_ActorCredit);

	//仙剑灵气
	pReq->GodSwordNimbus	= __GetPropValue(enCrtProp_ActorGodSwordNimbus);


	//聚灵气
	pReq->ActorPolyNimbus	= __GetPropValue(enCrtProp_ActorPolyNimbus);

	//金剑诀伤害等级
	pReq->GoldDamageLv	= __GetPropValue(enCrtProp_GoldDamageLv);

	//木剑诀伤害等级
	pReq->WoodDamageLv	= __GetPropValue(enCrtProp_WoodDamageLv);

	//水剑诀伤害等级
	pReq->WaterDamageLv	= __GetPropValue(enCrtProp_WaterDamageLv);

	//火剑诀伤害等级
	pReq->FireDamageLv	= __GetPropValue(enCrtProp_FireDamageLv);

	//土剑诀伤害等级
	pReq->SoilDamageLv	= __GetPropValue(enCrtProp_SoilDamageLv);

	//金剑诀伤害
	pReq->GoldDamage	= __GetPropValue(enCrtProp_GoldDamage);

	//木剑诀伤害
	pReq->WoodDamage	= __GetPropValue(enCrtProp_WoodDamage);

	//水剑诀伤害
	pReq->WaterDamage	= __GetPropValue(enCrtProp_WaterDamage);

	//火剑诀伤害
	pReq->FireDamage	= __GetPropValue(enCrtProp_FireDamage);

	//土剑诀伤害
	pReq->SoilDamage	= __GetPropValue(enCrtProp_SoilDamage);

	//爆击
	pReq->Crit	= __GetPropValue(enCrtProp_ActorCrit);

	//坚韧
	pReq->Tenacity	= __GetPropValue(enCrtProp_ActorTenacity);

	//命中
	pReq->Hit	= __GetPropValue(enCrtProp_ActorHit);

	//回避
	pReq->Dodge	= __GetPropValue(enCrtProp_ActorDodge);

	//法术回复
	pReq->MagicCD	= __GetPropValue(enCrtProp_MagicCD);

	//爆击等级
	pReq->CritLv	= __GetPropValue(enCrtProp_ActorCritLv);

	//坚韧等级
	pReq->TenacityLv	= __GetPropValue(enCrtProp_ActorTenacityLv);

	//命中等级
	pReq->HitLv	= __GetPropValue(enCrtProp_ActorHitLv);

	//回避等级
	pReq->DodgeLv	= __GetPropValue(enCrtProp_ActorDodgeLv);

	//法术回复等级
	pReq->MagicCDLv	= __GetPropValue(enCrtProp_MagicCDLv);

	//灵魄
	pReq->GhostSoul	= __GetPropValue(enCrtProp_GhostSoul);

	if(m_pMaster)
	{
		pReq->uidMaster		= m_pMaster->GetUID().m_uid;
	}
	else
	{
		pReq->uidMaster		= UID().m_uid;
	}

	memcpy(&pReq->m_UseFlag, &m_UseFlag, sizeof(pReq->m_UseFlag));

	//vip等级
	pReq->VipLevel			= __GetPropValue(enCrtProp_VipLevel);

	//历史充值数量
	pReq->Recharge			= __GetPropValue(enCrtProp_Recharge);

	//帮战等级
	pReq->SynCombatLevel	= __GetPropValue(enCrtProp_SynCombatLevel);

	//夺宝等级
	pReq->DuoBaoLevel	= __GetPropValue(enCrtProp_DuoBaoLevel);

	pReq->CombatAbility = __GetPropValue(enCrtProp_ActorCombatAbility);

	//生物共有属性

	//等级
	pReq->Level				= __GetPropValue(enCrtProp_Level);

	//灵力
	pReq->Spirit			= __GetPropValue(enCrtProp_ForeverSpirit);

	//护盾
	pReq->Shield			= __GetPropValue(enCrtProp_ForeverShield);

	//身法
	pReq->Avoid				= __GetPropValue(enCrtProp_ForeverAvoid);

	//方向
	pReq->Dir				= __GetPropValue(enCrtProp_Dir);

	//坐标
	XPoint ptLoc			= GetLoc();
	pReq->ptX				= ptLoc.x;
	pReq->ptY				= ptLoc.y;



	//thing共有属性
	strncpy(pReq->Name,m_szName,sizeof(pReq->Name));

	if((this->GetMaster()==0) && (strcmp(m_szName,this->m_szNameOld)!=0))
	{
		TRACE("<error> %s : %d line 玩家[%s]名称有变动,新名[%s]!",__FUNCTION__,__LINE__,m_szNameOld,m_szName);
	}

	pReq->uid				= GetUID().ToUint64();



	nLen = sizeof(SDB_Update_ActorBasicData_Req);


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：发送消息给本实体
// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
// 返回值：
// 备  注：在此函数内，可能会发FireVote和FireAction
//////////////////////////////////////////////////////////////////////////
void Actor::OnMessage( UINT32 dwMsg, const void * buf, int nLen)
{
}

IResOutputPart * Actor::GetResOutputPart() 
{
	return (IResOutputPart *)GetPart(enThingPart_Actor_ResOutPut);
}


//伏魔洞
IFuMoDongPart * Actor::GetFuMoDongPart()
{
	return (IFuMoDongPart *)GetPart(enThingPart_Actor_FuMoDong);
}

//练功堂
ITrainingHallPart * Actor::GetTrainingHallPart()
{
	return (ITrainingHallPart *)GetPart(enThingPart_Actor_TrainingHall);
}

//背包
IPacketPart *  Actor::GetPacketPart()
{
	return (IPacketPart *)GetPart(enThingPart_Actor_Packet);
}

//装备栏
IEquipPart * Actor::GetEquipPart() 
{
	return (IEquipPart *)GetPart(enThingPart_Actor_Equip); 
}

//法术栏
IMagicPart * Actor::GetMagicPart() 
{
	return (IMagicPart *)GetPart(enThingPart_Actor_Magic); 
}

//战斗
ICombatPart * Actor::GetCombatPart()
{
	return (ICombatPart *)GetPart(enThingPart_Crt_Combat); 
}

//副本
IFuBenPart * Actor::GetFuBenPart()
{
	return (IFuBenPart *)GetPart(enThingPart_Actor_FuBen); 
}

//聚仙楼
IGatherGodHousePart * Actor::GetGatherGodHousePart()
{
	return (IGatherGodHousePart *)GetPart(enThingPart_Actor_GatherGodHouse);
}

//法宝
ITalismanPart * Actor::GetTalismanPart()
{
	return (ITalismanPart *)GetPart(enThingPart_Actor_Talisman);
}

//修炼
IXiuLianPart * Actor::GetXiuLianPart()
{
	return (IXiuLianPart *)GetPart(enThingPart_Actor_XiuLian);
}

//好友
IFriendPart * Actor::GetFriendPart() 
{
	return (IFriendPart *)GetPart(enThingPart_Actor_Friend);
}

//邮件
IMailPart * Actor::GetMailPart()
{
	return (IMailPart *)GetPart(enThingPart_Actor_Mail);
}

//帮派技能
ISynMagicPart * Actor::GetSynMagicPart()
{
	return (ISynMagicPart *)GetPart(enThingPart_Actor_SynMagic);
}

//剑冢
IGodSwordShopPart * Actor::GetGodSwordShopPart()
{
	return (IGodSwordShopPart *)GetPart(enThingPart_Actor_GodSwordShop);
}

//基本
IActorBasicPart * Actor::GetActorBasicPart() 
{
	return (IActorBasicPart *)GetPart(enThingPart_Crt_Basic);
}

//任务
ITaskPart * Actor::GetTaskPart() 
{
	return (ITaskPart *)GetPart(enThingPart_Actor_Task);
}



//斗法
IDouFaPart * Actor::GetDouFaPart()
{
	return (IDouFaPart *)GetPart(enThingPart_Actor_DouFa);
}

//成就
IChengJiuPart * Actor::GetChengJiuPart()
{
	return (IChengJiuPart *)GetPart(enThingPart_Actor_ChengJiu);
}

//活动
IActivityPart * Actor::GetActivityPart() 
{
	return (IActivityPart *)GetPart(enThingPart_Actor_Activity);
}

//冷却
ICDTimerPart * Actor::GetCDTimerPart()
{
	return (ICDTimerPart *)GetPart(enThingPart_Actor_CDTimer);
}

//组队
ITeamPart *	Actor::GetTeamPart()
{
	return (ITeamPart *)GetPart(enThingPart_Actor_Team);
}

//帮派
ISynPart * Actor::GetSynPart()
{
	return (ISynPart *)GetPart(enThingPart_Actor_Syn);
}


//取得本实体的enPartID部件
IThingPart* Actor::GetPart(enThingPart enPartID)
{
	//背包需要特殊处理，如果是雇佣人员，却返回主人的背包
	if(enPartID == enThingPart_Actor_Packet && GetMaster() != 0)
	{
		return  GetMaster()->GetPacketPart();
	}

	if(enPartID> enThingPart_Actor && enPartID < enThingPart_Crt_Max)
	{
		return m_pThingParts[enPartID - enThingPart_Actor];
	}
	return Super::GetPart(enPartID);
}


//向本实体添加部件pPart。如果部件已存在，则会添加失败
bool Actor::AddPart(IThingPart *pPart)
{
	if(pPart==NULL)
	{
		return false;
	}
	enThingPart enPartID = pPart->GetPartID();

	if(enPartID >= enThingPart_Crt_Max)
	{
		return false;		
	}

	if(enPartID>enThingPart_Actor)
	{
		m_pThingParts[enPartID-enThingPart_Actor] = pPart;
	}


	return Super::AddPart(pPart);
}

//删除本实体的enPartID部件，但并不释放这个部件
bool Actor::RemovePart(enThingPart enPartID)
{
	if(enPartID>enThingPart_Actor && enPartID< enThingPart_Crt_Max)
	{
		m_pThingParts[enPartID-enThingPart_Actor] = 0;	
	}

	return Super::RemovePart(enPartID);
}

//////////////////////////////////////////////////////////////////////////
// 描  述：改变本生物的数字型属性，将原来的值增加nValue
// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
//         属性同步标志enFlag
// 返回值：成功pNewValue返回改变后的新值，
//////////////////////////////////////////////////////////////////////////
bool Actor::AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue,IActor * pActor)
{
	if(enCrtProp_ActorMultipExp == enPropID && nValue>0)
	{
		nValue = nValue * __GetPropValue(enCrtProp_ActorExpRate) /1000.0f + 0.9999;
		enPropID = enCrtProp_ActorExp;
	}
	else if ( enCrtProp_ActorCredit == enPropID && nValue != 0)
	{
		nValue += this->GetVipValue(enVipType_AddGetCredit);
	}

	if(enPropID == enCrtProp_ActorExp && nValue>0)
	{
		nValue *= GetWallowFactor();
	}

	INT32 nNewValue = 0;
	if( SetCrtProp(enPropID,GetCrtProp(enPropID)+nValue,&nNewValue,pActor)==false)
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
// 描  述：设置本生物的属性，替换原来的值
// 输  入：数字型属性enPropID，属性值nValue
//         属性同步标志enFlag
// 返回值：成功pNewValue返回改变后的新值
//////////////////////////////////////////////////////////////////////////
bool Actor::SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue,IActor * pActor)
{

	//判断血上限
		if((enPropID == enCrtProp_Blood ) && (nValue > __GetPropValue(enCrtProp_ActorBloodUp)))
		{
			nValue = __GetPropValue(enCrtProp_ActorBloodUp);
		}


	INT32 nOldValue = GetCrtProp(enPropID);

	INT32 NewValue = 0;
	if(enPropID>enCrtProp_Actor && enPropID< enCrtProp_Actor_End)
	{		
		__SetPropValue(enPropID,nValue);

		NewValue = __GetPropValue(enPropID);	

		if(s_ActorPropMapSave[enPropID-enCrtProp_Actor])
		{
			this->SetSaveFlag(true);
		}
	}
	else if(Super::SetCrtProp(enPropID,nValue,&NewValue,this)==false)
	{
		return false;
	}


	if(pNewValue)
	{
		*pNewValue = NewValue;
	}

	IActorBasicPart * pActorBasicPart = GetActorBasicPart();

	if(pActorBasicPart)
	{
		pActorBasicPart->OnActorPropChange(this,enPropID,NewValue,nOldValue);
	}

	if(pActor && pActor!=this)
	{
		pActor->GetActorBasicPart()->OnActorPropChange(this,enPropID,NewValue,nOldValue);
	}

	return true;
}

//恢复气血
void Actor::RecoverBlood(IActor * pActor)
{
	//上限
	INT32 nBloodUp = GetCrtProp(enCrtProp_ActorBloodUp);

	if(nBloodUp != GetCrtProp(enCrtProp_Blood))
	{
		SetCrtProp(enCrtProp_Blood,nBloodUp,0,pActor);
	}
}

//取得本生物的属性
INT32 Actor::GetCrtProp(enCrtProp enPropID)
{
	return this->__GetPropValue(enPropID);
}

//设置网络通讯接口
void Actor::SetSession(ISession * pSession)
{
	m_pSession = pSession;
}

//和玩家客户端一一对应的网络通讯接口
ISession*  Actor::GetSession(void)
{
	return m_pSession;
}

//发送数据
bool  Actor::SendData(OStreamBuffer & osb)
{
	if(m_pMaster)
	{
		return m_pMaster->SendData(osb);
	}

	if(this->m_SourceActor != 0)
	{
		return m_SourceActor->SendData(osb);
	}

	if(0==m_pSession)
	{
		return false;
	}

	return m_pSession->SendData(osb);
}

INT32 Actor::__GetPropValue(enCrtProp PropID)
{
	if((enCrtProp_ActorExpRate == PropID /*|| enCrtProp_ActorNimbusSpeed == PropID*/ || enCrtProp_SynMagicPhysicsParam == PropID || 
		enCrtProp_SynMagicMagicParam == PropID || enCrtProp_SynMagicDefendParam == PropID || 
		enCrtProp_SynMagicAloneXLParam == PropID) && (this->GetMaster()!=0))
	{
		//所有主角共用的属性,得从主角身上获取
		return this->GetMaster()->GetCrtProp(PropID);
	}

	if(PropID >enCrtProp_Actor && PropID<enCrtProp_Actor_End)
	{
		return m_ActorProp[PropID - enCrtProp_Actor];
	}

	return Super::__GetPropValue(PropID);
}

bool Actor::__SetPropValue(enCrtProp PropID,int Value)
{
	if(PropID >enCrtProp_Actor && PropID<enCrtProp_Actor_End)
	{
		m_ActorProp[PropID - enCrtProp_Actor] = Value;
		if( m_ActorProp[PropID - enCrtProp_Actor] < 0){
			m_ActorProp[PropID - enCrtProp_Actor]=0;
		}
		return true;
	}
	return Super::__SetPropValue(PropID,Value);;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：重新计算生物的属性，然后更新客户端的属性数据
// 输  入：bSyncProp为FALSE表示不更新客户端的属性数据
// 备  注：1、生物创建时，计算生物属性
//         2、生物更换了装备，计算生物属性
//         3、仅广播生物已改变的公共属性给周围的生物
//         4、仅同步生物已改变的属性给客户端
//////////////////////////////////////////////////////////////////////////
void Actor::RecalculateProp(bool bSyncProp )
{
	//获取装备栏
	IEquipPart * pEquipPart = GetEquipPart();

	if(pEquipPart==0)
	{
		return;
	}

	//灵力
	INT32 Spirit = __GetPropValue(enCrtProp_Spirit) ;

	//护盾
	INT32 Shield = __GetPropValue(enCrtProp_Shield);


	//剑气1
	INT32 GodSwordOne = 0;
	INT32 MagicValueOne = 0;
	IEquipment * pEquipment = pEquipPart->GetEquipByPos(enEquipPos_GodSwordOne);
	if(pEquipment != 0)
	{
		IGodSword * pGodSword = (IGodSword *)pEquipment;
		GodSwordOne = pGodSword->GetSwordkee();
		MagicValueOne = pGodSword->GetMagicValue();
	}


	//剑气2
	INT32 GodSwordTwo = 0;
	INT32 MagicValueTwo = 0;
	pEquipment = pEquipPart->GetEquipByPos(enEquipPos_GodSwordTwo);
	if(pEquipment != 0)
	{
		IGodSword * pGodSword = (IGodSword *)pEquipment;
		GodSwordTwo = pGodSword->GetSwordkee();
		MagicValueTwo = pGodSword->GetMagicValue();
	}

	//剑气3
	INT32 GodSwordThree = 0;
	INT32 MagicValueThree = 0;
	pEquipment = pEquipPart->GetEquipByPos(enEquipPos_GodSwordThree);
	if(pEquipment != 0)
	{
		IGodSword * pGodSword = (IGodSword *)pEquipment;
		GodSwordThree = pGodSword->GetSwordkee();
		MagicValueThree = pGodSword->GetMagicValue();
	}

	//身法
	INT32 Avoid = this->GetCrtProp(enCrtProp_Avoid);

	//物理伤害=（灵力+身法+仙剑1剑气+仙剑2剑气+仙剑3剑气）*帮派技能
	float Physics = (Spirit + Avoid + GodSwordOne+GodSwordTwo + GodSwordThree);
	Physics += (Physics * this->GetCrtProp(enCrtProp_SynMagicPhysicsParam) / 1000.0f + 0.99999);

	//法术伤害=（灵力+仙剑1法术+仙剑2法术+仙剑3法术）*帮派技能
	float Magic =  (Spirit + MagicValueOne + MagicValueTwo + MagicValueThree);
	Magic	+= (Magic * this->GetCrtProp(enCrtProp_SynMagicMagicParam) / 1000.0f + 0.99999);

	//防御=(护盾*2+身法*2)*帮派技能
	float Defend = (Shield*2.0f+Avoid*2.0f);
	Defend	+= (Defend * this->GetCrtProp(enCrtProp_SynMagicDefendParam) / 1000.0f + 0.99999);

	//灵气上限
	const SActorLayerCnfg * pLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(this->GetCrtProp(enCrtProp_ActorLayer));
	if( 0 == pLayerCnfg){
		return;
	}

	UINT32 NimbusUp = pLayerCnfg->m_NimbusUp;

	if(bSyncProp)
	{		

		//物理伤害
		if(GetCrtProp(enCrtProp_ActorPhysics)!=Physics)
		{		
			SetCrtProp(enCrtProp_ActorPhysics,Physics);
		}

		//法术伤害
		if(GetCrtProp(enCrtProp_ActorMagic) != Magic)
		{
			SetCrtProp(enCrtProp_ActorMagic,Magic);
		}

		//防御
		if(GetCrtProp(enCrtProp_ActorDefend) != Defend)
		{
			SetCrtProp(enCrtProp_ActorDefend,Defend);
		}

		//灵气上限
		if(GetCrtProp(enCrtProp_ActorNimbusUp) != NimbusUp)
		{
			SetCrtProp(enCrtProp_ActorNimbusUp,NimbusUp);
		}

	}
	else
	{
		//物理伤害
		__SetPropValue(enCrtProp_ActorPhysics,Physics);

		//法术伤害
		__SetPropValue(enCrtProp_ActorMagic,Magic);

		//防御
		__SetPropValue(enCrtProp_ActorDefend,Defend);

		//灵气上限
		__SetPropValue(enCrtProp_ActorNimbusUp,NimbusUp);
	}
	//战斗力
	this->RecalculateCombatAbility();

}

//重新计算战斗力
void Actor::RecalculateCombatAbility()
{

	//物理伤害
	INT32 Physics = __GetPropValue(enCrtProp_ActorPhysics) ;

	//法术伤害
	INT32 Magic = __GetPropValue(enCrtProp_ActorMagic) ;

	//防御
	INT32 Defend = __GetPropValue(enCrtProp_ActorDefend) ;

	//爆击
	INT32 Crit = __GetPropValue(enCrtProp_ActorCrit) ;

	//坚韧
	INT32 Tenacity = __GetPropValue(enCrtProp_ActorTenacity) ;

	//命中
	INT32 Hit = __GetPropValue(enCrtProp_ActorHit) ;

	//回避
	INT32 Dodge = __GetPropValue(enCrtProp_ActorDodge) ;


	//战斗力 = 物理伤害+法术伤害+防御+暴击值+坚韧值+命中值+回避值
	INT32 CombatAbility = Physics + Magic + Defend + Crit + Tenacity + Hit + Dodge;

	if(GetCrtProp(enCrtProp_ActorCombatAbility) != CombatAbility)
	{
		SetCrtProp(enCrtProp_ActorCombatAbility,CombatAbility);
	}

}

//获得主人，如果返回NULL，表示体身即为主角
IActor * Actor::GetMaster()
{
	return m_pMaster;
}

//获得雇用
IActor * Actor::GetEmployee(int nIndex)
{
	if(nIndex<0 || nIndex>=sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0]))
	{
		return 0;
	}
	return g_pGameServer->GetGameWorld()->FindActor(m_uidEmploy[nIndex]);
}

IActor * Actor::GetEmployee(UID uidActor)
{
	for(int nIndex=0 ; nIndex<sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0]); ++nIndex)
	{
		if(m_uidEmploy[nIndex]==uidActor)
		{
			return g_pGameServer->GetGameWorld()->FindActor(m_uidEmploy[nIndex]);
		}	
	}

	return 0;
}

//获得招募角色个数
UINT8  Actor::GetEmployeeNum()
{
	int Num = 0;
	for(int i = 0; i < MAX_EMPLOY_NUM; i++)
	{
		if(m_uidEmploy[i] != UID())
			Num++;
	}

	return Num;
}

bool  Actor::SetEmployee(int nIndex, IActor* pEmployee)
{
	int nSize = sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0]);

	if(nIndex< -1 || nIndex>=(int)(sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0])))
	{
		return false;
	}
	if(nIndex >= nSize){
		return false;
	}
	if(nIndex>=(int)(sizeof(m_uidEmploy)/sizeof(m_uidEmploy[0])))
	{
		return false;
	}

	if(-1 != nIndex){
		m_uidEmploy[nIndex] = pEmployee->GetUID();
		return true;
	}else{
		//自动找个位置插入
		for(int i = 0; i < MAX_EMPLOY_NUM; ++i){
			if( !m_uidEmploy[i].IsValid()){
				m_uidEmploy[i] = pEmployee->GetUID();
				return true;
			}
		}
	}
	return false;
}

//保存玩家的各种数据,速度较慢
void	Actor::SaveData()
{
	this->SaveActorBasicData();


	//招募角色数据保存
	for( int i = 0; i < MAX_EMPLOY_NUM; ++i){
		if(! m_uidEmploy[i].IsValid()){
			continue;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_uidEmploy[i]);
		if( 0 != pActor){
			pActor->SaveData();
		}
	}

	m_LastSaveToDBTime = ::time(0);
}

//立即保存玩家基本数据（不含物品，宠物），
bool	Actor::SaveActorBasicData()
{
	if (!this->SaveActorProp()){

		return false;
	}

	//保存各PART数据
	IResOutputPart * pResOutputPart =  this->GetResOutputPart();
	if( 0 != pResOutputPart){
		pResOutputPart->SaveData();
	}

	IFuMoDongPart * pFuMoDongPart = this->GetFuMoDongPart();
	if( 0 != pFuMoDongPart){
		pFuMoDongPart->SaveData();
	}

	ITrainingHallPart * pTrainingHallPart = this->GetTrainingHallPart();
	if( 0 != pTrainingHallPart){
		pTrainingHallPart->SaveData();
	}



	IGatherGodHousePart * pGatherGodHousePart = this->GetGatherGodHousePart();
	if( 0 != pGatherGodHousePart){
		pGatherGodHousePart->SaveData();
	}

	IXiuLianPart * pXiuLianPart = this->GetXiuLianPart();
	if( 0 != pXiuLianPart){
		pXiuLianPart->SaveData();
	}


	IFuBenPart * pFuBenPart = this->GetFuBenPart();
	if( 0 != pFuBenPart){
		pFuBenPart->SaveData();
	}

	IGodSwordShopPart * pGodSwordShopPart = this->GetGodSwordShopPart();
	if( 0 != pGodSwordShopPart){
		pGodSwordShopPart->SaveData();
	}


	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 != pPacketPart){
		pPacketPart->SaveData();
	}

	IEquipPart * pEquipPart = this->GetEquipPart();
	if( 0 != pEquipPart){
		pEquipPart->SaveData();
	}

	IMagicPart * pMagicPart = this->GetMagicPart();
	if( 0 != pMagicPart){
		pMagicPart->SaveData();
	}

	ISynMagicPart * pSynMagicPart = this->GetSynMagicPart();
	if( 0 != pSynMagicPart){
		pSynMagicPart->SaveData();
	}

	ICombatPart * pCombatPart = this->GetCombatPart();
	if( 0 != pCombatPart){
		pCombatPart->SaveData();
	}

	ITaskPart * pTaskPart = this->GetTaskPart();
	if( 0 != pTaskPart){
		pTaskPart->SaveData();
	}

	IChengJiuPart * pChengJiuPart = this->GetChengJiuPart();
	if( 0 != pChengJiuPart){
		pChengJiuPart->SaveData();
	}


	IActivityPart * pActivityPart = this->GetActivityPart();
	if( 0 != pActivityPart){
		pActivityPart->SaveData();
	}

	ICDTimerPart * pCDTimerPart = this->GetCDTimerPart();
	if( 0 != pCDTimerPart){
		pCDTimerPart->SaveData();
	}

	IStatusPart * pStatusPart = (IStatusPart *)this->GetPart(enThingPart_Crt_Status);
	if( 0 != pStatusPart){
		pStatusPart->SaveData();
	}

	IDouFaPart * pDouFaPart = this->GetDouFaPart();
	if( 0 != pDouFaPart){
		pDouFaPart->SaveData();
	}

	IFriendPart * pFriendPart = this->GetFriendPart();
	if( 0 != pFriendPart){
		pFriendPart->SaveData();
	}

	ISynPart * pSynPart = this->GetSynPart();

	if ( 0 != pSynPart ){
		pSynPart->SaveData();
	}

	ITalismanPart * pTalismanPart = this->GetTalismanPart();

	if ( 0 != pTalismanPart ){
		pTalismanPart->SaveData();
	}

	return true;
}

//保存物品数据，如果只需要立即保存物品数据，调这个接口，快很多
bool		Actor::SaveGoodsData()
{
	return true;
}

//按属性实时保存，速度最快,注意目前只支持有限的几个属性，包括（经验（经验30级以下内部实时存盘,外部不用处理）,金币，元宝），防止当机丢数据，实时存盘
bool	Actor::SaveDataByProperty(enCrtProp propID)
{
	return true;
}

//通知客户端通过私有数据创建thing
bool	Actor::NoticClientCreatePrivateThing(UID uidThing)
{
	IPacketPart * pPacketPart = this->GetPacketPart();

	if( 0 == pPacketPart){
		return false;
	}

	if(pPacketPart->IsAlreadySendThing(uidThing))
	{
		return true;
	}

	IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidThing);
	if(pThing == 0)
	{
		return false;
	}



	//通知客户端创建
	char Data[1024] =  {0};
	int size = sizeof(Data);
	if(pThing->OnGetPrivateContext(Data,size)==false)
	{
		return false;
	}

	OBuffer1k ob1;

	enThing_Class ThingClass = pThing->GetThingClass();
	if(ThingClass == enThing_Class_Goods)
	{
		IGoods * pGoods = (IGoods*)pThing;
		SendGoodsCnfg(pGoods->GetGoodsID());

		if( pGoods->GetGoodsClass() == enGoodsCategory_Equip){
			//如果装备镶嵌宝石的话,要把宝石的配置信息下发
			IEquipment * pEquipment = (IEquipment *)pGoods;

			int GemGoodsID = 0;
			pEquipment->GetPropNum(enGoodsProp_Gem_ID1, GemGoodsID);
			if( GemGoodsID != INVALID_GOODS_ID){
				this->SendGoodsCnfg(GemGoodsID);
			}

			pEquipment->GetPropNum(enGoodsProp_Gem_ID2, GemGoodsID);
			if( GemGoodsID != INVALID_GOODS_ID){
				this->SendGoodsCnfg(GemGoodsID);
			}

			pEquipment->GetPropNum(enGoodsProp_Gem_ID3, GemGoodsID);
			if( GemGoodsID != INVALID_GOODS_ID){
				this->SendGoodsCnfg(GemGoodsID);
			}
		}

		else if( pGoods->GetGoodsClass() == enGoodsCategory_Talisman){
			//如果法宝附有灵件的话,要把灵件的配置信息下发
			ITalisman * pTalisman = (ITalisman *)pGoods;

			int GhostGoodsID = 0;
			pTalisman->GetPropNum(enGoodsProp_GhostGoodsID, GhostGoodsID);
			if( GhostGoodsID != INVALID_GOODS_ID){
				this->SendGoodsCnfg(GhostGoodsID);
			}


		}
	}
	else if(ThingClass == enThing_Class_Monster)
	{
		//发送怪物法术配置信息
		SendMonsterMagicCnfg((IMonster*)pThing);

	}

	SC_CreateThing CreateThing;
	CreateThing.enThingClass = pThing->GetThingClass();
	ob1 << GameWorldHeader(enGameWorldCmd_SC_Private_Context,sizeof(SC_CreateThing)+size) << CreateThing ;
	ob1.Push(Data,size);

	if(SendData(ob1.TakeOsb())==false)
	{
		return false;
	}

	pPacketPart->InsertSendThing(pThing);

	return true;
}

//发送物品配置信息
void Actor::SendGoodsCnfg(TGoodsID GoodsID)
{
	//是否已经发送过给客户端
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	if( pPacketPart->IsAlreadySend(GoodsID)){
		return;
	}

	const  SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);
	if(pGoodsCnfg==0)
	{
		return;
	}
	UINT8 Category = pGoodsCnfg->m_GoodsClass;

	OBuffer1k ob1;

	if(Category == enGoodsCategory_Common )
	{
		SC_GoodsCnfg GoodsCnfg;
		GoodsCnfg.m_BindType	= pGoodsCnfg->m_BindType;
		GoodsCnfg.m_GoodsClass	= pGoodsCnfg->m_GoodsClass;
		GoodsCnfg.m_GoodsID		= pGoodsCnfg->m_GoodsID;
		GoodsCnfg.m_GoodsLevel	= pGoodsCnfg->m_GoodsLevel;
		GoodsCnfg.m_PileNum		= pGoodsCnfg->m_PileNum;
		GoodsCnfg.m_Quality		= pGoodsCnfg->m_Quality;
		GoodsCnfg.m_SubClass	= pGoodsCnfg->m_SubClass;
		GoodsCnfg.m_ThirdClass	= pGoodsCnfg->m_ThirdClass;
		GoodsCnfg.m_UsedLevel	= pGoodsCnfg->m_UsedLevel;


		// fly add	20121106
		strncpy(GoodsCnfg.m_szName,(char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID),MEM_SIZE(GoodsCnfg.m_szName));
		//strncpy(GoodsCnfg.m_szName,(char*)pGoodsCnfg->m_szName,MEM_SIZE(GoodsCnfg.m_szName));
		GoodsCnfg.m_SellPrice	= pGoodsCnfg->m_SellPrice;
		GoodsCnfg.m_ResID		= pGoodsCnfg->m_ResID;
		GoodsCnfg.m_GemPropID   = (enEquipProp)pGoodsCnfg->m_BloodOrSwordkee;  //宝石属性ID
		GoodsCnfg.m_GemValue    = pGoodsCnfg->m_AvoidOrSwordLvMax;  //宝石属性值

		GoodsCnfg.m_bManyUse	= pGoodsCnfg->m_SpiritOrMagic;		//是否多个使用
		GoodsCnfg.m_bSelectUser = pGoodsCnfg->m_ShieldOrWuXing;;	//是否选择使用角色

		GoodsCnfg.m_bIsGod		= pGoodsCnfg->m_bIsGod;


		// fly add	20121106
		INT32 len = strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsDescLangID)) + 1;

		ob1 << GameWorldHeader(enGameWorldCmd_SC_GoodsCnfg,SIZE_OF(GoodsCnfg)+len) << GoodsCnfg;

		if(len>0)
		{
			ob1.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsDescLangID),len);

		}
	}
	else
	{
		SC_EquipCnfg GoodsCnfg;

		GoodsCnfg.m_BindType	= pGoodsCnfg->m_BindType;
		GoodsCnfg.m_GoodsClass	= pGoodsCnfg->m_GoodsClass;
		GoodsCnfg.m_GoodsID		= pGoodsCnfg->m_GoodsID;
		GoodsCnfg.m_GoodsLevel	= pGoodsCnfg->m_GoodsLevel;
		GoodsCnfg.m_PileNum		= pGoodsCnfg->m_PileNum;
		GoodsCnfg.m_Quality		= pGoodsCnfg->m_Quality;
		GoodsCnfg.m_SubClass	= pGoodsCnfg->m_SubClass;
		GoodsCnfg.m_ThirdClass	= pGoodsCnfg->m_ThirdClass;
		GoodsCnfg.m_UsedLevel	= pGoodsCnfg->m_UsedLevel;


		// fly add	20121106
		strncpy(GoodsCnfg.m_szName,(char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID),MEM_SIZE(GoodsCnfg.m_szName));
		//strncpy(GoodsCnfg.m_szName,(char*)pGoodsCnfg->m_szName,MEM_SIZE(GoodsCnfg.m_szName));
		GoodsCnfg.m_SellPrice	= pGoodsCnfg->m_SellPrice;
		GoodsCnfg.m_bSelectUser = 0;
		GoodsCnfg.m_ResID		= pGoodsCnfg->m_ResID;

		GoodsCnfg.m_AvoidOrSwordLvMax = pGoodsCnfg->m_AvoidOrSwordLvMax;
		GoodsCnfg.m_BloodOrSwordkee = pGoodsCnfg->m_BloodOrSwordkee;
		GoodsCnfg.m_ShieldOrWuXing = pGoodsCnfg->m_ShieldOrWuXing;
		GoodsCnfg.m_SpiritOrMagic = pGoodsCnfg->m_SpiritOrMagic;
		GoodsCnfg.m_SuitIDOrSwordSecretID = pGoodsCnfg->m_SuitIDOrSwordSecretID;

		GoodsCnfg.m_bIsGod		= pGoodsCnfg->m_bIsGod;

		if( GoodsCnfg.m_GoodsClass == enGoodsCategory_Talisman){
			//如果是法宝的话，把法宝带的法术信息也发送
			if( GoodsCnfg.m_SpiritOrMagic != INVALID_MAGIC_ID)
			{
				this->SendMagicLevelCnfg(GoodsCnfg.m_SpiritOrMagic);
			}
		}
		else if( GoodsCnfg.m_GoodsClass == enGoodsCategory_GodSword && GoodsCnfg.m_SuitIDOrSwordSecretID != 0)
		{
			//如果是仙剑的话，把仙剑带的法术信息也发送
			const SGodSwordWorldCnfg * pSwordWorldCnfg = g_pGameServer->GetConfigServer()->GetGoldSwordWorldCnfg(GoodsCnfg.m_SuitIDOrSwordSecretID);
			if( !pSwordWorldCnfg)
			{
				TRACE("<error> %s : %d 行 获取剑印世界配置信息出错！！剑决ID=%d", __FUNCTION__,__LINE__,GoodsCnfg.m_SuitIDOrSwordSecretID);
				return;
			}

			GoodsCnfg.m_SuitIDOrSwordSecretID = pSwordWorldCnfg->m_MagicID;		//对于仙剑，发送法术ID	

			this->SendMagicLevelCnfg(pSwordWorldCnfg->m_MagicID);
		}
		else if(GoodsCnfg.m_GoodsClass == enGoodsCategory_Equip && GoodsCnfg.m_SuitIDOrSwordSecretID != 0)
		{
			//套装信息			
			const SSuitCnfg* pSuitCnfg = g_pGameServer->GetConfigServer()->GetSuitCnfg(GoodsCnfg.m_SuitIDOrSwordSecretID);
			if(pSuitCnfg==0)
			{
				TRACE("<error> %s : %d 行 获取套装配置信息出错！！GoodsID = %d 套装ID=%d", __FUNCTION__,__LINE__,GoodsCnfg.m_GoodsID,GoodsCnfg.m_SuitIDOrSwordSecretID);
				GoodsCnfg.m_SuitIDOrSwordSecretID = 0;
			}
			else
			{
				GoodsCnfg.m_SuitPropID1 = (enEquipProp)pSuitCnfg->m_SuitPropID1;
				GoodsCnfg.m_SuitPropValue1 = pSuitCnfg->m_SuitPropValue1;
				GoodsCnfg.m_SuitPropID2 = (enEquipProp)pSuitCnfg->m_SuitPropID2;
				GoodsCnfg.m_SuitPropValue2 = pSuitCnfg->m_SuitPropValue2;
			}

		}

		//fly add	20121106

		INT32 len = strlen(g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsDescLangID)) + 1;
		//INT32 len = pGoodsCnfg->m_strDesc.length()+1;

		ob1 << GameWorldHeader(enGameWorldCmd_SC_EquipCnfg,SIZE_OF(GoodsCnfg)+len) << GoodsCnfg;

		if(len>0)
		{
			ob1.Push(g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsDescLangID),len);
			//ob1.Push(pGoodsCnfg->m_strDesc.c_str(),len);
		}
	}

	pPacketPart->InsertSendGoodsCnfg(GoodsID);

	SendData(ob1.TakeOsb());	
}

//发送招募角色配置信息
void Actor::SendEmployeeCnfg(TActorID EmployeeID)
{
	//是否已经发送过给客户端
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	if( pPacketPart->IsAlreadySendEmployeeCnfg(EmployeeID)){
		return;
	}

	const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(EmployeeID);
	if( 0 == pEmployeeDataCnfg){
		return;
	}

	SC_EmployeeCnfg EmployeeCnfg;

	EmployeeCnfg.m_EmployeeID = pEmployeeDataCnfg->m_EmployeeID;
	EmployeeCnfg.m_Level	= pEmployeeDataCnfg->m_Level;
	EmployeeCnfg.m_Aptitude = pEmployeeDataCnfg->m_Aptitude;
	EmployeeCnfg.m_ResID    = pEmployeeDataCnfg->m_ResID;
	EmployeeCnfg.m_Price	= pEmployeeDataCnfg->m_Price;
	EmployeeCnfg.m_NenLiType = (enNenLiType)pEmployeeDataCnfg->m_NenLi;

	// fly add	20121106
	strncpy(EmployeeCnfg.m_szName, (char*) g_pGameServer->GetGameWorld()->GetLanguageStr(pEmployeeDataCnfg->m_NameLanguageID), sizeof(EmployeeCnfg.m_szName));


	//下面4个属性发给客户端的是算好的

	//当级气血值=资质×当前等级×50+初始气血
	EmployeeCnfg.m_BloodUp	= (pEmployeeDataCnfg->m_Aptitude / 1000.0f) * pEmployeeDataCnfg->m_Level * 48 + 0.99999 + pEmployeeDataCnfg->m_BloodUp;
	//当级灵力值=资质×当前等级×10+初始灵力
	EmployeeCnfg.m_Spirit	= (pEmployeeDataCnfg->m_Aptitude / 1000.0f) * pEmployeeDataCnfg->m_Level * 10 + 0.99999 + pEmployeeDataCnfg->m_Spirit;
	//当级护盾值=资质×当前等级×8+初始护盾
	EmployeeCnfg.m_Shield	= (pEmployeeDataCnfg->m_Aptitude / 1000.0f) * pEmployeeDataCnfg->m_Level * 8 + 0.99999 + pEmployeeDataCnfg->m_Shield;
	//当级身法值=资质×当前等级×5+初始身法
	EmployeeCnfg.m_Avoid	= (pEmployeeDataCnfg->m_Aptitude / 1000.0f) * pEmployeeDataCnfg->m_Level * 5 + 0.99999 + pEmployeeDataCnfg->m_Avoid;

	pPacketPart->InsertSendEmployeeCnfg(EmployeeID);

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_EmployeeCnfg, SIZE_OF(EmployeeCnfg)) << EmployeeCnfg;
	this->SendData(ob.TakeOsb());
}

//发送怪物法术配置信息
void  Actor::SendMonsterMagicCnfg(IMonster * pMonster)
{
	if(pMonster==0)
	{
		return;
	}

	for(int i=0; i<MAX_MONSTER_MAGIC_NUM; i++)
	{
		IMagic * pMagic =(IMagic*) pMonster->GetCrtProp((enCrtProp)(enCrtProp_MonsterMagic1+i));
		if(pMagic==0)
		{
			break;
		}

		SendMagicLevelCnfg(pMagic->GetMagicID());

	}
}

//发送法术等级配置
void Actor::SendMagicLevelCnfg(TMagicID MagicID)
{
	//是否已经发送过给客户端
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	if( pPacketPart->IsAlreadySendMagicLevelCnfg(MagicID)){
		return;
	}

	const std::vector<SMagicLevelCnfg *>  vectLevelCnfg = g_pGameServer->GetConfigServer()->GetMagicAllLevelCnfg(MagicID);

	if( 0 == vectLevelCnfg.size()){
		return;
	}

	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(MagicID);
	if( 0 == pMagicCnfg){
		return;
	}

	SC_MagicLevelCnfg MagicLevelCnfg;
	MagicLevelCnfg.m_MagicID  = MagicID;
	MagicLevelCnfg.m_MaxLevel = vectLevelCnfg.size();
	MagicLevelCnfg.m_WuXing	  = pMagicCnfg->m_WuXing;
	MagicLevelCnfg.m_IconID   = pMagicCnfg->m_IconID;
	MagicLevelCnfg.m_DongHuaID =  pMagicCnfg->m_DongHuaID;
	MagicLevelCnfg.m_CDTime = pMagicCnfg->m_CDTime;

	// fly add	20121106

	strncpy(MagicLevelCnfg.m_szName, g_pGameServer->GetGameWorld()->GetLanguageStr(pMagicCnfg->m_MagicLangID), sizeof(MagicLevelCnfg.m_szName));
	//strncpy(MagicLevelCnfg.m_szName, pMagicCnfg->m_strName.c_str(), sizeof(MagicLevelCnfg.m_szName));

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_MagicLevelCnfg, SIZE_OF(MagicLevelCnfg) + MagicLevelCnfg.m_MaxLevel * sizeof(MagicLevelInfo)) << MagicLevelCnfg;

	for( int i = 0; i < vectLevelCnfg.size(); ++i)
	{
		MagicLevelInfo LevelInfo;
		LevelInfo.m_Level		= vectLevelCnfg[i]->m_Level;
		LevelInfo.m_NeedLayer   = vectLevelCnfg[i]->m_NeedLayer;
		LevelInfo.m_NeedNimbus  = vectLevelCnfg[i]->m_NeedNimbus;

		// fly add	20121106
		strncpy(LevelInfo.m_Descript, g_pGameServer->GetGameWorld()->GetLanguageStr(vectLevelCnfg[i]->m_MagicLevelDescLangID), sizeof(LevelInfo.m_Descript));
		//strncpy(LevelInfo.m_Descript, vectLevelCnfg[i]->m_Descript.c_str(), sizeof(LevelInfo.m_Descript));

		ob << LevelInfo;
	}

	pPacketPart->InsertSendMagicLevelCnfg(MagicID);

	this->SendData(ob.TakeOsb());
}

//发送帮派技能配置
void Actor::SendSynMagicCnfg(TSynMagicID SynMagicID)
{
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	if( pPacketPart->IsAlreadySendSynMagic(SynMagicID)){
		return;
	}

	const std::vector<SSynMagicCnfg> * pvectSynMagic = g_pGameServer->GetConfigServer()->GetSynMagicCnfg(SynMagicID);
	if( 0 == pvectSynMagic){
		TRACE("<error> %s : %d 行 找不到帮派技能配置信息!!,技能ID = %d", __FUNCTION__, __LINE__, SynMagicID);
		return;
	}

	if( pvectSynMagic->size() <= 0){
		return;
	}

	SC_SynMagicCnfg SynCnfg;

	SynCnfg.m_SynMagicID = SynMagicID;
	SynCnfg.m_MaxLevel   = pvectSynMagic->size();
	SynCnfg.m_ResID		 = (*pvectSynMagic)[0].m_ResID;

	// fly add	20121106
	strncpy(SynCnfg.m_szName, g_pGameServer->GetGameWorld()->GetLanguageStr((*pvectSynMagic)[0].m_SynMagicNameLangID), sizeof(SynCnfg.m_szName));
	//strncpy(SynCnfg.m_szName, (*pvectSynMagic)[0].m_szSynMagicName, sizeof(SynCnfg.m_szName));

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_SynMagicCnfg, SIZE_OF(SynCnfg) + SynCnfg.m_MaxLevel * sizeof(SynMagicLevelInfo)) << SynCnfg;

	for( int i = 0; i < pvectSynMagic->size(); ++i)
	{
		SynMagicLevelInfo SynMagicLevel;

		SynMagicLevel.m_Level = (*pvectSynMagic)[i].m_SynMagicLevel;
		SynMagicLevel.m_NeedSynContribution = (*pvectSynMagic)[i].m_NeedContribution;
		SynMagicLevel.m_NeedStone = (*pvectSynMagic)[i].m_NeedStone;
		SynMagicLevel.m_NeedSynLevel = (*pvectSynMagic)[i].m_NeedSynLevel;

		// fly add	20121106
		strncpy(SynMagicLevel.m_Descript, g_pGameServer->GetGameWorld()->GetLanguageStr((*pvectSynMagic)[i].m_SynMagicDescLangID), sizeof(SynMagicLevel.m_Descript));
		//strncpy(SynMagicLevel.m_Descript, (*pvectSynMagic)[i].m_strSynMagicDes.c_str(), sizeof(SynMagicLevel.m_Descript));

		ob.Push(&SynMagicLevel, sizeof(SynMagicLevel));
	}

	this->SendData(ob.TakeOsb());

	pPacketPart->InsertSendSynMagicCnfg(SynMagicID);
}

//通知客户端通过公用数据创建thing
bool Actor::NoticClientCreatePublicThing(UID uidThing)
{
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return false;
	}

	if(pPacketPart->IsAlreadySendThing(uidThing))
	{
		return true;
	}

	IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidThing);
	if(pThing == 0)
	{
		return false;
	}
	//通知客户端创建
	char Data[1024] = {0};
	int size = sizeof(Data);
	if(pThing->OnGetPublicContext(Data,size)==false)
	{
		return false;
	}

	OBuffer1k ob1;

	enThing_Class ThingClass = pThing->GetThingClass();
	if(ThingClass == enThing_Class_Goods)
	{
		IGoods * pGoods = (IGoods*)pThing;
		SendGoodsCnfg(pGoods->GetGoodsID());		
	}


	SC_CreateThing CreateThing;
	CreateThing.enThingClass = pThing->GetThingClass();
	ob1 << GameWorldHeader(enGameWorldCmd_SC_Public_Context,sizeof(SC_CreateThing)+size) << CreateThing ;
	ob1.Push(Data,size);

	if( SendData(ob1.TakeOsb())==false)
	{
		return false;
	}

	pPacketPart->InsertSendThing(pThing);

	return true;		
}


//通知销毁物品
bool Actor::NoticClientDestroyThing(UID uidThing)
{
	IPacketPart * pPacketPart = this->GetPacketPart();
	if( 0 == pPacketPart){
		return false;
	}

	OBuffer1k ob;
	SC_DestroyThing DestroyThing;
	DestroyThing.m_uidThing = uidThing;

	ob << GameWorldHeader(enGameWorldCmd_SC_DestroyThing,sizeof(DestroyThing)) << DestroyThing ;

	if(SendData(ob.TakeOsb())==false)
	{
		return false;
	}

	pPacketPart->ClearSendThing(uidThing);

	return true;	
}

//通知Thing属性改变
bool Actor::NoticClientUpdateThing(UID uidThing,UINT8 PropID,INT32 Value)
{
	OBuffer1k ob;
	SC_UpdateThing UpdateThing;
	UpdateThing.m_uidThing = uidThing;
	UpdateThing.m_PropID = PropID;
	UpdateThing.m_PropValue = Value;

	ob << GameWorldHeader(enGameWorldCmd_SC_UpdateThing,sizeof(SC_UpdateThing)) << UpdateThing ;

	return SendData(ob.TakeOsb());	
}

//通知Thing属性改变(带小数的)
bool Actor::NoticClientUpdateThingFloat(UID uidThing,UINT8 PropID, float Value)
{
	OBuffer1k ob;
	SC_UpdateThingFloat UpdateThing;
	UpdateThing.m_uidThing = uidThing;
	UpdateThing.m_PropID = PropID;
	UpdateThing.m_PropValue = Value;

	ob << GameWorldHeader(enGameWorldCmd_SC_UpdateThing,sizeof(SC_UpdateThing)) << UpdateThing ;	

	return SendData(ob.TakeOsb());
}

//回到主场景
void Actor::ComeBackMainScene()
{
	TSceneID SceneID(this->GetCrtProp(enCrtProp_ActorMainSceneID));

	IGameScene * pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);

	if(pGameScene != 0)
	{
		pGameScene->EnterScene(this);

		SC_EnterPortal_Rsp Rsp;

		Rsp.m_Result = enGameWorldRetCode_Ok;


		OBuffer1k ob;
		ob << GameWorldHeader(enGameWorldCmd_EnterPortal,sizeof(Rsp)) << Rsp;
		this->SendData(ob.TakeOsb());
	}
}

//添加关注的玩家
bool Actor::AddToAttentionUser(UID uid_DestUser)
{
	if( m_setAttentionUser.size() == 0){
		if( m_setAttentionUser.insert(uid_DestUser).second == true){
			//增加对玩家卸载的否决
			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

			IEventServer * pEnventServer = g_pGameServer->GetEventServer();
			if( 0 == pEnventServer){
				return false;
			}
			pEnventServer->AddListener(this, msgID, enEventSrcType_Actor, 0, "Actor::AddToAttentionUser"); 

			return true;
		}
	}

	return m_setAttentionUser.insert(uid_DestUser).second;
}

//移除关注
void Actor::RemoveToAttentionUser(UID uid_DestUser)
{
	m_setAttentionUser.erase(uid_DestUser);

	if( m_setAttentionUser.size() == 0){
		//取消对玩家卸载的否决
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

		IEventServer * pEnventServer = g_pGameServer->GetEventServer();
		if( 0 == pEnventServer){
			return;
		}
		pEnventServer->RemoveListener(this, msgID, enEventSrcType_Actor, 0);
	}
}

bool Actor::OnVote(XEventData & EventData)
{
	SS_UnloadActorContext * pUnloadActorContext = (SS_UnloadActorContext *)EventData.m_pContext;
	if( 0 == pUnloadActorContext){
		return true;
	}

	std::set<UID>::iterator iter = m_setAttentionUser.find(pUnloadActorContext->m_uidActor);
	if( iter == m_setAttentionUser.end()){
		return true;
	}

	return false;
}

//设置名字
void Actor::SetName(const char * szNewName)
{
	strncpy(this->m_szName, szNewName, THING_NAME_LEN - 1);

	//通知客户端
	SC_UpdateName UpdateName;
	strncpy(UpdateName.m_szName, szNewName, THING_NAME_LEN - 1);
	UpdateName.m_UserUid = this->GetUID();

	OBuffer1k ob;
	ob << GameWorldHeader(enGameWorldCmd_SC_UpdateName, SIZE_OF(UpdateName)) << UpdateName;
	this->SendData(ob.TakeOsb());

	this->SetSaveFlag(true);
}

//解雇招募角色
void Actor::UnLoadEmployee(UID EmployeeUid, enUnLoadEmployeeRetCode & RetCode)
{
	if( EmployeeUid == this->GetUID()){
		RetCode = enUnLoadEmployeeRetCode_ErrUser;
		return;
	}

	IActor * pEmployee = this->GetEmployee(EmployeeUid);
	if( 0 == pEmployee){
		RetCode = enUnLoadEmployeeRetCode_NoEmployee;
		return;
	}

	IEquipPart * pEquipPart = pEmployee->GetEquipPart();
	if( 0 == pEquipPart){
		return;
	}

	//先发布事件,让其它模块停止和此玩家相关的事
	SS_UnloadEmployee Unload;

	Unload.m_uidActor = EmployeeUid.ToUint64();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadEmployee);

	this->OnEvent(msgID,&Unload,sizeof(Unload));

	//招募角色身上的装备要返还给玩家
	pEquipPart->RemoveAllEquipToPacket();

	//返还高级经验丹
	this->UnloadEmployeeBackExp(pEmployee);

	//获取参战人员
	ICombatPart * pCombatPart = this->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}

	std::vector<SCreatureLineupInfo> vect = pCombatPart->GetJoinBattleActor(); 
	for(int i=0; i<vect.size();++i)
	{
		if(vect[i].m_pCreature->GetUID() == EmployeeUid)
		{
			//移除阵型
			pCombatPart->RemoveJoinBattleActor(EmployeeUid);

			//战斗PART数据保存下，防止服务器挂掉，阵形里面又有此玩家
			//this->SaveData();

			break;
		}
	}


	RetCode = enUnLoadEmployeeRetCode_OK;

	this->NoticClientDestroyThing(EmployeeUid);

	for( int nIndex = 0; nIndex < MAX_EMPLOY_NUM; ++nIndex)
	{
		if( m_uidEmploy[nIndex] == EmployeeUid){
			m_uidEmploy[nIndex] = UID();
			break;
		}
	}

	SDB_Delete_Actor Delete_Actor;
	Delete_Actor.m_ActorUID = EmployeeUid.ToUint64();

	OBuffer1k ob;
	ob << Delete_Actor;
	g_pGameServer->GetDBProxyClient()->Request(this->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteActor,ob.TakeOsb(),0,0);

	//删除
	g_pGameServer->GetGameWorld()->DestroyThing(EmployeeUid);
}

//得到玩家能力
enNenLiType Actor::GetNenLi()
{
	INT32 nAptitude = this->GetCrtProp(enCrtProp_ActorAptitude);

	const SGameConfigParam & GameConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if( nAptitude < GameConfigParam.m_MaxNenLiPuTong){
		return enNenLiType_PuTong;
	}
	else if( nAptitude < GameConfigParam.m_MaxNenLiLiangHao){
		return enNenLiType_LiangHao;
	}
	else if( nAptitude < GameConfigParam.m_MaxNenLiYouZhi){
		return enNenLiType_YouZhi;
	}
	else if( nAptitude >= GameConfigParam.m_MaxNenLiYouZhi){
		return enNenLiType_QiCai;
	}
	else{
		TRACE("<error> %s : %d 行,资质数值错,玩家：%s", __FUNCTION__, __LINE__, this->GetName());
	}

	return enNenLiType_Max;
}

//获得沉迷系数
float Actor::GetWallowFactor()
{
	UINT8 WallowSwitch = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_WallowSwitch ; ///防沉迷开关,0关闭,1打开

	float Factor = 1.0;

	if(WallowSwitch==0)
	{
		return Factor;
	}

	if(this->GetMaster() != 0)
	{
		return this->GetMaster()->GetWallowFactor();
	}

	if(enIdentityStatus_Eighteen != GetCrtProp(enCrtProp_ActorIdentity))
	{
		//登陆时间
		INT32 LoginTime = GetCrtProp(enCrtProp_ActorLogin);

		//在线时长
		INT32 OnlineTime =  GetCrtProp(enCrtProp_ActorOnlineTimes) + CURRENT_TIME() - LoginTime;

		if(OnlineTime >= 5 * 3600)
		{
			Factor = 0;
		}
		else if(OnlineTime >= 3 * 3600)
		{
			Factor = 0.5;
		}

	}

	return Factor;
}

//计算灵力
INT32 Actor::RecalculateSpiri(float Aptitude,INT32 Level,INT32 ForeverSpirit,INT32 LayerSpirit)
{
	//重新计算基本灵力 当前等级×15+25+（当前等级×15+25）×资质+永久增加的+境界灵力
	INT32 Value = (Level * 15 +25)*(1+Aptitude) + 0.99999 + ForeverSpirit + LayerSpirit;

	return Value;

}

//计算护盾
INT32 Actor::RecalculateShield(float Aptitude,INT32 Level,INT32 ForeverShield,INT32 LayerShield)
{
	//重新计算基本护盾 当前等级×12+20+（当前等级×12+20）×资质+永久增加的
	INT32 Value = (Level * 12 +20)*(1+Aptitude) + 0.99999 + ForeverShield + LayerShield;

	return Value;

}

//计算身法
INT32 Actor::RecalculateAvoid(float Aptitude,INT32 Level,INT32 ForeverAvoid,INT32 LayerAvoid)
{
	//重新计算基本身法 当前等级×7.5+10+（当前等级×7.5+10）×资质+永久增加的
	INT32 Value = (Level * 7.5 +10)*(1+Aptitude) + 0.99999 + ForeverAvoid + LayerAvoid;

	return Value;

}

//计算气血
INT32 Actor::RecalculateBoold(float Aptitude,INT32 Level,INT32 ForeverBoold,INT32 LayerBoold)
{
	//重新计算基本气血上限 当前等级×72+300+（当前等级×72+300）×资质+永久增加的
	INT32 Value = (Level * 72 +300)*(1+Aptitude) + 0.99999 + ForeverBoold + LayerBoold;

	return Value;

}


//当等级或资质变化时,得重新计算部分基本属性
void Actor::LevelOrAptitudeChange()
{
	//当前资质
	float Aptitude = this->GetCrtProp(enCrtProp_ActorAptitude) / 1000.0f;

	//当前等级
	UINT8 Level = this->GetCrtProp(enCrtProp_Level);

	const SActorLayerCnfg * pActorLayerCnfg = g_pGameServer->GetConfigServer()->GetActorLayerCnfg(this->GetCrtProp(enCrtProp_ActorLayer));

	if ( 0 == pActorLayerCnfg){

		TRACE("<error> %s : %d Line 获取玩家境界配置信息出错！！境界等级:%d", __FUNCTION__, __LINE__, this->GetCrtProp(enCrtProp_ActorLayer));
		return;
	}

	//重新计算基本灵力 当前等级×15+25+（当前等级×15+25）×资质+永久增加的
	this->SetCrtProp(enCrtProp_ActorSpiritBasic, RecalculateSpiri(Aptitude,Level ,this->GetCrtProp(enCrtProp_ForeverSpirit), pActorLayerCnfg->m_AddSpirit));

	//重新计算基本护盾 当前等级×12+20+（当前等级×12+20）×资质+永久增加的
	this->SetCrtProp(enCrtProp_ActorShieldBasic,RecalculateShield(Aptitude,Level ,this->GetCrtProp(enCrtProp_ForeverShield) , pActorLayerCnfg->m_AddShield));

	//重新计算基本身法 当前等级×7.5+10+（当前等级×7.5+10）×资质+永久增加的
	INT32 Value =  RecalculateAvoid(Aptitude,Level , this->GetCrtProp(enCrtProp_ForeverAvoid) , pActorLayerCnfg->m_AddAvoid);

	this->SetCrtProp(enCrtProp_ActorAvoidBasic, RecalculateAvoid(Aptitude,Level , this->GetCrtProp(enCrtProp_ForeverAvoid) , pActorLayerCnfg->m_AddAvoid));

	//重新计算基本气血上限 当前等级×72+300+（当前等级×72+300）×资质+永久增加的
	this->SetCrtProp(enCrtProp_ActorBloodUpBasic, RecalculateBoold(Aptitude,Level , this->GetCrtProp(enCrtProp_ForeverBloodUp) , pActorLayerCnfg->m_AddBlood));
}

//获取上次存盘时间
UINT32 Actor::GetLastSaveToDBTime()
{
	return m_LastSaveToDBTime;
}

//设置是否需要存盘
void Actor::SetSaveFlag(bool bSave)
{
	m_bNeedSave = bSave;
}

bool Actor::GetSaveFlag()
{
	return m_bNeedSave;
}

//得到给各模块使用的标志
bool Actor::GetUseFlag(enUseFlag UseFlag)
{
	return m_UseFlag.get(UseFlag);
}

//设置给各模块使用的标志
void Actor::SetUseFlag(enUseFlag UseFlag, bool bValue)
{
	m_UseFlag.set(UseFlag, bValue);
}

//得到我的帮派
ISyndicate * Actor::GetSyndicate()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return 0;
	}

	return pSynMgr->GetSyndicate(this->GetUID());
}

//得到我的帮派ID
TSynID Actor::GetSynID()
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return 0;
	}

	return pSynMgr->GetUserSynID(this->GetUID());
}

//掉线，下线要做的一些事
void	Actor::LeaveDoSomeThing()
{
	//下线时保存玩家的帮派成员信息，如果玩家有帮派的话
	ISyndicateMgr * pSyndicateMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSyndicateMgr){
		return;
	}

	ISyndicateMember * pSyndicateMember = pSyndicateMgr->GetSyndicateMember(this->GetUID());

	if( 0 != pSyndicateMember){
		//同步下数据
		pSyndicateMember->SycUserData(this->GetCrtProp(enCrtProp_ActorCombatAbility), this->GetCrtProp(enCrtProp_Level), this->GetCrtProp(enCrtProp_TotalVipLevel), this->GetCrtProp(enCrtProp_SynCombatLevel), this->GetCrtProp(enCrtProp_ActorFacade));

		pSyndicateMember->UpdateSynMemberData();
	}

	IFriendPart  * pFriendPart = this->GetFriendPart();
	if( 0 == pFriendPart){
		return;
	}

	pFriendPart->LeaveDoSomeThing();

	ITrainingHallPart * pTrainingPart = this->GetTrainingHallPart();
	if( 0 == pTrainingPart){
		return;
	}

	pTrainingPart->LeaveDoSomeThing();

	IActorBasicPart * pActorBasicPart = this->GetActorBasicPart();
	if( 0 == pActorBasicPart){
		return;
	}

	pActorBasicPart->LeaveDoSomeThing();

	IFuMoDongPart * pFuMoDongPart = this->GetFuMoDongPart();

	if ( 0 == pFuMoDongPart ){
		return;
	}

	pFuMoDongPart->LeaveDoSomeThing();

	ITeamPart * pTeamPart = this->GetTeamPart();

	if ( 0 == pTeamPart ){
		return;
	}

	pTeamPart->LeaveDoSomeThing();

	ITalismanPart * pTalismanPart = this->GetTalismanPart();

	if ( 0 == pTalismanPart ){
		return;
	}

	pTalismanPart->LeaveDoSomeThing();

	IDouFaPart * pDouFaPart = this->GetDouFaPart();

	if ( 0 == pDouFaPart ){
		return;
	}

	pDouFaPart->LeaveDoSomeThing();

	//修改并保存上次登录最后在线时间  !!注意要写在保存练功堂后面，练功堂下线时有用到
	if ( m_bSelfOnline )
	{
		SDB_Update_LastOnlineTime DBReq;

		DBReq.m_uidUser = this->GetUID().ToUint64();

		UINT32 CurTime = CURRENT_TIME();

		DBReq.m_LastOnlineTime = CurTime;

		__SetPropValue(enCrtProp_LastOnlineTime, CurTime);

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(this->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_Update_LastOnlineTime,ob.TakeOsb(),0,0);			
	}

	this->SetIsSelfOnline(false);
}

//给出战角色获取经验
void Actor::CombatActorAddExp(INT32 nAddExp,bool bMultipExp, INT32 * pNewExp/*, bool bLianGong, IActor * pActor*/)
{
	ICombatPart * pCombatPart = this->GetCombatPart();
	if( 0 == pCombatPart){
		return;
	}
	

	//std::string szTip = g_pGameServer->GetGameWorld()->GetLanguageStr(10089);


	std::vector<SCreatureLineupInfo> vecCombatActor = pCombatPart->GetJoinBattleActor();

	for( int i = 0; i < vecCombatActor.size(); ++i)
	{
		ICreature * pCreature = vecCombatActor[i].m_pCreature;
		if( 0 == pCreature || pCreature->GetThingClass() != enThing_Class_Actor){
			continue;
		}

		IActor * pActor = (IActor *)pCreature;

		if( bMultipExp){
			INT32 nNewExpEmployee = 0;
			INT32 nOldExp = pActor->GetCrtProp(enCrtProp_ActorExp);

			if( pActor->GetMaster() == 0){
				pActor->AddCrtPropNum(enCrtProp_ActorMultipExp,nAddExp, pNewExp);
			}else{
				pActor->AddCrtPropNum(enCrtProp_ActorMultipExp,nAddExp,&nNewExpEmployee);
			}

			////获得练功经验时，在聊天框显示各个角色获得的经验
			//if(bLianGong){
			//	char szMsg[DESCRIPT_LEN_50] = "\0";
			//	std::string douhao = g_pGameServer->GetGameWorld()->GetLanguageStr(10092);

			//	if( pActor->GetMaster() == 0){
			//		sprintf_s(szMsg,sizeof(szMsg),g_pGameServer->GetGameWorld()->GetLanguageStr(10090),pActor->GetName(),*pNewExp - nOldExp);

			//	}else{
			//		sprintf_s(szMsg,sizeof(szMsg),g_pGameServer->GetGameWorld()->GetLanguageStr(10090),pActor->GetName(),nNewExpEmployee - nOldExp);

			//	}
			//	
			//	if(i < vecCombatActor.size() - 1){
			//		szTip = szTip + szMsg + douhao;
			//	}else{
			//		szTip = szTip + szMsg;
			//	}
			//				
			//}
		}else{

			if( pActor->GetMaster() == 0){
				pActor->AddCrtPropNum(enCrtProp_ActorExp,nAddExp, pNewExp);
			}else{
				pActor->AddCrtPropNum(enCrtProp_ActorExp,nAddExp);
			}
		}
	}

	//if(bLianGong){
	//	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip.c_str(),pActor,enTalkMsgType_UpLevel);
	//}
}

//解雇招募角色,返还经验
void Actor::UnloadEmployeeBackExp(IActor * pEmployee)
{
	if( this == pEmployee){
		return;
	}

	UINT64 nTotalExp = 0;

	for( int i = pEmployee->GetCrtProp(enCrtProp_Level); i > 0; --i)
	{
		const SActorLevelCnfg * pLvCnfg = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(i);
		if( 0 == pLvCnfg){
			return;
		}

		nTotalExp += pLvCnfg->m_NeedExp;
	}

	nTotalExp += pEmployee->GetCrtProp(enCrtProp_ActorExp);

	const SGameServerConfigParam & ServerParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	///超级经验丹ID
	TGoodsID  GaoJiExpGoodsID = ServerParam.m_SuperGaoJiExpGoodsID;

	///超级经验丹增加多少经验
	UINT32    GaoJiExpAddExpNum = ServerParam.m_SuperGaoJiExpAddExpNum;

	///解雇招募角色最多返还多少经验(百分比)
	UINT32	 BackExpRate = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_UnEmployeeBackExpRate;

	BackExpRate += this->GetVipValue(enVipType_UnLoadEmployAddExp);

	//最多返回多少高级经验丹
	//UINT16	 MaxBackGaoJiNum = ServerParam.m_MaxBackGaoJiNum;

	nTotalExp = nTotalExp * BackExpRate / 100.0f + 0.99999;

	if( 0 == GaoJiExpAddExpNum){
		return;
	}

	UINT32	 GetGoodsNum = nTotalExp / GaoJiExpAddExpNum;

	if( GetGoodsNum <= 0){
		return;
	}

	SWriteSystemData SysMailData;

	SysMailData.m_DestUID = this->GetUID();

	// fly add	20121106
	strncpy(SysMailData.m_szThemeText,g_pGameServer->GetGameWorld()->GetLanguageStr(10010), sizeof(SysMailData.m_szThemeText));
	sprintf_s(SysMailData.m_szContentText, sizeof(SysMailData.m_szContentText),g_pGameServer->GetGameWorld()->GetLanguageStr(10046),GetGoodsNum);
	//strncpy(SysMailData.m_szThemeText, "解雇角色返还经验", sizeof(SysMailData.m_szThemeText));
	//sprintf_s(SysMailData.m_szContentText, sizeof(SysMailData.m_szContentText),"解雇角色返还70%%的经验(vip返还90%%的经验)，最多返还500W的经验!!返还经验丹%d个!!",GetGoodsNum);

	std::vector<IGoods *> vectGoods = g_pGameServer->GetGameWorld()->CreateGoods(GaoJiExpGoodsID, GetGoodsNum, true);

	g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData, vectGoods);
}

//得到VIP配置值
INT32	Actor::GetVipValue(enVipType VipType)
{
	INT32 VipLevel = this->GetCrtProp(enCrtProp_TotalVipLevel);

	if ( 0 >= VipLevel){

		return 0;
	}

	const SVipConfig * pVipConfig = g_pGameServer->GetConfigServer()->GetVipConfig(VipLevel);

	if ( 0 == pVipConfig){

		TRACE("<error> %s : %d Line,获取VIP配置信息出错!!VIP等级 : %d", __FUNCTION__, __LINE__, VipLevel);
		return 0;
	}

	switch (VipType)
	{
	case enVipType_TakeResAddStone:
		{
			return pVipConfig->m_TakeResAddStone;
		}
		break;
	case enVipType_bCanAutoTakeRes:
		{
			return pVipConfig->m_bCanAutoTakeRes;
		}
		break;
	case enVipType_AutoKillAddExp:
		{
			return pVipConfig->m_AutoKillAddExp;
		}
		break;
	case enVipType_AccelKillDesMoney:
		{
			return pVipConfig->m_AccelKillDesMoney;
		}
		break;
	case enVipType_AccelKillAddHour:
		{
			return pVipConfig->m_AccelKillAddHour;
		}
		break;
	case enVipType_FinishTrainingFreeNum:
		{
			return pVipConfig->m_FinishTrainingFreeNum;
		}
		break;
	case enVipType_AddTrainingNum:
		{
			return pVipConfig->m_AddTrainingNum;
		}
		break;
	case enVipType_BuySwordEmployMagicDesMoney:
		{
			return pVipConfig->m_BuySwordEmployMagicDesMoney;
		}
		break;
	case enVipType_AddGodSwordWorldNum:
		{
			return pVipConfig->m_AddGodSwordWorldNum;
		}
		break;
	case enVipType_AddXiuLianPos:
		{
			return pVipConfig->m_AddXiuLianPos;
		}
		break;
	case enVipType_AddSynCombatNum:
		{
			return pVipConfig->m_AddSynCombatNum;
		}
		break;
	case enVipType_AutoKillSynCombat:
		{
			return pVipConfig->m_AutoKillSynCombat;
		}
		break;
	case enVipType_bFreeSellGoods:
		{
			return pVipConfig->m_bFreeSellGoods;
		}
		break;
	case enVipType_bFreeRemoveGem:
		{
			return pVipConfig->m_bFreeRemoveGem;
		}
		break;
	case enVipType_AddFuBenNum:
		{
			return pVipConfig->m_AddFuBenNum;
		}
		break;
	case enVipType_bAutoKillFuBen:
		{
			return pVipConfig->m_bAutoKillFuBen;
		}
		break;
	case enVipType_UnLoadEmployAddExp:
		{
			return pVipConfig->m_UnLoadEmployAddExp;
		}
		break;
	case enVipType_AddStrongerRate:
		{
			return pVipConfig->m_AddStrongerRate;
		}
		break;
	case enVipType_AddMaxHonorToday:
		{
			return pVipConfig->m_AddMaxHonorToday;
		}
		break;
	case enVipType_AddGetHonor:
		{
			return pVipConfig->m_AddGetHonor;
		}
		break;
	case enVipType_AddXLNimbus:
		{
			return pVipConfig->m_AddXlNimbus;
		}
		break;
	case enVipType_AddEnterTalismanNum:
		{
			return pVipConfig->m_AddEnterTalismanNum;
		}
		break;
	case enVipType_AddDuoBaoCreditUp:
		{
			return pVipConfig->m_AddDuoBaoCreditUp;
		}
		break;
	case enVipType_AddSynWarCreditUp:
		{
			return pVipConfig->m_AddSynWarCreditUp;
		}
		break;
	case enVipType_AddGetCredit:
		{
			return pVipConfig->m_AddGetCredit;
		}
		break;
	default:
		{
			TRACE("<error> %s : %d Line VIP类型错误!!类型 : %d", __FUNCTION__, __LINE__, VipType);
		}
		break;
	}

	return 0;
}

//保存玩家属性
bool Actor::SaveActorProp()
{
	if (this->GetSaveFlag())
	{
		SDB_Update_ActorBasicData_Req ActorBasicData;
		int len = sizeof(SDB_Update_ActorBasicData_Req);

		if(OnGetDBContext(&ActorBasicData,len)==false)
		{
			return false;
		}

		TOBuffer<sizeof(SDB_Update_ActorBasicData_Req)> ob;
		ob << ActorBasicData;
		g_pGameServer->GetDBProxyClient()->Request(this->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_Update_ActorBasicDataInfo,ob.TakeOsb(),0,0);

		SetSaveFlag(false);
	}

	return true;
}

//得到玩家在干什么
enActorDoing Actor::GetActorDoing()
{
	if ( this->GetIsInCombat() )
	{
		//在战斗
		return enActorDoing_Combat;
	}

	if ( this->IsInDuoBao() )
	{
		//在夺宝
		return enActorDoing_DuoBao;
	}

	if ( this->IsInTeamFuBen() )
	{
		//在组队副本
		return enActorDoing_FuBen_Team;
	}

	if ( this->IsInFastWaitTeam() )
	{
		//等待组队
		return enActorDoing_WaitTeam;
	}

	SS_Get_ActorDoing ActorDoing;

	ActorDoing.m_uidActor = this->GetUID().ToUint64();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);
	this->OnEvent(msgID,&ActorDoing,SIZE_OF(ActorDoing));

	return (enActorDoing)ActorDoing.m_ActorDoing;
}

//克隆
IActor * Actor::MakeCloner()
{
	/*ISession *  m_pSession;

	IThingPart * m_pThingParts[enThingPart_Crt_Max-enThingPart_Actor];

	int  m_ActorProp[enCrtProp_Max-enCrtProp_Actor];

	UID      m_uidEmploy[MAX_EMPLOY_NUM];  //招募

	IActor*  m_pMaster;

	std::set<UID>	m_setAttentionUser;	//关注的玩家，会否决这些玩家释放内存

	UINT32  m_LastSaveToDBTime; //最后一次存盘时间

	bool  m_bNeedSave; //是否需要存盘

	TBitArray<enUseFlag_Max>   m_UseFlag;	//给各模块使用的标志*/

	Actor * pActor = new Actor();

	memcpy(pActor->m_CreatureProp,m_CreatureProp,MEM_SIZE(m_CreatureProp));

	memcpy(pActor->m_szName,m_szName,MEM_SIZE(m_szName));

	pActor->m_Uid = m_Uid;	


	pActor->m_pSession = m_pSession;
	memcpy(pActor->m_ActorProp,m_ActorProp,MEM_SIZE(m_ActorProp));
	memcpy(pActor->m_uidEmploy,m_ActorProp,MEM_SIZE(m_uidEmploy));
	pActor->m_pMaster = m_pMaster;
	pActor->m_setAttentionUser = m_setAttentionUser;
	pActor->m_bNeedSave = false;
	pActor->m_UseFlag = m_UseFlag;

	//创建状态part
	IStatusPart * pStatusPart = new StatusPart();

	pStatusPart->Create(pActor,0,0);

	pActor->AddPart(pStatusPart);

	pActor->m_SourceActor = this;

	return pActor;
}

//是否是克隆人
bool  Actor::IsClone()
{
	return m_SourceActor != 0;
}

//多倍经验系数
float Actor::GetMultipExpFactor()
{
	UINT8 ActorLevel = this->GetCrtProp(enCrtProp_Level);

	const SServer_Info & ServerInfo = g_pGameServer->GetServerInfo();

	UINT32 CurTime = CURRENT_TIME();

	if(ActorLevel>= ServerInfo.m_MinMultipExpLv && ActorLevel<ServerInfo.m_MaxMultipExpLv
		&& CurTime>= ServerInfo.m_MultipExpBeginTime && CurTime < ServerInfo.m_MultipExpEndTime)
	{
		return ServerInfo.m_fMultipExp;
	}

	return 1.0;
}

//设置是否在战斗
void	Actor::SetIsInCombat(bool bInCombat)
{
	if ( this->GetMaster() == 0 )
	{
		m_bInCombat = bInCombat;
	}
}

//得到是否在战斗
bool	Actor::GetIsInCombat()
{
	if ( this->GetMaster() == 0 )
	{
		return m_bInCombat;
	}
	
	IActor * pMaster = this->GetMaster();

	if ( 0 != pMaster )
	{
		return pMaster->GetIsInCombat();
	}

	return false;
}

//得到是否在夺宝中
bool	Actor::IsInDuoBao()
{
	IDouFaPart * pDouFaPart = this->GetDouFaPart();

	if ( 0 == pDouFaPart )
		return false;

	return pDouFaPart->IsInDuoBao();
}

//得到是否在组队副本中
bool	Actor::IsInTeamFuBen()
{
	IFuBenPart * pFuBenPart = this->GetFuBenPart();

	if ( 0 == pFuBenPart )
		return false;

	return pFuBenPart->IsInTeamFuBen();
}

//得到是否有队伍
bool	Actor::HaveTeam()
{
	ITeamPart * pTeamPart = this->GetTeamPart();

	if ( 0 == pTeamPart )
		return false;

	return pTeamPart->IsHaveTeam();
}

//设置是否是自己在线
void	Actor::SetIsSelfOnline(bool bSelf)
{
	m_bSelfOnline = bSelf;
}

//得到是否是自己在线
bool	Actor::GetIsSelfOnline()
{
	return m_bSelfOnline;
}

//得到是否在快速组队等待中
bool	Actor::IsInFastWaitTeam()
{
	ITeamPart * pTeamPart = this->GetTeamPart();

	if ( 0 == pTeamPart )
		return false;

	return pTeamPart->GetIsInFastTeam();
}
