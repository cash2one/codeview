
#ifndef __THINGSERVER_IACTOR_H__
#define __THINGSERVER_IACTOR_H__

#include "ICreature.h"
#include "GameSrvProtocol.h"
#include "DMsgSubAction.h"

struct ISession;

struct IResOutputPart;
struct IFuMoDongPart;
struct ITrainingHallPart;
struct IPacketPart;
struct IEquipPart;
struct IGatherGodHousePart;
struct IXiuLianPart;
struct IFriendPart;
struct IMailPart;
struct IMagicPart;
struct ICombatPart;
struct IFuBenPart;
struct ISynMagicPart;
struct IGodSwordShopPart;
struct ITalismanPart;
struct IActorBasicPart;
struct ITaskPart;
struct IChengJiuPart;
struct IActivityPart;
struct ICDTimerPart;

struct IDouFaPart;

struct ISyndicate;
struct ITeamPart;
struct ISynPart;


//VIP功能类型
enum enVipType
{
	enVipType_TakeResAddStone = 0,			//收取灵石额外获得灵石数
	enVipType_bCanAutoTakeRes,				//是否一键收取灵石
	enVipType_AutoKillAddExp,				//增加挂机获得经验(百分比)
	enVipType_AccelKillDesMoney,			//挂机加速减少多少仙石
	enVipType_AccelKillAddHour,				//挂机加速增加几小时
	enVipType_FinishTrainingFreeNum,		//每天练功可免费加速次数
	enVipType_AddTrainingNum,				//增加几次练功机会
	enVipType_BuySwordEmployMagicDesMoney,	//购买仙剑、招募角色、购买法术书费用减少(百分比)
	enVipType_AddGodSwordWorldNum,			//剑印世界增加次数
	enVipType_AddXiuLianPos,				//增加几个修炼空位
	enVipType_AddSynCombatNum,				//帮派保卫战增加几次
	enVipType_AutoKillSynCombat,			//是否快速攻打帮派保卫战
	enVipType_bFreeSellGoods,				//是否交易免寄售费
	enVipType_bFreeRemoveGem,				//是否免费摘除宝石
	enVipType_AddFuBenNum,					//副本免费进入增加几次
	enVipType_bAutoKillFuBen,				//是否快速攻打副本
	enVipType_UnLoadEmployAddExp,			//解雇角色获得经验增加(百分比)
	enVipType_AddStrongerRate,				//强化几率提升(百分比)
	enVipType_AddMaxHonorToday,				//荣誉上限增加
	enVipType_AddGetHonor,					//每次获得荣誉时增加
	enVipType_AddXLNimbus,					//修炼灵气获得增加(百分比)
	enVipType_AddEnterTalismanNum,			//增加法宝世界进入次数
	enVipType_AddDuoBaoCreditUp,			//增加夺宝声望上限
	enVipType_AddSynWarCreditUp,			//增加帮战声望上限
	enVipType_AddGetCredit,					//增加每次获得声望时
};

struct IActor : public ICreature
{

	//设置网络通讯接口
	virtual void SetSession(ISession * pSession) = 0;

	//和玩家客户端一一对应的网络通讯接口
	virtual ISession*  GetSession(void) = 0;

		//////////////////////////////////////////////////////////////////////////
	// 描  述：重新计算生物的属性，然后更新客户端的属性数据
	// 输  入：bSyncProp为FALSE表示不更新客户端的属性数据
	// 备  注：1、生物创建时，计算生物属性
	//         2、生物更换了装备，计算生物属性
	//         3、仅广播生物已改变的公共属性给周围的生物
	//         4、仅同步生物已改变的属性给客户端
	//////////////////////////////////////////////////////////////////////////
	virtual void RecalculateProp(bool bSyncProp = true)=0;

		
	//保存玩家的各种数据,速度较慢
	virtual	void	SaveData()=0;

	//立即保存玩家基本数据（不含物品，宠物），
	virtual	bool	SaveActorBasicData()=0;

	//保存物品数据，如果只需要立即保存物品数据，调这个接口，快很多
	virtual bool		SaveGoodsData()=0;

	//按属性实时保存，速度最快,注意目前只支持有限的几个属性，包括（经验（经验30级以下内部实时存盘,外部不用处理）,金币，元宝），防止当机丢数据，实时存盘
	virtual	bool	SaveDataByProperty(enCrtProp propID)=0;

	//发送数据
	virtual bool  SendData(OStreamBuffer & osb) = 0;

	//资源产出part
	virtual IResOutputPart * GetResOutputPart() = 0;


	//伏魔洞
	virtual IFuMoDongPart * GetFuMoDongPart() = 0;

		//练功堂
	virtual ITrainingHallPart * GetTrainingHallPart()=0;

	//背包
	virtual IPacketPart *  GetPacketPart()=0;

		//装备栏
	virtual IEquipPart * GetEquipPart() = 0;

		//法术栏
    virtual IMagicPart * GetMagicPart()=0;

	//战斗
	virtual ICombatPart * GetCombatPart() = 0;

	//副本
	virtual IFuBenPart * GetFuBenPart() = 0;

	//聚仙楼
	virtual IGatherGodHousePart * GetGatherGodHousePart() = 0;

	//修炼
	virtual IXiuLianPart * GetXiuLianPart() = 0;

	//好友
	virtual IFriendPart * GetFriendPart() = 0;

	//邮件
	virtual IMailPart * GetMailPart() = 0;

	//帮派技能
	virtual ISynMagicPart * GetSynMagicPart() = 0;

	//剑冢
	virtual IGodSwordShopPart * GetGodSwordShopPart() = 0;

	//法宝
	virtual ITalismanPart * GetTalismanPart() = 0;

	//基本
	virtual IActorBasicPart * GetActorBasicPart() = 0;

