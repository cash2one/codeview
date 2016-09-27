

#ifndef __XJCQ_GAMESRV_COMBAT_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_COMBAT_CMD_PROTOCOL_H__

#pragma pack(push,1)





/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//消息
enum enCombatCmd VC_PACKED_ONE
{
	enCombatCmd_Combat,  //战斗
	enCombatCmd_CombatNpcInFuBen, //副本中挑战怪物
	enCombatCmd_LeaveFuBen,  //离开副本	
	enCombatCmd_CombatPlayBack,//战斗回放

	enCombatCmd_Max,
}PACKED_ONE;

//战斗结果码
enum enCombatCode VC_PACKED_ONE
{
	enCombatCode_OK,			 //成功
	enCombatCode_ErrBusy,            //对方正忙，不能战斗
	enCombatCode_ErrNoFuBen,  //副本不存在
	enCombatCode_ErrNoNpc,    //该怪物已不存在
	enCombatCode_ErrOrder,    //打怪顺序错误

	//剑印世界结果码
	enCombatCode_ErrNoGoods,		//物品不存在
	enCombatCode_ErrNotGodSword,    //该物品不是仙剑
	enCombatCode_ErrNotExistNpc,	//NPC不存在
	enCombatCode_ErrSwordWorldCnfg, //找不到剑印世界配置信息
	enCombatCode_ErrFuBenCnfg,      //找不到副本配置信息

	enCombatCode_ErrFuBenLv,		//该等级副本不能开启困难模式

	enCombatCode_ErrThrough,		//必须先通过普通模式才能挑战困难模式

	enCombatCode_ErrDieCantMagic,  //该玩家已死亡不能施放法术

	enCombatCode_ErrAutoMode,  //自动模式不能手动施放法术

	enCombatCode_ErrNoMagic,  //没有法术可施放

	enCombatCode_ErrOrderMagic,  //错误的法术施放顺序

	enCombatCode_ErrCDTime,  //法术CD冷却时间未到

	enCombatCode_ForbidMagic, //当前被禁止施放法术

	enCombatCode_OnlyLeadCan,	//只有队长才能点击挑战怪物

	enCombatCode_ErrNoPurview, //没有权限操作该角色的法术

	enCombatCode_ErrMemberNoFree, //无法进入，队友不是空闲状态


	enCombatCode_ErrInDuoBao,	//无法进入，正在匹配夺宝战

	enCombatCode_ErrHaveTeam,	//组队中，无法进入

	enCombatCode_ErrWaitTeam,	//等待组队中，无法操作

	enCombatCode_ErrBackMain,	//请回府后进入单人副本

	

}PACKED_ONE;

//消息头
struct SCombatHeader : public AppPacketHeader
{
	SCombatHeader(enCombatCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Combat;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};



//战斗子命令定义
enum enCombatSubCmd VC_PACKED_ONE
{
	enCombatSubCmd_SC_Init = 0,  //初始化战斗信息，两方人员，场景名称等信息	
	enCombatSubCmd_SC_Action,       //战斗指令
	enCombatSubCmd_SC_Over,      //战斗结束，下发战斗结果给客户端
	enCombatSubCmd_CS_FireMagic, //玩家手动施放法术
	enCombatSubCmd_CS_ActionAck,  //指令确认


} PACKED_ONE;

//
struct SCombatCombat
{
   enCombatSubCmd  m_SubCmd;  //命令
   UINT64          m_CombatID; //战斗ID
   /*union{
            CombatCombatData            // enCombatSubCmd_SC_Init
	        SCombatAction              // enCombatSubCmd_SC_Action
			CombatCombatOver           // enCombatSubCmd_SC_Over
			CS_CombatFireMagic_Req     //enCombatSubCmd_CS_FireMagic
			SCombatActionAck           // enCombatSubCmd_CS_ActionAck
	 */
};

//指令确认
struct SCombatActionAck
{
	UINT32        m_AckNo;  //确认号
};


//////////////////////////////////////////////////////////////////////////////////////////////////////
//战斗回放
struct SCombatCombatPlayBack
{
	INT16   m_ActionNum;  //指令数量
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////

struct SMagicPosInfo
{
	TMagicID         m_MagicID;
	UINT8            m_Index;
};

//参战人员占位信息
struct SLineupInfo
{
	SLineupInfo()
	{
		MEM_ZERO(this);
	}
	UID        m_uidCreature;				//参战的生物
	UINT8      m_Pos;						//位置
	char       m_szName[THING_NAME_LEN];	//名称
	UINT8      m_Level;						//等级
	UINT16     m_Facade;					//外观
	INT32      m_BloodUp;					//血量上限    
	UINT8      m_MagicNum;					//法术数量
	INT32	   m_Exp;						//当前经验值
	INT32	   m_NeedExpUpLevel;			//需要多少经验升级
	UINT8	   m_NenLi;                     //能力
	INT16      m_DecreaseCDTime;            //减少CD时间
	bool       m_IsMaster;                  //是否是主角
	bool       m_IsTeamLeader;              //是否是队长
	//SMagicPosInfo  m_Magics[m_MagicNum]; //法术列表
};

//战斗类型定义
enum enCombatType VC_PACKED_ONE
{
      enCombatType_FuMoDong = 0, //伏魔洞
	  enCombatType_BaoDe ,       //宝地
	  enCombatType_GoldSword,    //仙剑
	  enCombatType_SynGuard,     //帮派保卫战
	  enCombatType_QieCuo,       //切磋
	  enCombatType_DouFa,        //斗法
	  enCombatType_Challenge,    //挑战
	  enCombatType_Talisman,     //法宝
	  enCombatType_XuanTian,     //玄天
	  enCombatType_SynChallenge, //帮派挑战
	  enCombatType_DuoBao,		 //夺宝

} PACKED_ONE;


//发起战斗数据
struct CombatCombatData
{
	CombatCombatData(){
		memset(this, 0, sizeof(*this));
	}

