
#ifndef __GAMESERVER_DMSGSUBACTION_H__
#define __GAMESERVER_DMSGSUBACTION_H__

#include "BclHeader.h"
#include <vector>
#include "FieldDef.h"
#include "DSystem.h"
#include "GameSrvProtocol.h"

struct IMonster;


//事件源类型
enum enEventSrcType VC_PACKED_ONE
{
	enEventSrcType_Root = 0, //根
	enEventSrcType_Thing,    //thing
	enEventSrcType_Creature,    //Creature
	enEventSrcType_Actor,   //Actor
	enEventSrcType_Monster,   //Monster
    enEventSrcType_Building, //Building
	enEventSrcType_Goods,    //物品
	enEventSrcType_Max,
    
}  PACKED_ONE;


//消息源继承关系
static TMsgSourceType g_SourceType[enEventSrcType_Max] = 
{
	enEventSrcType_Root,
	enEventSrcType_Root,
	enEventSrcType_Thing,
	enEventSrcType_Creature,
	enEventSrcType_Creature,
	enEventSrcType_Root,
	enEventSrcType_Thing,
};




//CIRCULTYPE：流通类型（4位）
//ACTIONID：行为事件的子消息码（8位）
#define MAKE_MSGID(CIRCULTYPE, ACTIONID)	UINT16(CIRCULTYPE << 8 | ACTIONID)

//获取消息的CIRCULTYPE（流通类型）
#define GET_CIRCULTYPE(MSGID)				(MSGID >> 8)

//获取消息的ACTIONID（行为事件子消息码）
#define GET_ACTIONID(MSGID)				(MSGID & 0xFF)



//////////////////////////////////////////////////////////////////////////
///////////////////////////// 消息流通类型 ///////////////////////////////
//[CSC]:  服务器与客户端相互通信的消息码。
#define CIRCULTYPE_CSC						1

//[SS]:  服务器各功能相互通信的消息码（只会用在服务器）
#define CIRCULTYPE_SS						3

//[CC]:  客户端各功能相互通信的消息码（只会用在客户端）
#define CIRCULTYPE_CC						4


/////////////////////////////////////////////////////////////////////////////////////////

