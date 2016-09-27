

#include "IActor.h"

#include "ResOutputPart.h"

#include "IBasicService.h"
#include "IConfigServer.h"
#include "IGameServer.h"
#include "GameWorld.h"
#include "ThingServer.h"
#include "time.h"
#include "XDateTime.h"
#include "ISyndicate.h"
#include "ISyndicateMgr.h"
#include "IFriendPart.h"

ResOutputPart::ResOutputPart()
{
	m_pActor = 0;
	for(int i=0; i<ARRAY_SIZE(m_BuildingData); ++i)
	{
		m_BuildingData[i] = UID();
	}

	m_bFirst = true;
}

ResOutputPart::~ResOutputPart()
{
}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool ResOutputPart::Create(IThing *pMaster, void *pContext, int nLen)
{

	if(pMaster == 0 || pMaster->GetThingClass() != enThing_Class_Actor)
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	TSceneID SceneID;
	SceneID.From(m_pActor->GetCrtProp(enCrtProp_ActorMainSceneID));

	SDB_Get_BuildingData_Rsp * pBuildingData = (SDB_Get_BuildingData_Rsp *)pContext;

	INT32 count = nLen/sizeof(SDB_Get_BuildingData_Rsp);

	for(int i=0; i<count; ++i,pBuildingData++)
	{
		UID uid(pBuildingData->m_Uid);

		if(!uid.IsValid())
		{
			continue;
		}
						
		IBuilding * pBuilding = (IBuilding*)g_pGameServer->GetGameWorld()->CreateThing(enThing_Class_Building,SceneID,
			(char*)(pBuildingData),sizeof(SDB_Get_BuildingData_Rsp),THING_CREATE_FLAG_DB_DATA);
	
		if(pBuilding)
		{
			pBuilding->SetSceneID(SceneID);

			SetBuilding(pBuilding);
		}
	}

	if(m_pActor->GetMaster()==0)
	{

		for(int i=0; i<ResOutputBuildingType_Max; ++i)
		{			
			if(m_BuildingData[i].IsValid()==false)
			{
				//创建
				SCreateBuildingContext  BuildingCnt;

				BuildingCnt.m_BuildingType = enBuildingType_Beast+i;
				BuildingCnt.m_uidOwner = m_pActor->GetUID();		
				BuildingCnt.SceneID = SceneID;

				IBuilding *  pBuilding = (IBuilding*)g_pGameServer->GetGameWorld()->CreateBuilding(BuildingCnt);	

				if(pBuilding)
				{
					SetBuilding(pBuilding);
				}
			}	
		}

		this->LoadBuildingRecord();
	}

	return true;
}

//释放
void ResOutputPart::Release(void) 
{
	delete this;
}

//取得部件ID
enThingPart ResOutputPart::GetPartID(void)
{
	return enThingPart_Actor_ResOutPut;
}

//取得本身生物
IThing*		ResOutputPart::GetMaster(void) 
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool ResOutputPart::OnGetDBContext(void * buf, int &nLen) 
{	
	nLen = 0;
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void ResOutputPart::InitPrivateClient()
{
	if ( !m_bFirst){
		return;
	}

	m_bFirst = false;

	for( int i = 0; i < ResOutputBuildingType_Max; ++i){

		IResOutputBuilding * pBuilding = GetBuilding(i);

		if(pBuilding == 0)
		{
			TRACE("<error> %s : %d Line 不存在的建筑  类型 %d",__FUNCTION__,__LINE__,i);
			return;
		}


		pBuilding->Online_AutoTake();
	}
} 


//玩家下线了，需要关闭该ThingPart
void ResOutputPart::Close()
{
	for( int i = 0; i < ResOutputBuildingType_Max; ++i){

		g_pThingServer->GetGameWorld()->DestroyThing(m_BuildingData[i]);		
	}
}

//保存数据
void ResOutputPart::SaveData()
{


	for( int i = 0; i < ResOutputBuildingType_Max; ++i){

		SaveBuildingToDB(m_BuildingData[i]);
		SaveBuildingRecordToDB(m_BuildingData[i]);		

	}
}

void ResOutputPart::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	}

	switch(ReqCmd)
	{
	case enDBCmd_GetBuildingRecordInfo:
		{
			this->HandleLoadBuildingRecord(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		break;
	}
}

//保存建筑数据
void ResOutputPart::SaveBuildingToDB(UID uidBuilding)
{
	SDB_Update_BuildingData_Req Req;

	int nLen = sizeof(SDB_Update_BuildingData_Req);

	IResOutputBuilding * pBuilding = GetBuildingFromGameWorld(uidBuilding);

	if(pBuilding==0)
	{
		return;
	}

	if( pBuilding->OnGetDBContext(&Req,nLen) == false)
	{
		return;
	}

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateBuildingInfo,ob.TakeOsb(),0,0);
}

