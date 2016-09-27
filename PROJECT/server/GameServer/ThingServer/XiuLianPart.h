
#ifndef __THINGSERVER_XIULIAN_PART_H__
#define __THINGSERVER_XIULIAN_PART_H__

#include "IXiuLianPart.h"
#include "DBProtocol.h"
#include "ITimeAxis.h"
#include <set>
#include "IEventServer.h"


struct IActor;

class XiuLianPart : public IXiuLianPart,public ITimerSink, public IEventListener
{
	//定时器ID
	enum enXiuLianTimerID
	{
		enXiuLianTimerID_AloneXL = 0,  //独自修炼
		enXiuLianTimerID_TwoXL,		   //双修
		enXiuLianTimerID_UnloadEmployee,	//解雇
	};

public:
	XiuLianPart();
	virtual ~XiuLianPart();

public:
	virtual void OnTimer(UINT32 timerID);

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
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	virtual void OnEvent(XEventData & EventData);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata);

	//打开
	virtual void Open(CS_OpenXiuLian_Req & Req);

	//独自修炼请求
	virtual void AloneXiuLian(const CS_AlongXiuLian_Req * pAlongXiuLianReq,const UID* pActorUid);

	virtual INT32 AloneXiuLian(INT32 nHours); 

	//取消独自修炼
	virtual void CancelAloneXiuLian();

	//发起双修请求
	virtual void TwoXiuLian(CS_TwoXiuLian_Req & Req);

	//取消双修
	virtual void CancelTwoXiuLian();


	//发起修炼法术请求
	virtual void MagicXiuLian(CS_MagicXiuLian_Req & Req);

	//同意修炼法术
	virtual void AcceptMagicXiuLian(CS_AcceptMagicXiuLian_Req & Req);

	//拒绝修炼法术
	virtual void RejectMagicXiuLian(CS_RejectMagicXiuLian_Req & Req);

	//取消修炼法术
	virtual void CancelMagicXiuLian(CS_CancelMagicXiuLian_Req & Req);

    //查看请求数据
	virtual void ViewAskData(/*CS_ViewXiuLianAskData_Req & Req*/);

	//往请求列表加条记录
	virtual bool AddToAskList(UINT32 AskID);

	//移除请求列表中的记录
	virtual void RemoveAsk(UINT32 AskID);

	//开始修炼法术
	virtual void StartMagicXiuLian(STwoXiuLianData * pAskXiuLianData);

	//同步法术修炼数据
	virtual void SyncMagicXiuLianData();

	//检测这个请求号是否在玩家的请求列表中
	virtual bool IsInAskList(UINT32 AskReqID);

	//取消前面发起的修炼法术请求
	virtual void CancelMagicAsk();

	//获得双休费用
	virtual INT32 GetTwoXLCharge(INT32 nHours);

	//停止修炼法术
    virtual void StopMagicXiuLian();

	//获得双修剩余时间
	virtual UINT32 GetTwoXiuLianRemainTime();

	//获得单修剩余时间
	virtual UINT32 GetAloneXiuLianRemainTime();

	//得到正在修炼中的角色UID
	virtual void   GetInXiuLianActorUID();

private:
	//独自修炼完成
	void FinishAloneXiuLian();

	//双修完成
    void FinishTwoXiuLian();

	//同步双修炼数据
	void SyncTwoXiuLianData();

	//同步独自修炼数据
	void SyncAloneXiuLianData();

	//获得修炼余下时间
	UINT32 GetXiuLianRemainTime(enXiuLianType XiuLianType);

	//获取独自修炼人数
	INT32 GetAloneXiuLianActorNum();

	//获得独自修炼费用
	INT32 GetAloneXLCharge(INT32 ActorNum,INT32 nHours);

	//启动修炼定时器,bCreateUser为是否创建时创建的 定时器
	void StartTimer(enXiuLianTimerID TimerID, bool bCreateUser = false);

	//停止定时器
    void StopTimer(enXiuLianTimerID TimerID);

	//请求双休
    INT32 AskTwoXiuLian(UID uidFriend,INT32 nHours, const char * FriendName);

	//获得下一个序列号
	UINT32  GetNextAskSeq();

	//根据序列号获得请求数据
	STwoXiuLianData * GetAskXiuLianData(UINT32 AskSeq);

	//请求修炼法术
	INT32 AskMagicXiuLian(UID uidFriend,enXiuLianMode Mode,TMagicID MagicID,const char * FriendName);

	//获取修炼法术需要的物品GoodsID
	TGoodsID GetMagicXiuLianNeedGoodsID();

	//法术修炼时间
	INT32 GetMagicXiuLianNeedTime();

	//增加一个修炼请求
	bool AddAskXiuLianData(STwoXiuLianData & XiuLianData);

	//删除一个修炼请求
	void DelAskXiuLianData(STwoXiuLianData & XiuLianData);

	//获得角色的修炼状态
    enXiuLianType GetXiuLianType(UID uidActor);

	//判断角色是否是有效的角色
     bool IsValidActor(UID uidActor);

	 //发送邮件
	 void SendEmail(enXiuLianType XiuLianType, const UID & uidDestUser, TMagicID MagicID = 0);

	 //获取双修灵气
	 void Take_TwoXLNimbus();

	 //获取单修灵气
	 void Take_AloneXLNimbus();

	 //请求双修数据库回调
	void	HandleTwoXiuLian(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//通知客户端显示修炼公告
	void	NoticeViewXiuLianMsg(enXiuLianType m_XiuLianType, UINT32 m_RemainTime);

	//中途取消显示修炼公告
	void	NoticeCancelViewXLMsg(enXiuLianType m_XiuLianType);

private:
	IActor*        m_pActor;

	SDBXiuLianData m_XiuLianData;

	typedef std::set<UINT32>  ASK_DATA;
	ASK_DATA  m_AskXiuLianData; //所有的修炼数据

	bool		   m_bAloneTimerInCreateUser;	//单修定时器是否是在创建角色时创建的
	bool		   m_bTwoTimerInCreateUser;		//双修定时器是否是在创建角色时创建的
};




#endif

