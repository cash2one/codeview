#include "ICreature.h"
#include "EffectCure.h"
#include "DMsgSubAction.h"

#include "IConfigServer.h"

EffectCure::EffectCure()
{
}

EffectCure::~EffectCure()
{
}


	    	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectCure::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
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
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Cure);

	m_pMaster->SubscribeAction(msgID,this,"EffectSuckBlood::Start [enMsgID_Attacked]");

	return true;
}

void EffectCure::OnAction(XEventData & EventData)
{
	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Cure))
	{
		SAttackedContext * pRoundStartCnt = (SAttackedContext*)EventData.m_pContext;

		switch(m_pEffectCnfg->m_ValueType)
		{
		case enEffectValueType_Fixed: //增减固定值
			{
				pRoundStartCnt->m_SuckBloodFactor += m_pEffectCnfg->m_Value;
			}
			break;
		case enEffectValueType_Scale:         //比例(千分之)
			{
				pRoundStartCnt->m_SuckBloodFactor += (float)m_pEffectCnfg->m_Value/1000;
			}
			break;
		case enEffectValueType_Final:        //终值,
			{
				pRoundStartCnt->m_SuckBloodFactor =  m_pEffectCnfg->m_Value;
			}
			break;
		}
	}
}
