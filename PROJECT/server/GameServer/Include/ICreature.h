

#ifndef __THINGSERVER_ICREATURE_H__
#define __THINGSERVER_ICREATURE_H__

#include "IThing.h"
#include "IThingPart.h"

struct IActor;




// 生物属性枚举
enum enCrtProp VC_PACKED_ONE
{
	enCrtProp_Min = 0,

      enCrtProp_Level     = 1,     //等级
      enCrtProp_Spirit    = 2,   //总灵力
	  enCrtProp_Shield    = 3,   //总护盾
	  enCrtProp_Blood     = 4,   //气血
	  enCrtProp_Avoid     = 5,    //总身法，躲避
	  enCrtProp_SceneID   = 6,    //所在场景
	  enCrtProp_PointX    = 7,    //X坐标
	  enCrtProp_PointY    = 8,   //Y坐标
	  enCrtProp_Dir       = 9,   //方向
	  enCrtProp_ActorCrit		= 10,  //爆击
	  enCrtProp_ActorTenacity	= 11,  //坚韧
	  enCrtProp_ActorHit		= 12,  //命中
	  enCrtProp_ActorDodge		= 13,  //回避
	  enCrtProp_MagicCD			= 14,  //法术回复
	  
	  enCrtProp_End ,  //生物属性结束

	  //怪物属性
      enCrtProp_Monster  = 50,

	  enCrtProp_MonsterSwordkee           = 52, //剑气
	  enCrtProp_MonsterMagic              = 53, //法术
	  enCrtProp_MonsterLayer              = 54, //境界
	  enCrtProp_MonsterID                 = 55, //monsterID
	  enCrtProp_MonsterLineup             = 56,  //怪物的阵型编号(0~5)
	  enCrtProp_MonsterCombatIndex        = 57, //战斗索引(普通模式)
	  enCrtProp_MonsterMagic1             = 58, //法术ID1
	  enCrtProp_MonsterMagic2             = 59, //法术ID2
	  enCrtProp_MonsterMagic3             = 60, //法术ID3
	  enCrtProp_MonsterMagic4             = 61, //法术ID4
	  enCrtProp_MonsterMagic5             = 62, //法术ID5
	  enCrtProp_MonsterDropID             = 63, //普通掉落ID
	  enCrtProp_MonsterDropIDHard         = 64, //困难掉落ID
	  enCrtProp_MonsterCombatIndexHard    = 65, //战斗索引(困难模式)
	  enCrtProp_MonsterDropIDTeam		  = 66,	//组队掉落ID
	  enCrtProp_MonsterCombatIndexTeam	  = 67,	//组队战斗索引(困难模式)
	  enCrtProp_MonsterCanReplace		  = 68, //是否可被替换

	 

	  enCrtProp_Monster_End,


   enCrtProp_Actor        = 100,   //角色专有属性

   enCrtProp_ActorExp     = 101, //经验
   enCrtProp_ActorLayer   = 102, //层次,境界
   enCrtProp_ActorNimbus  = 103, //灵气
   enCrtProp_ActorAptitude =104, //资质(单位:千分之几)

   enCrtProp_ActorSex     = 105,  //性别
   enCrtProp_ActorUserID  = 106,  //UserID  

   enCrtProp_ActorMoney   = 107, //仙石，可交易的币
   enCrtProp_ActorTicket  = 108, //礼券
   enCrtProp_ActorStone   = 109, //灵石,邦定的货币
   enCrtProp_ActorFacade  = 110, //当前外观
   enCrtProp_ActorBloodUp = 111, //气血上限
   enCrtProp_ActorPhysics = 112, //物理伤害
   enCrtProp_ActorMagic   = 113, //法术伤害
   enCrtProp_ActorDefend  = 114, //防御
   enCrtProp_ActorNimbusSpeed = 115,  //总灵气速率

   enCrtProp_ActorMainSceneID = 116,  //主场景ID
   enCrtProp_ActorFuMoDongSceneID = 117,  //伏魔洞
   enCrtProp_ActorHouShanSceneID  = 118,  //后山

    enCrtProp_ActorSpiritEqup    = 119,   //灵力
	enCrtProp_ActorShieldEqup    = 120,   //护盾
	enCrtProp_ActorBloodEqup     = 121,   //气血上限
	enCrtProp_ActorAvoidEqup     = 122,    //身法，躲避
	enCrtProp_ActorDefendMagic   = 123,    //法术增加的防御

