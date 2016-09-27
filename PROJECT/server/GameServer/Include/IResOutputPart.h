
#ifndef __THINGSERVER_IRES_OUTPUT_PART_H__
#define __THINGSERVER_IRES_OUTPUT_PART_H__

#include "IThingPart.h"
#include "GameSrvProtocol.h"
#include "DBProtocol.h"

struct IActor;

struct IResOutputPart : public IThingPart
{
   //打开建筑
	virtual void OpenBuilding(CS_OpenBuilding_Req & Req) = 0;

	 //领取资源
	virtual void TakeRes(CS_TakeRes_Req & Req) = 0;

	//代收
	virtual void CollectionOther(CS_CollectionOther_Req & Req) = 0;

	//访问别人的建筑
	virtual void VisitOtherBuilding(CS_VisitOtherBuilding_Req & Req) = 0;

	//手动设置上次领取时间，用于资源快熟
	virtual void SetLastTakeResTime(enBuildingType  BuildType, UINT32 nTime) = 0;

	//查看指定资源建筑记录
	virtual void ViewBuildingRecord(enBuildingType BuildingType) = 0;

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) = 0;

	//同步资源建筑信息
	virtual void SynResBuildInfo() = 0;

	//把建筑同步信息发给其它玩家
	virtual void SendSynResBuildInfo(IActor * pTargetActor) = 0;

	//得到今天的总代收灵石量
	virtual UINT32 GetTotalCollectToday() = 0;

	//设置自动收取灵石
	virtual void SetAutoTakeRes(bool bAutoTakeRes) = 0;

	//领取资源
	virtual INT32 TakeResource(enBuildingType m_BuildingType, INT32 SetTakeNum = 0) = 0;

	//获取是否设置自动获取灵石
	virtual bool GetbOpenAutoTakeRes() = 0;
};

#endif

