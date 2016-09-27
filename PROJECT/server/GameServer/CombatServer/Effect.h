
#ifndef __COMBATSERVER_EFFECT_H__
#define __COMBATSERVER_EFFECT_H__

#include "IEffect.h"

class Effect : public IEffect
{
public:
	Effect();
	virtual ~Effect();

	virtual bool Create(const SEffectCnfg * pEffectCnfg);


public:
		// 释放
	virtual void Release();

	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
	virtual bool Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong = NULL);

	// 效果结束
	virtual bool End();

	// 获取效果ID
	virtual TEffectID GetEffectID();

	// 获取效果类型
	virtual enEffectType GetType();

	//获得效果配置信息
	const SEffectCnfg * GetEffectCnfg();

protected:
	const SEffectCnfg * m_pEffectCnfg;
	ICreature	*m_pMaster;					// 本效果作用生物
	UID			m_uidMaster;				// 本效果作用生物的UID

	UID			m_uidCreator;				// 添加效果的对象UID
	IStatus		*m_pStatusBelong;			// 携带该效果的状态
	bool		m_started;			// 是否已启动

};



#endif
