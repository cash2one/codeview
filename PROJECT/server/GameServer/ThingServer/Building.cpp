

#include "Building.h"
#include "XDateTime.h"
#include "IActor.h"
#include "IConfigServer.h"
#include "IGameWorld.h"
#include "ThingServer.h"
#include "IResOutputPart.h"
#include "IBasicService.h"
#include "IGameServer.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Building::Building()
{
	m_BuildingType = enBuildingType_Max;
}
Building::~Building()
{
}

		//初始化，
bool Building::Create(void)
 {
	 return true;
 }

void Building::Release()
{
	delete this;
}

enBuildingType Building::GetBuildingType()
{
	return m_BuildingType;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的公开现场
// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
//////////////////////////////////////////////////////////////////////////
bool Building::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SBuildingPublicData))
	{
		return false;
	}

	SBuildingPublicData * pPublicData = (SBuildingPublicData *)buf;

	pPublicData->m_BuildingType = m_BuildingType;

	pPublicData->m_Uid = GetUID();
	pPublicData->m_SceneID = m_SceneID;

	pPublicData->m_ResOutputRemainingTime = 0;  //资源产出余下时间

	nLen = sizeof(SBuildingPublicData);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool Building::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SBuildingPrivateData))
	{
		return false;
	}

	SBuildingPrivateData * pBuildingPrivateData = (SBuildingPrivateData *)buf;

	pBuildingPrivateData->m_BuildingType       = m_BuildingType;

	pBuildingPrivateData->m_Uid = GetUID();

	pBuildingPrivateData->m_SceneID = m_SceneID;

	pBuildingPrivateData->m_uidOwner = UID();

	pBuildingPrivateData->m_szOwnerName[0] = 0;

	pBuildingPrivateData->m_ResOutputRemainingTime = 0;  //资源产出余下时间



	nLen = SIZE_OF(*pBuildingPrivateData);

	return true;

}
bool Building::OnSetPrivateContext(const void * buf, int nLen)
{
	if(buf==0 || nLen < sizeof(SBuildingPrivateData))
	{
		return false;
	}

	SBuildingPrivateData * pPrivateData = (SBuildingPrivateData*)buf;

	m_BuildingType = (enBuildingType)pPrivateData->m_BuildingType;

	SetUID(pPrivateData->m_Uid);

	m_SceneID = pPrivateData->m_SceneID;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool Building::OnSetDBContext(const void * buf, int nLen)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool Building::OnGetDBContext(void * buf, int &nLen)
{
	return false;
}

void Building::SetSceneID(TSceneID SceneID) 
{
	m_SceneID = SceneID;
}

TSceneID Building::GetSceneID() 
{
	return m_SceneID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResOutputBuilding::ResOutputBuilding()
{
	memset(&m_BuildingData,0,sizeof(m_BuildingData));
	m_bNeedResetTimer = 0;
}

ResOutputBuilding::~ResOutputBuilding()
{
}



void ResOutputBuilding::OnTimer(UINT32 timerID)
{
	switch(timerID)
	{
	case enTIMER_ID_Guard:
		{
		}
		break;
	case enTIMER_ID_AutoTakeRes:
		{
			//先检测收灵石
			this->__TakeResource();
			
			if ( m_bNeedResetTimer){

				//设置自动收取灵石定时器
				this->SetAutoTakeResTimer(false);				
			}
		}
		break;
	default:
		break;
	}
}


//上次领取时间
UINT32 ResOutputBuilding::GetLastTakeResTime()
{
	return m_BuildingData.m_LastTakeResTime;
}

//资源产出余下时间
INT32 ResOutputBuilding::GetOutputRemainingTime()
{
	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	time_t nCurTime = CURRENT_TIME();

	time_t lastTakeTime = GetLastTakeResTime();

	INT32 RemainingTime = 0;

	if(lastTakeTime+ConfigParam.m_BuildingResOutputInterval> nCurTime)
	{
		RemainingTime = lastTakeTime+ConfigParam.m_BuildingResOutputInterval - nCurTime;
	}

	return RemainingTime;
}

//领取资源
INT32 ResOutputBuilding::TakeRes()
{	
	if(this->GetOutputRemainingTime()<=0)
	{
		//可领取
		m_BuildingData.m_LastTakeResTime = CURRENT_TIME();

		return this->GetTakeResNum();
	}

	return 0;
}


//被pActor代收资源
INT32 ResOutputBuilding::Collectioned(IActor* pActor,INT32 maxNum)
{
	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	SC_CollectionOther_Rsp Rsp;
	Rsp.m_uidBuild = GetUID();

	if(this->GetOutputRemainingTime()>0)
	{
		Rsp.m_RetCode = enFilchRetCode_ErrNoRes;
		Rsp.m_ResNum = 0;
		Rsp.m_OtherResNum = 0;
	}
	else
	{
		INT32 ResNum = this->GetTakeResNum();

		Rsp.m_RetCode = enFilchRetCode_OK;
		Rsp.m_ResNum = std::min(maxNum,ResNum * ConfigParam.m_BuildingResFilchRate / 100);
		Rsp.m_OtherResNum = ResNum * ConfigParam.m_BuildingResFilchedRate / 100;	

		pActor->AddCrtPropNum(enCrtProp_ActorStone,Rsp.m_ResNum);

		IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(this->GetMasterUID());
		if( 0 != pFriend){
			pFriend->AddCrtPropNum(enCrtProp_ActorStone,Rsp.m_OtherResNum);
		}

		m_BuildingData.m_OwnerNoTakeResNum += enCrtProp_ActorStone,Rsp.m_ResNum;

		m_BuildingData.m_LastTakeResTime = CURRENT_TIME();

		char szContext[1024] = "\0";

		// fly add	20121106
		sprintf_s(szContext, sizeof(szContext), g_pGameServer->GetGameWorld()->GetLanguageStr(10041), pActor->GetName(), Rsp.m_ResNum,Rsp.m_OtherResNum);
		this->RecordHandleOtherUser(pActor->GetName(), g_pGameServer->GetGameWorld()->GetLanguageStr(10042), szContext);
		//sprintf_s(szContext, sizeof(szContext), "代收资源%s得到%d灵石,自己获得%d", pActor->GetName(), Rsp.m_ResNum,Rsp.m_OtherResNum);
		//this->RecordHandleOtherUser(pActor->GetName(), "代收", szContext);

	}

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_Collection,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

	return Rsp.m_ResNum;
}

    //当天已代收资源数
 INT32 ResOutputBuilding::GetTotalCollectResOfDay()
 {
	 time_t nCurTime = CURRENT_TIME();
	 if(XDateTime::GetInstance().IsSameDay(nCurTime,m_BuildingData.m_LastCollectTime)==false)
	 {
         m_BuildingData.m_TotalCollectResOfDay = 0;
	 }

	 return m_BuildingData.m_TotalCollectResOfDay;
 }

  //增加当天代收资源数
 bool ResOutputBuilding::AddCollectRes(INT32 Num)
 {
	 GetTotalCollectResOfDay(); //更新一下当天领取资源数

	 m_BuildingData.m_TotalCollectResOfDay += Num;

	 UINT32 nCurTime = CURRENT_TIME();

	 m_BuildingData.m_LastCollectTime = nCurTime;

	 m_bNeedResetTimer = false;

	 return true;
 }


//初始化，
bool ResOutputBuilding::Create(void)
{
	return true;
}

void ResOutputBuilding::Release()
{
	delete this;
}

enBuildingType ResOutputBuilding::GetBuildingType()
{
	return (enBuildingType)m_BuildingData.m_BuildingType;
}

//获得位置
XPoint ResOutputBuilding::GetLoc()
{
	return XPoint();
}


//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的公开现场
// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
//////////////////////////////////////////////////////////////////////////
bool ResOutputBuilding::OnGetPublicContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SBuildingPublicData))
	{
		return false;
	}

	SBuildingPublicData * pBuildingPublicData = (SBuildingPublicData *)buf;

	pBuildingPublicData->m_BuildingType       = m_BuildingData.m_BuildingType;

	pBuildingPublicData->m_Uid = GetUID();

	pBuildingPublicData->m_SceneID = m_SceneID;

	pBuildingPublicData->m_ResOutputRemainingTime = GetOutputRemainingTime();  //资源产出余下时间

	nLen = SIZE_OF(*pBuildingPublicData);


	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：获得本实体对象的私有现场
// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
// 备  注：私有现场为实体对象的详细信息，
//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
//////////////////////////////////////////////////////////////////////////
bool ResOutputBuilding::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SBuildingPrivateData))
	{
		return false;
	}

	SBuildingPrivateData * pBuildingPrivateData = (SBuildingPrivateData *)buf;

	pBuildingPrivateData->m_BuildingType       = m_BuildingData.m_BuildingType;

	pBuildingPrivateData->m_Uid = GetUID();

	pBuildingPrivateData->m_SceneID = m_SceneID;

	pBuildingPrivateData->m_ResOutputRemainingTime = GetOutputRemainingTime();  //资源产出余下时间

	pBuildingPrivateData->m_CollectResNum = GetTotalCollectResOfDay();

	nLen = SIZE_OF(*pBuildingPrivateData);


	return true;
}
bool ResOutputBuilding::OnSetPrivateContext(const void * buf, int nLen)
{
	if(buf==0 || nLen<sizeof(SBuildingPrivateData))
	{
		return false;
	}

	const SBuildingPrivateData * pBuildingPrivateData = (const SBuildingPrivateData * )buf;

	m_BuildingData.m_BuildingType = pBuildingPrivateData->m_BuildingType;

	m_SceneID = pBuildingPrivateData->m_SceneID;


	SetUID(pBuildingPrivateData->m_Uid);

	m_BuildingData.m_uidOwner = pBuildingPrivateData->m_uidOwner;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将数据库保存的数据传给本实体
// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
// 返回值：返回TRUE表示设置数据成功
//////////////////////////////////////////////////////////////////////////
bool ResOutputBuilding::OnSetDBContext(const void * buf, int nLen)
{
	if(buf==0 || nLen < sizeof(SDB_Get_BuildingData_Rsp))
	{
		return false;
	}

	SDB_Get_BuildingData_Rsp & BuildingData = *(SDB_Get_BuildingData_Rsp*)buf;
	
	m_BuildingData.m_BuildingType = BuildingData.m_BuildingType; 
 
    m_BuildingData.m_LastTakeResTime = BuildingData.m_LastTakeResTime; 	
	SetUID(UID(BuildingData.m_Uid));
	m_BuildingData.m_OwnerNoTakeResNum = BuildingData.m_OwnerNoTakeResNum;

	m_BuildingData.m_LastCollectTime = BuildingData.m_LastCollectTime;
	m_BuildingData.m_TotalCollectResOfDay = BuildingData.m_TotalCollectResOfDay;

	m_BuildingData.m_uidOwner			= UID(BuildingData.m_uidOwner);

	m_BuildingData.m_bAutoTakeRes		= BuildingData.m_bAutoTakeRes;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool ResOutputBuilding::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SDB_Update_BuildingData_Req))
	{
		TRACE("%s : %d Line 数据长度有误 nLen = %d",__FUNCTION__,__LINE__,nLen);
		return false;
	}

	SDB_Update_BuildingData_Req * pBuildData = (SDB_Update_BuildingData_Req *)buf;


	pBuildData->m_BuildingType	 = m_BuildingData.m_BuildingType;
	pBuildData->m_LastTakeResTime = m_BuildingData.m_LastTakeResTime;
	pBuildData->m_Uid = GetUID().ToUint64();

	pBuildData->m_OwnerNoTakeResNum = m_BuildingData.m_OwnerNoTakeResNum;

	pBuildData->m_uidOwner = m_BuildingData.m_uidOwner.ToUint64();
	pBuildData->m_LastCollectTime = m_BuildingData.m_LastCollectTime;
	pBuildData->m_TotalCollectResOfDay = m_BuildingData.m_TotalCollectResOfDay;
	pBuildData->m_bAutoTakeRes	  = m_BuildingData.m_bAutoTakeRes;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：发送消息给本实体
// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
// 返回值：
// 备  注：在此函数内，可能会发FireVote和FireAction
//////////////////////////////////////////////////////////////////////////
void ResOutputBuilding::OnMessage( UINT32 dwMsg, const void * buf, int nLen)
{
}


IResOutputBuilding * ResOutputBuilding::GetBuildingFromGameWorld(UID uidBuilding)
{
	IThing * pBuilding = g_pGameServer->GetGameWorld()->GetThing(uidBuilding);
	if(pBuilding==0 || pBuilding->GetThingClass()!=enThing_Class_Building)
	{
		return 0;
	}

	return (IResOutputBuilding*)pBuilding;
}


 void ResOutputBuilding::SetSceneID(TSceneID SceneID) 
{
	m_SceneID = SceneID;
}

 TSceneID ResOutputBuilding::GetSceneID() 
 {
	 return m_SceneID;
 }

//手动设置上次领取时间，用于资源快熟
void ResOutputBuilding::SetLastTakeResTime(UINT32 nTime)
{
	m_BuildingData.m_LastTakeResTime = nTime;
}

 //记录别人对自己的操作
void ResOutputBuilding::RecordHandleOtherUser(const char * pszTargetUserName, const char * pszHandleName, const char * pszContext)
{
	ResBuildRecordInfo RecordData;

	RecordData.m_bInsert		  = true;
	RecordData.m_BuildingType = m_BuildingData.m_BuildingType;
	RecordData.m_HappenTime	  = CURRENT_TIME();
	strncpy(RecordData.m_szContext, pszContext, sizeof(RecordData.m_szContext));
	strncpy(RecordData.m_szHandleName, pszHandleName, sizeof(RecordData.m_szHandleName));
	strncpy(RecordData.m_szTargetName, pszTargetUserName,sizeof(RecordData.m_szTargetName));

	m_vectBuildingRecordData.push_back(RecordData);
}

