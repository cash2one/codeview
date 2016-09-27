#include "ICreature.h"
#include "EffectForbidMagic.h"

#include "DMsgSubAction.h"

#include "IConfigServer.h"

EffectForbidMagic::EffectForbidMagic()
{
}
EffectForbidMagic::~EffectForbidMagic()
{
}


// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectForbidMagic::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong )
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
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack);

	m_pMaster->SubscribeVote(msgID,this,"EffectForbidMagic::Start [enMsgID_Attack]");


	return true;
}

bool EffectForbidMagic::OnVote(XEventData & EventData)
{
 	if(EventData.m_MsgID == MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Attack))
	{
		SAttackContext * pAttackCnt = (SAttackContext *)EventData.m_pContext;
		if(pAttackCnt->m_AttackType == enAttackType_Magic)
		{
			return false;
		}
	}

	return true;
}
