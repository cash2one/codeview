
#include "Combat.h"
#include "IActor.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "ICombatPart.h"
#include "IMonster.h"
#include "CombatCmd.h"
#include "ICombatServer.h"
#include "IStatusPart.h"
#include "RandomService.h"
#include "IGameScene.h"
#include "IMagicPart.h"
#include "IMagic.h"
#include "IEquipPart.h"
#include "XDateTime.h"
#include "IGoodsServer.h"
#include "ITeamPart.h"

Fighter::Fighter(Combat * pCombat,ICreature *  pCreature,UINT8 nPos,bool bAutoFighte,bool bNeedSync,bool bOneSelf)
{
	m_pCombat = pCombat;
	if(pCreature->GetThingClass()==enThing_Class_Actor)
	{
		m_pCreature = ((IActor*)pCreature)->MakeCloner();
		m_bNeedDeleteCreature = true;
	}
	else
	{
		m_pCreature = pCreature;  //生物
		m_bNeedDeleteCreature = false;
	}
	m_Pos = nPos;         //生物站位信息
	m_bAutoFighte = bAutoFighte; //是否自动战斗
	m_NextMagicTime = 0; //下一次放法术时间
	memset(m_MagicList,0,MEM_SIZE(m_MagicList)); //该生物可以施放的法术
	m_CurMagicIndex = 0; //当前施放的法术索引
	m_bNeedSync = bNeedSync;  //是否需要同步战斗信息

	m_bOneSelf = bOneSelf;

	m_GiveExp = 0;	

	m_DamageValueToBoss = 0;

	m_pMasterFighter = 0;

	m_CurAckNo = 0;

	for(int i=0; i<ARRAY_SIZE(m_GoldSwordWuXing);i++)
	{
		m_GoldSwordWuXing[i] = enWuXing_Max;
	}
}

Fighter::~Fighter()
{	
	if(m_bNeedDeleteCreature)
	{
		m_pCreature->Release();
	}
}

void Fighter::Start()
{
	if(m_bAutoFighte)
	{
		StartMagicTimer();
	}
	else
	{
		for(; m_CurMagicIndex<ARRAY_SIZE(m_MagicList);m_CurMagicIndex++)
		{
			UpdateCDTime();
			if(GetMagic(m_CurMagicIndex) != NULL)
			{
				break;
			}
		}
	}
}

void Fighter::Stop()
{
	if(m_bAutoFighte)
	{
		g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Magic,this);
	}


}

std::vector<TGoodsID> Fighter::GiveUserDropGoods(UINT16 DropID)
{
	std::vector<TGoodsID> vectGoods;

	if(m_pCreature->GetThingClass() == enThing_Class_Actor)
	{
		g_pGameServer->GetGoodsServer()->GiveUserDropGoods((IActor*)m_pCreature,DropID,vectGoods);

		if(vectGoods.size()>0)
		{

			for(int i=0; i<vectGoods.size();i++)
			{					
				((IActor*)m_pCreature)->SendGoodsCnfg(vectGoods[i]);				


			}			
		}			
	}

	return vectGoods;


}


//发送数据
bool Fighter::SendToClient(OStreamBuffer & osb)
{
	if(m_bNeedSync==false || m_pCreature->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	IActor * pActor = (IActor*)g_pGameServer->GetGameWorld()->FindActor(m_pCreature->GetUID());

	if(pActor == 0)
	{
		return false;
	}

	return pActor->SendData(osb);
}

void Fighter::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTimerID_Magic:
		{
			g_pGameServer->GetTimeAxis()->KillTimer(enTimerID_Magic,this);
			AutotMagic();
		}
		break;
	default:
		break;
	}
}

//是否可以发起物理攻击
bool Fighter::IsLaunchPhysicsAttacke()
{
	SAttackContext AttackCnt;
	AttackCnt.m_uidSource =  GetUID();
	AttackCnt.m_BaseAttackValue =0;	
	AttackCnt.m_AttackType = enAttackType_Physics;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack);

	//投票
	if(m_pCreature->FireVote(msgID,&AttackCnt,sizeof(AttackCnt))==false)
	{
		return false;
	}

	return true;
}


//获得爆击值
INT32 Fighter::GetCriticalHitValue()
{
	INT32 Value = 0;
	Value = m_pCreature->GetCrtProp(enCrtProp_ActorCrit);
	return Value;
}

//获得坚韧值
INT32 Fighter::GetTenacityValue() {
	INT32 Value = 0;

	Value = m_pCreature->GetCrtProp(enCrtProp_ActorTenacity);
	return Value;
}

//获得命中值
INT32 Fighter::GetHitValue() {
	INT32 Value = 0;

	Value = m_pCreature->GetCrtProp(enCrtProp_ActorHit);

	return Value;
}

//获得回避值
INT32 Fighter::GetDodgeValue() {
	INT32 Value = 0;

	Value = m_pCreature->GetCrtProp(enCrtProp_ActorDodge);

	return Value;
}
//获得法术回复值
INT32 Fighter::GetMagicCDReduceValue(){
	INT32 Value = 0;

	Value = m_pCreature->GetCrtProp(enCrtProp_MagicCD);

	Value = (INT32)((Value/(float)5000+0.0999999)*1000)/100*100;

	return Value;
}

//获得仙剑五行
enWuXing Fighter::GetGoldSwordWuXing(INT32 Index)
{
	return  m_GoldSwordWuXing[Index % MAX_LOAD_GODSWORD_NUM];
}

//设置仙剑五行
void Fighter::SetGoldSwordWuXing(INT32 Index,enWuXing WuXing)
{
	m_GoldSwordWuXing[Index % MAX_LOAD_GODSWORD_NUM] = WuXing;
}

//判断是否物理攻击命中
bool Fighter::IsPhysicsAttackHit(Fighter * pEnemy)
{

	//物理命中系数=【攻方[身法]/(攻方[身法]+目标[身法])+0.3+物理命中回避参数】×攻方状态参数×目标状态参数
	//如果[物理命中系数]>rand()，则判定命中。
	//如果[物理命中系数]<rand()，则判定为未命中。
	//(rand()取0-1之间的随机数，精确到0.0001)



	//攻方身法
	INT32 SelfAvoid = m_pCreature->GetCrtProp(enCrtProp_Avoid);

	//守方身法
	INT32 EnemyAvoid = pEnemy->GetCreature()->GetCrtProp(enCrtProp_Avoid);

	if(EnemyAvoid==0)
	{
		EnemyAvoid = 1;
	}

	//攻方状态参数 单位:千分之
	INT32 AttackStatusParam = 1000;

	const SStatusTypeCnfg * pTypeCnfg = GetMaxPriorityStatusTypeCnfg();

	if(pTypeCnfg != 0)
	{
		AttackStatusParam = pTypeCnfg->m_PhysicsHitAttackStatusParam;
	}

	//守方
	//守方状态参数 单位:千分之
	INT32 AttackedStatusParam = 1000;

	const SStatusTypeCnfg * pAttackedTypeCnfg = pEnemy->GetMaxPriorityStatusTypeCnfg();

	if(pAttackedTypeCnfg != 0)
	{
		AttackedStatusParam = pAttackedTypeCnfg->m_PhysicsHitAttackedStatusParam;
	}

	//命中回避参数
	float HitAndAvoidParam = this->CalculatePhysicsHitAndAvoidParam(pEnemy);

	//命中系数
	INT32 PhysicsHitFactor = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_PhysicsHitFactor;

	float PhysicsAttackHitParam = ((float)SelfAvoid/(SelfAvoid+EnemyAvoid)+ PhysicsHitFactor/(float)1000 + HitAndAvoidParam)*AttackStatusParam*AttackedStatusParam/1000/1000;

	//随机数
	INT32 RandomNum = RandomService::GetRandom()%10000;

	if(PhysicsAttackHitParam*10000 < RandomNum)
	{
		return false; 
	}

	return true;
}

//获得生物所中状态中优先级最高的状态的类型信息
const SStatusTypeCnfg * Fighter::GetMaxPriorityStatusTypeCnfg()
{
	IStatusPart * pStatusPart = (IStatusPart *)m_pCreature->GetPart(enThingPart_Crt_Status);

	if(pStatusPart==0)
	{
		return 0;
	}

	return pStatusPart->GetMaxPriorityStatusTypeCnfg() ;	
}

//获得物理爆击参数
float Fighter::GetPhysicsAttackCriticalHitParam(Fighter * pEnemy)
{

	//物理爆击率=（0.05+攻方[爆击]/25000-守方[坚韧]/30000）×攻方状态参数×目标状态参数

	//攻方[爆击]
	INT32 OneselfCriticalHitValue = this->GetCriticalHitValue();

	//守方[坚韧]
	INT32 EnemyTenacityValue = pEnemy->GetTenacityValue();

	//攻方状态
	const SStatusTypeCnfg * pTypeCnfg = GetMaxPriorityStatusTypeCnfg();

	//物理爆击状态参数
	INT32 PhysicsKnockingAttackStatusParam = 1000;
	if(pTypeCnfg !=0)
	{
		PhysicsKnockingAttackStatusParam = pTypeCnfg->m_PhysicsKnockingAttackStatusParam;
	}

	//守方状态

	INT32 PhysicsKnockingAttackedStatusParam = 1000;
	const SStatusTypeCnfg * pAttackedTypeCnfg = pEnemy->GetMaxPriorityStatusTypeCnfg();

	if(pAttackedTypeCnfg != 0)
	{
		PhysicsKnockingAttackedStatusParam = pAttackedTypeCnfg->m_PhysicsKnockingAttackedStatusParam;
	}



	float PhysicsAttackCriticalHitParam = (float)(0.05+OneselfCriticalHitValue/(float)25000-EnemyTenacityValue/(float)30000)*1000*PhysicsKnockingAttackStatusParam*PhysicsKnockingAttackedStatusParam/1000/1000;

	//随机数
	INT32 RandomNum = RandomService::GetRandom()%1000;

	if ( RandomNum < 0 || RandomNum >= 1000 )
	{
		TRACE("<error> %s : %d Line 获取随机值出错！！对1000取随机取到%d", __FUNCTION__, __LINE__, RandomNum);
	}

	//是否爆击
	if(RandomNum > PhysicsAttackCriticalHitParam)
	{
		PhysicsAttackCriticalHitParam = 0;
	}
	else
	{
		const SGameConfigParam & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

		//按概率选取
		static INT32 TotalProb = 0;  //总概率
		if(TotalProb == 0)
		{
			for(int i=0; i<ConfigParam.m_PhysicsKnockingParamAndProb.size()/2; i++)
			{
				TotalProb += ConfigParam.m_PhysicsKnockingParamAndProb[i*2+1];
			}
		}

		if(TotalProb == 0)
		{
			return 0;
		}

		//随机数
		INT32 RandomNum = RandomService::GetRandom()%TotalProb;

		for(int i=0; i<ConfigParam.m_PhysicsKnockingParamAndProb.size()/2; i++)
		{
			if(RandomNum<ConfigParam.m_PhysicsKnockingParamAndProb[i*2+1])
			{
				PhysicsAttackCriticalHitParam = (float)ConfigParam.m_PhysicsKnockingParamAndProb[i*2] / 1000;
				break;
			}

			RandomNum -= ConfigParam.m_PhysicsKnockingParamAndProb[i*2+1];
		}
	}

	return PhysicsAttackCriticalHitParam;
}