//保存记录
void ResOutputBuilding::SaveBuildingRecordToDB(UID uidUser)
{
	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	OBuffer1k ob;

	std::vector<ResBuildRecordInfo>::iterator iter = m_vectBuildingRecordData.begin();

	for(; iter != m_vectBuildingRecordData.end(); )
	{
		ResBuildRecordInfo & RecordInfo = *iter;

		if( (CURRENT_TIME() - RecordInfo.m_HappenTime) >  GameParam.m_BuildRecordSaveTime){
			//超出保存时间的删除
			SDB_Delete_BuildingRecord_Req DBDelete;

			DBDelete.m_UidUser = uidUser.ToUint64();
			DBDelete.m_BuildingType = RecordInfo.m_BuildingType;
			DBDelete.m_HappenTime	= RecordInfo.m_HappenTime;

			ob.Reset();
			ob << DBDelete;
			g_pGameServer->GetDBProxyClient()->Request(DBDelete.m_BuildingType,enDBCmd_Delete_ResBuildRecord,ob.TakeOsb(),0,0);

			iter = m_vectBuildingRecordData.erase(iter);
			continue;
		}

		
		if( RecordInfo.m_bInsert){
			//需要插入的才插入
			SDB_Insert_BuildingRecord_Req DBSave;

			DBSave.m_UidUser      = uidUser.ToUint64();
			DBSave.m_BuildingType = RecordInfo.m_BuildingType;
			DBSave.m_HappenTime   = RecordInfo.m_HappenTime;
			strncpy(DBSave.m_szTargetName, RecordInfo.m_szTargetName, sizeof(DBSave.m_szTargetName));
			strncpy(DBSave.m_szHandleName, RecordInfo.m_szHandleName, sizeof(DBSave.m_szHandleName));
			strncpy(DBSave.m_szContext, RecordInfo.m_szContext, sizeof(DBSave.m_szContext));

			ob.Reset();
			ob << DBSave;
			g_pGameServer->GetDBProxyClient()->Request(DBSave.m_BuildingType,enDBCmd_Insert_ResBuildRecord,ob.TakeOsb(),0,0);

			RecordInfo.m_bInsert = false;
		}

		++iter;
	}
}

//加进记录
void ResOutputBuilding::Push(const BuildingRecordData & RecordData)
{
	ResBuildRecordInfo ResRecordInfo;

	ResRecordInfo.m_bInsert = false;
	ResRecordInfo.m_BuildingType = RecordData.m_BuildingType;
	ResRecordInfo.m_HappenTime   = RecordData.m_HappenTime;
	strncpy(ResRecordInfo.m_szContext, RecordData.m_szContext, sizeof(ResRecordInfo.m_szContext));
	strncpy(ResRecordInfo.m_szHandleName, RecordData.m_szHandleName, sizeof(ResRecordInfo.m_szHandleName));
	strncpy(ResRecordInfo.m_szTargetName, RecordData.m_szTargetName, sizeof(ResRecordInfo.m_szTargetName));

	m_vectBuildingRecordData.push_back(ResRecordInfo);
}


//查看指定资源建筑记录
void ResOutputBuilding::ViewBuildingRecord()
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(m_BuildingData.m_uidOwner);
	if( 0 == pActor){
		return;
	}

	SC_RecordNum_Rsp RspNum;
	RspNum.m_RecordNum = m_vectBuildingRecordData.size();

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_ViewBuildRecord,sizeof(RspNum) + RspNum.m_RecordNum * sizeof(SC_ViewBuildRecord_Rsp)) << RspNum;
	
	for( int i = 0; i < m_vectBuildingRecordData.size(); ++i)
	{	
		SC_ViewBuildRecord_Rsp RspRecord;
		RspRecord.m_HappenTime = m_vectBuildingRecordData[i].m_HappenTime;
		strncpy(RspRecord.m_szContext, m_vectBuildingRecordData[i].m_szContext, sizeof(RspRecord.m_szContext));
		strncpy(RspRecord.m_szHandleName, m_vectBuildingRecordData[i].m_szHandleName, sizeof(RspRecord.m_szHandleName));
		strncpy(RspRecord.m_szOtherUserName, m_vectBuildingRecordData[i].m_szTargetName, sizeof(RspRecord.m_szOtherUserName));

		ob << RspRecord;
	}

	pActor->SendData(ob.TakeOsb());
}