//命令字与数据结构的映射
#define TASK_ALL_CMD_TO_TYPE  \
	 YASK_CMD_TO_TYPE(enMsgID_TakeBuildingRes,      SS_TakeBuildingRes) \
     YASK_CMD_TO_TYPE(enMsgID_BuyGoods,             SS_BuyGoodsContext) \
	 YASK_CMD_TO_TYPE(enMsgID_OnEquip,              SS_OnEquip) \
	 YASK_CMD_TO_TYPE(enMsgID_AttackFuMoDong,       SS_AttackFuMoDong) \
	 YASK_CMD_TO_TYPE(enMsgID_OnHookFuMoDong,       SS_OnHookFuMoDong) \
	 YASK_CMD_TO_TYPE(enMsgID_CreateEmployee,       SS_CreateEmployee) \
	 YASK_CMD_TO_TYPE(enMsgID_UseGoods,             SS_UseGoods) \
	 YASK_CMD_TO_TYPE(enMsgID_EuipMagic,            SS_EquipMagic) \
	 YASK_CMD_TO_TYPE(enMsgID_ThroughFuBen,         SS_ThroughFuBen) \
	 YASK_CMD_TO_TYPE(enMsgID_EquipStronger,        SS_EquipStronger) \
	 YASK_CMD_TO_TYPE(enMsgID_UpLayer,				SS_UpLayer) \
	 YASK_CMD_TO_TYPE(enMsgID_AddSyn,				SS_AddSyn) \
	 YASK_CMD_TO_TYPE(enMsgID_SynCombat,			SS_SynCombat) \
	 YASK_CMD_TO_TYPE(enMsgID_BuyShopMall,			SS_BuyShopMall) \
	 YASK_CMD_TO_TYPE(enMsgID_Training,				SS_Training) \
	 YASK_CMD_TO_TYPE(enMsgID_XiuLian,				SS_XiuLianFinish) \
	 YASK_CMD_TO_TYPE(enMsgID_DouFa,				SS_DouFa) \
	 YASK_CMD_TO_TYPE(enMsgID_TalismanSpawn,		SS_TalismanSpawn) \
	 YASK_CMD_TO_TYPE(enMsgID_MagicUpLevel,			SS_MagicUpLevel) \
	 YASK_CMD_TO_TYPE(enMsgID_LearnSynMagic,		SS_LearnSynMagic) \
	 YASK_CMD_TO_TYPE(enMsgID_Swordkee,				SS_SwordKee) \
	 YASK_CMD_TO_TYPE(enMsgID_AddFriend,			SS_AddFriend) \
	 YASK_CMD_TO_TYPE(enMsgID_KillMonster,			SS_KillMonster) \
	 YASK_CMD_TO_TYPE(enMsgID_GoodsNum,				SS_GoodsNum) \
	 YASK_CMD_TO_TYPE(enMsgID_SynShopBuyGoods,		SS_SynShopBuyGoods) \
	 YASK_CMD_TO_TYPE(enMsgID_UseSynContribution,	SS_UseSynContribution) \
	 YASK_CMD_TO_TYPE(enMsgID_EnterFuMoDong,		SS_EnterFuMoDong) \
	 YASK_CMD_TO_TYPE(enMsgID_FlushGodHouse,		SS_FlushGodHouse) \
	 YASK_CMD_TO_TYPE(enMsgID_ActoreCreate,			SS_ActoreCreateContext) \
	 YASK_CMD_TO_TYPE(enMsgID_UpLevel,				SS_UpLevel) \
	 YASK_CMD_TO_TYPE(enMsgID_EquipInlay,			SS_EquipInlay) \
	 YASK_CMD_TO_TYPE(enMsgID_TaskFinished,			SS_TaskFinished) \
	 YASK_CMD_TO_TYPE(enMsgID_StudyMagic,			SS_StudyMagic) \
	 YASK_CMD_TO_TYPE(enMsgID_SellSuccess,			SS_SellSuccess) \
	 YASK_CMD_TO_TYPE(enMsgID_ChangeFriendRelatin,	SS_ChangeFriendRelation) \
	 YASK_CMD_TO_TYPE(enMsgID_PacketExtend,			SS_PacketExtend) \
	 YASK_CMD_TO_TYPE(enMsgID_UpSecretLevel,		SS_UpSecretLevel) \
	 YASK_CMD_TO_TYPE(enMsgID_PayMoney,				SS_PayMoney) \
	 YASK_CMD_TO_TYPE(enMsgID_Up_VipLevel,			SS_UpVipLevel)\
	 YASK_CMD_TO_TYPE(enMsgID_UpDamageLv,			SS_UpDamageLv) \
	 YASK_CMD_TO_TYPE(enMsgID_UpCreditMagicLv,		SS_UpCreditMagicLv)\
	 YASK_CMD_TO_TYPE(enMsgID_TalismanAddGhost,		SS_TalismanAddGhost) \
	 YASK_CMD_TO_TYPE(enMsgID_UserUpSecretLevel,	SS_UserUpSecretLevel) \
	 YASK_CMD_TO_TYPE(enMsgID_UpGhost,				SS_UpGhost) \
	 YASK_CMD_TO_TYPE(enMsgID_OnCombat,				SS_OnCombat) \
	 YASK_CMD_TO_TYPE(enMsgID_GetSynWarScore,		SS_GetSynWarScore) \
	 YASK_CMD_TO_TYPE(enMsgID_JieSanSyn,			SS_JieSanSyn) \
	 YASK_CMD_TO_TYPE(enMsgID_ActorLogin,			SS_ActorLogin)


//////////////////////////////////////////////////////////////////////////
/////////////////////// 实体行为事件的子消息码 ///////////////////////////

