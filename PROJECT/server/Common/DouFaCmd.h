#ifndef __XJCQ_GAMESRV_DOUFA_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_DOUFA_CMD_PROTOCOL_H__

#pragma pack(push,1)


#define MAX_DOUFA_ACTOR_NUM  3    //斗法候选角色数量




enum enDouFaCmd VC_PACKED_ONE
{
	///////////////////////////////////
	//没用了
    enDouFaCmd_OpenDuoBao22,		//打开夺宝标签 
	///////////////////////////////////
	enDouFaCmd_OpenDouFa,			//打开斗法标签
	enDouFaCmd_OpenQieCuo,			//打开切磋标签
	enDouFaCmd_DouFaCombat,			//斗法战斗
	enDouFaCmd_SC_DouFaOver,		//斗法结束,通知客户端
	enDouFaCmd_SC_EnterQieCuo,		//进入切磋场景
	enDouFaCmd_SC_EnterDouFa,		//进入斗法场景
	enDouFaCmd_SynQieCuo,			//同步切磋数据
	enDouFaCmd_QieCuoBattle,		//切磋战斗
	enDouFaCmd_Flush_DouFa,			//刷新斗法的对手
	enDouFaCmd_Flush_QieCuo,		//刷新切磋的对手
	enDouFaCmd_SynDouFa,			//同步斗法数据

	enDouFaCmd_FlushMyChallenge,	//刷新我的挑战信息
	enDouFaCmd_ViewLvGroup,			//查看等级组排行
	enDouFaCmd_Challenge,			//挑战玩家
	enDouFaCmd_SC_EnterChallenge,	//进入挑战场景
	enDouFaCmd_ChallengeForward,	//查看挑战奖励
	
	//夺宝
	enDouFaCmd_OpenDuoBao,			//打开夺宝
	enDouFaCmd_SC_PopReady,			//弹出战斗准备框
	enDouFaCmd_ReadySelect,			//点击战斗准备框按钮
	enDouFaCmd_JoinDuoBao,			//加入夺宝战
	enDouFaCmd_ViewRetCode,			//显示结果码
	
	enDouFaCmd_FlushChallengeRecord, //刷新挑战回放信息
	enDouFaCmd_ViewChallengeRecord,	 //查看挑战回放

	enDouFaCmd_SC_WaitSelect,		//等待对方确认框
	enDouFaCmd_CancelDuoBao,		//取消夺宝战

	enDouFaCmd_CombatResult,		//夺宝战斗结果

	enDouFaCmd_BuyCombatNum,		//购买战斗次数
	

	enDouFaCmd_Max,

}PACKED_ONE;

