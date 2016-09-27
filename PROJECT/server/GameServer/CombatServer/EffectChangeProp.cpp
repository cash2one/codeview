#include "ICreature.h"
#include "EffectChangeProp.h"

#include "IConfigServer.h"

EffectChangeProp::EffectChangeProp()
{
	m_VarPropValue = 0;
}


EffectChangeProp::~EffectChangeProp()
{
}


		// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectChangeProp::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
{
	if(Effect::Start(pMaster,uidCreator,pStatusBelong)==false)
	{
		return true;
	}

	//当前属性值
	INT32 nCurValue = m_pMaster->GetCrtProp((enCrtProp)m_pEffectCnfg->m_PropID);
	
	//变动值
	INT32 VarValue = 0;

	//类型
	switch(m_pEffectCnfg->m_ValueType)
	{
	case enEffectValueType_Fixed:  //变动值为固定值
		{
			VarValue = m_pEffectCnfg->m_Value;
		}
		break;
	case enEffectValueType_Scale:  //变动百分比
		{
			VarValue = (float)nCurValue*(m_pEffectCnfg->m_Value)/1000 + 0.9999;
		}
		break;
	case enEffectValueType_Final: //最值终
		{
			VarValue = m_pEffectCnfg->m_Value - nCurValue;
		}
		break;
	case enEffectValueType_PropScale:
		{
			INT32 PropValue = m_pMaster->GetCrtProp((enCrtProp)m_pEffectCnfg->m_CommonParam);
			VarValue = (float)PropValue*(m_pEffectCnfg->m_Value)/1000 + 0.9999;
		}
		break;

	default:
		return false;
	}

	INT32 NewValue = 0;

	pMaster->AddCrtPropNum((enCrtProp)m_pEffectCnfg->m_PropID,VarValue,&NewValue);

	m_VarPropValue = NewValue - nCurValue;

	return true;
}

// 效果结束
bool EffectChangeProp::End()
{
	if(Effect::End() == false){
		return false;
	}

	m_pMaster->AddCrtPropNum((enCrtProp)m_pEffectCnfg->m_PropID,-m_VarPropValue);

	return true;
}
