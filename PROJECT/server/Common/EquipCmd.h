
#ifndef __XJCQ_GAMESRV_EQUIP_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_EQUIP_CMD_PROTOCOL_H__

#include "ProtocolHeader.h"

#pragma pack(push,1)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//装备消息

//装备消息命令字
enum enEquipCmd VC_PACKED_ONE
{
	enEquipCmd_SyncEquip = 0,       //同步装备栏数据
	enEquipCmd_AddEquip,            //增加装备栏装备
	enEquipCmd_RemoveEquip,        //移除装备栏装备

	enEquipCmd_StrongerEquip,      //装备强化
	enEquipCmd_AutoStronger,       //自动强化
    enEquipCmd_Inlay,              //镶嵌

	enEquipCmd_OpenMagic,       //打开装备栏法术数据
	enEquipCmd_AddMagic,            //增加装备栏法术
	enEquipCmd_RemoveMagic,        //移除装备栏法术
	enEquipCmd_StudyMagic,         //学会新法术
	enEquipCmd_Upgrade,            //升级法术


	enEquipCmd_LineupSync,  //同步阵型
	enEquipCmd_JoinBattle,  //设置参战
	
	enEquipCmd_AddLayerLevel,	   //境界升级

	enEquipCmd_OpenSynMagic,	   //查看帮派技能

	enEquipCmd_ViewActorStatus,	   //查看人物状态
	enEquipCmd_DeleteStatus,	   //删除状态

	enEquipCmd_RemoveGem,		   //宝石摘除

	enEquipCmd_ReplaceGem,		   //宝石替换

	enEquipCmd_StrongInheritance,  //强化传承

	//外观栏
    enEquipCmd_OpenFacade,   //打开外观栏
	enEquipCmd_SetFacade,    //设置当前显示的外观

	enEquipCmd_Max,

}PACKED_ONE;