//得到主人UID
UID  ResOutputBuilding::GetMasterUID()
{
	return m_BuildingData.m_uidOwner;
}

//设置自动收取灵石
void ResOutputBuilding::SetAutoTakeRes(bool bAutoTakeRes)
{
	if ( m_BuildingData.m_bAutoTakeRes != bAutoTakeRes){
	
		m_BuildingData.m_bAutoTakeRes = bAutoTakeRes;

		if ( !m_BuildingData.m_bAutoTakeRes){
			
			//删除定时器
			g_pGameServer->GetTimeAxis()->KillTimer(enTIMER_ID_AutoTakeRes, this);
		}else{
			//先检测是否可收灵石
			this->__TakeResource();
			
			//设置自动收取灵石定时器
			this->SetAutoTakeResTimer(true);
		}
	}
}

//设置自动收取灵石定时器
void ResOutputBuilding::SetAutoTakeResTimer(bool bNeedResetTimer)
{
	m_bNeedResetTimer = bNeedResetTimer;

	INT32 RemainingTime = this->GetOutputRemainingTime();

	if ( RemainingTime == 0){
		
		this->__TakeResource();

		RemainingTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_BuildingResOutputInterval;
	}

	//创建定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enTIMER_ID_AutoTakeRes, this, RemainingTime * 1000, "ResOutputBuilding::SetAutoTakeRes");
}

//收取灵石
void ResOutputBuilding::__TakeResource(INT32 SetTakeResNum)
{
	UID uidMaster = this->GetMasterUID();

	IActor * pMaster = g_pGameServer->GetGameWorld()->FindActor(uidMaster);
	
	if ( 0 == pMaster){
		
		return;
	}

	IResOutputPart * pResPart = pMaster->GetResOutputPart();

	if ( 0 == pResPart){
		
		return;
	}

	if( this->GetOutputRemainingTime()<=0 )
	{
		INT32 nTakeRes = pResPart->TakeResource((enBuildingType)m_BuildingData.m_BuildingType,SetTakeResNum);

		SC_TakeRes_Rsp Rsp;
		Rsp.m_uidBuilding = this->GetUID();
		Rsp.m_TakeResNum =  nTakeRes;

		if ( nTakeRes <= 0){
			
			return;
		}

		OBuffer1k ob;

		ob << BuildingHeader(enBuildingCmd_TakeRes,sizeof(Rsp)) << Rsp;

		pMaster->SendData(ob.TakeOsb());	
	}

}

bool ResOutputBuilding::GetAutoTakeRes()
{
	return m_BuildingData.m_bAutoTakeRes;
}

INT32 ResOutputBuilding::GetTakeResNum()
{
	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	UID uidMaster = this->GetMasterUID();

	IActor * pMaster = g_pGameServer->GetGameWorld()->FindActor(uidMaster);
	
	if ( 0 == pMaster){
		
		return ConfigParam.m_BuildingOutputResNum;
	}

	return ConfigParam.m_BuildingOutputResNum + pMaster->GetVipValue(enVipType_TakeResAddStone);
}

//上线时，如果有自动收取灵石，则自动收取
void ResOutputBuilding::Online_AutoTake()
{
	if ( m_BuildingData.m_bAutoTakeRes){
		
		UINT32 LastTakeResTime = this->GetLastTakeResTime();

		UINT32 CurTime = CURRENT_TIME();

		const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

		int count = (CurTime - LastTakeResTime) / GameParam.m_BuildingResOutputInterval;

		if ( count >= 1){
			
			int yusu = (CurTime - LastTakeResTime) % GameParam.m_BuildingResOutputInterval;

			INT32 TakeResNum = count * this->GetTakeResNum();

			this->__TakeResource(TakeResNum);

			this->SetLastTakeResTime( CurTime - (GameParam.m_BuildingResOutputInterval - yusu));
		}

		this->SetAutoTakeResTimer(true);
	}	
}
