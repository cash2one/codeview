
#ifndef __THINGSERVER_IFUBENPART_H__
#define __THINGSERVER_IFUBENPART_H__

#include "DSystem.h"
#include "UniqueIDGenerator.h"
#include "IThingPart.h"

//副本模式
enum enFuBenMode
{
	enFuBenMode_Single = 0, //单人模式
	enFuBenMode_Team,		//组队模式
};

//副本进度
struct SFuBenProgressInfo
{
	SFuBenProgressInfo()
	{
        m_SceneID = INVALID_SCENE_ID;
		m_FuBenID = INVALID_FUBEN_ID;
		m_Level = 0;
		m_KillMonsterNum = 0;
	}

	//是否已开启
	bool IsStart()
	{
		return m_Level>1 || m_KillMonsterNum>0;
	}

	TSceneID m_SceneID;			//对应的副本场景ID
	TFuBenID m_FuBenID;			//副本ID
	UINT8    m_Level;			//级别
	UINT8    m_KillMonsterNum;	//杀怪数量
};

struct SFuBenNum
{
	INT16	m_FreeNum;			//还可免费进入次数
	INT16	m_VipNum;			//VIP还可进入次数
	INT16	m_StoneNum;			//灵石还可进入次数
	INT16	m_SynWelNum;		//帮派福利还可进入次数
};


struct IFuBenPart : public IThingPart
{
	//打开副本
	virtual void OpenFuBen() = 0;

	//打开帮派副本面板
	virtual void OpenSynFuBen() = 0;

	//挑战副本
	virtual void ChallengeFuBen(TFuBenID  FuBenID) = 0;

	//挑战帮派副本
	virtual void ChallengeSynFuBen() = 0;

	//重置副本
	virtual void ResetFuBen(TFuBenID  FuBenID) = 0;

	//重置帮派保卫战
	virtual void ResetSynFuBen() = 0;

	//挑战怪物
	virtual void CombatNpc(TFuBenID  FuBenID,UID uidNpc, enCombatIndexMode CombatIndexMode = enCombatIndexMode_Com) = 0;

	//离开副本
	virtual void LeaveFuBen(TFuBenID  FuBenID)=0;

	//进入副本次数
	virtual UINT16 EnterFuBenNum() = 0;

	//副本最大进入次数
	virtual UINT16 MaxEnterFuBenNum() = 0;

	//进入仙剑副本
	virtual void OnEnterSwordFuBen(UID uidGodSword,UINT8 Level) = 0;

	//重置所有仙剑副本进度
	virtual void ResetGodSwordFuBen() = 0;

	//下线时，删除所有副本场景
	virtual void ReleaseScene() = 0;

	//清除副本的次数限制
	virtual void GMCmdClearFuBenNumLimit() = 0;

	//进入帮派保卫战次数
	virtual UINT16 GetEnterSynFuBenNum() = 0;

	//获得进入仙剑副本次数
	virtual UINT16 GetEnterGodSwordNum() = 0;

	//得到最大仙剑副本可进入次数
	virtual UINT16 GetMaxGodSwordEnterNum() = 0;

	//自动快速打普通副本, bOpenHardType为是否开启困难模式
	virtual void CommonFuBenAutoCombat(TFuBenID FuBenID, bool bOpenHardType) = 0;

	//自动快速打帮派副本
	virtual void SynFuBenAutoCombat() = 0;

	//副本还可进次数
	virtual void GetFuBenNum(SFuBenNum & FuBenNum) = 0;

	//组队挑战副本
	virtual void TeamChallengeFuBen(TFuBenID FuBenID, bool bSycProgress = false) = 0;

	//得到副本是否完成过
	virtual bool GetFuBenIsFinished(TFuBenID FuBenID) = 0;

	//得到副本是否有进度
	virtual bool	HaveProcessFuBen(TFuBenID FuBenID) = 0;

	//得到副本进度
	virtual SFuBenProgressInfo * GetFuBenProgressInfo(TFuBenID  FuBenID) = 0;

	//创建副本进度
	virtual SFuBenProgressInfo * CreateFuBenProgress(TFuBenID FuBenID) = 0;

	//检测是否可进入副本
	virtual UINT8	Check_CanEnterFuBen(TFuBenID FuBenID) = 0;

	//进入副本
	virtual UINT8	EnterComFuBen(SFuBenProgressInfo * pFuBenProgree, bool bTeam = false) = 0;

	//询问是否同步进度
	virtual void	AskSynProgress(SFuBenProgressInfo * pMeFuBenProgress, SFuBenProgressInfo * pMemberFuBenProgress, IActor * pMember) = 0;

	//切换副本模式
	virtual void	SetFuBenMode(enFuBenMode FuBenMode) = 0;

	//是否正在组队副本中
	virtual bool	IsInTeamFuBen() = 0;

	//得到队友的进度
	virtual SFuBenProgressInfo * GetTeamMemberFuBenProgress(TFuBenID FuBenID) = 0;

	//增加普通副本进入次数
	virtual void	AddCommonFuBenEnterNum(SFuBenProgressInfo * pFuBenProgress) = 0;

	//设置所进入的普通副本场景ID
	virtual void	SetSceneIDFuBen(TSceneID SceneID) = 0;
};








#endif
