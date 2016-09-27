#ifndef __THINGSERVER_MAILPART_H__
#define __THINGSERVER_MAILPART_H__

#include "IMailPart.h"
#include "IThing.h"
#include "IThingPart.h"

struct IActor;

class MailPart : public IMailPart
{
public:
	MailPart();
	virtual ~MailPart();

public:
	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功 
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void);

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata);

public:
	//显示邮件主界面
	virtual void	ViewMailMain();

	//显示邮件主界面
	virtual void	ViewMailMain(INT16 nBegin, INT16 nEnd);

	//查看邮件
	virtual void	ViewMail(TMailID MailID);

	//删除邮件
	virtual void	DeleteMail(TMailID MailID);

	//写邮件
	virtual void	WriteMail(const SWriteData & WriteData);

	//拒绝帮派邀请
	virtual void	RefuseSynInvite(TMailID MailID);

	//接受帮派邀请
	virtual void	AcceptSynInvite(TMailID MailID);

	//更新邮件数据到数据库
	virtual void	UpdateMailData(TMailID MailID, const SMailData & MailData);

	//接受邮件物品
	virtual void	AcceptMailItem(TMailID MailID);

	//确认删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	virtual void	OKDeleteMail(TMailID MailID);

	//取消删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
	virtual void	CancelDeleteMail(TMailID MailID);

	//清除邮件数据
	virtual void	ClearAllMailData();

	//增加邮件数量
	virtual void	AddMailNum(INT16  nAddNum = 1);
	
private:
	//写邮件
	void	WriteMail(SDB_Insert_MailData_Req Req);

	void	HandleWriteMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void	HandleAddMailDataList(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);


	void	HandleLoadMailInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	
	//加载指定区间的邮件邮件
	void	LoadMailInfo(INT16	nBegin, INT16 nEnd);

	void	HandleLoadSectionMailInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//加载邮件数量
	void	LoadMailNum();

	//加载邮件数量
	void	HandleGetMailNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

private:
	bool		m_bLoadData;		//数据是否加载过(有用到邮件时才会去加载数据)

	IActor *	m_pActor;

	typedef std::hash_map<TMailID, SMailData> MAP_MAILDATA;

	MAP_MAILDATA	m_mapMailData;

	UINT16			m_HaveMailNum;		//当前有多少邮件			

	INT16			m_MaxMailNum;		//玩家邮件最多保存数量

	std::hash_map<TActorID, SDB_Insert_MailData_Req>	m_mapInsertMailData;
};

#endif
