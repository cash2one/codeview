#include "IActor.h"
#include "EventActivity.h"
#include "ThingServer.h"
#include "DMsgSubAction.h"
#include "IConfigServer.h"
#include "IActivityPart.h"
#include "IBasicService.h"
#include "XDateTime.h"



EventActivity::EventActivity()
{
	m_pActivityCnfg = 0;
}

EventActivity::~EventActivity()
{
}


void EventActivity::OnEvent(XEventData & EventData)
{
		if ( !this->IsInActivityTime()){
			//不在活动有效期内
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
		IActivityPart * pActivityPart = pActor->GetActivityPart();
		if(pActivityPart == 0)
		{
			return ;
		}
	
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pActivityCnfg->m_EventID);
				
		if(EventData.m_MsgID == msgID)
		{	  			
				//满足条件	
			if(g_pThingServer->IsEqual(EventData,m_pActivityCnfg->m_EventID,m_pActivityCnfg->m_vectParam)==false)
			{
				return ;
			}

			pActivityPart->AdvanceProgress(m_pActivityCnfg->m_ActivityID);
		}		
}


bool EventActivity::Create(const  SActivityCnfg * pActivityCnfg, bool bRegistEvent)
{
	if(pActivityCnfg == 0)
	{
		return false;
	}

	const  SActivityCnfg * pTmpActivityCnfg = g_pGameServer->GetConfigServer()->GetActivityCnfg(pActivityCnfg->m_ActivityID);
	if( 0 == pTmpActivityCnfg){
		TRACE("<error> %s : %d 获取活动配置信息有错!! id=%d", __FUNCTION__, __LINE__, pActivityCnfg->m_ActivityID);
		return false;
	}

	m_pActivityCnfg = pTmpActivityCnfg;

	if( bRegistEvent)
	{
		//注册事件
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pActivityCnfg->m_EventID);

		g_pGameServer->GetEventServer()->AddListener(this,msgID,enEventSrcType_Actor,0,"EventActivity::Create");	
	}

	return true;
}

void EventActivity::Release()
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,m_pActivityCnfg->m_EventID);
	g_pGameServer->GetEventServer()->RemoveListener(this,msgID,enEventSrcType_Actor,0);

	delete this;
}

const  SActivityCnfg * EventActivity::GetActivityCnfg()
{
	return m_pActivityCnfg;
}

//是否在活动有效期内
bool	EventActivity::IsInActivityTime()
{
	UINT32 nCurTime = CURRENT_TIME();

	if ( nCurTime <= m_pActivityCnfg->m_EndTime && nCurTime >= m_pActivityCnfg->m_BeginTime){
		
		return true;
	}

	return false;
}








