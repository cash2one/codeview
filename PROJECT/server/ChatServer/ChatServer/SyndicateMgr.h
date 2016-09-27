#ifndef __CHATSERVER_SYNDICATEMGR_H__
#define __CHATSERVER_SYNDICATEMGR_H__

#include "DSystem.h"
#include "Actor.h"
#include <vector>
#include <map>


class SyndicateMgr
{
public:
	//加入到在线帮派成员
	void	AddToSynMember(TSynID SynID, Actor * pActor);

	//发送帮派聊天
	void	SendSynTalk(TSynID SynID);

	//发送帮派系统消息
	void	SendSynSysMsg(TSynID SynID);

	//从在线帮派成员中移除
	void	RemoveSynMember(Actor * pActor);

private:
	typedef std::hash_map<UID, Actor *,std::hash<UID>, std::equal_to<UID>>	MAP_SYNMEMBER;

	MAP_SYNMEMBER							m_mapMember;	//在线帮派成员

	std::hash_map<TSynID,MAP_SYNMEMBER>	m_mapSyn;	
};


#endif