//获得仙剑伤害
INT32  Fighter::GetGodSwordPhysicsDamage()
{
	INT32 PhysicsDamage = 0;

	if(m_pCreature->GetThingClass() == enThing_Class_Actor)
	{		
		PhysicsDamage = m_pCreature->GetCrtProp(enCrtProp_ActorPhysics);
	}
	else if(m_pCreature->GetThingClass() == enThing_Class_Monster)
	{
		IMonster * pMonster = (IMonster*)m_pCreature;
		PhysicsDamage = pMonster->GetPhysicalDamage();
	}

	return PhysicsDamage;
}


//获得物理攻击伤害值
INT32 Fighter::GetPhysicsAttackValue(Fighter * pEnemy,float CriticalHitParam)
{
	//不爆击时:
	//物理伤害数值=『攻方[物理伤害]×（1--免伤值）』×攻方状态参数×目标状态参数×随机数

	//爆击时:
	//爆击伤害值=不爆击时物理伤害值×参数×【1-守方[坚韧]/ （守方[坚韧]+30000）】

	//随机数随机取0.97至1.03之间的数，准确到小数点第2位

	float fRomdan = ((970+RandomService::GetRandom()%(1030-970)+5))/10/(float)100;



	//攻方
	const SStatusTypeCnfg * pTypeCnfg = GetMaxPriorityStatusTypeCnfg();

	//物理伤害状态参数
	INT32 PhysicsDamageAttackStatusParam = 1000;
	if(pTypeCnfg !=0)
	{
		PhysicsDamageAttackStatusParam = pTypeCnfg->m_PhysicsDamageAttackStatusParam;
	}

	//守方

	INT32 PhysicsDamageAttackedStatusParam = 1000;
	const SStatusTypeCnfg * pAttackedTypeCnfg = pEnemy->GetMaxPriorityStatusTypeCnfg();

	if(pAttackedTypeCnfg != 0)
	{
		PhysicsDamageAttackedStatusParam = pAttackedTypeCnfg->m_PhysicsDamageAttackedStatusParam;
	}

	//免伤值
	float AvoidDamageValue = CalculateAvoidDamageValue(pEnemy);

	//攻方物理伤害
	INT32 GodSwordDamageValue = GetGodSwordPhysicsDamage();

	//攻击值
	float AttackValue = (GodSwordDamageValue*(1-AvoidDamageValue))*(float)PhysicsDamageAttackStatusParam/1000*PhysicsDamageAttackedStatusParam/1000 * fRomdan*g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_PhysicsAttackFactor/1000;

	if(CriticalHitParam>1.0)
	{
		//爆击了

		//守方[坚韧]
		INT32 EnemyTenacityValue = pEnemy->GetTenacityValue();

		AttackValue = AttackValue*CriticalHitParam*(1-(float)EnemyTenacityValue/(EnemyTenacityValue+30000));

	}

	return AttackValue+0.999999;	
}

void Fighter::AddDamageValueToBoss(INT32 Value)
{ 
	if(m_pMasterFighter==0)
	{
		m_DamageValueToBoss += Value;
	}
	else
	{
		m_pMasterFighter->AddDamageValueToBoss(Value);
	}
}



//是否已死
bool Fighter::IsDie()
{
	return m_pCreature->GetCrtProp(enCrtProp_Blood) == 0;
}

//是否可以施放法术
bool Fighter::CanUseMagic()
{
	IMagic * pMagic = GetMagic(m_CurMagicIndex);

	if(pMagic==NULL)
	{
		return false;
	}

	if(pMagic->CanUseSkill(this->GetCreature(),UID())==false)
	{
		return false;
	}


	return true;
}
//自动施放法术
void Fighter::AutotMagic()
{
	if(this->IsAutoFighte()==false || IsDie())
	{
		return;
	}

	IMagic * pMagic = 0;
	if(CanUseMagic()==true)
	{
		pMagic = GetMagic(m_CurMagicIndex);

		INT32 Index = m_CurMagicIndex;

		m_CurMagicIndex = (m_CurMagicIndex+1)%ARRAY_SIZE(m_MagicList);

		UpdateCDTime();

		StartMagicTimer();

		m_pCombat->UseMagic(this,pMagic,Index);
		return;
	}

	m_CurMagicIndex = (m_CurMagicIndex+1)%ARRAY_SIZE(m_MagicList);

	UpdateCDTime();

	StartMagicTimer();
}

//手动施放法术
void Fighter::ManualMagic(IActor * pActor,TMagicID MagicID)
{
	SC_CombatFireMagic_Rsp Rsp;
	Rsp.m_MagicID = MagicID;
	Rsp.m_uidActor =GetUID();
	Rsp.m_Result = enCombatCode_OK;

	INT32 MagicIndex = 0;

	TRACE_COMBAT("玩家[%s]请求施放法术[%d]",m_pCreature->GetName(),MagicID);

	do
	{
		if(IsDie())
		{

			Rsp.m_Result = enCombatCode_ErrDieCantMagic;	
			break;
		}

		if(IsAutoFighte())
		{
			Rsp.m_Result = enCombatCode_ErrAutoMode;
			break;
		}

		IMagic * pMagic = GetMagic(m_CurMagicIndex);

		if(pMagic==0)
		{
			Rsp.m_Result = enCombatCode_ErrNoMagic;
			break;
		}

		if(pMagic->GetMagicID() != MagicID)
		{
			Rsp.m_Result = enCombatCode_ErrOrderMagic;
			break;
		}

		if(this->NextMagicTime()>CURRENT_TIME()*1000)
		{
			Rsp.m_Result = enCombatCode_ErrCDTime;
			break;
		}

		if(pMagic->CanUseSkill(this->GetCreature(),UID())==false)
		{
			Rsp.m_Result = enCombatCode_ForbidMagic;
			break;
		}

		MagicIndex = m_CurMagicIndex;

		do
		{
			m_CurMagicIndex = (m_CurMagicIndex+1)%ARRAY_SIZE(m_MagicList);
			UpdateCDTime();
		}while(GetMagic(m_CurMagicIndex) == 0);

		OBuffer4k ob;
		SCombatCombat CombatCombat;
		CombatCombat.m_CombatID = m_pCombat->GetCombatID();
		CombatCombat.m_SubCmd = enCombatSubCmd_CS_FireMagic;


		ob << SCombatHeader(enCombatCmd_Combat,sizeof(SCombatCombat)+sizeof(Rsp)) << CombatCombat << Rsp;

		pActor->SendData(ob.TakeOsb());

		if(Rsp.m_Result == enCombatCode_OK)
		{
			TRACE_COMBAT("玩家[%s]手动施放法术[%d]成功!",m_pCreature->GetName(),MagicID);
		}
		else
		{
			TRACE_COMBAT("玩家[%s]手动施放法术[%d]失败!",m_pCreature->GetName(),MagicID);
		}

		m_pCombat->UseMagic(this,pMagic,MagicIndex);

		return;



	} while(false);

	OBuffer4k ob;
	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = m_pCombat->GetCombatID();
	CombatCombat.m_SubCmd = enCombatSubCmd_CS_FireMagic;


	ob << SCombatHeader(enCombatCmd_Combat,sizeof(SCombatCombat)+sizeof(Rsp)) << CombatCombat << Rsp;

	pActor->SendData(ob.TakeOsb());
}

//计算免伤
float  Fighter::CalculateAvoidDamageValue(Fighter * pEnemy)
{
	//物理攻击免伤值=『守方[防御]*A/（守方[防御]*A+B）』
	//A=0.06、B=150
	//最大值85%

	//守方[防御]
	INT32 EnemyDefense  = pEnemy->GetDefenseValue();

	//攻方[等级]

	INT32 Level = m_pCreature->GetCrtProp(enCrtProp_Level);

	const SGameServerConfigParam   & ServerConfigParam  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam;

	float value = (float)EnemyDefense*ServerConfigParam.m_AvoidDamageFactor1/(EnemyDefense*ServerConfigParam.m_AvoidDamageFactor1+ServerConfigParam.m_AvoidDamageFactor2);

	if(value>0.85)
	{
		value = 0.85;
	}

	return value;
}

//获得生物防御值
INT32 Fighter::GetDefenseValue()
{
	INT32 DefenseValue = 0;
	if(m_pCreature->GetThingClass() == enThing_Class_Actor)
	{		
		DefenseValue = m_pCreature->GetCrtProp(enCrtProp_ActorDefend);
	}
	else if(m_pCreature->GetThingClass() == enThing_Class_Monster)
	{
		IMonster * pMonster = (IMonster*)m_pCreature;
		DefenseValue = pMonster->GetDefense();
	}

	return DefenseValue;
}

void Fighter::SetMagic(UINT8 Index,TMagicID MagicID,UINT8 Level)
{
	if(Index>=ARRAY_SIZE(m_MagicList))
	{
		return;
	}

	SCreateMagicCnt  CreateMagicCnt;	
	CreateMagicCnt.m_MagicID = MagicID;
	CreateMagicCnt.m_Level = Level;
	IMagic * pMagic = g_pGameServer->GetCombatServer()->CreateMagic(CreateMagicCnt);
	if(pMagic!=0)
	{
		m_MagicList[Index] = pMagic;
	}
}

UINT64  Fighter::NextMagicTime()
{
	if(m_NextMagicTime==0)
	{
		UpdateCDTime();
	}

	return m_NextMagicTime;
}

