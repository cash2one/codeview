#ifndef __XJCQ_GAMESRV_RANK_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_RANK_CMD_PROTOCOL_H__

#pragma pack(push,1)

enum enRankCmd VC_PACKED_ONE
{
    enRankCmd_OpenRank = 0  ,        //打开排行榜
	enRankCmd_ClickUser,			 //点击玩家
		
	enRankCmd_Max,

}PACKED_ONE;

//排行榜消息
struct  RankHeader  : public AppPacketHeader
{
	RankHeader(enRankCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Rank;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//排行榜类型
enum enRank_Type VC_PACKED_ONE
{
	enRank_Type_Level   = 0,	//玩家排行榜
	enRank_Type_Syn,			//帮派排行榜
	enRank_Type_Challenge,		//挑战排行榜
	enRank_Type_SynWar,			//帮战排行榜
	enRank_Type_DuoBao,			//夺宝排行榜

	enRank_Type_Max,
}PACKED_ONE;

//打开排行榜类型
struct CS_OpenRank_Req
{
	CS_OpenRank_Req() : m_RankType(enRank_Type_Level), m_BeginRank(0), m_Num(0)
	{
	}

	enRank_Type  m_RankType;	//要查看的排行榜类型
	UINT32		 m_BeginRank;	//第几名开始(1为第一名,0表示第一次请求)
	INT16		 m_Num;			//请求多少个
};

struct SC_OpenRank_Rsp
{
	SC_OpenRank_Rsp()
	{
		m_RankType = enRank_Type_Level;
		m_SynID = 0;
		m_Num   = 0;
	}

	enRank_Type  m_RankType;				//要查看的排行榜类型
	TSynID		 m_SynID;					//我的帮派ID
	UINT16		 m_Num;						//数量
//	UserRankActorInfo	ActorInfo[m_Num];	//如果排行类型为玩家排行,则发送这个
//	SynRankSynInfo		SynInfo[m_Num];		//如果排行类型为帮派排行,则发送这个
//  ChallengeRankInfo	ChallengeInfo[m_Num];	//如果排行类型为挑战，则发送这个
//	SynRankSynWarInfo	SynWarInfo[m_Num];	//如果排行类型为帮战排行，则发送这个

};

//玩家排行的玩家信息
struct UserRankActorInfo
{
	UINT32	m_Rank;							//排名
	UID		m_uidUser;						//玩家UID
	UINT8	m_Level;						//玩家等级
	INT32	m_Exp;							//经验
	UINT8	m_Layer;						//玩家境界
	UINT8	m_vipLevel;						//vip等级
	char	m_Name[THING_NAME_LEN];			//玩家名字
	char	m_SynName[THING_NAME_LEN];		//帮派名字
};

//夺宝排行的玩家信息
struct DuoBaoRankActorInfo
{
	UINT32	m_Rank;							//排名
	UID		m_uidUser;						//玩家UID
	UINT8	m_Level;						//玩家等级
	UINT32	m_CombatAbility;				//战斗力
	INT32	m_DuoBaoLevel;					//夺宝等级
	UINT8	m_vipLevel;						//vip等级
	char	m_Name[THING_NAME_LEN];			//玩家名字
	char	m_SynName[THING_NAME_LEN];		//帮派名字
};



//帮派排行榜
struct SynRankSynInfo
{
	UINT32	m_Rank;							//排名
	TSynID  m_SynID;						//帮派ID
	char	m_SynName[THING_NAME_LEN];		//帮派名字
	UINT8	m_SynLevel;						//帮派等级
	UINT16  m_SynMemberNum;					//帮派人数
	char	m_LeaderName[THING_NAME_LEN];	//帮主名字
};

//帮战排行榜
struct SynRankSynWarInfo
{
	UINT32	m_Rank;							//排名
	TSynID  m_SynID;						//帮派ID
	char	m_SynName[THING_NAME_LEN];		//帮派名字
	UINT8	m_SynLevel;						//帮派等级
	UINT16  m_SynMemberNum;					//帮派人数
	char	m_LeaderName[THING_NAME_LEN];	//帮主名字
	INT32	m_SynWarAbility;				//帮派战力
	UINT32	m_SynWarScore;					//帮战总积分
	char	m_PreEnemySynName[THING_NAME_LEN];   //上一场帮战敌对帮派名
	enumSynWarbWin	m_bWin;							//上一场帮战胜负
};

//挑战排行榜
struct ChallengeRankInfo
{
	UINT32	m_Rank;							//排名
	UID		m_uidUser;						//玩家UID
	UINT8	m_Level;						//等级
	UINT8	m_Layer;						//玩家境界
	UINT16	m_ChallengeNum;					//挑战次数
	char	m_Name[THING_NAME_LEN];			//玩家名字
	char	m_SynName[THING_NAME_LEN];		//帮派名字
	UINT8	m_vipLevel;						//vip等级
};

//点击玩家
struct CS_ClickUser_Req
{
	UID		m_uidUser;
};

struct SC_ClickUser_Rsp
{
	TSynID	m_SynID;
	//若帮派ID有效，则发下面的
//	UserSynInfo	m_SynInfo; 
};

struct UserSynInfo
{
	char		 m_szSynName[THING_NAME_LEN];			//好友帮派名称
	UINT16		 m_SynMemberNum;						//帮派人数
	UINT8		 m_SynLevel;							//帮派等级
	char		 m_szSynLeaderName[THING_NAME_LEN];		//帮主名字
	INT32		 m_SynWarAbility;						//帮派战力
};


#pragma pack(pop)

#endif
