#ifndef __XJCQ_GAMESRV_GAMEWORLD_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_GAMEWORLD_CMD_PROTOCOL_H__

#pragma pack(push,1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//游戏世界消息命令字
enum enGameWorldCmd VC_PACKED_ONE
{
	enGameWorldCmd_SC_Private_Context  = 0,  //使用私有属性创建Thing
	enGameWorldCmd_SC_Public_Context  ,  //使用公有属性创建Thing
	enGameWorldCmd_SC_Enter_Scene    ,   //进入场景
	enGameWorldCmd_SC_Enter_SceneFinish  ,   //进入场景完成
	enGameWorldCmd_SC_DestroyThing,          //销毁thing
	enGameWorldCmd_SC_UpdateThing,           //Thing属性改变
	enGameWorldCmd_EnterPortal,           //进入传送门

	enGameWorldCmd_SC_GoodsCnfg,          //物品配置信息
	enGameWorldCmd_SC_EquipCnfg,          //装备配置信息
	enGameWorldCmd_SC_EmployeeCnfg,		  //招募角色配置信息
	enGameWorldCmd_SC_MagicLevelCnfg,	  //法术所有等级信息

	enGameWorldCmd_SC_AddStatus,		  //通知客户端玩家增加一个状态
	enGameWorldCmd_SC_SycStatus,		  //同步一个状态
	enGameWorldCmd_SC_UpdateThingFloat,	  //Thing属性改变(有带小数的)

	enGameWorldCmd_SC_UpdateName,		  //名字更改

	enGameWorldCmd_UnLoadEmployee,		  //解雇招募角色

	enGameWorldCmd_SC_SynMagicCnfg,		  //帮派技能信息

	enGameWorldCmd_SC_TipBox,			  //弹出框提示

	enGameWorldCmd_Max,

} PACKED_ONE;


