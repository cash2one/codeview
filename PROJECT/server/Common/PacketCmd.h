#ifndef __XJCQ_GAMESRV_PACKET_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_PACKET_CMD_PROTOCOL_H__

#pragma pack(push,1)



//背包消息

//背包消息命令字
enum enPacketCmd VC_PACKED_ONE
{
	enPacketCmd_Sync = 0,           //数据同步
	enPacketCmd_AddGoods,           //增加物品
	enPacketCmd_RemoveGoods,        //移除物品	
	enPacketCmd_Extend,             //扩展容量
	enPacketCmd_UseGoods,           //使用物品
	enPacketCmd_DiscardGoods,       //丢弃物品
	enPacketCmd_Equip,              //装备
	enPacketCmd_Compose,            //合成

	enPacketCmd_ActorChangeName,	//角色更名卡使用

	enPacketCmd_SC_TiShi,			//只提示玩家(使用物品时,显示服务端发给客户端的文本)
	enPacketCmd_SC_TiShiReplaceOK,  //提示并让玩家选择是否替换状态(使用物品时,玩家选是的话,客户端发确认使用物品消息给服务端)

	enPacketCmd_ChangYinFu,			//传音符使用

	enPacketCmd_SellGoods,			//出售物品

	enPacketCmd_ConfirmationUseGoods, //确认使用物品

	enPacketCmd_ClickAptitude,		//点击人物面板的资质按键

	enPacketCmd_Max,

}PACKED_ONE;


