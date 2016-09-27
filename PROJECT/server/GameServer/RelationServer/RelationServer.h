
#ifndef __RELATIONSERVER_RELATIONSERVER_H__
#define __RELATIONSERVER_RELATIONSERVER_H__

#include "IRelationServer.h"
#include "Talk.h"
#include "Trade.h"
#include "SynMagic.h"
#include "SyndicateShop.h"
#include "ITalk.h"
#include "Rank.h"
#include "SystemMsg.h"

extern IGameServer * g_pGameServer;

struct ISyndicateMgr;
struct IActor;

class RelationServer : public IRelationServer
{
public:
	RelationServer();
	virtual ~RelationServer();

	virtual bool Create();

	virtual ISyndicateMgr * GetSyndicateMgr();

	//帮派招募成员
	virtual void	ZhaoMuSynMember(IActor * pActor);


	//注册GM命令处理者
	virtual bool RegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler);

	//取消注册GM命令处理者
	virtual bool UnRegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler);

	//分发GM命令
	virtual void DispatchGMMessage(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	virtual ITalk * GetTalk();

	virtual ISystemMsg * GetSystemMsg();

	//右上角的提示悬浮框
	virtual void  SendTipBox(IActor * pActor, const char * pszTip);

	//刷新帮战排行
	virtual void WarEndFlushRank();

public:
   //释放
	virtual void Release(void) ;

	virtual void Close();

private:
	ISyndicateMgr *  m_pSyndicateMgr;
	Talk			 m_Talk;
	Trade			 m_Trade;
	Rank			 m_Rank;
	SystemMsg		 m_SystemMsg;

	typedef std::hash_map<UINT8/*enGMCmd*/, IGMCmdHandler *> MAP_GMCMDHANDLE;

	MAP_GMCMDHANDLE  m_mapGMCmdHandle;	//接收GM命令
};






#endif
