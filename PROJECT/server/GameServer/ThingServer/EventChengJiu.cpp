
#include "IActor.h"
#include "EventChengJiu.h"
#include "ThingServer.h"
#include "DMsgSubAction.h"
#include "IConfigServer.h"
#include "IChengJiuPart.h"
#include "IBasicService.h"


EventChengJiu::EventChengJiu()
{
	m_pChengJiuCnfg = 0;
}


EventChengJiu::~EventChengJiu()
{
}

void EventChengJiu::OnEvent(XEventData & EventData)
{
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


		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_EventID);
		UINT32 ResetmsgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_ResetEventID);

		
		if(EventData.m_MsgID == msgID)
		{	  			
				//满足条件	
			if(g_pThingServer->IsEqual(EventData,m_pChengJiuCnfg->m_EventID,m_pChengJiuCnfg->m_vectParam)==false)
			{
				return ;
			}

			pChengJiuPart->AdvanceProgress(m_pChengJiuCnfg->m_ChengJiuID);
		}
		else
		{
			if(g_pThingServer->IsEqual(EventData,m_pChengJiuCnfg->m_EventID,m_pChengJiuCnfg->m_vectResetParam)==false)
			{
				return ;
			}

			pChengJiuPart->ResetProgress(m_pChengJiuCnfg->m_ChengJiuID);
		}
}

bool EventChengJiu::Create(const  SChengJiuCnfg * pChengJiuCnfg)
{
	if(pChengJiuCnfg==0)
	{
		return false;
	}

	m_pChengJiuCnfg = pChengJiuCnfg;

	//注册事件
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_EventID);

	g_pGameServer->GetEventServer()->AddListener(this,msgID,enEventSrcType_Actor,0,"EventChengJiu::Create");

	if(m_pChengJiuCnfg->m_ResetEventID != 0)
	{
		UINT32 ResetmsgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_ResetEventID);
		g_pGameServer->GetEventServer()->AddListener(this,ResetmsgID,enEventSrcType_Actor,0,"EventChengJiu::Create");
	}

	return true;
}

void EventChengJiu::Release()
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_EventID);
	g_pGameServer->GetEventServer()->RemoveListener(this,msgID,enEventSrcType_Actor,0);

	if(m_pChengJiuCnfg->m_ResetEventID != 0)
	{
		UINT32 ResetmsgID = MAKE_MSGID(CIRCULTYPE_SS,m_pChengJiuCnfg->m_ResetEventID);
		g_pGameServer->GetEventServer()->RemoveListener(this,ResetmsgID,enEventSrcType_Actor,0);
	}

	delete this;
}

const  SChengJiuCnfg * EventChengJiu::GetChengJiuCnfg()
{
	return m_pChengJiuCnfg;
}
