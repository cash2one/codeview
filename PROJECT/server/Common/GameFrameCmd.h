
#ifndef __XJCQ_GAMESRV_GAMEFRAME_CMD_PROTOCOL_H__
#define __XJCQ_GAMESRV_GAMEFRAME_CMD_PROTOCOL_H__

#pragma pack(push,1)


//游戏服框架消息命令字定义
enum enGameFrameCmd VC_PACKED_ONE
{
	enGameFrameCmd_CS_Enter           = 0,  //进入游戏服
	enGameFrameCmd_SC_Enter,                //进入游戏服应答
	enGameFrameCmd_CS_CreateActor,          //创建角色
	enGameFrameCmd_SC_InitClient,           //初始化客户端
	enGameFrameCmd_SC_Kick,                 //踢除下线	
	enGameFrameCmd_CS_CheckCnfg,            //检查配置文件
	enGameFrameCmd_SC_CheckCnfg,            //配置文件返回
	enGameFrameCmd_SC_UpdateCnfg,           //更新配置文件

	enGameFrameCmd_CS_Keeplive,				//心跳

	enGameFrameCmd_CS_SendPoint,			//发送游戏节点

} PACKED_ONE;


//游戏服框架消息
struct  GameFrameHeader  : public AppPacketHeader
{
	GameFrameHeader(enGameFrameCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_GameFrame;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};



//防沉迷认证状态
enum enIdentityStatus VC_PACKED_ONE
{
	enIdentityStatus_Non = 0,  //未认证

	enIdentityStatus_LessEighteen = 1, //少于18岁

    enIdentityStatus_Eighteen = 2,     //已满18岁

}PACKED_ONE;


//进入游戏 enGameFrameCmd_CS_Enter
struct CS_EnterGame_Req
{
	char  ClientVersion[VERSION_LEN];              //客户端版本号
	char  szClientCnfgVersion[MD5_LEN];  //客户端配置文件MD5

	char  szClientResVersion[MD5_LEN];  //客户端资源文件MD5

	TUserID   UserID;  //用户ID
	char      szTicket[TICKET_LEN];  //登陆返回来的票

	//enIdentityStatus  IdentityStatus; //认证状态
	//UINT32             OnlineTime; //在线时长
};


//进入游戏结果码定义
enum enEnterGameRetCode VC_PACKED_ONE
{
	enEnterGameRetCode_Ok = 0,
	enEnterGameRetCode_Unknow , //未知错误
	enEnterGameRetCode_NoActor , //没有角色
	enEnterGameRetCode_DBError , //DB错误
	enEnterGameRetCode_ExistName , //角色名已存在
	enEnterGameRetCode_ErrKeyName,       //用户名包含有敏感关键词
	enEnterGameRetCode_ServerMaintain,  //服务器维护
	enEnterGameRetCode_ErrVersion,      //版本号有误，请退出后重新登陆。

} PACKED_ONE;


//进入游戏应答 enGameFrameCmd_SC_Enter
struct SC_EnterGame_Rsp
{	
	enEnterGameRetCode Result;	

	// char    m_szGuideContext[]; //新手引导步骤

	//char      m_szServiceTel[]; //客服电话
	//char      m_szServiceEmail[]; //客服Email
	//char      m_szServiceQQ[];    //客服QQ
};

 
//创建角色 enGameFrameCmd_CS_CreateActor
struct CS_CreateActor
{
	TUserID   UserID;  //用户ID
	char      szName[THING_NAME_LEN]; //名称
	UINT8     sex;   //性别 0:女,1:男
	UINT16    facade;    //外观,取值由客户端定义
	UINT16	  cityID;	 //城市编号
};

//初始化客户端 enGameFrameCmd_SC_InitClient
struct SC_InitClient
{
	UID         m_uidActor;  //主角
	TSceneID    m_SceneID;

	char        m_ChatServerIp[IP_LEN];                      //聊天服务器IP
	UINT16      m_ChatServerPort;                            //聊天服务器端口	


	char     m_szServerCnfgVersion[MD5_LEN];  //服务器端配置文件MD5

	char     m_szServerResVersion[MD5_LEN];  //服务器端资源文件MD5
	
	INT32    m_FileTotalSize;  //文件总大小，如果为
	
};


//踢下线enGameFrameCmd_SC_Kick

//踢下线原因
enum enKickType  VC_PACKED_ONE
{
	enKickType_Repeat = 0,  //同一帐号重复登陆
	enKickType_Manager = 1,  //被网管踢下线
	enKickType_Stop    = 2,  //停机维护
	enKickType_SealNo  = 3,	 //账号已冻结，请联系客服

} PACKED_ONE;

struct SC_KickOffLine
{
	enKickType  m_KickType;  //类型

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

 //检查配置文件 enGameFrameCmd_CS_CheckCnfg,  
struct CS_CheckCnfg_Req
{
	char   m_szClientFileVersion[MD5_LEN];  //客户端配置文件MD5
	//char        m_szFileName[];  //文件名
};


enum enCheckCnfgRetCode VC_PACKED_ONE
{
	enCheckCnfgRetCode_Ok = 0,
	enCheckCnfgRetCode_Nonexist , //文件不存在

} PACKED_ONE;

struct FileUpdateInfo
{
	FileUpdateInfo()
	{
		MEM_ZERO(this); 
	}

	char     m_szFileCnfgVersion[MD5_LEN];  //文件MD5
	INT32    m_FileSize;  //文件总大小，即m_FileData的大小
	//char      m_szFileName[];  //文件名
	//UINT8       m_FileData[];    //文件数据
};

struct SC_CheckCnfg_Rsp
{
	SC_CheckCnfg_Rsp()
	{
		MEM_ZERO(this);
	}

	enCheckCnfgRetCode m_Result;

	//FileUpdateInfo m_FileInfo;  //如果版本比对不一至，才有该字段
};

//更新配置文件
struct SC_UpdateCnfg
{
	SC_UpdateCnfg()
	{
		MEM_ZERO(this);
	}
	bool        m_bZip;      //是否压缩
	INT32       m_DataLen;       //文件长度,不包括文件名的长度
	//char        m_szFileName[];  //文件名
	//UINT8       m_FileData[];    //文件数据	
};

//进入游戏时的操作
enum enGamePoint VC_PACKED_ONE
{
	enGamePoint_CheckVersion = 1,		//检查版本号(登录服协议里)
	enGamePoint_EnterSelectServer,		//进入选服页面(登录服协议里)
	enGamePoint_EnterCreateActor,		//进入创建角色页面(游戏服协议里)
}PACKED_ONE;

//发送游戏节点
struct CS_SendPoint
{
	UINT32				m_Vid;		//虚拟ID
	TUserID				m_UserID;	//玩家ID
	enGamePoint			m_Operator;	//操作
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)








#endif
