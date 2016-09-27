
#include "IActor.h"
#include "Magic.h"

#include "IConfigServer.h"
#include "DMsgSubAction.h"
#include "CombatServer.h"
#include "IGameWorld.h"
#include "IStatusPart.h"
#include "RandomService.h"
#include "IMonster.h"
#include "IMagicPart.h"
#include "IEquipPart.h"
#include "IEquipment.h"
#include "IGodSword.h"
#include "IBasicService.h"

Magic::Magic()
{
	m_pMagicLevelCnfg = 0;
}
Magic::~Magic()
{
}

//获取技能ID
TMagicID  Magic::GetMagicID()
{
	return m_pMagicLevelCnfg->m_MagicID;
}

//获取技能等级
UINT8  Magic::GetLevel()
{
	return m_pMagicLevelCnfg->m_Level;
}

//获得配置信息
const SMagicLevelCnfg*  Magic::GetMagicLevelCnfg()
{
	return m_pMagicLevelCnfg;
} 

// 创建技能
// 参数：pResObject 该技能的资源对象
bool Magic::Create(const SMagicLevelCnfg* pMagicConfig)
{
	m_pMagicLevelCnfg = pMagicConfig;

	return true;
}

// 释放
void Magic::Release()
{
	delete this;
}

//加载
bool Magic::OnEquip(IActor * pActor)
{
	//如果不是被动技能，不需要处理
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(this->GetMagicID());

	if(pMagicCnfg==0 || pMagicCnfg->m_bPassive==false)
	{
		return true;
	}


	IStatusPart * pStatusPart = (IStatusPart *)pActor->GetPart(enThingPart_Crt_Status);

	//增加效果
	for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
	{
		pStatusPart->AddEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
	}

	//增加状态
	for(int i=0; i<m_pMagicLevelCnfg->m_vectTargetStatus.size();i++)
	{
		std::vector<UINT8> vectDeleteStatusType;

		pStatusPart->AddStatus(m_pMagicLevelCnfg->m_vectTargetStatus[i],pActor->GetUID(), vectDeleteStatusType, GetMagicID());
	}

	//重新计算属性
	pActor->RecalculateProp();

	return true;
}

//取下
bool Magic::OnUnEquip(IActor * pActor)
{
	//如果不是被动技能，不需要处理
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(this->GetMagicID());

	if(pMagicCnfg==0 || pMagicCnfg->m_bPassive==false)
	{
		return true;
	}

	IStatusPart * pStatusPart = (IStatusPart *)pActor->GetPart(enThingPart_Crt_Status);

	//增加效果
	for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
	{
		pStatusPart->RemoveEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
	}

	//增加状态
	for(int i=0; i<m_pMagicLevelCnfg->m_vectTargetStatus.size();i++)
	{
		pStatusPart->RemoveStatus(m_pMagicLevelCnfg->m_vectTargetStatus[i],pActor->GetUID());
	}

	//重新计算属性
	pActor->RecalculateProp();

	return true;
}

// 判断生物能否使用该技能
bool Magic::CanUseSkill(ICreature * pCreature,UID uidTarget)
{	
	if(pCreature == 0 || (pCreature->GetThingClass() != enThing_Class_Actor
		&& pCreature->GetThingClass() !=enThing_Class_Monster))
	{
		return false;
	}

	//被动法术不能施放
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(this->GetMagicID());

	if(pMagicCnfg==0 || pMagicCnfg->m_bPassive)
	{
		return false;
	}
	SAttackContext AttackCnt;
	AttackCnt.m_uidSource =  pCreature->GetUID();
	AttackCnt.m_BaseAttackValue = 0;
	AttackCnt.m_uidTarget = uidTarget;
	AttackCnt.m_AttackType = enAttackType_Magic;
	AttackCnt.m_MagicID = GetMagicID();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack);

	//投票
	if(pCreature->FireVote(msgID,&AttackCnt,sizeof(AttackCnt))==false)
	{
		return false;
	}

	return true;
}

//获得玩家的法术伤害 
INT32 Magic::GetActorMagicDamage(ICreature * pCreature)
{
	INT32 MagicDamage = 0;

	if(pCreature->GetThingClass() == enThing_Class_Actor)
	{
		MagicDamage = ((IActor*)pCreature)->GetCrtProp(enCrtProp_ActorMagic);
	}
	else if(pCreature->GetThingClass() == enThing_Class_Monster)
	{
		MagicDamage = ((IMonster*)pCreature)->GetMagicDamage();

	}

	return MagicDamage;
}

//获得法术基础命中率
float Magic::GetMagicBaseHitRate(IFighter * pSource,IFighter * pEnemy)
{

	//计算基础命中系数

	//不带状态:	法术基础命中率=40%+命中回避参数
	//带状态:为法术命中参数

	float BaseHitRate = 0;

	if(m_pMagicLevelCnfg->m_vectTargetStatus.empty())
	{
		//不带状态
		BaseHitRate = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_MagicHitFactor/(float)1000 + pSource->CalculateMagicHitAndAvoidParam(pEnemy);
	}
	else
	{
		BaseHitRate = m_pMagicLevelCnfg->m_HitParam/(float)100;
	}




	return BaseHitRate;

}


