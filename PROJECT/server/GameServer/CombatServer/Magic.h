
#ifndef __COMBATSERVER_MAGIC_H__
#define __COMBATSERVER_MAGIC_H__

#include "IMagic.h"

#include <vector>
#include "ICombatPart.h"

struct SMagicCnfg;

class Magic : public IMagic
{
public:
	Magic();
	virtual ~Magic();

public:

	//获取技能ID
    virtual TMagicID  GetMagicID();

	//获取技能等级
	virtual UINT8  GetLevel();

		//获得配置信息
	const SMagicLevelCnfg*  GetMagicLevelCnfg(); 
	
	// 创建技能
	// 参数：pResObject 该技能的资源对象
	virtual bool Create(const SMagicLevelCnfg* pMagicConfig);

	// 释放
	virtual void Release() ;

	//加载
	virtual bool OnEquip(IActor * pActor);

			//取下
	virtual bool OnUnEquip(IActor * pActor); 

	// 判断生物能否使用该技能
	virtual bool CanUseSkill(ICreature * pCreature,UID uidTarget);
	
	// 使用该等级技能
		virtual bool UseSkill(UINT64 CombatID,INT32 RoundNum,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
		std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
		OBuffer4k & ob,INT32 & RecordNum);

		//对指定对象使用攻击技能
     bool UseSkillToSingleObjectWithAttack(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource,float fScopeParam,IFighter * pCreature,
		 SAttackedTarget & AttackedTarget,INT32 & AddBloodValue,OBuffer4k & obStauts,INT32 & RecordNum, bool & bDecreaseCount);

	 		//对指定对象使用治疗技能
     bool UseSkillToSingleObjectWithCure(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource,float fScopeParam,IFighter * pCreature,
		 SAddBloodTarget & AddBloodTarget,OBuffer4k & obStauts,INT32 & RecordNum);

		//升级
	virtual bool Upgrade() ;

	//设置等级
	virtual bool SetLevel(UINT8 level);

	//获得CD时间
	virtual INT32 GetCDTime();

	virtual const SMagicCnfg * GetMagicCnfg();

private:
	  //法术攻击是否命中
  bool IsMagicAttackHit(const SMagicCnfg * pMagicCnfg,IFighter * pSource,IFighter * pEnemy);

  //获得法术爆击参数
  float GetMagicKnockingParam(IFighter * pSource,IFighter * pEnemy);

    //获得基础攻击力
  INT32 GetBaseAttackValue(const SMagicCnfg * pMagicCnfg,ICreature * pSource);

  std::vector<IFighter*> GetAttackedTarget(const SMagicCnfg * pMagicCnfg,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
					  std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex);

  std::vector<IFighter*> Magic::GetAttackedTarget(const SMagicCnfg * pMagicCnfg,
														  std::vector<IFighter*> & vectCreature,INT32 FirstIndex);

  //获得生物血量上限
  INT32 GetBloodUp(ICreature * pCreature);

  //使用攻击型技能
bool  UseSkillWithAttack(UINT64 CombatID,INT32 RoundNum,const SMagicCnfg * pMagicCnfg ,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
					  std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
					  OBuffer4k & ob,INT32 & RecordNum);

 //使用治疗型技能
bool  UseSkillWithCure(UINT64 CombatID,INT32 RoundNum,const SMagicCnfg * pMagicCnfg ,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
					  std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
					  OBuffer4k & ob,INT32 & RecordNum);


//获得法术基础命中率
float GetMagicBaseHitRate(IFighter * pSource,IFighter * pEnemy);

//获得玩家法术伤害 
INT32 GetActorMagicDamage(ICreature * pCreature);

//计算法术最后的伤害值
INT32 CalculateMagicDamage(float fScopeParam,INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource,IFighter * pEnemy,
						   INT32 BaseDamageValue,float MagicKnockingParam);

 const SStatusTypeCnfg * GetMaxPriorityStatusTypeCnfg(ICreature * pCreature);

 //获得五行参数
float GetWuXinParam(INT32 RoundNum,const SMagicCnfg * pMagicCnfg,IFighter * pSource);

  //获得生物防御值
  INT32 GetDefenseValue(ICreature * pCreature);

  //获得剑诀伤害值
INT32 GetSwordizeDamageValue(ICreature * pCreature,UINT8 WuXing);

     //获得攻击力浮动系数
  float GetAttackFloatParam();

  //获取状态回合参数pSource攻方,pCreature守方
float GetStatusRoundParam(ICreature * pSource,ICreature * pCreature);

//获取持续伤害基本值pSource攻方,pCreature守方
INT32 GetBasePersistDamage(ICreature * pSource,ICreature * pCreature);

//计算范围参数
float CalculateScopeParam(INT32 TargetNum);

  //计算免伤值
  float  CalculateAvoidDamageValue(ICreature * pSource,ICreature * pEnemy);

private:
	const SMagicLevelCnfg*  m_pMagicLevelCnfg;

};




#endif

