
#ifndef __COMBATSERVER_ICOMBATSERVER_H__
#define __COMBATSERVER_ICOMBATSERVER_H__
#include "DSystem.h"

#include "IGameServer.h"
#include "IStatus.h"
#include "ICombatPart.h"


#ifndef BUILD_COMBATSERVER_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"CombatServer.lib")
#endif
#endif

struct IMagic;
struct IEffect;
struct ICombat;

struct SCreateMagicCnt
{
	TMagicID   m_MagicID;  //法术ID
	UINT8      m_Level;    //等级
};

struct ICombatServer
{
	//释放
	virtual void Release(void) = 0;

	virtual void Close()=0;

	//创建法术
	virtual IMagic * CreateMagic(const SCreateMagicCnt & CreateMagicCnt) = 0;

	//创建效果
	virtual IEffect * CreateEffect(TEffectID EffectID) = 0;

	//创建状态
	virtual IStatus * CreateStatus(TStatusID  StatusID) = 0;

	//创建状态
	virtual IStatus * CreateStatus(const SStatusInfo & StatusInfo) = 0;

	//玩家pActor和生物uidCreature开始战斗
	virtual bool Combat(IActor * pActor,UID uidCreature) = 0;

	//增加战斗
	virtual void AddCombat(ICombat * pCombat) = 0;

	//删除战斗
	virtual void DeleteCombat(ICombat * pCombat) = 0;

		//结束战斗
	virtual void EndCombat(UINT64 CommbatID,enCombatResult CombatResult)=0;
};

BCL_API  ICombatServer * CreateCombatServer(IGameServer *pServerGlobal);

#endif
