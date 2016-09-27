#ifndef __TALISMANGAMEPROTOCOL_H__
#define __TALISMANGAMEPROTOCOL_H__

#include "ProtocolHeader.h"


#pragma pack(push,1)

//法宝世界


//消息
enum enTalismanWorldCmd VC_PACKED_ONE
{		
	enTalismanWorldCmd_Open = 0,			//打开法宝世界
	enTalismanWorldCmd_EnterTW,				//进入法宝世界
	enTalismanWorldCmd_Combat,				//战斗
	enTalismanWorldCmd_ChangePhoto,			//通知更改图片
	enTalismanWorldCmd_SC_PopWaitCombat,	//弹出等待战斗框
	enTalismanWorldCmd_CancelWait,			//取消等待
	enTalismanWorldCmd_CancelReplace,		//取消替换图片

	enTalismanWorldCmd_GetTWInfo,			//客户端请求法宝世界信息

	enTalismanWorldCmd_SC_OccupatGetSoul,	//占领获得灵魄

	enTalismanWorldCmd_ViewRec,				//查看回放
	enTalismanWorldCmd_GetOccupationInfo,	//得到占领时被谁击败

	/***********下面几个消息没用了*******************************/
	enTalismanWorldCmd_Enter,    //进入法宝世界，开始游戏
	enTalismanWorldCmd_Leave,     //离开法宝世界
	enTalismanWorldCmd_Game,      //游戏包

	enTalismanWorldCmd_Max,
}PACKED_ONE;

//法宝世界，消息头
struct TalismanWorldHeader : public AppPacketHeader
{
	TalismanWorldHeader(enTalismanWorldCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_TalismanWorld;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};


//操作结果码
enum enTalismanWorldRetCode VC_PACKED_ONE
{
	enTalismanWorldRetCode_OK = 0,
	
	enTalismanWorldRetCode_ErrOccupation,			//正在占领中，不能再进法宝世界
	enTalismanWorldRetCode_ErrLevel,				//等级不足
	enTalismanWorldRetCode_WaitCombat,				//正在排队等待战斗
	enTalismanWorldRetCode_ErrOccupationNoCombat,	//正在占领中，不能再挑战
	enTalismanWorldRetCode_ErrNoEnterNum,			//进入次数已用完
	enTalismanWorldRetCode_ErrInDuoBao,				//无法进入，正在匹配夺宝战
	enTalismanWorldRetCode_ErrHaveTeam,				//组队中，无法进入
	enTalismanWorldRetCode_ErrWaitTeam,				//等待组队中，无法操作

	enTalismanWorldRetCode_ErrNoRec,				//没有回放记录

	/***************下面是没用的定义*********************/
	enTalismanWorldRetCode_ErrNotTalisman,  //该物品不是法宝
	enTalismanWorldRetCode_ErrNoTalismanWorld,  //该法宝没有法宝世界
	enTalismanWorldRetCode_ErrNotInTalismanWorld, //不在法宝世界内

	enTalismanWorldRetCode_Max,
}PACKED_ONE;

//打开法宝世界
struct SC_OpenTalismanWorld
{
	SC_OpenTalismanWorld()
	{
		MEM_ZERO(this);
	}
	UINT8	m_LeftEnterNum;					//剩余进入次数
	UINT8	m_VipLeftEnterNum;				//VIP剩余进入次数
	UINT8	m_VipTotalEnterNum;				//VIP最多允许进入次数
	UINT32	m_OccupatLeftTime;				//占领剩余时间
	UINT32	m_GetGhostSoul;					//得到的灵魄数量
	char	m_PlaceName[THING_NAME_LEN];	//占领地


};

//进入法宝世界
struct CS_EnterTW
{
	UINT8	m_Floor;		//层数
};

struct SC_EnterTW
{
	SC_EnterTW()
	{
		m_RetCod			= enTalismanWorldRetCode_OK;
		m_Level				= 0;
		m_Floor				= 0;
		m_TotalFloor		= 0;
	}