	enCrtProp_ActorHonor		 = 124,		//荣誉

	enCrtProp_ActorCityID		 = 125,		//城市编号

	enCrtProp_ActorNimbusUp		= 126, //灵气上限

	enCrtProp_ActorLastSceneID	= 127, //上次场景

	enCrtProp_ActorCredit		= 128,	//声望

    enCrtProp_ActorSpiritBasic  = 129,  //基本灵力
	enCrtProp_ActorShieldBasic  = 130,  //基本护盾
	enCrtProp_ActorBloodUpBasic = 131,  //基本气血上限
	enCrtProp_ActorAvoidBasic   = 132,  //基本身法，躲避

	enCrtProp_ActorExpRate     = 133,  //经验速率(单位：千分之几)

	enCrtProp_ActorMultipExp   = 134,  //多倍经验

	enCrtProp_ActorNenLi	   = 135,  //能力

	enCrtProp_ActorGodSwordNimbus = 136,  //仙剑灵气

	enCrtProp_SynMagicPhysicsParam = 137,	//帮派技能增加的物理伤害系数(百分数)

	enCrtProp_SynMagicMagicParam = 138,		//帮派技能增加的法术伤害系数(百分数)

	enCrtProp_SynMagicDefendParam = 139,	//帮派技能增加的防御系数(百分数)

	enCrtProp_SynMagicAloneXLParam = 140,	//帮派技能增加独自修炼灵气速率

	enCrtProp_ActorLogin           = 141,  //上线时间点

	enCrtProp_ActorOnlineTimes     = 142,  //在线时长

	enCrtProp_ActorIdentity         = 143,  //身份认证状态

	enCrtProp_ForeverSpirit			= 144,	//永久增加的灵力
	enCrtProp_ForeverShield			= 145,	//永久增加的护盾
	enCrtProp_ForeverBloodUp		= 146,	//永久增加的气血上限
	enCrtProp_ForeverAvoid			= 147,	//永久增加的身法

	enCrtProp_ForeverActorNimbusSpeed = 148,	//永久增加的灵气速率

	enCrtProp_VipLevel				= 149,	//VIP等级
	
	enCrtProp_Recharge				= 150,	//历史充值数量

	enCrtProp_ForeverAptitude		= 151,	//永久增加的资质

	enCrtProp_ActorPolyNimbus		= 152,  //聚灵气	

	enCrtProp_ActorCombatAbility	= 153,  //战斗力

	enCrtProp_GoldDamageLv			= 154,  //金剑诀伤害等级
	enCrtProp_WoodDamageLv			= 155,  //木剑诀伤害等级
	enCrtProp_WaterDamageLv			= 156,  //水剑诀伤害等级
	enCrtProp_FireDamageLv			= 157,  //火剑诀伤害等级
	enCrtProp_SoilDamageLv			= 158,  //土剑诀伤害等级

	enCrtProp_SynCombatLevel		= 159,	//帮战等级

	enCrtProp_GhostSoul				= 160,	//灵魄

	enCrtProp_DuoBaoLevel			= 161,	//夺宝等级


	enCrtProp_ActorCritLv		    = 162,  //爆击技能等级
	enCrtProp_ActorTenacityLv		= 163,  //坚韧技能等级
	enCrtProp_ActorHitLv			= 164,  //命中技能等级
	enCrtProp_ActorDodgeLv			= 165,  //回避技能等级
	enCrtProp_MagicCDLv				= 166,  //法术回复技能等级

	enCrtProp_GoldDamage			= 167,  //金剑诀伤害
	enCrtProp_WoodDamage			= 168,  //木剑诀伤害
	enCrtProp_WaterDamage			= 169,  //水剑诀伤害
	enCrtProp_FireDamage			= 170,  //火剑诀伤害
	enCrtProp_SoilDamage			= 171,  //土剑诀伤害

	enCrtProp_LastOnlineTime		= 172,	//上次登录最后在线时间

	enCrtProp_TotalVipLevel			= 173,	//总VIP等级（不入库，给状态增加VIP等级使用，防止状态增加的VIP等级入库，然后服务器挂掉，导致状态增加的VIP等级没有扣除）

    enCrtProp_Actor_End ,

   enCrtProp_Max

} PACKED_ONE;

