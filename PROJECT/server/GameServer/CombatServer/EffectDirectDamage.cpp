#include "ICreature.h"
#include "EffectDirectDamage.h"
#include "IEventServer.h"

#include "DMsgSubAction.h"

#include "IConfigServer.h"
#include "RandomService.h"

EffectDirectDamage::EffectDirectDamage()
{
}

EffectDirectDamage::~EffectDirectDamage()
{
}

// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectDirectDamage::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong)
{
	if(Effect::Start(pMaster,uidCreator,pStatusBelong)==false)
	{
		return false;
	}

	if(m_pMaster==0)
	{
		return false;
	}

	//注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attacked);

	m_pMaster->SubscribeAction(msgID,this,"EffectPersistDamage::Start [enMsgID_Attacked]");

	return true;
}

void EffectDirectDamage::OnAction(XEventData & EventData)
{
	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attacked))
	{
		SAttackedContext * pRoundStartCnt = (SAttackedContext*)EventData.m_pContext;

		if(m_pEffectCnfg->m_Value<0)
		{
			pRoundStartCnt->m_bDefense = true;
		}

		switch(m_pEffectCnfg->m_ValueType)
		{
		case enEffectValueType_Fixed: //增减固定值
			{
				pRoundStartCnt->m_AddDamageValue += m_pEffectCnfg->m_Value;
			}
			break;
		case enEffectValueType_Scale:         //比例(千分之)
			{
				pRoundStartCnt->m_AddDamageValue += pRoundStartCnt->m_BaseDamageValue * (float)m_pEffectCnfg->m_Value/1000 + 0.9999;
			}
			break;
		case enEffectValueType_Final:        //终值,
			{
				pRoundStartCnt->m_AddDamageValue =  m_pEffectCnfg->m_Value;
			}
			break;

		case enEffectValueType_RandomRange:
		{
			INT32 Range = m_pEffectCnfg->m_CommonParam -  m_pEffectCnfg->m_Value;
			if(Range<1)
			{
				Range = 1;
			}

			pRoundStartCnt->m_AddDamageValue += m_pEffectCnfg->m_Value+ (RandomService::GetRandom()% Range);
		}
		break;
		}
	}
}
