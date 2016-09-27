
#ifndef __COMBATSERVER_COMBATSERVER_H__
#define __COMBATSERVER_COMBATSERVER_H__

#include "ICombatServer.h"
#include "IGameServer.h"

#include "TBuffer.h"
#include "IMessageDispatch.h"
#include <list>
#include "ITimeAxis.h"

struct ICombat;



extern IGameServer * g_pGameServer;

class CombatServer : public ICombatServer,public IMsgRootDispatchSink,public ITimerSink
{
	enum {enTimerID_DelCombat,};
public:
	CombatServer();
	virtual ~CombatServer();

	virtual bool Create();

	virtual void OnTimer(UINT32 timerID) ;

		//创建法术
	virtual IMagic * CreateMagic(const SCreateMagicCnt & CreateMagicCnt); 

		//创建效果
	virtual IEffect * CreateEffect(TEffectID EffectID);

	//创建状态
	virtual IStatus * CreateStatus(TStatusID  StatusID);

	//创建状态
	virtual IStatus * CreateStatus(const SStatusInfo & StatusInfo);

	//玩家pActor和生物uidCreature开始战斗
	virtual bool Combat(IActor * pActor,UID uidCreature);

		//增加战斗
	virtual void AddCombat(ICombat * pCombat);

	//删除战斗
	virtual void DeleteCombat(ICombat * pCombat) ;

			//结束战斗
	virtual void EndCombat(UINT64 CommbatID,enCombatResult CombatResult);

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib);

public:
	
	//发起战斗
	void OnCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		
	//挑战副本中的怪物
	void OnCombatNpcInFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//离开副本
	void OnLeaveFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:

	void ManualMagic(IActor *pActor,UINT64 CombatID,IBuffer & ib);

	void AckAction(IActor *pActor,UINT64 CombatID,IBuffer & ib);


public:

		//释放
	virtual void Release(void);

	virtual void Close();

private:
	typedef std::hash_map<UINT64,ICombat*> MAP_COMBAT;

	MAP_COMBAT  m_mapCombat;

	//需要删除的战斗
	std::list<ICombat*>  m_listDelCombat;

};


#endif
