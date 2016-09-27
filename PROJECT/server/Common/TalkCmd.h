#ifndef __XJCQ_GAMESRV_TALK_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_TALK_CMD_PROTOCOL_H__

#pragma pack(push,1)


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//聊天
//消息
enum enTalkCmd VC_PACKED_ONE
{
	enTalkCmd_Private,				//私人聊天
	enTalkCmd_WorldTalk,			//世界聊天
	enTalkCmd_WorldViewItem,		//世界展示物品
	enTalkCmd_SynTalkRet,			//帮派聊天
	enTalkCmd_SycSynTalk,			//同步帮派聊天
	enTalkCmd_ZhaoMuSynMember,		//帮派招募成员
	enTalkCmd_OpenMenu,				//打开菜单(聊天框中点击玩家名字出现的菜单)
	enTalkCmd_ExitMenu,				//退出菜单
	enTalkCmd_SycPrivate,			//同步私人聊天
	enTalkCmd_SycWorldTalk,			//同步世界聊天
	enTalkCmd_SycWorldViewItem,		//同步世界展示物品

	enTalkCmd_WorldNotice,			//全服公告

	enTalkCmd_SC_Tip,				//右上角的提示悬浮框

	enTalkCmd_ViewGoodsSuperLink,	//查看物品超链接

	enTalkCmd_Add_SystemMsg,		//增加系统消息显示(后台发来的)

	enTalkCmd_SC_ViewSysMsg,		//显示系统消息

	enTalkCmd_SC_CancelXiuLianMsg,	//取消显示修炼公告
	enTalkCmd_SC_CancelEffectMsg,	//取消显示效果公告

	enTalkCmd_SC_TalkSysMsg,		//聊天框系统公告

	enTalkCmd_SC_CancelHouTaiMsg,	//取消显示后台公告

	enTalkCmd_Team,					//组队聊天
	enTalkCmd_SycTeam,				//同步组队聊天

	enTalkCmd_ViewSynInfo,			//显示帮派数据

	enTalkCmd_Max,
}PACKED_ONE;

