
#ifndef __COMBATSERVER_EFFECTADDSTATUS_H__
#define __COMBATSERVER_EFFECTADDSTATUS_H__


#include "Effect.h"

class EffectAddStatus : public Effect
{
public:
	EffectAddStatus();
	virtual ~EffectAddStatus();

public:
	    	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
	virtual bool Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong );



};



#endif