//注意新增加的命令，只能在最后面增加!!!!!
enum enMsgID  VC_PACKED_ONE
{
	enMsgID_RoundStart           = 1,		//新的一回合开始
	enMsgID_Attack               = 2,		//发起攻击
	enMsgID_Attacked             = 3,		//被攻击
	enMsgID_Cure                 = 4,       //受到治疗

	enMsgID_ActoreCreate         = 5,		//角色创建
	enMsgID_ThingDestroy           = 6,		//Thing销毁

	enMsgID_UnloadActor          = 7,		//卸载角色

	enMsgID_TakeBuildingRes      = 8,		//领取建筑资源

	enMsgID_BuyGoods             = 9,       //购买物品

	enMsgID_TaskFinished		 = 10,		//完成一个任务

	enMsgID_UpLevel				 = 11,		//升级
	
	enMsgID_OnEquip				 = 12,		//穿装备
	enMsgID_AttackFuMoDong		 = 13,		//攻打伏魔洞
	enMsgID_OnHookFuMoDong		 = 14,		//伏魔洞挂机和挂机加速事件
	enMsgID_CreateEmployee		 = 15,		//招募一名角色
	enMsgID_UseGoods			 = 16,		//使用物品
	enMsgID_EuipMagic			 = 17,		//装备法术
	enMsgID_ThroughFuBen		 = 18,		//通过副本
	enMsgID_EquipStronger		 = 19,		//强化装备
	enMsgID_UpLayer				 = 20,		//提升人物境界
	enMsgID_AddSyn				 = 21,		//加入帮派
	enMsgID_SynCombat			 = 22,		//帮派保卫战
	enMsgID_BuyShopMall			 = 23,		//商城购物
	enMsgID_Training			 = 24,		//练功
	enMsgID_XiuLian				 = 25,		//修炼
	enMsgID_DouFa				 = 26,		//斗法
	enMsgID_TalismanSpawn		 = 27,		//法宝孕育
	enMsgID_MagicUpLevel		 = 28,		//升级法术
	enMsgID_LearnSynMagic		 = 29,		//学习帮派法术
	enMsgID_Swordkee			 = 30,		//剑气
	enMsgID_AddFriend			 = 31,		//加好友
	enMsgID_KillMonster			 = 32,		//杀死怪物
	enMsgID_GoodsNum			 = 33,		//背包是否有此数量的物品
	enMsgID_SynShopBuyGoods		 = 34,		//帮派商铺购买物品
	enMsgID_UseSynContribution	 = 35,		//使用帮贡
	enMsgID_EnterFuMoDong		 = 36,		//进入伏魔洞
	enMsgID_FlushGodHouse		 = 37,		//刷新聚仙楼
	enMsgID_TalismanWorld		 = 38,		//完整游玩法宝世界
	enMsgID_EquipInlay			 = 39,		//镶嵌宝石
	enMsgID_FinishTask			 = 40,		//完成任务
	enMsgID_StudyMagic			 = 41,		//学会法术
	enMsgID_SellSuccess			 = 42,		//成功出售一件物品
	enMsgID_ChangeFriendRelatin  = 43,		//改变好友度
	enMsgID_PacketExtend		 = 44,		//背包扩充
	enMsgID_UpSecretLevel		 = 45,		//仙剑的提升剑诀等级
	enMsgID_EnterScene			 = 46,		//进场景
	enMsgID_CombatOver           = 47,      //战斗结束
	enMsgID_QuitSyn				 = 48,		//退出帮派

	enMsgID_MyCampAttacked      = 49,       //本方人员受到攻击

	enMsgID_PayMoney			= 50,		//充值

	enMsgID_UnloadEmployee		= 51,		//解雇

	enMsgID_Up_VipLevel			= 52,		//vip升级

	enMsgID_Get_ActorDoing		= 53,		//发出事件，得到玩家在干嘛

	enMsgID_UpDamageLv		= 54,		    //提升剑诀伤害等级

	enMsgID_UpCreditMagicLv		= 55,		//提升声望技能等级

