
#ifndef __THINGSERVER_MAINUIMGR_H__
#define __THINGSERVER_MAINUIMGR_H__

#include "IMessageDispatch.h"
#include "ITimeAxis.h"
#include "XuanTianFuBen.h"

class MainUIMgr : public IMsgRootDispatchSink, public ITimerSink
{
	enum enTimerID
	{
		enTimerID_ResetGodSwordFuBen = 0,		//重置仙剑副本
	};

public:
	MainUIMgr();
	~MainUIMgr();

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	virtual void OnTimer(UINT32 timerID);

	//打开日常
	INT32 GetRemainTimeOpenXT(void);

	//玩家保存玄天数据
	void SaveXTData(IActor * pActor);

	//重新加载玄天怪物信息
	void ReloadXTMonsterCnfg();

private:
	//打开副本
	void OnOpenFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//挑战副本
	void OnChallengeFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//重置副本
	void OnResetFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);


	//孕育法宝物
	void OnSpawnTalismanGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//领取法宝孕育物
	void OnTakeTalismanGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//提升法宝品质
	void OnUpgradeTalismanQuality(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//仙剑升级
	void OnUpgradeSword(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//进入仙剑副本
	void OnEnterSwordFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开仙剑面板
	void OpenGodSword(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//认证通过了
    void OnIdentity(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//新手引导步骤
	void NewPlayerGuideIndex(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看普通副本掉落物品
	void ViewFuBenDropGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	void NewPlayerGuideConfirm(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//普通副本自动快速打怪
	void CommonFuBenAutoCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开查看掉落界面
	void OpenViewDrop(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//选择是否同步进度
	void SelectSynProgress(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开玄天页面
	void OpenXuanTian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//显示玄天奖励
	void ShowXTForward(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//攻击玄天BOSS
	void AttackXTBoss(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//使用仙石缩短玄天再次攻击时间
	void MoneyLessAttackTime(IActor *pActor,UINT8 nCmd, IBuffer & ib);



	//组队挑战副本
	void TeamChallgeFuBen(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//显示玄天伤害排行
	void ViewXTDamageRank(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//显示最后一击
	void ViewLastKill(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:
	XuanTianFuBen	m_XuanTianFuBen;
};


#endif