struct Packet_Header: public AppPacketHeader
{
	Packet_Header(enPacketCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Packet;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//背包操作相关结果码
enum enPacketRetCode VC_PACKED_ONE
{
	enPacketRetCode_OK    =  0,			//OK
    enPacketRetCode_ExtendNumLimit,		//次数限制
	enPacketRetCode_NoStone,			//没灵石
	enPacketRetCode_NoGoods,			//物品不存在
	enPacketRetCode_NoCompose,			//该物品不能合成
	enPacketRetCode_NoSpace,			//背包已满，请清理背包
	enPacketRetCode_NoMaterial,			//数量不足，无法合成
	enPacketRetCode_NoMoney,			//仙石不足
	enPacketRetCode_UnEquip,			//该物品不是装备，不能装载
	enPacketRetCode_LevelLimit,			//等级不足，无法装备
	enPacketRetCode_NoTarget,			//目标不存在
	enPacketRetCode_NoGoodsUse,			//该物品不能使用
	enPacketRetCode_NoNum,				//物品数量不足
	enPacketRetCode_ErrParam,			//使用参数有误
	enPacketRetCode_ErrStudyMagic,		//学习法术失败
	enPacketRetCode_ErrAddGoods,		//背包已满，请清理背包
	enPacketRetCode_ErrAddResource,		//增加资源失败
	enPacketRetCode_ErrActorProp,		//增加人物属性失败
	enPacketRetCode_ErrGoodsProp,		//增加物品属性失败
	enPacketRetCode_ErrCreateEmployee,	//创建招募角色失败
	enPacketRetCode_ErrNumLimit,		//超出可招募角色数量上限
	enPacketRetCode_ErrWorldNotice,		//全服公告失败
	enPacketRetCode_ErrCDTime,			//使用失败，冷却时间未到
	enPacketRetCode_Error,				//程序错误
	enPacketRetCode_NoActor,			//不存在该角色

	enPacketRetCode_FullStatusGroup,	//该组状态已满

	enPacketRetCode_ChangYinFu,			//该物品是传音符(接着走enPacketCmd_ChangYinFu这条消息)
	enPacketRetCode_ActorChangeName,	//该物品角色更名卡(接着走enPacketCmd_ActorChangeName这条消息)

	enPacketRetCode_NotSynMember,		//玩家没有帮派

	enPacketRetCode_ErrChangeUserName,	//不能更改主角名字

	enPacketRetCode_ErrFullAptitude,	//资质已开发完毕，无法使用
	enPacketRetCode_ErrLevel,			//玩家等级不足

	enPacketRetCode_ErrNotSell,			//该物品无法出售

	enPacketRetCode_ErrExistMagic,		//学习失败，无法重复学习

	enPacketRetCode_ErrFullEmployee,	//使用失败，招募角色已达上限

	enPacketRetCode_ErrMaxEffect,		//最多存在2种效果，请取消后其中一种再使用

	enPacketRetCode_ErrSameGodSwrod,    //不能装备两把相同的仙剑或法宝

	enPacketRetCode_ErrExistEmploy,		//使用失败，已经拥有该角色

	enPacketRetCode_ErrLevelUp,			//已达等级上限,无法使用

	enPacketRetCode_OK_ChangeName,		//改名成功

	enPacketRetCode_ErrNoMoneyExtender,	//仙石不足，扩充失败

	enPacketRetCode_ErrNimbusUp,		//已达灵气上限,无法使用

	enPacketRetCode_ErrGodSwordNimbusUp,//已达仙剑灵气上限,无法使用

	enPacketRetCode_NoChangeCardName,	//不可更改人物卡招唤出的角色的名字

	enPacketRetCode_NoChangeThisName,	//不可更改成传奇不筘角色名字

	enPacketRetCode_ErrName,			//使用失败，已经拥有该角色

	enPacketRetCode_ErrKeyword,			//不能使用非法字符

	enPacketRetCode_ErrLevelUseGaoJiExp,//高级经验丹无法对35级以上的角色使用

	enPacketRetCode_ErrNoMasterUse,		//主角无法使用高级经验丹

	enPacketRetCode_ErrMasterLv,		//无法使用，使用后经验将超过主角

	enPacketRetCode_ErrNoMasterUseSuper,	//主角无法使用超级经验丹

	enPacketRetCode_ErrAptitude18,			//1.8资质以上提升需要固灵丹

	enPacketRetCode_ErrAptitude20,			//2.0资质以上提升需要护灵丹

	enPacketRetCode_Err,
} PACKED_ONE;

//同步背包数据
struct SC_PacketData_Sync
{
	SC_PacketData_Sync() : m_Capacity(0), m_CanExtend(0),  m_Charges(0), m_GoodsNum(0)
	{
	}

	INT16  m_Capacity;  //当前容量
	INT16  m_CanExtend; //下次可扩展容量
	INT32  m_Charges;   //扩展需要费用
	INT16  m_GoodsNum;  //当前拥用物品数量
	//UID  m_uidGoods[m_GoodsNum]; //后面跟物品UID
};


//增加物品
struct SC_PacketAddGoods
{
	UID m_uidGoods;
};

//移除物品
struct SC_PacketRemoveGoods
{
	UID m_uidGoods;
};


//扩充容量

//扩充容量应答
struct SC_PacketExtend_Rsp
{
	SC_PacketExtend_Rsp() : m_Result(enPacketRetCode_OK), m_Capacity(0), m_CanExtend(0), m_Charges(0)
	{
	}

	enPacketRetCode  m_Result;

	INT16  m_Capacity;  //当前容量
	INT16  m_CanExtend; //下次可扩展容量
	INT32  m_Charges;   //扩展需要费用
};


//使用物品请求
struct CS_PacketUseGoods_Req
{
	UID       m_uidUseGoods; //使用的物品
	UID       m_uidTarget;   //使用目标
	INT32     m_UseNum;      //使用数量
};

struct SC_PacketUseGoods_Rsp
{
	enPacketRetCode  m_Result;
//	char			 UseDesc[];		//物品使用说明,如果使用成功的话			
};

//丢弃物品
struct CS_PacketDiscardGoods_Req
{
	UID  m_uidGoods;
};

struct SC_PacketDiscardGoods_Rsp
{
	enPacketRetCode  m_Result;
};

//装备请求
struct CS_PacketEquip_Req
{
	UID m_uidGoods;
	UID m_uidTarget;
	UINT8 m_Pos; //位置
};

struct SC_PacketEquip_Rsp
{
	enPacketRetCode  m_Result;
};

//合成
struct CS_PacketCompose_Req
{
	TGoodsID  m_gidOutput; //产出物	
	INT16     m_OutputNum; //产出数量
};


struct SC_PacketCompose_Rsp
{
	enPacketRetCode  m_Result;
};

//角色更名卡使用
struct SC_ActorChangeName
{
	UID       m_uidUseGoods;		//使用的物品
	UID       m_uidTarget;			//使用目标
	INT32     m_UseNum;				//使用数量
};

struct CS_ActorChangeName
{
	UID       m_uidUseGoods;		//使用的物品
	UID       m_uidTarget;			//使用目标
	char	  m_szChangeName[THING_NAME_LEN];		//更改后的名字
};

struct SC_ActorChangeNameRet
{
	enPacketRetCode  m_Result;
};

//弹出确认框,是否替换状态
struct SC_ReplaceStatus_Rsp
{
	UINT8	  m_Index;		 //检测到第几个状态组
	UID       m_uidUseGoods; //使用的物品
	UID	      m_uidTarget;   //使用目标
	INT32     m_UseNum;      //使用数量
	char	  m_szRepalceTiShi[DESCRIPT_LEN_75];	//替换提示
};

//只给提示
struct SC_TiShi_Rsp
{
	char	  m_szRepalceTiShi[DESCRIPT_LEN_75];	//提示	
};


struct SC_ChangYinFu
{
	UID       m_uidUseGoods; //使用的物品
	UID       m_uidTarget;   //使用目标
	INT32     m_UseNum;      //使用数量
};

//传音符使用
struct CS_ChangYinFu
{
	UID       m_uidUseGoods;		//使用的物品
	char	  m_szContext[DESCRIPT_LEN_50];	//传音符文本	
};

struct SC_ChangYinFuRet
{
	enPacketRetCode  m_Result;	
};

//出售物品
struct CS_SellPacketGoods_Req
{
	UID		m_GoodsUID;				//出售的物品UID
	INT32	m_SellGoodsNum;			//出售的物品数量
};

struct SC_SellGoods_Rsp
{
	SC_SellGoods_Rsp() : m_Result(enPacketRetCode_OK), m_Price(0){}
	enPacketRetCode  m_Result;	
	INT32			 m_Price;		//得到的灵石
};

//确认使用物品请求
struct CS_ConfirmationUseGoods_Req
{
	UID       m_uidUseGoods; //使用的物品
	UID       m_uidTarget;   //使用目标
	INT32     m_UseNum;      //使用数量
	UINT8	  m_Index;		 //检测到第几个状态组
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//点击人物面板的资质按键
struct CS_ClickAptitude
{
	TGoodsID	m_GoodsID;
	UID			m_TargetUID;
};

struct SC_ClickAptitude
{
	INT32	  m_NextSpirit;		//下一级灵力值
	INT32	  m_NextShield;		//下一级护盾值
	INT32	  m_NextBloodUp;	//下一级气血上限值
	INT32	  m_NextAvoid;		//下一级身法值

	TGoodsID  m_GoodsID;		//需要的辅助物品ID (0为不需要辅助物品)
	UINT16	  m_GoodsNum;		//需要的物品数

	INT32	  m_DescAptitude;	//描述用的资质
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
