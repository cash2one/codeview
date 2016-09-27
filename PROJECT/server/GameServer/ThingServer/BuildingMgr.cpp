
#include "BuildingMgr.h"
#include "ThingServer.h"
#include "IGameServer.h"
#include "IActor.h"
#include "GameSrvProtocol.h"
#include "IBasicService.h"
#include "IResOutputPart.h"
#include "IFuMoDongPart.h"
#include "ITrainingHallPart.h"
#include "IGameScene.h"
#include "IGameWorld.h"
#include "IGatherGodHousePart.h"
#include "IGodSwordShopPart.h"
#include "XDateTime.h"
#include "IBuilding.h"
#include "IConfigServer.h"
#include "IResOutputPart.h"

BuildingMgr::BuildingMgr()
{
}


BuildingMgr::~BuildingMgr()
{
	Close();
}

bool BuildingMgr::Create()
{
	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Building,this);
}

void BuildingMgr::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_Building,this);
}

		//收到MSG_ROOT消息
 void BuildingMgr::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
 {
	 typedef  void (BuildingMgr::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enBuildingCmd_Max]=
	 {
		& BuildingMgr::OpenBuilding,
		& BuildingMgr::TakeRes,
		& BuildingMgr::Collection,		
		& BuildingMgr::VisitBuilding,

		& BuildingMgr::EnterFuMoDong,
		& BuildingMgr::AutoKillMonster,
		& BuildingMgr::CancelKillMonster,
		& BuildingMgr::AccelKillMonster,
		& BuildingMgr::KillMonster,
		NULL,

		//练功堂
		& BuildingMgr::EnterTrainingHall,
		& BuildingMgr::StratTraining,
		& BuildingMgr::StopTraining,
		NULL,

		//聚仙楼
		& BuildingMgr::EnterGatherGod,
		& BuildingMgr::ViewGatherGod,
		& BuildingMgr::FlushGatherGod,
		& BuildingMgr::BuyGatherGod,
		NULL,
		NULL,

		//进入后山
		& BuildingMgr::EnterHouShan,

		//剑冢
		& BuildingMgr::EnterGodSwordShop,
		& BuildingMgr::FlushGodSwordShop,
		& BuildingMgr::BuyGodSword,
		& BuildingMgr::SyncGodSwordShop,

		//建筑
		& BuildingMgr::ViewBuildingRecord,

		& BuildingMgr::TrainingFinishNow,
		NULL,
		NULL,

		& BuildingMgr::AutoFlushEmploy,

		& BuildingMgr::SetAutoTakeRes,

		& BuildingMgr::StartXiWu,
		& BuildingMgr::CancelXiWu,
		NULL,
		
	 };

	 if(nCmd>=enBuildingCmd_Max || 0==s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
 }

 
void  BuildingMgr::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if( 0 == pResOutputPart){
		return;
	}

	pResOutputPart->OnDBRet(userID,ReqCmd,nRetCode,RspOsb,ReqOsb,userdata);
}

//打开建筑
void  BuildingMgr::OpenBuilding(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_OpenBuilding_Req OpenBuilding_Req;

	ib >> OpenBuilding_Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 打开建筑请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	switch(OpenBuilding_Req.m_BuildingType)
	{
	case enBuildingType_Beast:
	case enBuildingType_Grass:
	case enBuildingType_Stone:
		{
			IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	        if(pResOutputPart==0)
	        {
		        return ;
	        }
	        pResOutputPart->OpenBuilding(OpenBuilding_Req);
		}
		break;
	case enBuildingType_FuMoDong:
		{
			EnterFuMoDong(pActor,nCmd,ib);
		}
		break;
	case enBuildingType_HouShan:
		{
			EnterHouShan(pActor,nCmd,ib);
		}
		break;
	case enBuildingType_TrainingHall:
		{
			EnterTrainingHall(pActor,nCmd,ib);
		}
		break;
	case enBuildingType_GatherGodHouse:
		{
			EnterGatherGod(pActor, nCmd, ib);
		}
		break;
	case enBuildingType_SwordDeath:
		{
			NULL;
		}
		break;
	default:
		break;
	}	

}

		//领取资源
