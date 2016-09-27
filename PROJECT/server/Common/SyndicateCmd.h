#ifndef __XJCQ_GAMESRV_SYNDICATE_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_SYNDICATE_CMD_PROTOCOL_H__

#include "ProtocolHeader.h"

#pragma pack(push,1)



//帮派

//消息
enum enSyndicateCmd VC_PACKED_ONE
{
	enSyndicateCmd_CreateSyn,						//创建帮派
	enSyndicateCmd_ApplySyn,						//申请加入帮派
	enSyndicateCmd_ViewSynList,						//查看帮派列表
	enSyndicateCmd_ViewSynMemberList,				//查看帮派成员列表
	enSyndicateCmd_ViewSynApplyList,				//查看帮派加入申请者列表
	enSyndicateCmd_RenMian,							//任免
	enSyndicateCmd_Invite,							//邀请玩家加入帮派
	enSyndicateCmd_Allow,							//批准玩家加入帮派
	enSyndicateCmd_NotAllow,						//不批准玩家加入帮派
	enSyndicateCmd_QuitSyn,							//玩家确定退出帮派(帮主为解散)
	enSyndicateCmd_ViewSynShop,						//查看帮派商铺
	enSyndicateCmd_BuySynGoods,						//买帮派物品
	enSyndicateCmd_ViewSynMagic,					//查看帮派技能
	enSyndicateCmd_LearnSynMagic,					//学习帮派技能
	enSyndicateCmd_EnterSyn,						//进入帮派界面


	enSyndicateCmd_OpenSynCombat,                //打开帮派保卫战面板
	enSyndicateCmd_EnterSynCombat,               //进入帮派保卫战
	enSyndicateCmd_CombatNpc,                    //和NPC战斗

	enSyndicateCmd_ZhaoMuSynMember,				 //招募帮派成员

	enSyndicateCmd_UpdateContribution,			 //更新玩家的帮派贡献值

	enSyndicateCmd_OpenSynWelfare,				 //查看帮派福利

	enSyndicateCmd_ResetSynCombat,				 //重置帮派保卫战

	enSyndicateCmd_ViewSynFuBenDropGoods,		//查看帮派副本掉落物品

	enSyndicateCmd_SC_CloseSynWindow,			//关闭帮派页面(帮派解散时，通知帮众)

	enSyndicateCmd_SynFuBenAutoCombat,			//帮派副本自动快速攻打



	enSyndicateCmd_OpenSynWar,					//打开帮战
	enSyndicateCmd_OpenDeclareWar,				//打开宣战页面
	enSyndicateCmd_DeclareWar,					//帮战宣战
	enSyndicateCmd_AcceptDeclareWar,			//接受宣战
	enSyndicateCmd_RefuseDeclareWar,			//拒绝宣战
	enSyndicateCmd_EnterSynWar,					//进入帮战
	enSyndicateCmd_BuyCombatNum,				//购买战斗次数
	enSyndicateCmd_SynWarCombat,				//帮战战斗

	enSyndicateCmd_ChangeSynMsg,				//修改帮派公告

	enSyndicateCmd_GetSynWarData,				//得到帮战数据

	enSyndicateCmd_PopSynResult,				//弹出帮派结果框

	enSyndicateCmd_SynFuBenBossInfo,			//帮派副本BOSS信息
	

	enSyndicateCmd_Max,
}PACKED_ONE;

