
#ifndef __THINGSERVER_RES_OUTPUT_PART_H__
#define __THINGSERVER_RES_OUTPUT_PART_H__

#include "IResOutputPart.h"
#include "GameSrvProtocol.h"
#include "Building.h"
#include "DBProtocol.h"
#include <vector>


struct IActor;


class ResOutputPart : public IResOutputPart
{
public:
    ResOutputPart();
	virtual ~ResOutputPart();

public:
		//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void) ;

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient() ;
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close() ;

	//保存数据
	virtual void SaveData();

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata);

public:
	   //打开建筑
	virtual void OpenBuilding(CS_OpenBuilding_Req & Req);

	 //领取资源
	virtual void TakeRes(CS_TakeRes_Req & Req);

	//代收
	virtual void CollectionOther(CS_CollectionOther_Req & Req);
	
	//访问别人的建筑
	virtual void VisitOtherBuilding(CS_VisitOtherBuilding_Req & Req);

		//获得当天偷窃数量
	virtual INT32 GetTotalFilchResOfDay(enBuildingType BuildingType);

	//手动设置上次领取时间，用于资源快熟
	virtual void SetLastTakeResTime(enBuildingType  BuildType, UINT32 nTime);

	//查看指定资源建筑记录
	virtual void ViewBuildingRecord(enBuildingType BuildingType);

	//得到资源建筑信息
	virtual void SynResBuildInfo();

	//把建筑同步信息发给其它玩家
	virtual void SendSynResBuildInfo(IActor * pTargetActor);

	//得到今天的总代收灵石量
	virtual UINT32 GetTotalCollectToday();

	//设置自动收取灵石
	virtual void SetAutoTakeRes(bool bAutoTakeRes);

	//领取资源
	virtual INT32 TakeResource(enBuildingType m_BuildingType, INT32 SetTakeNum = 0);

	//获取是否设置自动获取灵石
	virtual bool GetbOpenAutoTakeRes();

private:
	IResOutputBuilding * GetBuildingFromGameWorld(UID uidBuilding);

	IResOutputBuilding * GetBuilding(UINT8 BuildingType);

	void SetBuilding(IBuilding * pBuilding);

	void __OpenBuilding(IResOutputBuilding * pBuilding);

	//保存建筑数据
	void SaveBuildingToDB(UID uidBuildint);

	//获取建筑记录
	void LoadBuildingRecord();

	//获取建筑记录的回调
	void	HandleLoadBuildingRecord(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//保存建筑记录
	void SaveBuildingRecordToDB(UID uidBuilding);

	//获得代收资源上限
	INT32 ResOutputPart::GetBuildingResFilchMax();

protected:
	IActor * m_pActor;
 
	UID    m_BuildingData[ResOutputBuildingType_Max];

	bool	m_bFirst;

};




#endif

