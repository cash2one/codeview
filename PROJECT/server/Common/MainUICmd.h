#ifndef __XJCQ_GAMESRV_MAINUI_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_MAINUI_CMD_PROTOCOL_H__

#pragma pack(push,1)





/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//主界面相关消息
enum enMainUICmd VC_PACKED_ONE
{
	enMainUICmd_OpenFuBen,  //打开副本面板
	enMainUICmd_Challenge,  //挑战副本
	enMainUICmd_Reset,      //重置副本	

    enMainUICmd_SpawnTalismanGoods ,  //孕育法宝物
	enMainUICmd_TakeTalismanGoods ,   //领取法宝孕育物
	enMainUICmd_UpgradeTalismanQuality,  //提升法宝品质点

	enMainUICmd_UpgradeSword,		 //仙剑升级
	enMainUICmd_EnterGodSwordFuBen,  //进入仙剑副本

	enMainUICmd_OpenGodSword,		 //打开仙剑面板

	enMainUICmd_OnlineTime,   //同步在线时长

	enMainUICmd_CS_Identity,  //通过了第三方验证

	enMainUICmd_CS_Guide,                //新手引导步骤

	enMainUICmd_GuideConfirm, //新手指引确认

	
	enMainUICmd_ViewDropGoods,			//普通副本查看掉落物品

	enMainUICmd_AutoCommonFuBen,		//普通副本快速打怪

	enMainUICmd_OpenViewDrop,			//打开查看掉落物品界面

	enMainUICmd_SC_UserSet,				//玩家的设置信息

	enMainUICmd_SC_SycProcess,				//弹出是否同步副本进度框

	enMainUICmd_SelectSycRet,					//是否同步副本进度的返回

	enMainUICmd_OpenXuanTian,				//打开玄天页面

	enMainUICmd_ShowXuanTianForward,		//显示玄天战斗奖励

	enMainUICmd_AttackXTBoss,				//攻击玄天BOSS

	enMainUICmd_LessCDTime,					//缩短CD时间

	enMainUICmd_TeamChallengeFuBen,			//组队挑战副本

	enMainUICmd_ViewXTDamageRank,			//显示玄天伤害排行

	enMainUICmd_ViewLastKill,					//显示最后一击

	enMainUICmd_PopWaitSyc,					//弹出等待同步框

	enMainUICmd_Max,
}PACKED_ONE;

//战斗结果码
enum enMainUICode VC_PACKED_ONE
{
	enMainUICode_OK,			 //成功
	enMainUICode_ErrNoStartFuBen, //副本无进度，无法重置
	enMainUICode_ErrEnterNumLimit, //进入次数限制
	enMainUICode_ErrFuBenCnfg,     //找不到副本配置信息
	enMainUICode_ErrNoMoney,  //仙石不足
	enMainUICode_ErrNoGoods,  //物品不存在
	enMainUICode_ErrNotTalisman,    //该物品不是法宝
	enMainUICode_ErrSpawnNumLimit,  //可孕育法宝数量上限限制
	enMainUICode_ErrIsSpawn,  //该法宝已在孕育
	enMainUICode_ErrNotCanSpawn,  //该法宝不允许孕育
	enMainUICode_ErrSpawnFail,  //该法宝孕育失败
	enMainUICode_ErrIsNotSpawn,  //该法宝没有在孕育不能领取孕育物
	enMainUICode_ErrNoPacketSpace, //背包已满，请清理背包
	enMainUICode_ErrSpawnNoFinish, //孕育时间未到
	enMainUICode_ErrNoSpawnGoods, //没有孕育物
	enMainUICode_ErrSpawnTimeEnd, //孕育已结束
	enMainUICode_ErrErrProp,      //错误的道具，该道具不能升级法宝品质点
	enMainUICode_ErrUseNumLimit,             //超出该道具可以使用次数的限制
	enMainUICode_ErrUpgradeFail,    //提升失败
	enMainUICode_ErrNotGodSword,    //该物品不是仙剑
	enMainUICode_ErrLevelLimit,     //级别已达最大，无法升级
	enMainUICode_ErrNotNimbus,         //仙剑灵气不足
	enMainUICode_ErrNotSecret,         //该仙剑没有剑诀
	enMainUICode_ErrLevelErr,           //层次错误，你还不能进入该层次的剑印世界
	enMainUICode_ErrSwordWorldCnfg,     //找不到剑印世界配置信息
	enMainUICode_ErrEnterLevelLimit,     //进入级别限制
	enMainUICode_ErrNotExistNpc,  //NPC不存在
	enMainUICode_ErrOrder,  //错误的击杀顺序
	enMainUICode_ErrBusy,                    //玩家正忙，不能战斗
	enMainUICode_ErrLevel,	//等级不足进副本
	enMainUICode_ErrNoStone,  //灵石不足
	enMainUICode_ErrGoods,	//材料不足