//法术攻击是否命中
bool  Magic::IsMagicAttackHit(const SMagicCnfg * pMagicCnfg,IFighter * pSource,IFighter * pEnemy)
{
	//源和目标相同100%命中
	if(pSource == pEnemy)
	{
		return true;
	}

	//法术命中率=(基础命中系数+角色等级命中系数)×命中调整基准
	//且法术命中率最大值为90%，最小值10%


	//计算基础命中系数
	float BaseHitRate = GetMagicBaseHitRate(pSource,pEnemy);


	//角色等级命中系数=施法角色等级×1%—承受角色等级×1%
	float ActorHitParam = (pSource->GetCreature()->GetCrtProp(enCrtProp_Level) - pEnemy->GetCreature()->GetCrtProp(enCrtProp_Level))*0.01;

	//命中调整
	float HitAdjust = 1.0;
	if(m_pMagicLevelCnfg->m_HitDdjustLevel > 0)
	{
		if(pEnemy->GetCreature()->GetCrtProp(enCrtProp_Level)>=m_pMagicLevelCnfg->m_HitDdjustLevel )
		{
			HitAdjust = 0.1;
		}
	}

	//千分之
	INT32 MagicHitRate = (BaseHitRate + ActorHitParam) * HitAdjust *1000;

	if(MagicHitRate<100)
	{
		MagicHitRate = 100;
	}
	else if(MagicHitRate > 900)
	{
		MagicHitRate = 900;
	}

	//随机数
	INT32 nRandom = RandomService::GetRandom() % 1000;

	return nRandom < MagicHitRate;
}

const SStatusTypeCnfg * Magic::GetMaxPriorityStatusTypeCnfg(ICreature * pCreature)
{
	IStatusPart * pStatusPart = (IStatusPart *)pCreature->GetPart(enThingPart_Crt_Status);

	if(pStatusPart==0)
	{
		return 0;
	}

	return pStatusPart->GetMaxPriorityStatusTypeCnfg() ;	
}

//获得法术爆击参数
float Magic::GetMagicKnockingParam(IFighter * pSource,IFighter * pEnemy)
{

	//法术爆击率=（攻方[爆击]/25000-守方[坚韧]/30000）×目标状态参数

	//怪物不能爆击
	if(pSource->GetCreature()->GetThingClass() == enThing_Class_Monster)
	{
		return 0;
	}


	//攻方[爆击]
	INT32 OneselfCriticalHitValue = pSource->GetCriticalHitValue();

	//守方[坚韧]
	INT32 EnemyTenacityValue = pEnemy->GetTenacityValue();



	INT32 MagicKnockingAttackedStatusParam = 1000;
	const SStatusTypeCnfg * pAttackedTypeCnfg = GetMaxPriorityStatusTypeCnfg(pEnemy->GetCreature());

	if(pAttackedTypeCnfg != 0)
	{
		MagicKnockingAttackedStatusParam = pAttackedTypeCnfg->m_MagicKnockingStatusParam;
	}

	INT32 MagicKnockingRate = (float)(OneselfCriticalHitValue/(float)25000-EnemyTenacityValue/(float)30000)*10 * MagicKnockingAttackedStatusParam;

	INT32 nRandom = RandomService::GetRandom() % 1000*1000;

	if(nRandom > MagicKnockingRate)
	{
		//不爆击
		return 0;
	}

	INT32 MagicAttackCriticalHitParam = 0;

	//爆击
	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	//按概率选取
	static INT32 TotalProb = 0;  //总概率
	if(TotalProb == 0)
	{
		for(int i=0; i<ConfigParam.m_MagicKnockingParamAndProb.size()/2; i++)
		{
			TotalProb += ConfigParam.m_MagicKnockingParamAndProb[i*2+1];
		}
	}

	if(TotalProb == 0)
	{
		return 0;
	}

	//随机数
	INT32 RandomNum = RandomService::GetRandom()%TotalProb;

	for(int i=0; i<ConfigParam.m_MagicKnockingParamAndProb.size()/2; i++)
	{
		if(RandomNum<ConfigParam.m_MagicKnockingParamAndProb[i*2+1])
		{
			MagicAttackCriticalHitParam = (float)ConfigParam.m_MagicKnockingParamAndProb[i*2];
			break;
		}

		RandomNum -= ConfigParam.m_MagicKnockingParamAndProb[i*2+1];
	}


	return (float)MagicAttackCriticalHitParam/1000;
}

//获得基础攻击力
INT32 Magic::GetBaseAttackValue(const SMagicCnfg * pMagicCnfg,ICreature * pSource)
{
	INT32 BaseAttackValue = 1;

	if(pMagicCnfg->m_Class != enMagicClass_Talisman)
	{
		BaseAttackValue = GetActorMagicDamage(pSource);
	}

	return BaseAttackValue;
}