//增加状态
void Fighter::OnAddStatus(TStatusID StatusID,TMagicID MagicID,INT32 StatusTime)
{
	const SStatusCnfg * pStatusCnfg = g_pGameServer->GetConfigServer()->GetStatusCnfg(StatusID);
	if(pStatusCnfg ==0)
	{
		return ;
	}

	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = m_pCombat->GetCombatID();
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	SCombatAction CombatAction ;
	CombatAction.m_ActionType = enActionType_AddStatus;

	SActionStatusInfo StatusInfo;
	StatusInfo.m_uidCreature = GetUID();

	StatusInfo.m_StatusType = pStatusCnfg->m_StatusType;

	StatusInfo.m_StatusResID = pStatusCnfg->m_StatusResID;

	StatusInfo.m_StatusTime = StatusTime;

	// fly add	20121106
	strncpy((char*)StatusInfo.m_szStatusName,g_pGameServer->GetGameWorld()->GetLanguageStr(pStatusCnfg->m_StatusNameLangID),ARRAY_SIZE(StatusInfo.m_szStatusName)-1);

	OBuffer4k obStauts;

	obStauts << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(StatusInfo)) << CombatCombat << CombatAction << StatusInfo;

	TRACE_COMBAT("指令 : 玩家[%s:%s]受到攻击增加状态[%s]",m_pCreature->GetName(),m_pCreature->GetUID().ToString(),pStatusCnfg->m_strName.c_str());

	this->Broadcast(obStauts);
}

//增加状态
bool Fighter::AddStatus(TStatusID StatusID,TMagicID MagicID,INT32 BasePersistDamage)
{
	const SStatusCnfg * pStatusCnfg = g_pGameServer->GetConfigServer()->GetStatusCnfg(StatusID);
	if(pStatusCnfg ==0)
	{
		return false;
	}

	IStatusPart * pStatusPart = (IStatusPart *)m_pCreature->GetPart(enThingPart_Crt_Status);

	if(pStatusPart==0)
	{
		return false;
	}

	//移除的状态
	std::vector<UINT8>  vectDelStatusType;

	if(pStatusPart->AddStatus(StatusID,GetUID(), vectDelStatusType, MagicID)==false)
	{
		return false;
	}

	IStatus * pStatus = pStatusPart->FindStatus(StatusID);

	if(pStatus==0)
	{
		return false;
	}

	pStatus->SetStatusData(BasePersistDamage);


	return true;
}

//移除状态
void Fighter::OnRemoveStatus(UINT8 StatusType)
{
	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = m_pCombat->GetCombatID();
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	SCombatAction CombatAction ;

	CombatAction.m_ActionType = enActionType_RemoveStatus;

	SActionRemoveStatus RemoveStatus;

	RemoveStatus.m_uidCreature = GetUID();

	RemoveStatus.m_StatusType = StatusType;

	OBuffer4k ob;

	ob << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(RemoveStatus)) << CombatCombat << CombatAction << RemoveStatus;

	this->Broadcast(ob);

	TRACE_COMBAT("指令 : 玩家[%s] 移除状态 %d \n",GetCreature()->GetName(),StatusType);

	return ;
}

//广播数据
void Fighter::Broadcast(OBuffer4k & ob)
{
	this->m_pCombat->Broadcast(ob);
}

//设置自动模式
void Fighter::SetAutoMagicMode()
{
	if(IsAutoFighte())
	{
		return;
	}

	m_bAutoFighte = true;
	
	if(this->IsDie()==false)
	{
		StartMagicTimer();
	}

	TRACE_COMBAT("玩家：%s[%s]进入自动模式!",m_pCreature->GetName(),m_pCreature->GetUID().ToString());	

}

//断线
void Fighter::OnOffLine()
{
	this->m_bNeedSync = false;

	if(this->m_CurAckNo != 0 && m_pCreature->GetThingClass()==enThing_Class_Actor)
	{

		IActor * pActor = (IActor *)m_pCreature;
		SCombatActionAck ActionAck;
		ActionAck.m_AckNo = this->m_CurAckNo;
		this->m_pCombat->AckAction(pActor,ActionAck);

		IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(pActor->GetUID());

		if ( 0 != pUser )
		{
			pUser->SetIsInCombat(false);
		}
	}
}

//计算物理命中回避参数
float Fighter::CalculatePhysicsHitAndAvoidParam(IFighter * pEnemy)
{
	//命中回避参数=攻方[命中值]/(1+（目标[回避值]+ 攻方[命中值]）*2)
	//该值定义最小值为0


	//攻方[命中值]
	INT32 OneselfHitValue = this->GetHitValue();

	//目标[回避值]
	INT32  EnemyAvoidValue = pEnemy->GetDodgeValue();

	float value = (float)OneselfHitValue/(1+(OneselfHitValue+EnemyAvoidValue)*2);

	return value;
}


//计算法术命中回避参数
float Fighter::CalculateMagicHitAndAvoidParam(IFighter * pEnemy)
{
	//命中回避参数=攻方[命中值]/（1+目标[回避值]+ 攻方[命中值]）
	//该值定义最小值为0


	//攻方[命中值]
	INT32 OneselfHitValue = this->GetHitValue();

	//目标[回避值]
	INT32  EnemyAvoidValue = pEnemy->GetDodgeValue();

	float value = (float)OneselfHitValue/(1+OneselfHitValue+EnemyAvoidValue);

	return value;
}

//计算打怪可以获得的经验
INT32 Fighter::CalculateExp(ICreature * pMonster)
{
	//角色等级
	INT32 ActorLevel = m_pCreature->GetCrtProp(enCrtProp_Level);

	//怪物等级
	INT32 NpcLevel = pMonster->GetCrtProp(enCrtProp_Level);

	float exp = 0;

	INT32 expParam = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_CombatExpParam;

	if(ActorLevel == NpcLevel )
	{
		//经验=（创建角色等级×2+70）
		exp = ActorLevel * 2 + expParam;
	}
	else if(ActorLevel < NpcLevel)
	{
		//经验=(创建角色等级×2+70)*[1+0.05*(怪物等级-创建角色等级)]
		exp = (ActorLevel * 2 + expParam)*(1+0.05*std::min(NpcLevel-ActorLevel,4)) ;
	}
	else if(ActorLevel-NpcLevel >= 10)  //高于10级不获得经验
	{
		exp =  0;
	}
	else
	{
		//经验=(创建角色等级×2+70)*[1-(创建角色等级-怪物等级)/10]
		exp = (ActorLevel * 2 + expParam)*(1-(ActorLevel-NpcLevel)/(float)10 ) ;
	}

	if(m_pCreature->GetThingClass() == enThing_Class_Actor)
	{
		IActor * pActor = (IActor*)m_pCreature;

		exp = exp + exp * pActor->GetVipValue(enVipType_AutoKillAddExp) / 100.0f;

		exp = exp*pActor->GetMultipExpFactor();
	}

	exp += 0.99999;

	return exp;	 
}

//启动自动施放法术定时器
void Fighter::StartMagicTimer()
{
	INT64 NextTime = NextMagicTime();
	INT64 CurTime = CURRENT_TIME()*1000;

	INT32 AutoMagicAttackDelay = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_AutoMagicAttackDelay;
	if(AutoMagicAttackDelay<1)
	{
		AutoMagicAttackDelay = 1;
	}

	INT64 Interval = NextTime-CurTime + (RandomService::GetRandom()%AutoMagicAttackDelay + 1)*1000;//随机延时一定时间
	if(Interval<100)
	{
		Interval = 100;
	}

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_Magic,this,Interval,"Fighter::StartMagicTimer[enTimerID_Magic]");
}

//更新CD时间
void Fighter::UpdateCDTime()
{
	IMagic* pMagic = GetMagic(m_CurMagicIndex);

	INT32 nCDTime = 0;
	if(pMagic==0)
	{
		nCDTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_CombatNullMagicCDTime;
	}
	else
	{
		nCDTime = pMagic->GetCDTime();
	}

	m_NextMagicTime = (CURRENT_TIME()+nCDTime)*1000 - this->GetMagicCDReduceValue();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Combat::Combat(UINT32 Param)
{
	m_pCombatObserver = 0;
	m_bBegin = false;
	m_DropID = 0;
	m_CombatID = 0;

	m_CurAckNo = 0; //当前确认号
	m_NeedAckNum = 0; //需要确认玩家数

	m_ActionNum = 0;

	m_bEnd = false;

	m_Param = Param;
}

Combat::~Combat()
{
	for(int i=0; i<m_vectAllFighter.size();i++)
	{
		delete m_vectAllFighter[i];
	}

	m_vectAllFighter.clear();
}

void Combat::Release()
{
	delete this;
}


UINT64 Combat::GetCombatID()
{
	if(m_CombatID == 0)
	{
		static UINT64 s_nestID = ::time(0);

		if(++s_nestID == 0)
		{
			++s_nestID;
		}

		m_CombatID = s_nestID;
	}


	return m_CombatID;
}


void Combat::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case TimerID_Init:
		{
			m_bBegin = true;
			g_pGameServer->GetTimeAxis()->KillTimer(TimerID_Init,this);
			INT32 nCDTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_PhysicsAttackCDTime;

			g_pGameServer->GetTimeAxis()->SetTimer(TimerID_Physics,this,nCDTime*1000,"Combat::OnTimer[TimerID_Physics]");
			OnPhysicsAttack();
		}
		break;
	case TimerID_Physics:
		{
			for(int i=0; i<m_vectAllFighter.size();i++)
			{
				Fighter * pFighter = m_vectAllFighter[i];
				if(pFighter->GetAckNo()!=0)
				{
					TRACE_COMBAT("玩家:%s[%s]超时没有发送确认号[%d]",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),pFighter->GetAckNo());
				}
			}
			OnPhysicsAttack();
		}
		break;
	}
}

//手动施放法术
bool  Combat::ManualMagic(IActor* pActor,CS_CombatFireMagic_Req & Req)
{
	if(m_bEnd)
	{
		return false;
	}
	Fighter * pFighter = GetFighter(Req.m_uidActor);
	if(pFighter == 0)
	{
		TRACE("<error> %s : %d line 找不到战斗人员 UID[%s]!",__FUNCTION__,__LINE__,Req.m_uidActor.ToString());
		return false;
	}

	if(pActor->GetUID()==Req.m_uidActor || (pActor->GetEmployee(Req.m_uidActor)!=0))

	{
		pFighter->ManualMagic(pActor,Req.m_MagicID);
	}
	else
	{
		SC_CombatFireMagic_Rsp Rsp;
		Rsp.m_MagicID = Req.m_MagicID;
		Rsp.m_uidActor = Req.m_uidActor;
		Rsp.m_Result = enCombatCode_ErrNoPurview;

		OBuffer4k ob;
		SCombatCombat CombatCombat;
		CombatCombat.m_CombatID = GetCombatID();
		CombatCombat.m_SubCmd = enCombatSubCmd_CS_FireMagic;


		ob << SCombatHeader(enCombatCmd_Combat,sizeof(SCombatCombat)+sizeof(Rsp)) << CombatCombat << Rsp;

		pActor->SendData(ob.TakeOsb());
	}

	return true;	
}

