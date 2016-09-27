#ifndef __THINGSERVER_IMAILPART_H__
#define __THINGSERVER_IMAILPART_H__

#include "DSystem.h"
#include "BclHeader.h"
#include <string>
#include "DBProtocol.h"
#include "IThingPart.h"

struct SMailData
{
	SMailData(){
		memset(this, 0, sizeof(*this));
	}

	TMailID			m_MailID;
	enMailType		m_MailType;							//邮件类型
	UID				m_uid_SendUser;						//邮件的发送者
	char			m_szSendUserName[THING_NAME_LEN];	//发送者的名字
	UINT32			m_Stone;							//邮寄的灵石数量
	UINT32			m_Money;							//邮寄的仙石数量
	UINT32			m_Ticket;							//邮寄的礼卷数量
	UINT32			m_PolyNimbus;						//聚灵气
	TSynID			m_SynID;							//帮派ID
	char			m_szThemeText[DESCRIPT_LEN_50];		//主题描述
	char			m_szContentText[DESCRIPT_LEN_600];	//文字内容
	UINT64			m_Time;								//发送时间
	UINT8			m_bRead;							//是否已读
	SDBGoodsData	m_SDBGoodsData[MAX_MAIL_GOODS_NUM];	//物品的数据
	

	bool IsHaveGoods(){
		if( m_Stone != 0 || m_Money != 0 || m_Ticket != 0){
			return true;
		}

		for( int i = 0; i < MAX_MAIL_GOODS_NUM; ++i)
		{
			if( m_SDBGoodsData[i].m_uidGoods.IsValid()){
				return true;
			}
		}
		return false;
	}
};

//写系统邮件
struct SWriteSystemData
{
	SWriteSystemData(){
		memset(this, 0, sizeof(*this));
	}
	UID				m_DestUID;							//邮件接收者UID值
	UINT32			m_Stone;							//邮寄的灵石数量
	UINT32			m_Money;							//邮寄的仙石数量
	UINT32			m_Ticket;							//邮寄的礼卷数量
	UINT32			m_PolyNimbus;						//聚灵气
	char			m_szThemeText[DESCRIPT_LEN_50];		//主题描述
	char			m_szContentText[DESCRIPT_LEN_600];	//邮件正文
	SDBGoodsData	m_SDBGoodsData[MAX_MAIL_GOODS_NUM];	//物品的数据	
};

//写私人邮件和帮派邮件
struct SWriteData
{
	SWriteData(){
		memset(this, 0, sizeof(*this));
	}

	enMailType	m_MailType;							//邮件类型

	UID			m_UidDestUser;						//收件人的UID值，如果为0则用名字去获取，不为0则直接用UID

	char		m_szDestUserName[THING_NAME_LEN];	//收件人的名字（收件人名字和UID值只需要填写一个就可以了，有UID值时最好写UID值）
	char		m_szThemeText[DESCRIPT_LEN_50];		//主题描述
	char		m_szContentText[DESCRIPT_LEN_300];	//文字内容
};

struct IMailPart : public IThingPart
{
	//显示邮件主界面
	virtual void	ViewMailMain() = 0;

	//显示邮件主界面
	virtual void	ViewMailMain(INT16 nBegin, INT16 nEnd) = 0;

	//查看邮件
	virtual void	ViewMail(TMailID MailID) = 0;
	
	//删除邮件
	virtual void	DeleteMail(TMailID MailID) = 0;

	//写邮件
	virtual void	WriteMail(const SWriteData & WriteData) = 0;

	//拒绝帮派邀请
	virtual void	RefuseSynInvite(TMailID MailID) = 0;

	//接受帮派邀请
	virtual void	AcceptSynInvite(TMailID MailID)	= 0;

	//更新邮件数据到数据库
	virtual void	UpdateMailData(TMailID MailID, const SMailData & MailData) = 0;

	//接受邮件物品
	virtual void	AcceptMailItem(TMailID MailID) = 0;

	//确认删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	virtual void	OKDeleteMail(TMailID MailID) = 0;

	//取消删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	virtual void	CancelDeleteMail(TMailID MailID) = 0;

	virtual	void	OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) = 0;

	//清除邮件数据
	virtual void	ClearAllMailData() = 0;

	//增加邮件数量
	virtual void	AddMailNum(INT16  nAddNum = 1) = 0;
};

#endif