//对指定对象使用治疗技能
bool Magic::UseSkillToSingleObjectWithCure(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSourceFighter ,float fScopeParam,IFighter * pCreatureFighter,
										   SAddBloodTarget & AddBloodTarget,OBuffer4k & obStauts,INT32 & RecordNum)
{
	ICreature * pSource = pSourceFighter->GetCreature();
	ICreature * pCreature = pCreatureFighter->GetCreature();

	AddBloodTarget.m_uidCreature = pCreature->GetUID();


	SAttackContext AttackCnt;
	AttackCnt.m_uidSource       = pSource->GetUID();
	AttackCnt.m_BaseAttackValue = GetBaseAttackValue(pMagicCnfg,pSource);
	AttackCnt.m_uidTarget       = pSource->GetUID();
	AttackCnt.m_AttackType      = enAttackType_Magic;


	float AttackCriticalHitParam = 0;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Cure);

	//发布事件
	pSource->OnAction(msgID,&AttackCnt,sizeof(AttackCnt));


	IStatusPart * pStatusPart = (IStatusPart *)pCreature->GetPart(enThingPart_Crt_Status);

	//如果是主动技术需要增加效果,被动技能不需要加
	if(pMagicCnfg->m_bPassive==false)
	{
		for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
		{
			pStatusPart->AddEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
		}
	}


	//真实攻击力
	INT32 AttackValue = AttackCnt.m_BaseAttackValue;


	//计算伤害
	SAttackedContext AttackedCnt;
	AttackedCnt.m_AddDamageValue = 0;
	AttackedCnt.m_DamageType = enDamageType_Magic;
	AttackedCnt.m_MagicID = GetMagicID();
	AttackedCnt.m_BaseDamageValue = AttackValue;
	AttackedCnt.m_uidSource = pSource->GetUID();
	AttackedCnt.m_uidTarget = pCreature->GetUID();

	pCreature->OnAction(msgID,&AttackedCnt,sizeof(AttackedCnt));

	INT32 DamageValue = AttackedCnt.m_BaseDamageValue + AttackedCnt.m_AddDamageValue;

	if(AttackedCnt.m_SuckBloodFactor != 0)
	{
		if(pMagicCnfg->m_Class == enMagicClass_Talisman)
		{
			////法宝治疗数值={(承受者最大气血)×治疗系数
			AddBloodTarget.m_BloodValue = GetBloodUp(pCreature)*AttackedCnt.m_SuckBloodFactor;

		}
		else
		{
			//普通法术治疗数值=自身[法术伤害]×治疗系数
			AddBloodTarget.m_BloodValue = GetActorMagicDamage(pSource)*AttackedCnt.m_SuckBloodFactor;
		}


		//加血
		INT32 nNewValue = 0;
		pCreature->AddCrtPropNum(enCrtProp_Blood,AddBloodTarget.m_BloodValue,&nNewValue);

		AddBloodTarget.m_bDie = (nNewValue==0);

		if(AddBloodTarget.m_BloodValue<0)
		{
			int i = 0;
		}

	}


	//效果是一次性的，需要移除
	if(pMagicCnfg->m_bPassive==false)
	{
		for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
		{
			pStatusPart->RemoveEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
		}
	}

	//需要判断目标是否已死亡，如已死亡不需要加状态了
	if(AddBloodTarget.m_bDie)
	{
		return true;
	}

	//增加状态
	for(int i=0; i<m_pMagicLevelCnfg->m_vectTargetStatus.size();i++)
	{
		pCreatureFighter->AddStatus(m_pMagicLevelCnfg->m_vectTargetStatus[i],GetMagicID(),0);	
	}

	return true;
}

//获得五行参数
float Magic::GetWuXinParam(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource)
{
	float WuXinParam = 1.0;

	if(pSource->GetCreature()->GetThingClass() != enThing_Class_Actor)
	{
		return WuXinParam;
	}

	enWuXing WuXing = pSource->GetGoldSwordWuXing(RoundNum);

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if(WuXing == pMagicCnfg->m_WuXing)
	{
		WuXinParam = ConfigParam.m_SameWuXinParam/(float)1000;
	}
	else
	{
		WuXinParam = ConfigParam.m_DifferentWuXinParam/(float)1000;
	}

	return WuXinParam;
}

//计算法术最后的伤害值
INT32  Magic::CalculateMagicDamage(float fScopeParam,INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource,IFighter * pEnemy,INT32 BaseDamageValue,
								   float MagicKnockingParam)
{
	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
	float MagicDamage = 0;

	//随机数随机取0.97至1.03之间的数，准确到小数点第2位

	float fRomdan = ((970+RandomService::GetRandom()%(1030-970)+5))/10/(float)100;



	//目标状态参数
	float MagicAttackedStatusParam = 1.0;
	const SStatusTypeCnfg * pAttackedTypeCnfg = GetMaxPriorityStatusTypeCnfg(pEnemy->GetCreature());

	if(pAttackedTypeCnfg != 0)
	{
		MagicAttackedStatusParam = (float)pAttackedTypeCnfg->m_MagicDamageStatusParam /1000;
	}


	if(pMagicCnfg->m_Class == enMagicClass_Talisman)
	{
		//法宝法术伤害数值=(攻方[法宝伤害参数])×目标状态参数

		MagicDamage = MagicAttackedStatusParam*fRomdan;
	}
	else if(pMagicCnfg->m_Class == enMagicClass_Sword)
	{
		//剑诀伤害数值=『攻方[法术伤害] ×（1--免伤值）』×法术伤害系数×法术爆击参数×（1+对应属性剑诀伤害值/1000）×目标状态参数×随机数
		//免伤值
		float AvoidDamageValue = this->CalculateAvoidDamageValue(pSource->GetCreature(),pEnemy->GetCreature());


		//剑诀伤害值
		float SwordDamage = GetSwordizeDamageValue(pSource->GetCreature(),pMagicCnfg->m_WuXing)/(float)1000;

		//范围伤害参数
		MagicDamage =( BaseDamageValue*(1-AvoidDamageValue))*(1+SwordDamage)*MagicAttackedStatusParam*MagicKnockingParam*fRomdan;
	}
	else
	{

		//普通法术伤害数值=『攻方[法术伤害] ×（1--免伤值）』×法术伤害系数×法术爆击参数×法术五行参数×目标状态参数×随机数

		float  WuXinParam = GetWuXinParam(RoundNum,pMagicCnfg,pSource);

		//免伤值
		float AvoidDamageValue = this->CalculateAvoidDamageValue(pSource->GetCreature(),pEnemy->GetCreature());

		//范围伤害参数
		MagicDamage =( BaseDamageValue*(1-AvoidDamageValue))*MagicAttackedStatusParam*MagicKnockingParam*WuXinParam*fRomdan;
	}

	//如果爆击，则爆击伤害值=物理伤害最终值×参数×【1-守方[坚韧]/ (守方[坚韧]+30000)】

	if(MagicKnockingParam>1.0)
	{

		//守方[坚韧]
		INT32 EnemyTenacityValue = pEnemy->GetTenacityValue();


		MagicDamage = MagicDamage*MagicKnockingParam*(1-(float)EnemyTenacityValue/(EnemyTenacityValue+30000));
	}

	INT32 value = MagicDamage*g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_MagicAttackFactor/(float)1000+0.999999;

	return value;
}