//结束战斗,bOneselfWin发起战斗方胜
void Combat::EndCombat(enCombatResult CombatResult)
{
	if(CombatResult==enCombatResult_Win)
	{
		for(int i=0; i<m_vectEnmey.size();i++)
		{
			m_vectEnmey[i]->Stop();
		}
		m_vectEnmey.clear();
	}
	else if(CombatResult==enCombatResult_Lose)
	{
		for(int i=0; i<m_vectOneself.size();i++)
		{
			m_vectOneself[i]->Stop();
		}

		m_vectOneself.clear();
	}
	else
	{
		for(int i=0; i<m_vectOneself.size();i++)
		{
			m_vectOneself[i]->Stop();
		}

		m_vectOneself.clear();

		for(int i=0; i<m_vectEnmey.size();i++)
		{
			m_vectEnmey[i]->Stop();
		}
		m_vectEnmey.clear();
	}

	CheckWinLose();
}

//确认指令
void Combat::AckAction(IActor* pActor,SCombatActionAck & Req)
{
	TRACE_COMBAT("玩家:%s[%s]发送确认号[%d]!",pActor->GetName(),pActor->GetUID().ToString(),Req.m_AckNo);

	if(m_bEnd)
	{
		return;
	}

	if(Req.m_AckNo != this->m_CurAckNo)
	{
		TRACE_COMBAT("玩家:%s[%s]发送的确认号[%d]与服务器端的确认号[%d]不一至!",pActor->GetName(),pActor->GetUID().ToString(),Req.m_AckNo,this->m_CurAckNo);
		return;
	}

	this->m_NeedAckNum--;

	Fighter * pFighter = this->GetFighter(pActor->GetUID());

	if(pFighter != 0)
	{
		pFighter->SetAckNo(0);


	}

	if(m_NeedAckNum<=0)
	{
		INT32 nCDTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_PhysicsAttackCDTime;
		g_pGameServer->GetTimeAxis()->KillTimer(TimerID_Physics,this);
		g_pGameServer->GetTimeAxis()->SetTimer(TimerID_Physics,this,nCDTime*1000,"Combat::OnTimer[TimerID_Physics]");
		OnPhysicsAttack();

	}
}

Fighter * Combat::GetFighter(UID uidCreature)
{
	for(int i=0; i<m_vectAllFighter.size();i++)
	{
		if(m_vectAllFighter[i]->GetUID() == uidCreature)
		{
			return m_vectAllFighter[i];
		}
	}

	return NULL;
}

bool Combat::OnVote(XEventData & EventData)
{
	return false;
}

//指定地点战斗
bool Combat::CombatWithScene(enCombatType CombatType,IActor* pActor,UID uidEnemy,ICombatObserver * pCombatObserver, const char* szSceneName,IGameScene * pGameScene, 
							 enJoinPlayer JoinPlayer,enCombatMode CombatMode,UINT8 ChiefIndex,UINT16 DropID)
{
	CombatCombatData CombatData ;

	m_pCombatObserver = pCombatObserver;
	this->m_uidOneself = pActor->GetUID();
	this->m_uidEnmey = uidEnemy;
	this->m_JoinPlayer = JoinPlayer;
	this->m_CombatMode = CombatMode;

	CombatData.m_CombatType = CombatType;

	m_SceneID = pGameScene->GetSceneID(); 



	IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidEnemy);
	if(pThing==0)
	{
		return false;
	}

	if(pThing->GetThingClass()==enThing_Class_Monster)
	{

		CombatData.m_bEnemyMonster = true;

		if( DropID==0)
		{
			IMonster * pMonster = (IMonster*)pThing;
			TSceneID  SceneID;
			SceneID.From(pMonster->GetCrtProp(enCrtProp_SceneID));

			//获取地图掉落
			const SMapConfigInfo * pMapInfo = g_pGameServer->GetConfigServer()->GetMapConfigInfo(SceneID.GetMapID());
			if(pMapInfo)
			{
				DropID = pMapInfo->m_DropID;
			}
		}
	}
	else
	{
		CombatData.m_bEnemyMonster = false;
	}

	m_DropID = DropID;


	strncpy(CombatData.m_SceneName,szSceneName,MEM_SIZE(CombatData.m_SceneName));

	std::vector<SCreatureLineupInfoEx> vectSelf ;

	GetJoinBattleActor(vectSelf,pActor,false,(JoinPlayer & enJoinPlayer_OneselfTeam)==enJoinPlayer_OneselfTeam,!((CombatMode&enCombatMode_OneselfTeam) == enCombatMode_OneselfTeam));


	std::vector<SCreatureLineupInfoEx> vectEnemy;

	GetJoinBattleCreature(vectEnemy,uidEnemy,pGameScene,!((CombatMode&enCombatMode_OnlyEnemy) == enCombatMode_OnlyEnemy),(JoinPlayer & enJoinPlayer_EnemyTeam)==enJoinPlayer_EnemyTeam,
		!((CombatMode&enCombatMode_EnemyTeam) == enCombatMode_EnemyTeam),ChiefIndex);


	OBuffer4k obMagic;

	for(int i=0; i<vectSelf.size();i++)
	{
		SCreatureLineupInfoEx & Info = vectSelf[i];

		Fighter* pFighter = MakeFighter(Info.m_pCreature,Info.m_Pos,Info.m_bAutoFighte,true);
		if(pFighter)
		{

			m_vectOneself.push_back(pFighter);
			GetLineupInfo(pFighter,Info.m_pCreature,obMagic);
			pFighter->Start();

			UINT32 MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddStatus);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_AddStatus]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RemoveStatus);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_RemoveStatus]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_DieAtCombat]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TimerStatus);
			pFighter->GetCreature()->SubscribeAction(MsgID,this,"Combat::CombatWithScene[enMsgID_TimerStatus]");

			if( Info.m_pCreature->GetThingClass()==enThing_Class_Actor)
			{
				IActor * pActorTemp = (IActor*)Info.m_pCreature;
				pFighter->GetCreature()->RecoverBlood();
				if(pActorTemp->GetMaster()==0)
				{					
					UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogout);

					pActorTemp->SubscribeEvent(msgID,this,"Combat::CombatWithScene[enMsgID_ActorLogout]");

					msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

					pActorTemp->SubscribeVote(msgID,this,"Combat::CombatWithScene[]");

					msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

					pActorTemp->SubscribeEvent(msgID,this,"Combat::CombatWithScene[enMsgID_EnterScene]");


				}

				if(Info.m_bAutoFighte == false)
				{
					//主角进入场景
					pGameScene->EnterScene(pActorTemp);
					if(CombatData.m_bEnemyMonster)
					{
						pFighter->CalculateExp((ICreature*)pThing);
					}

					pActorTemp->SetIsInCombat(true);
				}
			}

			TRACE_COMBAT("我方:%s[%s] 血量[%d]",pFighter->GetCreature()->GetName(),pFighter->GetCreature()->GetUID().ToString(),pFighter->GetCreature()->GetCrtProp(enCrtProp_Blood));

		}
	}

	TRACE_COMBAT("                       VS                     ");

	for(int i=0; i<vectEnemy.size();i++)
	{
		SCreatureLineupInfoEx & Info = vectEnemy[i];


		Fighter* pFighter = MakeFighter(Info.m_pCreature,Info.m_Pos,Info.m_bAutoFighte,false);
		if(pFighter)
		{
			m_vectEnmey.push_back(pFighter);
			GetLineupInfo(pFighter,Info.m_pCreature,obMagic);
			pFighter->Start();

			UINT32 MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddStatus);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_AddStatus]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RemoveStatus);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_RemoveStatus]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat);
			pFighter->GetCreature()->SubscribeEvent(MsgID,this,"Combat::CombatWithScene[enMsgID_DieAtCombat]");

			MsgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TimerStatus);
			pFighter->GetCreature()->SubscribeAction(MsgID,this,"Combat::CombatWithScene[enMsgID_TimerStatus]");



			if(Info.m_pCreature->GetThingClass()==enThing_Class_Actor)
			{
				IActor * pActorTemp = (IActor*)Info.m_pCreature;
				pFighter->GetCreature()->RecoverBlood();
				if(pActorTemp->GetMaster()==0)
				{

					UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogout);

					pActorTemp->SubscribeEvent(msgID,this,"Combat::CombatWithScene[enMsgID_ActorLogout]");

					msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UnloadActor);

					pActorTemp->SubscribeVote(msgID,this,"Combat::CombatWithScene[]");

					msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene);

					pActorTemp->SubscribeEvent(msgID,this,"Combat::CombatWithScene[enMsgID_EnterScene]");

				}

				if(Info.m_bAutoFighte == false)
				{
					//主角进入场景
					pGameScene->EnterScene(pActorTemp);
					if(CombatData.m_bEnemyMonster)
					{
						pFighter->CalculateExp((ICreature*)pThing);
					}

				}
			}

			TRACE_COMBAT("敌方:%s[%s] 血量[%d]",pFighter->GetCreature()->GetName(),pFighter->GetCreature()->GetUID().ToString(),pFighter->GetCreature()->GetCrtProp(enCrtProp_Blood));

		}
	}

	m_vectAllFighter = m_vectOneself;
	m_vectAllFighter.insert(m_vectAllFighter.end(),m_vectEnmey.begin(),m_vectEnmey.end());

	for(int i=0; i < m_vectAllFighter.size(); i++ )
	{
		Fighter* pFighter = m_vectAllFighter[i];
		ICreature * pCreature = pFighter->GetCreature();

		if(pCreature->GetThingClass()==enThing_Class_Actor)
		{
			IActor * pActor = (IActor*)pCreature;
			IActor* pMaster = pActor->GetMaster();
			if(pMaster)
			{
				Fighter* pMasterFighter = GetFighter(pMaster->GetUID());
				pFighter->SetMasterFighter(pMasterFighter);
			}
		}
	}


	CombatData.m_SelfActorNum = m_vectOneself.size();
	CombatData.m_EnemyNum = m_vectEnmey.size();

	SCombatCombat CombatCombat;
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Init;
	CombatCombat.m_CombatID = GetCombatID();

	OBuffer4k ob;

	ob << SCombatHeader(enCombatCmd_Combat,sizeof(SCombatCombat)+sizeof(CombatData)+obMagic.Size()) << CombatCombat << CombatData << obMagic;

	Broadcast(ob);

	//启动定时器

	INT32 nTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_CombatReadyTime;

	g_pGameServer->GetTimeAxis()->SetTimer(TimerID_Init,this,nTime*1000,"Combat::CombatWithScene[TimerID_Init]");

	return true;
}

