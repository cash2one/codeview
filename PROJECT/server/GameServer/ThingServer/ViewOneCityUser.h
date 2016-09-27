#ifndef __THINGSERVER_VIEWONECITYUSER_H__
#define __THINGSERVER_VIEWONECITYUSER_H__

#include "GameSrvProtocol.h"
#include "IActor.h"
#include "string.h"
#include "ThingServer.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IVisitWorldThing.h"
#include "IChengJiuPart.h"

class ViewOneCityUser : public IVisitWorldThing
{
public:
	ViewOneCityUser(IActor * pActor = 0)
	{
		m_pActor = pActor;
	}

	virtual void Visit(IThing * pThing)
	{
		if( pThing==0 || pThing->GetThingClass() != enThing_Class_Actor
			|| pThing == (IThing*)m_pActor)
		{
			return;
		}
		
		IActor * pActor = (IActor*)pThing;

		if( pActor->GetMaster() != 0){
			return;
		}

		if( m_pActor->GetCrtProp(enCrtProp_ActorCityID) == pActor->GetCrtProp(enCrtProp_ActorCityID)){
			m_vectActor.push_back(pActor);
		}	
	}

	void SendViewMsg()
	{
		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return;
		}

		SC_OneCityOnlineUser_Rsp Rsp;
		Rsp.m_nFriendNum = m_vectActor.size();

		OBuffer1k ob;
		ob << FriendHeader(enFriendCmd_ViewOneCityOnlineUser, sizeof(Rsp) + Rsp.m_nFriendNum * sizeof(SOneCityUserDataRsp)) << Rsp;

		for( int i = 0; i < m_vectActor.size(); ++i)
		{
			IActor * pTmpActor = m_vectActor[i];

			SOneCityUserDataRsp UserDataRsp;
			UserDataRsp.m_Level = pTmpActor->GetCrtProp(enCrtProp_Level);
			UserDataRsp.m_Sex	= pTmpActor->GetCrtProp(enCrtProp_ActorSex);
			UserDataRsp.m_Facade= pTmpActor->GetCrtProp(enCrtProp_ActorFacade);
			strncpy(UserDataRsp.m_szUserName, pTmpActor->GetName(), sizeof(UserDataRsp.m_szUserName));
			UserDataRsp.m_uidUser = pTmpActor->GetUID().ToUint64();

			IChengJiuPart * pChengJiuPart = pTmpActor->GetChengJiuPart();
			if( 0== pChengJiuPart){
				continue;
			}

			UserDataRsp.m_TitleID = pChengJiuPart->GetTitleID();

			ISyndicate * pSyn = pSynMgr->GetSyndicate(pTmpActor->GetUID());
			if( 0 == pSyn){
				UserDataRsp.m_szSynName[0] = '\0';
			}else{
				strncpy(UserDataRsp.m_szSynName, pSyn->GetSynName(), sizeof(UserDataRsp.m_szSynName));
			}

			ob << UserDataRsp;
		}
		m_pActor->SendData(ob.TakeOsb());
	}

private:
	IActor     * m_pActor;

	std::vector<IActor *>   m_vectActor;	//同城的玩家指针

};

#endif
