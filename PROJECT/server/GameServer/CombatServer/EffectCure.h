

#ifndef __COMBATSERVER_EFFECTCURE_H__
#define __COMBATSERVER_EFFECTCURE_H__


#include "Effect.h"
#include "IEventServer.h"

class EffectCure : public Effect,public IActionListener
{
public:
	EffectCure();
	virtual ~EffectCure();

public:
	    	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
	virtual bool Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong );

	virtual void OnAction(XEventData & EventData);

};



#endif