	enTalismanWorldRetCode  m_RetCod;
	UINT8					m_Level;			//级别
	UINT8					m_Floor;			//层数
	UINT8					m_TotalFloor;		//总层数
};

struct ReplaceInfo
{
	UID		m_uidNpc;
	UID		m_uidUser;
	UINT16	m_Facade;
	char	m_Name[THING_NAME_LEN];
	UINT32	m_BeginOccupatTime;
};

//战斗
struct CS_TWCombat
{
	UID		uidEnemy;
	UINT8	m_Level;	//级别
	UINT8	m_Floor;	//层数
};

struct SC_TWCombat
{
	SC_TWCombat()
	{
		m_RetCod = enTalismanWorldRetCode_OK;
	}
	enTalismanWorldRetCode m_RetCod;
};

//通知更改图片
struct SC_ChangePhoto
{
	INT32					m_Num;
//	ReplaceInfo m_ReplaceInfo[m_Num];
};

//弹出等待战斗框
struct SC_PopWaitCombat
{
	INT32		m_Pos;		//队列第几位
};

//取消等待
struct SC_CancelWait
{
	SC_CancelWait()
	{
		m_RetCod = enTalismanWorldRetCode_OK;
	}
	enTalismanWorldRetCode m_RetCod;
};

//取消替换图片
struct SC_CancelReplace
{
	UID uidMonster;
};

//客户端请求法宝世界信息
struct CS_GetTWInfo
{
	UINT8 Level;
	UINT8 Floor;
};

struct SC_GetTWInfo
{
	UINT8					m_leftEnterNum;		//剩余进入次数
	UINT8					m_VipLeftEnterNum;	//VIP剩余进入次数
	UINT8					m_VipTotalEnterNum;	//VIP最多允许进入次数
//还会发同步替换消息
};

//占领获得灵魄
struct SC_OccupatGetSoul
{
	INT32			m_GetGhostSoul;		//获得的灵魄
};


/********************下面的没用了********************************************************/
//进入法宝世界
struct CS_TalismanWorldEnter_Req
{
	UID   m_uidTalisman;  //法宝
	UID   m_uidActor[MAX_TALISMAN_GAME_ACTOR_NUM];   //参加游戏的角色
};

//进入法宝世界应答
struct SC_TalismanWorldEnter_Rsp
{
	enTalismanWorldRetCode  m_Result;
	UINT32                  m_GameID; //游戏ID，后面需要用到该标识
};


//离开法宝世界
struct CS_TalismanWorldLeave_Req
{
	UINT32                  m_GameID; //游戏ID，
};


//离开法宝世界
struct SC_TalismanWorldLeave_Rsp
{
	enTalismanWorldRetCode  m_Result;
};


//游戏消息
struct CSC_TalismanWorldGame
{
	UINT32        m_GameID; //游戏ID，后面需要用到该标识
	UINT8         m_SubCmd; //游戏子命令字
};



//修炼游戏消息
enum enGameXiuLianCmd VC_PACKED_ONE
{		
	enGameXiuLianCmd_InitClient =0,     //初始化游戏客户端
	enGameXiuLianCmd_Start,             //游戏开始
	enGameXiuLianCmd_BornAirMass,       //产生气团
	enGameXiuLianCmd_Hit,               //打击
	enGameXiuLianCmd_EndPoint,           //气团到达终点
	enGameXiuLianCmd_Over,              //游戏结束
	enGameXiuLianCmd_Max,
}PACKED_ONE;
//初始化

struct SC_XiuLianGame_Init
{
	UINT32               m_TotalGameTime;  //游戏总时长(单位:秒)
	UINT16               m_StartAfterTime; //游戏在多少秒之后正式开始
	UINT16               m_WhiteAirMassScore; //白色气团代表的分值
	UINT16               m_BlackAirMassScore; //黑色气团代表的分值
};


//游戏正式开始


//气团类型
enum enAirMassType VC_PACKED_ONE
{		
	enAirMassType_White =0,     //白色
	enAirMassType_Black,             //黑色
	enAirMassType_Max,
}PACKED_ONE;

//产生气团
struct SC_XiuLianGame_BornAirMass
{
	UINT32              m_AirMassID;  //气图标识
	enAirMassType       m_AirMassType; //气团类型    
	UINT64              m_EndTime;  //存活到期时间
	UINT8               m_BornAddr;    //出生地(0~9号门)
};


//击打气团
struct CS_XiuLianGame_HitAirMass_Req
{
	UINT32              m_AirMassID;  //气图标识
};

//击打应答
struct CS_XiuLianGame_HitAirMass_Rsp
{
	UINT32              m_AirMassID;  //气图标识
};

//气团到达终点
struct CSC_XiuLianGame_EndPoint
{
	UINT32              m_AirMassID;  //气图标识
};

//游戏结束
struct SC_XiuLianGame_Over
{
	UINT32              m_TotalNimbus;           //共获得灵气,三个角色平分
	UINT16              m_TotalWhiteAirMassNum;  //白色气团总数
	UINT16              m_HitWhiteAirMassNum;    //击打白气团数
	UINT16              m_TotalBlackAirMassNum;  //黑色气团总数
	UINT16              m_HitBlackAirMassNum;    //击打黑色气团数
	UINT8               m_FinishLevel;           //完成级别
	UINT16              m_AdventureAwardID;      //奇遇奖励

};

////////////////////////////////////////////////////////////////////////////////////////////////////
//清理土地游戏消息

enum enGameQingLiCmd VC_PACKED_ONE
{
	enGameQingLiCmd_InitClient =0,     //初始化游戏客户端
	enGameQingLiCmd_Start,             //游戏开始
	enGameQingLiCmd_BornDiShu,		   //产生地鼠
	enGameQingLiCmd_Hit,               //打击
	enGameQingLiCmd_DelDiShu,		   //删除地鼠
	enGameQingLiCmd_Over,              //游戏结束

