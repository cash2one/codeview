#include "ICreature.h"
#include "EffectChangeAttack.h"

EffectChangeAttack::EffectChangeAttack()
{
}

EffectChangeAttack::~EffectChangeAttack()
{
}
	    	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectChangeAttack::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
{
	return true;
}
