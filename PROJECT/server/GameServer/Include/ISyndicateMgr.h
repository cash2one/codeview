#ifndef __RELATIONSERVER_ISYNDICATEMGR_H__
#define __RELATIONSERVER_ISYNDICATEMGR_H__

#include "DSystem.h"
#include "UniqueIDGenerator.h"
#include "IVisitSynMember.h"

struct ISyndicate;
struct ISyndicateMember;

struct ISyndicateMgr
{
	virtual ~ISyndicateMgr(){}

	virtual bool				 Create() = 0;
	virtual void				 Close()  = 0;
	
	//得到玩家的帮派ID
	virtual TSynID				 GetUserSynID(const UID & uid_User) = 0;

	//获得玩家的帮派
	virtual ISyndicate *		 GetSyndicate(const UID & uid_User) = 0;

	//获得玩家的帮派
	virtual ISyndicate *		 GetSyndicate(TSynID SynID) = 0;

	//获得玩家个人的帮派成员
	virtual ISyndicateMember *	 GetSyndicateMember(const UID & uid_User) = 0;

	//遍历帮派成员
	virtual void				 VisitAllSynMember(TSynID SynID, IVisitSynMember & visit) = 0;

	//通过邮件加入帮派
	virtual bool				 MailJoinSyndicate(IActor * pActor, TSynID SynID, enMailRetCode & MailRetCode) = 0;

	//得到帮派排名榜
	virtual void				 GetSynRank(std::vector<ISyndicate *> & vectSynRank) = 0;

	//得到帮战开始剩余时间
	virtual UINT32	GetRemainStartSynCombat() = 0;

	//得到敌对帮派ID
	virtual TSynID GetVsSynID(IActor * pActor) = 0;

	//得到帮战排名榜
	virtual void GetSynWarRank(std::vector<ISyndicate *> & vectSynWarRank) = 0;

	//更新帮派战力
	virtual void UpdateSynCombatAbility() = 0;

	//得到敌对帮派名
	virtual std::string GetVsSynName(IActor * pActor)   = 0;

};

#endif
