#ifndef __RELATIONSERVER_ISYNDICATEMEMBER_H__
#define __RELATIONSERVER_ISYNDICATEMEMBER_H__

#include "DSystem.h"
#include "BclHeader.h"
#include "UniqueIDGenerator.h"
#include "SyndicateCmd.h"


//帮派成员的信息
struct SyndicateMemberInfo
{
	SyndicateMemberInfo()
		: m_uidUser(UID()), m_SynID(0), m_Userlevel(0)
		, m_Position(enumSynPosition_General), m_Contribution(0)
		,m_VipLevel(0),m_CombatAbility(0),m_ActorFacade(700),m_Score(0),m_SynWarLv(0),m_LastOnlineTime(0)
	{
	}


	UID					m_uidUser;						//玩家的UID值
	TSynID				m_SynID;						//玩家所在的帮派ID
	char				m_szUserName[THING_NAME_LEN];	//玩家的名字
	UINT8				m_Userlevel;					//玩家等级
	enumSynPosition		m_Position;						//玩家在帮派的职位
	UINT32				m_Contribution;					//玩家的帮派贡献
	UINT8				m_VipLevel;						//vip等级
	INT32				m_CombatAbility;				//战斗力
	UINT16				m_ActorFacade;					//外观
	INT32				m_Score;						//帮派功勋	
	INT32				m_SynWarLv;						//帮战等级
	UINT32				m_LastOnlineTime;				//最后在线时间
};

struct ISyndicateMember
{
	//获取帮派成员信息
	virtual	const SyndicateMemberInfo & GetSynMemberInfo()			= 0; 

	//获取所在帮派的ID
	virtual TSynID			GetSynID() const						= 0;

	//获取玩家的UID
	virtual UID				GetSynMemberUID() const					= 0;

	//获取玩家的名字
	virtual const char *	GetSynMemberName() const				= 0;

	//获取玩家的职位
	virtual enumSynPosition GetPosition() const						= 0;

	//设置玩家的职位
	virtual void			SetPosition(enumSynPosition Position, bool bUpdate = true)	= 0;

	//获取玩家的帮派贡献
	virtual INT32			GetContribution() const					= 0;

	//增加或减少玩家的帮派贡献
	virtual void			AddContribution(INT32 Contribution)		= 0;

	//得到玩家的等级
	virtual UINT8			GetLevel()								= 0;

	//保存玩家帮派成员数据，在玩家退出游戏时调用
	virtual void			UpdateSynMemberData()					= 0;

	//得到vip等级
	virtual UINT8			GetVipLevel()							= 0;

	//得到战斗力
	virtual INT32			GetCombatAbility()						= 0;

	//和玩家同步下数据
	virtual void			SycUserData(INT32 CombatAbility, UINT8 Level, UINT8 VipLevel, INT32 SynWarLv,INT32 ActorFacade) = 0;

	//获得外观
	virtual UINT16			GetFacade()								= 0;

	//获得帮派功勋
	virtual INT32			GetSynMemberScore()						= 0;

	//获得帮战等级
	virtual INT32			GetSynWarLv()							= 0;

	//增加帮派功勋
	virtual void			AddSynScore(INT32 ScoreNum)				= 0;

	//得到最后在线时间
	virtual UINT32			GetLastOnlineTime()						= 0;
};

#endif
