
#ifndef __THINGSERVER_BUILDING_MGR_H__
#define __THINGSERVER_BUILDING_MGR_H__

#include "IMessageDispatch.h"
#include "IGameWorld.h"
#include "ITimeAxis.h"
#include "IDBProxyClient.h"

#include <hash_map>

//建筑管理信息
struct SBuildingInfo
{
	UINT32    m_UnloadTime; //卸载时间
};


class BuildingMgr : public IMsgRootDispatchSink, public IDBProxyClientSink
{
	
public:
	BuildingMgr();
	~BuildingMgr();

	bool Create();

	void Close();

public:
		//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

private:
	//打开建筑
	void  BuildingMgr::OpenBuilding(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//领取资源
	void  BuildingMgr::TakeRes(IActor *pActor,UINT8 nCmd, IBuffer & ib);

			//代收
	void  BuildingMgr::Collection(IActor *pActor,UINT8 nCmd, IBuffer & ib);

		//访问别人的建筑
	void  BuildingMgr::VisitBuilding(IActor *pActor,UINT8 nCmd, IBuffer & ib);


	//伏魔洞处理
	void  BuildingMgr::EnterFuMoDong(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::AutoKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::CancelKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::AccelKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::KillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib);


	//练功堂处理
	void  BuildingMgr::EnterTrainingHall(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::StratTraining(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::StopTraining(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//进入后山
	void  BuildingMgr::EnterHouShan(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//聚仙楼处理
	void  BuildingMgr::EnterGatherGod(IActor  *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::ViewGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::FlushGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::BuyGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib);

	//剑冢处理
	void  BuildingMgr::EnterGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::FlushGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::BuyGodSword(IActor  *pActor, UINT8 nCmd, IBuffer & ib);
	void  BuildingMgr::SyncGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//查看建筑记录
	void  BuildingMgr::ViewBuildingRecord(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//立即完成练功
	void  BuildingMgr::TrainingFinishNow(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//自动刷新招募角色
	void  BuildingMgr::AutoFlushEmploy(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//设置自动收取灵石
	void  BuildingMgr::SetAutoTakeRes(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//开始习武
	void  BuildingMgr::StartXiWu(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

	//取消习武
	void  BuildingMgr::CancelXiWu(IActor  *pActor, UINT8 nCmd, IBuffer & ib);

private:


};





#endif