//获得剑诀伤害值
INT32 Magic::GetSwordizeDamageValue(ICreature * pCreature,UINT8 WuXing)
{
	enCrtProp PropID;

	switch(WuXing)
	{
	case enWuXing_Jing:
		{
			PropID = enCrtProp_GoldDamage;
		}
		break;
	case enWuXing_Mu:
		{
			PropID = enCrtProp_WoodDamage;
		}
		break;
	case enWuXing_Shui:
		{
			PropID = enCrtProp_WaterDamage;
		}
		break;
	case enWuXing_Huo:
		{
			PropID = enCrtProp_FireDamage;
		}
		break;
	case enWuXing_Tu:
		{
			PropID = enCrtProp_SoilDamage;
		}
		break;
	default:
		return 0;
		break;
	}
	/*
	const SDamageLevelCnfg * pDamageLevel = g_pGameServer->GetConfigServer()->GetMagicLvCnfg(pCreature->GetCrtProp(PropID));
	if( 0 == pDamageLevel){
	return 0;
	}

	return pDamageLevel->m_Damage;
	*/
	return pCreature->GetCrtProp(PropID);
}

//获得生物防御值
INT32 Magic::GetDefenseValue(ICreature * pCreature)
{
	INT32 DefenseValue = 0;
	if(pCreature->GetThingClass() == enThing_Class_Actor)
	{		
		DefenseValue = pCreature->GetCrtProp(enCrtProp_ActorDefend);
	}
	else if(pCreature->GetThingClass() == enThing_Class_Monster)
	{
		IMonster * pMonster = (IMonster*)pCreature;
		DefenseValue = pMonster->GetDefense();
	}

	return DefenseValue;
}

//获得攻击力浮动系数
float Magic::GetAttackFloatParam()
{
	return 1.0;

	const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	INT32 Random = 0;
	if(ConfigParam.m_AttackFloatLowLimit != ConfigParam.m_AttackFloatHigLimit)
	{
		Random = RandomService::GetRandom()%(ConfigParam.m_AttackFloatHigLimit-ConfigParam.m_AttackFloatLowLimit);
	}

	float value = (ConfigParam.m_AttackFloatLowLimit+Random)/(float)1000;

	return value;
}