//创建参战人员
Fighter* Combat::MakeFighter(ICreature *  pCreature,UINT8 nPos,bool bAutoFighte,bool bOneSelf)
{
	bool bNeedSync = false;

	if(bAutoFighte==false && pCreature->GetThingClass()==enThing_Class_Actor)
	{
		IActor * pActor = (IActor*)pCreature;

		if(pActor->GetMaster() == 0)
		{
			bNeedSync = true;
		}
	}

	Fighter* pFighter = new Fighter(this,pCreature,nPos,bAutoFighte,bNeedSync,bOneSelf);

	return pFighter;
}

//获得我方参战人员
void Combat::GetJoinBattleActor(std::vector<SCreatureLineupInfoEx>& vectEx,IActor* pActor,bool bIsAutoFighte,bool bIsTeam ,bool bIsTeamAutoFighte)
{
	std::vector<SCreatureLineupInfo> vect;

	ICombatPart * pCombatPart = pActor->GetCombatPart();
	if(pCombatPart==0)
	{
		TRACE("<error> %s : %d line 获取不到玩家CombatPart!",__FUNCTION__,__LINE__);			
		return ;
	}

	vect = pCombatPart->GetJoinBattleActor(bIsTeam);

	for(int i=0; i<vect.size();i++)
	{
		SCreatureLineupInfoEx InfoEx;

		if((vect[i].m_pCreature == pActor)|| (((IActor*)vect[i].m_pCreature)->GetMaster() == pActor))
		{
			InfoEx.m_bAutoFighte = bIsAutoFighte;
		}
		else
		{
			InfoEx.m_bAutoFighte = bIsTeamAutoFighte;
		}


		InfoEx.m_pCreature = vect[i].m_pCreature;
		InfoEx.m_Pos = vect[i].m_Pos;
		vectEx.push_back(InfoEx);
	}

	return ;
}

//获得参战生物
void Combat::GetJoinBattleCreature(std::vector<SCreatureLineupInfoEx> & vectEx,UID uidEnemy,IGameScene * pGameScene,bool bIsAutoFighte,bool bIsTeam,bool bIsTeamAutoFighte,UINT8 ChiefIndex)
{
	std::vector<SCreatureLineupInfo> vect;
	IThing * pThing = g_pGameServer->GetGameWorld()->GetThing(uidEnemy);

	if(pThing==0)
	{
		return ;
	}

	if(pThing->GetThingClass() == enThing_Class_Actor)
	{
		IActor * pActor = (IActor*)pThing;
		GetJoinBattleActor(vectEx,pActor,bIsAutoFighte,bIsTeam,bIsTeamAutoFighte);
	}
	else if(pThing->GetThingClass() == enThing_Class_Monster)
	{
		//从场景中获取
		std::vector<UID> vectMonster = pGameScene->GetAllMonster();
		for(int i=0; i<vectMonster.size();i++)
		{
			IMonster * pMonster = g_pGameServer->GetGameWorld()->FindMonster(vectMonster[i]);
			if(pMonster)
			{
				SCreatureLineupInfoEx Info;
				Info.m_pCreature = pMonster;
				Info.m_Pos = pMonster->GetCrtProp(enCrtProp_MonsterLineup);
				Info.m_bAutoFighte = true;
				if(ChiefIndex - 1==Info.m_Pos)
				{
					vectEx.insert(vectEx.begin(),Info);
				}
				else
				{
					vectEx.push_back(Info);
				}
			}
		}
	}

	return ;

}



//获得参战人员信息
void  Combat::GetLineupInfo(Fighter* pFighter,ICreature * pCreature, OBuffer4k & ob)
{
	SLineupInfo LineupInfo;

	LineupInfo.m_uidCreature =pFighter->GetUID();
	LineupInfo.m_Pos		 =pFighter->GetPos();
	strncpy(LineupInfo.m_szName, pCreature->GetName(), sizeof(LineupInfo.m_szName));
	LineupInfo.m_Level		 =  pCreature->GetCrtProp(enCrtProp_Level);
	LineupInfo.m_MagicNum    = 0;
	LineupInfo.m_NeedExpUpLevel = 0;
	LineupInfo.m_Exp		 = 0;
	LineupInfo.m_NenLi		 = enNenLiType_PuTong;
	LineupInfo.m_DecreaseCDTime = pFighter->GetMagicCDReduceValue()/100;

	std::vector<SMagicPosInfo> vectMagicID;

	switch(pCreature->GetThingClass())
	{
	case enThing_Class_Actor:
		{
			IActor * pActor = (IActor *)pCreature;

			LineupInfo.m_Facade = pActor->GetCrtProp(enCrtProp_ActorFacade);

			LineupInfo.m_BloodUp = pActor->GetCrtProp(enCrtProp_ActorBloodUp);

			LineupInfo.m_Exp    = pActor->GetCrtProp(enCrtProp_ActorExp);

			LineupInfo.m_NenLi  = pActor->GetNenLi();

			//得到等级下级需要经验
			const SActorLevelCnfg * pLevelCnfg = g_pGameServer->GetConfigServer()->GetActorLevelCnfg(pActor->GetCrtProp(enCrtProp_Level) + 1);
			if( 0 != pLevelCnfg){
				LineupInfo.m_NeedExpUpLevel = pLevelCnfg->m_NeedExp;
			}

			//普通法术
			IMagicPart * pMagicPart = pActor->GetMagicPart();
			if( 0 == pMagicPart){
				return;
			}

			for( int i = 0; i < MAX_EQUIP_MAGIC_NUM; ++i)
			{
				IMagic * pMagic = pMagicPart->GetLoadedMagic(i);
				if( 0 == pMagic){
					continue;
				}



				const SMagicCnfg * pMagicCnfg = pMagic->GetMagicCnfg();

				if(pMagicCnfg==0 || pMagicCnfg->m_bPassive)
				{
					continue;
				}



				++LineupInfo.m_MagicNum;

				SMagicPosInfo MagicInfo;

				MagicInfo.m_MagicID = pMagic->GetMagicID();
				MagicInfo.m_Index = (i)*3;
				vectMagicID.push_back(MagicInfo );

				pFighter->SetMagic(MagicInfo.m_Index,MagicInfo.m_MagicID,pMagic->GetLevel());
			}

			//仙剑法术
			IEquipPart * pEquipPart = pActor->GetEquipPart();
			if( 0 == pEquipPart){
				return;
			}

			for( int i = enEquipPos_GodSwordOne; i <= enEquipPos_GodSwordThree; ++i)
			{
				IEquipment * pEquipment = pEquipPart->GetEquipByPos(i);
				if( 0 == pEquipment){
					continue;
				}

				const  SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pEquipment->GetGoodsID());
				if( 0 == pGoodsCnfg){
					TRACE("<error> %s : %d 行 获取物品配置信息出错!!物品ID = %d", __FUNCTION__, __LINE__, pEquipment->GetGoodsID());
					continue ;
				}

				const SGodSwordWorldCnfg * pSwordWorldCnfg = g_pGameServer->GetConfigServer()->GetGoldSwordWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);
				if( 0 == pSwordWorldCnfg){
					continue;
				}

				int nSecretLevel = 0;
				if( !pEquipment->GetPropNum(enGoodsProp_SecretLevel, nSecretLevel)){
					continue;
				}

				//剑决等级大于1的仙剑才有法术
				if( nSecretLevel <= 0){
					continue;
				}

				const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(pSwordWorldCnfg->m_MagicID);

				if(pMagicCnfg==0 || pMagicCnfg->m_bPassive)
				{
					continue;
				}

				++LineupInfo.m_MagicNum;

				SMagicPosInfo MagicInfo;

				MagicInfo.m_MagicID = pSwordWorldCnfg->m_MagicID;
				MagicInfo.m_Index = 2+(i-enEquipPos_GodSwordOne)*3;

				vectMagicID.push_back(MagicInfo );

				pFighter->SetMagic(MagicInfo.m_Index,MagicInfo.m_MagicID,nSecretLevel);

			}

			//法宝法术
			for( int i = enEquipPos_TalismanOne; i <= enEquipPos_TalismanThree; ++i)
			{
				IEquipment * pEquipment = pEquipPart->GetEquipByPos(i);
				if( 0 == pEquipment){
					continue;
				}

				const  SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pEquipment->GetGoodsID());
				if( 0 == pGoodsCnfg){
					TRACE("<error> %s : %d 行 获取物品配置信息出错!!物品ID = %d", __FUNCTION__, __LINE__, pEquipment->GetGoodsID());
					continue ;
				}

				int nMagicLevel = 0;
				if( !pEquipment->GetPropNum(enGoodsProp_MagicLevel, nMagicLevel)){
					continue;
				}

				const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg( pGoodsCnfg->m_SpiritOrMagic);

				if(pMagicCnfg==0 || pMagicCnfg->m_bPassive)
				{
					continue;
				}



				++LineupInfo.m_MagicNum;


				SMagicPosInfo MagicInfo;

				MagicInfo.m_MagicID = pGoodsCnfg->m_SpiritOrMagic;
				MagicInfo.m_Index = 1+(i-enEquipPos_TalismanOne)*3;
				vectMagicID.push_back(MagicInfo );				

				pFighter->SetMagic(MagicInfo.m_Index,MagicInfo.m_MagicID,nMagicLevel);		
			}
		}
		break;
	case enThing_Class_Monster:
		{

			const SMonsterCnfg* pMonsterCnfg = g_pGameServer->GetConfigServer()->GetMonsterCnfg(pCreature->GetCrtProp(enCrtProp_MonsterID));
			if( 0 == pMonsterCnfg){
				TRACE("<error> %s ; %d 行 获取怪物配置信息出错!!!怪物ID = %d", __FUNCTION__, __LINE__, pCreature->GetCrtProp(enCrtProp_MonsterID));
				return;
			}

			LineupInfo.m_Facade = pMonsterCnfg->m_Facade;

			LineupInfo.m_BloodUp = pCreature->GetCrtProp(enCrtProp_Blood);

			for( int i = (int)enCrtProp_MonsterMagic1; i <= (int)enCrtProp_MonsterMagic5; ++i)
			{
				IMagic * pMagic = (IMagic*)pCreature->GetCrtProp((enCrtProp)i);
				if( 0 == pMagic){
					continue;
				}

				const SMagicCnfg * pMagicCnfg = pMagic->GetMagicCnfg();

				if(pMagicCnfg==0 || pMagicCnfg->m_bPassive)
				{
					continue;
				}

				++LineupInfo.m_MagicNum;

				SMagicPosInfo MagicInfo;

				MagicInfo.m_MagicID = pMagic->GetMagicID();
				MagicInfo.m_Index = (i-enCrtProp_MonsterMagic1);
				vectMagicID.push_back(MagicInfo );

				pFighter->SetMagic(MagicInfo.m_Index,MagicInfo.m_MagicID,1);	
			}
		}
		break;
	}

	ob << LineupInfo;

	IActor * pActor = 0;
	if( pCreature->GetThingClass() == enThing_Class_Actor){
		pActor = (IActor *)pCreature;
	}

	for( int i = 0; i < vectMagicID.size(); ++i)
	{
		SMagicPosInfo & MagicInfo =vectMagicID[i] ;
		ob << MagicInfo;

		if( 0 != pActor){
			pActor->SendMagicLevelCnfg(vectMagicID[i].m_MagicID);
		}
	}
}

