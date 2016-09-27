#ifndef __THINGSERVER_XUANTIANFUBEN_H__
#define __THINGSERVER_XUANTIANFUBEN_H__

#include "IActor.h"
#include "DSystem.h"
#include "ITimeAxis.h"
#include <vector>
#include "IGoods.h"
#include "IConfigServer.h"
#include "ICombatPart.h"
#include <map>
#include "IDBProxyClient.h"

struct IMonster;

//玄天BOSS信息
struct XTBossInfo
{
	XTBossInfo()
	{
		m_MonsterID = INVALID_MONSTER_ID;
		m_CombatMap = INVALID_MAP_ID;
		m_CombatIndex = 0;
	}
	TMonsterID	m_MonsterID;	//怪物ID
	TMapID		m_CombatMap;	//战斗地图
	UINT32		m_CombatIndex;	//战斗索引
};

//参加玄天的玩家信息
struct XTActorInfo
{
	XTActorInfo()
	{
		m_Damage = 0;
		m_AttackNum = 0;
		m_Level  = 0;
	}
	UID		m_uidUser;
	UINT8	m_Level;					//玩家等级
	char	m_Name[THING_NAME_LEN];		//名字
	INT32	m_Damage;					//伤害
	INT32	m_AttackNum;				//攻击次数
};

//最后一击信息
struct BossLastKill
{
	BossLastKill()
	{
		m_TotalDamage = 0;
		m_CurDamage = 0;
		m_AttackNum = 0;
		m_UserLevel = 0;
	}

	UID		m_uidUser;
	char	m_szUserName[THING_NAME_LEN];	//玩家名字
	char	m_szBossName[THING_NAME_LEN];	//BOSS名字
	INT32	m_TotalDamage;					//玩家对BOSS的总伤害
	INT32	m_CurDamage;					//玩家对BOSS的当次伤害
	INT32	m_AttackNum;					//攻击次数
	UINT8	m_UserLevel;					//玩家等级
};

//数据库创建玄天BOSS
struct DBCreateXTBoss
{
	DBCreateXTBoss()
	{
		m_uidBoss = UID();
		m_MonsterID = INVALID_MONSTER_ID;
		m_CurBlood = 0;
	}
	UID			m_uidBoss;
	TMonsterID	m_MonsterID;
	INT32		m_CurBlood;
};

class XuanTianFuBen : public ITimerSink, public ICombatObserver, public IDBProxyClientSink
{
	enum enXuanTianTimerID
	{
		enXuanTianTimerID_Save = 0,		//保存玄天数据

		enXuanTianTimerID_Over,			//结束定时器

		enXuanTianTimerID_Start,		//开始的定时器
	};
public:
	XuanTianFuBen();

	bool	Create();

	//玄天副本是否开启
	bool	IsOpen() const;

	virtual void OnTimer(UINT32 timerID);

	//攻击BOSS
	void	AttackBoss(IActor * pActor, UID uidBoss);

	//打开玄天页面
	void	OpenXuanTiam(IActor * pActor);

	//显示玄天奖励
	void	ShowXTForward(IActor * pActor, UINT32 Rank);

	//使用仙石缩短玄天再次攻击时间
	void	MoneyLessAttackTime(IActor * pActor);

	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

	//得到玄天开放剩余时间
	UINT32		GetRemainOpenXT();

	//显示最后一击
	void	ShowLastKill(IActor * pActor);

	//显示伤害排行
	void	ShowDamageRank(IActor * pActor, int index);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//保存玩家的伤害数据
	void	SaveUserDamage(IActor * pActor);

	//重新加载玄天怪物配置信息
	void	ReloadXTMonsterCnfg();

private:
	//开启玄天
	void		StartXuanTian(std::vector<DBCreateXTBoss> * pCreateBoss = 0);

	//玄天结束(三个BOSS都死时，提前结束)
	void		OverXuanTian();

	//玄天时间结束
	void		Over_TimeXT();

	//检测玄天是否结束
	bool		Check_OverXT();

	//随机产生一个玄天BOSS信息
	XTBossInfo	RandomGetXTBossID();

	//是否能攻击玄天BOSS
	UINT8		Check_CanAttackBoss(IActor * pActor, UID uidBoss);

	//玄天奖励
	void		XTForward();

	//按等级给奖励
	void		GetRankForward(UID uidUser, INT32 Rank, INT32 AttackNum, INT32 Damage, const char * pBossName);

	//随机获得物品奖励
	std::vector<IGoods *> RandomGetGoodsForward(const SXuanTianForward * pXTForward);

	//得到玄天结束剩余时间
	UINT32		GetRemainOverXT();

	//创建玄天BOSS
	bool		CreateXTBoss(IGameScene * pGameScene, std::vector<DBCreateXTBoss> * pCreateBoss = 0);

	//得到玩家的级别组下标
	int			GetLevelGroupIndex(UINT8 Level);

	//在玄天开放的第几个时期，0表示还没开放并且今天也没开放过
	UINT8		InOpenTimeSpaceIndex(UINT32 time);

	//得到玄天BOSS信息
	void		HandleGetXTBossInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到玄天伤害数据
	void		HandleGetXTDamage(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到玄天BOSS杀死数据
	void		HandleGetXTBossDie(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//清除数据库的BOSS信息
	void		Clear_BossInfo();

	//更改数据库上一次玄天信息
	void		ChangeDB_LastInfo();

	//保存BOSS数据
	void		SaveBossInfo(IMonster * pBoss);

	//距离下次开启玄天的秒数
	UINT32	GetNextOpenTimeNum();

	//距离玄天结束的秒数
	UINT32	GetOverTimeNum();

	//设置开启定时器
	void	SetStartTimer();

	//得到玄天BOSS配置信息
	void		HandleGetXTBossCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
private:
	bool		m_bOpen;									//玄天是否开启

	TSceneID	m_BossCombatSceneID;						//玄天BOSS所在的战斗场景

	std::map<UID, TMonsterID>	m_mapBoss;					//玄天BOSS的UID

	std::map<UID, XTActorInfo>	m_mapDamage;				//伤害信息

	typedef std::vector<XTActorInfo> VEC_XTACTORINFO;

	//区分玩家等级级组的
	std::vector<VEC_XTACTORINFO>	m_vectLastDamageRank;	//上一次伤害排行

	std::map<UID, BossLastKill>	m_mapKillUser;				//最后一击的玩家

	std::vector<BossLastKill>	m_vecLastKillUser;			//上一次最后一击的玩家

	bool						m_bOver;					//是否结束
};

#endif
