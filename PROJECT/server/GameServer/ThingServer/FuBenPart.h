
#ifndef __THINGSERVER_FUBENPART_H__
#define __THINGSERVER_FUBENPART_H__

#include "IFuBenPart.h"
#include <hash_map>
#include "DBProtocol.h"
#include "IEventServer.h"
#include "IConfigServer.h"
#include "ICombatPart.h"
#include <set>

struct IActor;
struct SGodSwordFuBenCnfg;
struct IGoods;


class FuBenPart : public IFuBenPart,public IEventListener,public ICombatObserver
{
public:
    FuBenPart();
	virtual ~FuBenPart();

public:
	//战斗结束了
	virtual void OnCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

private:
	void OnCommonCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);

	void OnGoldSwordCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);


	void OnSynGuardCombatOver(const SCombatContext * pCombatCnt, CombatCombatOver * pCombatResult,std::vector<TGoodsID> & vectGoods);


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
	virtual IThing*		GetMaster(void);

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

		//打开副本
	virtual void OpenFuBen();

	//打开帮派副本面板
	virtual void OpenSynFuBen();

	//挑战普通副本
	void ChallengeFuBen(TFuBenID  FuBenID);

	//挑战帮派副本
	virtual void ChallengeSynFuBen();

	//重置副本
	virtual void ResetFuBen(TFuBenID  FuBenID);

	//重置帮派保卫战
	virtual void ResetSynFuBen();

		//挑战怪物 bOpenHardType为是否开启困难模式
	virtual void CombatNpc(TFuBenID  FuBenID,UID uidNpc, enCombatIndexMode CombatIndexMode = enCombatIndexMode_Com);

	//离开副本
	virtual void LeaveFuBen(TFuBenID  FuBenID);

		//进入副本次数
	virtual UINT16 EnterFuBenNum(); 

		//副本最大进入次数
	virtual UINT16 MaxEnterFuBenNum();

	//进入仙剑副本
	virtual void OnEnterSwordFuBen(UID uidGodSword,UINT8 Level);

	//重置所有仙剑副本进度
	virtual void ResetGodSwordFuBen();

	//下线时，删除所有副本场景
	virtual void ReleaseScene();

	//清除副本的次数限制
	virtual void GMCmdClearFuBenNumLimit();

	//进入帮派保卫战次数
	virtual UINT16 GetEnterSynFuBenNum();

	//获得进入仙剑副本次数
	virtual UINT16 GetEnterGodSwordNum();

	//得到最大仙剑副本可进入次数
	virtual UINT16 GetMaxGodSwordEnterNum();

	//自动快速打普通副本, bOpenHardType为是否开启困难模式
	virtual void CommonFuBenAutoCombat(TFuBenID FuBenID, bool bOpenHardType);

	//自动快速打帮派副本
	virtual void SynFuBenAutoCombat();

	//副本还可进次数
	virtual void GetFuBenNum(SFuBenNum & FuBenNum);

	//组队挑战副本
	virtual void TeamChallengeFuBen(TFuBenID FuBenID, bool bSycProgress = false);

	//得到副本是否完成过
	virtual bool	GetFuBenIsFinished(TFuBenID FuBenID);

	//得到副本是否有进度
	virtual bool	HaveProcessFuBen(TFuBenID FuBenID);

	//得到副本进度
	virtual SFuBenProgressInfo * GetFuBenProgressInfo(TFuBenID  FuBenID);

	//创建副本进度
	virtual SFuBenProgressInfo * CreateFuBenProgress(TFuBenID FuBenID);

	//检测是否可进入副本
	virtual UINT8	Check_CanEnterFuBen(TFuBenID FuBenID);

	//进入副本
	virtual UINT8	EnterComFuBen(SFuBenProgressInfo * pFuBenProgree, bool bTeam = false);

	//询问是否同步进度
	virtual void	AskSynProgress(SFuBenProgressInfo * pMeFuBenProgress, SFuBenProgressInfo * pMemberFuBenProgress, IActor * pMember);

	//切换副本模式
	virtual void	SetFuBenMode(enFuBenMode FuBenMode);

	//是否正在组队副本中
	virtual bool	IsInTeamFuBen();

	//得到队友的进度
	virtual SFuBenProgressInfo * GetTeamMemberFuBenProgress(TFuBenID FuBenID);

	//增加普通副本进入次数
	virtual void	AddCommonFuBenEnterNum(SFuBenProgressInfo * pFuBenProgress);

	//设置所进入的普通副本场景ID
	virtual void	SetSceneIDFuBen(TSceneID SceneID);