	enMainUICode_NotOpenCombat,				//帮派保卫战时间没到，未开启
	enMainUICode_ErrNumLimit,               //帮派保卫战进入次数限制

	enMainUICode_ErrLevelMode,				//您的等级不能进入该模式的帮派保卫战

	enMainUICode_NoSyndicate,				//您没有帮派

	enMainUICode_SpaceLess,					//背包空位已不足10格，不能进行快速攻打

	enMainUICode_NotFinished,				//只有通过1次副本后，才能使用快速副本功能

	enMainUICode_ErrVipLevel,				//您的VIP等级不足，不能开启快速打副本

	enMainUICode_ErrFuBenLv,				//该副本没有困难模式

	enMainUICode_NoFinishSingle,			//必须打通普通模式才能进入双人副本

	enMainUICode_FastCreateTeam,			//正在快速组队

	enMainUICode_NoFree,					//无法进入，队友必须在空闲状态

	enMainUICode_OnlyLeadCan,				//只有队长才能点击挑战副本

	enMainUICode_WaitSycProcess,			//等待进度同步

	enMainUICode_NoSycProgress,				//队友不同步进度，不能进副本

	enMainUICode_InTeamFuBen,				//正在组队副本中

	enMainUICode_NoCD,						//正在冷却时间中

	enMainUICode_NoOpen,					//还未开放，敬请等待

	enMainUICode_NoBossDie,					//BOSS已经死亡，请等待下一次玄天

	enMainUICode_NoCDTime,					//没有CD

	enMainUICode_ErrIsInTeamFuBen,			//正在组队副本中，不能进入单人副本

	enMainUICode_ErrNoTeamFuBen,			//该副本没有组队模式

	enMainUICode_ErrHaveProgress,			//副本有进度时无法快速组队

	enMainUICode_ErrLevelEnterXT,			//等级不足以进玄天

	enMainUICode_ErrNoLeaderReset,			//只有队长才能重置副本

	enMainUICode_ErrInDuoBao,				//无法进入，正在匹配夺宝战

	enMainUICode_NoFreeMe,					//无法进入，不在空闲状态

	enMainUICode_ErrHaveTeam,				//组队中，无法进入

	enMainUICode_ErrWaitTeam,				//等待组队中，无法操作

	enMainUICode_Err,						

}PACKED_ONE;

