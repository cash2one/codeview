#ifndef __RELATIONSERVER_SYNDICATE_H__
#define __RELATIONSERVER_SYNDICATE_H__

#include "ISyndicate.h"
#include "IGameWorld.h"

class Syndicate : public ISyndicate
{
public:
	Syndicate();
	Syndicate(const SyndicateInfo & SynInfo);
	~Syndicate();

public:
	//得到玩家获得的福利值,百分比的数值直接返回数值，比如%8返回8
	virtual UINT8			GetWelfareValue(enWelfare Welfare);

	//得到帮派的信息
	virtual const SyndicateInfo & GetSyndicateInfo();

	//得到帮派ID
	virtual TSynID			GetSynID() const;

	//得到帮主UID
	virtual const UID		GetLeaderUID() const;

	//设置帮主UID
	virtual void			SetLeaderUID(UINT64 LeaderUID, bool bUpdate = true);

	//得到帮主名字
	virtual const char *	GetLeaderName() const;

	//设置帮主名字
	virtual void			SetLeaderName(const char * szLeaderName, bool bUpdate = true);

	//得到帮派名字
	virtual const char *	GetSynName() const;

	//得到帮派等级
	virtual UINT8			GetSynLevel() const;

	//设置帮派等级
	virtual bool			SetSynLevel(UINT8 Level, bool bUpdate = true);
	
	//得到帮派经验
	virtual INT32			GetSynExp() const;

	//增加帮派经验值
	virtual void			AddSynExp(INT32 Exp, bool bUpdate = true);

	//得到帮派人数
	virtual UINT16			GetSynMemberNum() const;

	//增加或减少帮派从数
	virtual void			AddSynMemberNum(INT16	AddMemberNum = 1, bool bUpdate = true);

	//得到帮派升级需要经验
	virtual UINT32			GetUpLevelNeedExp() const;

	//设置帮派升级需要经验
	virtual void			SetUpLevelNeedExp(UINT32 Exp);

	//得到帮派最多可容纳人数
	virtual UINT16			GetMaxMemberNum() const;

	//设置帮派最多可容纳人数
	virtual void			SetMaxMemberNum(UINT16 MaxMemberNum);

	//保存帮派的信息
	virtual void			UpdateSyndicate();

	//加入帮派
	virtual bool			JoinSyndicate(const SyndicateMemberInfo & SynMemberInfo);

	//获得帮派成员
	virtual ISyndicateMember * GetSynMember(UID uidUser);

	//解散帮派
	virtual bool			JieSanSyndicate();

	//离开帮派
	virtual bool			RemoveSyndicate(UID uidUser);

	//遍历帮派成员
	virtual void			VisitAllSynMember(IVisitSynMember & visit);

	//查看帮派成员列表
	virtual void			ViewSynMemberList(IActor * pActor);

	//加入到帮派成员集合中
	virtual void			AddToMemberList(ISyndicateMember * pSynMember);

	//是否能参加帮战
	virtual UINT8			Check_JoinSynCombat();

	//得到帮派成员的随机分布位置
	virtual void			RandomSynMemberPos(std::vector<UID> & vecMember);

	//得到帮派积分
	virtual INT32			GetSynWarScore() const;

	//设置帮派积分
	virtual void			SetSynWarScore(INT32 SynWarScore);

	//得到帮战总积分
	virtual INT32			GetSynWarTotalScore() const;

	//增加帮战积分
	virtual void			AddSynWarScore(INT32 SynWarScore);

	//得到帮战实力
	virtual INT32			GetSynWarAbility() const;

	//设置帮战实力
	virtual void			SetSynWarAbility(INT32 SynWarAbility);

	//得到帮派公告
	virtual const char *	GetSynMsg();

	//设置帮派公告
	virtual void			SetSynMsg(const char * szSynMsg);

	//得到上一场帮战敌对帮派名字
	virtual const char * GetPreEnemySynName() const;

	//得到上一场帮战是否胜利
	virtual enumSynWarbWin GetWinFail() const;

	//设置上一场帮战数据
	virtual void SetPreSynWarData(const char * szSynName,enumSynWarbWin bWin);

	//更新帮派战力
	virtual void		UpdateSynCombatAbility();

private:
	SyndicateInfo		m_SyndicateInfo;			//帮派信息结构

	typedef std::hash_map<UID, ISyndicateMember *>		MAPSYNMEMBER;

	MAPSYNMEMBER		m_SynMember;				//帮派成员的集合	


};

#endif
