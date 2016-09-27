#ifndef __RELATIONSERVER_ITALK_H__
#define __RELATIONSERVER_ITALK_H__

struct IActor;

struct ITalk
{
	//全服公告
	virtual void  WorldNotice(IActor * pActor, const char * pszNoticeContext) = 0;

	//世界频道的系统消息 (pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
	virtual void  WorldSystemMsg(const char * pszMsgContext, IActor * pActor = 0, enTalkMsgType enMsgType = enTalkMsgType_System) = 0;

	//加入到禁言中
	virtual void DontTalk_Push(SDB_DontTalk & DBDontTalk) = 0;

	//加入聊天框的系统公告
	virtual void TalkSysMsg_Push(SDB_SysMsgInfo & DBSysMsg) = 0;
};

#endif
