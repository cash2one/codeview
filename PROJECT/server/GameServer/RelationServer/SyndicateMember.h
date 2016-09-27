#ifndef __RELATIONSERVER_SYNDICATEMEMBER_H__
#define __RELATIONSERVER_SYNDICATEMEMBER_H__

#include "ISyndicateMember.h"

struct IActor;

class SyndicateMember : public ISyndicateMember
{
public:
	SyndicateMember();
	SyndicateMember(const SyndicateMemberInfo & SynMemberInfo);
	~SyndicateMember();

public:
	//获取帮派成员信息
	virtual	const SyndicateMemberInfo & GetSynMemberInfo(); 

	//获取所在帮派的ID
	virtual TSynID			GetSynID() const;

	//获取玩家的UID
	virtual UID				GetSynMemberUID() const;

	//获取玩家的名字
	virtual const char *	GetSynMemberName() const;

	//获取玩家的职位
	virtual enumSynPosition GetPosition() const;

	//设置玩家的职位
	virtual void			SetPosition(enumSynPosition Position, bool bUpdate = true);

	//获取玩家的帮派贡献
	virtual INT32			GetContribution() const;

	//增加或减少玩家的帮派贡献
	virtual void			AddContribution(INT32 Contribution);

	//得到玩家的等级
	virtual UINT8			GetLevel();

	//保存玩家帮派成员数据，在玩家退出游戏时调用
	virtual void			UpdateSynMemberData();

	//得到vip等级
	virtual UINT8			GetVipLevel();

	//得到战斗力
	virtual INT32			GetCombatAbility();

	//和玩家同步下数据
	virtual void			SycUserData(INT32 CombatAbility, UINT8 Level, UINT8 VipLevel, INT32 SynWarLv,INT32 ActorFacade);

	//获得外观
	virtual UINT16			GetFacade();

	//获得帮派功勋
	virtual INT32			GetSynMemberScore();

	//获得帮战等级
	virtual INT32			GetSynWarLv();

	//增加帮派功勋
	virtual void			AddSynScore(INT32 ScoreNum);

	//得到最后在线时间
	virtual UINT32			GetLastOnlineTime();


private:
	IActor * GetActor();
	

private:
	SyndicateMemberInfo		m_SyndicateMemberInfo;

	TSceneID                m_SceneID;  //当前所进入的帮派保卫战场景
	TSceneID                m_CombatSceneID;  //当前所进入的帮派保卫战战斗场景
	UINT8                   m_nCurMode;   //玩家当前进入的模式。(需要记录该值是因为，玩家在杀怪过程中可能升级，不能正确计算玩家所进的模式是那个)
};


#endif
