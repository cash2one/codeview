#ifndef __THINGSERVER_TALISMANWORLDFUBEN_H__
#define __THINGSERVER_TALISMANWORLDFUBEN_H__

#include "IActor.h"
#include <vector>
#include "DSystem.h"
//#include "TalismanWorldFuBen.h"
#include "ICombatPart.h"
#include <map>
#include "IGameWorld.h"
#include <list>
#include "IEventServer.h"
#include "IDBProxyClient.h"

//被挑战生物信息
struct ChallengedInfo
{
	ChallengedInfo()
		: m_uidUser(UID()), m_uidMonster(UID()), m_Level(0), m_Floor(0)
	{
	}
	UID		m_uidUser;		//占领者UID
	UID		m_uidMonster;	//怪物UID
	UINT8	m_Level;		//法宝世界级别
	UINT8	m_Floor;		//法宝世界层数
};

//法宝世界生物信息
struct TWCreatureInfo
{
	TWCreatureInfo()
		:m_uidCreature(UID()),m_Level(0),m_Floor(0)
		,m_bInCombat(false),m_uidOccupater(UID()),m_bCanReplace(false),m_Index(-1)
	{
	}

	UID		m_uidCreature;	//UID
	UINT8	m_Level;		//法宝世界级别
	UINT8	m_Floor;		//法宝世界层数
	bool	m_bInCombat;	//是否在战斗中
	UID		m_uidOccupater;	//占领者UID
	bool	m_bCanReplace;	//是否可替换
	INT8	m_Index;		//地图中的索引
};

//等待战斗的玩家
struct TWWaitUser
{
	UID		m_uidUser;
	UINT8	m_Level;
	UINT8	m_Floor;
};

class TalismanWorldFuBen : public ICombatObserver,public IEventListener,public IVoteListener, public IDBProxyClientSink
{
	enum enTalismanWorldTimer
	{
		enTalismanWorldTimer_Save = 0,
	};
public:
	TalismanWorldFuBen();

	bool	Create(IGameWorld * pGameWorld);

	bool	OnSetDBData();

	//进入法宝世界
	void	EnterTalismanWorld(IActor * pActor, UINT8 floor);

	//战斗
	void	Combat(IActor * pActor, UID uidEnemy, UINT8 level, UINT8 floor);

	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

	//占领
	virtual bool Occupation(IActor * pActor, UID uidMonster, UINT8	Level, UINT8 floor);

	//结束占领
	virtual bool OverOccupation(IActor * pActor, UID uidMonster, UINT8 Level, UINT8 Floor);

	//同步整张场景的替换信息给指定玩家
	void	SynReplaceInfoMap(IActor * pActor, UINT8 Level, UINT8 Floor);

	//同步单个替换信息给场景内所有玩家
	void	SynReplaceInfoSingle(const ReplaceInfo & Info, UINT8 Level, UINT8 Floor);

	//通知场景内所有玩家取消替换图片
	void	CancelReplace(UID uidMonster,UINT8 Level, UINT8 Floor);

	//取消等待进攻
	void	Cancel_WaitCombat(UID uidUser);

	virtual void OnEvent(XEventData & EventData);

	virtual bool OnVote(XEventData & EventData);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//关服后，玩家上线获取玩家的占领信息
	virtual void GetUserOccupation(UID uidUser, UINT8 Level, UINT8 Floor, UID & uidMonster, UINT32 & OccupatTime);

	//查看回放
	void	ViewRec(IActor * pActor,UINT32 m_Rank);

	//得到上次占领被打败时的信息
	void GetPreOccupationInfo(IActor * pActor);


	
	
private:
	//开始战斗
	bool			BeginCombat(IActor * pActor, IThing * pEnemy, UINT8 level, UINT8 floor);

	//挑选一个和玩家打,首选玩家点击的那只，若在战斗则选别的
	TWCreatureInfo * SelectCombatEnemy(UID uidEnemy, UINT8 Level, UINT8 Floor);

	//加入等待进攻队列
	void			Push_WaitList(IActor * pActor,UINT8 Level, UINT8 Floor);

	//取出等待进攻队列的队首玩家
	UID				Pop_WaitList(UINT8 Level, UINT8 Floor);

	//得到被玩家占领的怪物UID
	UID				GetMonsterOccupated(IActor * pActor);

	//进行下一场战斗，从进攻队列中选择进攻者
	void			NextCombat(UID uidMonster, UINT8 level, UINT8 floor);

	//得到一个怪物是否是可替换的
	bool			IsCanReplace(UID uidMonster, UINT8 Level);

	//得到法宝世界数据库信息
	void			HandleGetTalismanWorldInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到法宝世界占领时被击败的战斗回放
	void			HandleGetTalismanWorldRecordBuf(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	
	//得到法宝世界占领时被击败的信息
	void			HandleGetOccupationInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);



private:
	typedef std::vector<TSceneID>					VECT_SCENEID;

	//每个级别对应的场景集合
	std::vector<VECT_SCENEID>						m_vecScene;

	//怪物对应的替换信息
	typedef std::map<UID/*Monster*/, ReplaceInfo>	MAP_REPLACEINFO;

	//每层对应的替换信息
	typedef std::vector<MAP_REPLACEINFO>			VECT_REPLACEFLOOR;

	//每个级别对应的替换信息
	std::vector<VECT_REPLACEFLOOR>					m_vecReplaceInfo;

	//战斗中
	std::map<UID/*挑战者*/, ChallengedInfo/*被挑战者信息*/>		m_InCombat;

	//等待进攻队列
	//std::list<UID>									m_listAttacker;
	std::list<TWWaitUser>							m_listAttacker;

	typedef std::map<UID, TWCreatureInfo>			MAP_TWCREATUREINFO;

	//每个级别对应的可替换生物信息
	std::vector<MAP_TWCREATUREINFO>					m_TWCreatureInfo;


};

#endif