//获取建筑记录
void ResOutputPart::LoadBuildingRecord()
{
	SDB_Get_BuildingRecord_Req Req;
	Req.m_UidUser = m_pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_GetBuildingRecordInfo,ob.TakeOsb(),&g_pThingServer->GetBuildingMgr(),m_pActor->GetUID().ToUint64());
}

//获取建筑记录的回调
void	ResOutputPart::HandleLoadBuildingRecord(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;	

	if(RspIb.Error()){
		TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		return;
	}

	INT32 nNum = RspIb.Remain() / sizeof(BuildingRecordData);

	for( int i = 0; i < nNum; ++i)
	{
		BuildingRecordData RecordData;
		RspIb >> RecordData;

		if(RspIb.Error()){
			TRACE("HandleAddFriend DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}
		
		IResOutputBuilding * pBuilding = GetBuildingFromGameWorld(m_BuildingData[RecordData.m_BuildingType]);
		if( 0 == pBuilding){
			continue;
		}

		pBuilding->Push(RecordData);
	}
}

//保存建筑记录
void ResOutputPart::SaveBuildingRecordToDB(UID uidBuilding)
{
	IResOutputBuilding * pBuilding = GetBuildingFromGameWorld(uidBuilding);
	if( 0 == pBuilding){
		return;
	}

	pBuilding->SaveBuildingRecordToDB(m_pActor->GetUID());
}

//打开建筑
void ResOutputPart::OpenBuilding(CS_OpenBuilding_Req & Req)
{
	if(Req.m_BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	IResOutputBuilding * pBuilding = GetBuilding(Req.m_BuildingType);

	if(pBuilding == 0)
	{
		TRACE("<error> %s : %d Line 不存在的建筑  类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	__OpenBuilding(pBuilding);

}

void ResOutputPart::__OpenBuilding(IResOutputBuilding * pBuilding)
{
	if(pBuilding == 0)
	{
		TRACE("<error> %s : %d Line 不存在的建筑",__FUNCTION__,__LINE__);
		return;
	}
	SC_OpenBuilding_Rsp Rsp;

	Rsp.m_ResOutputRemainingTime = pBuilding->GetOutputRemainingTime();
	Rsp.m_BuildingType = pBuilding->GetBuildingType();

	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();
	Rsp.m_ResOutputSpace = ConfigParam.m_BuildingResOutputInterval;

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_OpenBuilding,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}

//获得当天偷窃数量
INT32  ResOutputPart::GetTotalFilchResOfDay(enBuildingType BuildingType)
{
	if(BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,BuildingType);
		return 0;
	}

	IResOutputBuilding * pBuilding = GetBuilding(BuildingType);


	return pBuilding->GetTotalCollectResOfDay();

}

//领取资源
void ResOutputPart::TakeRes(CS_TakeRes_Req & Req)
{
	if(Req.m_BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	if(Req.m_BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	IResOutputBuilding * pBuilding = GetBuilding(Req.m_BuildingType);

	if ( 0 == pBuilding){
		
		return;
	}

	INT32 nTakeResNum = this->TakeResource(Req.m_BuildingType);

	SC_TakeRes_Rsp Rsp;
	Rsp.m_uidBuilding = pBuilding->GetUID();
	Rsp.m_TakeResNum =  nTakeResNum;

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_TakeRes,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	__OpenBuilding(pBuilding);

}

//获得代收资源上限
INT32 ResOutputPart::GetBuildingResFilchMax()
{
	const SGameConfigParam &  ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	INT32 BuildingResFilchMax = ConfigParam.m_BuildingResFilchMax;

	ISyndicate * pSyndicate = g_pGameServer->GetSyndicateMgr()->GetSyndicate(m_pActor->GetUID());

	if(pSyndicate != 0)
	{
		BuildingResFilchMax = BuildingResFilchMax * (1+pSyndicate->GetWelfareValue(enWelfare_MaxFilchCollectionOther)/100.0f) + 0.99999;
	}

	return BuildingResFilchMax;
}

//代收
void ResOutputPart::CollectionOther(CS_CollectionOther_Req & Req)
{
	if(Req.m_BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	IResOutputBuilding * pBuilding = GetBuilding(Req.m_BuildingType);


	INT32 BuildingResFilchMax = GetBuildingResFilchMax();

	INT32 TotalFilchResNum = GetTotalCollectToday();

	SC_CollectionOther_Rsp Rsp;
	Rsp.m_ResNum = 0;
	Rsp.m_uidBuild = Req.m_uidOther;

	if(TotalFilchResNum>=BuildingResFilchMax)
	{
		//达到窃取上限
		Rsp.m_RetCode = enFilchRetCode_ErrResLimit;
	}
	else 
	{
		IResOutputBuilding * pOtherBuilding = GetBuildingFromGameWorld(Req.m_uidOther);
		if(pOtherBuilding==0)
		{
			return;
		}

		INT32 nNum = pOtherBuilding->Collectioned(m_pActor,BuildingResFilchMax-TotalFilchResNum);

		//记录当天已代收数量
		pBuilding->AddCollectRes(nNum);
		
		IFriendPart * pFriendPart = m_pActor->GetFriendPart();
		if( 0 == pFriendPart){
			return;
		}
		
		IActor * pFriend = g_pGameServer->GetGameWorld()->FindActor(pOtherBuilding->GetMasterUID());
		if( 0 == pFriend){
			return;
		}
		
		//加好友度
		pFriendPart->AddRelationNum(pFriend->GetUID(), pFriend->GetName(), enHelpStone);
	
		return;
	}

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_Collection,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}




//访问别人的建筑
void ResOutputPart::VisitOtherBuilding(CS_VisitOtherBuilding_Req & Req)
{
	if(Req.m_BuildingType >= ResOutputBuildingType_Max)
	{
		TRACE("<error> %s : %d Line 错误的建筑类型 %d",__FUNCTION__,__LINE__,Req.m_BuildingType);
		return;
	}

	IResOutputBuilding * pBuilding = GetBuildingFromGameWorld(Req.m_uidOther);

	if(pBuilding==0)
	{
		return;
	}

	SC_VisitOtherBuilding_Rsp Rsp;

	Rsp.m_UidBuilding = pBuilding->GetUID();

	Rsp.m_TotalCollectResOfDay = this->GetTotalFilchResOfDay(Req.m_BuildingType);

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_VisitBuilding,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

}

IResOutputBuilding * ResOutputPart::GetBuildingFromGameWorld(UID uidBuilding)
{
	IThing * pBuilding = g_pGameServer->GetGameWorld()->GetThing(uidBuilding);
	if(pBuilding==0 || pBuilding->GetThingClass()!=enThing_Class_Building)
	{
		return 0;
	}

	return (IResOutputBuilding*)pBuilding;
}


IResOutputBuilding * ResOutputPart::GetBuilding(UINT8 BuildingType)
{
	if(BuildingType>=ResOutputBuildingType_Max)
	{
		return 0;
	}

	return GetBuildingFromGameWorld(m_BuildingData[BuildingType]);
}

void ResOutputPart::SetBuilding(IBuilding * pBuilding)
{
	UINT8 BuildingType = 0;
	if(pBuilding==0 || (BuildingType=pBuilding->GetBuildingType())>=ResOutputBuildingType_Max)
	{
		return ;
	}

	m_BuildingData[BuildingType] = pBuilding->GetUID();

}



//手动设置上次领取时间，用于资源快熟
void ResOutputPart::SetLastTakeResTime(enBuildingType  BuildType, UINT32 nTime)
{
	IResOutputBuilding * pBuilding = GetBuilding(BuildType);
	if( 0 == pBuilding){
		return;
	}

	pBuilding->SetLastTakeResTime(nTime);
}

//查看指定资源建筑记录
void ResOutputPart::ViewBuildingRecord(enBuildingType BuildingType)
{
	IResOutputBuilding * pBuilding = GetBuilding(BuildingType);
	if( 0 == pBuilding){
		return;
	}

	pBuilding->ViewBuildingRecord();
}

//同步资源建筑信息
void ResOutputPart::SynResBuildInfo()
{
	IResOutputBuilding * pLingShouBuilding = this->GetBuilding(enBuildingType_Beast);
	if( 0 == pLingShouBuilding){
		return;
	}

	IResOutputBuilding * pBaiCaoBuilding = this->GetBuilding(enBuildingType_Grass);
	if( 0 == pBaiCaoBuilding){
		return ;
	}

	IResOutputBuilding * pLingShiBuilding = this->GetBuilding(enBuildingType_Stone);
	if( 0 == pLingShiBuilding){
		return;
	}

	SC_SynResBuildInfo Rsp;
	
	Rsp.m_uidLingShouBuild = pLingShouBuilding->GetUID();
	Rsp.m_LingShouRemainingTime = pLingShouBuilding->GetOutputRemainingTime();

	Rsp.m_uidBaiCaoBuild   = pBaiCaoBuilding->GetUID();
	Rsp.m_BaiCaoRemainingTime = pBaiCaoBuilding->GetOutputRemainingTime();

	Rsp.m_uidLingShiBuild  = pLingShiBuilding->GetUID();
	Rsp.m_LingShiRemainingTime = pLingShiBuilding->GetOutputRemainingTime();

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SynResBuildInfo,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//把建筑同步信息发给其它玩家
void ResOutputPart::SendSynResBuildInfo(IActor * pTargetActor)
{
	IResOutputBuilding * pLingShouBuilding = this->GetBuilding(enBuildingType_Beast);
	if( 0 == pLingShouBuilding){
		return;
	}

	IResOutputBuilding * pBaiCaoBuilding = this->GetBuilding(enBuildingType_Grass);
	if( 0 == pBaiCaoBuilding){
		return ;
	}

	IResOutputBuilding * pLingShiBuilding = this->GetBuilding(enBuildingType_Stone);
	if( 0 == pLingShiBuilding){
		return;
	}

	SC_SynResBuildInfo Rsp;
	
	Rsp.m_uidLingShouBuild = pLingShouBuilding->GetUID();
	Rsp.m_LingShouRemainingTime = pLingShouBuilding->GetOutputRemainingTime();

	Rsp.m_uidBaiCaoBuild   = pBaiCaoBuilding->GetUID();
	Rsp.m_BaiCaoRemainingTime = pBaiCaoBuilding->GetOutputRemainingTime();

	Rsp.m_uidLingShiBuild  = pLingShiBuilding->GetUID();
	Rsp.m_LingShiRemainingTime = pLingShiBuilding->GetOutputRemainingTime();

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SynResBuildInfo,sizeof(Rsp)) << Rsp;
	pTargetActor->SendData(ob.TakeOsb());
}

//得到今天的总代收灵石量
UINT32 ResOutputPart::GetTotalCollectToday()
{
	UINT32 nTotalCollect = 0;

	for( int i = enBuildingType_Beast; i < ResOutputBuildingType_Max; ++i)
	{
		IResOutputBuilding * pResBuilding = this->GetBuilding(i);
		if( 0 == pResBuilding){
			continue;
		}	

		nTotalCollect += pResBuilding->GetTotalCollectResOfDay();
	}

	return nTotalCollect;
}

//设置自动收取灵石
void ResOutputPart::SetAutoTakeRes(bool bAutoTakeRes)
{
	SC_Set_AutoTakeRes Rsp;

	if ( bAutoTakeRes && m_pActor->GetCrtProp(enCrtProp_TotalVipLevel) == 0){
		
		Rsp.m_RetCode = enBuildingRetCode_NoVip;

	}else if ( bAutoTakeRes && !m_pActor->GetVipValue(enVipType_bCanAutoTakeRes)){
		
		Rsp.m_RetCode = enBuildingRetCode_ErrVipLv;

	}else{
		
		for ( int i = enBuildingType_Beast; i <= enBuildingType_Stone; ++i)
		{
			IResOutputBuilding * pResBuilding = this->GetBuilding((enBuildingType)i);

			if ( 0 == pResBuilding){
				
				continue;
			}

			pResBuilding->SetAutoTakeRes(bAutoTakeRes);
		}

		this->SynResBuildInfo();
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Set_AutoTakeRes,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//领取资源
INT32 ResOutputPart::TakeResource(enBuildingType m_BuildingType, INT32 SetTakeNum)
{
	IResOutputBuilding * pBuilding = GetBuilding(m_BuildingType);

	if ( 0 == pBuilding){
		
		return 0;
	}

	//发布事件
	SS_TakeBuildingRes TakeBuildingRes;

	TakeBuildingRes.m_AddValue = 0;
	TakeBuildingRes.m_BuildingType = m_BuildingType;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TakeBuildingRes);

	m_pActor->OnEvent(msgID,&TakeBuildingRes,sizeof(TakeBuildingRes));

	INT32 nTakeResNum = 0;

	if ( 0 == SetTakeNum){
		
		nTakeResNum = pBuilding->TakeRes();
	}else{
		
		nTakeResNum = SetTakeNum;
	}

	if ( nTakeResNum > 0){
		
		nTakeResNum = nTakeResNum + nTakeResNum *  TakeBuildingRes.m_AddValue / 1000;

		nTakeResNum *= m_pActor->GetWallowFactor();

		m_pActor->AddCrtPropNum(enCrtProp_ActorStone, nTakeResNum);
	}

	return nTakeResNum;
}

//获取是否设置自动获取灵石
bool ResOutputPart::GetbOpenAutoTakeRes()
{
	IResOutputBuilding * pLingShouBuilding = this->GetBuilding(enBuildingType_Beast);

	if ( 0 == pLingShouBuilding){

		return false;
	}

	return pLingShouBuilding->GetAutoTakeRes();
}
