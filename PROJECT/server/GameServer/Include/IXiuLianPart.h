
#ifndef __THINGSERVER_IXIULIAN_PART_H__
#define __THINGSERVER_IXIULIAN_PART_H__

#include "IThingPart.h"
#include "GameSrvProtocol.h"
#include "DBProtocol.h"


struct IXiuLianPart : public IThingPart
{
	//打开
	virtual void Open(CS_OpenXiuLian_Req & Req)  = 0;

	//独自修炼请求
	virtual void AloneXiuLian(const CS_AlongXiuLian_Req * pAlongXiuLianReq,const UID* pActorUid) = 0;

	virtual INT32 AloneXiuLian(INT32 nHours) = 0;

	//取消独自修炼
	virtual void CancelAloneXiuLian() = 0;

	//发起双修请求
	virtual void TwoXiuLian(CS_TwoXiuLian_Req & Req) = 0;

	//取消双修
	virtual void CancelTwoXiuLian() = 0;


	//发起修炼法术请求
	virtual void MagicXiuLian(CS_MagicXiuLian_Req & Req) = 0;

	//同意修炼法术
	virtual void AcceptMagicXiuLian(CS_AcceptMagicXiuLian_Req & Req) = 0;

	//拒绝修炼法术
	virtual void RejectMagicXiuLian(CS_RejectMagicXiuLian_Req & Req) = 0;

	//取消修炼法术
	virtual void CancelMagicXiuLian(CS_CancelMagicXiuLian_Req & Req) = 0;


    //查看请求数据
	virtual void ViewAskData(/*CS_ViewXiuLianAskData_Req & Req*/) = 0;

	virtual void StopMagicXiuLian() = 0;

	//往请求列表加条记录
	virtual bool AddToAskList(UINT32 AskID) = 0;

	//移除请求列表中的记录
	virtual void RemoveAsk(UINT32 AskID) = 0;

	//开始修炼法术
	virtual void StartMagicXiuLian(STwoXiuLianData * pAskXiuLianData) = 0;

	//同步法术修炼数据
	virtual void SyncMagicXiuLianData() = 0;

	//检测这个请求号是否在玩家的请求列表中
	virtual bool IsInAskList(UINT32 AskReqID) = 0;

	//取消前面发起的修炼法术请求
	virtual void CancelMagicAsk() = 0;

	//获得双休费用
	virtual INT32 GetTwoXLCharge(INT32 nHours) = 0;

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) = 0;

	//获得双修剩余时间
	virtual UINT32 GetTwoXiuLianRemainTime() = 0;

		//获得单修剩余时间
	virtual UINT32 GetAloneXiuLianRemainTime() = 0;

	//得到正在修炼中的角色UID
	virtual void   GetInXiuLianActorUID() = 0;
};



#endif