struct Equip_Header: public AppPacketHeader
{
	Equip_Header(enEquipCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Equip;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//装备操作相关结果码
enum enEquipRetCode VC_PACKED_ONE
{
	enEquipRetCode_OK            =  0,  //OK
	enEquipRetCode_PacketFull,          //背包已满，请清理背包
	enEquipRetCode_NoEquip,             //装备不存在
	enEquipRetCode_UnEquip,             //该物品不是装备，不能强化
	enEquipRetCode_LevelLimit,          //已是最大级别，不能再强化了
	enEquipRetCode_NoMaterial,          //缺少原料
	enEquipRetCode_NoExtender,          //缺少增加剂
	enEquipRetCode_ProbFaild,           //强化失败
	enEquipRetCode_NumLimit,            //强化次数不足以强化至目标等级
	enEquipRetCode_NoHole,              //孔没打开
	enEquipRetCode_NoStudyMagic,        //没有学会该法术
	enEquipRetCode_NoNimbus,            //灵气不足
	enEquipRetCode_LayerLimit,            //境界未达到要求
	enEquipRetCode_ErrorPos,              //错误的位置
	enEquipRetCod_NoLoad,                 //没有加载该法术

	enEquipRetCode_ErrActor,         //错误的角色
	enEquipRetCode_ErrLineup,       //错误的阵型编号
	enEquipRetCode_NoJoinBattle,    //该角色没有参战，不能移除参参战

	enEquipRetCode_ErrNimbus,		//灵气不足
	enEquipRetCode_ErrLevel,		//等级不足
	enEquipRetCode_ErrNotOpenLevel, //目前此级别境界尚未开放	

	enEquipRetCode_ErrNoSpace,      //该位置已有参战角色
	enEquipRetCode_ErrNotRemoveMaster,      //主角不能移除，必须参战

	enEquipRetCode_ErrMagicExist,	//该位置已存在法术，不能再装备法术

	enEquipRetCode_ErrSameMagic,	//已经装备有相同法术，不能再装备该法术

	enEquipRetCode_ErrMaxLayer,		//已到达最高境界,不可再升

	enEquipRetCode_MaterialLimit,   //原料不足以强化至目标等级

	enEquipRetCode_HanBinShiLimit,	//寒冰石不足以强化至目标次数

	enEquipRetCode_MaxStrongLvOver, //超出最大可强化等级

	enEquipRetCode_LieYanShiLimit,  //烈焱石不足

	enEquipRetCode_ErrTargetLevelLow,	//目标强化等级必须大于当前强化等级

	enEquipRetCode_OverMaxCombatActorNum,	//超出可出战人数上限

	enEquipRetCode_Err,

	enEquipRetCode_NoStrone_ReGem,			//灵石不足，无法摘除

	enEquipRetCode_LevelLow,				//传承物品的强化等级必须高于被传承物品

	enEquipRetCode_NoMoney,					//仙石不足

	enEquipRetCode_NoStone_Inherit,			//灵石不足，无法传承

	enEquipRetCode_NoPolyNimbus,			//聚灵气不足

	enEquipRetCode_NoFacade,     //你当前没有该外观

	enEquipRetCode_ExistHave,	//已有宝石，不能再次镶嵌

	enEquipRetCode_Max,

	

} PACKED_ONE;

struct EquipInfo
{
	UID   m_uidEquip;
	UINT8 m_pos;
};

//同步装备栏数据
struct SC_EquipPanel_Sync
{
	UID    m_uidActor; //角色
	UINT8 m_EquipNum;  //装备数量
   // EquipInfo  m_uidEquips[m_EquipNum];
};

//增加装备
struct SC_AddEquip_Rsp
{
	UID    m_uidActor; //角色
	UID     m_uidEquip;
	UINT8   m_pos;
};

//移除装备
struct CS_RemoveEquip_Req
{
	UID    m_uidActor; //角色
	UID    m_uidEquip;
};

struct SC_RemoveEquip_Rsp
{
	UID    m_uidActor; //角色
	UID    m_uidEquip;
	enEquipRetCode m_Result;

};


//装备强化
struct CS_EquipStronger_Req
{
	UID  m_TargetActor;	  //需要强化的角色UID
	UID  m_uidEquip;      //需要强化的装备
	bool m_bUseLockStone; //是否使用幸运石
};

struct SC_EquipStronger_Rsp
{
	enEquipRetCode m_Result;
};

//自动强化
struct CS_EquipAutoStronger_Req
{
	UID	   m_TargetActor;	//需要强化的角色UID
	UID    m_uidEquip;      //需要强化的装备
	UINT8  m_starLevel;     //需要强化到达的星级
	INT16  m_StrongerNum;   //强化次数
	bool   m_bUseLockStone; //是否使用幸运石	
};

struct SC_EquipAutoStronger_Rsp
{
	SC_EquipAutoStronger_Rsp() : m_Result(enEquipRetCode_OK)
	{
	}

	enEquipRetCode  m_Result;
//  char			m_Desc[]; //以0结尾的字符串
};

struct EquipStrongerRet
{
	EquipStrongerRet() : m_StrongerNum(0), m_GoodsID(INVALID_GOODS_ID), m_GoodsNum(0)
	{
	}

	INT16						m_StrongerNum;   //强化次数
	TGoodsID					m_GoodsID;		 //使用的物品ID
	UINT8						m_GoodsNum;		 //消耗材料数
};

//镶嵌
struct CS_EquipInlay_Req
{
	UID    m_uidEquip;      //需要镶嵌的装备
	UID    m_uidGem;        //宝石
	UINT8  m_Hole;          //要镶嵌的孔，从零开始   
};

struct SC_EquipInlay_Rsp
{
	enEquipRetCode  m_Result;
};


//打开法术栏


//法术信息
struct SMagicInfo
{
	TMagicID  m_MagicID; //法术ID
	UINT8     m_Level;   //等级
};

//打开法术栏应答
struct SC_EquipOpenMagic_Rsp
{
	UID       m_uidActor;  //角色
	TMagicID  m_EquipMagic[MAX_EQUIP_MAGIC_NUM]; //法术栏已装备的法术,为零表示该栏为空
	UINT16    m_MagicInfoNum;  //已学会的法术数量
	//SMagicInfo m_MagicInfo[m_MagicInfoNum]; //后面跟法术信息
};


//加载法术请求
struct CS_EquipAddMagic_Req
{
	UID       m_uidActor; //角色
	UINT8     m_Pos;  //位置，从零开始
	TMagicID  m_MagicID; //法术ID
};

//应答
struct SC_EquipAddMagic_Rsp
{
	UID       m_uidActor; //角色
	enEquipRetCode m_Result;
	UINT8          m_Pos;  //位置，从零开始
	TMagicID       m_MagicID; //法术ID
};



//卸载法术请求
struct CS_EquipRemoveMagic_Req
{
	UID       m_uidActor; //角色
	UINT8     m_Pos;  //位置，从零开始
	TMagicID  m_MagicID; //法术ID
};

//应答
struct SC_EquipRemoveMagic_Rsp
{
	UID       m_uidActor; //角色
	enEquipRetCode m_Result;
	UINT8          m_Pos;  //位置，从零开始
	TMagicID       m_MagicID; //法术ID
};

//学会新法术
struct SC_EquipStudyMagic_Rsp
{
	UID       m_uidActor; //角色
	TMagicID       m_MagicID;  //法术ID
	UINT8          m_Level;   //等级
};

//升级法术
struct CS_EquipUpgradeMagic_Req
{
	UID       m_uidActor; //角色
	TMagicID    m_MagicID;
};

//应答
struct CS_EquipUpgradeMagic_Rsp
{
	UID       m_uidActor; //角色
	enEquipRetCode m_Result;
	TMagicID       m_MagicID;  //法术ID
	UINT8          m_Level;   //等级
};

//境界升级的对象
struct CS_AddLayerLevel_Req
{
	CS_AddLayerLevel_Req() : uid_Actor(UID().ToUint64()){}
	UINT64 uid_Actor;
};

//境界升级
struct SC_AddLayerLevel_Rsp
{
	SC_AddLayerLevel_Rsp() : m_RetCode(enEquipRetCode_OK){}
	enEquipRetCode m_RetCode;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//同步阵型
struct SC_EquipLineup_Sync
{
	SC_EquipLineup_Sync()
	{
		for(int i=0; i<MAX_LINEUP_POS_NUM; i++)
		{
			m_uidLineup[i] = UID();
		}
	}

	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //阵型
};

//设为参战
struct CS_EquipJoinBattle_Req
{
	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //阵型
};


//设为参战
struct SC_EquipJoinBattle_Rsp
{
	enEquipRetCode m_Result;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//帮派技能

//打开帮派技能负栏
struct SC_OpenSynMagic
{
	UINT8	m_Num;
//	SynMagicInfo_Rsp m_SynMagic[m_Num];
};

struct SynMagicInfo_Rsp
{
	TSynMagicID m_SynMagicID;	//帮派技能ID
	UINT8		m_Level;		//当前帮派技能等级
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//查看角色状态
struct CS_ViewActorStatus_Req
{
	UID			m_uidActor;
};

struct SC_ViewActorStatus_Rsp
{
	SC_ViewActorStatus_Rsp()
	{
		m_Num = 0;
	}
	
	UINT8	m_Num;
//	StatusShowInfo	 StatusInfo[m_Num];
};

struct StatusShowInfo
{
	TStatusID	m_StatusID;					//状态ID
	char		m_Name[THING_NAME_LEN];		//效果名称
	UINT32		m_TotalTime;				//状态总时间
	UINT32		m_RemainTime;				//状态剩余时间
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//删除状态
struct CS_DeleteStatus_Req
{
	TStatusID	m_StatusID;					//状态ID
	UID			m_uidActor;
};

struct SC_DeleteStatus_Rsp
{
	enEquipRetCode m_Result;
};


//宝石摘除
struct CS_RemoveGem_Req
{
	UID			m_uidEquip;			//装备UID
	UINT8		m_index;			//下标,从0开始
};

struct SC_RemoveGem_Rsp
{
	enEquipRetCode m_Result;
};

//宝石替换
struct CS_ReplaceGem_Req
{
	UID			m_uidEquip;			//装备UID
	UINT8		m_index;			//下标，从0开始
	UID			m_uidGem;			//新宝石UID
};

struct SC_ReplaceGem_Rsp
{
	enEquipRetCode m_Result;	
};

//传承方式
enum InheritanceType VC_PACKED_ONE
{
	InheritanceType_Money = 0,		//付仙石传承
	//InheritanceType_stone,		//付灵石传承
	InheritanceType_Ordinary,		//普通传承
}PACKED_ONE;

//强化传承
struct CS_StrongInheritance_Req
{
	InheritanceType m_Type;			//传承方式
	UID				m_SrcEquip;		//传承物品UID
	UID				m_DesEquip;		//被传承物品UID
};

struct SC_StrongInheritance_Rsp
{
	enEquipRetCode m_Result;	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//打开外观栏
struct CS_OpenFacade_Req
{
	UID     m_uidActor;
};

//打开外观栏应答
struct SC_OpenFacade_Rsp
{
	UID     m_uidActor;
	UINT8   m_FacadeNum;
	//UINT16 m_FacadeID[m_FacadeNum];
};


//设置显示外观
struct CS_SetFacade_Req
{
	UID     m_uidActor;
	UINT16 m_FacadeID;
};

struct SC_SetFacade_Rsp
{
	enEquipRetCode m_Result;	
	UID            m_uidActor;
	UINT16         m_FacadeID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