//聊天，消息头
struct TalkHeader : public AppPacketHeader
{
	TalkHeader(enTalkCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Talk;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

enum enTalkRetCode VC_PACKED_ONE
{
	enTalkRetCode_OK,
	enTalkRetCode_ErrorUser,		//玩家不在线或没有此玩家
	enTalkRetCode_ErrorNoSyn,		//玩家没有帮派
	enTalkRetCode_ErrorCDTime,		//冷却时间未到
	enTalkRetCode_ErrorGoodsNot,	//此物品已不存在
	enTalkRetCode_DontTalk,			//您已被禁言
	enTalkRetCode_NotExitMagic,		//该法术不存在
	enTalkRetCode_ErrorNoTeamMember,		//没有队友

	enTalkRetCode_Max,
}PACKED_ONE;


//私人聊天时
struct CS_PrivateTalk_Req
{
	CS_PrivateTalk_Req(){
		MEM_ZERO(this);
	}

	UINT64	m_DestUserUID;
	char	m_TalkContent[DESCRIPT_LEN_100];		//聊天内容
};

//私人聊天时
struct SC_PrivateTalkRet_Rsp
{
	enTalkRetCode m_TalkRetCode;							//结果码
};
//私人聊天时
struct SC_PrivateTalk_Rsp
{
	SC_PrivateTalk_Rsp(){
		MEM_ZERO(this);
	}

	UINT64		  m_SendUserUID;							//发送者的UID值
	UINT8		  m_SendUserVipLevel;						//vip等级
	char		  m_szSendUserName[THING_NAME_LEN];			//聊天内容的发送者名字
	char		  m_szTitleName[THING_NAME_LEN];			//称号名字
	char		  m_szTalkContent[DESCRIPT_LEN_100];		//聊天内容
};

//组队聊天
struct CS_TeamTalk_Req
{
	CS_TeamTalk_Req(){
		MEM_ZERO(this);
	}

	char	m_TalkContent[DESCRIPT_LEN_100];		//聊天内容
};

//组队聊天结果码
struct SC_TeamTalkRet_Rsp
{
	enTalkRetCode m_TalkRetCode;							//结果码
};
//组队聊天时
struct SC_TeamTalk_Rsp
{
	SC_TeamTalk_Rsp(){
		MEM_ZERO(this);
	}

	UINT64		  m_SendUserUID;							//发送者的UID值
	UINT8		  m_SendUserVipLevel;						//vip等级
	char		  m_szSendUserName[THING_NAME_LEN];			//聊天内容的发送者名字
	char		  m_szTitleName[THING_NAME_LEN];			//称号名字
	char		  m_szTalkContent[DESCRIPT_LEN_100];		//聊天内容
};

//世界聊天
struct CS_WorldTalk_Req
{
	CS_WorldTalk_Req(){
		MEM_ZERO(this);
	}
	char	m_TalkContent[DESCRIPT_LEN_100];
};

//世界聊天结果码
struct SC_WorldTalkRet_Rsp
{
	SC_WorldTalkRet_Rsp() : m_RetCode(enTalkRetCode_OK){}
	enTalkRetCode  m_RetCode;
};

//同步世界聊天时
struct SC_WorldTalk_Rsp
{
	SC_WorldTalk_Rsp(){
		MEM_ZERO(this);
	}
	UID			m_SendUserUID;								//发送者的UID值
	char		m_szSendUserName[THING_NAME_LEN];			//发送者名字
	char		m_szTitleName[THING_NAME_LEN];				//称号名字
	char		m_szTalkContent[DESCRIPT_LEN_100];			//聊天内容
	UINT8		m_SendUserVipLevel;							//发送者vip等级
};

//帮派聊天时
struct CS_SynTalk_Req
{
	CS_SynTalk_Req(){
		MEM_ZERO(this);
	}
	char	m_TalkContent[DESCRIPT_LEN_100];
};

//帮派聊天时，返回结果码
struct SC_SynTalkRetCode_Rsp
{
	SC_SynTalkRetCode_Rsp() : RetCode(enTalkRetCode_OK){}
	enTalkRetCode  RetCode;
};

//同步帮派聊天
struct SC_SynTalk_Rsp
{
	SC_SynTalk_Rsp(){
		MEM_ZERO(this);
	}

	UINT64		m_SendUserUID;								//发送者UID值
	UINT8		m_SendVipLevel;								//发送者VIP等级
	char		m_szSendUserName[THING_NAME_LEN];			//发送者名字
	char		m_szTitleName[THING_NAME_LEN];				//称号名字
	char		m_szTalkContent[DESCRIPT_LEN_100];			//聊天内容
};

//超链接类型
enum enSuperLink VC_PACKED_ONE
{
	enSuperLink_Magic = 0,		//法术超链接
	enSuperLink_Goods,			//物品超链接
	enSuperLink_ChengJiu,		//成就超链接

	enSuperLink_Max,
}PACKED_ONE;

//世界展示
struct  CS_ViewThingSuperLink_Req
{
	enSuperLink m_SuperLinkType;			//超链接类型
//根据展示类型，发下面的结构
};

//展示法术
struct ViewMagic
{
	TMagicID	m_MagicID;			//法术ID
};

//展示物品
struct ViewGoods
{
	UID			m_uidGoods;			//要展示的物品UID
};

//展示成就
struct ViewChengJiu
{
	TChengJiuID	m_ChengJiuID;		//成就ID
};

//世界展示结果码
struct SC_ViewThingSuperLink_Rsp
{
	SC_ViewThingSuperLink_Rsp() : m_RetCode(enTalkRetCode_OK)
	{
	}

	enTalkRetCode  m_RetCode;
};

//同步世界展示事物
struct SC_WorldViewThing_Rsp
{
	SC_WorldViewThing_Rsp(){
		MEM_ZERO(this);
	}

	UINT64		m_SendUserUID;								//发送者UID值
	UINT8		m_SendUserVipLevel;							//发送者vip等级
	char		m_szSendUserName[THING_NAME_LEN];			//发送者名字
	char		m_szTitleName[THING_NAME_LEN];				//称号名字
	enSuperLink m_SuperLink;								//超链接类型
//根据类型，发下面的结构
};

//同步展示物品
struct SynViewGoods
{
	TGoodsID	m_GoodsID;		//物品ID
	UID			m_uidGoods;		//物品UID
};

//同步展示法术
struct SynViewMagic
{
	TMagicID	m_MagicID;
	UINT8		m_Level;
};

//同步展示成就
struct SynViewChengJiu
{
	TChengJiuID	m_ChengJiuID;
	UINT32		m_FinishTime;
};


//帮派成员招募
struct SC_ZhaoMuSynMember_Rsq
{
	SC_ZhaoMuSynMember_Rsq(){
		MEM_ZERO(this);
	}
	UINT64		m_SendUserUID;								//发送者UID值
	char		m_szSendUserName[THING_NAME_LEN];			//发送者名字
	char		m_szContent[DESCRIPT_LEN_100];				//内容
	TSynID		m_SynID;									//帮派ID
	char		m_SynName[THING_NAME_LEN];					//帮派名字
	char        m_szTitleName[TITLE_NAME_LEN];				//发送者的称号名称
};

//打开菜单
struct CS_OpenMenu_Req
{
	UINT64		m_ActorUID;				//玩家UID值
};

struct SC_OpenMenu_Rsp
{
	UINT8		m_ActorLevel;						//玩家等级
	char		m_szActorName[THING_NAME_LEN];		//发送者名字
};

//退出菜单
struct CS_ExitMenu_Req
{
	UINT64		m_ActorUID;				//玩家UID值
};

//全服公告
struct CS_WorldNotice_Req
{
	CS_WorldNotice_Req(){
		memset(this, 0, sizeof(*this));
	}
	char		m_szNoticeContext[DESCRIPT_LEN_100];	//公告内容
};

struct SC_WorldNotice_Rsp
{
	SC_WorldNotice_Rsp(){
		memset(this, 0, sizeof(*this));
	}
	char		m_szName[THING_NAME_LEN];				//发公告的玩家名字
	char		m_szNoticeContext[DESCRIPT_LEN_100];	//公告内容
};

//右上角的提示悬浮框
//char		m_Content[];	//内容,以0结尾

//查看物品超链接
struct CS_ViewGoodsSuperLink_Req
{
	UID		m_uidGoods;
};

struct SC_ViewGoodsSuperLink_Rsp
{
	enTalkRetCode  m_RetCode;
	UID		m_uidGoods;
};


//显示帮派数据
struct CS_ViewSynInfo
{
	TSynID		m_SynID;
};

struct SC_ViewSynInfo
{
	TSynID		m_SynID;						//帮派ID
	UINT8		m_SynLevel;						//帮派等级
	UINT16		m_SynMemberNum;					//帮派人数
	char		m_SynLeaderName[THING_NAME_LEN];//帮主名字
	char		m_SynName[THING_NAME_LEN];		//帮派名字
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum enMsgType VC_PACKED_ONE
{
	enMsgType_XiuLian = 0,		//修炼类公告
	enMsgType_Effect,			//效果类公告
	enMsgType_System,			//后台类公告
	enMsgType_Insert,			//插入式公告
}PACKED_ONE;

//后台通知服务器增加系统消息
struct AddSystemMsg
{
	UINT32	m_SynMsgID;	//系统消息ID
};

//通知客户端显示系统消息
struct SC_ViewSystemMsg
{
	enMsgType m_MsgType;
};

//修炼类消息
//struct SXiuLianMsg
//{
//	enXiuLianType m_XiuLianType;
//	UINT32		  m_RemainTime;
//};

//状态类消息
//struct SEffectMsg
//{
//	TEffectID	 m_EffectID;
//	char		 m_EffectName[THING_NAME_LEN];
//	UINT32		 m_RemainTime;
//};

//插入式公告
//struct SInsertMsg
//{
//	char szMsgBody[DESCRIPT_LEN_100];
//};

//后台类公告
//struct HouTaiMsg
//{
//	UINT32	m_ID;
//	char	m_szMsgBody[DESCRIPT_LEN_100];
//	UINT32	m_RemainTime;
//};

//取消显示修炼公告
struct SC_CancelViewXLMsg
{
	enXiuLianType m_XiuLianType;
};

//取消显示效果公告
struct SC_CancelViewEffectMsg
{
	TEffectID	m_EffectID;
};

//取消显示后台公告
struct SC_CancelViewHouTaiMsg
{
	UINT32		m_MsgID;
};

//聊天频道
enum enTalkChannel VC_PACKED_ONE
{
	enTalkChannel_World = 0,	//世界频道
	enTalkChannel_Person,		//私人频道
	enTalkChannel_Sys,			//帮派频道

}PACKED_ONE;

//聊天公告类型
enum enTalkMsgType VC_PACKED_ONE
{
	enTalkMsgType_UpLevel = 0,	//玩家升级信息
	enTalkMsgType_System,		//系统消息
};

//聊天框系统公告
struct SC_TalkSysMsg
{
	char		m_szSysMsg[DESCRIPT_LEN_100];				//消息内容
	enTalkChannel	m_Channel;								//频道
	enTalkMsgType	m_TalkMsgType;							//公告类型
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