	enMsgID_ActorLogout         = 56,       //玩家退出登陆

	enMsgID_RemoveStatus        = 57, //移除状态

	enMsgID_AddStatus           = 58, //增加状态

	enMsgID_DieAtCombat           = 59, //战死沙场

	enMsgID_TalismanAddGhost      = 60, //法宝附灵

	enMsgID_TimerStatus            = 61, //定时状态启动

	enMsgID_QuitTeam            = 62,	//离队

	enMsgID_UserUpSecretLevel	= 63,	//人物的提升剑诀等级

	enMsgID_UpGhost				= 64,	//升级灵件

	enMsgID_OnCombat			= 65,	//战斗

	enMsgID_GetSynWarScore		= 66,	//获得帮战功勋

	enMsgID_JieSanSyn			= 67,   //解散帮派

	enMsgID_ActorLogin			= 68,	//玩家登陆
	

	enMsgID_Max,

} PACKED_ONE;

//新的战斗回合开始现场
struct SRoundStartContext
{
	INT32  m_BaseDamageValue; //基础伤害
	INT32  m_AddDamageValue; //受到伤害值
	INT32  m_AddBlood;       //增减血量
};

//攻击事件现场
//命令字:enMsgID_Attack
struct SAttackContext
{
	SAttackContext()
	{
        m_BaseAttackValue = 0;
		m_AddAttackValue = 0;		
		m_uidSource = UID();
		m_uidTarget = UID();
		m_MagicID = INVALID_MAGIC_ID;
		m_AttackType = enAttackType_Max;
	}
	UID           m_uidSource;  //发起者
	UID           m_uidTarget;  //被攻击目标
	enAttackType  m_AttackType; //类型
	TMagicID      m_MagicID;    //法术ID

	INT32  m_BaseAttackValue; //基础攻击力
	INT32  m_AddAttackValue; //效果增减的攻击力
	
};

//受攻击现场
//命令字:enMsgID_Attack
struct SAttackedContext
{
	SAttackedContext()
	{
        m_BaseDamageValue = 0;
		m_AddDamageValue = 0;		
		m_uidSource = UID();
		m_uidTarget = UID();
		m_MagicID = INVALID_MAGIC_ID;
		m_DamageType = enDamageType_Max;
		m_SuckBloodFactor = 0;
		m_AddBloodFactor = 0;
		m_bAvoidDie = false;
		m_bDefense = false;
		
	}
	UID           m_uidSource;  //发起者
	UID           m_uidTarget;  //被攻击目标
	enDamageType  m_DamageType; //类型
	TMagicID      m_MagicID;    //法术ID

	INT32  m_BaseDamageValue; //基础伤害
	INT32  m_AddDamageValue; //受到伤害值
	

	float  m_SuckBloodFactor;  //吸血系数

	float  m_AddBloodFactor; //回血系数

	bool   m_bAvoidDie;  //是否免死

	bool   m_bDefense; //是否防御
};

 //本方人员受到攻击
//命令字enMsgID_MyCampAttacked      = 49, 
struct SMyCampAttackedContext
{
	UID                  m_uidSource;  //发起者
	std::vector<UID>     m_uidTarget;  //被击中的目标	

	std::vector<UINT8>  m_vectRemoveStatus; //移除的状态类型
};

//Thing销毁
//命令字:enMsgID_ThingDestroy
struct SS_ThingDestroyContext
{
	UID   m_uidThing;
};

//角色创建现场
//命令字:enMsgID_ActoreCreate
struct SS_ActoreCreateContext
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_uidActor);
	FIELD(bool,		m_bNewUser);	//是否新创建玩家

	FIELD_END();
};


//卸载角色
//命令字:enMsgID_UnloadActor
struct SS_UnloadActorContext
{
	UID        m_uidActor;
};

//领取建筑资源
//enMsgID_TakeBuildingRes
struct SS_TakeBuildingRes
{
	FIELD_BEGIN();
	FIELD(UINT8,	m_BuildingType);	//建筑类型
	FIELD(INT32,	m_AddValue);		//建筑资源产量增加多少(千分之几)