//对指定对象使用攻击技能
bool Magic::UseSkillToSingleObjectWithAttack(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSourceFighter,float fScopeParam,IFighter * pCreatureFighter,
											 SAttackedTarget & AttackedTarget,INT32 & AddBloodValue
											 ,OBuffer4k & obStauts,INT32 & RecordNum, bool & bDecreaseCount)
{
	ICreature * pSource = pSourceFighter->GetCreature();

	ICreature * pCreature = pCreatureFighter->GetCreature();

	AttackedTarget.m_uidTarget = pCreature->GetUID();

	bDecreaseCount = false;


	SAttackContext AttackCnt;
	AttackCnt.m_uidSource       = pSource->GetUID();
	AttackCnt.m_BaseAttackValue = GetBaseAttackValue(pMagicCnfg,pSource); 
	AttackCnt.m_uidTarget       = pSource->GetUID();
	AttackCnt.m_AttackType      = enAttackType_Magic;



	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack);

	//计算命中
	if(IsMagicAttackHit(pMagicCnfg,pSourceFighter,pCreatureFighter)==false)
	{
		AttackedTarget.m_bHit = false;		
		AttackedTarget.m_DamageValue = 0;
		return true;
	}
	else
	{
		AttackedTarget.m_bHit = true;	
	}


	//发布事件
	pSource->OnAction(msgID,&AttackCnt,sizeof(AttackCnt));


	IStatusPart * pStatusPart = (IStatusPart *)pCreature->GetPart(enThingPart_Crt_Status);

	//如果是主动技术需要增加效果,被动技能不需要加
	if(pMagicCnfg->m_bPassive==false)
	{
		for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
		{
			pStatusPart->AddEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
		}
	}


	enDamageType DamageType = enDamageType_Magic; 

	//真实攻击力
	INT32 AttackValue = AttackCnt.m_BaseAttackValue+AttackCnt.m_AddAttackValue;
	if(AttackValue<=0)
	{
		TRACE("<error> %s : %d line 攻击力为零 magicid=%d level=%d",__FUNCTION__,__LINE__,pMagicCnfg->m_MagicID,this->GetLevel());

		AttackValue = 1;

	}

	float AttackCriticalHitParam = GetMagicKnockingParam(pSourceFighter,pCreatureFighter);

	if(AttackCriticalHitParam>0)
	{
		AttackedTarget.m_bKnocking = true;	
	}
	else
	{
		AttackedTarget.m_bKnocking = false;		
		AttackCriticalHitParam = 1.0;
	}




	//计算伤害
	SAttackedContext AttackedCnt;

	AttackedCnt.m_AddDamageValue = 0;
	AttackedCnt.m_DamageType = DamageType;
	AttackedCnt.m_MagicID = GetMagicID();
	AttackedCnt.m_BaseDamageValue = CalculateMagicDamage(fScopeParam,RoundNum,pMagicCnfg,pSourceFighter,pCreatureFighter,AttackValue,	AttackCriticalHitParam)*GetAttackFloatParam()+0.99999;
	AttackedCnt.m_uidSource = pSource->GetUID();
	AttackedCnt.m_uidTarget = pCreature->GetUID();

	pCreature->OnAction(MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attacked),&AttackedCnt,sizeof(AttackedCnt));

	INT32 DamageValue = AttackedCnt.m_AddDamageValue;


	bDecreaseCount = AttackedCnt.m_bDefense;

	if(DamageValue>0)
	{
		//受到伤害,扣血
		AttackedTarget.m_DamageType = DamageType;

		INT32 nCurrBlood = pCreature->GetCrtProp(enCrtProp_Blood);

		if(nCurrBlood<=DamageValue && AttackedCnt.m_bAvoidDie == true ) //免死
		{
			AttackedTarget.m_DamageValue = nCurrBlood-1; //留一点血
			bDecreaseCount = true;
		}
		else
		{
			AttackedTarget.m_DamageValue = DamageValue;
		}

		if( AttackedTarget.m_DamageValue>0)
		{
			INT32 nNewValue = 0;

			pCreature->AddCrtPropNum(enCrtProp_Blood,- AttackedTarget.m_DamageValue,&nNewValue);

			pSourceFighter->AddDamageValueToBoss(AttackedTarget.m_DamageValue);

			if(nNewValue==0)
			{
				AttackedTarget.m_bDie = true;
				if(AttackedCnt.m_SuckBloodFactor < 0)
				{
					AttackedCnt.m_SuckBloodFactor = 0; //死亡了，不触发返伤
				}
				else
				{
					bDecreaseCount = true;
				}
			}
			else if(AttackedCnt.m_AddBloodFactor>0) //回血
			{
				AttackedTarget.m_AddBloodValue =  AttackedTarget.m_DamageValue * AttackedCnt.m_AddBloodFactor +0.99999;
				pCreature->AddCrtPropNum(enCrtProp_Blood,AttackedTarget.m_AddBloodValue,&nNewValue);

				bDecreaseCount = true;
			}
		}
	}

	AddBloodValue =  AttackedTarget.m_DamageValue * AttackedCnt.m_SuckBloodFactor +0.99999;

	if(AddBloodValue != 0)
	{
		//加血
		INT32 nNewValue = 0;
		pSource->AddCrtPropNum(enCrtProp_Blood,AddBloodValue,&nNewValue);
	}


	//效果是一次性的，需要移除
	if(pMagicCnfg->m_bPassive==false)
	{
		for(int i=0; i<m_pMagicLevelCnfg->m_vectEffect.size();i++)
		{
			pStatusPart->RemoveEffect(m_pMagicLevelCnfg->m_vectEffect[i]);
		}
	}


	//需要判断目标是否已死亡，如已死亡不需要加状态了
	if(AttackedTarget.m_bDie)
	{
		return true;
	}

	SCombatAction CombatAction ;
	CombatAction.m_ActionType = enActionType_AddStatus;

	SActionStatusInfo StatusInfo;
	StatusInfo.m_uidCreature = pCreature->GetUID();

	//状态持续伤害基础值
	INT32 BasePersistDamage = this->GetBasePersistDamage(pSource,pCreature);

	//增加状态
	for(int i=0; i<m_pMagicLevelCnfg->m_vectTargetStatus.size();i++)
	{
		pCreatureFighter->AddStatus(m_pMagicLevelCnfg->m_vectTargetStatus[i],GetMagicID(),BasePersistDamage);	

	}

	return true;
}

//获取持续伤害基本值pSource攻方,pCreature守方
INT32 Magic::GetBasePersistDamage(ICreature * pSource,ICreature * pCreature)
{
	//每一次受到法术伤害数值=『攻方[法术伤害] ×（1--免伤值）』
	//已方
	INT32 SelfDamage = this->GetActorMagicDamage(pSource);

	//免伤值
	float AvoidDamage = this->CalculateAvoidDamageValue(pSource,pCreature);

	INT32 BaseValue = SelfDamage * (1-AvoidDamage);

	return BaseValue;
}

//计算范围参数
float Magic::CalculateScopeParam(INT32 TargetNum)
{
	//范围参数=(10--计算人数)/10
	//当计算人数=1，则法术命中范围参数=1

	float fScopeParam = 1.0;
	if(TargetNum >1)
	{
		fScopeParam = (float)(10-TargetNum)/10;
	}

	return fScopeParam;

}

//获取状态回合参数pSource攻方,pCreature守方
float Magic::GetStatusRoundParam(ICreature * pSource,ICreature * pCreature)
{
	//回合参数=法术伤害（施法方）/法术伤害（承受方）

	//已方
	INT32 SelfDamage = this->GetActorMagicDamage(pSource);

	//守方
	INT32 EnemyDamage = this->GetActorMagicDamage(pCreature);

	if(EnemyDamage<1)
	{
		EnemyDamage = 1;
	}

	float statusparam = (float)SelfDamage/EnemyDamage;

	if(statusparam>1.0)
	{
		statusparam = 1.0;
	}

	return statusparam;

}

