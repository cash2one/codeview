#ifndef __XJCQ_GAMESRV_MAIL_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_MAIL_CMD_PROTOCOL_H__

#pragma pack(push,1)




//邮件

//消息
enum enMailCmd VC_PACKED_ONE
{
	enMailCmd_ViewMailMain,			//查看邮件主界面
	enMailCmd_ViewMail,				//查看邮件
	enMailCmd_WriteMail,			//写私人邮件
	enMailCmd_SynMail,				//写帮派邀请邮件
	enMailCmd_AcceptSyn,			//接受帮派邀请
	enMailCmd_RefuseSyn,			//拒绝帮派邀请
	enMailCmd_AcceptMailItem,		//接受邮件物品
	enMailCmd_DeleteMail,			//删除邮件
	enMailCmd_CloseMailMain,		//关闭邮件窗口
	enMailCmd_ClickFriend,			//点击选择收信人按钮
	enMailCmd_CustomerService,		//客服邮件
	enMailCmd_NoticeNewMail,		//通知有新邮件到

	enMailCmd_Max,
}PACKED_ONE;

//邮件结果码
enum enMailRetCode VC_PACKED_ONE
{
	enMailRetCode_OK,			 //成功
	enMailRetCode_NoFindMail,	 //没找到此邮件
	enMailRetCode_NoUser,		 //不存在此用户
	enMailRetCode_WriteError,	 //写邮件失败
	enMailRetCode_Item_IsDelete, //邮件有物品没领，让玩家确认是否删除邮件
	enMailRetCode_Syn_IsDelete,	 //邮件有帮派邀请没处理，让玩家确认是否删除邮件
	enMailRetCode_FullPacket,	 //背包已满，请清理背包
	enMailRetCode_ExistSyn,		 //已经有帮派
	enMailRetCode_NoSyn,		 //没找到这个帮派
	enMailRetCode_MemberFull,	 //帮派成员已满
	enMailRetCode_ErrAddSyn,     //加入帮派失败
	enMailRetCode_ErrCDTimer,	 //再次发送客服邮件需要等待1分钟
}PACKED_ONE;