	FIELD_END();
};

enum enMoneyType
{
	enMoneyType_Money = 0,	//仙石
	enMoneyType_Stone,	//灵石
	enMoneyType_Ticket,	//礼卷
	enMoneyType_Honor,	//荣誉
	enMoneyType_Credit,	//声望

	enMoneyType_Max,
};

enum enBugPlaceType
{
	enBugPlaceType_ShopMall = 0,	//商城
	enBugPlaceType_GodGather,		//聚仙楼
	enBugPlaceType_GodSword,		//剑冢
};
//购买物品
struct SS_BuyGoodsContext
{
    FIELD_BEGIN();
	FIELD(TGoodsID       , m_GoodsID);		//物品ID
	FIELD(UINT8          , m_Category);		//物品一级分类取值 enGoodsCategory
	FIELD(UINT8          , m_SubClass);		//物品二级分类取值 enGoodsSecondType
	FIELD(UINT8          , m_ThreeClass);	//物品三级分类
	FIELD(UINT8          , m_MoneyType);	//所用货币类型
	FIELD(UINT8			 , m_BuyPlaceType); //购买场所

	FIELD_END();
};

//任务完成
struct SS_TaskFinished
{
	FIELD_BEGIN();
	FIELD(TTaskID,		m_TaskID);
	FIELD(UINT8,		m_TaskClass);	//任务类型

	FIELD_END();
};

//玩家升级
struct SS_UpLevel
{
	FIELD_BEGIN();
	FIELD(UINT8,		m_Level);		//升到的级数
	FIELD(UINT64,		m_uidUser);		//玩家等级

	FIELD_END();
};

//穿装备
struct SS_OnEquip
{
	FIELD_BEGIN();
	FIELD(UINT8,		m_Pos);			//装备位置
	FIELD(TGoodsID,		m_GoodsID);
	FIELD(UINT8,		m_Category);	//物品一级分类取值 enGoodsCategory

	FIELD_END();

};

//攻打伏魔洞
struct SS_AttackFuMoDong
{
	FIELD_BEGIN();
	FIELD(UINT8,		m_Floor);	//第几层
	FIELD(bool,			m_bWin);	//是否胜利

	FIELD_END();
};

//伏魔洞挂机
struct SS_OnHookFuMoDong
{
	FIELD_BEGIN();
	FIELD(bool,		m_bAccel);	//是否挂机加速
	FIELD(bool,		m_bOnHook);	//是否挂机
	FIELD(UINT32,	m_DecTime); //挂机减少时间
	FIELD(bool,	    m_bOnHourMore); //是否已经修炼大于1小时

	FIELD_END();
};

//招募角色
struct SS_CreateEmployee
{
	SS_CreateEmployee() : m_EmployeeID(0), m_Index(0), m_bChangQi(false)
	{
	}

	FIELD_BEGIN();
	FIELD(TEmployeeID, m_EmployeeID);	//招募角色配置表中的ID
	FIELD(UINT8,	   m_Index);		//第几个招募角色
	FIELD(bool,		   m_bChangQi);		//是否是传奇人物

	FIELD_END();
};

//使用物品
struct SS_UseGoods
{
	FIELD_BEGIN();
	FIELD(TGoodsID       , m_GoodsID);		//物品ID
	FIELD(UINT8          , m_Category);		//物品一级分类取值 enGoodsCategory
	FIELD(UINT8          , m_SubClass);		//物品二级分类取值 enGoodsSecondType
	FIELD(UINT8          , m_ThreeClass);	//物品三级分类

	FIELD_END();
};

//装备法术
struct SS_EquipMagic
{
	FIELD_BEGIN();
	FIELD(TMagicID		 , m_MagicID);		//装备法术ID

	FIELD_END();
};