	enGameQingLiCmd_Max

}PACKED_ONE;

//清理土地地鼠类型
enum enQingLiDiShuType VC_PACKED_ONE
{
	enQingLiDiShuType_DiShuGuai = 0,	//地鼠怪
	enQingLiDiShuType_DiShuJing,		//地鼠精
	enQingLiDiShuType_ShuJing,			//树精

	enQingLiDiShuType_Max,
}PACKED_ONE;

//初始化
struct SC_QingLiGame_Init
{
	UINT16				m_GameTotalTime;			//时间总时长
	UINT8				m_StartAfterTime;			//游戏倒数几秒开始
};

//出现地鼠
struct SC_QingLiGame_BornDiShu
{
	enQingLiDiShuType   m_QingLiDiShuType;			//出现的地鼠类型   
	UINT32				m_DiShuID;					//出现的地鼠ID
	UINT8				m_Pos;						//击打位置(1-90,屏幕从左到右,从上到下分为90个可击打位置)
};

//击打怪物
struct CS_QingLiGame_HitDiShu_Req
{
	enQingLiDiShuType   m_QingLiDiShuType;			//击打的要地鼠类型
	UINT8				m_HitPos;					//击打位置(1-90)
	UINT32				m_DiShuID;					//地鼠ID
};

struct SC_QingLiGame_HitDiShu_Rsp
{
	SC_QingLiGame_HitDiShu_Rsp() : m_DiShuID(0), m_bHit(false), m_GetExpNum(0){}
	UINT32		m_DiShuID;		//地鼠ID
	bool		m_bHit;			//是否打中
	UINT16		m_GetExpNum;	//获得经验值
};

//删除地鼠
struct SC_QingLiGame_DeleteDiShu
{
	UINT32		m_DiShuID;		//地鼠ID
};

//游戏结束
struct SC_QingLiGame_Over
{
	UINT32              m_TotalExp;				 //共获得经验,三个角色平分
	UINT16				m_TotalDiShuGuai;		 //地鼠怪总数
	UINT16              m_TotalHitDiShuGuai;	 //击打地鼠怪总数
	UINT16              m_TotalDiShuJing;		 //地鼠精总数
	UINT16				m_TotalHitDiShuJing;	 //击打地鼠精总数
	UINT16				m_TotalShuJing;			 //树精总数
	UINT16				m_TotalHitShuJing;		 //击打树精总数
	UINT8               m_FinishLevel;           //完成级别
	UINT16              m_AdventureAwardID;      //奇遇奖励
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//寻宝游戏
enum enGameXunBaoCmd VC_PACKED_ONE
{
	enGameXunBaoCmd_InitClient =0,     //初始化游戏客户端
	enGameXunBaoCmd_Start,             //游戏开始
	enGameXunBaoCmd_Detect,            //探测
	enGameXunBaoCmd_Over,              //游戏结束