//游戏世界，消息头
struct GameWorldHeader : public AppPacketHeader
{
	GameWorldHeader(enGameWorldCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_GameWorld;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//进入游戏结果码定义
enum enGameWorldRetCode VC_PACKED_ONE
{
	enGameWorldRetCode_Ok = 0,
	enGameWorldRetCode_ErrSceneID , //无效的目标场景ID
	enGameWorldRetCode_ErrNoScene , //目标场景不存在
	enGameWorldRetCode_Teleport,    //传送失败
	enGameWorldRetCode_PacketFew,	//背包容量已低于5，请清理
	enGameWorldRetCode_ErrNoPortal,	//传送门不存在

	enGameWorldRetCode_Max,

} PACKED_ONE;


//进入传送门
struct CS_EnterPortal_Req
{
	UID         m_uidPortal; //传送门
};

struct SC_EnterPortal_Rsp
{
	enGameWorldRetCode m_Result;
};


//创建Thing
struct SC_CreateThing
{
	SC_CreateThing()
	{
        ThingNum = 1;
	}
	UINT8 enThingClass;  //thing类型
	UINT8 ThingNum;      //数量
};

//能力类型
enum enNenLiType VC_PACKED_ONE
{
	enNenLiType_PuTong = 0,		//普通
	enNenLiType_LiangHao,		//良好
	enNenLiType_YouZhi,			//优质
	enNenLiType_QiCai,			//奇材

	enNenLiType_Max,
}PACKED_ONE;

//角色公有数据
struct SActorPublicData
{
	SActorPublicData()
	{
		MEM_ZERO(this);
	}
	char    m_szName[THING_NAME_LEN];
	UID     m_uid;
	UINT8   m_Level;            //等级
	int     m_Spirit;           //基本灵力
	int     m_Shield;           //基本护盾
	int     m_BloodUp;          //基本气血上限
	int     m_Avoid;			//基本身法
	int     m_ActorExp;			//经验
	int     m_ActorLayer;		//层次，境界
	int     m_ActorNimbus;		//灵气
	int     m_ActorAptitude;	//资质  浮点数
	UINT8   m_ActorSex;			//性别 0:女,1:男
	int     m_ActorUserID;		//UserID
	UINT16  m_ActorFacade;		//外观,取值由客户端定义
	int     m_ActorPhysics;		//物理伤害
	int     m_ActorMagic;		//法术伤害
	int     m_ActorDefend;		//防御
	int     m_ActorNimbusSpeed; //灵气速率
	char    m_nDir;				//方向
	UINT16  m_ptX;				//X坐标
	UINT16  m_ptY;				//Y坐标
	UID     m_uidMaster;		//主人，如为INVALID_UID 表示本身是主角
	UID     m_uidEmploy[MAX_EMPLOY_NUM];  //招募
	TSceneID  m_SceneID;        //所在场景
	int     m_SpiritEquip;      //装备增加灵力
	int     m_ShieldEquip;      //装备增加护盾
	int     m_BloodUpEquip;     //装备增加气血上限
	int     m_AvoidEquip;		//装备增加身法
	enNenLiType m_NenLi;		//能力
};

//角色私有数据
struct SActorPrivateData : public SActorPublicData
{
	int      m_ActorMoney;      //游戏币
	int      m_ActorTicket;     //礼券
	int      m_ActorStone;      //灵石
	UINT16   m_ActorCityID;		//城市编号
	INT32	 m_ActorHonor;		//荣誉
	INT32	 m_ActorCredit;		//声望
	INT32	 m_GodSwordNimbus;	//仙剑灵气
	UINT8	 m_VipLevel;		//vip等级
	INT32	 m_ActorPolyNimbus;	//聚灵气
	UINT8    m_CritLv;          //爆击等级
	UINT8    m_TenacityLv;      //坚韧等级
	UINT8    m_HitLv;           //命中等级	40
	UINT8    m_DodgeLv;			//回避等级
	int      m_CombatAbility;	//战斗力
	UINT8	 m_GoldDamageLv;	//金剑诀伤害等级
	UINT8	 m_WoodDamageLv;	//木剑诀伤害等级
	UINT8	 m_WaterDamageLv;	//水剑诀伤害等级
	UINT8	 m_FireDamageLv;	//火剑诀伤害等级
	UINT8	 m_SoilDamageLv;	//土剑诀伤害等级
	UINT8	 m_MagicCDLv;		//法术回复等级
	INT32	 m_GhostSoul;		//灵魄

	INT32    m_Crit;			//爆击	50
	INT32    m_Tenacity;		//坚韧
	INT32    m_Hit;				//命中
	INT32    m_Dodge;			//回避
	INT32	 m_MagicCD;			//法术回复
	INT32	 m_GoldDamage;		//金剑诀伤害
	INT32	 m_WoodDamage;		//木剑诀伤害
	INT32	 m_WaterDamage;		//水剑诀伤害
	INT32	 m_FireDamage;		//火剑诀伤害
	INT32	 m_SoilDamage;		//土剑诀伤害

	INT32	 m_DuoBaoLevel;		//夺宝等级

	UINT32	 m_Recharge;		//历史充值总额
};

//怪物公有数据
struct SMonsterPublicData
{
	char    m_szName[THING_NAME_LEN];
	UID     m_uid;
	UINT8   m_Level;            //等级
	TMonsterID m_MonsterID;		//怪物ID
	char    m_nDir;				//方向
	UINT16  m_ptX;				//X坐标
	UINT16  m_ptY;				//Y坐标
	TSceneID  m_SceneID;        //所在场景	
};

//怪物私有数据
struct SMonsterPrivateData : public SMonsterPublicData
{
	SMonsterPrivateData()
	{
		MEM_ZERO(this);
	}

    int     m_Spirit;           //灵力
	int     m_Shield;           //护盾
	int     m_Blood;			//气血
	int     m_Avoid;			//身法
	int     m_MonsterSwordkee;	//剑气
	int     m_MonsterMagic;		//法术
	int     m_MonsterLayer;      //境界
	UINT8   m_Lineup;           //阵型
	UINT16  m_Facade;		   //外观
	TMagicID m_Magics[MAX_MONSTER_MAGIC_NUM];  //怪物法术，
	INT32    m_Crit;          //爆击
	INT32    m_Tenacity;      //坚韧
	INT32    m_Hit;           //命中
	INT32    m_Dodge;		  //回避
	INT32	 m_MagicCD;		  //法术回复
	UINT8	 m_bCanReplace;	  //是否可被占领

};


//建筑公有数据
struct SBuildingPublicData
{
	UID                         m_Uid;                //建筑标识
	UINT8                       m_BuildingType;       //建筑类型
	TSceneID                    m_SceneID;            //所在场景
	INT32                       m_ResOutputRemainingTime;  //资源产出余下时间
};

//建筑私有数据
struct SBuildingPrivateData : public SBuildingPublicData
{
	SBuildingPrivateData()
	{
		MEM_ZERO(this);
	}
	UID                         m_uidOwner;						//建筑所属的主人
	char                        m_szOwnerName[THING_NAME_LEN];  //建筑所属主人的姓名
	INT32						m_CollectResNum;				//代收的资源数
};

//传送门公有数据
struct SPortalPublicData
{
	UID                m_uidPortal;	
	TPortalID          m_PortalID;        //ID
	TSceneID           m_SceneID;            //所在场景
	UINT16             m_ptX;           //X坐标
	UINT16             m_ptY;           //Y坐标
};

//传送门私有数据
struct SPortalPrivateData : public SPortalPublicData
{

};

//装备属性ID
enum enEquipProp VC_PACKED_ONE
{
	enEquipProp_Spirit = 0,   //灵力
	enEquipProp_Shield,       //护盾
	enEquipProp_BloodUp ,      //气血
	enEquipProp_Avoid,      //身法
	enEquipProp_Max,

}PACKED_ONE;

//灵件属性ID
enum enGhostProp VC_PACKED_ONE
{
	enGhostProp_Spirit = 0,   //灵力
	enGhostProp_Shield,       //护盾
	enGhostProp_Avoid,        //身法
	enGhostProp_BloodUp ,     //气血
	
	enGhostProp_Max,

}PACKED_ONE;

//物品配置信息
//物品配置
struct SC_GoodsCnfg
{
	SC_GoodsCnfg()
	{
		MEM_ZERO(this);
	}

	
	TGoodsID                      m_GoodsID;				  //GoodsID
	char                          m_szName[THING_NAME_LEN];   //名称 
	UINT8                         m_GoodsClass;				  //一级分类
	UINT8                         m_SubClass;				  //二级分类
	UINT8                         m_ThirdClass;				  //三级分类
	UINT8                         m_UsedLevel;				  //使用等级	
	UINT8                         m_GoodsLevel;				  //物品等级
	UINT8                         m_PileNum;				  //叠加数量上限
	UINT8                         m_Quality;				  //品质
	UINT8                         m_BindType;				  //绑定类型
	INT32                         m_SellPrice;				  //价格
	UINT8						  m_bManyUse;				  //是否可以多个使用
	UINT8						  m_bSelectUser;			  //物品使用是否要选择角色　0：不要，1:要
	INT16						  m_ResID;					  //资源ID
	
	enEquipProp                   m_GemPropID;				  //宝石属性ID  仅对宝石属性有效
	INT16                         m_GemValue;				  //宝石属性值
	UINT8						  m_bIsGod;						//是否仙级(0:不是，1:是)

	//char                       szDesc[];					  //物品说明，以0终止
};


//装备配置
struct SC_EquipCnfg : public SC_GoodsCnfg
{
	SC_EquipCnfg()
	{
		MEM_ZERO(this);
	}

	INT32                        m_SpiritOrMagic;    //对于装备：灵力 ，对仙剑:法术,对法宝:法术ID
	INT32                        m_ShieldOrWuXing;   //对于装备：护盾, 对仙剑:五行	对普通物品:是否选择角色使用(0:不需要，1:需要)
	INT32                        m_BloodOrSwordkee;    //对于装备：气血 对仙剑:剑气
	INT32                        m_AvoidOrSwordLvMax;    //对于装备：身法 对仙剑:剑气级别上限
	INT16                        m_SuitIDOrSwordSecretID;   ///对于装备：套装ID ,对仙剑: 法术ID, ,对法宝: 法宝世界ID

	enEquipProp                m_SuitPropID1;             //对装备:套装属性ID1,仅当m_SuitIDOrSwordSecretID不为零时，该字段有效。
	INT32                        m_SuitPropValue1;          //套装属性1
	enEquipProp                m_SuitPropID2;             //对装备:套装属性ID2
	INT32                        m_SuitPropValue2;          //套装属性2

	//char                       szDesc[];     //物品说明，以0终止
};

//招募角色配置
struct SC_EmployeeCnfg
{
	TActorID					m_EmployeeID;				//招募角色ID
	char						m_szName[THING_NAME_LEN];	//名字
	UINT8						m_Level;					//等级
	enNenLiType					m_NenLiType;				//能力
	UINT32						m_Aptitude;					//资质	浮点数
	UINT32					    m_BloodUp;					//气血上限
	UINT32					    m_Spirit;					//灵力
	UINT32					    m_Shield;					//护盾
	UINT32						m_Avoid;					//身法
	INT16						m_ResID;					//资源ID
	INT32						m_Price;					//价格

};

//法术等级配置信息
struct SC_MagicLevelCnfg
{
	TMagicID					m_MagicID;					//法术ID
	char						m_szName[THING_NAME_LEN];	//法术名字
	UINT8						m_WuXing;					//五行
	UINT8						m_MaxLevel;					//最大等级
	UINT16						m_IconID;					//图标资源ID
	UINT16						m_DongHuaID;				//动画资源ID
	INT16                       m_CDTime;                   //CD时间，单位:秒
//	MagicLevelInfo				m_LevelInfo[m_MaxLevel];			
};

struct MagicLevelInfo
{
	UINT8						m_Level;						//法术等级
	UINT32						m_NeedLayer;					//需要的境界
	UINT32						m_NeedNimbus;					//需要的灵气
	char						m_Descript[DESCRIPT_LEN_150];	//法术描述
};

//帮派技能配置信息
struct SC_SynMagicCnfg
{
	TSynMagicID					m_SynMagicID;
	char						m_szName[THING_NAME_LEN];	//帮派技能名字
	UINT8						m_MaxLevel;					//最大技能
	UINT16						m_ResID;					//资源ID
//  SynMagicLevelInfo			MagicLevelInfo;
};

struct SynMagicLevelInfo
{
	UINT8						m_Level;					//帮派技能等级
	INT32						m_NeedSynContribution;		//需要帮派贡献
	INT32						m_NeedStone;				//需要灵石
	INT8						m_NeedSynLevel;				//需要帮派等级
	char						m_Descript[DESCRIPT_LEN_75];//帮派技能描述
};


// 装备特有的属性
struct SEquipProp
{
	UINT8       m_Star;      //强化星级,增加基础属性的比例看 GameConfig.ini 文件m_vectEquipStrongPropRate
	TGoodsID    m_GemGoodsID[MAX_INLAY_NUM];  //镶嵌的宝石
	
	bool        m_bActiveSuit1;   //套装属性1是否激活
	bool        m_bActiveSuit2;   //套装属性2是否激活
	
	UINT8       m_Reserve[5];       //保留
};

//仙剑特有的数据
struct SGodSwordProp
{
	UINT32 m_Nimbus;     //灵气
	UINT8  m_SwordLevel; //剑气级别
	UINT8  m_SecretLevel;    //剑诀等级

	INT16  m_MagicValue;  //法术伤害
	INT16  m_SwordkeeValue; //剑气值

	UINT8  m_KillNpcNum;	//杀怪次数
	UINT8  m_FuBenFloor;	//剑印世界第几层(0:没进度,1:外层,2:中层,3:内层)
		
	UINT8  m_Reserve[2];       //保留
};

//法宝有的，需要存盘的数据
struct STalismanProp
{
	UINT16    m_QualityPoint;   //品质点
	TGoodsID  m_GestateGoodsID; //孕育的物品ID
	UINT32    m_BeginGestateTime;    //开始孕育时间
	TGoodsID  m_GhostGoodsID; //法宝上的灵件的物品ID
	UINT8	  m_GhostLevel;	//灵件的等级
	UINT8     m_ExtendData[1]; //扩展数据 
	UINT8     m_EnterNum;  //当天进入法宝世界的次数
	UINT8     m_MagicLevel; //法宝法术的等级

};


//灵件特有的属性
struct SGhostProp
{
	enum enPropID VC_PACKED_ONE
   {
	  enPropID_GhostLevel = 6,   //灵件等级
	
	   enGhostProp_Max,

    }PACKED_ONE;

	UINT8	m_GhostLevel;	//灵件等级
};



//普通物品有的属性
struct SCommonGoodsProp
{
//	UINT8		enGoodsThreeType;
	union
	{
		SGhostProp		m_Ghost;		  //灵件有的属性
		UINT8       m_ExtendData[13]; //扩展数据 
	};
};



//物品公有数据
struct SGoodsPublicData
{
	UID         m_uidGoods;  //UID标识物品
	TGoodsID    m_GoodsID;   //GoodsID
	UINT32      m_CreateTime; //创建时间
	UINT8       m_Number;     //数量
	bool        m_Binded;    //是否已绑定
	UINT8       m_GoodsClass; //物品类型
	union
	{
		SEquipProp     m_EquipProp;
		SGodSwordProp  m_GodSwordProp;
		STalismanProp  m_TalismanProp;
		SCommonGoodsProp   m_CommonProp;   //普通物品属性
	};
};

//物品私有数据
struct SGoodsPrivateData : public SGoodsPublicData
{

};

//进入场景
struct SC_EnterScene
{
	TMapID    m_MapID;              //对应的地图
	TSceneID  m_SceneID;            //场景
};


//销毁Thing
struct SC_DestroyThing
{
	UID      m_uidThing;
};


//Thing属性改变
struct SC_UpdateThing
{
	UID      m_uidThing;
	UINT16   m_PropID;   //属性ID
	INT32    m_PropValue; //属性值
};

//Thing属性改变(有小数的)
struct SC_UpdateThingFloat
{
	UID      m_uidThing;
	UINT16   m_PropID;   //属性ID
	float    m_PropValue; //属性值
};

//发送出战人员数量
struct SC_CombatNum
{
	UINT8	m_CombatNum;
};

//通知客户端玩家增加一个状态
struct SC_AddStatus
{
	UINT32		m_StatusID;		//状态ID
};

//状态的记录类型
enum enStatusRocordType VC_PACKED_ONE
{
	enStatusRocordType_Interval = 1,		//定时间隔
	enStatusRocordType_Time,		//持续时间
	enStatusRocordType_Count,		//击攻次数

	enStatusRocordType_Max,
}PACKED_ONE;

//同步一个状态
struct SC_SycStatus
{
	UINT32				m_StatusID;			//状态ID
	UINT32				m_EndTime;			//状态结束时间
	UINT8				m_LeftRound;		//状态剩余回合数
	enStatusRocordType  m_RocordType;		//状态的记录类型
};

//更改名字，通知客户端
struct SC_UpdateName
{
	UID			m_UserUid;					//更改了名字的玩家UID
	char		m_szName[THING_NAME_LEN];	//更改的名字
};

//解雇招募角色
struct CS_UnLoadEmployee_Req
{
	UID			m_EmployeeUid;
};

enum enUnLoadEmployeeRetCode VC_PACKED_ONE
{
	enUnLoadEmployeeRetCode_OK = 0,
	enUnLoadEmployeeRetCode_NoEmployee,	//没有此角色
	enUnLoadEmployeeRetCode_ErrUser,	//主角不能解雇
} PACKED_ONE;

struct SC_UnLoadEmployee_Rsp
{
	enUnLoadEmployeeRetCode UnLoadEmployeeRetCode;
};

//弹出框提示
struct SC_Tip_PopUpBox
{
	enGameWorldRetCode GameWorldRetCode;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)


#endif
