#include "ICreature.h"
#include "Effect.h"
#include "IStatus.h"
#include "IConfigServer.h"



Effect::Effect()
{
	m_pEffectCnfg = 0;
	m_pMaster     = 0;					// 本效果作用生物
    m_uidMaster   = UID();				// 本效果作用生物的UID

	m_uidCreator  = UID();				// 添加效果的对象UID
	m_pStatusBelong = 0;			// 携带该效果的状态
	m_started = false;			// 是否已启动
}
Effect::~Effect()
{
}

bool Effect::Create(const SEffectCnfg * pEffectCnfg)
{
	if(pEffectCnfg==0)
	{
		return false;
	}

	m_pEffectCnfg = pEffectCnfg;

	return true;
}

// 释放
void Effect::Release()
{
	delete this;
}

// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool Effect::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong)
{
	if (m_started)
	{
		return true;
	}

	if (!pMaster)
	{
		return false;
	}

	m_pMaster = pMaster;
	m_uidMaster = pMaster->GetUID();
	m_uidCreator = uidCreator;

	m_pStatusBelong = pStatusBelong;

	//这里要注册目标消失消息！目标对象如果被销毁了，效果要自动停止，否则m_pMaster等指针全部无效了！


	m_started = true;
	
	return true;
}

// 效果结束
bool Effect::End()
{
	if (!m_started)
	{
		return false;
	}

	if (m_pMaster == NULL)
	{
		return false;
	}

	m_started = false;

	
	return true;
}

// 获取效果ID
TEffectID Effect::GetEffectID()
{
	return m_pEffectCnfg->m_EffectID;
}

// 获取效果类型
enEffectType Effect::GetType()
{
	return (enEffectType)m_pEffectCnfg->m_EffectType;;
}


//获得效果配置信息
const SEffectCnfg * Effect::GetEffectCnfg()
{
     return m_pEffectCnfg;
}
