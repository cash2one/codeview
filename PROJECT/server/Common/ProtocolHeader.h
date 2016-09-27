
#ifndef __PROTOCOL_HEADER_H__
#define __PROTOCOL_HEADER_H__

#include "ISocketSystem.h"

/*/////////////////////////////////////////////////////////////////////////////

命令字命名规范：
1）仅客户端发给服务器  CS
2) 仅服务器发给客户端  SC
3) 即可以是服务器发给客户端，也可以是客户端发给服务器 CSC
4) 服务器发给服务器    SS
5) 客户端发给客户端    CC
5) 服务器和DB前端      DB


//////////////////////////////////////////////////////////////////////////////*/


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//服消息分类定义
enum enMsgCategory VC_PACKED_ONE
{
	enMsgCategory_DB         = 0,      //DB消息,客户端不使用该消息
	enMsgCategory_Login      = 1,      //登陆消息
	enMsgCategory_GameServer = 2,      //来处GameServer的消息
    enMsgCategory_GameFrame  = 3,      //游戏服框架消息
	enMsgCategory_GameWorld  = 4,      //游戏世界消息
	enMsgCategory_Building   = 5,      //建筑消息
	enMsgCategory_Packet     = 6,      //背包消息
	enMsgCategory_Equip      = 7,      //装备
	enMsgCategory_XiuLian    = 8,      //修练
	enMsgCategory_ShopMall	 = 9,	   //商城
	enMsgCategory_Relation	 = 10,	   //好友
	enMsgCategory_Mail		 = 11,	   //邮件
	enMsgCategory_Talk		 = 12,	   //聊天
	enMsgCategory_Trade		 = 13,	   //交易
   	enMsgCategory_Combat     = 14,     //战斗
	enMsgCategory_Syndicate  = 15,	   //帮派
	enMsgCategory_GodSwordShop = 16,   //剑冢
	enMsgCategory_TalismanWorld = 17,  //法宝世界
	enMsgCategory_MainUI       = 18,   //主界面
	enMsgCategory_DouFa        = 19,   //斗法
	enMsgCategory_Task         = 20,   //任务
	enMsgCategory_ChengJiu     = 21,   //成就
	enMsgCategory_Activity     = 22,   //活动
	enMsgCategory_ChatServer   = 23,   //聊天服
	enMsgCategory_Rank		   = 24,   //排行
	enMsgCategory_WebServer    = 25,   //web服务器
	enMsgCategory_Team		   = 26,   //组队
	enMsgCategory_MiJing	   = 27,   //秘境


	enMsgCategory_Max,
} PACKED_ONE ;

////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push,1)

//应用层通讯包头
struct AppPacketHeader : public NetPacketHeader
{
	AppPacketHeader()
	{
		MsgCategory = enMsgCategory_Max;
		command = 0;
		this->m_length = sizeof(*this);
	}
	enMsgCategory        MsgCategory;  //消息分类	
	UINT8               command;       //命令字
};



//服务器状态
enum enServerState VC_PACKED_ONE
{
	enServerState_Fine = 0,  //良好
	enServerState_Common = 1, //一般
	enServerState_Full = 2,   //爆满
	enServerState_Close = 3, //不能连接
    
} PACKED_ONE;

////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)

#endif