//邮件，消息头
struct MailHeader : public AppPacketHeader
{
	MailHeader(enMailCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_Mail;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//查看邮件主界面
struct CS_ViewMailMain_Req
{
	INT16	m_Begin;	//开始位置
	INT16	m_End;		//结束位置
};

//先发送邮件个数
struct SC_ViewMailNum_Rsp
{
	SC_ViewMailNum_Rsp(){
		MEM_ZERO(this);
	}

	UINT16		m_HaveMailNum;		//拥有邮件个数
	UINT16		m_MailNum;			//发送的邮件个数
//	ViewMailMainData_Rsp MailData[m_MailNum];
};

//邮件类型
enum enMailType VC_PACKED_ONE
{
	enMailType_Person,		//私人邮件
	enMailType_Syn,			//帮派邮件
	enMailType_System,		//系统邮件

	enMailType_Max,
}PACKED_ONE;

//查看邮件主界面数据
struct ViewMailMainData_Rsp
{
	ViewMailMainData_Rsp(){
		MEM_ZERO(this);
	}
		
	TMailID		m_MailID;							//邮件ID
	enMailType	m_MailType;							//邮件类型
	UINT64		m_Time;								//邮件发送时间
	char		m_szThemeText[DESCRIPT_LEN_50];		//邮件主题
	char		m_szSendUserName[THING_NAME_LEN];	//发送者名字
	bool		m_bHaveGoods;						//邮件内是否有附带物品
	bool		m_bHaveSynInvate;					//邮件内是否有帮派邀请
	bool		m_bRead;							//是否已读
};

//查看邮件
struct CS_ViewMailData_Req
{
	CS_ViewMailData_Req(){
		m_MailID = 0;
	}
	TMailID			m_MailID;	//邮件ID
};

//邮件基本部分
struct SC_ViewMailBasic
{
	SC_ViewMailBasic(){
		MEM_ZERO(this);
	}

	enMailType		m_MailType;							//邮件类型
	TMailID			m_MailID;							//邮件ID
	char			m_szSendUserName[THING_NAME_LEN];	//发件人的名字
	char			m_szThemeText[DESCRIPT_LEN_50];		//主题
	char			m_szContentText[DESCRIPT_LEN_600];	//文本内容
};

//发送私人邮件
//SC_ViewMailBasic MailBasic;


//发送帮派邮件
struct ViewSynMail
{
	//ViewMailBasic		m_MailBasic;
	TSynID			m_SynID;							//帮派邀请
};



//查看系统邮件
struct ViewSystemMail
{
	ViewSystemMail() : m_Stone(0), m_Money(0), m_Ticket(0), m_GoodsNum(0)
	{
	}

	//ViewMailBasic	m_MailBasic;
	UINT32			m_Stone;							//发送的灵石数量
	UINT32			m_Money;							//发送的仙石数量
	UINT32			m_Ticket;							//发送的礼卷
	UINT32			m_PolyNimbus;						//聚灵气
	UINT8			m_GoodsNum;							//物品数量
//	MailGoodsData	m_GoodsData[m_GoodsNum];			//物品数据
};

struct MailGoodsData
{
	INT16		m_ResID;			//资源ID
	UINT8       m_Number;			//数量
	char		m_szGoodsName[THING_NAME_LEN];
	UINT8		m_Quality;			//品质
};


//写私人邮件(私人和帮派邮件不可以发送物品)
struct CS_WritePersonMail_Req
{
	CS_WritePersonMail_Req(){
		MEM_ZERO(this);
	}
	char		m_szDestUserName[THING_NAME_LEN];		//收件人的我名字
	char		m_szThemeText[DESCRIPT_LEN_50];			//主题
	char		m_szContentText[DESCRIPT_LEN_300];		//文本内容
};

//写帮派邮件(私人和帮派邮件不可以发送物品)
struct CS_WriteSynMail_Req
{
	CS_WriteSynMail_Req(){
		MEM_ZERO(this);
	}
	char		m_szDestUserName[THING_NAME_LEN];		//收件人的名字
};

//只需要用到邮件ID时，用这请求
struct CS_MailID_Req
{
	CS_MailID_Req(){
		m_MailID = 0;
	}
	TMailID		m_MailID;
};

//接收系统邮件物品
struct CS_AcceptSysMail
{
	CS_AcceptSysMail() : m_MailID(0)
	{
	}

	TMailID		m_MailID;
};

//接受帮派邀请
struct CS_AcceptSynMail
{
	CS_AcceptSynMail() : m_MailID(0)
	{
	}

	TMailID		m_MailID;
};

//拒绝帮派邀请
struct CS_RefuseSynMail
{
	CS_RefuseSynMail() : m_MailID(0)
	{
	}

	TMailID		m_MailID;
};

//删除邮件
struct CS_DeleteMail_Req
{
	CS_DeleteMail_Req() : m_MailID(0){ }
	TMailID		m_MailID;
};

//只需要返回结果时用
struct SC_MailRetCode_Rsp
{
	enMailRetCode MailRetCode;		
};


//写客服邮件
struct CS_WriteKFMail_Req
{
	char			m_szThemeText[DESCRIPT_LEN_50];		//主题
	char			m_szContentText[DESCRIPT_LEN_300];	//文本内容
};

struct SC_WriteKFMail_Rsp
{
	SC_WriteKFMail_Rsp(){
		MailRetCode = enMailRetCode_OK;
	}

	enMailRetCode MailRetCode;
};

//点击选择收信人按钮
struct SC_ClickFriend_Rsp
{
	UINT16		m_Num;
//	FriendName	m_FriendName[m_Num];
};

struct FriendName
{
	char		m_szFriendName[THING_NAME_LEN];
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)



#endif
