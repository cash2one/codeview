#ifndef __RELATIONSERVER_SYNDICATEMGR_H__
#define __RELATIONSERVER_SYNDICATEMGR_H__

#include "Syndicate.h"
#include "SyndicateMember.h"
#include <hash_map>
#include "DSystem.h"
#include "IMessageDispatch.h"
#include "IDBProxyClient.h"
#include "SyndicateShop.h"
#include "IGameWorld.h"
#include "SynMagic.h"
#include "ISyndicateMgr.h"
#include "GameSrvProtocol.h"
#include "IEventServer.h"
#include "SyndicateCombat.h"

struct IActor;

//权限
enum enPermission VC_PACKED_ONE
{
	enPermission_Invite,		//邀请
	enPermission_Allow,			//批准
	enPermission_RenMian,		//任免
	enPermission_Task,			//任务
	enPermission_Magic,			//技能
	enPermission_Welfare,		//福利
	enPermission_Shop,			//商铺
	enPermission_Strong,		//强化
	enPermission_Guard,			//保卫战

	enPermission_Max,
}PACKED_ONE;


class SyndicateMgr : public ISyndicateMgr, public IMsgRootDispatchSink, public IDBProxyClientSink
{
public:
	SyndicateMgr();
	~SyndicateMgr();

	virtual bool Create();
	virtual void Close();

	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

public:
	//得到玩家的帮派ID
	virtual TSynID				 GetUserSynID(const UID & uid_User);

	//获得玩家的帮派
	virtual ISyndicate *		 GetSyndicate(const UID & uid_User);

	//获得玩家的帮派
	virtual ISyndicate *		 GetSyndicate(TSynID SynID);

	//获得玩家个人的帮派信息
	virtual ISyndicateMember *	 GetSyndicateMember(const UID & uid_User);
	
	//通过邮件加入帮派
	virtual bool				 MailJoinSyndicate(IActor * pActor, TSynID SynID, enMailRetCode & MailRetCode);

	//遍历帮派成员
	virtual void				 VisitAllSynMember(TSynID SynID, IVisitSynMember & visit);

	//得到帮派排名榜
	virtual void				 GetSynRank(std::vector<ISyndicate *> & vectSynRank);


	//得到帮战开始剩余时间
	virtual UINT32	GetRemainStartSynCombat();

	//得到敌对帮派ID
	virtual TSynID GetVsSynID(IActor * pActor);

	//得到帮战排名榜
	virtual void GetSynWarRank(std::vector<ISyndicate *> & vectSynWarRank);

	//更新帮派战力
	virtual void UpdateSynCombatAbility();

