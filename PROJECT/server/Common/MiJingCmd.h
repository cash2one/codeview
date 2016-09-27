
#ifndef __XJCQ_GAMESRV_MIJING_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_MIJING_CMD_PROTOCOL_H__


#pragma pack(push,1)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//秘境消息

//秘境消息命令字
enum enMiJingCmd VC_PACKED_ONE
{

	enMiJingCmd_StrongInheritance = 0,   //强化传承
	enMiJingCmd_AddGhost,				 //法宝附灵
	enMiJingCmd_RemoveGhost,			 //灵件摘除
	enMiJingCmd_UpGhost,				 //灵件升级
	enMiJingCmd_UpCreditSkill,			 //声望技能提升
	enMiJingCmd_EquipMake,				 //装备制作
    enMiJingCmd_UpSwordMigic,			 //剑诀提升
	enMiJingCmd_OpenMiJing,				 //打开秘境界面
	enMiJingCmd_ReplaceGhost,			 //灵件替换


	enMiJingCmd_Max,

}PACKED_ONE;


struct MiJing_Header: public AppPacketHeader
{
	MiJing_Header(enMiJingCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_MiJing;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//秘境操作相关结果码
enum enMiJingRetCode VC_PACKED_ONE
{
	enMiJingRetCode_OK    =  0,         //OK
	enMiJingRetCode_ActorLevelLow,      //角色等级不足
	enMiJingRetCode_ErrType,			//类型错误，不能升级
	enMiJingRetCode_UpFail,				//升级失败
	enMiJingRetCode_ErrMaxLevel,		//已达最高等级，不能再升
	enMiJingRetCode_ErrPacket,			//获取背包失败
	enMiJingRetCode_NoGoods,            //剑魂碎片不足
	enMiJingRetCode_ErrCnfg,			//获取剑诀配置或声望技能配置或装备强化配置失败

	enMiJingRetCode_NoPolyNimbus,		//聚灵气不足
	enMiJingRetCode_NoMoney,			//仙石不足
	enMiJingRetCode_EquipLevelLow,		//传承物品的强化等级必须高于被传承物品
	enMiJingRetCode_ErrEquip,		    //传承物品错误


	
	enMiJingRetCode_NoNimbus,           //仙剑灵气不足
	enMiJingRetCode_NoCredit,			//声望不足
	enMiJingRetCode_NoSpace,			//背包已满，请清理背包
	enMiJingRetCode_NoMap,				//图纸不足
	enMiJingRetCode_NoTalisman,			//法宝不存在
	enMiJingRetCode_NotTalisman,		//不是法宝，不能附灵
	enMiJingRetCode_NotGhost,			//灵件不存在
	enMiJingRetCode_NoGhostSoul,		//灵魄不足
	enMiJingRetCode_NoMaterial,			//材料不足

	


	enMiJingRetCode_Max,

	

} PACKED_ONE;

//传承方式
enum InheritanceTypeMJ VC_PACKED_ONE
{
	InheritanceTypeMJ_Money = 0,		//付仙石传承
	InheritanceTypeMJ_Ordinary,		//普通传承
}PACKED_ONE;

//强化传承
struct CS_StrongInheritanceMJ_Req
{
	InheritanceTypeMJ m_Type;			//传承方式
	UID				m_SrcEquip;		//传承物品UID
	UID				m_DesEquip;		//被传承物品UID
};

struct SC_StrongInheritanceMJ_Rsp
{
	enMiJingRetCode m_Result;	
};



//法宝附灵
struct CS_AddGhost_Req
{
	UID    m_uidTalisman;      //需要附灵的法宝
	UID    m_uidGhost;         //灵件  
};

struct SC_AddGhost_Rsp
{
	enMiJingRetCode  m_Result;
};


//灵件摘除
struct CS_RemoveGhost_Req
{
	UID			m_uidTalisman;			//法宝UID
};

struct SC_RemoveGhost_Rsp
{
	enMiJingRetCode m_Result;
};

//灵件替换
struct CS_ReplaceGhost_Req
{
	UID			m_uidTalisman;		//装备UID
	UID			m_uidGhost;			//新灵件UID
};

struct SC_ReplaceGhost_Rsp
{
	enMiJingRetCode m_Result;
};


//灵件升级
struct CS_UpGhost_Req
{
	CS_UpGhost_Req()
	{
		m_uidActor = UID();
		m_uidGoods = UID();
	}
	UID				m_uidActor; //角色
	UID				m_uidGoods;  //原灵件物品UID
	
};

struct SC_UpGhost_Rsp
{
	SC_UpGhost_Rsp()
	{
		m_RetCode = enMiJingRetCode_OK;
	}
	enMiJingRetCode m_RetCode;
};


//声望技能类型
enum enCreditMagicType VC_PACKED_ONE
{
	enMagicType_Crit =0 ,      //爆击
	enMagicType_Tenacity,	   //坚韧	
	enMagicType_Hit,           //命中
	enMagicType_Dodge,         //回避
	enMagicType_MagicCD,	   //法术回复

} PACKED_ONE;

//声望技能升级的对象
struct CS_CreditMagicUp_Req
{
	CS_CreditMagicUp_Req() 
	{
		uid_Actor = UID();
		m_MagicType = enMagicType_Crit;
	}

	UID uid_Actor;
	enCreditMagicType     m_MagicType;
};

//声望技能升级
struct SC_CreditMagicUp_Rsp
{
	SC_CreditMagicUp_Rsp()
	{
		m_RetCode = enMiJingRetCode_OK;
	}
	enMiJingRetCode m_RetCode;
};


//升级声望技能
struct CS_UpCreditMagic_Req
{
	CS_UpCreditMagic_Req() 
	{
		m_CreditMagicID = 0;
	}
	UINT16		m_CreditMagicID;
};

//装备制作
struct CS_EquipMake_Req
{
	CS_EquipMake_Req()
	{
		m_uidActor = UID();
		m_Pos = 0;
	}
	UID				m_uidActor; //角色
	UID				m_uidEquip; //原装备
	UINT8  m_Pos;				//部位	
};

//装备制作结果
struct SC_EquipMake_Rsp
{
	SC_EquipMake_Rsp()
	{
		m_RetCode = enMiJingRetCode_OK;
	}
	enMiJingRetCode m_RetCode;
};


//剑诀种类
enum enDamageCategory VC_PACKED_ONE
{
	enGoldDamage =0 ,      //金剑诀升级
	enWoodDamage ,	       //木剑诀升级	
	enWaterDamage,         //水剑诀升级
	enFireDamage,          //火剑诀升级
	enSoilDamage,		   //火剑诀升级

} PACKED_ONE;

//剑诀升级的对象
struct CS_DamageUp_Req
{
	CS_DamageUp_Req() 
	{
		uid_Actor = UID();
		m_DamageCategory = enGoldDamage;
	}

	UID uid_Actor;
	enDamageCategory m_DamageCategory;
};

//剑诀升级
struct SC_DamageUp_Rsp
{
	SC_DamageUp_Rsp() : m_RetCode(enMiJingRetCode_OK){}
	enMiJingRetCode m_RetCode;
};


//打开秘境界面
struct SC_OpenMiJing_Rsp
{
	SC_OpenMiJing_Rsp() 
	{
		m_GodSwordNimbus = 0;
		m_MaxGodSwordNimbus = 0;
	}

	INT32	m_GodSwordNimbus;		 //当前仙剑灵气值
	INT32	m_MaxGodSwordNimbus;	 //仙剑灵气上限

};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
