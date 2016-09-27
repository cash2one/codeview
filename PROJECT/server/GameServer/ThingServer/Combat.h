
#ifndef __THINGSERVER_COMBAT_H__
#define __THINGSERVER_COMBAT_H__

#include "IActor.h"

#include "ICombatPart.h"
#include "ITimeAxis.h"
#include <vector>

struct IActor;
struct ICreature;
struct IMagic;
class Combat;

//战斗中玩家最大可以施放法术的数量
#define MAX_FIGHTE_MAGIC_NUM   9



//战斗者
class Fighter : public IFighter,public ITimerSink
{
	enum{enTimerID_Magic,};  //施放法术
public:
	Fighter(Combat * pCombat,ICreature *  pCreature,UINT8 nPos,bool bAutoFighte,bool bNeedSync,bool bOneSelf);
	virtual ~Fighter();

	void Start();

	void Stop();

	std::vector<TGoodsID> GiveUserDropGoods(UINT16 DropID);

	UID GetUID(){return m_pCreature->GetUID();}

	ICreature * GetCreature(){return m_pCreature;}
	UINT8       GetPos(){return m_Pos;}
	bool        IsAutoFighte(){return m_bAutoFighte;}	
	IMagic *    GetMagic(UINT8 Index){if(Index>=ARRAY_SIZE(m_MagicList)) return 0; return m_MagicList[Index];}
	bool        IsNeedSync(){return m_bNeedSync;}
	void        SetSync(bool bNeedSync){  m_bNeedSync = bNeedSync;}
	UINT8       CurMagicIndex(){return m_CurMagicIndex;}
	bool        IsOneSelf(){return m_bOneSelf;}
	INT32       GetGiveExp(){return m_GiveExp;}

	Fighter*   GetMasterFighter(){return m_pMasterFighter;}
	
	void SetMasterFighter(Fighter* pFighter){m_pMasterFighter = pFighter;}

	 		//手动施放法术
    void ManualMagic(IActor * pActor,TMagicID MagicID);

	void SetMagic(UINT8 Index,TMagicID MagicID,UINT8 Level);

	//断线
void Fighter::OnOffLine();

	//启动自动施放法术定时器
	void StartMagicTimer();

	//更新CD时间
	void UpdateCDTime();

	//发送数据
	bool SendToClient(OStreamBuffer & osb);

	//是否可以发起物理攻击
	bool IsLaunchPhysicsAttacke();


	//获得生物所中状态中优先级最高的状态的类型信息
	const SStatusTypeCnfg * GetMaxPriorityStatusTypeCnfg();

	//获得仙剑伤害
     INT32  GetGodSwordPhysicsDamage();

	 //计算免伤
      float  CalculateAvoidDamageValue(Fighter * pEnemy);

	  //获得物理爆击参数
     float GetPhysicsAttackCriticalHitParam(Fighter * pEnemy);

	 //获得生物防御值
     INT32 GetDefenseValue();

	//获得物理攻击伤害值
       INT32 GetPhysicsAttackValue(Fighter * pEnemy,float CriticalHitParam);

	   //对boss造成的伤害
	   INT32       GetDamageValueToBoss(){return m_DamageValueToBoss;}

	   void AddDamageValueToBoss(INT32 Value);


	   //是否已死
	   bool IsDie();

	   //自动施放法术
	   void AutotMagic();

	   //是否可以施放法术
     bool CanUseMagic();

	 UINT64  NextMagicTime();


	//计算打怪可以获得的经验
      INT32 CalculateExp(ICreature * pMonster);

	  	//增加状态
	virtual bool AddStatus(TStatusID StatusID,TMagicID MagicID,INT32);

		  	//增加状态
     void OnAddStatus(TStatusID StatusID,TMagicID MagicID,INT32 StatusTime);

	//移除状态
	virtual void OnRemoveStatus(UINT8 StatusType);

	//广播数据
void Broadcast(OBuffer4k & ob);

//设置自动模式
void SetAutoMagicMode();

void SetAckNo(UINT32 AckNo){m_CurAckNo = AckNo;}

UINT32 GetAckNo(){ return m_CurAckNo ;}

//计算物理命中回避参数
float CalculatePhysicsHitAndAvoidParam(IFighter * pEnemy);

	  //获得法术命中回避参数
 virtual float CalculateMagicHitAndAvoidParam(IFighter * pEnemy);


	//判断是否物理攻击命中
bool Fighter::IsPhysicsAttackHit(Fighter * pEnemy);


	//获得爆击值
	virtual INT32 GetCriticalHitValue() ;

		//获得坚韧值
	virtual INT32 GetTenacityValue() ;

		//获得命中值
	virtual INT32 GetHitValue() ;

		//获得回避值
	virtual INT32 GetDodgeValue() ;

		//获得法术回复值
	virtual INT32 GetMagicCDReduceValue();

	//获得仙剑五行
	virtual enWuXing GetGoldSwordWuXing(INT32 Index);

	//设置仙剑五行
	virtual void SetGoldSwordWuXing(INT32 Index,enWuXing WuXing);

public:

	   virtual void OnTimer(UINT32 timerID);

private:
    Fighter   *  m_pMasterFighter;
    Combat    *  m_pCombat;
	ICreature *  m_pCreature;  //生物
	bool         m_bNeedDeleteCreature; //m_pCreature是否需要释放
	UINT8        m_Pos;         //生物站位信息
	bool         m_bAutoFighte; //是否自动战斗
	UINT64       m_NextMagicTime; //下一次放法术时间
	IMagic  *    m_MagicList[MAX_FIGHTE_MAGIC_NUM]; //该生物可以施放的法术
	UINT8        m_CurMagicIndex; //当前施放的法术索引
	bool         m_bNeedSync;  //是否需要同步战斗信息
	bool         m_bOneSelf;  //是否是自己方
	INT32        m_GiveExp; //打怪胜利后可以获得经验数
	INT32        m_DamageValueToBoss; //对boss造成的伤害值