UINT32 Combat::NextAckNo()
{
	static UINT32 s_NextAck = 0;

	if(++s_NextAck == 0)
	{
		++s_NextAck;
	}

	return s_NextAck;
}


//广播数据
void Combat::Broadcast(OBuffer4k & ob,UINT32 AckNo)
{
	if(AckNo != 0)
	{
		this->m_CurAckNo = AckNo;
		this->m_NeedAckNum = 0;
	}

	for(int i=0;i<m_vectAllFighter.size();i++)
	{
		Fighter * pFighter = m_vectAllFighter[i];
		if(pFighter->IsNeedSync())
		{
			OBuffer4k obTemp;
			obTemp << ob;
			pFighter->SendToClient(obTemp.TakeOsb());
			if(AckNo != 0)
			{
				pFighter->SetAckNo(AckNo);				
				this->m_NeedAckNum++;
				TRACE_COMBAT("给玩家:%s[%s]发送确认号[%d]",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),pFighter->GetAckNo());
			}
		}
	}	

	const SCombatHeader * pCombatHeader = (const SCombatHeader*)ob.Buffer();

	m_obAction.Push(pCombatHeader+1,ob.Size()-sizeof(SCombatHeader));

	m_ActionNum++;

}

void Combat::SetAutoMagicMode(UID uidActor)
{
	Fighter * pFighter = GetFighter(uidActor);

	if(pFighter)
	{
		//if(pFighter->IsAutoFighte()==false)
		{
			INT32 manualNum = 0; //手动模式玩家数量

			std::vector<Fighter *> vect;

			for(int i=0;i<m_vectOneself.size();i++)
			{
				Fighter * pFighterOther = m_vectOneself[i];
				if(pFighterOther->IsAutoFighte())
				{
					continue;
				}

				manualNum++;

				if((pFighterOther == pFighter)|| (pFighterOther->GetMasterFighter() == pFighter ))
				{
					vect.push_back(pFighterOther);		
				}					
			}

			for(int i=0;i<m_vectEnmey.size();i++)
			{
				Fighter * pFighterOther = m_vectEnmey[i];
				if(pFighterOther->IsAutoFighte())
				{
					continue;
				}

				manualNum++;

				if((pFighterOther == pFighter)|| (pFighterOther->GetMasterFighter() == pFighter ))
				{
					vect.push_back(pFighterOther);		
				}					
			}	


			if(manualNum <= vect.size())
			{
				if((m_CombatMode&enJoinPlayer_OnlyEnemy) != 0)
				{
					EndCombat(enCombatResult_Peace);
				}
				else
				{
					EndCombat(pFighter->IsOneSelf()?enCombatResult_Lose : enCombatResult_Win );
				}

			}
			else
			{
				for(int i=0; i<vect.size();i++)
				{
					vect[i]->SetAutoMagicMode();
				}

				pFighter->OnOffLine();

			}
		}
	}
}

void Combat::OnEvent(XEventData & EventData)
{
	switch(EventData.m_MsgID)
	{
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogout):
		{
			SS_ActorLogout * pActorLogout = (SS_ActorLogout *)EventData.m_pContext;
			SetAutoMagicMode(pActorLogout->m_uidActor);
		}
		break;
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_EnterScene):
		{
			SS_EnterScene * pEnterScene = (SS_EnterScene *)EventData.m_pContext;
			if(m_SceneID.ToID() == pEnterScene->m_OldSceneID && EventData.m_EventSrcType == enEventSrcType_Actor)
			{
				IActor * pAcor = (IActor *)EventData.m_MsgSource;
				SetAutoMagicMode(pAcor->GetUID());
			}
		}
		break;
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_AddStatus):
		{
			SS_AddStatus * pAddStatus = (SS_AddStatus*)EventData.m_pContext;
			OnAddStatus(pAddStatus);
		}
		break;
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RemoveStatus):
		{
			SS_RemoveStatus * pRemoveStatus = (SS_RemoveStatus*)EventData.m_pContext;
			OnRemoveStatus(pRemoveStatus);
		}
		break;
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat):
		{
			SS_DieAtCombat * pDieAtCombat = (SS_DieAtCombat*)EventData.m_pContext;
			OnCreatureDie(pDieAtCombat);
		}
		break;
	}
}

void Combat::OnAction(XEventData & EventData)
{
	switch(EventData.m_MsgID)
	{
	case MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TimerStatus):
		{
			SS_TimerStatus * pTimerStatus = (SS_TimerStatus *)EventData.m_pContext;
			OnTimerStatus(pTimerStatus);
		}
		break;
	}
}

void Combat::OnAddStatus(SS_AddStatus * pAddStatus)
{
	Fighter * pFighter = GetFighter(pAddStatus->m_uidActor);
	if(pFighter != 0)
	{
		pFighter->OnAddStatus(pAddStatus->m_StatusID,pAddStatus->m_MagicID,pAddStatus->m_StatusTime);
	}
}

void Combat::OnRemoveStatus(SS_RemoveStatus * pRemoveStatus)
{
	Fighter * pFighter = GetFighter(pRemoveStatus->m_uidActor);
	if(pFighter != 0)
	{
		pFighter->OnRemoveStatus(pRemoveStatus->m_StatusType);
	}
}

void Combat::OnCreatureDie(SS_DieAtCombat * pDieAtCombat)
{
	Fighter * pFighter = GetFighter(pDieAtCombat->m_uidActor);
	if(pFighter != 0)
	{
		pFighter->Stop();
		Remove(pFighter);
	}
	else
	{
		TRACE_COMBAT("找不到死亡的玩家[%s]",pDieAtCombat->m_uidActor.ToString());
	}

	CheckWinLose();
}

void Combat::OnTimerStatus(SS_TimerStatus * pTimerStatus)
{
	Fighter * pFighter = GetFighter(pTimerStatus->m_uidActor);
	if(pFighter == 0)
	{
		return;
	}

	if(pFighter->IsDie())
	{
		return;
	}

	SRoundStartContext RoundStartCnt;

	INT32 BaseAttackValue =  1;

	RoundStartCnt.m_AddBlood = 0;
	RoundStartCnt.m_AddDamageValue = 0;
	RoundStartCnt.m_BaseDamageValue = BaseAttackValue;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RoundStart);

	//发布事件
	pFighter->GetCreature()->OnAction(msgID,&RoundStartCnt,sizeof(RoundStartCnt));


	//真实攻击力
	INT32 DamageValue = RoundStartCnt.m_AddDamageValue;

	OBuffer4k ob;

	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = GetCombatID();
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	bool bDie = false;

	if(DamageValue>0)
	{

		//受到伤害,扣血
		INT32 nNewValue = 0;
		pFighter->GetCreature()->AddCrtPropNum(enCrtProp_Blood,-DamageValue,&nNewValue);

		SCombatAction CombatAction ;
		CombatAction.m_ActionType = enActionType_Damage;

		SActionDamageInfo    DamageInfo;   //伤害信息

		DamageInfo.m_bDie = (nNewValue==0);

		bDie = DamageInfo.m_bDie;

		DamageInfo.m_DamageType = enDamageType_Persist;

		DamageInfo.m_DamageValue = DamageValue;

		DamageInfo.m_uidCreature = pFighter->GetUID();

		ob << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(DamageInfo)) << CombatCombat << CombatAction << DamageInfo;

		pFighter->Broadcast(ob);


		TRACE_COMBAT("指令: 玩家[%s] 受到状态伤害 %d 是否死亡 [%d] \n",pFighter->GetCreature()->GetName(),DamageValue,DamageInfo.m_bDie);
	}
	else if(RoundStartCnt.m_AddBlood != 0)
	{
		//加血
		INT32 nNewValue = 0;
		pFighter->GetCreature()->AddCrtPropNum(enCrtProp_Blood,RoundStartCnt.m_AddBlood,&nNewValue);

		SCombatAction CombatAction ;
		CombatAction.m_ActionType = enActionType_AddBlood;

		SActionAddBloodInfo    BloodInfo;   //伤害信息
		BloodInfo.m_TargetNum = 1;
		BloodInfo.m_uidSource = pFighter->GetUID();;

		SAddBloodTarget TargetInfo;

		TargetInfo.m_bDie = (nNewValue==0);

		bDie = TargetInfo.m_bDie;

		TargetInfo.m_BloodValue = RoundStartCnt.m_AddBlood;

		TargetInfo.m_uidCreature = pFighter->GetUID();

		ob<< SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(BloodInfo)+sizeof(TargetInfo)) <<  CombatCombat << CombatAction << BloodInfo << TargetInfo;

		TRACE_COMBAT("指令 : 玩家[%s] 受到状态加血 %d \n",pFighter->GetCreature()->GetName(),RoundStartCnt.m_AddBlood);

		pFighter->Broadcast(ob);
	}

	if(bDie)
	{

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat);

		SS_DieAtCombat DieAtCombat ;
		DieAtCombat.m_uidActor = pFighter->GetUID();
		pFighter->GetCreature()->OnEvent(msgID,&DieAtCombat,sizeof(DieAtCombat));	
	}
}

//按身法优先级排序
std::vector<Fighter*> Combat::SortByPrority(std::vector<Fighter*> & vectSelf,
											std::vector<Fighter*> & vectEnemy)
{
	std::vector<Fighter*> vect;

	InsertByPrority(vect,vectSelf);

	InsertByPrority(vect,vectEnemy);

	return vect; 
}

