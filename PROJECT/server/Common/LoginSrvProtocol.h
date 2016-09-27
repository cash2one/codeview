
#ifndef __XJCQ_LOGIN_SRV_PROTOCOL_H__
#define __XJCQ_LOGIN_SRV_PROTOCOL_H__

#include "DSystem.h"

#include "ISocketSystem.h"

#include "ProtocolHeader.h"



#pragma pack(push,1)

//登陆模块消息命令字
enum enLoginCmd VC_PACKED_ONE
{
	enLoginCmd_CS_Login            = 0,  //登陆
    enLoginCmd_SC_LoginSuccess     = 1,  //登陆成功
	enLoginCmd_SC_LoginFaile       = 2,  //登陆失败
	enLoginCmd_SC_ServerList       = 3 , //游戏服务器列表
    enLoginCmd_CS_ServerState      = 4 , //客户端请求游戏服务器状态
	enLoginCmd_SC_ServerState      = 5 , //服务器应答游戏服务器状态
	enLoginCmd_CS_Register         = 6,  //玩家注册请求
	enLoginCmd_SC_RegisterSuccess  = 7,  //玩家注册成功
	enLoginCmd_SC_RegisterFaile    = 8,  //玩家注册失败
	enLoginCmd_SC_Finish           = 9,  //登陆结束



	enLoginCmd_CS_CheckVersion     = 10, //检查版本更新
	enLoginCmd_SC_CheckVersion     = 11, //检查版本更新返回
	enLoginCmd_SC_MyServerList     = 14, //我的服务器列表
	enLoginCmd_CS_CheckVersionEx   = 15, //检查版本更新

	enLoginCmd_CS_SendPoint		   = 16,	//发送节点
	enLoginCmd_SC_SendPoint		   = 17,	//发送节点返回


	enLoginCmd_Max,

} PACKED_ONE;


//登陆模块错误码
enum enLoginRetCode VC_PACKED_ONE
{
	enLoginRetCode_OK            = 0,  //OK 
	enLoginRetCode_ErrDB,              //数据库操作失败
	enLoginRetCode_ErrExistName,       //已用户名已存在

	enLoginRetCode_ErrPwd,  //密码错误
	enLoginRetCode_ErrVersion, //客户端版本太低
	enLoginRetCode_ErrNoUser,  //用户不存在
	enLoginRetCode_ErrKeyName,       //用户名包含有敏感关键词
	enLoginRetCode_ErrSealNo,      //帐号已被封
	enLoginRetCode_ErrDefend,      //服务器维护中

	enLoginRetCode_ErrUnknow,

	enLoginRetCode_Max,

} PACKED_ONE;

////////////////////////////////////////////////////////////////////////////////////////////

//登陆消息包头
struct LoginHeader : public AppPacketHeader
{
	//length 不包括LoginHeader本身的长度
	LoginHeader( enLoginCmd enCmd,UINT32 length)
	{
		MsgCategory = enMsgCategory_Login;
		command = enCmd;
		this->m_length = SIZE_OF(*this)+length;
	}
};

//用户来源渠道
enum enChanelType VC_PACKED_ONE
{	
	enChanelType_XunYou = 0,  //讯游
	enChanelType_XinLang  =1 , //新浪
	enChanelType_DangLe   =2 , //当乐
	enChanelType_NineOne   =3 , //九一
	enChanelType_Max,

} PACKED_ONE;


//登陆请求
struct CS_Login_Req
{	
    char  ClientVersion[VERSION_LEN];              //客户端版本号
	char   szUserName[USER_NAME_LEN];  //用户名
	
	char   szPassword[SHA256_LEN];        //密码,Access Token

	enChanelType  ChanelType;  //渠道
	char   szAccessSecret[ACCESS_SECRET_LEN]; //访问密钥

};


//登陆成功应答
struct SC_Login_Success_Rsp
{	
	SC_Login_Success_Rsp()
	{
		MEM_ZERO(this);
	}
	TUserID   UserID;                 //用户ID
	UINT32    ServerTime;             //服务器当前时间
	UINT8      szTicket[TICKET_LEN];  //登陆游戏需要用到的票
};

//登陆失败应答
struct SC_Login_Faile_Rsp
{	
	enLoginRetCode  m_Result;		//失败描述
};