//通过副本
struct SS_ThroughFuBen
{
	FIELD_BEGIN();
	FIELD(TFuBenID,		m_FuBenID);		//副本ID
	FIELD(UINT8,		m_FuBenType);	//副本类型
	FIELD(UINT8,		m_Level);		//级别

	FIELD_END();
};

//强化装备
struct SS_EquipStronger
{
	FIELD_BEGIN();
	FIELD(UINT8          , m_Category);		//物品一级分类取值 enGoodsCategory
	FIELD(INT32			 , m_LevelStronger);//强化到等级

	FIELD_END();
};

//镶嵌宝石
struct SS_EquipInlay
{
	FIELD_BEGIN();
	FIELD(UINT8          , m_Category);		//物品一级分类取值 enGoodsCategory
	FIELD(UINT8			 , m_NumInlay);		//镶嵌第几颗

	FIELD_END();
};

//法宝附灵
struct SS_TalismanAddGhost
{
	FIELD_BEGIN();
	FIELD(UINT8          , m_Category);		//物品一级分类取值 enGoodsCategory

	FIELD_END();
};

//提升人物境界
struct SS_UpLayer
{
	FIELD_BEGIN();
	FIELD(UINT8			  , m_LayerLevel);	//境界层级

	FIELD_END();
};

//加入帮派
struct SS_AddSyn
{
	FIELD_BEGIN();
	FIELD(bool,			m_bCreate);		//是否创建帮派
	FIELD(TSynID,		m_SynID);		//帮派ID

	FIELD_END();
};

//帮派保卫
struct SS_SynCombat
{
	FIELD_BEGIN();
	FIELD(UINT8				,m_Mode);		//模式

	FIELD_END();
};

//商城购物
struct SS_BuyShopMall
{
	FIELD_BEGIN();
	FIELD(UINT8,			m_Label);		//标签
	FIELD(UINT8,			m_MoneyType);	//所用货币类型

	FIELD_END();
};

//练功
struct SS_Training
{
	FIELD_BEGIN();
	FIELD(bool			,m_bAutoStop);	//是不是自动停止

	FIELD_END();
};

//修炼完成
struct SS_XiuLianFinish
{
	FIELD_BEGIN();
	FIELD(UINT8				,m_XiuLianType);	////修炼类型
	FIELD(bool				,m_bYiXing);		//是否异性

	FIELD_END();
};

//斗法
struct SS_DouFa
{
	FIELD_BEGIN();
	FIELD(bool,		m_bWin);

	FIELD_END();
};

//法宝孕育
struct SS_TalismanSpawn
{
	FIELD_BEGIN();
	FIELD(TGoodsID,		m_GoodsID);

	FIELD_END();
};

//升级法术
struct SS_MagicUpLevel
{
	FIELD_BEGIN();
	FIELD(TMagicID,		m_MagicID);	//法术ID

	FIELD_END();
};

//学习帮派法术
struct SS_LearnSynMagic
{
	FIELD_BEGIN();
	FIELD(TMagicID,		m_MagicID);	//法术ID
	FIELD(UINT8,		m_Level);	//法术等级

	FIELD_END();
};

//剑气
struct SS_SwordKee
{
	FIELD_BEGIN();
	FIELD(INT32			,m_SwordLevel);		//剑气等级

	FIELD_END();
};

//加好友
struct SS_AddFriend
{
	FIELD_BEGIN();
	FIELD(bool,		m_bYiXing);		//是否异性
	FIELD_END();
};

//杀死怪物
struct SS_KillMonster
{
	FIELD_BEGIN();
	FIELD(UINT64		,m_MonsterUID);

	FIELD_END();
};

//获得物品
struct SS_GetGoods
{
	FIELD_BEGIN();
	FIELD(TGoodsID,		m_GoodsID);

	FIELD_END();
};

//背包是否有此数量的物品
struct  SS_GoodsNum
{
	FIELD_BEGIN();
	FIELD(TGoodsID,			m_GoodsID);	//是否有足够的任务物品

	FIELD_END();
};

