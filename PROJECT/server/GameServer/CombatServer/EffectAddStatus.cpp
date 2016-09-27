
#include "ICreature.h"
#include "EffectAddStatus.h"

EffectAddStatus::EffectAddStatus()
{
}

EffectAddStatus::~EffectAddStatus()
{
}

// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectAddStatus::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
{
	if(Effect::Start(pMaster,uidCreator,pStatusBelong)==false)
	{
		return false;
	}
	return true;
}