	//任务
	virtual ITaskPart * GetTaskPart() = 0;
	
	//斗法
	virtual IDouFaPart * GetDouFaPart() = 0;

	//成就
	virtual IChengJiuPart * GetChengJiuPart() = 0;

	//活动
	virtual IActivityPart * GetActivityPart() = 0;

	//冷却
	virtual ICDTimerPart * GetCDTimerPart() = 0;

	//组队
	virtual ITeamPart *	GetTeamPart() = 0;

	//帮派
	virtual ISynPart * GetSynPart() = 0;

	//获得主人，如果返回NULL，表示体身即为主角
	virtual IActor * GetMaster()=0;

	//获得雇用
	virtual IActor * GetEmployee(int nIndex) = 0;
	virtual IActor * GetEmployee(UID uidActor) = 0;

	//获得招募角色个数
	virtual UINT8 GetEmployeeNum() = 0;
	//nIndex == -1表示自动找个空位置插入
	virtual bool  SetEmployee(int nIndex, IActor* pEmployee) = 0;

	//通知客户端通过私有数据创建thing
	virtual bool NoticClientCreatePrivateThing(UID uidThing) = 0;

	//通知客户端通过公用数据创建thing
	virtual bool NoticClientCreatePublicThing(UID uidThing) = 0;

	//通知销毁物品
	virtual bool NoticClientDestroyThing(UID uidThing) = 0;

	//通知Thing属性改变
	virtual bool NoticClientUpdateThing(UID uidThing,UINT8 PropID,INT32 Value) = 0;

	//通知Thing属性改变(带小数的)
	virtual bool NoticClientUpdateThingFloat(UID uidThing,UINT8 PropID, float Value) = 0;

	//发送物品配置信息
	virtual void SendGoodsCnfg(TGoodsID GoodsID) = 0;

	//发送招募角色配置信息
	virtual void SendEmployeeCnfg(TActorID EmployeeID) = 0;

	//发送法术等级配置
	virtual void SendMagicLevelCnfg(TMagicID MagicID) = 0;

	//设置玩家的招募角色UID值
	virtual bool SetEmployUID(const void * buf, int nLen) = 0;

	//发送帮派技能配置
	virtual void SendSynMagicCnfg(TSynMagicID SynMagicID) = 0;

	//回到主场景
	virtual void ComeBackMainScene() = 0;

	//添加关注的玩家
	virtual bool AddToAttentionUser(UID uid_DestUser) = 0;

	//移除关注
	virtual void RemoveToAttentionUser(UID uid_DestUser) = 0;

	//设置名字
	virtual void SetName(const char * szNewName) = 0;

	//解雇招募角色
	virtual void UnLoadEmployee(UID EmployeeUid, enUnLoadEmployeeRetCode & RetCode) = 0;

	//得到玩家能力
	virtual enNenLiType GetNenLi() = 0;

	//获得沉迷系数
	virtual float GetWallowFactor() = 0;

	//当等级或资质变化时,得重新计算部分基本属性
	virtual void LevelOrAptitudeChange() = 0;

	//获取上次存盘时间
	virtual UINT32 GetLastSaveToDBTime() = 0;

	//给出战角色获取经验
	virtual void CombatActorAddExp(INT32 nAddExp, bool bMultipExp = true, INT32 * pNewExp = 0/*, bool bLianGong = false, IActor * pActor = 0*/) = 0;

	//得到给各模块使用的标志
	virtual bool GetUseFlag(enUseFlag UseFlag) = 0;

	//设置给各模块使用的标志
	virtual void SetUseFlag(enUseFlag UseFlag, bool bValue) = 0;

	//得到我的帮派
	virtual ISyndicate * GetSyndicate() = 0;

	//得到我的帮派ID
	virtual TSynID	 GetSynID() = 0;

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing() = 0;

	//得到VIP配置值
	virtual INT32	GetVipValue(enVipType VipType) = 0;

	//保存玩家属性
    virtual bool SaveActorProp() = 0;

	//重新计算玩家战斗力
    virtual void RecalculateCombatAbility() = 0;

   //得到玩家在干什么
   virtual enActorDoing GetActorDoing() = 0;

   //克隆
   virtual IActor * MakeCloner() = 0;

   //是否是克隆人
   virtual bool  IsClone() = 0;

   //多倍经验系数
   virtual float GetMultipExpFactor() = 0;

     //计算灵力
	virtual INT32 RecalculateSpiri(float Aptitude,INT32 Level,INT32 ForeverSpirit,INT32 LayerSpirit) = 0;

	//计算护盾
	virtual INT32 RecalculateShield(float Aptitude,INT32 Level,INT32 ForeverShield,INT32 LayerShield) = 0;


	//计算身法
	virtual INT32 RecalculateAvoid(float Aptitude,INT32 Level,INT32 ForeverAvoid,INT32 LayerAvoid) = 0;


	//计算气血
	virtual INT32 RecalculateBoold(float Aptitude,INT32 Level,INT32 ForeverBoold,INT32 LayerBoold) = 0;

	//设置是否在战斗
	virtual void	SetIsInCombat(bool bInCombat) = 0;

	//得到是否在战斗
	virtual bool	GetIsInCombat() = 0;

	//得到是否在夺宝中
	virtual bool	IsInDuoBao() = 0;

	//得到是否在组队副本中
	virtual bool	IsInTeamFuBen() = 0;

	//得到是否有队伍
	virtual bool	HaveTeam() = 0;

	//设置是否是自己在线
	virtual void	SetIsSelfOnline(bool bSelf) = 0;

	//得到是否是自己在线
	virtual bool	GetIsSelfOnline() = 0;

	//得到是否在快速组队等待中
	virtual bool	IsInFastWaitTeam() = 0;
};


#endif