	enGameXunBaoCmd_Max

}PACKED_ONE;


//指示灯color
enum enIndicatorColor VC_PACKED_ONE
{
	enIndicatorColor_Black = 0,                    //黑色
	enIndicatorColor_Red ,                  //红色
	enIndicatorColor_Yellow,                   //黄色
	enIndicatorColor_Blue,                     //绿色
	
	enIndicatorColor_Max

}PACKED_ONE;

//初始化enGameXunBaoCmd_InitClient
struct SC_XunBaoGame_Init
{
	UINT16				m_GameTotalTime;			//时间总时长
	UINT8				m_StartAfterTime;			//游戏倒数几秒开始
	UINT16              m_XDetectNum;               //X坐标探测点数
	UINT16              m_YDetectNum;               //Y坐标探测点数
	UINT16              m_TotalDetectNum;         //允许探测次数
	UINT16              m_ColorRange[enIndicatorColor_Max] ;            //指示灯color代表附近多少格范围内有宝物，为零表示附近没有宝物

};


//探测enGameXunBaoCmd_Detect
struct CS_XunBaoGame_Detect_Req
{
	UINT16             m_DetectPoint;  //探测点,从零开始编号
};



//探测结果
struct SC_XunBaoGame_Detect_Rsp
{
	SC_XunBaoGame_Detect_Rsp()
	{
		MEM_ZERO(this);
	}
	UINT16             m_DetectPoint;          //探测点,从零开始编号
	TGoodsID           m_GoodsID;              //获得的宝物 如为零表示没宝物
	UINT8              m_GoodsNum;             //获得宝物数量
	UINT8              m_IndicatorColor;      //指示灯color,取值看enIndicatorColor

};

//游戏结束
struct SC_XunBaoGame_Over
{
	UINT16				m_TotalGoodsNum;		 //获得宝物总数
	UINT8               m_FinishLevel;           //完成级别
	UINT16              m_AdventureAwardID;      //奇遇奖励
};

/////////////////////////////////////////////////////////////////////////////////////////////////
//答题游戏
enum enGamDaTiCmd VC_PACKED_ONE
{
	enGamDaTiCmd_InitClient =0,     //初始化游戏客户端
	enGamDaTiCmd_Start,             //游戏开始
	enGamDaTiCmd_BornTiMu,			//产生题目
	enGamDaTiCmd_DaTi,				//答题
	enGamDaTiCmd_TimeOut,			//答题超时
	enGamDaTiCmd_Over,              //游戏结束

	enGamDaTiCmd_Max

}PACKED_ONE;

//初始化
struct SC_DaTiGame_Init
{
	UINT16				m_GameTotalTime;			//时间总时长
	UINT8				m_StartAfterTime;			//游戏倒数几秒开始
	UINT16				m_TotalTiMu;				//题目总数
};

//产生题目
struct SC_BornTiMu
{
	UINT16				m_TiMuIndex;	//题目索引
//  char szTiMu[];		//题目
//  char szAnserA[];	//答案A
//  char szAnserB[];	//答案B
//  char szAnserC[];	//答案C
//  char szAnserD[];	//答案D
};


//答题
struct CS_DaTi_Req
{
	UINT16	m_TiMuIndex;	//题目索引

	UINT8	m_nDaAnIndex;	//选择的答案，0,1,2,3对应于A,B,C,D
};

struct SC_DaTi_Rsp
{
	SC_DaTi_Rsp() : m_TiMuIndex(0), m_bRight(false){}
	UINT16	m_TiMuIndex;	//题目索引