	enCombatType        m_CombatType;  //战斗类型
	char				m_SceneName[THING_MAP_NAME_LEN];	//战斗地点名字, 24字节
	bool                m_bEnemyMonster;	//敌方是否为怪物
	UINT8               m_SelfActorNum;		//本方参战人数
	UINT8               m_EnemyNum;			//对方人数		
	//SLineupInfo       m_SelfActor[m_SelfActorNum]; //本方人员
	//SLineupInfo       m_EnemyActor[m_EnemyNum]; //对方人员	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//战斗结束
struct CombatCombatOver
{
	CombatCombatOver()
	{
		MEM_ZERO(this);
	}

	bool                m_bWin;				//战斗结果，是否胜利
	INT32				m_GetExp;			//获得经验
	INT32				m_SynContribution;	//获得的帮派贡献值
	INT32				m_SynExp;			//获得的帮派经验
	INT32				m_SynWarLevel;		//增加的帮战等级
	INT32				m_Credit;			//增加的声望
	INT32				m_SynScore;			//增加的帮战积分
	INT32               m_PolyNimbus;       //获得的聚灵气
	bool				m_bTeam;			//
	UINT8				m_nGoodsNum;		//掉落物品数量
	//TGoodsID			m_goodsID[m_nGoodsNum];
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩家施放法术
struct CS_CombatFireMagic_Req
{
	UID          m_uidActor;     //施法者
	TMagicID     m_MagicID;      //法术ID
};

//施放法术应答
struct SC_CombatFireMagic_Rsp
{
	enCombatCode m_Result;       //施放结果
	UID          m_uidActor;     //施法者
	TMagicID     m_MagicID;      //法术ID
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//动作类型
enum enActionType  VC_PACKED_ONE
{
	enActionType_AddStatus       = 0, //中状态 
	enActionType_RemoveStatus,       //移除状态
	enActionType_Damage,             //受到伤害
	enActionType_Attack,             //发起攻击
	enActionType_AddBlood,           //加血	
	enActionTYpe_Max,
} PACKED_ONE;

//中状态信息
struct SActionStatusInfo
{
	SActionStatusInfo()
	{
		MEM_ZERO(this);
	}
	UID                 m_uidCreature;         //中状态的生物
	UINT8               m_StatusType;          //状态类型
	UINT8               m_szStatusName[THING_NAME_LEN] ; //状态名称
	UINT16              m_StatusResID;  //法术资源ID
	INT32               m_StatusTime;  //状态持续的时间
	
};


//移除状态
struct SActionRemoveStatus
{
	UID                 m_uidCreature;         //中状态的生物
	UINT8               m_StatusType;          //状态类型
};

//伤害类型
enum enDamageType  VC_PACKED_ONE
{
   enDamageType_Physics = 0,   //物理伤害
   enDamageType_Magic,          //法术伤害
   enDamageType_Talisman,        //法宝伤害
   enDamageType_Sword,          //剑诀伤害
   enDamageType_Persist,        //持续伤害
   enDamageType_Max,
} PACKED_ONE;


//受到伤害
struct SActionDamageInfo
{
	UID                m_uidCreature;         //受到攻击的生物
	enDamageType       m_DamageType;          //伤害类型
	INT16              m_DamageValue;         //伤害值
	bool               m_bDie;                //是否死亡	
};

//攻击类型
enum enAttackType  VC_PACKED_ONE
{
	enAttackType_Physics = 0,   //物理攻击
	enAttackType_Magic,         //法术攻击
	enAttackType_Talisman,
	enAttackType_Sword,
	enAttackType_Max,
};

//受攻击者信息
struct SAttackedTarget
{
	UID           m_uidTarget;  //被攻击目标
	INT32         m_DamageValue;   //伤害值
	
