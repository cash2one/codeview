#ifndef __RELATIONSERVER_SYNDICATECOMBAT_H__
#define __RELATIONSERVER_SYNDICATECOMBAT_H__


#include <vector>
#include "DSystem.h"
#include "ISyndicate.h"
#include <hash_map>
#include "ITimeAxis.h"
#include <map>
#include <set>
#include "IEventServer.h"
#include "ICombatPart.h"
#include "IDBProxyClient.h"
#include "ISyndicate.h"

struct UserSynWarInfo
{
	TBitArray<MAX_SYN_USERNUM>	m_SynWarHitroy;	//帮战杀敌记录
};

class SyndicateCombat : public ITimerSink, public IEventListener, public ICombatObserver, public IDBProxyClientSink,public IVoteListener
{
	enum enSynWarTimer
	{
		enSynWarTimer_Control		= 0,
		enSynWarTimer_StartSynWar,
		enSynWarTimer_OverSynWar,
		enSynWarTimer_StartDeclare,
		enSynWarTimer_OverDeclare,

	};
public:

	SyndicateCombat();

	bool Create();

	virtual void OnEvent(XEventData & EventData);

	virtual void OnTimer(UINT32 timerID);

	//打开帮战界面
	void OpenSynCombat(IActor * pActor);

	//打开宣战页面
	void OpenDeclareWar(IActor * pActor);
	
	//宣战
	void DeclareWar(IActor * pActor, TSynID SynID);

	//接受宣战
	void AcceptDeclareWar(IActor * pActor, TSynID SynID);

	//拒绝宣战
	void RefuseDeclareWar(IActor * pActor, TSynID SynID);

	//开始系统宣战
	void BeginSystemDeclareWar();

	//进入帮战场景
	void EnterSynCombatScene(IActor * pActor);

	//购买战斗次数
	void BuyCombatNum(IActor * pActor);

	//战斗
	void Combat(IActor * pActor, UID uidEnemy);

	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

	//得到帮战数据
	void GetSynWarData(IActor * pActor);

	//得到帮战开始剩余时间
	bool GetbOpenAttack();

	//得到敌对帮派
	TSynID SyndicateCombat::GetVsSynID(IActor * pActor);

	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual bool OnVote(XEventData & EventData);

	//得到敌对帮派名
	std::string GetVsSynName(IActor * pActor);

 	//多久开启下次帮战
	UINT32		GetNextOpenWarTimeNum();

private:
	//开始帮战
	void StartSynCombat();

	//结束帮战
	void OverSynCombat();

	//开始帮战宣战
	void StartSynWarDeclare();

	//结束帮战宣战
	void OverSynWarDeclare();

	//是否在宣战时间
	bool IsInDeclareWarTime();

	void OkDeclareWar(TSynID SynID, TSynID SynID2);

	//配对帮战
	void PairSynCombat(ISyndicate * pSyn1, ISyndicate * pSyn2);

	//检测两个帮派是否可以配对
	UINT8 CanPairJoin(ISyndicate * pSyn1, ISyndicate * pSyn2);

	//检测一个帮派是否可以参加帮战
	UINT8 CanJoinSynCombat(ISyndicate * pSyn);

	//随机分配各帮派成员在场景中的位置
	void  RandomSetSynMemberPos();

	//战斗胜利奖励
	void  CombatForward(IActor * pWiner, IActor * pFailer);
	void  CombatForward(IActor * pActor, IActor * pEnemy, bool bWin, CombatCombatOver * pCombatResult);

	//帮战奖励
	void  SynWarOverForward();

	//获得每日获得声望上限
	INT32 GetCreditUp(IActor * pActor);

	//开始战斗
	void  BeginCombat(IActor * pActor, IActor * pEnemy);

	//该时间是否开启帮战战斗
	bool  IsOpenCombatTime(UINT32 time);

	//该时间是否开启帮战宣战
	bool  IsOpenDeclareTime(UINT32 time);

	//得到帮战最后记录时间
	void		HandleGetSynWarLastTime(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到帮战数据
	void		HandleGetSynWarData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到帮战杀敌记录
	void		HandleGetSynWarKill(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到帮战宣战数据
	void		HandleGetSynWarDeclare(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);


	//保存帮派的帮战信息
	void		SaveSynSynWar(TSynID SynID, bool bWin);

	//帮派胜利的奖励
	void		SynWinForward(TSynID SynID);

	//更新上一场帮战数据
	void UpdatePreSynWarData(TSynID SynID,enumSynWarbWin bWin,const char * SynName);

	//弹出帮战胜利/失败弹出框
	void		SendSynResult(IActor * pActor, enumSynWarbWin enbWin);

	//给帮派成员发送帮战结果框
	void		SendSynAllResult(ISyndicate * pSyn, enumSynWarbWin enbWin);



	//多久结束帮战
	UINT32		GetOverWarTimeNum();

	//多久开启下次帮战宣战
	UINT32		GetNextOpenDeclareTimeNum();

	//多久结束帮战宣战
	UINT32		GetOverDeclareTime();
private:
	//帮战是否开启
	bool			m_bOpenAttack;

	//帮战是否开启宣战
	bool			m_bOpenDeclare;

	typedef std::hash_map<TSynID, TSynID>		MAP_SYN_PAIR;

	//帮派配对情况
	MAP_SYN_PAIR	m_SynPair;

	std::hash_map<TSynID, INT32>				m_SynGetScore;

	typedef std::vector<UID>					VECT_MEMBER;
	
	//各帮派成员在场景的随机分布位置
	std::hash_map<TSynID, VECT_MEMBER>			m_SynMemberPos;

	//帮战中玩家杀敌记录
	std::map<UID, UserSynWarInfo>				m_KillHistory;

	typedef std::vector<TSynID>					VECT_SYNID;

	typedef std::hash_map<TSynID, VECT_SYNID>	MAP_SYN_APPLY;

	//帮战宣战列表
	MAP_SYN_APPLY								m_SynApply;

	//时间没到就胜利的帮派
	std::set<TSynID>							m_FastWinSyn;

	//帮战领奖时需要装载给奖励的玩家
	std::set<UID>								m_NoTakeForward;

	//战斗时，玩家不在线，装载
	std::map<UID, UID>							m_mapCombat;

	//玩家的帮战场景
	std::map<UID, TSceneID>						m_mapSceneID;
			
};


#endif