//斗法消息
struct  DouFaHeader  : public AppPacketHeader
{
	DouFaHeader(enDouFaCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_DouFa;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//斗法结果码定义
enum enDouFaRetCode VC_PACKED_ONE
{
	enDouFaRetCode_Ok = 0,	
	enDouFaRetCode_NoUser,			//不存在此玩家
	enDouFaRetCode_InDouFa,			//此玩家已在斗法玩家列表中，不能再参加切磋
	enDouFaRetCode_ErrMaxChallenge,	//已达到当日挑战次数上限，无法继续挑战
	enDouFaRetCode_ErrRankChange,	//名次已发生变化，请再次尝试
	enDouFaRetCode_ErrCDTimer,		//冷却时间未到，请稍候再试
	enDouFaRetCode_ErrFinished,		//已跟该对手战斗过，不可再战斗
	enDouFaRetCode_ErrNotOver,		//还未决出胜负，无法更新
	enDouFaRetCode_ErrNoEnemy,		//玩家不足，不能进行斗法

	enDouFaRetCode_NotOpenDuoBao,	//夺宝未开启
	enDouFaRetCode_ErrInDuoBao,		//已经在夺宝中，不能再次加入
	enDouFaRetCode_ErrMeberInDuoBao,//队友已经在夺宝中，不能再次加入
	enDouFaRetCode_ErrLevel,		//等级不足
	enDouFaRetCode_ErrNotLeader,	//只有队长能选择加入
	enDouFaRetCode_ErrRun,			//正受到逃跑惩罚，不能加入夺宝
	enDouFaRetCode_ErrMemRun,		//队友正受到逃跑惩罚，不能加入夺宝

	enDouFaRetCode_ErrJoinDuoBao,	//加入夺宝失败

	enDouFaRetCode_EnemyQuit,		//对方退出战斗，战斗胜利
	enDouFaRetCode_ErrNoRecord,		//没有回放记录

	enDouFaRetCode_ErrNeedLeader,	//只有队长能选择取消
	enDouFaRetCode_ErrMemberLevel,	//队友等级不足

	enDouFaRetCode_ErrInDuoBao2,		//无法进入，正在匹配夺宝战

	enDouFaRetCode_ErrInTeamFuBen,	//无法进入，正在双人副本中

	enDouFaRetCode_ErrHaveTeam,		//组队中，无法进入

	enDouFaRetCode_ErrWaitTeam,		//等待组队中，无法操作

	enDouFaRetCode_ErrNoGodStone,	//仙石不足
	enDouFaRetCode_Err,

} PACKED_ONE;

//应答
struct SC_DouFaOpenDuoBao_Rsp
{
	enDouFaRetCode    m_Result;

	INT32             m_GetHonorOfDay;    //今天获得的荣誉

	INT32             m_GetHonorUpOfDay;  //今天可获得荣誉上限

	UINT8             m_MaxJoinNumOfDay; //每天参加每种类型的夺宝最大次数

	UINT8            m_DuoBaoNum;  //夺宝活动数量

	//SDuoBaoInfo      m_DuoBaoInfo[m_DuoBaoNum];  //每个活动已参加次数

};


//打开斗法标签

//应答
struct SC_DouFaOpenDouFa_Rsp
{
	enDouFaRetCode    m_Result;
	//OK则同步
};

//同步斗法数据
struct	SC_SynDouFa
{
	INT32             m_GetHonorOfDay;    //今天获得的荣誉
	INT32			  m_MaxGetHonorOfDay;	//今天最多可获得的荣誉
	UINT8			  m_Num;
//	DouFaUser		  userdata[m_Num];
};

struct DouFaUser
{
	UID			m_uidUser;
	char		m_Name[THING_NAME_LEN];
	UINT16		m_Facade;	//外观
	bool		m_bFinish;
	bool		m_bWin;
	UINT8		m_VipLevel;
};

//刷新斗法
struct SC_Flush_DouFa_Rsp
{
	SC_Flush_DouFa_Rsp(){
		m_Result = enDouFaRetCode_Ok;
	}
	enDouFaRetCode    m_Result;
	//OK则同步
};


//刷新切磋
struct SC_FlushQieCuo_Rsp
{
	SC_FlushQieCuo_Rsp(){
		m_Result = enDouFaRetCode_Ok;
	}

	enDouFaRetCode    m_Result;
};

//同步切磋数据
struct SC_SynQieCuo
{
	INT32			  m_Num;
//  QieCuoUserData	  UserData[m_Num]		
};


//切搓的玩家数据
struct QieCuoUserData
{
	QieCuoUserData(){
		memset(this, 0, sizeof(*this));
	}


	UID							m_UidUser;					//玩家的UID
	char						m_Name[THING_NAME_LEN];		//名字
	UINT8						m_Level;					//等级
	UINT32						m_Layer;					//境界
	TTitleID					m_TitleID;					//称号ID
	bool						m_bFinish;					//是否已经切搓过
	bool						m_bWin;						//是否赢
	char						m_SynName[THING_NAME_LEN];	//帮派名字
	UINT8						m_vipLevel;					//vip等级
};

//切磋战斗
struct CS_QieCuoBattle
{
	UID				  m_uidEnemy;
};

struct SC_QieCuoBattle
{
	enDouFaRetCode    m_Result;
};



//斗法战斗
struct CS_DouFaDouFa_Req
{
      UID   m_uidUser;
};


struct SC_DouFaDouFa_Rsp
{
	 enDouFaRetCode    m_Result;
//Ok的话，发下面的数据
//	 DouFaCombatData   m_CombatData;
};

struct DouFaCombatData
{
	 UID			   m_uidEnemy;
	 INT32             m_Credit; //获得声望
//	 char			   m_SceneName[THING_MAP_NAME_LEN];//当前战斗地点名字

//	 //CombatCombatData m_CombatData;
};

//斗法全部结束
struct SC_DouFaOver_Rsp
{
	bool			   m_bWin;				//是否胜利
	INT32			   m_Honor;				//得到的荣誉值
	INT32			   m_GetHonorToday;		//今天获得的荣誉值
	INT32			   m_Max_GetHonorToday;	//今天可获得的最高荣誉值
};

//进入切磋场景
struct SC_EnterQieCuoScene
{
	enDouFaRetCode    m_Result;
};

//进入斗法场景
struct SC_EnterDouFaScene
{
	enDouFaRetCode    m_Result;
};

//加入某个夺宝活动
struct CS_DouFaJoinDuoBao_Req
{
	UINT8    m_DuoBaoID;  //ID
};


struct SC_DouFaJoinDuoBao_Rsp
{
	enDouFaRetCode    m_Result;
	UINT8    m_DuoBaoID;  //ID
};


//退出某个夺宝活动
struct CS_DouFaUnjoinDuoBao_Req
{
	UINT8    m_DuoBaoID;  //ID
};


struct SC_DouFaUnjoinDuoBao_Rsp
{
	enDouFaRetCode    m_Result;
	UINT8             m_DuoBaoID;  //ID
};


//查看战报
struct CS_DouFaViewZhanBao_Req
{
	UINT8             m_DuoBaoID;  //ID
};

//应答
struct SC_DouFaViewZhanBao_Rsp
{
	enDouFaRetCode    m_Result;
	//SZhangBaoData   m_ZhangBaoData;
};

//参赛人员信息及名次
struct SPlayerInfoAndRanking
{
	UID          m_uidActor;     //UID
	char         m_szActorName[THING_NAME_LEN];  //玩家名
	UINT8        m_Level;                        //等级
	UINT8        m_Sex;                          //性别
	UINT16        m_FacadeID;                     //外观
	UINT8        m_Ranking;                      //最终名次
	UINT8        m_KillEnemyNum;                 //杀敌数量
	UINT16       m_GetHonor;                     //获得荣誉
};

//等级区间段
enum enDuoBaoCombatLevelRange VC_PACKED_ONE
{
	enDuoBaoCombatLevelRange_20_29 = 0,	//20到29级段
	enDuoBaoCombatLevelRange_30_39 = 0,	//30到39级段
	enDuoBaoCombatLevelRange_40_49 = 0,	//40到49级段
	enDuoBaoCombatLevelRange_50_55 = 0,	//50到55级段

	enDuoBaoCombatLevelRange_Max,
} PACKED_ONE;

//对阵信息
struct SCombatData
{
	UINT16              m_CombatID;     //战斗场次
	UINT8               m_FirstIndex;   //第一个战斗者在战报玩家列表中的索引
	UINT8               m_SecondIndex;   //第二个战斗者在战报玩家列表中的索引
	bool                m_bFirstWin;         //战斗结果，是否第一个玩家胜利
	INT16               m_ActionNum;    //指令数量
	//SCombatAction     m_CombatAction[m_ActionNum];  //后面跟一系列的战斗指令
};

//战报数据
struct SZhangBaoData
{
	UINT32                  m_ZhangBaoID;  //战报ID

	UINT8                   m_DuoBaoID;  //夺宝战ID

	UINT8                   m_Level;     //参赛级别

	INT32                   m_AverageCredit;   //平均声望

	UINT8                   m_PlayerNum;  //参赛人数

	UINT8                   m_CombatNum;  //战斗场数

	//SPlayerInfoAndRanking m_PlayerInfoAndRanking [m_PlayerNum];  //参赛人员
	//SCombatData           m_CombatData[m_CombatNum];

};


//查看参赛人员名单

//参赛人员信息
struct SPlayerInfo
{
	char         m_szActorName[THING_NAME_LEN];  //玩家名
	UINT8        m_Level;                        //等级
	UINT8        m_Sex;                          //性别
	char		 m_szSynName[THING_NAME_LEN];    //帮派名
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//挑战

//等级组
enum enLevelGroup VC_PACKED_ONE
{
	enLevelGroup_30_34 = 0,			//等级30到34的组
	enLevelGroup_35_39,				//等级35到39的组
	enLevelGroup_40_44,				//等级40到44的组
	enLevelGroup_45_49,				//等级45到49的组
	enLevelGroup_50_54,				//等级50到54的组
	enLevelGroup_55_60,				//等级55到60的组

	enLevelGroup_Max,
}PACKED_ONE;

//刷新我的挑战信息
struct SC_FlushMyChallenge
{
	INT32		m_MainRank;			//我的总排行
	INT32		m_LvGroupRank;		//等级组排行
	UINT16		m_ChallengeNum;		//今天剩余次数
	INT32		m_GetHonorToday;	//今天获得荣誉
	INT32		m_MaxGetHonorToday;	//今天最多可获得荣誉
	UINT16		m_MaxChallengeNum;	//今天最多可挑战次数
	UINT8		m_NumEnemy;			//发送给客户端的敌人数量
//	EnemyInfo	Enemy[m_NumEnemy];
};

//刷新我的挑战回放信息
struct SC_FlushMyChallengeRecord
{
	UINT8		m_Num;							//信息条数
  //MyChallengeRecordInfo	RecordInfo			//我的挑战回放信息
};


//我的挑战回放信息
struct MyChallengeRecordInfo
{
	char		m_EnemyName[THING_NAME_LEN];	//对手名字
	UINT8		m_EnemyLevel;					//对手等级
	bool		m_bWin;							//挑战胜负
	bool		m_bActive;						//是否主动挑战
	UINT8		m_VipLevel;						//vip等级
	UINT32		m_RecordBufIndex;				//挑战回放场次（第几场）
	UINT16		m_EnemyFacade;					//对手外观

};

struct EnemyInfo
{
	INT32		m_MainRank;				//对手总排行
	UID			m_UidEnemy;				//对手的UID
	char		m_Name[THING_NAME_LEN];	//对手名字
	UINT8		m_Level;				//对手等级
	UINT8		m_vipLevel;				//对手VIP等级
};

//查看等级组排行
struct CS_ViewLvGroup
{
	enLevelGroup m_lvGroup;
	INT32		 m_BeginPos;
	INT32		 m_Num;
};

struct SC_ViewLvGroup
{
	UINT8		m_Num;		//发的数量
//	LvGroupUserInfo	UserInfo[m_Num];
};

struct LvGroupUserInfo
{
	INT32		m_LvGroupRank;			//等级组排行
	UID			m_UidUser;				//玩家UID
	char		m_Name[THING_NAME_LEN];	//玩家名字
	UINT8		m_Level;				//对手等级
	UINT8		m_vipLevel;				//对手VIP等级
	INT32		m_DuoBaoLevel;			//夺宝等级
	UINT8		m_Layer;				//境界
	char		m_SynName[THING_NAME_LEN];		//帮派名字
};

//挑战玩家
struct CS_Challenge
{
	UID			m_uidEnemy;				//挑战的对手UID
};

struct SC_Challenge
{
	SC_Challenge()
	{
		m_Result = enDouFaRetCode_Ok;
	}

	enDouFaRetCode    m_Result;
//如果m_Result是OK，则发下面的
//	ChallengeData	  Data;
};
struct ChallengeData
{
	ChallengeData()
	{
		m_GetHonor = 0;
		m_NowHonor = 0;
	}

	INT32		m_GetHonor;				//获得的荣誉
	INT32		m_NowHonor;				//今天获得荣誉
//	char		m_SceneName[THING_MAP_NAME_LEN];//当前战斗地点名字

	//CombatCombatData m_CombatData;
};

//进入挑战场景
struct SC_EnterChallengeScene
{
	enDouFaRetCode    m_Result;
};

//查看挑战奖励
struct CS_ChallengeForward
{
	enLevelGroup m_lvGroup;
	UINT8		 m_Rank;
};

struct SC_ChallengeForward
{
	SC_ChallengeForward()
	{
		m_Num = 0;
	}
	INT8		m_Num;
//	GoodsForward	goods[m_Num];
};

struct GoodsForward
{
	TGoodsID	m_GoodsID;
	INT32		m_GoodsNum;
};

////////////////////////////////////////////////////////////////////////////////////////////////
//夺宝

//打开夺宝
struct SC_OpenDuoBao
{
	UINT32		m_MaxCredit;		//今日获得声望上限
	UINT32		m_GetCredit;		//今日获得声望
	UINT32		m_RunRemainTime;	//逃跑惩罚剩余时间
	bool		m_bOpen;			//是否开启
};

//弹出战斗准备框
struct SC_PopReady
{
	char		m_szName[THING_NAME_LEN];	//对手名字
};

//点击战斗准备框按钮
struct CS_ReadySelect
{
	bool		m_bEnterCombat;			//是否进入战斗
};

struct SC_ReadySelect
{
	SC_ReadySelect()
	{
		m_Result = enDouFaRetCode_Ok;
	}

	enDouFaRetCode    m_Result;
};

//加入夺宝战
struct SC_JoinDuoBao
{
	SC_JoinDuoBao()
	{
		m_Result = enDouFaRetCode_Ok;
	}
	enDouFaRetCode    m_Result;
};

//显示结果码
struct SC_ViewRetCode
{
	enDouFaRetCode    m_Result;
};

//等待对方确认框
struct SC_WaitSelect
{
	bool	m_bOpen;	//true为打开，false为关闭
};

//取消夺宝战
struct SC_CancelDuoBao
{
	SC_CancelDuoBao()
	{
		m_Result = enDouFaRetCode_Ok;
	}
	enDouFaRetCode    m_Result;
};

//夺宝战斗结果
struct SC_CombatResult
{
	SC_CombatResult()
	{
		m_Num = 0;
	}
	UINT32				m_Num;
//	DuoBaoCombatResult	m_Result[m_Num];

};

struct DuoBaoCombatResult
{
	char	m_szName[THING_NAME_LEN];
	UINT8	m_Level;
	INT32	m_CombatAbility;
	INT32	m_AddDuoBaoLevel;
	UINT8	m_NenLiType;		//能力
};

//enNenLiType_PuTong = 0,		//普通
//enNenLiType_LiangHao,		//良好
//enNenLiType_YouZhi,			//优质
//enNenLiType_QiCai,			//奇材
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//查看挑战回放
struct CS_ViewChallengeRecord
{
	UINT32	m_Rank;	//场次
};

//查看挑战回放
struct SC_ViewChallengeRecord
{
	SC_ViewChallengeRecord(){
		m_Result = enDouFaRetCode_Ok;
	}
	enDouFaRetCode    m_Result;
  //ChallengeRecordBuf	RecordBuf;	//回放buf
};

struct ChallengeRecordBuf
{
	int								m_BufLen;			//buf长度
	TBinArray<sizeof(OBuffer4k)>	m_ChallengeRecord;	//回放buf
	
};

//购买战斗次数
struct SC_BuyChallengeNum
{
	SC_BuyChallengeNum()
	{
		m_Result = enDouFaRetCode_Ok;
		m_MaxCombatNum = 0;
		m_CanCombatNum = 0;
	}
	enDouFaRetCode	m_Result;

	UINT16			m_MaxCombatNum;		//当前最大可战斗次数
	UINT16			m_CanCombatNum;		//还可战斗次数
};


#pragma pack(pop)



#endif