//玩家属性枚举ID映射玩家私有属性排序编号
static INT32 ActorPropMapID[enCrtProp_Max-enCrtProp_Actor - 1] = 
{
	7/*经验*/,
	8/*层次,境界*/,
	9/*灵气*/,
	10/*资质*/,
	11/*性别*/,
	12/*UserID*/,
	29/*仙石*/,
	30/*礼券*/,
	31/*灵石*/,
	13/*外观*/,
	-1/*总气血上限*/,
	14/*物理伤害*/,
	15/*法术伤害*/,
	16/*防御*/,
	17/*灵所速率*/,
	-1/*主场景ID*/,
	-1/*伏魔洞*/,
	-1/*后山*/,
	24/*灵力*/,
	25/*护盾*/,
	26/*装备气血上限*/,
	27/*身法*/,
	-1/*法术增加的防御*/,
	33/*荣誉*/,
	32/*城市编号*/,
	-1/*灵气上限*/,
	-1/*上次场景*/,
	34/*声望*/,
	3,//基本灵力
	4,//基本护盾
	5,//基本气血上限
	6,//基本身法，躲避
	-1, //经验速率
	-1, //多倍经验
	28,//能力
	35,//仙剑灵气
	-1,//帮派技能增加的物理伤害系数(百分数)
	-1,//帮派技能增加的法术伤害系数(百分数)
	-1,//帮派技能增加的防御系数(百分数)
	-1,//帮派技能增加独自修炼灵气速率
	-1,//上线时间点
	-1,//在线时长
	-1,//身份认证状态
	-1,//永久增加的灵力
	-1,//永久增加的护盾
	-1,//永久增加的气血上限
	-1,//永久增加的身法
	-1,//永久增加的灵气速率
	-1,//VIP等级
	-1,//历史充值数量
	-1,//永久增加的资质
	37,//聚灵气
	42,//战斗力
	43,//金剑诀伤害等级
	44,//木剑诀伤害等级
	45,//水剑诀伤害等级
	46,//火剑诀伤害等级
	47,//土剑诀伤害等级
	-1,//帮战等级
	49,//灵魄
	60,//夺宝等级
	38,//爆击等级
	39,//坚韧等级
	40,//命中等级
	41,//回避等级
	48,//法术回复等级
	-1,//金剑诀伤害
	-1,//木剑诀伤害
	-1,//水剑诀伤害
	-1,//火剑诀伤害
	-1,//土剑诀伤害
	-1,//上次登录最后在线时间
	36,//总VIP等级（不入库，给状态增加VIP等级使用，防止状态增加的VIP等级入库，然后服务器挂掉，导致状态增加的VIP等级没有扣除）
};

//生物基本属性枚举ID映射生物私有属性排序编号
static INT32 CreaturePropMapID[enCrtProp_End-enCrtProp_Min - 1] =
{
	2/*等级*/,
	-1/*总灵力*/,
	-1/*总护盾*/,
	-1/*当前气血*/,
	-1/*总身法*/,
	23/*所在场景*/,
	19/*X坐标*/,
	20/*Y坐标*/,
	18/*方向*/,
	50,//爆击
	51,//坚韧
	52,//命中
	53,//回避
	54,//法术回复
};

//怪物基本属性枚举ID映射怪物私有属性排序编号
static INT32 MonsterPropMapID[enCrtProp_Monster_End - enCrtProp_Monster - 1] = 
{
	12,13,14,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};


