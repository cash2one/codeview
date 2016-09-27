
#include "ICreature.h"
#include "EffectAddBuildingRes.h"
#include "DMsgSubAction.h"

#include "IConfigServer.h"

EffectAddBuildingRes::EffectAddBuildingRes()
{
}

EffectAddBuildingRes::~EffectAddBuildingRes()

{
}

// 效果开始启动
// pMaster 本效果作用生物
// UID uidCreator 产生者UID
// pStatusBelong 效果属于的状态
// 返回值：BOOL - 操作成功则返回TRUE，否则返回FASLE
bool EffectAddBuildingRes::Start(ICreature *pMaster, UID uidCreator, IStatus *pStatusBelong)
{
	if(Effect::Start(pMaster,uidCreator,pStatusBelong)==false)
	{
		return true;
	}

	//注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TakeBuildingRes);

	m_pMaster->SubscribeEvent(msgID,this,"EffectAddBuildingRes::Start");

	return true;
}

void EffectAddBuildingRes::OnEvent(XEventData & EventData)
{
	SS_TakeBuildingRes * pTakeBuildingRes = (SS_TakeBuildingRes *)EventData.m_pContext;
	if( 0 == pTakeBuildingRes){
		return;
	}

	pTakeBuildingRes->m_AddValue = m_pEffectCnfg->m_Value;
}