	INT32         m_AddBloodValue; //回血值

	bool          m_bDie;          //是否死亡
	bool          m_bHit;       //是否命中
	bool          m_bKnocking;  //是否爆击
	enDamageType  m_DamageType;          //伤害类型


	SAttackedTarget()
	{
		MEM_ZERO(this);
	}
};

//攻击信息
struct SActionAttackInfo
{
	SActionAttackInfo()
	{
		MEM_ZERO(this);
	}
	UID           m_uidSource;  //发起者
	enAttackType  m_AttackType; //类型
	UINT32        m_AckNo;      //确认号，为零不需要确认
	TMagicID      m_MagicID;    //法术ID	
	char          m_szMagicName[THING_NAME_LEN] ; //法术名称
	UINT16        m_MagicResID;  //法术资源ID

	INT32       m_AddBloodValue; //发起者吸血或者扣血
	bool        m_bDie;        //是否死亡

	UINT8         m_TargetNum;  //被攻击目标数量
	//SAttackedTarget [m_TargetNum]; //后面跟被攻击目标信息
	
};


//加血信息
struct SAddBloodTarget
{
	UID           m_uidCreature;         //加血的生物
	INT32         m_BloodValue;          //血量
	bool          m_bDie;                //是否死亡

	SAddBloodTarget()
	{
		MEM_ZERO(this);
	}

};

struct SActionAddBloodInfo
{
	SActionAddBloodInfo()
	{
		MEM_ZERO(this);
	}
	UID           m_uidSource;  //发起者
	TMagicID      m_MagicID;    //法术ID	
	char          m_szMagicName[THING_NAME_LEN] ; //法术名称
	UINT16        m_MagicResID;  //法术资源ID
	UINT8         m_TargetNum;  //被攻击目标数量
	//SAddBloodTarget [m_TargetNum]; //后面跟被攻击目标信息
};

//战斗动作
struct SCombatAction
{
	enActionType             m_ActionType;
	/*union
	//{
		SActionStatusInfo    m_StatusInfo;   //状态信息
		SActionRemoveStatus  m_RemoveStatus; //移除状态
		SActionDamageInfo    m_DamageInfo;   //伤害信息
		SActionAttackInfo    m_AttackInfo;   //攻击信息
		SActionAddBloodInfo  m_AddBloodInfo; //加血信息
	//};*/
	
};


enum enCombatIndexMode VC_PACKED_ONE
{
	enCombatIndexMode_Com = 0,	//普通战斗模式
	enCombatIndexMode_Hard,		//困难战斗模式
	enCombatIndexMode_Team,		//组队战斗模式
}PACKED_ONE;




//副本中挑战怪物
struct CS_CombatCombatNpcInFuBen_Req
{
	UID					m_uidNpc;			//怪物
	TFuBenID			m_FuBenID;			//副本ID
	enCombatIndexMode	m_CombatIndexMode;	//战斗模式
};


struct SC_CombatCombatNpcInFuBen_Rsp
{
	enCombatCode			m_Result;
	//CombatNpcInFuBenData  m_FuBenData;
};

struct CombatNpcInFuBenData
{
	CombatNpcInFuBenData() : m_bOver(false),m_FuBenID(INVALID_FUBEN_ID),m_SecretLevel(0)
	{
		MEM_ZERO(this);
	}

	bool				m_bOver;			//是否已打通关
	TFuBenID			m_FuBenID;			//副本ID
	UINT8				m_SecretLevel;		//剑诀等级提升到
};

//离开副本
struct CS_CombatLeaveFuBen_Req
{
	TFuBenID     m_FuBenID;  //副本ID
};

struct SC_CombatLeaveFuBen_Rsp
{
	enCombatCode m_Result;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