	bool	m_bRight;		//是否答对
};

//答题超时

//游戏结束
struct SC_DaTiGame_Over
{
	UINT16				m_TotalTiMuNum;			//题目总数
	UINT16				m_RightNum;				//答应个数
	UINT8               m_FinishLevel;          //完成级别
	UINT16              m_AdventureAwardID;     //奇遇奖励
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//采集游戏
enum enGameGatherCmd VC_PACKED_ONE
{
	enGameGatherCmd_InitClient =0,     //初始化游戏客户端
	enGameGatherCmd_Start,             //游戏开始
	enGameGatherCmd_Detect,            //探测
	enGameGatherCmd_Gather,            //采集
	enGameGatherCmd_ObtainGoods,       //获得物品	
	enGameGatherCmd_Over,              //游戏结束

	enGameGatherCmd_Max

}PACKED_ONE;



//初始化enGameGatherCmd_InitClient
struct SC_GatherGame_Init
{
	UINT16				m_GameTotalTime;			//时间总时长
	UINT8				m_StartAfterTime;			//游戏倒数几秒开始
	UINT16              m_XDetectNum;               //X坐标可以探测点数
	UINT16              m_YDetectNum;               //Y坐标可以探测点数
	UINT16              m_ColorRange[enIndicatorColor_Max] ;            //指示灯color代表附近多少格范围内有宝物，为零表示附近没有宝物

};


//探测enGameGatherCmd_Detect
struct CS_GatherGame_Detect_Req
{
	UINT16             m_DetectPoint;  //探测点,从零开始编号
};



//探测结果
struct SC_GatherGame_Detect_Rsp
{
	SC_GatherGame_Detect_Rsp()
	{
		MEM_ZERO(this);
	}
	UINT16             m_DetectPoint;          //探测点,从零开始编号
	TGoodsID           m_GoodsID;              //获得的宝物 如为零表示没宝物
	UINT8              m_IndicatorColor;      //指示灯color取值看enIndicatorColor

};

//采集enGameGatherCmd_Gather
struct CS_GatherGame_Gather_Req
{
	UINT16             m_DetectPoint;  //探测点,从零开始编号
};

struct SC_GatherGame_Gather_Rsp
{
	UINT16             m_DetectPoint;  //探测点,从零开始编号
	UINT16             m_TotalGatherNum; //累积总采集数
};

//获得物品enGameGatherCmd_ObtainGoods
struct SC_GatherGame_ObtainGoods
{
	UINT16             m_DetectPoint;  //探测点,从零开始编号
	TGoodsID           m_GoodsID;              //获得的宝物
	UINT16             m_GoodsNum;        //获得物品数量
};


//游戏结束
struct SC_GatherGame_Over
{
	UINT16				m_MaxGatherNum;		 //最大连续采集次数
	UINT8               m_FinishLevel;           //完成级别
	UINT16              m_AdventureAwardID;      //奇遇奖励
};

//查看回放
struct CS_ViewRec
{
	UINT32	m_Rank;	//场次
};

//查看回放
struct SC_ViewRec
{
	SC_ViewRec(){
		m_Result = enTalismanWorldRetCode_OK;
	}
	enTalismanWorldRetCode    m_Result;
  //TalismanWorldRecordBuf	RecordBuf;	//回放buf
};

struct TalismanWorldRecordBuf
{
	int								m_BufLen;			//buf长度
	TBinArray<sizeof(OBuffer4k)>	m_TalismanWorldRecord;	//回放buf
	
};

//得到占领时被谁击败
struct SC_OccupationInfo
{
	SC_OccupationInfo(){
		MEM_ZERO(this);
	}

	UINT32		m_RecIndex;						//战斗回放场次（点击查看回放时要用）
	UINT32		m_GhostSoul;					//占领被打败时获得的灵魄
	char		m_EnemyName[THING_NAME_LEN];	//对手名字
	UINT8		m_VipLevel;						//vip等级
};

#pragma pack(pop)

#endif