//帮派商铺购买物品
struct SS_SynShopBuyGoods
{
	FIELD_BEGIN();
	FIELD(TGoodsID,			m_GoodsID);

	FIELD_END();
};

//使用帮贡
struct SS_UseSynContribution
{
	FIELD_BEGIN();
	FIELD(UINT32,			m_UseNum);		//使用的数量

	FIELD_END();
};

//进入伏魔洞
struct SS_EnterFuMoDong
{
	FIELD_BEGIN();
	FIELD(bool,		bOK);

	FIELD_END();
};

//刷新聚仙楼
struct SS_FlushGodHouse
{
	FIELD_BEGIN();
	FIELD(bool,		bMoney);	//是否需要钱
	FIELD(bool,		bEmployee);	//是否刷新招募角色

	FIELD_END();
};

//游玩法宝世界
struct SS_TalismanWorld
{
	FIELD_BEGIN();
	FIELD(TTalismanWorldID,	m_TalismanWorldID);	//法宝世界ID

	FIELD_END();
};

//完成任务
struct SS_FinishTask
{
	FIELD_BEGIN();
	FIELD(UINT8,		m_TaskClass);	//任务类型

	FIELD_END();
};

//学会法术
struct SS_StudyMagic
{
	FIELD_BEGIN();
	FIELD(TMagicID,		m_MagicID);
	FIELD(UINT8,		m_MagicLevel);	//法术的学习等级

	FIELD_END();
};

//成功出售一件物品
struct SS_SellSuccess
{
	FIELD_BEGIN();
	FIELD(TGoodsID,		m_GoodsID);

	FIELD_END();
};

//好友度改变
struct SS_ChangeFriendRelation
{
	FIELD_BEGIN();
	FIELD(UINT32,		m_FriendRelation);	//现有好友度

	FIELD_END();
};

//背包扩充
struct SS_PacketExtend
{
	FIELD_BEGIN();
	FIELD(bool,		m_bCanExtend);		//还能继续扩充不

	FIELD_END();
};

//仙剑的提升剑诀等级
struct SS_UpSecretLevel
{
	FIELD_BEGIN();
	FIELD(UINT8,		m_Level);		//剑诀等级

	FIELD_END();
};

//进场景类型
enum enSceneType VC_PACKED_ONE
{
	enSceneType_CommonFuBen = 0,	//普通副本
	enSceneType_GodSwordFuBen,		//仙剑副本
	enSceneType_SynFuBen,			//帮派副本

	enSceneType_Max,

} PACKED_ONE;

//进场景
struct SS_EnterScene
{
	UID	m_uidUser;
	UINT32	m_NewSceneID;
	UINT32	m_OldSceneID;
};

//退出帮派
struct SS_QuitSyn
{
	FIELD_BEGIN();
	FIELD(UINT64, uidUser);

	FIELD_END();
};

//充值
struct SS_PayMoney
{
	FIELD_BEGIN();
	FIELD(UINT32,	m_nMoney);		//充值多少元

	FIELD_END();
};

//解雇
struct SS_UnloadEmployee
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_uidActor);

	FIELD_END();
};

//vip升级
struct SS_UpVipLevel
{
	FIELD_BEGIN();
	FIELD(UINT64,	m_uidUser);
	FIELD(UINT8,	m_bChange);
	FIELD(UINT8,	m_vipLevel);

	FIELD_END();
};

//玩家在干什么
enum enActorDoing VC_PACKED_ONE
{
	enActorDoing_None = 0,		//闲着
	enActorDoing_Combat,		//战斗
	enActorDoing_Training,		//练功
	enActorDoing_OnHook,		//挂机
	enActorDoing_FuBen_Single,	//单人副本
	enActorDoing_FuBen_Team,	//组队副本
	enActorDoing_SynCombat,		//帮派保卫战
	enActorDoing_GodSword,		//剑印世界
	enActorDoing_Talisman,		//法宝世界
	enActorDoing_DuoBao,		//夺宝
	enActorDoing_SynWar,		//帮战
	enActorDoing_WaitTeam,		//等待组队

