
#include "IActor.h"
#include "MagicLvSecChengJiu.h"
#include "DMsgSubAction.h"
#include "IConfigServer.h"
#include "IBasicService.h"
#include "IChengJiuPart.h"


bool MagicLvSecChengJiu::Create(const  SChengJiuCnfg * pChengJiuCnfg)
{
	if( !EventChengJiu::Create(pChengJiuCnfg)){
		return false;
	}

	return true;
}

void MagicLvSecChengJiu::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_EventID);

	if( EventData.m_MsgID != msgID || 0 == EventData.m_pContext){
		return;
	}

	if( m_pChengJiuCnfg->m_vectParam.size() < 2){
		TRACE("<error> %s : %d 行，成就配置事件参数有错！！　成就ID=%d", __FUNCTION__, __LINE__, m_pChengJiuCnfg->m_ChengJiuID);
		return;
	}

	//事件源
	if(EventData.m_EventSrcType != enEventSrcType_Actor)
	{
		return ;
	}

	//玩家
	IActor * pActor = (IActor*)EventData.m_MsgSource;

	if(pActor == 0)
	{
		TRACE("<warming> %s : %d line 事件源指针为空!",__FUNCTION__,__LINE__);
		return;
	}


	//判断玩家是否已获得该成就
	IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
	if(pChengJiuPart == 0)
	{
		return ;
	}

	if(pChengJiuPart->IsAttainChengJiu(m_pChengJiuCnfg->m_ChengJiuID))
	{
		return ;
	}

	SS_StudyMagic * pStudyMagic = (SS_StudyMagic *)EventData.m_pContext;

	if( pStudyMagic->m_MagicLevel >= m_pChengJiuCnfg->m_vectParam[0] && pStudyMagic->m_MagicLevel <= m_pChengJiuCnfg->m_vectParam[1]){
		//满足条件
		pChengJiuPart->AdvanceProgress(m_pChengJiuCnfg->m_ChengJiuID);
	}
}