	UINT32       m_CurAckNo; //当前确认号

	enWuXing     m_GoldSwordWuXing[MAX_LOAD_GODSWORD_NUM]; //仙剑五行属性
      

};


//生物阵型信息
struct SCreatureLineupInfoEx
{
	ICreature * m_pCreature;
	UINT8       m_Pos;
	bool        m_bAutoFighte;
};


class Combat : public ICombat, public ITimerSink, public IVoteListener,public IEventListener,public IActionListener
{
	enum
	{
		TimerID_Init = 0, //初始化
		TimerID_Physics ,//物理攻击
	};
public:
    Combat(UINT32 Param = 0);
	virtual ~Combat();

	UINT64 GetCombatID();

	virtual void Release();

		//结束战斗,bOneselfWin发起战斗方胜
	virtual void EndCombat(enCombatResult CombatResult);

		//确认指令
	virtual void AckAction(IActor* pActor,SCombatActionAck & Req);

	//指定地点战斗
bool CombatWithScene(enCombatType CombatType,IActor* pActor,UID uidEnemy,ICombatObserver * pCombatObserver, const char* szSceneName,
					 IGameScene * pGameScene, enJoinPlayer JoinPlayer,enCombatMode CombatMode,UINT8 ChiefIndex,UINT16 DropID=0);

//手动施放法术
bool ManualMagic(IActor* pActor,CS_CombatFireMagic_Req & Req);

//广播数据
void Broadcast(OBuffer4k & ob,UINT32 AckNo=0);

UINT32 NextAckNo();

//物理攻击
void OnPhysicsAttack();

//施放法术
bool UseMagic(Fighter* pFighter,IMagic* pMagic,INT32 nIndex);

void OnEvent(XEventData & EventData);

void OnAction(XEventData & EventData);

void SetAutoMagicMode(UID uidActor);

private:
	//创建参战人员
Fighter* MakeFighter(ICreature *  pCreature,UINT8 nPos,bool bAutoFighte,bool bOneSelf);

//获得我方参战人员
void GetJoinBattleActor(std::vector<SCreatureLineupInfoEx> &vectEx,IActor* pActor,bool bIsAutoFighte,bool bIsTeam,bool bIsTeamAutoFighte); 

//获得参战生物
void GetJoinBattleCreature(std::vector<SCreatureLineupInfoEx> & vectEx,UID uidEnemy,IGameScene * pGameScene,bool bIsAutoFighte,bool bIsTeam,bool bIsTeamAutoFighte,UINT8 ChiefIndex);

//获得参战人员信息
void  GetLineupInfo(Fighter* pFighter, ICreature * pCreature,OBuffer4k & ob);

  //按身法优先级排序
 std::vector<Fighter*> SortByPrority(std::vector<Fighter*> & vectSelf,
								  std::vector<Fighter*> & vectEnemy);

    //按身法，灵力的优先级把vect2合并到vect1中
  void InsertByPrority(std::vector<Fighter*> & vect1,
								  const std::vector<Fighter*> & vect2);

  //获得当前发动物理攻击参战人
  Fighter* GetLaunchFighter();

   //获得物理攻击目标
  INT32 GetPhysicsAttackedTarget(INT32 AttackLineup,std::vector<Fighter*> & vectTarget);

    //获得法术攻击目标
  INT32 GetMagicAttackedTarget(INT32 AttackLineup,std::vector<Fighter*> & vectTarget);

  //获得攻击力浮动系数
float GetAttackFloatParam();

//检查胜负
bool CheckWinLose();


//移除玩家
void Remove(Fighter * pFighter);

//查询生物中数组中的索引
INT32 GetIndexInVect(std::vector<Fighter*> & vect,Fighter* pCreature);

Fighter * GetFighter(UID uidCreature);

 bool OnVote(XEventData & EventData);

private:
	void OnAddStatus(SS_AddStatus * pAddStatus);

void OnRemoveStatus(SS_RemoveStatus * pRemoveStatus);

void OnCreatureDie(SS_DieAtCombat * pDieAtCombat);

void OnTimerStatus(SS_TimerStatus * pTimerStatus);


public:
	virtual void OnTimer(UINT32 timerID);

private:
   ICombatObserver * m_pCombatObserver;

     
   UID                     m_uidOneself;  //我方主将 
   UID                     m_uidEnmey;    //敌方主将

   std::vector<Fighter*>   m_vectOneself;  //我方战斗人员
   std::vector<Fighter*>   m_vectEnmey;    //敌方战斗人员

   std::vector<Fighter*>   m_vectSortFighter; //按身法排序战人员  

   std::vector<Fighter*>   m_vectAllFighter; //所有人员

   bool                    m_bBegin; //战斗开始
   bool                    m_bEnd;   //战斗是否结束

   UINT16                  m_DropID; //掉落

   TSceneID                m_SceneID;

   UINT64                  m_CombatID;

   	UINT32       m_CurAckNo; //当前确认号
	INT8        m_NeedAckNum; //需要确认玩家数

	OBuffer4k    m_obAction;
	INT16   m_ActionNum;  //指令数量

	enJoinPlayer m_JoinPlayer;

	enCombatMode m_CombatMode;

	UINT32		 m_Param;
};





#endif