void  BuildingMgr::TakeRes(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if(pResOutputPart==0)
	{
		return ;
	}

	CS_TakeRes_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 领取请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pResOutputPart->TakeRes(Req);
}


//代收
void  BuildingMgr::Collection(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if(pResOutputPart==0)
	{
		return ;
	}

	CS_CollectionOther_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 代收请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pResOutputPart->CollectionOther(Req);
}



//访问别人的建筑
void  BuildingMgr::VisitBuilding(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if(pResOutputPart==0)
	{
		return ;
	}

	CS_VisitOtherBuilding_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 访问别人的建筑请求客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pResOutputPart->VisitOtherBuilding(Req);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//伏魔洞处理
void  BuildingMgr::EnterFuMoDong(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if(pFuMoDongPart==0)
	{
		return ;
	}

	pFuMoDongPart->Enter();
}
void  BuildingMgr::AutoKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if(pFuMoDongPart==0)
	{
		return ;
	}

	CS_AutoKillMonster_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pFuMoDongPart->AutoKillMonster(Req);
}
void  BuildingMgr::CancelKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if(pFuMoDongPart==0)
	{
		return ;
	}

	CS_CancelKillMonster_Req Req;

	/*ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}*/

	pFuMoDongPart->CancelKillMonster(Req);
}
void  BuildingMgr::AccelKillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if(pFuMoDongPart==0)
	{
		return ;
	}

	CS_AccelKillMonster_Req Req;

	/*ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}*/

	pFuMoDongPart->AccelKillMonster(Req);
}
void  BuildingMgr::KillMonster(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	IFuMoDongPart * pFuMoDongPart = pActor->GetFuMoDongPart();
	if(pFuMoDongPart==0)
	{
		return ;
	}

	CS_KillMonster_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pFuMoDongPart->KillMonster(Req);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//练功堂处理
void  BuildingMgr::EnterTrainingHall(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();
	if(pTrainingHallPart==0)
	{
		return ;
	}

	pTrainingHallPart->Enter();
}
void  BuildingMgr::StratTraining(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();
	if(pTrainingHallPart==0)
	{
		return ;
	}

	pTrainingHallPart->StartTraining();
}

void  BuildingMgr::StopTraining(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();

	if(pTrainingHallPart==0)
	{
		return ;
	}

	pTrainingHallPart->StopTraining();
}


//进入后山
void BuildingMgr::EnterHouShan(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	TSceneID SceneID;
	SceneID.From(pActor->GetCrtProp(enCrtProp_ActorHouShanSceneID));

	IGameScene * pGameScene = 0;
	if(!SceneID.IsValid())
	{
		//创建
		TMapID MapID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_HouShanMapID;

		pGameScene = g_pGameServer->GetGameWorld()->CreateGameSceneByMapID(MapID);
		SceneID =pGameScene->GetSceneID();
	}
	else
	{
		pGameScene = g_pGameServer->GetGameWorld()->GetGameScene(SceneID);
	}

	if(pGameScene)
	{
		if(pGameScene->EnterScene(pActor))
		{
			pActor->SetCrtProp(enCrtProp_ActorHouShanSceneID,SceneID.m_id);
		}
	}

	SC_EnterBuilding_Rsp Rsp;

	Rsp.m_RetCode = enBuildingRetCode_OK;

	OBuffer1k ob;

	ob << BuildingHeader(enBuildingCmd_EnterHouShan,sizeof(Rsp)) << Rsp;

	pActor->SendData(ob.TakeOsb());

}

////////////////////////聚仙楼处理////////////////////////////////////////////
void  BuildingMgr::EnterGatherGod(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGatherGodHousePart* pGatherGodHousePart = pActor->GetGatherGodHousePart();

	if(0 == pGatherGodHousePart)
	{
		return;
	}

	pGatherGodHousePart->Enter();
}

void  BuildingMgr::ViewGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGatherGodHousePart* pGatherGodHousePart = pActor->GetGatherGodHousePart();

	if(0 == pGatherGodHousePart)
	{
		return;
	}

	CS_ViewItem_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pGatherGodHousePart->ViewItem(Req);
}

