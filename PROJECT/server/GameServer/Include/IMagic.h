
#ifndef __COMBATSERVER_IMAGIC_H__
#define __COMBATSERVER_IMAGIC_H__
#include "DSystem.h"
#include "UniqueIDGenerator.h"
#include "ICombatPart.h"
#include <vector>
#include <TBuffer.h>

#if 0
#define TRACE_COMBAT(...) TRACE(__VA_ARGS__);
#else
#define TRACE_COMBAT(...)
#endif

struct IActor;
struct SMagicLevelCnfg;
struct IFighter;

struct IMagic
{
	//获取技能ID
    virtual TMagicID  GetMagicID() = 0;

	//获取技能等级
	virtual UINT8  GetLevel() = 0;

	//获得配置信息
    virtual const SMagicLevelCnfg*  GetMagicLevelCnfg() = 0;
	
	// 创建技能
	virtual bool Create(const SMagicLevelCnfg* pMagicConfig) = 0;

	// 释放
	virtual void Release() = 0;

	//加载
	virtual bool OnEquip(IActor * pActor) = 0;

		//取下
	virtual bool OnUnEquip(IActor * pActor) = 0;

	// 判断生物能否使用该技能
	virtual bool CanUseSkill(ICreature * pCreature,UID uidTarget) = 0;
	
	// 使用该等级技能
	virtual bool UseSkill( UINT64 CombatID,INT32 RoundNum,std::vector<IFighter*> & vectSelf,INT32 SourceIndex,
		std::vector<IFighter*> & vectEnemy,INT32 EnemyIndex,
		OBuffer4k & ob,INT32 & RecordNum) = 0;

	//升级
	virtual bool Upgrade() = 0;

	//设置等级
	virtual bool SetLevel(UINT8 level) = 0;

	//获得CD时间
	virtual INT32 GetCDTime() = 0;

	virtual const SMagicCnfg * GetMagicCnfg() = 0;

};




#endif

