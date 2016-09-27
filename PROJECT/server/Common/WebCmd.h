
#ifndef __JXCQ_WEB_CMD_H__
#define __JXCQ_WEB_CMD_H__



#include "DSystem.h"
#include "BclHeader.h"
#include "UniqueIDGenerator.h"
#include "ProtocolHeader.h"



#pragma pack(push,1)

//WEB服消息命令字定义
enum enWebSrvCmd VC_PACKED_ONE
{
	enWebSrvCmd_DontTalk = 0,			//禁言
    enWebSrvCmd_SealNo   = 1,			//封号
	enWebSrvCmd_Goods    = 2,			//发放物品
	enWebSrvCmd_KeyWorld  = 3,			//关键字
	enWebSrvCmd_Resource = 4,			//资源管理
	enWebSrvCmd_Mail     = 5,			//邮件
	enWebSrvCmd_Message  = 6,			//消息
	enWebSrvCmd_PublicNotic = 7,		//公告
	enWebSrvCmd_UpdateDesc  = 8,		//更新说明
	enWebSrvCmd_Version     = 9,		//版本更新
	enWebSrvCmd_Service  = 10,			//客服信息
	enWebSrvCmd_Pay		 = 11,			//支付
	enWebSrvCmd_ShopMall = 12,			//商城
	enWebSrvCmd_RechargeForward = 13,	//充值返利修改
	enWebSrvCmd_MonsterCnfg = 14,		//修改怪物配置
	enWebSrvCmd_Activity = 15,			//活动
	enWebSrvCmd_MultipExp = 16,			//多倍经验
	enWebSrvCmd_FirstRecharge = 17,		//首次充值


	enWebSrvCmd_Max,

} PACKED_ONE;



//WEB服框架消息
struct  WebSrvCmdHeader  : public AppPacketHeader
{
	WebSrvCmdHeader(enWebSrvCmd Cmd,int length)
	{
		MsgCategory = enMsgCategory_WebServer;
		this->command = Cmd;
		this->m_length = SIZE_OF(*this) + length;
	}
};

//禁言enWebSrvCmd_DontTalk
struct SS_DontTalk_Req
{
	UINT32        m_ID; //对应t_donttalk表的id
};

//封号enWebSrvCmd_SealNo
struct SS_SealNo_Req
{
	UINT32      m_ID;
};

//发放物品enWebSrvCmd_Goods
struct SS_GoodsMgr_Req
{
	UINT32		m_MailID;
};


//关键字 enWebSrvCmd_KeyWorld , 
//没有包体

//资源管理enWebSrvCmd_Resource
struct SS_ResourceMgr_Req
{
	UINT32		m_MailID;
};

//邮件管理enWebSrvCmd_Mail
struct SS_MailMgr_Req
{
	UINT32        m_MailID;
};

//消息管理
struct SS_MessageMgr_Req
{
	UINT32    m_MsgID; //消息ID
};


//维护公告
struct SS_PublicNotic_Req
{
	UINT32   m_ID;
};

//更新说明
struct SS_UpdateDesc_Req
{
	UINT32   m_ID;
};

//版本更新
struct SS_Version_Req
{
	UINT32   m_ID;
};

//客服信息
struct SS_ServiceInfo_Req
{
	UINT32	m_ID;
};

//支付
struct SS_Pay_Req
{
	bool		  m_bOk;
//  SS_Pay_Ok	  m_PayOK;		//成功发这个
//  SS_Pay_Error  m_PayError;	//失败发这个
};

struct SS_Pay_Ok
{
	UINT32	m_ID;
};

struct SS_Pay_Error
{
	TUserID		m_UserID;
	char		m_szErrorInfo[DESCRIPT_LEN_500];	//出错内容
};

//商城
struct SS_ShopMall
{
	UINT32  m_ID;
};

//活动
enum enActivity VC_PACKED_ONE
{
	enActivity_Del = 0,	//删除活动
	enActivity_Add,		//添加活动
	enActivity_Change,	//修改活动
}PACKED_ONE;

struct SS_Activity
{
	UINT16		m_ID;
	enActivity	m_enActivity;
};		
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(pop)






























#endif

