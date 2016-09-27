#ifndef __RELATIONSERVER_SENDZHAOMUSYNMEMBER_H__
#define __RELATIONSERVER_SENDZHAOMUSYNMEMBER_H__

#include "GameSrvProtocol.h"
#include "IActor.h"
#include "RelationServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IVisitWorldThing.h"
#include "IChengJiuPart.h"

class SendZhaoMuSynMember : public IVisitWorldThing
{
public:
	SendZhaoMuSynMember(IActor * pActor)
	{
		ISyndicate * pSyndicate = pActor->GetSyndicate();

		if( 0 == pSyndicate)
		{
			bErr = true;
			return;
		}

		bErr = false;

		m_Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		strncpy(m_Rsp.m_szSendUserName, pActor->GetName(), sizeof(m_Rsp.m_szSendUserName));
		strncpy(m_Rsp.m_SynName, pSyndicate->GetSynName(), sizeof(m_Rsp.m_SynName));
		sprintf_s(m_Rsp.m_szContent, sizeof(m_Rsp.m_szContent), g_pGameServer->GetGameWorld()->GetLanguageStr(10053), pSyndicate->GetSynName());
		m_Rsp.m_SynID = pSyndicate->GetSynID();

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();

		if ( 0 != pChengJiuPart )
		{
			std::string strTitleName = pChengJiuPart->GetTitle();

			strncpy(m_Rsp.m_szTitleName, strTitleName.c_str(), sizeof(m_Rsp.m_szTitleName));
		}
	}

	virtual void Visit(IThing * pThing)
	{
		if( bErr || pThing==0 || pThing->GetThingClass() != enThing_Class_Actor)
		{
			return;
		}

		IActor * pActor = (IActor*)pThing;

		if( 0 != pActor->GetMaster() ){
			return;
		}
		
		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_ZhaoMuSynMember, sizeof(m_Rsp)) << m_Rsp;

		pActor->SendData(ob.TakeOsb());	
	}

private:
	SC_ZhaoMuSynMember_Rsq m_Rsp;

	bool bErr;
};

#endif
