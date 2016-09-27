
#ifndef __COMBATSERVER_IEFFECT_H__
#define __COMBATSERVER_IEFFECT_H__

#include "DSystem.h"
#include "UniqueIDGenerator.h"

#include <vector>

struct ICreature;
struct IStatus;
struct SEffectCnfg;

// 效果类型ID
enum enEffectType VC_PACKED_ONE
{
	enEffectType_Non = 0,

	enEffectType_ChangeProp,   //增减属性

	enEffectType_PersistDamage, //持续伤害

	enEffectType_DirectDamage, //直接伤害

	enEffectType_ForbidMagic, //禁止施放法术

	enEffectType_ForbidPhysics, //禁止施放物理攻击

	enEffectType_ChangeTriggerProb, //改变触发几率

	enEffectType_ChangeAttack,  //改变攻击力

	enEffectType_AddStatus,     //增加状态

	enEffectType_SuckBlood,   //吸血

	enEffectType_Cure,         //治疗

	enEffectType_ReturnBlood,         //回血

	enEffectType_AvoidDie,         //免死

	enEffectType_Max,

} PACKED_ONE;

struct IEffect
{
		// 释放
	virtual void Release() = NULL;

	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
	virtual bool Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong = NULL) = NULL;

	// 效果结束
	virtual bool End() = NULL;

	// 获取效果ID
	virtual TEffectID GetEffectID() = NULL;

	// 获取效果类型
	virtual enEffectType GetType() = NULL;

	//获得效果配置信息
	virtual const SEffectCnfg * GetEffectCnfg() = 0;

};





#endif
