#ifndef __RELATIONSERVER_SENDWORLDNOTICE_H__
#define __RELATIONSERVER_SENDWORLDNOTICE_H__

//全服公告
#include "GameSrvProtocol.h"
#include "IVisitWorldThing.h"
#include "string.h"
#include "IChengJiuPart.h"

//传音符
class SendWorldNotice : public IVisitWorldThing
{
public:
	SendWorldNotice(IActor * pActor, const char * pszNoticeContext)
	{
		if ( pszNoticeContext == 0){
			
			return;
		}

		if ( 0 == pActor){

			//fly add 20121029
			//const std::vector<SLanguage> & vectLanguage = g_pGameServer->GetConfigServer()->GetLanguageCnfg();
			//const std::string & strName9= vectLanguage[9].m_Language;
			//strncpy(m_Rsp.m_szName, strName9.c_str(), sizeof(m_Rsp.m_szName));
			strncpy(m_Rsp.m_szName, "系统", sizeof(m_Rsp.m_szName));

		}else{
			
			strncpy(m_Rsp.m_szName, pActor->GetName(), sizeof(m_Rsp.m_szName));
		}
		
		strncpy(m_Rsp.m_szNoticeContext, pszNoticeContext, sizeof(m_Rsp.m_szNoticeContext));
	}

	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing || enThing_Class_Actor != pThing->GetThingClass()){
			return;
		}

		IActor * pActor = (IActor *)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_WorldNotice, sizeof(m_Rsp)) << m_Rsp;
		pActor->SendData(ob.TakeOsb());
	}

private:
	SC_WorldNotice_Rsp	m_Rsp;
};

//聊天框系统公告
class SendTalkSysMsg : public IVisitWorldThing
{
public:
	SendTalkSysMsg(const char * pszMsgContext){

		strncpy(m_Rsp.m_szSysMsg, pszMsgContext, sizeof(m_Rsp.m_szSysMsg));
		m_Rsp.m_Channel = enTalkChannel_World;
		m_Rsp.m_TalkMsgType = enTalkMsgType_System;
	}

	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing || enThing_Class_Actor != pThing->GetThingClass()){
			return;
		}

		IActor * pActor = (IActor *)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SC_TalkSysMsg, sizeof(m_Rsp)) << m_Rsp;
		pActor->SendData(ob.TakeOsb());
	}

private:
	SC_TalkSysMsg	m_Rsp;
};

//跑马灯系统公告
class SendMainSysMsg : public IVisitWorldThing
{
public:
	SendMainSysMsg(UINT32 MsgID, const char * pszMsgContent, UINT32 nRemainTime)
	{
		m_ViewMsg.m_MsgType = enMsgType_System;

		m_HouTaiMsg.m_ID = MsgID;
		m_HouTaiMsg.m_RemainTime = nRemainTime;
		strncpy(m_HouTaiMsg.m_szMsgBody, pszMsgContent, sizeof(m_HouTaiMsg.m_szMsgBody));
	}

	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing || enThing_Class_Actor != pThing->GetThingClass()){
			return;
		}

		IActor * pActor = (IActor *)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SC_ViewSysMsg, sizeof(m_ViewMsg) + sizeof(m_HouTaiMsg)) << m_ViewMsg << m_HouTaiMsg;
		pActor->SendData(ob.TakeOsb());
	}

private:
	HouTaiMsg		m_HouTaiMsg;

	SC_ViewSystemMsg m_ViewMsg;
};

//取消显示后台跑马灯
class SendCancelViewHouTaiMsg : public IVisitWorldThing
{
public:
	SendCancelViewHouTaiMsg(UINT32 MsgID){
		m_Msg.m_MsgID = MsgID;
	}

	virtual void Visit(IThing * pThing)
	{
		if( 0 == pThing || enThing_Class_Actor != pThing->GetThingClass()){
			return;
		}

		IActor * pActor = (IActor *)pThing;

		if( 0 != pActor->GetMaster()){
			//非玩家
			return;
		}

		OBuffer1k ob;
		ob << TalkHeader(enTalkCmd_SC_CancelHouTaiMsg, sizeof(m_Msg)) << m_Msg;
		pActor->SendData(ob.TakeOsb());
	}

private:
	SC_CancelViewHouTaiMsg m_Msg;
};

#endif