//获得生物血量上限
INT32 Magic::GetBloodUp(ICreature * pCreature)
{
	INT32 BloodUp = 0;

	enThing_Class ThingClass = pCreature->GetThingClass();

	if(ThingClass == enThing_Class_Actor)
	{
		BloodUp = pCreature->GetCrtProp(enCrtProp_ActorBloodUp);
	}
	else if(ThingClass == enThing_Class_Monster)
	{
		TMonsterID MonsterID = pCreature->GetCrtProp(enCrtProp_MonsterID);

		const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(MonsterID);

		if(pMonsterCnfg != 0)
		{
			BloodUp = pMonsterCnfg->m_Blood;
		}
	}

	return BloodUp;
}

//获取攻击目标
std::vector<IFighter*> Magic::GetAttackedTarget(const SMagicCnfg * pMagicCnfg,
												std::vector<IFighter*> & vectCreature,INT32 FirstIndex)
{
	if(vectCreature.size()<2 || pMagicCnfg->m_AffectRange == enMagicAffectRange_All)
	{
		return vectCreature;
	}

	std::vector<IFighter*>  vect;

	//根据效果类型区分
	if(pMagicCnfg->m_EffectType == enMagicType_Cure)  //治疗
	{
		//优先选择血量低于50%
		std::vector<IFighter*> vectHalfBlood;

		//血量大于50%的玩家
		std::vector<IFighter*> vectGreaterHalfBlood;

		//施放者是否半血
		bool bSourceHalf = false;

		for(int i=0; i<vectCreature.size();i++)
		{
			//当前血量
			INT32 nCurBlood = vectCreature[i]->GetCreature()->GetCrtProp(enCrtProp_Blood);
			INT32 BloodUp = GetBloodUp(vectCreature[i]->GetCreature());
			if(BloodUp && nCurBlood < BloodUp/2)
			{				
				if(FirstIndex == i)
				{
					bSourceHalf = true;
				}
				else
				{
					vectHalfBlood.push_back(vectCreature[i]);
				}
			}
			else
			{
				if(FirstIndex != i)
				{
					vectGreaterHalfBlood.push_back(vectCreature[i]);
				}	

			}
		}

		//选择的目标数必须低于当前存活的目标

		INT16 TargetNum = 1;

		//目标范围
		if(pMagicCnfg->m_AffectRange != enMagicAffectRange_Single)
		{	
			//群体治疗最大目标数
			INT16 MaxGroupTargetNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGroupCureTargetNum;
			if(MaxGroupTargetNum>1)
			{		
				//首先随机肯定目标数
				TargetNum = RandomService::GetRandom() % MaxGroupTargetNum +1;
				if(TargetNum >= vectCreature.size())
				{
					TargetNum = vectCreature.size()-1;
				}
			}				
		}		



		//优先选择血量少的玩家

		//优先选择施放法术的角色(自己)
		if(bSourceHalf)
		{
			vect.push_back(vectCreature[FirstIndex]);
		}

		if(vect.size()+vectHalfBlood.size()<=TargetNum)
		{
			vect.insert(vect.end(),vectHalfBlood.begin(),vectHalfBlood.end());
		}
		else
		{
			for(int i =vect.size();i<TargetNum;i++)
			{
				//随机取一个
				INT32 nRandom = RandomService::GetRandom() % vectHalfBlood.size();
				vect.push_back(vectHalfBlood[nRandom]);
				vectHalfBlood.erase(vectHalfBlood.begin()+nRandom);
			}
		}

		//自己没有在其中，优先选择施放法术的角色(自己)
		if(bSourceHalf==false && vect.size()<TargetNum)
		{
			vect.push_back(vectCreature[FirstIndex]);
		}

		//从vectGreaterHalfBlood中随机选
		for(int i=vect.size(); i<TargetNum && vectGreaterHalfBlood.size()>0;i++)
		{
			INT32 nRandom = RandomService::GetRandom() % vectGreaterHalfBlood.size();
			vect.push_back(vectGreaterHalfBlood[nRandom]);
			vectGreaterHalfBlood.erase(vectGreaterHalfBlood.begin()+nRandom);
		}

	}
	else
	{
		//目标范围
		if(pMagicCnfg->m_AffectRange == enMagicAffectRange_Single)
		{
			vect.push_back(vectCreature[FirstIndex]);
		}		
		else
		{
			//群体攻击最大目标数
			INT16 MaxGroupTargetNum = 0;

			//区分NPC和玩家
			if(vectCreature[0]->GetCreature()->GetThingClass() == enThing_Class_Monster)
			{
				MaxGroupTargetNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxGroupAttackNpcTargetNum;
			}
			else 
			{
				//还需要区分是否带状态
				if(m_pMagicLevelCnfg->m_vectTargetStatus.size()>0)
				{
					MaxGroupTargetNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxNoStatusGroupAttackTargetNum;
				}
				else
				{
					MaxGroupTargetNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxStatusGroupAttackTargetNum;
				}
			}

			//首先随机肯定目标数
			INT32 TargetNum = RandomService::GetRandom() % MaxGroupTargetNum +1;
			if(TargetNum >= vectCreature.size())
			{
				TargetNum = vectCreature.size()-1;
			}

			std::vector<IFighter*>  vectTemp = vectCreature;
			for(int i=0; i<vectTemp.size() && i<TargetNum; ++i)
			{
				INT32 nRandom = RandomService::GetRandom() % vectTemp.size();
				vect.push_back(vectTemp[nRandom]);
				vectTemp.erase(vectTemp.begin()+nRandom);
			}

		}	
	}

	return vect;
}