//帮派，消息头
struct SyndicateHeader : public AppPacketHeader
{
	SyndicateHeader(enSyndicateCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Syndicate;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//创建帮派
struct CS_CreateSyndicate_Req
{
	CS_CreateSyndicate_Req(){
		MEM_ZERO(this);
	}
	char		m_szCreateSynName[THING_NAME_LEN];		//创建的帮派名称
};



//操作结果码
enum enSynRetCode VC_PACKED_ONE
{
	enSynRetCode_OK             =0,
	enSynRetCode_ExistName,					//帮派名称已经存在
	enSynRetCode_ExistSyn,					//玩家已经有帮派了
	enSynRetCode_ExistSynApply,				//玩家已经在帮派的申请加入名单中
	enSynRetCode_NoSyn,						//该帮派不存在
	enSynRetCode_MemberFull,				//帮派人数已满
	enSynRetCode_NoPermission,				//你没有此权限
	enSynRetCode_NoUser,					//不存在此玩家，名字输错
	enSynRetCode_NoMoney,					//玩家钱不够
	enSynRetCode_PacketFull,				//背包已满，请清理背包
	enSynRetCode_NotStone,					//灵石不够学帮派技能
	enSynRetCode_NotContribution,			//帮派贡献值不够
	enSynRetCode_ErrorSynLevel,				//帮派等级不够
	enSynRetCode_ErrLevelLimit,                  //你当前的等级不能进入该模式的帮派保卫战
	enSynRetCode_ErrNumLimit,                  //进入次数限制
	enSynRetCode_ErrNoNpc,                    //该NPC不存在
	nSynRetCode_ErrOrder,                   //击杀顺序有误
	nSynRetCode_ErrBusy,                    //玩家正忙，不能战斗
	enSynRetCode_NotOpenCombat,				//帮派保卫战时间没到，未开启
	enSynRetCode_ErrLevel,					//玩家等级不够创建帮派
	enSynRetCode_ErrSynName,				//创建的帮派名称不能为空字符串
	enSynRetCode_ErrLevelJoin,				//玩家等级不够加入帮派
	enSynRetCode_ErrQuitSynLeader,			//帮主不能退出帮派，必须得先任命一位亲帮主

	enSynRetCode_ErrSelf,					//不能任免自己

	enSynRetCode_ErrJieSan,					//帮派人数大于10人，无法解散

	enSynRetCode_ErrMaxMagicLevel,			//已达到最高等级，不能再升级

	enSynRetCode__ErrKeyword,				//不能使用非法名字

	enSynRetCode_NoSynFuBen,				//当前没有帮派保卫战进度

	enSynRetCode_NotExistApply,				//玩家已不在申请列表中

	enSynRetCode_NotFinished,				//只有通过1次副本后，才能使用快速副本功能

	enSynRetCode_SpaceLess,					//背包空位已不足10格，不能进行快速攻打

	enSynRetCode_NoSyndicate,				//您没有帮派

	enSynRetCode_ErrEnterLevelLimit,     //进入级别限制

	enSynRetCode_ErrVipLevel,				//您的VIP等级不足，不能开启快速打副本

	enSynRetCode_IsInTeamFuBen,				//正在组队副本中

	enSynRetCode_NoMember,					//低于5人可参战的帮派无法帮战

	enSynRetCode_ErrLowJoinLeve,			//帮派必须有高于30级的人员才能报名

	enSynRetCode_ErrDeclared,				//已经有帮战，不可再宣战

	enSynRetCode_ErrOtherDeclared,			//对方已经向其他帮派宣战

	enSynRetCode_ErrNotInTime,				//不在宣战时间

	enSynRetCode_ExistDeclareThis,			//已经向此帮派宣战过

	enSynRetCode_DeclareMySelf,				//不能向自己宣战

	enSynRetCode_ErrNotInAttackTime,		//不在攻击时间

	enSynRetCode_ErrNoEnemySyn,				//您的帮派没有帮战

	enSynRetCode_ErrNoCombatNum,			//您今天的战斗次数用完了

	enSynRetCode_ErrNoGodStone,				//仙石不足

	enSynRetCode_InCDTime,					//再一次发送帮派招募必须等待1分钟

	enSynRetCode_ErrNotLeaderDeclare,		//只有帮主能宣战

	enSynRetCode_ErrInDuoBao,				//无法进入，正在匹配夺宝战

	enSynRetCode_ErrHaveTeam,				//组队中，无法进入

	enSynRetCode_ErrWaitTeam,				//等待组队中，无法操作

	enSynRetCode_Err,


	enSynRetCode_Max,
}PACKED_ONE;

//创建帮派操作结果
struct SC_CreateSyndicate_Rsp
{
	SC_CreateSyndicate_Rsp() : m_RetCode(enSynRetCode_OK){}
	enSynRetCode	m_RetCode;
};

//操作结果(帮派里只返回一个操作结果的都用这个)
struct SC_Syndicate_Rsp
{
	SC_Syndicate_Rsp() : m_SynRetCode(enSynRetCode_OK)
	{
	}

	enSynRetCode  m_SynRetCode;
};

//申请加入帮派
struct CS_ApplyJoinSyn_Req
{
	CS_ApplyJoinSyn_Req() : m_SynID(0){	}
	TSynID		m_SynID;
};

//查看列表时发的，包括帮派列表和帮派成员列表
struct CS_ViewList_Req
{
	CS_ViewList_Req() : m_RankNumBegin(0), m_RankNumEnd(0){}
	UINT16		m_RankNumBegin;			//查看从第几名开始
	UINT16		m_RankNumEnd;			//到第几名结束
};

//查看帮派列表
struct ViewSynList_Rsp
{
	TSynID		m_SynID;						//帮派ID
	UINT16		m_nRank;						//第几名
	char		m_szSynName[THING_NAME_LEN];	//帮派名称
	UINT8		m_Level;						//帮派等级
	char		m_szLeaderName[THING_NAME_LEN];	//帮主名字
	UINT16		m_MemberNum;					//帮派成员数量
};

//查看列表时,发的真实帮派数量，包括在帮派列表和帮派成员列表
struct SC_ViewListNum_Rsp
{
	SC_ViewListNum_Rsp() : m_SynNum(0){}
	UINT16		m_SynNum;					//发的真实帮派数量
//	ViewSynList_Rsp SynList[m_SynNum];		//帮派信息
};

//查看帮派成员列表
struct SC_ViewSynMemberList_Rsp
{
	UID			m_uidSynMember;						//帮派成员的UID值
	char		m_szSynMemberName[THING_NAME_LEN];  //帮派成员的名字
	UINT8		m_Level;							//成员的等级
	UINT32		m_Contribution;						//成员的贡献值
	UINT8		m_Position;							//成员的职位
	UINT8		m_vipLevel;							//vip等级
	UINT32		m_SynWarLv;							//帮战等级
	UINT32		m_SynScore;							//功勋
	UINT32		m_LastOnLineTime;					//最后在线时间
	bool		m_bIsValid;							//最后在线时间是否有效
};

//帮派职位
enum  enumSynPosition VC_PACKED_ONE
{
	enumSynPosition_Leader,						//帮主
	enumSynPosition_SecondLeader,				//副帮主
	enumSynPosition_Elder,						//长老
	enumSynPosition_General,					//帮派普通成员

	enumSynPosition_Max,
}PACKED_ONE;

//查看帮派加入申请名字列表，先发申请者数量
struct SC_ViewSynApplyListNum_Rsp
{
	SC_ViewSynApplyListNum_Rsp() : m_SynApplyListNum(0){}
	UINT16		m_SynApplyListNum;					//申请者的数量
};

//查看帮派加入申请名单列表
struct SC_ViewSynApplyList_Rsp
{
	UINT64		m_uidApplyUser;						//申请者的UID值
	char		m_szApplyUserName[THING_NAME_LEN];	//申请者的名字
	UINT8		m_Level;							//申请者的等级
	UINT8		m_Layer;							//申请者的境界
};

//任免类型
enum enRenMianType VC_PACKED_ONE
{
	enRenMianType_Leader = 0,				//任免为帮主(值与enumSynPosition中的帮主对应)
	enRenMianType_SecondLeader,				//任免为副帮主(值与enumSynPosition中的副帮主对应)
	enRenMianType_Elder,					//任免为长老(值与enumSynPosition中的长老对应)
	enRenMianType_General,					//任免为普通帮派成员(值与enumSynPosition中的普通成员对应)

	enRenMianType_LetOut,					//踢出

	enRenMianType_Max,
}PACKED_ONE;
//任免
struct CS_RenMianSynMember_Req
{
	CS_RenMianSynMember_Req(){
		MEM_ZERO(this);
	}
	enRenMianType	m_RenMianType;						//任免类型
	UID				m_TargetUserUID;					//任免目标的UID值
	char			m_szTargetUserName[THING_NAME_LEN];	//任免目标的名字
};

struct SC_RenMianSynMember_Rsp
{
	SC_RenMianSynMember_Rsp() : m_SynRetCode(enSynRetCode_OK), m_SynMemberNum(0)
	{
	}

	enSynRetCode  m_SynRetCode;			//返回码
	UINT16	      m_SynMemberNum;		//帮派人数,返回码OK时才为有效值
};

//帮派邀请玩家加入帮派(写帮派信)
struct CS_InviteUser_Req
{
	CS_InviteUser_Req(){
		MEM_ZERO(this);
	}
	char		m_szDestUserName[THING_NAME_LEN];		//收件人的名字
};

//批准玩家加入帮派
struct CS_AllowAdd_Req
{
	CS_AllowAdd_Req() : m_num(0)
	{
	}

	UINT8		m_num;					//发送过来的UID个数
//	UID			m_uidApplyUser[m_num];	//申请者的UID值
};

struct SC_AllowRetCode_Rsp
{
	SC_AllowRetCode_Rsp() : m_SynRetCode(enSynRetCode_OK), m_SynMemberNum(0)
	{
	}

	enSynRetCode  m_SynRetCode;
	UINT16	      m_SynMemberNum;		//帮派人数,返回码OK时才为有效值
};

//拒绝玩家加入帮派
struct CS_RefuseAdd_Req
{
	CS_RefuseAdd_Req() : m_num(0)
	{
	}

	UINT8		m_num;
//	UID		m_uidApplyUser[m_num];			//申请者的UID值	
};

struct SC_RefuseAdd_Rsp
{
	SC_RefuseAdd_Rsp() : m_SynRetCode(enSynRetCode_OK)
	{
	}

	enSynRetCode  m_SynRetCode;
};

//退出帮派结果码
struct SC_QuitSynRetCode_Rsp
{
	SC_QuitSynRetCode_Rsp() : m_SynRetCode(enSynRetCode_OK){}
	enSynRetCode  m_SynRetCode;
};

//帮派商铺标签
enum enSynShopLabel VC_PACKED_ONE
{
	enSynShopLabel_General = 0,				//普通物品
	enSynShopLabel_Equip,					//装备
	enSynShopLabel_GodSword,				//仙剑
	enSynShopLabel_Talisman,				//法宝

	enSynShopLabel_Max,
}PACKED_ONE;

//查看帮派商铺
struct CS_ViewSynShop_Req
{
	CS_ViewSynShop_Req() : m_SynShopLabel(enSynShopLabel_General){}
	enSynShopLabel	m_SynShopLabel;
};

//查看帮派商铺，先发的
struct SC_ViewSynShop_Rsp
{
	SC_ViewSynShop_Rsp() : m_GoodsNum(0){}
	UINT16			m_GoodsNum;			//此标签的帮派物品数量
	//ViewSynGoods_Rsp SynGoods[m_GoodsNum];	//帮派物品信息
};
//查看帮派商铺
struct ViewSynGoods_Rsp
{
	ViewSynGoods_Rsp() : m_GoodsID(0), m_Contribution(0){}
	TGoodsID		m_GoodsID;			//帮派物品ID
	UINT32			m_Contribution;		//购买需要的贡献值
};

//购买帮派物品
struct CS_BuySynGoods_Req
{
	CS_BuySynGoods_Req() : m_GoodsID(0){}
	TGoodsID		m_GoodsID;			//物品ID
};

//查看帮派技能(先发的一部分)
struct SC_LearnSynMagicNum_Rsp
{
	SC_LearnSynMagicNum_Rsp() : m_SynMagicNum(0)
	{
	}

	UINT8			m_SynMagicNum;		//发送的帮派技能数量
};

//查看帮派技能(后发的部分)
struct SC_LearnSynMagicData_Rsp
{
	SC_LearnSynMagicData_Rsp() : m_SynMagicID(0){}
	TSynMagicID		m_SynMagicID;		//帮派技能的ID
	UINT8			m_Level;			//当前等级
};

//学习帮派技能
struct CS_LearnSynMagic_Req
{
	CS_LearnSynMagic_Req() : m_SynMagicID(0){}
	TSynMagicID		m_SynMagicID;
};

struct SC_LearnSynMagic_Rsp
{
	SC_LearnSynMagic_Rsp()
	{
		m_SynRetCode = enSynRetCode_OK;
		m_Contribution = 0;
	}

	enSynRetCode  m_SynRetCode;
	INT32		  m_Contribution;
};

//进入帮派界面
struct SC_EnterSyn_Rsp
{
	SC_EnterSyn_Rsp() : bHaveSyn(false){}
	bool	bHaveSyn;						//是否有帮派
//  if bHaveSyn == ture
//  EnterSynData SynData;
};

//进入帮派界面数据
struct EnterSynData
{
	TSynID	SynID;							//帮派ID
	char    SynName[THING_NAME_LEN];		//帮派名称
	UINT8	Level;							//帮派等级
	UINT16	SynMemberNum;					//帮派人数
	char	LeaderName[THING_NAME_LEN];		//帮主名字
	UINT16	Rank;							//帮派目前排名
	INT32	SynExp;							//帮派当前经验
	INT32	UpNeedExp;						//升级总需经验
	INT32   Contribution;					//贡献值
	UINT16	MaxMemberNum;					//可容纳人数
	INT32	SynWarAbility;					//帮派战力
	INT32	SynWarScore;					//帮战积分
	bool	bCanChangeMsg;					//是否有权限修改帮派公告
	char	SynMsg[DESCRIPT_LEN_300];		//帮派公告
};

//帮派招募成员结果
struct SC_ZhaoMuSynMemberRet_Rsp
{
	SC_ZhaoMuSynMemberRet_Rsp() : m_SynRetCode(enSynRetCode_OK)
	{
	}

	enSynRetCode  m_SynRetCode;
};

//更新玩家的帮派贡献
struct SC_UpdateContribution
{
	UID		m_UserUID;
	INT32	m_Contribution;		//贡献值
};

//帮派福利

struct SC_SynWelfare_Rsp
{
	SC_SynWelfare_Rsp() : m_Num(0)
	{
	}

	UINT8			m_Num;
//	SynWelfareData  WelfareData[m_Num];
};
struct SynWelfareData
{
	SynWelfareData(){
		memset(this, 0, sizeof(*this));
	}

	TSynWelfareID		m_SynWelfareID;						//福利ID
	char				m_SynWelfareName[THING_NAME_LEN];	//福利名字
	UINT8				m_NeedSynLevel;						//需要帮派等级
	UINT16				m_ResID;							//资源ID
	char				m_Descript[DESCRIPT_LEN_75];		//描述
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//打开帮派面板

//打开帮派面板应答
struct SC_OpenSynCombat_Rsp
{
	UINT8        m_CombatNum;		//今天已挑战次数
	UINT16		 m_MaxVipEnterNum;	//vip最多可挑战次数
	TFuBenID	 m_FuBenID;			//副本ID
	UINT8		 m_Level;			//副本级别
	UINT8		 m_KillNum;			//杀敌数
};

//进入保卫战请求
struct CS_EnterSynCombat_Req
{
	UINT8       m_Mode;  //模式，从1开始

};

//挑战怪物
struct CS_SynCombatNpc_Req
{
	UID          m_uidNpc;
	TFuBenID     m_FuBenID;  //副本ID
};

struct SC_SynCombatNpc_Rsp
{
	enSynRetCode  m_Result;
	bool       m_bWin;   //是否胜利
	bool       m_bOver;  //是否整个帮战结束
	UINT32	   m_SynContribution;	//得到的帮派贡献值
	UINT32	   m_SynExp;			//帮派得到多少经验
};

//重置帮派保卫战
struct SC_ResetSynCombat_Rsp
{
	enSynRetCode  m_Result;
};

enum enSynMode VC_PACKED_ONE
{
	enSynMode_Easy = 1,	//简单
	enSynMode_Com,		//普通
	enSynMode_Hard,		//困难
	enSynMode_HardHard,	//噩梦

}PACKED_ONE;


//帮派副本BOSS信息
struct CS_SynFuBenBossInfo
{
	enSynMode SynMode;
};


struct SC_SynFuBenBossInfo
{
	UINT8			m_Num;
//	SynFuBenBoss	m_FuBenBoss[m_Num];
};

struct SynFuBenBoss
{
	UINT16  DropID;
	char	m_BossName[THING_NAME_LEN];
};


//查看帮派副本掉落物品
struct CS_ViewSynFuBenForwardGoods
{
	UINT16 DropID;
};

struct SC_ViewSynFuBenForwardGoods
{
	UINT16			m_Num;
//	TGoodsID		m_GoodsID[m_Num];
};

enum enCloseSynWindow
{
	enCloseSynWindow_JieSan = 0,	//解散帮派关窗
	enCloseSynWindow_TiChu,			//被踢出帮派关窗
};

struct SC_CloseSynWindow
{
	enCloseSynWindow m_CloseType;
};

//帮派副本自动快速打
struct SC_SynFuBenAutoCombat
{
	SC_SynFuBenAutoCombat()
	{
		m_Result = enSynRetCode_OK;
		m_Num	 = 0;
		m_SynContribution = 0;
		m_SynExp = 0;
	}

	enSynRetCode	m_Result;
	INT32			m_SynContribution;
	INT32			m_SynExp;
	UINT16			m_Num;
//	AutoSynCombatNpc m_AutoCombatNpc[m_Num];
};

struct AutoSynCombatNpc
{
	AutoSynCombatNpc()
	{
		m_Exp = 0;
		m_DropNum = 0;
	}

	char			 m_NpcName[THING_NAME_LEN];
	INT32			 m_Exp;					//经验
	UINT16			 m_DropNum;				//掉落数量
//	DropSynGoodsInfo m_GoodsInfo[m_DropNum];	
};

struct DropSynGoodsInfo
{
	TGoodsID	m_GoodsID;
	UINT16		m_GoodsNum;
};

//打开帮战
struct SC_OpenSynWar
{
	SC_OpenSynWar()
	{
		m_SynWarLevel = 0;
		m_CreditUp = 0;
		m_GetCreditWeak = 0;
		m_bHaveVsSyn = false;
		m_bOpenCombat= false;
		m_bOpenDeclare = false;
	}
	INT32		m_SynWarLevel;					//帮战等级
	INT32		m_CreditUp;						//本周声望上限
	INT32		m_GetCreditWeak;				//本周获得声望
	bool		m_bOpenCombat;					//是否开启帮战战斗
	bool		m_bOpenDeclare;					//是否开启帮战宣战斗
	bool		m_bHaveVsSyn;					//是否有和你挑战的帮派
//如果有的话
//	VsSynName	m_SynName;						//和你挑战的帮派名称
};

struct VsSynName
{
	char		m_VsSynName[THING_NAME_LEN];	//和你挑战的帮派名称
};

//打开宣战页面
struct SC_OpenDeclareWar
{
	SC_OpenDeclareWar()
	{
		m_SynNum = 0;
		m_ApplySynNum = 0;
	}
	INT32			m_SynNum;						//可以宣战的帮派数量
	INT32			m_ApplySynNum;					//向你宣战的帮派数量
//	VsSynInfo		m_VsSynInfo[m_SynNum];
//	ApplySynInfo	m_ApplySynInfo[m_ApplySynNum];
};

struct VsSynInfo
{
	TSynID	m_SynID;				//帮派ID
	UINT8	m_SynLevel;				//帮派等级
	UINT8	m_MemberNum;			//帮派人数
	UINT32	m_Rank;					//帮派排名
	INT32	m_SynWarAbility;		//帮战实力
	char	m_Name[THING_NAME_LEN];	//帮派名称
};

struct ApplySynInfo
{
	TSynID	m_SynID;				//帮派ID
	INT32	m_SynWarAbility;		//帮战实力
	char	m_Name[THING_NAME_LEN];	//帮派名称
};

//帮战宣战
struct CS_DeclareWar
{
	TSynID		m_SynID;	//被宣战帮派
};

struct SC_DeclareWar
{
	SC_DeclareWar()
	{
		m_Result = enSynRetCode_OK;
	}
	enSynRetCode  m_Result;
};

//接受宣战
struct CS_AcceptDeclareWar
{
	TSynID	m_SynID;
};

struct SC_AcceptDeclareWar
{
	SC_AcceptDeclareWar()
	{
		m_Result = enSynRetCode_OK;
	}
	enSynRetCode  m_Result;
};

//拒绝宣战
struct CS_RefuseDeclareWar
{
	TSynID	m_SynID;
};

struct SC_RefuseDeclareWar
{
	SC_RefuseDeclareWar()
	{
		m_Result = enSynRetCode_OK;
	}
	enSynRetCode  m_Result;
};

//进入帮战
struct SC_EnterSynWar
{
	SC_EnterSynWar()
	{
		m_Result = enSynRetCode_OK;
	}
	enSynRetCode	m_Result;
};

struct SynWarUserInfo
{
	UID			m_uidUser;
	UINT16		m_Facade;				//外观
	UINT8		m_Level;				//等级
	char		m_Name[THING_NAME_LEN];	//名字
	bool		m_bCombat;				//是否可以战斗
};

//购买战斗次数
struct SC_BuyCombatNum
{
	SC_BuyCombatNum()
	{
		m_Result = enSynRetCode_OK;
		m_MaxCombatNum = 0;
		m_CanCombatNum = 0;
	}
	enSynRetCode	m_Result;

	UINT16			m_MaxCombatNum;		//当前最大可战斗次数
	UINT16			m_CanCombatNum;		//还可战斗次数
};

//帮战战斗
struct CS_SynWarCombat
{
	UID		uidEnemy;
};

struct SC_SynWarCombat
{
	SC_SynWarCombat()
	{
		m_Result = enSynRetCode_OK;
	}
	enSynRetCode	m_Result;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//修改帮派公告
struct CS_ChangeSynMsg
{
	char		m_szSynMsg[DESCRIPT_LEN_300];
};

struct SC_ChangeSynMsg
{
	SC_ChangeSynMsg()
	{
		MEM_ZERO(this);
	}
	enSynRetCode	m_Result;
	char			m_szSynMsg[DESCRIPT_LEN_300];
};

struct EnterSynWarInfo
{
	INT32			m_SynWarLevel;					//帮战等级
	UINT16			m_CanJoinCombatNum;				//还可参加战斗次数
	UINT16			m_MaxJoinCombatNum;				//最多可参加战斗次数
	UINT16			m_CombatWinNum;					//已战胜数
	INT32			m_SynScore;						//我的当前帮派积分
	INT32			m_EnemySynScore;				//敌方的当前帮派积分
	char			m_EnemySynName[THING_NAME_LEN];	//敌方帮派名字
	UINT8			m_Num;
//	SynWarUserInfo	m_UserInfo[m_Num];
};

//得到帮战数据
struct SC_GetSynWarData
{
	INT32			m_SynWarLevel;					//帮战等级
	UINT16			m_CanJoinCombatNum;				//还可参加战斗次数
	UINT16			m_MaxJoinCombatNum;				//最多可参加战斗次数
	UINT16			m_CombatWinNum;					//已战胜数
	INT32			m_SynScore;						//我的当前帮派积分
	INT32			m_EnemySynScore;				//敌方的当前帮派积分
	char			m_EnemySynName[THING_NAME_LEN];	//敌方帮派名字
	UINT8			m_Num;
//	SynWarUserInfo	m_UserInfo[m_Num];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//帮战胜负
enum  enumSynWarbWin VC_PACKED_ONE
{
	enumSynWarbWin_Fail,				//输
	enumSynWarbWin_Win,					//赢
	enumSynWarbWin_Draw,				//平局

	enumSynWarbWin_Max,
}PACKED_ONE;

//弹出帮派结果框
struct SC_PopSynResult
{
	enumSynWarbWin m_SynWarbWin;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
