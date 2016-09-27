
#ifndef __THINGSERVER_COMBATPART_H__
#define __THINGSERVER_COMBATPART_H__

#include "DBProtocol.h"
#include "ICombatPart.h"
#include "TBuffer.h"
#include <vector>
#include "UniqueIDGenerator.h"
#include "IEventServer.h"
#include <hash_map>

struct IActor;

struct ICreature;
struct IGameScene;
struct SCombatMapMonster;
class  Combat;


//生物阵营信息
struct SCreatureCampInfo
{
	ICreature * m_pCreature;
	bool        m_bSelf; //是否是自己一方

};



class CombatPart : public ICombatPart,public IEventListener
{
public:
	CombatPart();
	virtual ~CombatPart();

public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	virtual void OnEvent(XEventData & EventData);

	
	//当前是否可以战斗
	virtual bool CanCombat();

		//指定地点战斗
	virtual bool CombatWithScene(enCombatType CombatType,UID uidEnemy,UINT64 & CombatID,ICombatObserver * pCombatObserver, const char* szSceneName,IGameScene * pGameScene,
		enJoinPlayer JoinPlayer=(enJoinPlayer)(enJoinPlayer_Oneself|enJoinPlayer_OnlyEnemy),enCombatMode CombatMode=enCombatMode_Oneself , UINT8 ChiefIndex=0,UINT16 DropID=0, UINT32 Param=0);

			//和怪物战斗  bOpenHardType为是否开启困难模式
	virtual bool CombatWithNpc(enCombatType CombatType,UID  uidNpc, UINT64 & CombatID,ICombatObserver * pCombatObserver, 
		const char* szSceneName,enCombatIndexMode CombatIndexMode = enCombatIndexMode_Com,enJoinPlayer JoinPlayer=(enJoinPlayer)(enJoinPlayer_Oneself|enJoinPlayer_OnlyEnemy),enCombatMode CombatMode=enCombatMode_Oneself, UINT32 Param=0);

		//当杀死了怪物
	virtual void OnKillMonster(UID uidNpc); 

		//获取参战人员
	std::vector<SCreatureLineupInfo> GetJoinBattleActor(bool bTeam = false); 


			//设置参战
	virtual bool SetJoinBattleActor(CS_EquipJoinBattle_Req & Req);

	
	virtual bool RemoveJoinBattleActor(UID uidActor);

	void LineupSync();

	//尝试设置参战，如果参战位已满，则返回false失败
	virtual bool TrySetJoinBattle(UID uidActor);

	   //计算打怪可以获得的经验
	virtual INT32 CalculateExp(UINT8 ActorLevel, UINT8 NpcLevel);

	//设置组队阵形
	virtual UINT8 SetTeamLineup(CS_SetTeamLineup & Req, bool bSycMember = false);

	  //同步组队阵形
	virtual void	SycTeamLineup();

	//设置组队参战状态
	virtual UINT8	SetTeamCombatStatus(bool bJoinCombat, bool bSyc = true);

	//得到是否参加组队战斗
	virtual bool	GetIsJoinTeamCombat();

	//同步组队参战状态
	virtual void	SycTeamCombatState();



private:


  //创建战斗怪
  bool CreateCombatMapMonster(IGameScene * pGameScene,const SCombatMapMonster * pCombatMapMonster,TMonsterID MonsterID = INVALID_MONSTER_ID,INT32 nBlood=0);

    //获得战斗怪及战位信息
  std::vector<SCreatureLineupInfo> GetCombatMapMonster(UINT8 ChiefIndex,IGameScene * pGameScene);

   //计算打怪可以获得的经验
 INT32 CalculateExp(ICreature * pMonster);

private:
	ICreature * m_pActor;

	bool          m_bNeedSave; //是否需要存盘
	SDBCombatData m_DBCombatData;

	
	UID  m_TeamLineup[MAX_LINEUP_POS_NUM]; //组队阵型

	bool		m_bTeamCombat;	//是否组队参战
};







#endif