	enActorDoing_Max,
}PACKED_ONE;

//发出事件，得到玩家在干什么
struct SS_Get_ActorDoing
{
	SS_Get_ActorDoing()
	{
		m_ActorDoing = (UINT8)enActorDoing_None;
	}

	FIELD_BEGIN();
	FIELD(UINT64,		m_uidActor);
	FIELD(UINT8,		m_ActorDoing);
	
	FIELD_END();
};

//剑诀提升
struct SS_UpDamageLv
{
	FIELD_BEGIN();
	FIELD(UINT8			 , m_DamageLevel);//提升到等级

	FIELD_END();
};

//声望技能提升
struct SS_UpCreditMagicLv
{
	FIELD_BEGIN();
	FIELD(UINT8			 , m_CreditMagicLv);//提升到等级

	FIELD_END();
};

//enMsgID_ActorLogout
struct SS_ActorLogout
{
	UID    m_uidActor;
};



//移除状态
//enMsgID_RemoveStatus
struct SS_RemoveStatus
{
	UID         m_uidActor;
	TStatusID   m_StatusID;
	UINT8       m_StatusType;
};


//增加状态
// enMsgID_AddStatus
struct SS_AddStatus
{
	UID         m_uidActor;
	TStatusID   m_StatusID;
	UINT8       m_StatusType;
	TMagicID    m_MagicID;
	INT32       m_StatusTime;
};

//战斗中死亡
//enMsgID_DieAtCombat
struct SS_DieAtCombat
{
	UID        m_uidActor;
};

//定时状态启动
// enMsgID_TimerStatus    
struct SS_TimerStatus
{
	UID         m_uidActor;
	UID         m_uidCreator;
	TStatusID   m_StatusID;
};

//离队
struct SS_QuitTeam
{
	bool		m_bMeQuit;			//是否是我离队(true:我离队 false:队友离队)
	UID			m_uidTeamMember;	//队友UID
	UID			m_uidTeamLeader;	//队长UID
};

//人物的提升剑诀等级
struct SS_UserUpSecretLevel
{
	FIELD_BEGIN();

	FIELD(UINT8,	Level);	//人物剑诀等级

	FIELD_END();
};

//法宝世界战斗
struct SS_TalismanWorldCombat
{
	FIELD_BEGIN();

	FIELD(UINT8,	bWin);

	FIELD_END();
};

//灵件升级
struct SS_UpGhost
{
	FIELD_BEGIN();

	FIELD(UINT8,	Level);

	FIELD_END();
};

//夺宝战斗
struct SS_DuoBaoCombat
{
	FIELD_BEGIN();

	FIELD(UINT8,	bWin);
	FIELD(UINT8,	bIsTeam);

	FIELD_END();
};

enum enCombatPlace VC_PACKED_ONE
{
	enCombatPlace_DuoBao = 0,	//夺宝
	enCombatPlace_XuanTian,		//玄天
	enCombatPlace_TalismanWorld,//法宝世界
	enCombatPlace_FuBen,		//副本
	enCombatPlace_SynWar,		//帮战
}PACKED_ONE;

//战斗
struct SS_OnCombat
{
	FIELD_BEGIN();

	FIELD(UINT8,	CombatPlace);
	FIELD(UINT8,	bWin);
	FIELD(UINT8,	bIsTeam);
	FIELD(UINT8,	bHard);		//是否困难怪物

	FIELD_END();
};

//获得帮战功勋
struct SS_GetSynWarScore
{
	FIELD_BEGIN();

	FIELD(INT32,	GetNum);

	FIELD_END();
};

//解散帮派
struct SS_JieSanSyn
{
	FIELD_BEGIN();

	FIELD(TSynID,		m_SynID);		//帮派ID

	FIELD_END();
};


//玩家登陆
struct SS_ActorLogin
{
	FIELD_BEGIN();

	FIELD(UINT64,	m_uidActor);		//玩家uid

	FIELD_END();
};


#endif
