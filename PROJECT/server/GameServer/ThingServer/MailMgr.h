#ifndef __THINGSERVER_MAILMGR_H__
#define __THINGSERVER_MAILMGR_H__

#include "IMessageDispatch.h"
#include "IDBProxyClient.h"

struct KFMail_Data
{
	KFMail_Data(){
		memset(this, 0, sizeof(*this));
	}
	char szTheme[DESCRIPT_LEN_50];
	char szBody[DESCRIPT_LEN_380];
};

class MailMgr : public IMsgRootDispatchSink, public IDBProxyClientSink
{
public:
	MailMgr();
	~MailMgr();

	bool Create();

	void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//IDBProxyClientSink接口
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

private:
	//显示邮件主界面
	void	ViewMailMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看邮件
	void	ViewMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//写私人邮件
	void	WritePersonMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//写帮派邮件
	void	WriteSynMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//接受帮派邀请
	void	AcceptSynInvite(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//拒绝帮派邀请
	void	RefuseSynInvite(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//接受邮件物品
	void	AcceptMailItem(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//删除邮件
	void	DeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//确认删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	void	OKDeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//取消删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	void	CancelDeleteMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//关闭邮件窗口
	void	CloseMailWindow(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//点击选择收信人按钮
	void	ClickFriend(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//发送客服邮件
	void	SendCustomerServiceMail(IActor *pActor,UINT8 nCmd, IBuffer & ib);
};

#endif