	virtual std::string GetVsSynName(IActor * pActor)  ;

private:
	//创建帮派
	void		CreateSyndicate(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//申请加入帮派
	void		ApplyJoinSyn(IActor *pActor, UINT8 nCmd,IBuffer & ib);

	//查看帮派列表
	void		ViewSynList(IActor * pActor, UINT8 nCmd,IBuffer & ib);

	//查看帮派成员列表
	void		ViewSynMemberList(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//查看帮派加入申请者列表
	void		ViewSynApplyList(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//任免
	void		RenMianUser(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//加入帮派, bCreater是否帮派创建者
	bool		AddToSyndicate(const SyndicateMemberInfo & SynMemberInfo);

	//邀请玩家加入帮派
	void		InviteUser(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//批准玩家加入帮派
	void		AllowUser(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//不批准玩家加入帮派
	void		NotAllowUser(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//玩家确认退出帮派
	void		QuitSyn(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//查看帮派商铺
	void		ViewSynShop(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//购买帮派物品
	void		BuySynGoods(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//查看帮派技能
	void		ViewSynMagic(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//学习帮派技能
	void		LearnSynMagic(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//进入帮派界面
	void		EnterSyn(IActor * pActor, UINT8 nCmd, IBuffer & ib);


	
	//打开帮派保卫战面板
	void		OpenSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//进入帮派保卫战
	void		EnterSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//挑战怪物
	void		SynCombatWithNpc(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//招募帮派成员
	void		ZhaoMuSynMember(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//查看帮派福利
	void		ViewSynWelfare(IActor * pActor, UINT8 nCmd, IBuffer & ib);
	
	//重置帮派保卫战
	void		ResetSynCombat(IActor * pActor, UINT8 nCmd, IBuffer & ib);

	//查看帮派副本掉落物品
	void		ViewSynFuBenDropGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//自动快速攻打帮派副本
	void		SynFuBenAutoCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开帮战
	void		OpenSynWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开宣战页面
	void		OpenDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//帮战宣战
	void		DeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//接受宣战
	void		AcceptDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//拒绝宣战
	void		RefuseDeclareWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//进入帮战
	void		EnterSynWar(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//购买战斗次数
	void		BuyCombatNum(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//帮战战斗
	void		SynWarCombat(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//修改帮派公告
	void		ChangeSynMsg(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到帮战数据
	void		GetSynWarData(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//确认帮派胜负弹出框
	void		ClickSynWarbWin(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到帮派副本BOSS信息
	void		GetSynFuBenBossInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	
private:
	//验证是否可以创建帮派
	bool		CanCreateSyndicate(IActor * pActor, const char * szSynName, enSynRetCode & SynRetCode);

	//创建帮派(数据库回调涵数调用)
	void		HandleCreateSyndicate(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//加载帮派的所有数据
	void		LoadSynInfo();

	//加入到帮派成员集合中
	void		AddToSynMemberList(ISyndicateMember * pSynMember);

	//加入到帮派申请者集合中
	void		AddToSynApplyList(const SyndicateApply SynApply);

	//检测是否可以申请加入此帮派
	bool		CheckCanApplyJoinSyn(IActor * pActor, TSynID SynID, enSynRetCode & enRetCode);

	//检测是否可任免
	enSynRetCode CanRenMian(ISyndicateMember * pSynMember, enumSynPosition BeforeRenMian, enRenMianType AfterRenMian);

	//踢出帮派
	void		LetOutOfSyndicate(TSynID SynID, const UID & uid_TargetUser);

	//获得帮派排名
	UINT8		GetSynRank(TSynID SynID);

	//加载数据(数据库回调涵数调用)
	void		HandleLoadSynListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	void		HandleLoadSynMemberListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	void		HandleLoadSynApplyListInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//邀请玩家加入帮派的数据库回调函数
	void		HandleInviteUser(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//允许玩家加入帮
	UINT8		AllowUserJoin(IActor * pActor, UID uidApplyUser);

	//拒绝玩家加入帮
	void		RefuseUserJoin(IActor * pActor, UID uidApplyUser);

	//查看帮派加入申请者列表
	void		__ViewSynApplyList(IActor * pActor);

	//解散帮派
	void		JieSanSyn(IActor * pActor, enSynRetCode & SynRetCode);

	//移除帮派成员
	bool		RemoveSynMember(TSynID SynID, const UID & uid_TargetUser);

	//从帮派申请中删除
	void		DeleteApply(TSynID SynID, UID uidApplyUser);

private:
	std::hash_map<TSynID, ISyndicate *>														m_mapSyndicate;

	typedef std::hash_map<UID, SyndicateApply,std::hash<UID>, std::equal_to<UID>>			MAPSYNAPPLY;

	std::hash_map<TSynID, MAPSYNAPPLY>														m_mapSyndicateApply;	//帮派加入申请者的集合

	std::hash_map<UID, TSynID, std::hash<UID>, std::equal_to<UID>>							m_mapSynID;				//快速得到玩家的帮派ID

	SyndicateShop			m_SyndicateShop;		//帮派商铺

	SynMagic				m_SynMagic;				//帮派技能

	SyndicateCombat			m_SynWar;				//帮战


};

#endif
