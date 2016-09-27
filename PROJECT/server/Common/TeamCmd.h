#ifndef __XJCQ_GAMESRV_TEAM_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_TEAM_CMD_PROTOCOL_H__

#pragma pack(push,1)


enum enTeamCmd VC_PACKED_ONE
{
	enTeamCmd_InviteTeam = 0,		//邀请组队
	enTeamCmd_SC_Invited,			//被邀请组队(弹出是否同意对方邀请框)
	enTeamCmd_Ok_Invite,			//同意组队
	enTeamCmd_No_Invite,			//拒绝组队
	enTeamCmd_SC_CreateTeam,		//创建队伍(把对方的角色信息发给客户端)
	enTeamCmd_QuitTeam,				//退出队伍
	enTeamCmd_SC_FuBen_FastTeam,	//副本快速组队
	enTeamCmd_QuitWaitOrTimeEnd,	//退出等待或者等待时间到
	enTeamCmd_SetTeamLineup,		//设置组队阵形
	enTeamCmd_SC_SycTeamLineup,		//同步组队阵形
	enTeamCmd_OpenTeamInfo,			//打开队伍信息
	enTeamCmd_SetTeamCombatState,	//设置组队参战状态
	enTeamCmd_SC_SycTeamCombatState,//同步组队参战状态

	enTeamCmd_Max,

}PACKED_ONE;

//组队消息
struct  TeamHeader  : public AppPacketHeader
{
	TeamHeader(enTeamCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Team;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//组队结果码定义
enum enTeamRetCode VC_PACKED_ONE
{
	enTeamRetCode_Ok = 0,
	enTeamRetCode_NotFriend,		//只有好友才能指定组队
	enTeamRetCode_OffLine,			//对方不在线，不能组队
	enTeamRetCode_InBusy,			//无法组队，对方必须在空闲状态
	enTeamRetCode_MeExist,			//已有队伍，不能再组队
	enTeamRetCode_OtExist,			//对方已有队伍，不能组队
	enTeamRetCode_NoTeam,			//您没有队伍
	enTeamRetCode_RefuseTeam,		//对方拒绝你的组队邀请
	enTeamRetCode_Wait,				//进入快速组队等待
	enTeamRetCode_NoHaveUuser,		//组队阵形中必须包含两个主角
	enTeamRetCode_ErrActor,			//错误的角色
	enTeamRetCode_MemQuitTeam,		//队友离队，自动解散队伍
	enTeamRetCode_ErrMasterCombat,	//主角必须参战
	enTeamRetCode_ErrTeamCombatNum,	//超过组队参战人员数量上限
	enTeamRetCode_ErrHaveTeam,		//正在组队中，不能设置组队参战
	enTeamRetCode_ErrTeamSelf,		//不能和自己组队
	enTeamRetCode_InCombat,			//正在战斗中，不能离队

	enTeamRetCode_ErrHaveFast,		//已经操作了快速组队

	enTeamRetCode_NotTwoFast,		//只能快速组队一个副本


	enTeamRetCode_Max,

} PACKED_ONE;

//邀请组队
struct CS_InviteTeam
{
	CS_InviteTeam()
	{
		m_uidFriend = UID();
	}

	UID		m_uidFriend;
};

struct SC_InviteTeam
{
	SC_InviteTeam()
	{
		m_RetCode = enTeamRetCode_Ok;
	}

	enTeamRetCode	m_RetCode;
};

//被邀请组队(弹出是否同意对方邀请框)
struct SC_Invited
{
	UID		m_uidUser;					//邀请者的UID
	char	m_Name[ACTOR_NAME_LEN];		//邀请者的名字
};

//同意组队
struct CS_AgreeTeam
{
	UID		m_uidUser;					//邀请者的UID
};

struct SC_AgreeTeam
{
	SC_AgreeTeam()
	{
		m_RetCode = enTeamRetCode_Ok;
	}

	enTeamRetCode	m_RetCode;

};

//创建队伍
struct SC_CreateTeam
{
	SC_CreateTeam()
	{
		m_Num = 0;
	}

	UID				m_uidLeader;		//队长的UID
	UINT8			m_Num;				//队友的角色数量
//	UID				m_uidMember[m_Num];	//队友的角色UID
};

//拒绝组队
struct CS_RefuseTeam
{
	UID		m_uidUser;				//邀请者的uid			
};

//向双方发送
struct SC_RefuseTeam
{
	enTeamRetCode	m_RetCode;
};

//退出队伍
struct SC_QuitTeam
{
	SC_QuitTeam()
	{
		m_RetCode = enTeamRetCode_Ok;
	}

	enTeamRetCode	m_RetCode;
};

//副本快速组队
struct SC_FuBen_FastTeam
{
	SC_FuBen_FastTeam()
	{
		m_RetCode = enTeamRetCode_Ok;
	}

	enTeamRetCode	m_RetCode;
};

//退出等待或者等待时间到
struct SC_QuitWaitOrTimeEnd
{
	SC_QuitWaitOrTimeEnd()
	{
		m_RetCode = enTeamRetCode_Ok;
	}

	enTeamRetCode	m_RetCode;
};

//设置组队阵形
struct CS_SetTeamLineup
{
	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //组队阵型
};

struct SC_SetTeamLineup
{
	enTeamRetCode	m_RetCode;
};

//同步组队阵形
struct SC_SynTeamLineup
{
	UID  m_uidLineup[MAX_LINEUP_POS_NUM]; //组队阵型
};

//打开队伍信息
struct SC_OpenTeamInfo
{
	SC_OpenTeamInfo()
	{
		memset(this, 0, sizeof(*this));
	}

	enTeamRetCode	m_RetCode;
	INT16			m_FreeNum;			//自己还可免费进入次数
	INT16			m_VipNum;			//自己VIP还可进入次数
	INT16			m_StoneNum;			//自己灵石还可进入次数
	INT16			m_MaxSynWelNum;		//自己帮派福利最大可进入次数
	INT16			m_SynWelNum;		//自己帮派福利还可进入次数

	INT16			m_MemFreeNum;		//队友还可免费进入次数
	INT16			m_MemVipNum;		//队友VIP还可进入次数
	INT16			m_MemStoneNum;		//队友灵石还可进入次数
	INT16			m_MaxMemSynWelNum;	//队友帮派福利最可进入次数
	INT16			m_MemSynWelNum;		//队友帮派福利还可进入次数
};

//设置组队参战状态
struct CS_SetTeamCombatState
{
	UID		m_uidActor;			//角色UID
	bool	m_bTeamCombat;		//是否设置组队参战
};

struct SC_SetTeamCombatState
{
	SC_SetTeamCombatState()
	{
		m_RetCode = enTeamRetCode_Ok;
	}
	enTeamRetCode	m_RetCode;
};

//同步组队参战状态
struct SC_SycTeamCombatState
{
	UID		m_uidCombat[MAX_TEAMCOMBAT_NUM];
};

#pragma pack(pop)

#endif
