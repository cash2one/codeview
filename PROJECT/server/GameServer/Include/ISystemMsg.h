#ifndef __RELATIONSERVER_ISYSTEMMSG_H__
#define __RELATIONSERVER_ISYSTEMMSG_H__

#include "GameSrvProtocol.h"
#include "DBProtocol.h"


struct IActor;

#pragma pack(push,1)

//修炼类消息
struct SXiuLianMsg
{
	enXiuLianType m_XiuLianType;
	UINT32		  m_RemainTime;
};

//效果类消息
struct SEffectMsg
{
	TEffectID	 m_EffectID;
	char		 m_EffectName[THING_NAME_LEN];
	UINT32		 m_RemainTime;
};

//后台类公告
struct HouTaiMsg
{
	UINT32	m_ID;
	char	m_szMsgBody[DESCRIPT_LEN_100];
	UINT32	m_RemainTime;
};

//插入式公告
struct SInsertMsg
{
	char szMsgBody[DESCRIPT_LEN_150];
};

#pragma pack(pop)

struct ISystemMsg
{
	//通知客户端显示公告消息
	virtual void ViewMsg(IActor * pActor, enMsgType MsgType, void * pMsgInfo) = 0;

	//通知客户端取消显示修炼公告消息
	virtual void CancelViewXiuLianMsg(IActor * pActor, enXiuLianType m_XiuLianType) = 0;

	//通知客户端取消显示效果公告消息
	virtual void CancelViewEffectMsg(IActor * pActor, TEffectID EffectID) = 0;

	//有后台消息到
	virtual void HouTaiMsg_Push(const SDB_SysMsgInfo & DBSysMsgInfo) = 0;
};

#endif
