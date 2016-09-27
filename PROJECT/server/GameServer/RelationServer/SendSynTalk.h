#ifndef __RELATIONSERVER_SENDSYNTALK_H__
#define __RELATIONSERVER_SENDSYNTALK_H__

#include "IVisitSynMember.h"
#include <vector>
#include "RelationServer.h"
#include "IChengJiuPart.h"

class SendSynTalk : public IVisitSynMember
{
public:
	SendSynTalk(const char * pszTalkContent, IActor * pActor)
	{
		if ( 0 == pActor || 0 == pszTalkContent){
			return;
		}

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if( 0 == pChengJiuPart){
			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		m_Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		m_Rsp.m_SendVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_szTalkContent, pszTalkContent, sizeof(m_Rsp.m_szTalkContent));
		strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
	}
	
	//遍历帮派成员，对所有帮派成员调用此函数
	virtual void VisitMember(IThing * pThing)
	{
		if( 0 == pThing || pThing->GetThingClass() != enThing_Class_Actor){
			return;
		}

		IActor * pActor = (IActor *)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SycSynTalk, sizeof(SC_SynTalk_Rsp)) << m_Rsp;

		pActor->SendData(ob.TakeOsb());
	}

private:
	SC_SynTalk_Rsp  m_Rsp;
};

#endif
