
#include "ICreature.h"
#include "EffectAvoidDie.h"
#include "DMsgSubAction.h"

#include "IConfigServer.h"

EffectAvoidDie::EffectAvoidDie()
{
}

EffectAvoidDie::~EffectAvoidDie()
{
}


	    	// 效果开始启动
	// pMaster 本效果作用生物
	// UID uidCreator 产生者UID
	// pStatusBelong 效果属于的状态
	// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectAvoidDie::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
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

	m_pMaster->SubscribeAction(msgID,this,"EffectAvoidDie::Start [enMsgID_Attacked]");

	return true;
}

void EffectAvoidDie::OnAction(XEventData & EventData)
{
	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attacked))
	{
		SAttackedContext * pAttackedCnt = (SAttackedContext*)EventData.m_pContext;
		pAttackedCnt->m_bAvoidDie = true;		
	}
}