std::vector<IFighter*> Magic::GetAttackedTarget(const SMagicCnfg * pMagicCnfg,
												std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
												std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex)
{	
	if(pMagicCnfg->m_Target == enMagicTarget_Enemy)
	{
		//对敌
		return GetAttackedTarget(pMagicCnfg,vectEnemy,EnemyIndex);

	}
	else
	{		
		return GetAttackedTarget(pMagicCnfg,vectSelf,SourceIndex);
	}	
}

//使用攻击型技能
bool  Magic::UseSkillWithAttack(UINT64 CombatID,INT32 RoundNum,const SMagicCnfg * pMagicCnfg ,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
								std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
								OBuffer4k & ob,INT32 & RecordNum)
{
	std::vector<IFighter*>  VectTarget = GetAttackedTarget(pMagicCnfg,vectSelf,SourceIndex,vectEnemy,EnemyIndex);

	float fScopeParam = this->CalculateScopeParam(VectTarget.size());

	//源
	ICreature * pSource = vectSelf[SourceIndex]->GetCreature();

	IFighter* pSourceFighter = vectSelf[SourceIndex];



	SCombatAction CombatAction ;
	CombatAction.m_ActionType = enActionType_Attack;

	SActionAttackInfo AttackInfo;

	AttackInfo.m_uidSource = pSource->GetUID();
	AttackInfo.m_AttackType = enAttackType_Magic;
	AttackInfo.m_MagicID = GetMagicID();
	AttackInfo.m_MagicResID = pMagicCnfg->m_DongHuaID;

	// fly add	20121106
	strncpy(AttackInfo.m_szMagicName,g_pGameServer->GetGameWorld()->GetLanguageStr(pMagicCnfg->m_MagicLangID),ARRAY_SIZE(AttackInfo.m_szMagicName)-2);
	AttackInfo.m_TargetNum = 0;
	INT32 TatolAddBloodValue = 0;


	//中状态
	OBuffer4k obStauts;

	OBuffer4k obTarget;


	TRACE_COMBAT("指令 %d : 玩家[%s:%s] 对 玩家",RecordNum+1,pSource->GetName(),pSource->GetUID().ToString());


	SMyCampAttackedContext MyCampAttackedCnt;
	MyCampAttackedCnt.m_uidSource = pSource->GetUID();

	std::vector<IFighter*> vectDie;


	for(int i=0; i<VectTarget.size(); ++i )
	{		
		SAttackedTarget AttackedTarget;
		INT32  AddBloodValue = 0;

		//是否需要减状态的生效次数
		bool bDecreaseCount = false;

		if(UseSkillToSingleObjectWithAttack(RoundNum,pMagicCnfg,pSourceFighter,fScopeParam,VectTarget[i],AttackedTarget,AddBloodValue,obStauts,RecordNum, bDecreaseCount))
		{
			AttackInfo.m_AddBloodValue += AddBloodValue;

			obTarget << AttackedTarget;
			AttackInfo.m_TargetNum++;

			TRACE_COMBAT("[%s:%s] 进行攻击，目标命中[%d]  造成伤害 %d 目标是否死亡[%d],自身加血[%d]",VectTarget[i]->GetCreature()->GetName(),VectTarget[i]->GetUID().ToString()
				,AttackedTarget.m_bHit,AttackedTarget.m_DamageValue,AttackedTarget.m_bDie,AddBloodValue);


			if(AttackedTarget.m_bHit && bDecreaseCount)
			{
				MyCampAttackedCnt.m_uidTarget.push_back(VectTarget[i]->GetUID());
			}

			if(AttackedTarget.m_bDie)
			{

				vectDie.push_back(VectTarget[i]);
			}

		}

		if(pSource->GetCrtProp(enCrtProp_Blood)==0)
		{
			AttackInfo.m_bDie = true;

			vectDie.push_back(pSourceFighter);
			break;
		}			
	}


	TRACE_COMBAT("\n");

	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = CombatID;
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	RecordNum++;

	OBuffer4k obTemp;

	obTemp << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(AttackInfo)+obTarget.Size()) << CombatCombat << CombatAction << AttackInfo;
	obTemp.Push(obTarget.Buffer(),obTarget.Size());

	pSourceFighter->Broadcast(obTemp);

	if(MyCampAttackedCnt.m_uidTarget.empty()==false)
	{

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_MyCampAttacked);

		for(int j=0; j<vectEnemy.size();j++)
		{
			MyCampAttackedCnt.m_vectRemoveStatus.clear();

			vectEnemy[j]->GetCreature()->OnEvent(msgID,&MyCampAttackedCnt,SIZE_OF(MyCampAttackedCnt));
		}	
	}

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat);

	for(int i=0; i<vectDie.size(); i++)
	{
		IFighter * pFighter = vectDie[i];

		SS_DieAtCombat DieAtCombat;
		DieAtCombat.m_uidActor = pFighter->GetUID();
		pFighter->GetCreature()->OnEvent(msgID,&DieAtCombat,sizeof(DieAtCombat));
	}

	return true;
}

