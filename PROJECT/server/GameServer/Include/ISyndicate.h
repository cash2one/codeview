#ifndef __RELATIONSERVER_ISYNDICATE_H__
#define __RELATIONSERVER_ISYNDICATE_H__

#include "UniqueIDGenerator.h"
#include "DSystem.h"
#include "ISyndicateMember.h"
#include "IVisitSynMember.h"



struct SyndicateInfo
{
	SyndicateInfo()
	{
		MEM_ZERO(this);
	}
	TSynID		m_SynID;							//帮派ID
	char		m_szSynName[THING_NAME_LEN];		//帮派名称
	UINT64		m_uidLeader;						//帮主UID值
	char		m_szLeaderName[THING_NAME_LEN];		//帮主名字
	UINT8		m_Level;							//帮派等级
	UINT32		m_Exp;								//帮派经验
	UINT16		m_MemberNum;						//帮派人数
	UINT32		m_NeedExp;							//帮派升级需要经验
	UINT16		m_MaxMemberNum;						//帮派最多可容纳人数
	UINT32		m_SynWarScore;						//帮战积分
	INT32		m_SynWarAbility;					//帮战实力
	char		m_szSynMsg[DESCRIPT_LEN_300];		//帮派公告
	char		m_PreEnemySynName[THING_NAME_LEN];	//上一场帮战敌对帮派名
	enumSynWarbWin		m_bWin;						//上一场帮战是否胜利
	UINT32		m_SynWarTotalScore;					//帮战总积分
};

//帮派申请者的信息
struct SyndicateApply
{
	TSynID		m_SynID;							//申请加入的帮派ID
	UID			m_uidApplyUser;						//申请者的UID值
	char		m_szApplyUserName[THING_NAME_LEN];	//申请者的名字				
	UINT8		m_ApplyUserLevel;					//申请者的等级
	UINT8		m_ApplyUserLayer;					//申请者的境界
	UINT32		m_LastOnlineTime;					//最后在线时间
};

//帮派福利
enum enWelfare VC_PACKED_ONE 
{
	enWelfare_ReduceTrainingHallMoney = 1,		//减少练功堂修炼费用
	enWelfare_MaxFilchCollectionOther,		//代收及窃取灵石的上限增加
	enWelfare_MaxHonorOneDay,				//玩家每天获得荣誉上限增加
	enWelfare_StrongRandom,					//强化几率提高
	enWelfare_MaxSellGoodsNum,				//出售物品挂单上限增加
	enWelfare_OnHookTime,					//伏魔洞挂机时间变为多少小时
	enWelfare_XiuLianTime,					//修炼时间增加48小时时间段
	enWelfare_TalismanStrongRandom,			//法宝强化几率提高
	enWelfare_OnHookNumOneDay,				//每日练功堂挂机次数增加1次
	enWelfare_ReduceXiuLianMoney,			//玩家进行独自修炼、双人修炼费用减少
	enWelfare_AloneXiuLianGodSwordNimbus,	//玩家独自修炼时，每分钟产生仙剑灵气增加
	enWelfare_AloneXiuLianAddXiuLianActor,	//独自修炼增加一名修炼角色
	enWelfare_EnterFuBenNum,				//每日进入副本次数增加1次

	enWelfare_Max,
}PACKED_ONE;


struct ISyndicate
{
	//得到玩家获得的福利值,百分比的数值直接返回数值，比如%8返回8
	virtual UINT8			GetWelfareValue(enWelfare Welfare)		= 0;

	//得到帮派的信息
	virtual const SyndicateInfo & GetSyndicateInfo()				= 0;

	//得到帮派ID
	virtual TSynID			GetSynID() const						= 0;

	//得到帮主UID
	virtual const UID		GetLeaderUID() const					= 0;

	//设置帮主UID
	virtual void			SetLeaderUID(UINT64 LeaderUID, bool bUpdate = true) = 0;

	//等到帮主名字
	virtual const char *	GetLeaderName() const					= 0;

	//设置帮主名字
	virtual void			SetLeaderName(const char * szLeaderName, bool bUpdate = true) = 0;

	//昨到帮派名字
	virtual const char *	GetSynName() const						= 0;

	//得到帮派等级
	virtual UINT8			GetSynLevel() const						= 0;

	//设置帮派等级
	virtual bool			SetSynLevel(UINT8 Level, bool bUpdate = true)				= 0;
	
	//得到帮派经验
	virtual INT32			GetSynExp() const						= 0;

	//增加帮派经验值
	virtual void			AddSynExp(INT32 Exp, bool bUpdate = true)					= 0;

	//得到帮派人数
	virtual UINT16			GetSynMemberNum() const					= 0;

	//增加或减少帮派从数
	virtual void			AddSynMemberNum(INT16 AddMemberNum, bool bUpdate = true)		= 0;

	//得到帮派升级需要经验
	virtual UINT32			GetUpLevelNeedExp() const				= 0;

	//设置帮派升级需要经验
	virtual void			SetUpLevelNeedExp(UINT32 Exp)			= 0;

	//得到帮派最多可容纳人数
	virtual UINT16			GetMaxMemberNum() const					= 0;

	//设置帮派最多可容纳人数
	virtual void			SetMaxMemberNum(UINT16 MaxMemberNum)	= 0;

	//保存帮派的信息
	virtual void			UpdateSyndicate()						= 0;

	//加入帮派
	virtual bool			JoinSyndicate(const SyndicateMemberInfo & SynMemberInfo) = 0;

	//获得帮派成员
	virtual ISyndicateMember * GetSynMember(UID uidUser)			= 0;

	//解散帮派
	virtual bool			JieSanSyndicate()						= 0;

	//离开帮派
	virtual bool			RemoveSyndicate(UID uidUser)			= 0;

	//遍历帮派成员
	virtual void			VisitAllSynMember(IVisitSynMember & visit) = 0;

	//查看帮派成员列表
	virtual void			ViewSynMemberList(IActor * pActor)		= 0;

	//加入到帮派成员集合中
	virtual void			AddToMemberList(ISyndicateMember * pSynMember) = 0;

	//是否能参加帮战
	virtual UINT8			Check_JoinSynCombat() = 0;

	//得到帮派成员的随机分布位置
	virtual void			RandomSynMemberPos(std::vector<UID> & vecMember) = 0;

	//得到帮派积分
	virtual INT32			GetSynWarScore() const = 0 ;

	//设置帮派积分
	virtual void			SetSynWarScore(INT32 SynWarScore) = 0;

	//得到帮战总积分
	virtual INT32			GetSynWarTotalScore() const = 0;

	//增加帮战积分
	virtual void			AddSynWarScore(INT32 SynWarScore) = 0;

	//得到帮战实力
	virtual INT32			GetSynWarAbility() const = 0;

	//设置帮战实力
	virtual void			SetSynWarAbility(INT32 SynWarAbility) = 0;

	//得到帮派公告
	virtual const char *	GetSynMsg() = 0;

	//设置帮派公告
	virtual void			SetSynMsg(const char * szSynMsg) = 0;

	//得到上一场帮战敌对帮派名字
	virtual const char * GetPreEnemySynName() const = 0;

	//得到上一场帮战是否胜利
	virtual enumSynWarbWin GetWinFail() const = 0;

	//设置上一场帮战数据
	virtual void SetPreSynWarData(const char * szSynName,enumSynWarbWin bWin) = 0;

	//更新帮派战力
	virtual void		UpdateSynCombatAbility() = 0;

};

#endif
