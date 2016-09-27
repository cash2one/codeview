
#ifndef __THINGSERVER_IResOutputBuilding_H__
#define __THINGSERVER_IResOutputBuilding_H__

#include "IThing.h"
#include "GameSrvProtocol.h"
#include "DBProtocol.h"

struct IActor;

struct IBuilding : public IThing
{
	virtual enBuildingType GetBuildingType() = 0;

	virtual void SetSceneID(TSceneID SceneID) = 0;

	virtual TSceneID GetSceneID() = 0;
};


//产出资源建筑
struct IResOutputBuilding : public IBuilding
{	 
	//上次领取时间
	virtual  UINT32 GetLastTakeResTime()=0;

	//资源产出余下时间
	virtual INT32 GetOutputRemainingTime()=0;

	//领取资源
	virtual  INT32 TakeRes()=0;


	//被pActor代收资源
	virtual   INT32 Collectioned(IActor* pActor,INT32 maxNum)=0;

	//当天已代收资源数
	virtual   INT32 GetTotalCollectResOfDay() = 0;

	//增加当天代收资源数
	virtual  bool AddCollectRes(INT32 Num) = 0;


	//手动设置上次领取时间，用于资源快熟
	virtual  void SetLastTakeResTime(UINT32 nTime) = 0;

	//记录别人对自己的操作
	virtual  void RecordHandleOtherUser(const char * pszTargetUserName, const char * pszHandleName, const char * pszContext) = 0;

	//保存记录
	virtual void SaveBuildingRecordToDB(UID uidUser) = 0;

	//加进记录
	virtual void Push(const BuildingRecordData & RecordData) = 0;

	//查看指定资源建筑记录
	virtual void ViewBuildingRecord() = 0;

	//得到主人UID
	virtual UID  GetMasterUID() = 0;

	//设置自动收取灵石
	virtual void SetAutoTakeRes(bool bAutoTakeRes) = 0;

	virtual bool GetAutoTakeRes() = 0;

	//上线时，如果有自动收取灵石，则自动收取
	virtual void Online_AutoTake() = 0;
};

#endif


