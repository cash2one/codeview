
#ifndef __THINGSERVER_XIULIANMGR_H__
#define __THINGSERVER_XIULIANMGR_H__

#include "TBuffer.h"
#include "IMessageDispatch.h"
#include "IDBProxyClient.h"
#include "ITimeAxis.h"
#include <map>

struct IActor;

class XiuLianMgr : public IMsgRootDispatchSink
                 , public IDBProxyClientSink
				 ,public ITimerSink
{
		//定时器ID
	enum enXiuLianTimerID
	{
		enXiuLianTimerID,        //双修
	
	};

public:
   XiuLianMgr();

   virtual ~XiuLianMgr();

   bool Create();

   void Close();

public:
		//卸载修炼数据
    void UnloadXiuLianRecord(UINT32 AskSeq);

public:

	virtual void OnTimer(UINT32 timerID);

		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib);

		// 前置机回调
	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0) ;

private:
	
	//打开
	void OpenXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//请求独自修炼
	void AskAloneXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//取消独自修炼
	void CancelAloneXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//请求双修
	void AskTwoXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//取消双修
	void CancelTwoXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

			//请求修炼法术
	void AskMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//接受修炼法术
	void AcceptMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//拒绝修炼法术
	void RejectMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//取消修炼法术
	void CancelMagicXiuLian(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//查看请求数据
	void ViewAskXiuLianData(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//取消修炼请求
	void CancelAskSeq(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到影响修炼的元素
	void GetEffectXLElement(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//得到正在修炼中的角色UID
	void GetInXiuLianActorUID(IActor *pActor,UINT8 nCmd, IBuffer & ib);

private:

    void MagicXiuLianOnTimer(STwoXiuLianData * pAskXiuLianData);

	//保存数据到数据库
	void SaveToDB(UINT32 AskSeq);

	void SaveToDB(STwoXiuLianData * pAskXiuLianData);

public:
	//增加一个修炼请求
bool  AddAskXiuLianData(STwoXiuLianData & XiuLianData);

//删除一个修炼请求
bool  DelAskXiuLianData(UINT32 AskSeq);

//根据序列号获得请求数据
STwoXiuLianData * GetAskXiuLianData(UINT32 AskSeq);

 //玩家上线时，把未加到玩家身上的请求加给玩家
 std::vector<UINT32> Check_AddToAskList(IActor * pActor);

private:
	typedef std::map<UINT32,STwoXiuLianData>  MAP_ASK_DATA;
	MAP_ASK_DATA  m_AskXiuLianData; //所有的修炼数据
};




#endif

