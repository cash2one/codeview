
#ifndef __XJCQ_SERVER_PROTOCOL_H__
#define __XJCQ_SERVER_PROTOCOL_H__

#include "DSystem.h"

#include "ISocketSystem.h"

#include "ProtocolHeader.h"


#pragma pack(push,1)

//服务器之间的消息命令字
enum enServerCmd VC_PACKED_ONE
{
	enServerCmd_SS_LoginLoginSrv            = 0,  //连接登陆服
	enServerCmd_SS_ReportState              = 1 , //汇报状态
	enServerCmd_SS_LoginChatSrv             = 2,  //连接聊天服
	enServerCmd_Max,

} PACKED_ONE;

////////////////////////////////////////////////////////////////////////////////////////////

//服务器间消息包头
struct ServerHeader : public AppPacketHeader
{
	//length 不包括ServerHeader本身的长度
	ServerHeader( enServerCmd enCmd,UINT32 length)
	{
		MsgCategory = enMsgCategory_GameServer;
		command = enCmd;
		this->m_length = SIZE_OF(*this)+length;
	}
};


//服务器状态
/*enum enServerState VC_PACKED_ONE
{
	enServerState_Fine = 0,  //良好
	enServerState_Common = 1, //一般
	enServerState_Full = 2,   //爆满
    
} PACKED_ONE;
*/

//游戏服务器信息
struct SS_Login_LoginSrv_Req
{
	TServerID   ServerID;                              //服务器ID
	char        szServerName[GAME_SERVER_NAME_LEN];    //服务器名称
	char      ServerIp[IP_LEN];                         //服务器IP
	UINT16      ServerPort;                            //服务器端口
	enServerState State;                               //服务器状态
	bool        bCommend;                              //是否推荐
	bool        bNew;                                  //是否是新服
};


//汇报状态
struct SS_Report_State_Req
{
	TServerID   ServerID; 
	enServerState State;         //服务器状态
};

//连接聊天服务器
struct SS_Login_ChatSrv_Req
{
	TServerID   ServerID;                              //服务器ID
	char        szServerName[GAME_SERVER_NAME_LEN];    //服务器名称
};

//登陆服务通过客户端传递给游戏服的数据
struct SS_UserPrivateData
{
	TUserID    m_UserID;
	UINT8      m_IdentityStatus; //认证状态
	INT32      m_OnlineTime;  //在线时间，单位:秒

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)


#endif