//按身法，灵力的优先级把vect2合并到vect1中
void Combat::InsertByPrority(std::vector<Fighter*> & vect1,
							 const std::vector<Fighter*> & vect2)
{

	for(int i=0; i< vect2.size(); i++)
	{
		Fighter * pFighter = vect2[i];
		ICreature * pCreature2 = pFighter->GetCreature();
		//身法值
		INT32 nCurValue = pCreature2->GetCrtProp(enCrtProp_Avoid);

		//查找在vect在的位置
		int Index = 0;
		for(;Index<vect1.size();++Index)
		{
			ICreature * pCreature = vect1[Index]->GetCreature();

			INT32 nCurValue2 = pCreature->GetCrtProp(enCrtProp_Avoid);
			if(nCurValue>nCurValue2)
			{
				break;
			}
			else if(nCurValue<nCurValue2)
			{
				continue;
			}

			//相等则比较灵力
			//灵力值
			INT32 nCurSpiritValue = pCreature2->GetCrtProp(enCrtProp_Spirit);
			for(;Index<vect1.size();++Index)
			{
				ICreature * pCreature = vect1[Index]->GetCreature();
				//身法
				if(pCreature->GetCrtProp(enCrtProp_Avoid)!=nCurValue2)
				{
					break;
				}
				INT32 nCurSpiritValue2 = pCreature->GetCrtProp(enCrtProp_Spirit);
				if(nCurSpiritValue>=nCurSpiritValue2)
				{
					break;
				}
			}
			break;
		}

		//插入
		vect1.insert(vect1.begin()+Index,pFighter);
	}
}

//施放法术
bool Combat::UseMagic(Fighter* pFighter,IMagic* pMagic,INT32 nIndex)
{
	INT32 TargetIndex = 0;
	INT32 SourceIndex = 0;
	Fighter * pEnemy = 0;

	OBuffer4k ob;
	INT32 ActionNum = 0;

	bool bOver = false;

	if(pFighter->IsOneSelf())
	{
		TargetIndex = GetMagicAttackedTarget(pFighter->GetPos(),m_vectEnmey);
		pEnemy = m_vectEnmey[TargetIndex];
		SourceIndex = GetIndexInVect(m_vectOneself,pFighter);
		return pMagic->UseSkill(GetCombatID(),nIndex,(std::vector<IFighter*> &)m_vectOneself,SourceIndex,(std::vector<IFighter*> &)m_vectEnmey,TargetIndex,ob,ActionNum);

	}
	else
	{
		TargetIndex = GetMagicAttackedTarget(pFighter->GetPos(),m_vectOneself);
		pEnemy = m_vectOneself[TargetIndex];
		SourceIndex = GetIndexInVect(m_vectEnmey,pFighter);
		return pMagic->UseSkill(GetCombatID(),nIndex,(std::vector<IFighter*> &)m_vectEnmey,SourceIndex,(std::vector<IFighter*> &)m_vectOneself,TargetIndex,ob,ActionNum);
	}


	return false;
}

//查询生物中数组中的索引
INT32 Combat::GetIndexInVect(std::vector<Fighter*> & vect,Fighter* pCreature)
{
	INT32 Index = 0;
	for(;Index < vect.size();Index++)
	{
		if(vect[Index] == pCreature)
		{
			return Index;
		}
	}

	return Index;
}
//物理攻击
void Combat::OnPhysicsAttack()
{
	Fighter * pFighter = GetLaunchFighter();

	if(pFighter==0)
	{
		TRACE("<error> %s : %d line pFighter==0",__FUNCTION__,__LINE__);
		return;
	}

	Fighter * pEnemy = 0;
	INT32 TargetIndex = 0;
	std::vector<Fighter*> * pVectTarget = 0;

	//获取攻击目标
	if(pFighter->IsOneSelf())
	{
		TargetIndex = GetPhysicsAttackedTarget(pFighter->GetPos(),m_vectEnmey);
		pEnemy = m_vectEnmey [TargetIndex];
		pVectTarget = &m_vectEnmey;
	}
	else
	{
		TargetIndex = GetPhysicsAttackedTarget(pFighter->GetPos(),m_vectOneself);
		pEnemy = m_vectOneself [TargetIndex];
		pVectTarget = &m_vectOneself;
	}

	SAttackContext AttackCnt;
	AttackCnt.m_uidSource =  pFighter->GetUID();
	AttackCnt.m_BaseAttackValue =0;
	AttackCnt.m_uidTarget = pEnemy->GetUID();
	AttackCnt.m_AttackType = enAttackType_Physics;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack);


	OBuffer4k ob;

	SCombatAction CombatAction ;
	CombatAction.m_ActionType = enActionType_Attack;

	SActionAttackInfo   AttackInfo;   //攻击信息

	AttackInfo.m_uidSource = pFighter->GetUID();
	AttackInfo.m_MagicID   = INVALID_MAGIC_ID;
	AttackInfo.m_MagicResID = 0;
	MEM_ZERO(AttackInfo.m_szMagicName);
	AttackInfo.m_AttackType = enAttackType_Physics;
	AttackInfo.m_TargetNum = 1;
	AttackInfo.m_AckNo = this->NextAckNo();

	//是否需要减状态的生效次数
	bool bDecreaseCount = false;

	SAttackedTarget TargetInfo;

	TargetInfo.m_DamageValue  = 0;
	TargetInfo.m_bHit = true;
	TargetInfo.m_bKnocking = false;
	TargetInfo.m_uidTarget =  pEnemy->GetUID();
	TargetInfo.m_DamageValue = enDamageType_Physics;

	std::string strEnemy = pEnemy->GetUID().ToString();

	TRACE_COMBAT("%s[%s] 对 %s[%s]进行物理攻击!",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),pEnemy->GetCreature()->GetName(),strEnemy.c_str());

	//计算命中
	if(pFighter->IsPhysicsAttackHit(pEnemy)==false)
	{
		TargetInfo.m_bHit = false;	
		TargetInfo.m_DamageValue = 0;

		TRACE_COMBAT("目标没有命中!\n");				
	}	
	else
	{
		//爆击参数
		float AttackCriticalHitParam = pFighter->GetPhysicsAttackCriticalHitParam(pEnemy);

		if(AttackCriticalHitParam>0)
		{
			TargetInfo.m_bKnocking = true;			
		}
		else
		{
			AttackCriticalHitParam = 1.0;

			TargetInfo.m_bKnocking = false;			
		}


		AttackCnt.m_BaseAttackValue = pFighter->GetPhysicsAttackValue(pEnemy,AttackCriticalHitParam);

		//发布事件
		pFighter->GetCreature()->OnAction(msgID,&AttackCnt,sizeof(AttackCnt));


		//真实攻击力
		INT32 AttackValue =  AttackCnt.m_BaseAttackValue + AttackCnt.m_AddAttackValue;

		if(AttackValue<1)
		{
			AttackValue = 1;
		}

		//发布被攻击事件
		SAttackedContext AttackedCnt;
		AttackedCnt.m_uidSource = pEnemy->GetUID();
		AttackedCnt.m_uidTarget = pEnemy->GetUID();
		AttackedCnt.m_BaseDamageValue = AttackValue;
		AttackedCnt.m_DamageType = enDamageType_Physics;

		pEnemy->GetCreature()->OnAction(MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attacked),&AttackedCnt,sizeof(AttackedCnt));

		bDecreaseCount = AttackedCnt.m_bDefense;

		INT32 DamageValue = AttackedCnt.m_BaseDamageValue + AttackedCnt.m_AddDamageValue;

		//增加随机浮动	
		DamageValue = DamageValue*GetAttackFloatParam()+0.99999;

		//当前血量
		INT32 nCurBlood = pEnemy->GetCreature()->GetCrtProp(enCrtProp_Blood);

		if(nCurBlood<=DamageValue && AttackedCnt.m_bAvoidDie==true)
		{
			DamageValue = nCurBlood-1;		
			bDecreaseCount = true;
		}

		TargetInfo.m_DamageValue = DamageValue;

		if(TargetInfo.m_DamageValue>0)
		{
			//受到伤害,扣血
			INT32 nNewValue = 0;
			pEnemy->GetCreature()->AddCrtPropNum(enCrtProp_Blood,-TargetInfo.m_DamageValue,&nNewValue);

			pFighter->AddDamageValueToBoss(TargetInfo.m_DamageValue);

			if(nNewValue==0)
			{
				TargetInfo.m_bDie     = true;	
			}
			else
			{
				//回血
				if(AttackedCnt.m_AddBloodFactor > 0)
				{
					TargetInfo.m_AddBloodValue = TargetInfo.m_DamageValue * AttackedCnt.m_AddBloodFactor + 0.99999;
					pEnemy->GetCreature()->AddCrtPropNum(enCrtProp_Blood,TargetInfo.m_AddBloodValue,&nNewValue);

					bDecreaseCount = true;
				}

				//反伤
				if(AttackedCnt.m_SuckBloodFactor != 0 && TargetInfo.m_bDie == false)
				{
					AttackInfo.m_AddBloodValue = AttackedCnt.m_SuckBloodFactor *(TargetInfo.m_DamageValue) + 0.99999;

					if( AttackInfo.m_AddBloodValue != 0)
					{
						bDecreaseCount = true;
						pFighter->GetCreature()->AddCrtPropNum(enCrtProp_Blood, AttackInfo.m_AddBloodValue,&nNewValue);
						if(nNewValue==0)
						{
							AttackInfo.m_bDie = true;
						}

						if(AttackInfo.m_AddBloodValue<0)
						{
							pEnemy->AddDamageValueToBoss(-AttackInfo.m_AddBloodValue);
						}
					}
				}
			}
		}	

		TRACE_COMBAT("目标命中,选成伤害 %d 目标是否死亡[%d]回血[%d],自身加血[%d]!\n",TargetInfo.m_DamageValue,TargetInfo.m_bDie,TargetInfo.m_AddBloodValue,AttackInfo.m_AddBloodValue);

		//判断胜负
	}

	SCombatCombat CombatCombat;
	CombatCombat.m_CombatID = GetCombatID();
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Action;

	ob << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatAction)+sizeof(AttackInfo)+sizeof(TargetInfo)) << CombatCombat << CombatAction << AttackInfo << TargetInfo;

	this->Broadcast(ob,AttackInfo.m_AckNo);


	if(TargetInfo.m_bHit && bDecreaseCount)
	{
		SMyCampAttackedContext MyCampAttackedCnt;
		MyCampAttackedCnt.m_uidSource = pFighter->GetUID();
		MyCampAttackedCnt.m_uidTarget.push_back(pEnemy->GetUID());

		msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_MyCampAttacked);

		for(int i=0; i<pVectTarget->size();i++)
		{
			(*pVectTarget)[i]->GetCreature()->OnEvent(msgID,&MyCampAttackedCnt,SIZE_OF(MyCampAttackedCnt));			
		}	

	}


	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_DieAtCombat);

	if(pFighter->IsDie())
	{
		SS_DieAtCombat DieAtCombat ;
		DieAtCombat.m_uidActor = pFighter->GetUID();
		pFighter->GetCreature()->OnEvent(msgID,&DieAtCombat,sizeof(DieAtCombat));		
	}
	else if(pEnemy->IsDie())
	{


		SS_DieAtCombat DieAtCombat ;
		DieAtCombat.m_uidActor = pEnemy->GetUID();
		pEnemy->GetCreature()->OnEvent(msgID,&DieAtCombat,sizeof(DieAtCombat));		
	}



}

