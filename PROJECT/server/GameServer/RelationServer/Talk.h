#ifndef __RELATIONSERVER_TALK_H__
#define __RELATIONSERVER_TALK_H__

#include "IMessageDispatch.h"
#include "ThingContainer.h"
#include "ICDTime.h"
#include <set>
#include "ITalk.h"
#include "IDBProxyClient.h"
#include "IConfigServer.h"
#include "ITimeAxis.h"

//使用GM命令
struct UseControlCmd
{
	UseControlCmd(){
		m_pGMCmdCnfg = 0;
	}

	SGMCmdCnfg * m_pGMCmdCnfg;
	char		 m_szPermission[DESCRIPT_LEN_100];
};

struct STalkSysMsg
{
	UINT32		m_MsgID;
	UINT32		m_BeginTime;
	UINT32		m_EndTime;
	UINT32		m_IntervalTime;	//发送间隔时间，单位：秒(聊天框用到)
	char		m_szMsgContent[DESCRIPT_LEN_100];
	bool		m_bUseTimer;	//是否正在使用定时器
};

struct IActor;

class Talk : public ITalk, public IMsgRootDispatchSink, public IDBProxyClientSink, public ITimerSink
{
public:
	Talk();
	~Talk();

	bool Create();
	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//帮派招募成员
	void	ZhaoMuSynMember(IActor *pActor);

	//传音符全服公告
	virtual void  WorldNotice(IActor * pActor, const char * pszNoticeContext);

	//世界频道的系统消息(pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
	virtual void  WorldSystemMsg(const char * pszMsgContext, IActor * pActor = 0, enTalkMsgType enMsgType = enTalkMsgType_System);

	//右上角的提示悬浮框
	virtual void  SendTipBox(IActor * pActor, const char * pszTip);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//加入到禁言中
	virtual void DontTalk_Push(SDB_DontTalk & DBDontTalk);

	//加入聊天框的系统公告
	virtual void TalkSysMsg_Push(SDB_SysMsgInfo & DBSysMsg);

	virtual void OnTimer(UINT32 timerID);

	//对玩家发送聊天框系统消息
	void	SendUserTalkSysMsg(IActor * pActor, const char * pszMsgContent, enTalkChannel enChannel = enTalkChannel_World, enTalkMsgType enMsgType = enTalkMsgType_System);

	//对世界玩家发送聊天框系统消息
	void	SendWorldTalkSysMsg(const char * pszMsgContent);

private:
	//私人聊天
	void	PrivateTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//组队聊天
	void	TeamTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//世界聊天
	void	WorldTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//点击物品超链接查看物品属性
	void	ViewItem(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//帮派聊天
	void	SynTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//检测并处理GM命令,返回值表示是否是GM命令
	bool	Check_Exec_GMCmd(IActor * pActor, const char * pszContent);

	//收到全服公告的消息
	void	CalWorldNotice(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看物品超链接
	void	ViewGoodsSuperLink(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	void	HandleUseControlCmd(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//GM命令ID是否在权限中
	bool	bInPermission(UINT8 GMCmdID, char * pszPermission);

	//是否被禁言
	bool	IsDontTalk(IActor * pActor);

	//移除聊天框的系统消息
	void	RemoveTalkSysMsg(UINT32	TalkSysMsgID);

	//开始聊天框的系统消息
	void	StartSysMsg(UINT32	 TalkSysMsgID);

	//显示帮派数据
	void	ViewSynInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	TCDTimerID			m_WorldTalkCDTimerID;		 //世界聊天的冷却ID
	TCDTimerID			m_SynTalkCDTimerID;			 //帮派聊天的冷却ID
	TCDTimerID			m_WorldViewGoodsCDTimerID;	 //世界展示物品的冷却ID

	std::map<UID, UseControlCmd>	 m_mapUserControlCmd; //使用的GM命令

	std::map<TUserID, SDB_DontTalk>	 m_mapDoutTalk;		  //禁言

	typedef std::map<UINT32/*SysMsgID*/,STalkSysMsg>	MAP_TALKSYSMSG;

	MAP_TALKSYSMSG					 m_mapTalkSysMap;	//聊天框的系统公告
};

#endif