//使用治疗型技能
bool  Magic::UseSkillWithCure(UINT64 CombatID,INT32 RoundNum,const SMagicCnfg * pMagicCnfg ,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
							  std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
							  OBuffer4k & ob,INT32 & RecordNum)
{
	std::vector<IFighter*>  VectTarget = GetAttackedTarget(pMagicCnfg,vectSelf,SourceIndex,vectEnemy,EnemyIndex);

	float fScopeParam = this->CalculateScopeParam(VectTarget.size());
	//源
	ICreature * pSource = vectSelf[SourceIndex]->GetCreature();

	IFighter * pSourceFighter = vectSelf[SourceIndex];

	OBuffer4k obTarget;

	SCombatAction CombatAction ;
	CombatAction.m_ActionType = enActionType_AddBlood;

	SActionAddBloodInfo BloodInfo;
	BloodInfo.m_TargetNum = 0; 
	BloodInfo.m_uidSource = pSource->GetUID();
	BloodInfo.m_MagicID = GetMagicID();
	BloodInfo.m_MagicResID = pMagicCnfg->m_DongHuaID;

	// fly add	20121106
	strncpy(BloodInfo.m_szMagicName,g_pGameServer->GetGameWorld()->GetLanguageStr(pMagicCnfg->m_MagicLangID),ARRAY_SIZE(BloodInfo.m_szMagicName)-2);
	//strncpy(BloodInfo.m_szMagicName,pMagicCnfg->m_strName.c_str(),ARRAY_SIZE(BloodInfo.m_szMagicName)-2);

	TRACE_COMBAT("指令 %d : 玩家[%s:%s] 对 玩家",RecordNum+1,pSource->GetName(),pSource->GetUID().ToString());


	//中状态
	OBuffer4k obStauts;


	for(int i=0; i<VectTarget.size(); i++)
	{		
		SAddBloodTarget BloodTarget ;

		if(UseSkillToSingleObjectWithCure(RoundNum,pMagicCnfg,pSourceFighter,fScopeParam,VectTarget[i],BloodTarget,obStauts,RecordNum))
		{
			obTarget << BloodTarget;
			BloodInfo.m_TargetNum++;


			TRACE_COMBAT("[%s:%s]进行治疗，加血 %d ",VectTarget[i]->GetCreature()->GetName(),VectTarget[i]->GetUID().ToString(),BloodTarget.m_BloodValue);

		}
	}


	TRACE_COMBAT("\n");

	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = CombatID;
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	RecordNum++;

	OBuffer4k Tempob;
	Tempob << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(BloodInfo)+obTarget.Size()) << CombatCombat << CombatAction << BloodInfo;

	Tempob.Push(obTarget.Buffer(),obTarget.Size());

	pSourceFighter->Broadcast(Tempob);



	/*	if(obStauts.Size()>0)
	{
	ob.Push(obStauts.Buffer(),obStauts.Size());
	}*/

	return true;
}





bool  Magic::UseSkill(UINT64 CombatID,INT32 RoundNum,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
					  std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
					  OBuffer4k & ob,INT32 & RecordNum)
{
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(m_pMagicLevelCnfg->m_MagicID);
	if(pMagicCnfg == 0)
	{
		return false;
	}

	if(pMagicCnfg->m_EffectType == enMagicType_Cure)
	{
		return UseSkillWithCure(CombatID,RoundNum,pMagicCnfg,vectSelf,SourceIndex,vectEnemy,EnemyIndex,ob,RecordNum);
	}
	else
	{
		return UseSkillWithAttack(CombatID,RoundNum,pMagicCnfg,vectSelf,SourceIndex  ,vectEnemy,EnemyIndex ,ob,RecordNum);
	}


	return true;
}

//升级
bool  Magic::Upgrade() 
{
	const SMagicLevelCnfg * pMagicLevelCnfg = g_pGameServer->GetConfigServer()->GetMagicLevelCnfg(m_pMagicLevelCnfg->m_MagicID,m_pMagicLevelCnfg->m_Level+1);
	if(pMagicLevelCnfg == 0)
	{
		return false;
	}

	m_pMagicLevelCnfg = pMagicLevelCnfg;

	return true;
}

//设置等级
bool  Magic::SetLevel(UINT8 level)
{
	const SMagicLevelCnfg * pMagicLevelCnfg = g_pGameServer->GetConfigServer()->GetMagicLevelCnfg(m_pMagicLevelCnfg->m_MagicID,level);
	if(pMagicLevelCnfg == 0)
	{
		return false;
	}

	m_pMagicLevelCnfg = pMagicLevelCnfg;

	return true;
}

const SMagicCnfg * Magic::GetMagicCnfg()
{
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(m_pMagicLevelCnfg->m_MagicID);
	return pMagicCnfg;
}

//获得CD时间
INT32 Magic::GetCDTime()
{
	const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(m_pMagicLevelCnfg->m_MagicID);
	return pMagicCnfg->m_CDTime;
}


//计算免伤
float  Magic::CalculateAvoidDamageValue(ICreature * pSource,ICreature * pEnemy)
{
	//法术攻击免伤值=『守方[防御]*C/（守方[防御]*C+D）』
	//C=0.02、D=75
	//最大值80%

	//守方[防御]
	INT32 EnemyDefense  = GetDefenseValue(pEnemy);

	//攻方[等级]

	INT32 Level = pSource->GetCrtProp(enCrtProp_Level);

	const SGameServerConfigParam   & ServerConfigParam  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	float value = (float)EnemyDefense*ServerConfigParam.m_MagicAvoidDamageFactor1/(EnemyDefense*ServerConfigParam.m_MagicAvoidDamageFactor1 + ServerConfigParam.m_MagicAvoidDamageFactor2);

	if(value>0.80)
	{
		value = 0.80;
	}

	return value;
}
