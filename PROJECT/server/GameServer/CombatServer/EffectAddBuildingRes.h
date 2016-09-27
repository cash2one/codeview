#ifndef __COMBATSERVER_EFFECTADDBUILDINGRES_H__
#define __COMBATSERVER_EFFECTADDBUILDINGRES_H__
//增加建筑产量

#include "Effect.h"
#include "IEventServer.h"

class EffectAddBuildingRes : public Effect,public IEventListener
{
public:
	EffectAddBuildingRes();
	virtual ~EffectAddBuildingRes();

public:
	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
	virtual bool Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong = NULL);

	virtual void OnEvent(XEventData & EventData);
};


#endif
