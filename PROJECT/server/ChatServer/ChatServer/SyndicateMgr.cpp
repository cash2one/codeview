#include "SyndicateMgr.h"
#include "IBasicService.h"


//加入到帮派成员
void	SyndicateMgr::AddToSynMember(TSynID SynID, Actor * pActor)
{
	std::hash_map<TSynID,MAP_SYNMEMBER>::iterator iter = m_mapSyn.find(SynID);

	if( iter == m_mapSyn.end()){
		MAP_SYNMEMBER  mapMember;
		mapMember[pActor->GetUID()] = pActor;
		m_mapSyn[SynID] = mapMember;
		return;
	}

	MAP_SYNMEMBER & mapMember = iter->second;

	mapMember[pActor->GetUID()] = pActor;
}

//发送帮派聊天
void	SyndicateMgr::SendSynTalk(TSynID SynID)
{
	std::hash_map<TSynID,MAP_SYNMEMBER>::iterator iter = m_mapSyn.find(SynID);

	if( iter == m_mapSyn.end()){
		TRACE("<error> %s : %d 行 不存在的帮派ID！！帮派ID = %d", __FUNCTION__, __LINE__, SynID);
		return;
	}

	MAP_SYNMEMBER & mapMember = iter->second;

	MAP_SYNMEMBER::iterator it = mapMember.begin();

	for( ; it != mapMember.end(); ++it)
	{
		Actor * pActor = it->second;

		//发送聊天信息
	}
}

//发送帮派系统消息
void	SyndicateMgr::SendSynSysMsg(TSynID SynID)
{
	std::hash_map<TSynID,MAP_SYNMEMBER>::iterator iter = m_mapSyn.find(SynID);

	if( iter == m_mapSyn.end()){
		TRACE("<error> %s : %d 行 不存在的帮派ID！！帮派ID = %d", __FUNCTION__, __LINE__, SynID);
		return;
	}

	MAP_SYNMEMBER & mapMember = iter->second;
	MAP_SYNMEMBER::iterator it = mapMember.begin();

	for( ; it != mapMember.end(); ++it)
	{
		Actor * pActor = it->second;

		//发送系统消息
	}	
}

//从在线帮派成员中移除
void	SyndicateMgr::RemoveSynMember(Actor * pActor)
{
	std::hash_map<TSynID,MAP_SYNMEMBER>::iterator iter = m_mapSyn.find(pActor->GetSynID());

	if( iter == m_mapSyn.end()){
		TRACE("<error> %s : %d 行 不存在的帮派ID！！帮派ID = %d", __FUNCTION__, __LINE__, pActor->GetSynID());
		return;
	}

	MAP_SYNMEMBER & mapMember = iter->second;

	MAP_SYNMEMBER::iterator it = mapMember.find(pActor->GetUID());
	if( it == mapMember.end()){
		return;
	}

	mapMember.erase(it);
}
