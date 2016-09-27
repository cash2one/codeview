#include "ICreature.h"
#include "EffectPersistDamage.h"
#include "DMsgSubAction.h"
#include "IConfigServer.h"
#include "CombatServer.h"
#include "IConfigServer.h"
#include "RandomService.h"

EffectPersistDamage::EffectPersistDamage()
{
}

EffectPersistDamage::~EffectPersistDamage()
{
}

// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectPersistDamage::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong)
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
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RoundStart);

	m_pMaster->SubscribeAction(msgID,this,"EffectPersistDamage::Start [enMsgID_RoundStart]");


	return true;
}

	// 效果结束
bool EffectPersistDamage::End()
{
	if(Effect::End()==false)
	{
		return false;
	}

	//取消注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RoundStart);

	m_pMaster->UnsubscribeAction(msgID,this);

	return true;

}

void EffectPersistDamage::OnAction(XEventData & EventData)
{

	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_RoundStart))
	{
		SRoundStartContext * pRoundStartCnt = (SRoundStartContext*)EventData.m_pContext;

		INT32 BaseAttackValue = 1;

		if(m_pStatusBelong) //法宝法术基础攻击力为1
		{
			TMagicID MagicID = m_pStatusBelong->GetMagicID();
			if(MagicID != INVALID_MAGIC_ID)
			{
				const SMagicCnfg * pMagicCnfg = g_pGameServer->GetConfigServer()->GetMagicCnfg(MagicID);
				if(pMagicCnfg && pMagicCnfg->m_Class != enMagicClass_Talisman)
				{
					BaseAttackValue = m_pStatusBelong->GetStatusData();
				}				
			}			
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
				pRoundStartCnt->m_AddDamageValue += BaseAttackValue * (float)m_pEffectCnfg->m_Value/1000;
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