//玩家属性枚举ID映射是否需要存盘
static bool s_ActorPropMapSave[enCrtProp_Max-enCrtProp_Actor - 1] = 
{
	true/*经验*/,
	true/*层次,境界*/,
	true/*灵气*/,
	true/*资质*/,
	true/*性别*/,
	true/*UserID*/,
	true/*仙石*/,
	true/*礼券*/,
	true/*灵石*/,
	true/*外观*/,
	true/*总气血上限*/,
	false/*物理伤害*/,
	false/*法术伤害*/,
	false/*防御*/,
	true/*灵所速率*/,
	false/*主场景ID*/,
	false/*伏魔洞*/,
	false/*后山*/,
	true/*灵力*/,
	true/*护盾*/,
	true/*装备气血上限*/,
	true/*身法*/,
	false/*法术增加的防御*/,
	true/*荣誉*/,
	true/*城市编号*/,
	true/*灵气上限*/,
	false/*上次场景*/,
	true/*声望*/,
	true,//基本灵力
	true,//基本护盾
	true,//基本气血上限
	true,//基本身法，躲避
	true, //经验速率
	true, //多倍经验
	true,//能力
	true,//仙剑灵气
	false,//帮派技能增加的物理伤害系数(百分数)
	false,//帮派技能增加的法术伤害系数(百分数)
	false,//帮派技能增加的防御系数(百分数)
	false,//帮派技能增加独自修炼灵气速率
	false,//上线时间点
	false,//在线时长
	false,//身份认证状态
	true,//永久增加的灵力
	true,//永久增加的护盾
	true,//永久增加的气血上限
	true,//永久增加的身法
	true,//永久增加的灵气速率
	true,//VIP等级
	true,//历史充值数量
	true,//永久增加的资质
	true,//聚灵气
	true,//战斗力
	true,//金剑诀伤害等级
	true,//木剑诀伤害等级
	true,//水剑诀伤害等级
	true,//火剑诀伤害等级
	true,//土剑诀伤害等级
	true, //帮战等级
	true,//灵魄
	true,//夺宝等级
	true,//爆击等级
	true,//坚韧等级
	true,//命中等级
	true,//回避等级
	true,//法术回复等级
	true,//金剑诀伤害
	true,//木剑诀伤害
	true,//水剑诀伤害
	true,//火剑诀伤害
	true,//土剑诀伤害
	false,//上次登录最后在线时间
	false,//总VIP等级（不入库，给状态增加VIP等级使用，防止状态增加的VIP等级入库，然后服务器挂掉，导致状态增加的VIP等级没有扣除）
};


//生物基本属性枚举ID映射存盘标志
static bool s_CreaturePropMapSave[enCrtProp_End-enCrtProp_Min - 1] =
{
	true/*等级*/,
	true/*总灵力*/,
	true/*总护盾*/,
	true/*当前气血*/,
	true/*总身法*/,
	false/*所在场景*/,
	false/*X坐标*/,
	false/*Y坐标*/,
	false/*方向*/,
	true/*爆击*/,
	true/*坚韧*/,
	true/*命中*/,
	true/*回避*/,
	true/*法术回复*/,

};

//资源类型
enum enResourceType VC_PACKED_ONE
{
	enResourceType_GoldStone =0, //仙石
	enResourceType_NimbusStone,  //灵石
	enResourceType_Ticket,       //礼券
	enResourceType_PolyNimbus,	 //聚灵气

   enResourceType_Max,

} PACKED_ONE;

//资源类型映射人物属性
static enCrtProp s_ResourceTypeMapCrtProp [enResourceType_Max] =
{
	enCrtProp_ActorMoney,
	enCrtProp_ActorStone,
	enCrtProp_ActorTicket,
	enCrtProp_ActorPolyNimbus,	//聚灵气
};

//性别
enum enCrtSex VC_PACKED_ONE
{
   enCrtSex_Female = 0, //女
    enCrtSex_Male,      //男
   enCrtSex_Max

} PACKED_ONE;

//能力
enum enAbility VC_PACKED_ONE
{
	enAbility_Common = 0,  //普通
    enAbility_Fine     ,   //良好
	enAbility_High     ,   //优质
	enAbility_Wonder   ,   //奇才
    enAbility_Max,

} PACKED_ONE;

/*
//生物属性
struct SCreatureProp
{
	int   m_Level;            //等级
	int   m_Spirit;           //灵力
	int   m_Shield;           //护盾
	int   m_Blood;           //气血
	int   m_Avoid;           //身法
};


//玩家属性
struct SActorProp
{
	int  m_ActorExp;       //经验
	int  m_ActorLayer;     //层次，境界
	int  m_ActorNimbus;    //灵气
	int  m_ActorAptitude;  //资质
	int  m_ActorSex;       //性别
	int  m_ActorUserID;    //UserID
};*/

struct ICreature : public IThing
{

		//取得本实体的enPartID部件
	virtual IThingPart* GetPart(enThingPart enPartID) = 0;

	//向本实体添加部件pPart。如果部件已存在，则会添加失败
	virtual bool AddPart(IThingPart *pPart) = 0;

	//删除本实体的enPartID部件，但并不释放这个部件
	virtual bool RemovePart(enThingPart enPartID) = 0;


	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本生物的数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值，
	//////////////////////////////////////////////////////////////////////////
	virtual bool AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0) = 0;
	
	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本生物的属性，替换原来的值
	// 输  入：数字型属性enPropID，属性值nValue
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0) = 0;


	//取得本生物的属性
	virtual INT32 GetCrtProp(enCrtProp enPropID) = 0;

	//恢复气血
	virtual void RecoverBlood(IActor * pActor=0) = 0;

};

#endif
