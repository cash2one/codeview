#ifndef __THINGSERVER_CHALLENGE_H__
#define __THINGSERVER_CHALLENGE_H__

#include "GameSrvProtocol.h"
#include "IDBProxyClient.h"
#include "IEventServer.h"
#include <map>
#include "ICombatPart.h"
#include <hash_set>

struct IActor;

class Challenge : public IDBProxyClientSink, public IEventListener,public ICombatObserver
{
public:
	//刷新我的挑战信息
	void	FlushMyChallenge(IActor * pActor);

	//刷新我的挑战回放信息
	void	FlushMyChallengeRecord(IActor * pActor);

	//查看等级组排行
	void	ViewLvGroup(IActor * pActor, enLevelGroup lvGroup, INT32 BeginPos, INT32 Num);

	//挑战玩家
	void	ChallengeUser(IActor * pActor, UID m_uidEnemy);

	//IDBProxyClientSink接口
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual void	OnEvent(XEventData & EventData);

	//排名前五及前5%的奖励
	void	LvGroupRankForward(UID uidUser, enLevelGroup lvGroup, UINT8 nRank);

	//购买战斗次数
	void BuyCombatNum(IActor * pActor);

public:
	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);


private:
	void	HandleFlushMyChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	HandleGetLvGroupRank(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	HandleCheckCanChallenge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	HandleFlushMyChallengeRecord(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//开始战斗
	void	BeginCombat(IActor * pActor, IActor * pEnemy);

	//进入挑战场景
	bool	EnterChallengeScene(IActor * pActor, IActor * pEnemy);

	//战斗奖励
	void	Forward(IActor * pActor, IActor * pEnemy, bool bWin, const OBuffer4k & ob);


private:
	//枯监听创建的玩家
	std::map<UID, UID>		m_mapListen;

	std::hash_set<UINT64>    m_setCombat;

};

#endif
