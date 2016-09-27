#ifndef __RELATIONSERVER_SENDWORLDTALK_H__
#define __RELATIONSERVER_SENDWORLDTALK_H__

#include "GameSrvProtocol.h"
#include <vector>
#include "IVisitWorldThing.h"
#include "RelationServer.h"
#include "IChengJiuPart.h"

struct IActor;

class SendWorldTalk : public IVisitWorldThing
{
public:
	SendWorldTalk(const char * pszSendUserName, const char * pszTalkContent, IActor * pActor)
	{
		if ( 0 == pActor){
			
			return;
		}

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if ( 0 == pChengJiuPart){

			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		m_Rsp.m_SendUserUID = pActor->GetUID();
		m_Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_szTalkContent, pszTalkContent, sizeof(m_Rsp.m_szTalkContent));
		strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
	}

	virtual void Visit(IThing * pThing)
	{
		if(pThing==0 || pThing->GetThingClass() != enThing_Class_Actor/* || pThing == (IThing*)m_pActor*/)
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		if( pActor->GetMaster() != 0){
			//非玩家
			return;
		}
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SycWorldTalk, sizeof(SC_WorldTalk_Rsp)) << m_Rsp;
		pActor->SendData(ob.TakeOsb());
	}

private:
	SC_WorldTalk_Rsp m_Rsp;
};

#endif
