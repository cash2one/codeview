#include "SystemMsg.h"
#include "IBasicService.h"
#include "IActor.h"
#include "SendWorldNotice.h"
#include "DMsgSubAction.h"
#include "RelationServer.h"
#include "XDateTime.h"



bool SystemMsg::Create()
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return false;
	}

	pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"SystemMsg::Create");	

	g_pGameServer->GetTimeAxis()->SetTimer(enTimerID_SysMsg, this, 60 * 1000, "Talk::StartSysMsg");

	return true;
}


//通知客户端显示公告消息
void SystemMsg::ViewMsg(IActor * pActor, enMsgType MsgType, void * pMsgInfo)
{
	switch(MsgType)
	{
	case enMsgType_XiuLian:
		this->ViewXiuLianMsg(pActor, (const SXiuLianMsg *)pMsgInfo);
		break;
	case enMsgType_Effect:
		this->ViewStatusMsg(pActor, (const SEffectMsg *)pMsgInfo);
		break;
	case enMsgType_System:
		this->ViewHouTaiMsg(pActor, (const HouTaiMsg *)pMsgInfo);
		break;
	case enMsgType_Insert:
		this->ViewInsertMsg(pActor, (const SInsertMsg *)pMsgInfo);
		break;
	default:
		TRACE("<error> %s : %d 行 无效的公告类型！！类型 = %d", __FUNCTION__, __LINE__, MsgType);
		break;
	}
}

//通知客户端取消显示修炼公告消息
void SystemMsg::CancelViewXiuLianMsg(IActor * pActor, enXiuLianType m_XiuLianType)
{
	SC_CancelViewXLMsg CancelMsg;
	CancelMsg.m_XiuLianType = m_XiuLianType;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_CancelXiuLianMsg, sizeof(SC_CancelViewXLMsg)) << CancelMsg;
	pActor->SendData(ob.TakeOsb());
}

//通知客户端取消显示状态公告消息
void SystemMsg::CancelViewEffectMsg(IActor * pActor, TEffectID EffectID)
{
	SC_CancelViewEffectMsg CancelMsg;
	CancelMsg.m_EffectID = EffectID;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_CancelEffectMsg, sizeof(SC_CancelViewEffectMsg)) << CancelMsg;
	pActor->SendData(ob.TakeOsb());
}

//通知客户端取消显示后台公告
void SystemMsg::CancelViewHouTaiMsg(UINT32 MsgID)
{
	SendCancelViewHouTaiMsg CancelMsg(MsgID);
	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, CancelMsg);
}

void SystemMsg::OnEvent(XEventData & EventData)
{
	SS_ActoreCreateContext * pActoreCreateContext = (SS_ActoreCreateContext *)EventData.m_pContext;
	if( 0 == pActoreCreateContext){
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(pActoreCreateContext->m_uidActor));
	if( !pActor){
		return;
	}

	MAP_SYSMSG::iterator iter = m_mapSysMsg.begin();

	for( ; iter != m_mapSysMsg.end(); )
	{
		SSysMsg & SysMsg = iter->second;

		UINT32 nCurTime = CURRENT_TIME();

		if( nCurTime < SysMsg.m_BeginTime){
			++iter;
			continue;
		}

		if( nCurTime >= SysMsg.m_EndTime){
			m_mapSysMsg.erase(iter++);
			continue;
		}

		HouTaiMsg Msg;

		Msg.m_ID = SysMsg.m_MsgID;
		Msg.m_RemainTime = SysMsg.m_EndTime - nCurTime;
		strncpy(Msg.m_szMsgBody, SysMsg.m_szMsgContent, sizeof(Msg.m_szMsgBody));

		this->ViewHouTaiMsg(pActor, &Msg);

		++iter;
	}
}