void  BuildingMgr::FlushGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGatherGodHousePart* pGatherGodHousePart = pActor->GetGatherGodHousePart();
	if(0 == pGatherGodHousePart)
	{
		return;
	}

	CS_FlushItem_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pGatherGodHousePart->FlushItem(Req);
}

void  BuildingMgr::BuyGatherGod(IActor *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGatherGodHousePart* pGatherGodHousePart = pActor->GetGatherGodHousePart();
	if(0 == pGatherGodHousePart)
	{
		return;
	}

	CS_BuyItem_Req Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pGatherGodHousePart->BuyItem(Req);
}

//剑冢处理
void  BuildingMgr::EnterGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGodSwordShopPart * pGodSwordShopPart = pActor->GetGodSwordShopPart();
	if( 0 == pGodSwordShopPart){
		return;
	}
	
	pGodSwordShopPart->EnterGodSwordShop();
}
void  BuildingMgr::FlushGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGodSwordShopPart * pGodSwordShopPart = pActor->GetGodSwordShopPart();
	if( 0 == pGodSwordShopPart){
		return;
	}

	CS_FlushGodSwordShop_Req Req;
	ib >> Req;

	pGodSwordShopPart->FlushGodSwordShop(Req.m_FlushType);
}
void  BuildingMgr::BuyGodSword(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGodSwordShopPart * pGodSwordShopPart = pActor->GetGodSwordShopPart();
	if( 0 == pGodSwordShopPart){
		return;
	}

	CS_BuyGodSwordShop_Req Req;
	ib >> Req;

	pGodSwordShopPart->BuyGodSword(Req.m_Index);
}
void  BuildingMgr::SyncGodSwordShop(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGodSwordShopPart * pGodSwordShopPart = pActor->GetGodSwordShopPart();
	if( 0 == pGodSwordShopPart){
		return;
	}

	pGodSwordShopPart->SyncGodSwordShop();
}



//查看建筑记录
void  BuildingMgr::ViewBuildingRecord(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_ViewBuildRecord_Req Req;
	ib >> Req;

	if( Req.m_BuildingType >= enBuildingType_Max){
		return;
	}

	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();
	if( 0 == pResOutputPart){
		return;
	}

	pResOutputPart->ViewBuildingRecord(Req.m_BuildingType);
}

//立即完成练功
void  BuildingMgr::TrainingFinishNow(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();

	if(pTrainingHallPart==0)
	{
		return ;
	}

	pTrainingHallPart->TrainingFinishNow();
}

//自动刷新招募角色
void  BuildingMgr::AutoFlushEmploy(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	IGatherGodHousePart* pGatherGodHousePart = pActor->GetGatherGodHousePart();
	if(0 == pGatherGodHousePart)
	{
		return;
	}

	CS_AutoFlushEmploy Req;

	ib >> Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	pGatherGodHousePart->AutoFlushEmployee(Req.m_FlushNum, Req.m_Aptitude);
}

//设置自动收取灵石
void  BuildingMgr::SetAutoTakeRes(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_Set_AutoTakeRes Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	IResOutputPart * pResOutputPart = pActor->GetResOutputPart();

	if ( 0 == pResOutputPart){

		return;
	}

	pResOutputPart->SetAutoTakeRes(Req.m_bOpen);
}

//开始习武
void  BuildingMgr::StartXiWu(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_StartXiWu Req;

	ib >> Req;

	if ( ib.Error())
	{
		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();

	if ( 0 == pTrainingHallPart )
	{
		return ;
	}

	pTrainingHallPart->StartXiWu(Req.m_uidActor);	
}

//取消习武
void  BuildingMgr::CancelXiWu(IActor  *pActor, UINT8 nCmd, IBuffer & ib)
{
	ITrainingHallPart * pTrainingHallPart = pActor->GetTrainingHallPart();

	if ( 0 == pTrainingHallPart )
	{
		return ;
	}

	pTrainingHallPart->CancelXiWu();
}