private:
	//得到帮派副本进度
	SFuBenProgressInfo * GetSynFuBenProgressInfo();	

	//打通副本可以获得的聚灵气
	INT32 FuBenPart::GetFuBenThroughPolyNimbus();

	//免费进入次数
	UINT8 GetFreeEnterFuBenNum();

	//帮派福利进入次数
	UINT8 GetSynWelfareEnterFuBenNum();

	//支付灵石进入次数
	UINT8 GetCostStoneEnterFuBenNum();

	//普通副本中挑战怪物, bOpenHardType为是否开启困难模式
	void CombatNpcCommon(TFuBenID  FuBenID,UID uidNpc, enCombatIndexMode CombatIndexMode = enCombatIndexMode_Com);

	//挑战帮派普通怪物
	void CombatNpcSyn(TFuBenID  FuBenID,UID uidNpc);

	//挑战仙剑怪物
	void CombatNpcGodSword(TFuBenID FuBenID, UID uidNpc);

	//获得仙剑副本配置信息
    const SGodSwordFuBenCnfg * GetSwordFuBenCnfg(IGoods * pGoods,UINT8 FuBenLevel);

	//得到副本类型
	enFuBenType	GetFuBenType(TFuBenID FuBenID);

	//今天最大可进入帮派副本次数
	UINT16 GetMaxEnterSynFuBenNum();

	//得到VIP进入普通副本次数
	UINT16 GetVipEnterCommonFuBenNum();

	//设置副本完成记录
	void	SetFuBenFinished(TFuBenID FuBenID, bool bFinish);

	//挑战普通副本第一步
	UINT8	ChallengeComFuBenFirst(SFuBenProgressInfo * pFuBenProgress, TFuBenID FuBenID);

	//快速挑战帮派副本第一步
	UINT8	FastChallengeSynFuBenFirst(SFuBenProgressInfo * pFuBenProgress);

	//挑战帮派副本第一步
	UINT8	ChallengeSynFuBenFirst(SFuBenProgressInfo * pFuBenProgress);

	//增加帮派副本进入次数
	void	AddSynFuBenEnterNum(SFuBenProgressInfo * pFuBenProgress);

	//是否能开启困难模式
	bool	CanOpenHardType(TFuBenID FuBenID);

	//检测是否可以组队挑战副本
	UINT8	Check_TeamChallenge(TFuBenID FuBenID);

	//副本次数检测
	UINT8	Check_EnterFuBenNum();

	//普通单人副本战斗胜利
	bool	CommonSingleCombatWin(SFuBenProgressInfo * pFuBenProgress, bool & bOver);

	//普通组队副本战斗胜利
	bool	CommonTeamCombatWin(const SCombatContext * pCombatCnt, SFuBenProgressInfo * pFuBenProgress, bool & bOver);

	//设置困难副本完成记录
	void	SetHardFuBenFinished(TFuBenID FuBenID, bool bFinish);

	//得到困难副本是否完成过
	bool	GetHardFuBenIsFinished(TFuBenID FuBenID);

	//进入副本
    void EnterFuBen(SFuBenProgressInfo * pFuBenProgress,bool bTeam = false);

	//获得当前进度对应的地图ID
    TMapID GetFuBenMapID(SFuBenProgressInfo * pFuBenProgress,UINT8& Level);

	//好友也进副本
    void FriendEnterFuBen(IGameScene * pGameScene,SFuBenProgressInfo * pFuBenProgress);

	//与队友同步成进度大的
	bool	SynTeamProgress(SFuBenProgressInfo * pFuBenProgress, IActor * pMember);

	//同步成队长进度
	bool	SynTeamLeaderProgress(SFuBenProgressInfo * pFuBenProgress, IActor * pMember);

protected:
	IActor *          m_pActor;

	UINT32            m_LastFreeEnterFuBenTime;  //最后一次免费进入且记录副本进度时间
	UINT32			  m_LastStoneEnterFuBenTiem;	 //最后一次付灵石进入且记录副本进度时间
	UINT32			  m_LastSynWelfareFuBenTime; //最后一次帮派福利进入且记录副本进度时间
	UINT32			  m_LastVipEnterFuBenTime;	 //最后一次VIP进入副本时间
	UINT16            m_FreeEnterFuBenNum;       //今天免费进入次数
	UINT16			  m_SynWelfareEnterFuBenNum; //帮派福利进入次数
	UINT16			  m_CostStoneEnterFuBenNum;	 //支付灵石进入次数
	UINT16			  m_VipEnterFuBenNum;		 //今天VIP进入次数


	typedef std::map<TFuBenID,SFuBenProgressInfo > MAP_FUBEN;

	MAP_FUBEN        m_mapFuBen;					//已开启的副本集合.

	//剑印世界
	UINT32			  m_LastEnterFuBenGodSword;		//最后一次进入剑印世界时间

	UINT16			  m_EnterGodSwordWorldNum;		//今天进入仙剑副本次数

	UID				  m_uidGodSword;				//正在进入的仙剑	
	UINT8			  m_FuBenLevel;
	TFuBenID          m_EnterFuBenID;  //正在进入的副本ID

	TSceneID		  m_SceneIDGodSward;			//所进入的仙剑副本场景ID

	TSceneID		  m_SceneIDFuBen;				//所进入的普通副本场景ID

	UINT32			  m_LastEnterSynFuBenTime;		//最后一次进入帮派副本时间
	UINT16			  m_EnterSynFuBenNum;			//进入帮派副本次数

	TBitArray<MAX_FUBEN_ID>   m_FinishedFuben;		//通过的副本记录

	TBitArray<MAX_FUBEN_ID>   m_HardFinishedFuben;	//困难模式通过的副本记录

	enFuBenMode		  m_FuBenMode;					//当前或者最近一场副本模式

	//UINT64				m_CommonCombatID;			//普通副本战斗ID
	std::set<UINT64>	m_setCommonCombatID;		//普通副本战斗ID
	enCombatIndexMode	m_CombatMode;				//普通副本的战斗模式
	UINT64				m_GoldSwordCombatID;		//帮派保卫战
	UINT64				m_SynGuardCombatID;			//帮派保卫战


};




#endif