//获得攻击力浮动系数
float Combat::GetAttackFloatParam()
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

//获得当前发动物理攻击参战人
Fighter* Combat::GetLaunchFighter()
{
	Fighter * pFighter = 0;

	bool bEnd = false;
	while(1)
	{
		if(m_vectSortFighter.empty())
		{
			m_vectSortFighter = SortByPrority(m_vectOneself,m_vectEnmey);
			bEnd = true;
		}


		if(!m_vectSortFighter.empty())
		{
			pFighter = m_vectSortFighter[0];
			m_vectSortFighter.erase(m_vectSortFighter.begin());
			if(pFighter->IsDie()==false && pFighter->IsLaunchPhysicsAttacke())
			{
				break;
			}		
		}
		else if(bEnd)
		{
			return 0;
		}
	}

	return pFighter;

}

//获得物理攻击目标
INT32 Combat::GetPhysicsAttackedTarget(INT32 AttackLineup,std::vector<Fighter*> & vectTarget)
{
	//九宫格
	static INT32 s_GirdeCell[3][9] = 
	{
		{0,3,6,      //攻击者在第一行时
		1,4,7,    
		2,5,8},

		{1,4,7,
		0,3,6,     //攻击者在第二行时
		2,5,8},


		{2,5,8,
		1,4,7,
		0,3,6},  //攻击者在第三行时
	};

	if(AttackLineup<0 || AttackLineup>8)
	{
		TRACE("<error> %s : %d line 源攻击者阵型号错误 lineup[%d]",__FUNCTION__,__LINE__,AttackLineup);
		return 0;
	}

	//攻击者行
	INT32 row = AttackLineup/3;

	INT32 minValue = INT_MAX;

	INT32 index = 0;

	for(int i=0; i<vectTarget.size();i++)
	{
		INT32 lineup = vectTarget[i]->GetPos();

		if(lineup>8)
		{
			TRACE("<error> %s : %d line 目标攻击者阵型号错误 lineup[%d]",__FUNCTION__,__LINE__,lineup);
			continue;
		}

		if(s_GirdeCell[row][lineup]<minValue)
		{
			minValue = s_GirdeCell[row][lineup];
			index = i;
		}
	}


	return index;
}

//获得法术攻击目标
INT32 Combat::GetMagicAttackedTarget(INT32 AttackLineup,std::vector<Fighter*> & vectTarget)
{
	//九宫格
	static INT32 s_GirdeCell[3][9] = 
	{
		{0,1,2,      //攻击者在第一行时
		3,4,5,    
		6,7,8},

		{3,4,5,
		0,1,2,     //攻击者在第二行时
		6,7,8},


		{6,7,8,
		3,4,5,
		0,1,2},  //攻击者在第三行时
	};

	if(AttackLineup<0 || AttackLineup>8)
	{
		TRACE("<error> %s : %d line 源攻击者阵型号错误 lineup[%d]",__FUNCTION__,__LINE__,AttackLineup);
		return 0;
	}

	//攻击者行
	INT32 row = AttackLineup/3;

	INT32 minValue = INT_MAX;

	INT32 index = 0;

	for(int i=0; i<vectTarget.size();i++)
	{
		INT32 lineup = vectTarget[i]->GetPos();

		if(lineup>8)
		{
			TRACE("<error> %s : %d line 目标攻击者阵型号错误 lineup[%d]",__FUNCTION__,__LINE__,lineup);
			continue;
		}

		if(s_GirdeCell[row][lineup]<minValue)
		{
			minValue = s_GirdeCell[row][lineup];
			index = i;
		}
	}


	return index;
}



//检查胜负
bool Combat::CheckWinLose()
{
	bool bWin = false;

	if(m_vectOneself.empty())
	{
		bWin = false;

	}
	else if(m_vectEnmey.empty())
	{

		bWin = true;
	}
	else
	{
		return false;
	}


	m_bEnd = true;


	//关闭定时器
	for(int i=0;i<m_vectOneself.size();i++)
	{
		m_vectOneself[i]->Stop();
	}

	for(int i=0;i<m_vectEnmey.size();i++)
	{
		m_vectEnmey[i]->Stop();
	}

	g_pGameServer->GetTimeAxis()->KillAllTimer(this);
	g_pGameServer->GetEventServer()->RemoveAllListener((IVoteListener*)this);
	g_pGameServer->GetEventServer()->RemoveAllListener((IEventListener*)this);
	g_pGameServer->GetEventServer()->RemoveAllListener((IActionListener*)this);



	SCombatCombatPlayBack CombatPlayBack ;
	CombatPlayBack.m_ActionNum = this->m_ActionNum+1;

	CombatCombatOver CombatOver;

	CombatOver.m_bWin = bWin;

	SCombatContext CombatCnt;
	CombatCnt.CombatID = this->m_CombatID;
	CombatCnt.uidEnemy = this->m_uidEnmey;
	CombatCnt.uidSource = this->m_uidOneself;
	CombatCnt.DropID    = this->m_DropID;
	CombatCnt.vectActor.resize(this->m_vectAllFighter.size());
	for(int i=0; i<this->m_vectAllFighter.size();i++)
	{
		CombatCnt.vectActor[i] =  this->m_vectAllFighter[i];
	}


	CombatCnt.ob << SCombatHeader(enCombatCmd_CombatPlayBack,sizeof(CombatPlayBack)+sizeof(CombatOver)+m_obAction.Size()) << CombatPlayBack << m_obAction << CombatOver;


	SCombatCombat CombatCombat;

	CombatCombat.m_CombatID = this->m_CombatID;
	CombatCombat.m_SubCmd = enCombatSubCmd_SC_Over;


	for(int i=0; i<m_vectAllFighter.size();i++)
	{

		Fighter * pFighter = m_vectAllFighter[i];
		ICreature * pCreature = pFighter->GetCreature();
		if(pCreature->GetThingClass() == enThing_Class_Actor)
		{
			IActor * pActor = (IActor*)pCreature;
			if(pActor->GetMaster()==0)
			{
				if(pFighter->IsOneSelf())
				{
					CombatOver.m_bWin = !m_vectOneself.empty();
					CombatCnt.bIsTeam = ((m_JoinPlayer&enJoinPlayer_OneselfTeam)==enJoinPlayer_OneselfTeam);
				}
				else
				{
					CombatOver.m_bWin = !m_vectEnmey.empty();		
					CombatCnt.bIsTeam = ((m_JoinPlayer&enJoinPlayer_EnemyTeam)==enJoinPlayer_EnemyTeam);
				}

				if(CombatOver.m_bWin)
				{
					CombatOver.m_GetExp = pFighter->GetGiveExp();
					CombatOver.m_nGoodsNum = 0;
				}

				std::vector<TGoodsID> vectGoodsID;

				CombatCnt.DamageValue = pFighter->GetDamageValueToBoss();
				CombatCnt.uidUser = pActor->GetUID();
				CombatCnt.bIsAutoFighte = pFighter->IsAutoFighte();
				CombatCnt.Param = m_Param;

				if(m_pCombatObserver)
				{					
					m_pCombatObserver->OnCombatOver(&CombatCnt, &CombatOver,vectGoodsID);
				}		 

				if(pFighter->IsNeedSync())
				{
					OBuffer4k obOver;
					CombatOver.m_bWin = bWin;
					CombatOver.m_nGoodsNum = vectGoodsID.size();

					obOver << SCombatHeader(enCombatCmd_Combat,sizeof(CombatCombat)+sizeof(CombatOver)+sizeof(TGoodsID)*vectGoodsID.size()) << CombatCombat << CombatOver;

					if(CombatOver.m_nGoodsNum>0)
					{
						obOver.Push(&vectGoodsID[0],sizeof(TGoodsID)*vectGoodsID.size());

						for(int i=0; i<vectGoodsID.size();i++)
						{
							pActor->SendGoodsCnfg(vectGoodsID[i]);						  
						}			

					}

					//TRACE("给玩家[%s]发送战斗结束消息!",pFighter->GetCreature()->GetName());
					pFighter->SendToClient(obOver.TakeOsb());

				}

				IActor * pUser = g_pGameServer->GetGameWorld()->FindActor(pActor->GetUID());

				if ( 0 == pUser )
					continue;

				pUser->SetIsInCombat(false);

				IPacketPart * pPacketPart = pUser->GetPacketPart();

				if ( 0 == pPacketPart )
					continue;

				const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

				if( !pFighter->IsAutoFighte() && pPacketPart->GetSpace() <= GameParam.m_Capacity_TipPopUpBox)
				{
					//弹出框提示背包即将不足
					g_pGameServer->GetGameWorld()->TipPopUpBox(pActor, enGameWorldRetCode_PacketFew);
				}
			}
		}
	}

	m_pCombatObserver->BackOver();

	TRACE_COMBAT("战斗结束 CombatID=%lld",CombatCombat.m_CombatID);

	g_pGameServer->GetCombatServer()->DeleteCombat(this);

	/*for ( int i = 0; i < m_vectAllFighter.size(); ++i )
	{
		IFighter * pFighter = m_vectAllFighter[i];

		if ( 0 == pFighter )
			continue;

		delete pFighter;
	}

	m_vectAllFighter.clear();*/

	return true;
}

//移除玩家
void  Combat::Remove(Fighter * pFighter)
{

	if(pFighter->IsOneSelf())
	{
		std::vector<Fighter*>::iterator it = std::find(m_vectOneself.begin(),m_vectOneself.end(),pFighter);
		if(it != m_vectOneself.end())
		{
			m_vectOneself.erase(it);

			TRACE_COMBAT("我方:%s[%s]死亡，还剩余 %d 人",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),m_vectOneself.size());
		}
		else
		{
			TRACE_COMBAT("找不到我方:%s[%s]死亡人员，还剩余 %d 人",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),m_vectOneself.size());
		}
	}
	else
	{
		std::vector<Fighter*>::iterator it = std::find(m_vectEnmey.begin(),m_vectEnmey.end(),pFighter);
		if(it != m_vectEnmey.end())
		{
			m_vectEnmey.erase(it);
			TRACE_COMBAT("敌方:%s[%s]死亡，还剩余 %d 人",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),m_vectEnmey.size());
		}
		else
		{
			TRACE_COMBAT("找不到敌方:%s[%s]死亡，还剩余 %d 人",pFighter->GetCreature()->GetName(),pFighter->GetUID().ToString(),m_vectEnmey.size());
		}
	}
}