//游戏服务器信息
struct SGameServerInfo
{
	TServerID     ServerID;                              //服务器ID
	char          szServerName[GAME_SERVER_NAME_LEN];    //服务器名称
	char          ServerIp[IP_LEN];                      //服务器IP
	UINT16        ServerPort;                            //服务器端口
	enServerState State;                                 //服务器状态
	bool          bCommend;                              //是否推荐
	bool          bNew;                                  //是否是新服
};


//服务器列表
struct SC_Server_List_Syn
{
	UINT16            ServerNum;          //服务器数量
  //  SGameServerInfo ServerInfoList[1];  //服务器信息列表

};



struct SServerStateInfo
{
	TServerID     ServerID; 
	enServerState State;         //服务器状态
};

//向服务器请求服务器状态
struct CS_Server_State_Req
{	
	TUserID   UserID;  //用户ID     
};

//服务器状态
struct SC_Server_State_Rsp
{
    UINT16             ServerNum;      //服务器数量
	//SServerStateInfo StateInfoList[1];
};




//玩家注册
struct CS_Register_Req
{	
	char  ClientVersion[VERSION_LEN];              //客户端版本号

	char   szUserName[USER_NAME_LEN];  //用户名
	
	char   szPassword[SHA256_LEN];        //密码

	enCryptType  m_CryptType;  //只支持明文或SHA256
};

//玩家注册成功
struct SC_Register_Success_Rsp
{	
	TUserID   UserID;  //用户ID	
	UINT32    ServerTime; //服务器当前时间
	char      szTicket[TICKET_LEN];  //登陆游戏需要用到的票
};


//玩家注册失败
struct SC_Register_Faile_Rsp
{	
	enLoginRetCode m_Result;
};


//检查版本更新 enLoginCmd_CS_CheckVersion 
struct CS_CheckVersion_Req
{
	char  ClientVersion[VERSION_LEN];              //客户端版本号
};

//检查版本更新 enLoginCmd_CS_CheckVersion 
struct CS_CheckVersionEx_Req
{
	char          ClientVersion[VERSION_LEN];              //客户端版本号
	enChanelType  ChanelType;  //渠道
};

//版本检测结果
enum enCheckVersionCode VC_PACKED_ONE
{	
	enCheckVersionCode_OK   = 0,  //已是最新版本
	enCheckVersionCode_Can  = 1 , //不是最新，可以更新
	enCheckVersionCode_Must  =2 , //版本太低，必须更新
	enCheckVersionCode_Max,
} PACKED_ONE;

struct SC_CheckVersion_Rsp
{
	enCheckVersionCode m_Result;	
	//char    szPublicNotice[];            //公告，以\0结束
	//char  szUpdateDesc[];                //更新说明，以\0结束
    //char  szInstallPackage[];            //最新安装包url
	//char      m_szServiceTel[]; //客服电话
	//char      m_szServiceEmail[]; //客服Email
	//char      m_szServiceQQ[];    //客服QQ
	//char    szResUrl[];                   //资源下载路径
	
};

//我的服务器列表 enLoginCmd_SC_MyServerList
struct SC_MyServerList
{
	UINT16     ServerNum;
	//UINT16   ServerList[];   //服务器ID列表
};

//进入游戏时的操作
enum enEnterGameOperator VC_PACKED_ONE
{
	enEnterGameOperator_CheckVersion = 1,		//检查版本号(登录服协议里)
	enEnterGameOperator_EnterSelectServer,		//进入选服页面(登录服协议里)
	enEnterGameOperator_EnterCreateActor,		//进入创建角色页面(游戏服协议里)
}PACKED_ONE;


/*********************************************************
虚拟ID：刚安装包时为0，在第一次检查版本号时，服务器会返回一个
	虚拟ID,客户端保存，以后每次都发这个虚拟ID

检查版本号：此时没有UserID,可发0
**********************************************************/
//发送节点
struct CS_SendPoint_Login
{
	UINT32				m_Vid;		//虚拟ID
	TUserID				m_UserID;	//玩家ID
	enEnterGameOperator	m_Operator;	//操作
};

//发送节点返回
struct SC_SendPoint
{
	UINT32				m_Vid;		//虚拟ID
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)


#endif
