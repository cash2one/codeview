
#ifndef __THINGSERVER_ICOMBATPART_H__
#define __THINGSERVER_ICOMBATPART_H__

#include "IThingPart.h"
#include "UniqueIDGenerator.h"
#include <vector>
#include "TBuffer.h"
#include "GameSrvProtocol.h"
#include "IConfigServer.h"

struct IActor;
struct IMagic;
struct ICreature;
struct SStatusTypeCnfg;
struct IGameScene;

//战斗状态
enum enCombatState  VC_PACKED_ONE
{
	enCombatState_Non = 0,  //非战斗态
	enCombatState_Ready,    //准备
	enCombatState_Doing,    //进行态
	enCombatState_Max,
} PACKED_ONE;


//战斗结果
enum enCombatResult  VC_PACKED_ONE
{
	enCombatResult_Win = 0,  //胜利
	enCombatResult_Lose,     //输
	enCombatResult_Peace,     //和

} PACKED_ONE;

//生物阵型信息
struct SCreatureLineupInfo
{
	ICreature * m_pCreature;
	UINT8       m_Pos;
};

struct IFighter
{
	virtual ~IFighter() {}
	virtual UID   GetUID()=0;	
	virtual bool  IsOneSelf()=0;
	virtual ICreature * GetCreature()=0;
	virtual void Stop()=0;

	virtual void AddDamageValueToBoss(INT32 Value)=0;

	//增加状态
	virtual bool AddStatus(TStatusID StatusID,TMagicID MagicID,INT32 )=0;


	//广播数据
	virtual 	void Broadcast(OBuffer4k & ob) = 0;

	  //获得法术命中回避参数
    virtual float CalculateMagicHitAndAvoidParam(IFighter * pEnemy)=0;

	//获得爆击值
	virtual INT32 GetCriticalHitValue() = 0;

		//获得坚韧值
	virtual INT32 GetTenacityValue() = 0;

		//获得命中值
	virtual INT32 GetHitValue() = 0;

		//获得回避值
	virtual INT32 GetDodgeValue() = 0;

		//获得法术回复值
	virtual INT32 GetMagicCDReduceValue() = 0;

		//获得仙剑五行
	virtual enWuXing GetGoldSwordWuXing(INT32 Index)=0;

	//获得是否自动模式
	virtual bool        IsAutoFighte() = 0;


	
};


//战斗上下文
struct SCombatContext
{
	UINT64                     CombatID;
	UID                        uidSource;
	UID                        uidEnemy;
	std::vector<IFighter*>     vectActor;
	UID                        uidUser;
	OBuffer4k                  ob;
	INT32                      DamageValue;
	UINT16                     DropID ;	
	bool                       bIsTeam;
	bool                       bIsAutoFighte;
	UINT32					   Param;
};

struct ICombatObserver
{
	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods) = 0;

	//回调结束
	virtual void BackOver(){};
};


struct ICombat
{
	virtual UINT64 GetCombatID() = 0;

     //手动施放法术
    virtual bool ManualMagic(IActor* pActor,CS_CombatFireMagic_Req & Req)=0;

	virtual void Release()=0;

	//结束战斗
	virtual void EndCombat(enCombatResult CombatResult) = 0;

	//确认指令
	virtual void AckAction(IActor* pActor,SCombatActionAck & Req) = 0;

};

//参战人员定义
enum enJoinPlayer  VC_PACKED_ONE
{
	enJoinPlayer_Oneself     = 1,  //自己
	enJoinPlayer_OneselfTeam = 3,    //自己及队友

	enJoinPlayer_OnlyEnemy   = 4,    //仅敌方一人，
	enJoinPlayer_EnemyTeam   = 12,   //敌方及队友

} PACKED_ONE;

//参战手动模式的人员
enum enCombatMode  VC_PACKED_ONE
{
	enCombatMode_Oneself     = 1,  //自己
	enCombatMode_OneselfTeam = 3,    //自己及队友

	enCombatMode_OnlyEnemy   = 4,    //仅敌方一人，
	enCombatMode_EnemyTeam   = 12,   //敌方及队友

} PACKED_ONE;

struct ICombatPart : public IThingPart
{
	
	//指定地点战斗
	virtual bool CombatWithScene(enCombatType CombatType,UID uidEnemy,UINT64 & CombatID,ICombatObserver * pCombatObserver, const char* szSceneName,
		IGameScene * pGameScene,enJoinPlayer JoinPlayer=(enJoinPlayer)(enJoinPlayer_Oneself|enJoinPlayer_OnlyEnemy),enCombatMode CombatMode=enCombatMode_Oneself , 
		UINT8 ChiefIndex=0,UINT16 DropID=0 , UINT32 Param=0) = 0;

	//和怪物战斗  bOpenHardType为是否开启困难模式
	virtual bool CombatWithNpc(enCombatType CombatType,UID  uidNpc, UINT64 & CombatID,ICombatObserver * pCombatObserver, const char* szSceneName,enCombatIndexMode CombatIndexMode = enCombatIndexMode_Com,
		enJoinPlayer JoinPlayer=(enJoinPlayer)(enJoinPlayer_Oneself|enJoinPlayer_OnlyEnemy),enCombatMode CombatMode=enCombatMode_Oneself, UINT32 Param=0) =0;

	//当前是否可以战斗
	virtual bool CanCombat() = 0;

	//获取参战人员
	virtual std::vector<SCreatureLineupInfo> GetJoinBattleActor(bool bTeam = false) = 0;

	//设置参战
	virtual bool SetJoinBattleActor(CS_EquipJoinBattle_Req & Req) = 0;

	
	virtual  bool RemoveJoinBattleActor(UID uidActor) = 0;

		
	virtual 	void LineupSync() = 0;

	//尝试设置参战，如果参战位已满，则返回false失败
	virtual bool TrySetJoinBattle(UID uidActor) = 0;

	//当杀死了怪物
	virtual void OnKillMonster(UID uidNpc) = 0;

	//计算打怪可以获得的经验
	virtual INT32 CalculateExp(UINT8 ActorLevel, UINT8 NpcLevel) = 0;

	//设置组队阵形
	virtual UINT8 SetTeamLineup(CS_SetTeamLineup & Req, bool bSycMember = false) = 0;

	//同步组队阵形
	virtual void	SycTeamLineup() = 0;

	//设置组队参战状态
	virtual UINT8	SetTeamCombatStatus(bool bJoinCombat, bool bSyc = true) = 0;

	//得到是否参加组队战斗
	virtual bool	GetIsJoinTeamCombat() = 0;

	//同步组队参战状态
	virtual void	SycTeamCombatState() = 0;

};








#endif

