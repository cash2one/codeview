#ifndef __RELATIONSERVER_SYSTEMMSG_H__
#define __RELATIONSERVER_SYSTEMMSG_H__

#include "ISystemMsg.h"
#include <map>
#include "IEventServer.h"
#include "DBProtocol.h"
#include "ITimeAxis.h"

struct SSysMsg
{
	UINT32		m_MsgID;
	UINT32		m_BeginTime;
	UINT32		m_EndTime;
	char		m_szMsgContent[DESCRIPT_LEN_100];
	bool		m_bSended;
};

class SystemMsg : public ISystemMsg, public IEventListener, public ITimerSink
{
	enum enTimerID
	{
		enTimerID_SysMsg,
	};
public:
	bool Create();

	//通知客户端显示公告消息
	virtual void ViewMsg(IActor * pActor, enMsgType MsgType, void * pMsgInfo);

	//通知客户端取消显示修炼公告消息
	virtual void CancelViewXiuLianMsg(IActor * pActor, enXiuLianType m_XiuLianType);

	//通知客户端取消显示效果公告消息
	virtual void CancelViewEffectMsg(IActor * pActor, TEffectID EffectID);

	//通知客户端取消显示后台公告
	virtual void CancelViewHouTaiMsg(UINT32 MsgID);

	virtual void OnEvent(XEventData & EventData);

	//有后台消息到
	virtual void HouTaiMsg_Push(const SDB_SysMsgInfo & DBSysMsgInfo);

	virtual void OnTimer(UINT32 timerID);

private:
	//通知客户端显示修炼公告消息
	void	ViewXiuLianMsg(IActor * pActor, const SXiuLianMsg * pXLMsg);

	//通知客户端显示效果公告消息
	void	ViewStatusMsg(IActor * pActor, const SEffectMsg * pEffectMsg);

	//通知客户端显示插入式公告消息
	void	ViewInsertMsg(IActor * pActor, const SInsertMsg * pInsertMsg);

	//通知客户显示后台公告消息
	void	ViewHouTaiMsg(IActor * pActor, const HouTaiMsg * pHouTaiMsg);

	//向世界广播后台跑马灯信息
	void	SendWorldHouTaiMsg(UINT32 MsgID);

private:
	typedef std::map<UINT32/*MsgID*/, SSysMsg>	MAP_SYSMSG;

	MAP_SYSMSG			m_mapSysMsg;
};


#endif
