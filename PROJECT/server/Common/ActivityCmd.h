
#ifndef __XJCQ_GAMESRV_ACTIVITY_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_ACTIVITY_CMD_PROTOCOL_H__

#pragma pack(push,1)



enum enActivityCmd VC_PACKED_ONE
{
    enActivityCmd_OpenDaily = 0  ,        //打开日常

	enActivityCmd_OpenSignIn         ,   //打开签到
	enActivityCmd_SignIn         ,       //签到
	enActivityCmd_SignInAward,           //领取签到奖励
	
	enActivityCmd_OpenActivity       ,   //打开活动
	enActivityCmd_SC_UpdateActivity     ,   //更新活动状态
	enActivityCmd_ActivityAward,           //领取活动奖励(没用了，获得就领取)

	enActivityCmd_OnlineAwardNotic,     //在线奖励通知

	enActivityCmd_OnlineAward,         //领取在线奖励

	enActivityCmd_ChangeGift,        //兑换礼券 

	enActivityCmd_MultipExp,          //多倍经验

		
	enActivityCmd_Max,

}PACKED_ONE;

//活动
struct  ActivityHeader  : public AppPacketHeader
{
	ActivityHeader(enActivityCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Activity;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//活动结果码定义
enum enActivityRetCode VC_PACKED_ONE
{
	enActivityRetCode_Ok = 0,	
	enActivityRetCode_ErrSignInOnce, //一天只能签到一次
	enActivityRetCode_ErrHaveTakeAward, //该奖励当月已领取过了
	enActivityRetCode_ErrAwardID, //错误的奖励ID
	enActivityRetCode_ErrSignInNum, //签到次数没达到,不能领取奖励 
	enActivityRetCode_ErrNoSpace,//背包已满，请清理背包
	enActivityRetCode_ErrNotFinish, //活动目标没达成，不能领奖
	enActivityRetCode_ErrActivityEnd, //活动已结束
	enActivityRetCode_ErrNotTime,   //在线领奖时间未到
	enActivityRetCode_ErrHaveTakeActivityAward, //该活动奖励已领取过了

	enActivityRetCode_ErrTicketNo,   //错误的礼券号
	enActivityRetCode_ErrHaveChange, //该礼券已兑换过物品
	enActivityRetCode_ErrNoActive,   //礼券码未激活
	enActivityRetCode_ErrDeadtime,   //礼券已过有效期
	enActivityRetCode_ErrTaked,		 //您已兑换过同类礼券



} PACKED_ONE;


//打开日常enActivityCmd_OpenDaily


//日常ID
enum enDailyID VC_PACKED_ONE
{
	enDailyID_XiuLian = 0,	 //修炼
	enDailyID_Employ     ,   //招幕
	enDailyID_MagicBook,     //法术书
	enDailyID_GoldSword,     //仙剑
	enDailyID_BaoWeiZhan,    //保卫战
	enDailyID_FuBen,         //副本	
	enDailyID_GoldSwordWorld, //剑印世界
	enDailyID_DuoBao,        //夺宝
	enDailyID_XuanTian,		 //玄天
	enDailyID_TalismanWorld, //法宝世界
	enDailyID_SynWar,		 //帮战
	enDailyID_Honor,		 //荣誉
	enDailyID_Credit,		 //声望
	enDailyID_SignIn,		 //签到
	enDailyID_DailyTask,	 //日常任务


	enDailyID_Max,

} PACKED_ONE;


//活动信息
struct SDailyInfo
{
	enDailyID    m_DailyID;						   //ID
	INT32        m_RemainTimes;					   //余下时间或次数
	INT16        m_TotalTimes;					   //总次数
	INT32		 m_HonorCredit;					   //当天获得荣誉值或当周获得声望值
	INT32		 m_MaxHonorCredit;				   //荣誉或声望上限
	INT32		 m_SynRemainTime;				   //帮战开始剩余时间
	char		 m_MySynName[THING_NAME_LEN];	   //我的帮派名
	char		 m_EnemySynName[THING_NAME_LEN];   //帮战敌对帮派名
	UINT8		 m_Index;						   //序号
};

//打开日常应答
struct SC_ActivityOpenDaily_Rsp
{
       SDailyInfo  m_DailyInfo[enDailyID_Max]; 
};


//打开签到

//活动奖励类型
enum enActivityAwardType VC_PACKED_ONE
{
    enActivityAwardType_Ticket    = 0,  //礼券
	enActivityAwardType_GodStone,      //仙石
	enActivityAwardType_NimbusStone,   //灵石
	enActivityAwardType_Goods,         //物品
	enActivityAwardType_PolyNimbus,    //聚灵气
	enActivityAwardType_Credit,        //声望

	enActivityAwardType_Max,

} PACKED_ONE;

//奖励信息
struct SActivityAwardInfo
{
	SActivityAwardInfo()
	{
		m_AwardType = enActivityAwardType_Max;

		m_Value = 0;

		m_Pile = 0;
	}
	enActivityAwardType m_AwardType;
	INT32               m_Value;  //资源数量或物品ID
	UINT8               m_Pile; //叠加数
};

//签到奖励状态
enum enSignAwardState VC_PACKED_ONE
{
    enSignAwardState_CanTake    = 0,  //可以领
	enSignAwardState_NotCanTake,      //不可以领
	enSignAwardState_HaveTake,   //已领取

	enSignAwardState_Max,

} PACKED_ONE;

//签到奖励
struct SSignInAwardInfo
{
	UINT8       m_AwardID;           //奖励ID
	UINT8       m_SignInDayNum;      //领取该奖励需要达到的签到次数
	enSignAwardState     m_State;          //状态
	UINT8       m_AwardNum;          //奖励数量
	//SActivityAwardInfo m_AwardInfo; //奖励
};



//打开签到应答
struct SC_ActivityOpenSignIn_Rsp
{
	bool                m_bSignInCurDay;     //当天是否已签到
	UINT8               m_SignInNymOfMonth;  //当月签到次数
	UINT32				m_PolyNimbus;		 //聚灵气
	UINT8               m_SignInAwardNum;    //签到奖励数
	//SSignInAwardInfo  m_SignInAward[m_SignInAwardNum]; //奖励
};

//签到

//签到应答
struct SC_ActivitySignIn_Rsp
{
	enActivityRetCode m_Result; //如果签到成功，则当天签到奖励需要设置为可领取
	UINT8             m_SignInNymOfMonth;  //当月签到次数

	UINT8            m_CanTakeAwardID;       //新的，除了当天奖励外的可以领奖的奖励ID,
};

//领取奖励请求
struct CS_ActivityTakeSignInAward_Req
{
	UINT8       m_AwardID;           //奖励ID
};

//领取签到奖励应答
struct SC_ActivityTakeSignInAward_Rsp
{
	enActivityRetCode     m_Result;       //结果
	UINT8                 m_AwardID;     //奖励ID
};



//打开活动应答
struct SActivityInfo
{
	SActivityInfo()
	{
		MEM_ZERO(this);
	}

	UINT16        m_ActivityID;  //活动ID
	UINT8         m_bFinished;   //是否已完成
	UINT8         m_bTakeAward;  //是否已领奖
	UINT32        m_ActivityProgress;  //进度

	INT16         m_Order;  //排序号
	
	UINT8         m_AwardNum;    //奖励数量
	//SActivityAwardInfo  m_AwardInfo[m_AwardNum] ; //奖励
	//char          m_szActivityName[]; //活动名称
	//char          m_szActivityRole[]; //活动规则
	//char          m_szActiivityAward; //活动奖励
	
};

struct SC_ActivityOpenActivity_Rsp
{
	UINT16          m_ActivityNum;  //活动数量
	//SActivityInfo m_Activity[m_ActivityNum]; //活动
};

//领取活动奖励请求
struct CS_ActivityTakeActivityAward_Req
{
	UINT16        m_ActivityID;  //活动ID
};

//应答
struct SC_ActivityTakeActivityAward_Rsp
{
	enActivityRetCode     m_Result;       //结果
	UINT16                m_ActivityID;  //活动ID
};

//更新活动状态
struct SC_ActivityUpdateActivity
{
	UINT16        m_ActivityID;  //活动ID
	UINT8         m_bFinished;   //是否已完成	
	UINT32        m_ActivityProgress;  //进度
};


//在线奖励通知
struct SC_ActivityOnlineAwardNotic
{
	INT32    m_RemainTime;  //倒计时,单位:秒,如为负数，表示没有奖项了
	UINT8                 m_AwardNum; //下级奖励数量
	//SActivityAwardInfo  m_AwardInfo[m_AwardNum] ; //下级奖励信息
};

//领取在线奖励


//应答
struct SC_ActivityOnlineAward_Rsp
{
	enActivityRetCode     m_Result;       //结果

	UINT8                 m_AwardNum; //奖励数量
	//SActivityAwardInfo  m_AwardInfo[m_AwardNum] ; //奖励
};

//兑换礼券
//enActivityCmd_ChangeGift,      
struct CS_ActivityChangeGift_Req
{
       char              m_szTicketNo[MAX_TICKET_NO_LEN];      //礼券号码
};


struct SC_ActivityChangeGift_Rsp
{
	enActivityRetCode    m_Result;       //结果
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //多倍经验
//enActivityCmd_MultipExp
struct SC_ActivityMultipExp
{
	UINT32               m_MultipExpEndTime;   //多倍经验结束时间
	//char               m_szContext[];     //需要显示的内存,以\0结束
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif
