
#ifndef __XJCQ_CHATSRV_CHATSRV_CMD_PROTOCOL_H__
#define __XJCQ_CHATSRV_CHATSRV_CMD_PROTOCOL_H__

#include "DSystem.h"
#include "BclHeader.h"
#include "UniqueIDGenerator.h"
#include "ProtocolHeader.h"




#pragma pack(push,1)

//聊天服消息命令字定义
enum enChatSrvCmd VC_PACKED_ONE
{
	enChatSrvCmd_CS_Enter           = 0,  //进入聊天服
	enChatSrvCmd_SC_Enter,                //进入聊天服应答
} PACKED_ONE;



//聊天服框架消息
struct  enChatSrvCmdHeader  : public AppPacketHeader
{
	enChatSrvCmdHeader(enChatSrvCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_ChatServer;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};


//进入聊天服结果码定义
enum enEnterChatSrvRetCode VC_PACKED_ONE
{
	enEnterChatSrvRetCode_Ok = 0,
    enEnterChatSrvRetCode_ServerNotOpen, //服务器未开启
	enEnterChatSrvRetCode_Max,

} PACKED_ONE;



//进入聊天服 enChatSrvCmd_CS_Enter
struct CS_EnterChatSrv_Req
{
	UID       uidActor; //UID
	TUserID   UserID;  //用户ID
	UINT16    ServerID; //服务器ID
	TSynID	  SynID  ; //帮派ID  
	char      szName[THING_NAME_LEN];  //名称
	char      szTicket[TICKET_LEN];  //登陆返回来的票
};

struct SC_EnterChatSrv_Rsp
{
	enEnterChatSrvRetCode   m_Result;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif



