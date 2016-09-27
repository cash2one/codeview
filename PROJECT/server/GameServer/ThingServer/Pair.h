#ifndef __THINGSERVER_PAIR_H__
#define __THINGSERVER_PAIR_H__

#include "IActor.h"
#include "ICombatPart.h"
#include "UniqueIDGenerator.h"
#include <vector>
#include "Pair.h"

class DuoBaoWar;

enum enReadyStatus
{
	enReadyStatus_No = 0,	//没选择
	enReadyStatus_Quit,		//退出战斗
	enReadyStatus_Combat,	//准备进入战斗
};

struct ActorPairInfo
{
	ActorPairInfo()
	{
		m_uidActor = UID();
		m_Status   = enReadyStatus_No;
	}
	UID				m_uidActor;
	enReadyStatus	m_Status;
};

class Pair : public ICombatObserver
{
public:
	bool    Create(UID uidActor, UID uidEnemy, bool bTeamGroup, DuoBaoWar * pDuoBaoWar);

	//删除
	void	Release();

	//战斗
	void	Combat();

	//发送倒计时和选择战斗框
	void	SendReadyCombat();

	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

	//回调结束
	virtual void BackOver();

	//选择退出战斗
	void	QuitCombat(IActor * pActor);

	//选择战斗
	void	SelectCombat(IActor * pActor);

	//检测是否能战斗
	bool	Check_CanCombat();

	//得到是否战斗结束就删除
	bool	GetIsCombatOverDelete();

	//设置是否战斗结束就删除
	void	SetIsCombatOverDelete(bool bCombatOverDelete);

	//得到战斗是否结束
	bool	GetCombatIsOver();

	//玩家退出或者离队时，检测是否结束
	bool	Check_Over();

	//玩家退出或者离队或者选择战斗时，检测是否开始战斗
	bool	Check_Combat();

private:

	//得到参战人员
	enJoinPlayer GetJoinPlayer();

	//得到战斗模式
	enCombatMode GetCombatMode();

	//得到一个参战人员
	IActor * GetCombatUser(bool bEnemy);

	//逃跑惩罚
	void	QuitForward(UID uidFailer, UID uidWin);

	//战斗奖励
	void	CombatForward(std::vector<ActorPairInfo> & vecFailer, std::vector<ActorPairInfo> & vecWiner, IActor * pFailer, IActor * pWiner);

	//通知创建敌方
	void	NoticeCreateEnemy();

	//清除玩家的夺宝数据
	void	ClearDuoBaoData();

	//通知弹出或者关闭等待框
	void	NoticeWaitTip(IActor * pActor, bool bOpen);
private:

	std::vector<ActorPairInfo>	m_vecUser;		//甲方

	std::vector<ActorPairInfo>	m_vecEenmy;		//敌方

	DuoBaoWar				  * m_pDuoBaoWar;

	bool						m_bCombatOverDelete;	//是否战斗结束就删除

	bool						m_bCombatOver;			//是否战斗结束

	bool						m_bInCombat;			//是否在战斗中


};

#endif
