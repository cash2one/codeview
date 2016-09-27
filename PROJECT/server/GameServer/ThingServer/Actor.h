
#ifndef __THINGSERVER_ACTOR_H__
#define __THINGSERVER_ACTOR_H__

#include "IActor.h"
#include "ThingBase.h"
#include "CreatureBase.h"
#include "IPacketPart.h"
#include "IGodSwordShopPart.h"
#include <set>
#include "ICDTimerPart.h"
#include "ISyndicate.h"

struct IFuMoDongPart;
struct IGatherGodHousePart;
struct ISynMagicPart;
struct IMonster;

class Actor : public CreatureBase<IActor>,public IVoteListener
{
	typedef CreatureBase<IActor> Super;
public:
	Actor();

	virtual ~Actor();

public:

	virtual TMsgSourceType GetEventSrcType(){ return enEventSrcType_Actor;}

	virtual enThing_Class GetThingClass(void){ return enThing_Class_Actor;}

		//初始化，
	virtual bool Create(void);

	virtual void Release();


	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen);

	virtual bool OnSetPrivateContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen);

	virtual bool SetEmployUID(const void * buf, int nLen);
	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：发送消息给本实体
	// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
	// 返回值：
	// 备  注：在此函数内，可能会发FireVote和FireAction
	//////////////////////////////////////////////////////////////////////////
	virtual void OnMessage( UINT32 dwMsg, const void * buf, int nLen);

			//取得本实体的enPartID部件
	virtual IThingPart* GetPart(enThingPart enPartID);

	//向本实体添加部件pPart。如果部件已存在，则会添加失败
	virtual bool AddPart(IThingPart *pPart);

	//删除本实体的enPartID部件，但并不释放这个部件
	virtual bool RemovePart(enThingPart enPartID);


	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本生物的数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值，
	//////////////////////////////////////////////////////////////////////////
	virtual bool AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0);
	
	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本生物的属性，替换原来的值
	// 输  入：数字型属性enPropID，属性值nValue
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0);

			//恢复气血
	virtual void RecoverBlood(IActor * pActor=0); 


	//取得本生物的属性
	virtual INT32 GetCrtProp(enCrtProp enPropID);


	//设置网络通讯接口
	virtual void SetSession(ISession * pSession);

	//和玩家客户端一一对应的网络通讯接口
	virtual ISession*  GetSession(void);

			//////////////////////////////////////////////////////////////////////////
	// 描  述：重新计算生物的属性，然后更新客户端的属性数据
	// 输  入：bSyncProp为FALSE表示不更新客户端的属性数据
	// 备  注：1、生物创建时，计算生物属性
	//         2、生物更换了装备，计算生物属性
	//         3、仅广播生物已改变的公共属性给周围的生物
	//         4、仅同步生物已改变的属性给客户端
	//////////////////////////////////////////////////////////////////////////
	virtual void RecalculateProp(bool bSyncProp = true);

		//获得主人，如果返回NULL，表示体身即为主角
	virtual IActor * GetMaster();

		//获得雇用
	virtual IActor * GetEmployee(int nIndex);

	virtual IActor * GetEmployee(UID uidActor);

	//获得招募角色个数
	virtual UINT8 GetEmployeeNum();
	
	//nIndex == -1表示自动找个空位置插入
	virtual bool  SetEmployee(int nIndex, IActor* pEmployee); 
	
	//保存玩家的各种数据,速度较慢
	virtual	void	SaveData();

	//立即保存玩家基本数据（不含物品，宠物），
	virtual	bool	SaveActorBasicData();

	//保存物品数据，如果只需要立即保存物品数据，调这个接口，快很多
	virtual bool		SaveGoodsData();

	//按属性实时保存，速度最快,注意目前只支持有限的几个属性，包括（经验（经验30级以下内部实时存盘,外部不用处理）,金币，元宝），防止当机丢数据，实时存盘
	virtual	bool	SaveDataByProperty(enCrtProp propID);

	//通知客户端通过私有数据创建thing
	virtual bool NoticClientCreatePrivateThing(UID uidThing);

		//通知客户端通过公用数据创建thing
	virtual bool NoticClientCreatePublicThing(UID uidThing); 

		//通知销毁物品
	virtual bool NoticClientDestroyThing(UID uidThing);

		//通知Thing属性改变
	virtual bool NoticClientUpdateThing(UID uidThing,UINT8 PropID,INT32 Value);

	//通知Thing属性改变(带小数的)
	virtual bool NoticClientUpdateThingFloat(UID uidThing,UINT8 PropID, float Value);

	//发送物品配置信息
	virtual void SendGoodsCnfg(TGoodsID GoodsID);



	//发送招募角色配置信息
	virtual void SendEmployeeCnfg(TActorID EmployeeID);

	//发送法术等级配置
	virtual void SendMagicLevelCnfg(TMagicID MagicID);

	//发送帮派技能配置
	virtual void SendSynMagicCnfg(TSynMagicID SynMagicID);

		//发送数据
	virtual bool  SendData(OStreamBuffer & osb);

	virtual IResOutputPart * GetResOutputPart(); 

		//伏魔洞
	virtual IFuMoDongPart * GetFuMoDongPart(); 

	//练功堂
	virtual ITrainingHallPart * GetTrainingHallPart();

		//背包
	virtual IPacketPart *  GetPacketPart();

			//装备栏
	virtual IEquipPart * GetEquipPart() ;

	//法术栏
    virtual IMagicPart * GetMagicPart() ;

	//战斗
	virtual ICombatPart * GetCombatPart();

	//副本
	virtual IFuBenPart * GetFuBenPart();
	
	//聚仙楼
	virtual IGatherGodHousePart * GetGatherGodHousePart();

		//法宝
	virtual ITalismanPart * GetTalismanPart();

	//修炼
	virtual IXiuLianPart * GetXiuLianPart();

	//好友
	virtual IFriendPart * GetFriendPart();

	//邮件
	virtual IMailPart * GetMailPart();

	//帮派技能
	virtual ISynMagicPart * GetSynMagicPart();

	//剑冢
	virtual IGodSwordShopPart * GetGodSwordShopPart();

	//基本
	virtual IActorBasicPart * GetActorBasicPart(); 

		//任务
	virtual ITaskPart * GetTaskPart() ;

	//斗法
	virtual IDouFaPart * GetDouFaPart();

	//成就
	virtual IChengJiuPart * GetChengJiuPart();

		//活动
	virtual IActivityPart * GetActivityPart() ;

	//冷却
	virtual ICDTimerPart * GetCDTimerPart();

	//组队
	virtual ITeamPart *	GetTeamPart();

	//帮派
	virtual ISynPart * GetSynPart();

		//回到主场景
	virtual void ComeBackMainScene();

	//添加关注的玩家
	virtual bool AddToAttentionUser(UID uid_DestUser);

	//移除关注
	virtual void RemoveToAttentionUser(UID uid_DestUser);

	virtual bool OnVote(XEventData & EventData);

	//设置名字
	virtual void SetName(const char * szNewName);

	//解雇招募角色
	virtual void UnLoadEmployee(UID EmployeeUid, enUnLoadEmployeeRetCode & RetCode);

	//得到玩家能力
	virtual enNenLiType GetNenLi();

		//获得沉迷系数
	virtual float GetWallowFactor();

	//当等级或资质变化时,得重新计算部分基本属性
	virtual void LevelOrAptitudeChange();

	//获取上次存盘时间
	virtual UINT32 GetLastSaveToDBTime();

	//给出战角色获取经验
	virtual void CombatActorAddExp(INT32 nAddExp, bool bMultipExp = true, INT32 * pNewExp = 0/*, bool bLianGong = false, IActor * pActor = 0*/);

			//设置是否需要存盘
	void SetSaveFlag(bool bSave);

	virtual bool GetSaveFlag();

	//得到给各模块使用的标志
	virtual bool GetUseFlag(enUseFlag UseFlag);

	//设置给各模块使用的标志
	virtual void SetUseFlag(enUseFlag UseFlag, bool bValue);

	//得到我的帮派
	virtual ISyndicate * GetSyndicate();

	//得到我的帮派ID
	virtual TSynID	GetSynID();

	//掉线，下线要做的一些事
	virtual void	LeaveDoSomeThing();

	//得到VIP配置值
	virtual INT32	GetVipValue(enVipType VipType);

	//保存玩家属性
    virtual bool SaveActorProp();

	//重新计算玩家战斗力
    virtual void RecalculateCombatAbility();

   //得到玩家在干什么
    virtual enActorDoing GetActorDoing();

   //克隆
   virtual IActor * MakeCloner();

      //是否是克隆人
   virtual bool  IsClone();

      //多倍经验系数
   virtual float GetMultipExpFactor(); 

   //计算灵力
	virtual INT32 Actor::RecalculateSpiri(float Aptitude,INT32 Level,INT32 ForeverSpirit,INT32 LayerSpirit);

	//计算护盾
	virtual INT32 Actor::RecalculateShield(float Aptitude,INT32 Level,INT32 ForeverShield,INT32 LayerShield);


	//计算身法
	virtual INT32 Actor::RecalculateAvoid(float Aptitude,INT32 Level,INT32 ForeverAvoid,INT32 LayerAvoid);


	//计算气血
	virtual INT32 Actor::RecalculateBoold(float Aptitude,INT32 Level,INT32 ForeverBoold,INT32 LayerBoold);

	//设置是否在战斗
	virtual void	SetIsInCombat(bool bInCombat);

	//得到是否在战斗
	virtual bool	GetIsInCombat();

	//得到是否在夺宝中
	virtual bool	IsInDuoBao();

	//得到是否在组队副本中
	virtual bool	IsInTeamFuBen();

	//得到是否有队伍
	virtual bool	HaveTeam();

	//设置是否是自己在线
	virtual void	SetIsSelfOnline(bool bSelf);

	//得到是否是自己在线
	virtual bool	GetIsSelfOnline();

	//得到是否在快速组队等待中
	virtual bool	IsInFastWaitTeam();

	protected:
		INT32 __GetPropValue(enCrtProp PropID);

		bool __SetPropValue(enCrtProp PropID,int Value);

	private:
					//发送怪物法术配置信息
	   void SendMonsterMagicCnfg(IMonster * pMonster);
	   
	   //解雇招募角色,返还经验
	   void UnloadEmployeeBackExp(IActor * pEmployee);

	private:
		ISession *  m_pSession;

		IThingPart * m_pThingParts[enThingPart_Crt_Max-enThingPart_Actor];

		int  m_ActorProp[enCrtProp_Max-enCrtProp_Actor];

		UID      m_uidEmploy[MAX_EMPLOY_NUM];  //招募

		IActor*  m_pMaster;

		std::set<UID>	m_setAttentionUser;	//关注的玩家，会否决这些玩家释放内存

		UINT32  m_LastSaveToDBTime; //最后一次存盘时间

		bool  m_bNeedSave; //是否需要存盘

		TBitArray<enUseFlag_Max>   m_UseFlag;	//给各模块使用的标志

		IActor *  m_SourceActor; //被克隆的玩家

		bool		m_bInCombat;	//是否在战斗

		bool		m_bSelfOnline;	//是否玩家自己在线

		 char  m_szNameOld[THING_NAME_LEN];  //定位存盘时玩家名字变少了问题，临时增加的字段

};





#endif
