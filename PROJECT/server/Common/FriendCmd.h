#ifndef __XJCQ_GAMESRV_FRIEND_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_FRIEND_CMD_PROTOCOL_H__

#pragma pack(push,1)




//好友
//消息
enum enFriendCmd VC_PACKED_ONE
{
	enFriendCmd_AddFriend,			//添加好友
	enFriendCmd_DeleteFriend,		//删除好友
	enFriendCmd_ViewFriend,			//查看好友

	enFriendCmd_ViewFriendEnvent,	//查看好友度改变事件

	enFriendCmd_ViewFriendMsg,		//查看好友信息标签里的信息

	enFriendCmd_ViewOneCityOnlineUser,	//查看同城的在线玩家

	enFriendCmd_VisitFriend,		//拜访好友

	enFriendCmd_EndVisitFriend,		//结束拜访好友

	enFriendCmd_ViewUserInfo,		//查看其它玩家人物信息
	enFriendCmd_LeaveViewUserInfo,	//离开其它玩家人物查看信息界面


	enFriendCmd_DeleteFriendMsg,	//删除好友信息

	enFriendCmd_Max,
}PACKED_ONE;

//好友，消息头
struct FriendHeader : public AppPacketHeader
{
	FriendHeader(enFriendCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Relation;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//结果码
enum enFriendRetCode VC_PACKED_ONE
{
	enFriendRetCode_OK,				//OK
	enFriendRetCode_Full,			//好友列表已满
	enFriendRetCode_Exist,			//该玩家已存在好友列表当中
	enFriendRetCode_NoExist,		//该玩家不在你的好友列表当中
	enFriendRetCode_ErrorAddMySelf,	//不能添加自己为好友
	enFriendRetCode_NoExistUser,	//不存在此玩家
	enFriendRetCode_ErrEnterScene,	//拜访好友失败
	enFriendRetCode_enHoldOn,				//请稍候再试
	enFriendRetCode_ErrInBusy,		//好友正在忙，不能拜访
	enFriendRetCode_NoFriendMsg,	//不存在此玩家信息
}PACKED_ONE;

//返回结果码
struct SC_FriendResult_Rsp
{
	enFriendRetCode m_Result;	
};

//添加好友
struct CS_AddFriend_Req
{
	char m_szFriendName[THING_NAME_LEN];		//要加的好友名字
};

//删除好友
struct CS_DeleteFriend_Req
{
	UINT64	m_uidFriend;
};

struct SC_DeleteFriend_Rsp
{
	enFriendRetCode m_Result;
};

//查看好友列表(发好友个数)
struct SViewFriendListDataRsp
{
	SViewFriendListDataRsp(){
		MEM_ZERO(this);
	}
	INT16	m_nFriendNum;						//好友数量
//	SFriendListDataRsp FriendData[m_nFriendNum];
};

//是单方好友还是双方好友
enum enFriendType VC_PACKED_ONE
{
	enFriendType_Single,								//单方好友
	enFriendType_Double,								//双方好友

	enFriendType_Max,
}PACKED_ONE;

//查看好友发送数据
struct SFriendListDataRsp
{
	SFriendListDataRsp(){
		MEM_ZERO(this);
	}
	UINT64		 m_uidFriend;							//好友UID值
	char		 m_szFriendName[THING_NAME_LEN];		//好友名字
	INT8		 m_Level;								//好友等级
	INT8		 m_Sex;									//好友性别
	UINT32		 m_RelationNum;							//和好友的好友度
	enFriendType m_enFriendType;						//单方好友还是双方好友
	bool		 m_bOneCity;							//是否同城
	UINT16		 m_Facade;								//外观
	TTitleID	 m_TitleID;								//称号ID 8位
	INT32		 m_DuoBaoLevel;							//夺宝等级
	UINT32		 m_CombatAbility;						//战斗力
	INT32		 m_SynWarLevel;							//帮战等级
	bool		 m_bOnLine;								//是否在线
	TSynID		 m_SynID;								//帮派ID
	//如果m_SynID有效，则发下面的
//	SSynInfo     m_SynInfo;									
};

struct SSynInfo
{
	char		 m_szSynName[THING_NAME_LEN];			//好友帮派名称
	UINT16		 m_SynMemberNum;						//帮派人数
	UINT8		 m_SynLevel;							//帮派等级
	char		 m_szSynLeaderName[THING_NAME_LEN];		//帮主名字
	INT32		 m_SynWarAbility;						//帮派战力
};

//查看好友度改变事件(发送事件个数)
struct CS_ViewFriendEvent_Req
{
	UID		m_uidFriend;
};

struct SC_ViewFriendEnvent_Rsp
{
	SC_ViewFriendEnvent_Rsp(){
		m_nFriendEnventNum = 0;
	}
	INT16	m_nFriendEnventNum;
//SFriendEnventDataRsp FriendEvent[m_nFriendEnventNum];
};

//查看好友度改变事件发送数据
struct SFriendEnventDataRsp
{
	SFriendEnventDataRsp(){
		MEM_ZERO(this);
	}
	UINT64		m_uTime;								//事件发生时间
	char		m_szDescript[DESCRIPT_LEN_100];			//事件描述
};

//查看好友信息标签里的信息(先发送个数)
struct SViewFriendMsgListDataRsp
{
	SViewFriendMsgListDataRsp(){
		MEM_ZERO(this);
	}
	INT16	m_nFriendMsgNum;
};

//查看好友信息标签里的信息数据
struct SFriendMsgDataRsp
{
	SFriendMsgDataRsp(){
		MEM_ZERO(this);
	}
	UID			m_uidSrcUser;								//加你好友的玩家UID
	char		m_szName[THING_NAME_LEN];					//玩家名字
	UINT8		m_Level;									//等级
	INT8		m_Sex;										//性别
	bool		m_bOneCity;									//是否同城
	UINT16		m_Facade;									//外观
	TTitleID	m_TitleID;									//称号ID
	INT32		 m_DuoBaoLevel;							//夺宝等级
	UINT32		 m_CombatAbility;						//战斗力
	INT32		 m_SynWarLevel;							//帮战等级
	bool		 m_bOnLine;								//是否在线
	TSynID		m_SynID;									//帮派ID
	//如果m_SynID有效，则发下面的
//	SSynInfo     m_SynInfo;		
};

//查看同城玩家数据
struct SOneCityUserDataRsp
{
	SOneCityUserDataRsp(){
		MEM_ZERO(this);
	}
	UINT64		 m_uidUser;							//好友UID值
	char		 m_szUserName[THING_NAME_LEN];		//好友名字
	INT8		 m_Level;								//好友等级
	INT8		 m_Sex;									//好友性别
	char		 m_szSynName[THING_NAME_LEN];			//好友帮派名称
	UINT16		 m_Facade;								//外观
	TTitleID	 m_TitleID;								//称号
};
//查看同城的在线玩家
struct SC_OneCityOnlineUser_Rsp
{
	SC_OneCityOnlineUser_Rsp(){
		MEM_ZERO(this);
	}
	INT16	m_nFriendNum;		//好友数量	
//	SOneCityUserDataRsp UserData[m_nFriendNum];
};

//拜访好友
struct CS_VisitFriend_Req
{
	UID m_uidFriend;
};

struct SC_VisitFriend_Rsp
{
	SC_VisitFriend_Rsp()
	{
		m_Result = enFriendRetCode_OK;
		m_TotalCollectToday = 0;
		m_MaxCollectToday = 0;
	}

	enFriendRetCode m_Result;
	UINT32			m_TotalCollectToday;	//当天共代收数量
	UINT32			m_MaxCollectToday;		//每日最大代收数量

//	VisitFriendInfo	FriendInfo;	//如果OK的话
};

struct CombatActorInfo
{
	UID		m_uidActor;	//角色UID
	UINT16	m_Facade;	//角色外观
	UINT8	m_Level;	//角色等级
	char	m_Name[THING_NAME_LEN];	//名字
};

struct VisitFriendInfo
{
	UID m_uidFriend; //主角UID
	UINT8	m_Num;
//	CombatActorInfo ActorInfo[m_Num];
};

enum enAddRelationNumType VC_PACKED_ONE
{
	enHelpStone,			//帮对方收取灵石
	enGuardOneHour,			//守护对方一小时以上
	enXiuLianOneHour,		//一起修炼1小时以上
	enXiuLianMagic,			//修炼成功一个法术
	enJoinFaBaoComplete,	//进入对方法宝世界游玩并达成任意一个完成条件
	enStealStone,			//窃取好友灵石
	enCancelBeGuard,		//取消被守护
	enCancelGuard,			//取消守护
}PACKED_ONE;

//查看其它玩家人物信息
struct CS_ViewUserInfo
{
	UID		m_uidUser;
};

struct SC_ViewUserInfo
{
	enFriendRetCode m_Result;
	UID		m_uidUser;						//要查看的玩家UID
	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //阵型
};

//离开其它玩家人物查看信息界面
struct CS_LeaveViewUserInfo
{
	UID		m_uidUser;
};

struct SC_LeaveViewUserInfo
{
	enFriendRetCode m_Result;
};


//删除好友信息
struct CS_DeleteFriendMsg
{
	UID		m_uidUser;
};

struct SC_DeleteFriendMsg
{
	SC_DeleteFriendMsg()
	{
		m_Result = enFriendRetCode_OK;
	}
	enFriendRetCode m_Result;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