//消息头
struct SMainUIHeader : public AppPacketHeader
{
	SMainUIHeader(enMainUICmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_MainUI;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//副本进度
struct SFuBenProgress
{
	SFuBenProgress()
	{
		m_FuBenID = INVALID_FUBEN_ID;
		m_Level	  = 0;
		m_KillMonsterNum = 0;
		m_bCanOpenHardType = false;
	}

	TFuBenID m_FuBenID;  //副本ID
	UINT8    m_Level;   //级别
	UINT8    m_KillMonsterNum; //杀怪数量
	bool	 m_bCanOpenHardType;	//是否能开启困难模式
};

//打开副本应答
struct SC_MainUIOpenFuBen_Rsp
{
	SC_MainUIOpenFuBen_Rsp()
	{
		m_FreeEnterNum = 0;
		m_VipEnterNum = 0;
		m_MaxVipEnterNum = 0;
		m_CostStoneEnterNum = 0;
		m_SynWelfareEnterNum = 0;
		m_MaxSynWelfareEnterNum = 0;
		m_StartFuBenNum = 0;
	}

	UINT16             m_FreeEnterNum;			//今天免费还可进入次数
	UINT16			   m_VipEnterNum;			//今天VIP还可进入次数
	UINT16			   m_MaxVipEnterNum;		//VIP最大进入次数
	UINT16			   m_CostStoneEnterNum;		//支付灵石还可进入次数
	UINT16			   m_SynWelfareEnterNum;	//帮派福利还可进入次数
	UINT16			   m_MaxSynWelfareEnterNum;	//帮派福利最大可进入次数
	UINT16             m_StartFuBenNum;			//已开启的副本数量
//	SFuBenProgress    m_FuBenProgress[m_StartFuBenNum]; //副本进度
};

//重置副本
struct CS_MainUIReset_Req
{
	TFuBenID     m_FuBenID;  //副本ID
};

struct SC_MainUIReset_Rsp
{
	enMainUICode m_Result;
	TFuBenID     m_FuBenID;  //副本ID
};

//挑战副本模式
enum enChallengeMode
{
	enChallengeMode_Single = 0, //单人模式
	enChallengeMode_Team,		//双人模式
};

//挑战副本
struct CS_MainUIChallenge_Req
{
	TFuBenID     m_FuBenID;			//副本ID
};

struct SC_MainUIChallenge_Rsp
{
	enMainUICode m_Result;
	TFuBenID     m_FuBenID;  //副本ID
	UINT8        m_Level;   //从1开始
	UINT8		 m_bOpenHardType;		//是否开启困难模式
};

//普通副本自动快速打怪的副本ID
struct CS_CommonFuBenAutoCombat
{
	CS_CommonFuBenAutoCombat()
	{
		m_FuBenID = 0;
		m_bOpenHardType = 0;
	}

	TFuBenID	m_FuBenID;
	UINT8		m_bOpenHardType;		//是否开启困难模式
};

struct SC_CommonFuBenAutoCombat
{
	SC_CommonFuBenAutoCombat()
	{
		m_Result = enMainUICode_OK;
		m_PolyNimbus = 0;
		m_Num	 = 0;
	}

	enMainUICode	m_Result;
	INT32			m_PolyNimbus;			//聚灵气
	UINT16			m_Num;
//	AutoCombatNpc	m_AutoCombatNpc[m_Num];
};

struct AutoCombatNpc
{
	AutoCombatNpc()
	{
		m_Exp	  = 0;
		m_DropNum = 0;
	}

	char			m_NpcName[THING_NAME_LEN];
	INT32			m_Exp;					//经验
	UINT16			m_DropNum;				//掉落数量
//	DropGoodsInfo	m_GoodsInfo[m_DropNum];
};

struct DropGoodsInfo
{
	TGoodsID	m_GoodsID;
	UINT16		m_GoodsNum;
};


////////////////////////////////////////////////////////////////////////////////////

//孕育法宝
struct CS_SpawnTalismanGoods_Req
{
	UID   m_uidTalisman;
};

struct SC_SpawnTalismanGoods_Rsp
{
	enMainUICode  m_Result;
	UID   m_uidTalisman;
};


//领取法宝孕育物
struct CS_TakeTalismanGoods_Req
{
	UID   m_uidTalisman;
};

struct SC_TakeTalismanGoods_Rsp
{
	enMainUICode  m_Result;
	UID           m_uidTalisman;  //所需提取的法宝
	TGoodsID      m_gidGoods;     //获得的孕育物品
};


//提升品质点
struct CS_UpgradeTalismanQuality_Req
{
	UID         m_uidTalisman;
	TGoodsID    m_gidProp;   //使用的道
};

struct SC_UpgradeTalismanQuality_Rsp
{
	enMainUICode  m_Result;
};


//仙剑升级
struct CS_UpgradeSword_Req
{
	UID       m_uidGoods;  //仙剑
};

struct CS_UpgradeSword_Rsp
{
	enMainUICode  m_Result;
	UID       m_uidGoods;  //仙剑
};



//进入仙剑副本请求
struct CS_EnterGodSwordFuBen_Req
{
	UID         m_uidGodSword;  //仙剑
	UINT8       m_Level;  //层次，从1开始
};

//进入结果
struct SC_EnterGodSwordFuBen_Rsp
{
	enMainUICode  m_Result;
};

//挑战怪物
struct CS_GodSwordCombatNpc_Req
{
	UID          m_uidNpc;
	TFuBenID     m_FuBenID;  //副本ID
};

struct SC_GodSwordCombatNpc_Rsp
{
	enMainUICode  m_Result;
	bool          m_bWin;   //是否胜利
	bool          m_bOver;  //是否通关
	UINT8	      m_SecretLevel;    //剑诀等级提升到  	
};

//打开仙剑面板
struct SC_OpenGodSword_Rsp
{
	SC_OpenGodSword_Rsp() : m_GodSwordNimbus(0), m_MaxGodSwordNimbus(0),m_EnterGodSwordFuBenNum(0), m_MaxVipEnterNum(0)
	{
	}

	INT32	m_GodSwordNimbus;		 //当前仙剑灵气值
	INT32	m_MaxGodSwordNimbus;	 //仙剑灵气上限
	UINT8	m_EnterGodSwordFuBenNum; //进入仙剑副本次数
	UINT8	m_MaxVipEnterNum;		 //今天VIP可进入次数
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////



//同步防沉迷
struct SC_OnlineTime_Sync
{
	enIdentityStatus      m_IdentityStatus; //认证状态
	INT32                 m_OnlineTime;		//在线时间，单位:秒
};


//enMainUICmd_CS_Identity,  //通过了第三方验证
struct CS_IdentityPass
{
	enIdentityStatus      m_IdentityStatus; //认证状态
};

 //新手引导步骤 enGameFrameCmd_CS_Guide,     
struct CS_NewPlayerGuide
{
	//INT16       m_GuideIndex;  //新手引导步骤
	//char   m_szGuideContext[]; //上下文
};

//查看普通副本的掉落物品
struct CS_ViewCommonFuBenDrop
{
	TMapID			m_MapID;
	UINT8			m_bOpenHardType;	//是否开启困难模式
};

struct SC_ViewCommonFuBenDrop
{
	SC_ViewCommonFuBenDrop()
	{
		m_Num = 0;
	}

	UINT16			m_Num;
//	TGoodsID		m_GoodsID[m_Num];
};

//打开查看掉落物品界面
struct CS_OpenViewDrop
{
	TFuBenID		m_FuBenID;
};

struct SC_OpenViewDrop
{
	SC_OpenViewDrop()
	{	
		m_Num = 0;
		m_bCanOpenHardType = false;
	}

	bool			m_bCanOpenHardType;	//是否能开启困难模式
	UINT8			m_Num;
//	BossInfo		m_Boss[m_Num];
};

struct BossInfo
{
	TMapID			m_MapID;				//BOSS所在地图ID
	char			m_Name[THING_NAME_LEN];	//BOSS名称
};

//玩家的设置信息
struct SC_UserSet
{
	bool			m_bOpenAutoTakeRes;		//是否设置成自动收取灵石
};

//弹出是否同步副本进度框
struct SC_SycProcess
{
	SC_SycProcess()
	{
		MEM_ZERO(this);
	}
	TFuBenID	m_FuBenID;

	UINT8		m_MonsterNum;					//怪物总数

	UINT8		m_KillNum;						//我的杀敌数

	UINT8		m_MemKillNum;					//队友的杀敌数

	char		m_MemberName[THING_NAME_LEN];	//队友名字
};

//是否同步副本进度的返回
struct CS_SelectSycRet
{
	bool		m_bOk;			//是否选择OK
	TFuBenID	m_FuBenID;		//副本ID
};

//会向我和队友都发，说明选择
struct SC_SelectSycRet
{
	enMainUICode m_RetCode;
};

//打开玄天页面
enum enXTState VC_PACKED_ONE
{
	enXTState_NoOpen = 0,	//未开放
	enXTState_Open,			//开放中

}PACKED_ONE;

struct SC_OpenXuanTian
{
	SC_OpenXuanTian()
	{
		m_XTState = enXTState_NoOpen;
		m_OpenRemainTime = 0;
		m_Num = 0;
	}

	enXTState	m_XTState;				//玄天状态
	UINT32		m_OpenRemainTime;		//状态为未开放时，表示开放剩余时间，已开放时，表示离结束剩余时间
	UINT8		m_Num;
//	XTBossState	BossState[m_Num];		//BOSS信息
};

struct XTBossState
{
	UID			m_uidBoss;				//BOSS的UID
	UINT32		m_BossBloodUp;			//BOSS的气血上限
	UINT32		m_BossBlood;			//BOSS的当前气血
	char		m_BossName[THING_NAME_LEN];	//BOSS名字
};

//显示玄天战斗奖励
struct CS_ShowXuanTianForward
{
	UINT32		m_Rank;			//第几名，0表示显示最后一击的奖励
};

struct SC_ShowXuanTianForward
{
	SC_ShowXuanTianForward()
	{
		m_Num = 0;
	}
	UINT16		m_Num;
//	TGoodsID	m_GoodsID[m_Num];
};

//攻击玄天BOSS
struct CS_AttackXTBoss
{
	UID		m_uidBoss;
};

struct SC_AttackXTBoss
{
	SC_AttackXTBoss()
	{
		m_RetCode = enMainUICode_OK;
	}
	enMainUICode m_RetCode;
};

//显示玄天伤害排行
struct CS_ViewXTDamageRank
{
	UINT8 m_index;		//索引,从0开始
};

struct SC_ViewXTDamageRank
{
	UINT8		m_index;		//索引,从0开始
	INT32		m_Num;
//XTRankName	XTRankName[m_Num];
};

struct XTRankName
{
	char	m_Name[THING_NAME_LEN];		//名字
	INT32	m_DamageValue;				//伤害值
};

//显示最后一击
struct SC_ViewLastKill
{
	UINT8		m_Num;
//XTBossDie		Bos[m_Num];
};

struct XTBossDie
{
	char		m_BossName[THING_NAME_LEN]; //BOSS名字
	char		m_UserNmae[THING_NAME_LEN]; //玩家名字
	INT32		m_DamageValue;				//伤害值
	UINT8		m_UserLevel;				//玩家等级
};

//缩短CD时间
struct SC_LessCDTime
{
	SC_LessCDTime()
	{
		m_RetCode = enMainUICode_OK;
		m_CDTimeNum = 0;
	}

	enMainUICode m_RetCode;
	//m_RetCode为OK时，才有效
	INT32		 m_CDTimeNum;	//新的CD时间多少
};

//组队挑战副本
struct CS_TeamChallengeFuBen
{
	TFuBenID		m_FuBenID;
};


//弹出等待同步框
struct SC_PopWaitSyc
{
	bool	m_bOpen;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