//有后台消息到
void SystemMsg::HouTaiMsg_Push(const SDB_SysMsgInfo & DBSysMsgInfo)
{
	if( DBSysMsgInfo.m_State == 1){
		//暂停，从列表中删除
		MAP_SYSMSG::iterator iter = m_mapSysMsg.find(DBSysMsgInfo.m_SysMsgID);
		if( iter == m_mapSysMsg.end()){
			return;
		}
		
		//通知所有玩家不显示这条消息
		this->CancelViewHouTaiMsg(iter->first);

		m_mapSysMsg.erase(iter);

		return;
	}else if(DBSysMsgInfo.m_State == 0){
		//启用
		UINT32 nCurTime = CURRENT_TIME();

		SSysMsg SysMsg;

		SysMsg.m_BeginTime = DBSysMsgInfo.m_BeginTime;
		SysMsg.m_EndTime   = DBSysMsgInfo.m_EndTime;
		SysMsg.m_MsgID	   = DBSysMsgInfo.m_SysMsgID;
		SysMsg.m_bSended   = false;
		strncpy(SysMsg.m_szMsgContent, DBSysMsgInfo.MsgContent, sizeof(SysMsg.m_szMsgContent));

		SysMsg.m_szMsgContent[DESCRIPT_LEN_100 - 1] = '\0';

		m_mapSysMsg[DBSysMsgInfo.m_SysMsgID] = SysMsg;

		if( nCurTime >= DBSysMsgInfo.m_BeginTime && nCurTime < DBSysMsgInfo.m_EndTime){

			this->SendWorldHouTaiMsg(DBSysMsgInfo.m_SysMsgID);
		}
	}
}

void SystemMsg::OnTimer(UINT32 timerID)
{
	MAP_SYSMSG::iterator iter = m_mapSysMsg.begin();

	for( ; iter != m_mapSysMsg.end(); )
	{
		SSysMsg & SysMsg = iter->second;

		UINT32 nCurTime = CURRENT_TIME();

		if( nCurTime < SysMsg.m_BeginTime){
			++iter;
			continue;
		}

		if( nCurTime >= SysMsg.m_EndTime){
			m_mapSysMsg.erase(iter++);
			continue;
		}
		
		if( !SysMsg.m_bSended && nCurTime >= SysMsg.m_BeginTime){
			this->SendWorldHouTaiMsg(SysMsg.m_MsgID);
			++iter;
			continue;
		}

		++iter;
	}
}

//通知客户端显示修炼公告消息
void SystemMsg::ViewXiuLianMsg(IActor * pActor, const SXiuLianMsg * pXLMsg)
{
	SC_ViewSystemMsg ViewMsg;
	ViewMsg.m_MsgType = enMsgType_XiuLian;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_ViewSysMsg, sizeof(ViewMsg) + sizeof(SXiuLianMsg)) << ViewMsg << *pXLMsg;
	pActor->SendData(ob.TakeOsb());
}

//通知客户端显示状态公告消息
void SystemMsg::ViewStatusMsg(IActor * pActor, const SEffectMsg * pEffectMsg)
{
	SC_ViewSystemMsg ViewMsg;
	ViewMsg.m_MsgType = enMsgType_Effect;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_ViewSysMsg, sizeof(ViewMsg) + sizeof(SEffectMsg)) << ViewMsg << *pEffectMsg;
	pActor->SendData(ob.TakeOsb());
}

//通知客户端显示插入式公告消息
void SystemMsg::ViewInsertMsg(IActor * pActor, const SInsertMsg * pInsertMsg)
{
	SC_ViewSystemMsg ViewMsg;
	ViewMsg.m_MsgType = enMsgType_Insert;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_ViewSysMsg, sizeof(ViewMsg) + sizeof(SInsertMsg)) << ViewMsg << *pInsertMsg;
	pActor->SendData(ob.TakeOsb());
}

//通知客户显示后台公告消息
void	SystemMsg::ViewHouTaiMsg(IActor * pActor, const HouTaiMsg * pHouTaiMsg)
{
	SC_ViewSystemMsg ViewMsg;
	ViewMsg.m_MsgType = enMsgType_System;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_ViewSysMsg, sizeof(ViewMsg) + sizeof(HouTaiMsg)) << ViewMsg << *pHouTaiMsg;
	pActor->SendData(ob.TakeOsb());
}

//向世界广播后台跑马灯信息
void	SystemMsg::SendWorldHouTaiMsg(UINT32 MsgID)
{
	MAP_SYSMSG::iterator iter = m_mapSysMsg.find(MsgID);
	if( iter == m_mapSysMsg.end()){
		return;
	}

	SSysMsg & SysMsg = iter->second;

	SendMainSysMsg MainSysMsg(SysMsg.m_MsgID, SysMsg.m_szMsgContent, SysMsg.m_EndTime - CURRENT_TIME());

	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, MainSysMsg);

	SysMsg.m_bSended = true;
}
