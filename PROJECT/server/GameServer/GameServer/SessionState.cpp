

#include <boost/timer.hpp>
#include "IBasicService.h"
#include "GameServer.h"
#include "SessionState.h"
#include "IDBProxyClient.h"
#include "GameSrvProtocol.h"
#include "Session.h"
#include "DBProtocol.h"
#include "ICreature.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "DMsgSubAction.h"
#include "IGameScene.h"
#include "ICDTime.h"
#include "IActorBasicPart.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "IGoods.h"
#include "ITaskPart.h"
#include "IKeywordFilter.h"
#include "ServerProtocol.h"
#include "IActivityPart.h"
#include "ICryptService.h"
#include "IThingServer.h"
#include "FileSystem.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

EnterServerState::EnterServerState(Session * pSession)
{
	m_pSession = pSession;
	m_BuildActor.m_pSession = m_pSession;

	m_bSuccessBuildUser = true;

	m_bNeedUpdateCnfg = false;

	m_bNeedUpdateRes = false;

	m_bSelfEnter = false;
}

EnterServerState::~EnterServerState()
{
}


void	EnterServerState::Enter(void *pContext)
{
}

void	EnterServerState::Leave(void * pContext)
{
}

//数据到达,
void EnterServerState::OnNetDataRecv(OStreamBuffer & osb)
{
	OBuffer4k ob(osb);
	IBuffer ib(ob.Buffer(),ob.Size());

	AppPacketHeader Header;
	ib >> Header;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息长度错误 len = %d!", __FUNCTION__,__LINE__,ob.Size());
		return ;
	}

	switch(Header.MsgCategory)
	{
	case enMsgCategory_GameFrame:
		{
			//游戏服框架消息
			switch(Header.command)
			{
			case enGameFrameCmd_CS_Enter:
				{
					HandleClientEnter(Header.command,ib,ob);
				}
				break;
			case enGameFrameCmd_CS_CreateActor:
				{
					HandleClientCreateActor(Header.command,ib,ob);
				}
				break;
			case enGameFrameCmd_CS_CheckCnfg:
				{
					HandleClientCheckCnfg(Header.command,ib,ob);
				}
				break;
			case enGameFrameCmd_CS_SendPoint:
				{
					CS_SendPoint Req;

					ib >> Req;

					if(ib.Error())
					{
						return;
					}

					SDB_Insert_PointMsg_Req DBReq;

					DBReq.m_Vid = Req.m_Vid;
					DBReq.m_UserID = Req.m_UserID;
					DBReq.m_Operator = (UINT8)enGamePoint_EnterCreateActor;

					OBuffer1k ob2;

					ob2 << DBReq;

					GameServer::GetInstance()->GetDBProxyClient()->Request(DBReq.m_UserID,enDBCmd_Insert_PointMsg,ob2.TakeOsb(),0,0);
				}
				break;
			default:
				TRACE("<error> %s : %d line 客户端消息命令字错误 cmd = %d!", __FUNCTION__,__LINE__,Header.command);
				break;
			}		
		}
		break;
	case enMsgCategory_WebServer:
		{
			//后台消息，没有玩家
			GameServer::GetInstance()->GetThingServer()->WebOnRecv(Header.command, ib);
		}
		break;
	default:
		TRACE("<error> %s : %d line 客户端消息分类错误 MsgCategory = %d!", __FUNCTION__,__LINE__,Header.MsgCategory);
		break;
	}
}

void EnterServerState::HandleClientEnter(int nCmd,IBuffer & ib, OBuffer4k & ob)
{
	CS_EnterGame_Req EnterReq;
	ib >> EnterReq;
	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息长度错误 len = %d!", __FUNCTION__,__LINE__,ob.Size());
		return;
	}

	m_bSelfEnter = true;

	const SServer_Info & ServerInfo = GameServer::GetInstance()->GetServerInfo();

	//验证客户端版本号
	INT32 retVer = strncmp(ServerInfo.m_strLastVersion.c_str(),EnterReq.ClientVersion,ARRAY_SIZE(EnterReq.ClientVersion));

	if(retVer != 0)
	{
		retVer = strncmp(EnterReq.ClientVersion,ServerInfo.m_strMinVersion.c_str(),ARRAY_SIZE(EnterReq.ClientVersion));

		if(retVer<0)
		{
			TRACE("<error> %s : %d line 客户端版本号[%s]错误,服务器最小兼容版本号为[%s]!", __FUNCTION__,__LINE__,EnterReq.ClientVersion,ServerInfo.m_strMinVersion.c_str());

			HandleEnterGameRsp(enEnterGameRetCode_ErrVersion,ob);

			return;
		}

	}

	if( GameServer::GetInstance()->IsSealNo(EnterReq.UserID)){
		//被封号
		m_pSession->KickUser(enKickType_SealNo);
		return;
	}

	//从数据库请求角色数据
	ob.Reset();

	//ActorBasicData
	SDB_Get_ActorBasicData_Req ReqActorBasicData;
	ReqActorBasicData.UserID = EnterReq.UserID;

	SS_UserPrivateData * pUserPrivateData = (SS_UserPrivateData*)EnterReq.szTicket;

	this->m_BuildActor.m_IdentityStatus = (enIdentityStatus)pUserPrivateData->m_IdentityStatus;
	this->m_BuildActor.m_OnlineTime     = pUserPrivateData->m_OnlineTime;

	if(EnterReq.UserID != pUserPrivateData->m_UserID)
	{
		TRACE("<error> %s : %d line UserID有错误 EnterReq.UserID[%d]  != pUserPrivateData->m_UserID[%d]!", __FUNCTION__,__LINE__,EnterReq.UserID,pUserPrivateData->m_UserID);
		return;
	}

	//验证配置文件版本
	if(GameServer::GetInstance()->GetConfigServer()->GetClientVersion() != std::string(EnterReq.szClientCnfgVersion))
	{
		m_bNeedUpdateCnfg = true;
	}

	if(GameServer::GetInstance()->GetConfigServer()->GetClientResVersion() != std::string(EnterReq.szClientResVersion))
	{
		m_bNeedUpdateRes = true;
	}

	ob.Reset();
	ob << ReqActorBasicData;

	GameServer::GetInstance()->GetDBProxyClient()->Request(EnterReq.UserID,enDBCmd_GetActorBasicDataInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());
}

void EnterServerState::HandleClientCreateActor(int nCmd,IBuffer & ib, OBuffer4k & ob)
{
	CS_CreateActor CreateActor ;
	ib >> CreateActor;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息长度错误 len = %d!", __FUNCTION__,__LINE__,ob.Size());
		return;
	}

	GameServer * pGameServer = GameServer::GetInstance();

	const SCreateActorCnfg & CreateActorCnfg = pGameServer->GetConfigServer()->GetCreateActorCnfg();

	CreateActor.szName[MEM_SIZE(CreateActor.szName)-1]=0;

	if(pGameServer->GetKeywordFilter()->IsValidName(CreateActor.szName)==false)
	{
		HandleEnterGameRsp(enEnterGameRetCode_ErrKeyName,ob);
		return ;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	//ActorBasicData
	SDB_Insert_ActorBasicData_Req ReqActorBasic;
	ReqActorBasic.UserID		= CreateActor.UserID;
	strncpy(ReqActorBasic.Name, CreateActor.szName, MEM_SIZE(ReqActorBasic.Name));
	ReqActorBasic.Level			= CreateActorCnfg.m_Level;
	ReqActorBasic.Spirit		= CreateActorCnfg.m_Spirit;
	ReqActorBasic.Shield		= CreateActorCnfg.m_Shield;
	ReqActorBasic.BloodUp		= CreateActorCnfg.m_Blood;	
	ReqActorBasic.Avoid			= CreateActorCnfg.m_Avoid;
	ReqActorBasic.ActorExp		= CreateActorCnfg.m_Exp;
	ReqActorBasic.ActorLayer	= CreateActorCnfg.m_Layer;
	ReqActorBasic.ActorNimbus	= CreateActorCnfg.m_Nimbus;
	ReqActorBasic.ActorAptitude = CreateActorCnfg.m_Aptitude;
	ReqActorBasic.ActorSex		= CreateActor.sex % enCrtSex_Max;
	//	ReqActorBasic.uid = UID().ToUint64();
	ReqActorBasic.uid			= (UniqueIDGeneratorService::GenerateUID(enThing_Class_Actor)).ToUint64();
	ReqActorBasic.ActorFacade	= CreateActor.facade;
	ReqActorBasic.uidMaster		= UID().ToUint64();
	ReqActorBasic.CityID		= CreateActor.cityID;
	ReqActorBasic.ActorNimbusSpeed = CreateActorCnfg.m_NimbusSpeed;

	ob.Reset();
	ob << ReqActorBasic;
	pGameServer->GetDBProxyClient()->Request(CreateActor.UserID,enDBCmd_InsertActorBasicDataInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void  EnterServerState::CheckCnfgFile(const std::string strFileName,const char* szMd5)
{
	SC_CheckCnfg_Rsp Rsp;
	Rsp.m_Result = enCheckCnfgRetCode_Ok;

	GameServer * pGameServer = GameServer::GetInstance();

	OBuffer16k obFile;
	FileUpdateInfo FileInfo;


	if(pGameServer->ReadFile(strFileName,obFile)==false)
	{
		Rsp.m_Result = enCheckCnfgRetCode_Nonexist;
	}
	else
	{
		ICryptService * pCryptService = pGameServer->GetCryptService();

		TMD5 md5 = pCryptService->CalculateMD5((const UINT8*)obFile.Buffer(),obFile.Size());

		std::string strMd5 = md5.toString();

		if( strMd5 != szMd5)
		{	
			strncpy(FileInfo.m_szFileCnfgVersion,strMd5.c_str(),MEM_SIZE(FileInfo.m_szFileCnfgVersion)-2);
			FileInfo.m_FileSize = obFile.Size();
		}
	}

	OBuffer16k obRsp;

	if(FileInfo.m_FileSize==0)
	{
		obRsp << GameFrameHeader(enGameFrameCmd_SC_CheckCnfg,SIZE_OF(Rsp)) << Rsp;
	}
	else
	{
		int len = SIZE_OF(Rsp)+SIZE_OF(FileInfo)+ obFile.Size() + strFileName.length()+1;

		obRsp << GameFrameHeader(enGameFrameCmd_SC_CheckCnfg,len) << Rsp << FileInfo << strFileName << obFile ;
	}

	m_pSession->SendData(obRsp.TakeOsb());	
}

void EnterServerState::HandleClientCheckCnfg(int nCmd,IBuffer & ib, OBuffer4k & ob)
{
	CS_CheckCnfg_Req CheckCnfg_Req ;

	ib >> CheckCnfg_Req;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息长度错误 len = %d!", __FUNCTION__,__LINE__,ob.Size());
		return;
	}

	std::string strFileName;

	ib >> strFileName;

	if(ib.Error())
	{
		TRACE("<error> %s : %d line 客户端消息长度错误 len = %d!", __FUNCTION__,__LINE__,ob.Size());
		return;
	}

	CheckCnfgFile(strFileName,CheckCnfg_Req.m_szClientFileVersion);
}


void	EnterServerState::OnCloseConnect()
{
	m_pSession->Close(); //直接关闭
}

// nRetCode: 取值于 enDBRetCode
void EnterServerState::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
							   OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error>DB应答错误 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_GetBuildingInfo:
		{	//资源产出建筑
			this->HandleGetBuildinData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertFuMoDongInfo:
	case enDBCmd_GetFuMoDongInfo:
		{	//伏魔洞
			this->HandleGetFuMoDongData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertTrainingHallInfo:
	case enDBCmd_GetTrainingHallInfo:
		{	//练功堂
			this->HandleGetTrainingHallData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertGatherGodHouseInfo:
	case enDBCmd_GetGatherGodHouseInfo:
		{	//聚仙楼
			this->HandleGetGatherGodHouseData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertXiuLianInfo:
	case enDBCmd_GetXiuLianInfo:
		{	//修炼
			this->HandleGetXiuLianData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetFuBenInfo:
	case enDBCmd_InsertFuBenInfo:
		{	//副本
			HandleGetFuBenData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetGodSwordShopInfo:
	case enDBCmd_InsertGodSwordShopInfo:
		{	//剑冢
			HandleGetGodSwordShopData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetPacketInfo:
	case enDBCmd_InsertPacketInfo:
		{	//背包
			HandleGetPacketData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertEquipInfo:
	case enDBCmd_GetEquipInfo:
		{	//装备
			HandleGetEquipPanelData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetMagicPanelInfo:
	case enDBCmd_InsertMagicPanelInfo:
		{	//法术栏
			HandleGetMagicPanelData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetSynMagicInfo:
	case enDBCmd_InsertSynMagicInfo:
		{	//帮派技能
			HandleGetSynMagicData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetCombatInfo:
	case enDBCmd_InsertCombatInfo:
		{	//战斗
			HandleGetCombatData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetStatusInfo:
		{	//状态
			HandleGetStatusData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertDouFaPartInfo:
	case enDBCmd_GetDouFaPartInfo:
		{	//斗法
			this->HandleGetDouFaData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertTaskPartInfo:
	case enDBCmd_GetTaskPartInfo:
		{	//任务
			this->HandleGetTaskData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetChengJiuInfo:
	case enDBCmd_InsertChengJiuPartInfo:
		{	//成就
			this->HandleGetChengJiuData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetActivityPartInfo:
		{
			//活动
			this->HandleGetActivityData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_InsertActorBasicDataInfo:
	case enDBCmd_GetActorBasicDataInfo:
	case enDBCmd_GetActorBasicDataByUIDInfo:
		{	//角色基本数据，并获取所有PART数据
			this->HandleGetActorBasicDataAndPart(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetCDTimer:
		{	
			//冷却时间
			this->HandleGetCDTimerData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_GetActorEmployInfo:
		{
			HandleGetEmployeeData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_SynPart:
		{
			HandleGetSynPart(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	case enDBCmd_Get_TalismanPart:
		{
			HandleGetTalismanPart(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
			TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		}
		break;
	}
}


void EnterServerState::HandleEnterGameRsp(enEnterGameRetCode RetCode,OBuffer4k & RspOb)
{

	if(RetCode==enEnterGameRetCode_NoActor)
	{
		//发送随机人名配置文件给客户端
		std::vector<std::string> vectFile = FileSystem::GetAllFileName("Config/clientinit/");

		std::vector<std::string> vectCommon = FileSystem::GetAllFileName("Config/clientcommon/");

		vectFile.insert(vectFile.end(),vectCommon.begin(),vectCommon.end());

		for(int i=0; i<vectFile.size();i++)
		{
			CheckCnfgFile(vectFile[i],"0");
		}
	}

	RspOb.Reset();

	SC_EnterGame_Rsp Rsp;
	Rsp.Result = RetCode;

	std::string strContext;

	IActor * pActor = m_pSession->Actor();
	if(pActor)
	{
		IActivityPart * pActivityPart = pActor->GetActivityPart();

		if(pActivityPart)
		{
			strContext = pActivityPart->GetNewPlayerGuideContext();
		}
	}

	const SServer_Info & ServerInfo = GameServer::GetInstance()->GetServerInfo();

	INT32 len = SIZE_OF(SC_EnterGame_Rsp())+strContext.length()+ServerInfo.m_strServiceTel.length()+ServerInfo.m_strServiceEmail.length()+ServerInfo.m_strServiceQQ.length()+4;


	RspOb << GameFrameHeader(enGameFrameCmd_SC_Enter,len) << Rsp << strContext << ServerInfo.m_strServiceTel << ServerInfo.m_strServiceEmail << ServerInfo.m_strServiceQQ;

	m_pSession->SendData(RspOb.TakeOsb());

}

//通知客户端初始化
void EnterServerState::NoticClientInit(TSceneID SecenID,UID uidActor)
{
	OBuffer16k obCnfg;  //更新配置文件

	OBuffer1k obRsp;

	SC_InitClient InitClient;
	InitClient.m_SceneID = SecenID;
	InitClient.m_uidActor = uidActor; 
	InitClient.m_ChatServerPort = g_ConfigParam.m_ChatServerInfo.m_GameSrvPort;
	strncpy(InitClient.m_ChatServerIp,(char*)g_ConfigParam.m_ChatServerInfo.m_szGameSrvIp,ARRAY_SIZE(InitClient.m_ChatServerIp));

	const std::string & strVersion =  GameServer::GetInstance()->GetConfigServer()->GetClientVersion();

	strncpy(InitClient.m_szServerCnfgVersion,strVersion.c_str(),MEM_SIZE(InitClient.m_szServerCnfgVersion));

	const std::string & strResVersion =  GameServer::GetInstance()->GetConfigServer()->GetClientResVersion();

	strncpy(InitClient.m_szServerResVersion,strResVersion.c_str(),MEM_SIZE(InitClient.m_szServerResVersion));


	InitClient.m_FileTotalSize = 0;

	if(this->m_bNeedUpdateCnfg)
	{
		InitClient.m_FileTotalSize = GameServer::GetInstance()->GetClientCnfgFileSize();

		//这些文件需要先下发
		std::vector<std::string> vectCommon = FileSystem::GetAllFileName("Config/clientcommon/");
		for(int i=0; i<vectCommon.size(); ++i)
		{
			CheckCnfgFile(vectCommon[i],"0");
		}
	}	

	if(this->m_bNeedUpdateRes)
	{
		InitClient.m_FileTotalSize += GameServer::GetInstance()->GetClientResFileSize();
	}

	obRsp << GameFrameHeader(enGameFrameCmd_SC_InitClient,SIZE_OF(InitClient)) << InitClient;

	m_pSession->SendData(obRsp.TakeOsb());

	if(this->m_bNeedUpdateCnfg)
	{
		GameServer::GetInstance()->SendClientCnfgFile(m_pSession);
	}	

	if(this->m_bNeedUpdateRes)
	{
		GameServer::GetInstance()->SendClientResFile(m_pSession);
	}	
}

void EnterServerState::Check_Build_Actor(OBuffer4k & ob)
{
	if( m_BuildActor.IsOK() == false){
		return;
	}

	//再次判断玩家是否已存在
	IActor * pActor = GameServer::GetInstance()->GetGameWorld()->FindActor(UID(m_BuildActor.m_pActorBasicData->uid));

	if(pActor != 0)
	{
		KickUser(pActor);
		return ;
	}



	TSceneID SecenID = GameServer::GetInstance()->GetGameWorld()->GetMainSceneID();

	NoticClientInit(SecenID,UID(m_BuildActor.m_pActorBasicData->uid));	

	//创建角色

	IThing * pThing = GameServer::GetInstance()->GetGameWorld()->CreateThing(enThing_Class_Actor,SecenID,(char*)&m_BuildActor,SIZE_OF(m_BuildActor),THING_CREATE_FLAG_DB_DATA);
	if(pThing==0)
	{
		HandleEnterGameRsp(enEnterGameRetCode_Unknow,ob);
		TRACE("<error> %s : %d 创建角色失败!",__FUNCTION__,__LINE__);
		return;
	}

	pActor = (IActor*)pThing;

	m_pSession->SetActor(pActor);

	pActor->SetSession(m_pSession);

	m_bSuccessBuildUser = true;

	for( int k = 0; k < m_BuildActor.m_nEmployNum && k < MAX_EMPLOY_NUM; ++k)
	{
		GameServer::GetInstance()->GetGameWorld()->CreateEmploy(*m_BuildActor.m_EmployBuildData[k]);	
	}

	pActor->SetIsSelfOnline(m_bSelfEnter);

	ActorLogin(pActor);

	ITaskPart * pTaskPart = pActor->GetTaskPart();
	if( 0 == pTaskPart){
		delete pActor;
		return;			
	}

	//如果是新创建的角色给予赠送物品及装备
	if(m_BuildActor.m_bInitCreate)
	{
		GameServer * pGameServer = GameServer::GetInstance();

		const SCreateActorCnfg & CreateActorCnfg = pGameServer->GetConfigServer()->GetCreateActorCnfg();

		//给装备
		IEquipPart * pEquipPart = pActor->GetEquipPart();

		for(int i=0; i<CreateActorCnfg.m_vectEquip.size(); ++i)
		{
			TGoodsID GoodsID = CreateActorCnfg.m_vectEquip[i];
			INT32    Number = 1;

			const SGoodsCnfg * pGoodsCnfg = pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);
			if(pGoodsCnfg == 0)
			{
				TRACE("<error> %s : %d line 找不到物品配置信息 goodsid=%d ",__FUNCTION__,__LINE__,GoodsID);
				continue;
			}

			if(pGoodsCnfg->m_GoodsClass != enGoodsCategory_Equip)
			{
				TRACE("<error> %s : %d line 该物品不是装备 goodsid=%d ",__FUNCTION__,__LINE__,GoodsID);
				continue;
			}

			SCreateGoodsContext CreateGoodsCnt;
			CreateGoodsCnt.m_GoodsID = GoodsID;
			CreateGoodsCnt.m_Number = Number;

			IGoods * pGoods = pGameServer->GetGameWorld()->CreateGoods(CreateGoodsCnt);

			if(pGoods == 0)
			{
				TRACE("<error> %s : %d line 创建物品失败 goodsid=%d ",__FUNCTION__,__LINE__,GoodsID);
				continue;
			}

			//通知客户端创建
			pActor->NoticClientCreatePrivateThing(pGoods->GetUID());

			if(pEquipPart->AddEquip(pGoods->GetUID(),pGoodsCnfg->m_SubClass)==false)
			{
				TRACE("<error> %s : %d line 穿装备失败 goodsid=%d ",__FUNCTION__,__LINE__,GoodsID);

				pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
				continue;
			}

			GameServer::GetInstance()->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_SystemGet,GoodsID,pGoods->GetUID(),Number,"创建玩家给予物品");
		}

		//给物品
		IPacketPart * pPacketPart = pActor->GetPacketPart();
		for(int i=0; i<CreateActorCnfg.m_vectPacket.size()/2; ++i)
		{
			TGoodsID GoodsID = CreateActorCnfg.m_vectPacket[i*2];
			INT32    Number = CreateActorCnfg.m_vectPacket[i*2+1];

			if(pPacketPart->AddGoods(GoodsID,Number)==false)
			{
				TRACE("<error> %s : %d line 给予物品失败 goodsid=%d ",__FUNCTION__,__LINE__,GoodsID);
				continue;
			}

			GameServer::GetInstance()->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_SystemGet,GoodsID,UID(),Number,"创建玩家给予物品");
		}

		//防止创建角色时，服务器挂掉
		pEquipPart->SaveData();

		pPacketPart->SaveData();
	}

	SS_ActoreCreateContext ActoreCreateCnt;
	ActoreCreateCnt.m_uidActor = pActor->GetUID().ToUint64();
	ActoreCreateCnt.m_bNewUser = m_BuildActor.m_bInitCreate;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);
	pActor->OnEvent(msgID,&ActoreCreateCnt,SIZE_OF(ActoreCreateCnt));


	m_BuildActor.Release();
}

bool EnterServerState::ActorLogin(IActor * pActor)
{
	//通知客户端创建玩家

	pActor->NoticClientCreatePrivateThing(pActor->GetUID());

	TSceneID SceneID;
	SceneID.From(pActor->GetCrtProp(enCrtProp_ActorMainSceneID));
	IGameScene * pGameScene = GameServer::GetInstance()->GetGameWorld()->GetGameScene(SceneID);

	if(pGameScene == 0)
	{
		return false;
	}

	ISession * pSession = pActor->GetSession();
	if(pSession && pSession->IsOnLine())
	{
		//正常玩家才登记上线，斗法装载的玩家不算在线
		OBuffer4k ob;

		//通知数据库用户登陆了
		DB_UserEnterGame_Req  Req;
		Req.m_ServerID = GameServer::GetInstance()->GetServerID();
		Req.m_UserID   = pActor->GetCrtProp(enCrtProp_ActorUserID);
		Req.m_GodStone = pActor->GetCrtProp(enCrtProp_ActorMoney);
		strncpy(Req.szIp,m_pSession->GetRemoteIP(),ARRAY_SIZE(Req.szIp));
		strncpy(Req.m_szActorName,pActor->GetName(),ARRAY_SIZE(Req.m_szActorName));
		Req.m_bFirstEnter = m_BuildActor.m_bInitCreate;

		OBuffer1k DBOb;
		DBOb << Req;

		GameServer::GetInstance()->RequestDBByActor(pActor,enDBCmd_UserEnterGame,DBOb.TakeOsb());
	}



	pActor->GetPacketPart()->ClearSendCnfg();

	//发布玩家登陆事件
	SS_ActorLogin ActorLogin;
	ActorLogin.m_uidActor = pActor->GetUID().ToUint64();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogin);
	pActor->OnEvent(msgID,&ActorLogin,sizeof(ActorLogin));


	EmployeeLogin(pActor);

	pGameScene->EnterScene(pActor);

	OBuffer4k ob;
	HandleEnterGameRsp(enEnterGameRetCode_Ok,ob);

	for(int i=enThingPart_Crt_Basic; i<enThingPart_Crt_Max;i++)
	{
		IThingPart* pThingPart = pActor->GetPart((enThingPart)i);
		if(pThingPart)
		{
			pThingPart->InitPrivateClient();
		}
	}




	//转入游戏态
	m_pSession->GotoState(enGamePlayState,NULL);


	return true;
}

bool	EnterServerState::EmployeeLogin(IActor * pMaster)
{
	IActor * pEmployee = 0;

	static UINT8 EmployPart[enThingPart_Crt_Max] =
	{
		1,			//生物的基础系统
		1,			//状态
		1,			//战斗
		0,
		0,
		0,			//资源产出部件
		0,			//伏魔洞
		0,			//练功堂
		0,			//聚仙楼
		0,			//背包thingpart
		1,			//装备栏
		0,			//修炼
		0,			//好友
		0,			//邮件
		1,			//法术部件
		0,			//副本
		0,			//帮派技能
		0,			//剑冢
		0,			//法宝
		0,			//斗法
		0,          //任务
		0,			//成就
		0,			//活动
		1,			//冷却时间
		0,			//组队
		0,			//帮派
	};

	for( int i = 0; i < MAX_EMPLOY_NUM; ++i,pEmployee = 0)
	{
		pEmployee = pMaster->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		//发数据给客户端
		pEmployee->NoticClientCreatePrivateThing(pEmployee->GetUID());


		for(int k = enThingPart_Crt_Basic; k < enThingPart_Crt_Max; ++k)
		{		
			if ( 0 == EmployPart[k] )
				continue;

			IThingPart* pThingPart = pEmployee->GetPart((enThingPart)k);
			if(pThingPart){
				pThingPart->InitPrivateClient();
			}
		}
	}

	return true;
}

//向数据库请求得到所有part数据
void	EnterServerState::GetThingPartDataCreateNewUser(UINT64 uidUser, TUserID UserID)
{
	GameServer * pGameServer = GameServer::GetInstance();
	if( 0 == pGameServer){
		return;
	}

	OBuffer4k  ob;
	//ThingPart
	//ResOutputPart
	m_BuildActor.m_BuildingLen      	= SIZE_OF(SDB_Get_BuildingData_Rsp())*ResOutputBuildingType_Max;

	m_BuildActor.m_pBuildingData		= new char[m_BuildActor.m_BuildingLen];;

	SDB_Get_BuildingData_Rsp * pBuildingData_Rsp = (SDB_Get_BuildingData_Rsp *)m_BuildActor.m_pBuildingData;
	for( int i = 0; i < ResOutputBuildingType_Max; ++i,pBuildingData_Rsp++)
	{		
		pBuildingData_Rsp->m_BuildingType = enBuildingType_Beast+i;
		pBuildingData_Rsp->m_Uid = UID().ToUint64();
		pBuildingData_Rsp->m_uidOwner = UID(uidUser).ToUint64();

	}

	//FuMoDongPart
	SDB_Get_FuMoDongData_Rsp * pFuMoDongData = new SDB_Get_FuMoDongData_Rsp;
	m_BuildActor.m_pFuMoDongData = pFuMoDongData;

	//TrainingHallPart
	SDB_Get_TrainingHallData_Rsp * pTrainingHall = new SDB_Get_TrainingHallData_Rsp;
	m_BuildActor.m_pTrainingHall = pTrainingHall;

	//GatherGodHousePart
	SDB_Get_GatherGodHouseData_Rsp * pGatherGodHouse = new SDB_Get_GatherGodHouseData_Rsp;
	m_BuildActor.m_pGatherGodHouseData = pGatherGodHouse;

	//XiuLianPart
	m_BuildActor.m_pXiuLianData = new char[sizeof(SDB_Get_XiuLianData_Rsp)];
	m_BuildActor.m_XiuLianDataSize = SIZE_OF(SDB_Get_XiuLianData_Rsp());
	*(SDB_Get_XiuLianData_Rsp *)m_BuildActor.m_pXiuLianData = SDB_Get_XiuLianData_Rsp();

	//FuBenPart
	SDBFuBenData * pFuBenData = new SDBFuBenData;
	m_BuildActor.m_pFuBenData = pFuBenData;

	//GodSwordShopPart
	SDB_Get_GodSwordShopData_Rsp * pGodSwordShop = new SDB_Get_GodSwordShopData_Rsp;
	m_BuildActor.m_pGodSwordShopData = pGodSwordShop;

	//PacketPart
	m_BuildActor.m_PacketData.m_pData = new char[sizeof(SDB_Get_PacketData_Rsp)];
	m_BuildActor.m_PacketData.m_nLen  = sizeof(SDB_Get_PacketData_Rsp);
	*(SDB_Get_PacketData_Rsp *)m_BuildActor.m_PacketData.m_pData = SDB_Get_PacketData_Rsp();

	//EquipPart
	SDBEquipPanel * pEquipPanel = new SDBEquipPanel;
	m_BuildActor.m_pEquipPanel = pEquipPanel;

	//MagicPart
	SDBMagicPanelData * pMagicPanel = new SDBMagicPanelData;
	m_BuildActor.m_pMagicPanelData = pMagicPanel;

	//SynMagicPart
	SDBSynMagicPanelData * pSynMagicPanel = new SDBSynMagicPanelData;
	m_BuildActor.m_pSynMagicPanelData = pSynMagicPanel;

	//CombatPart
	SDBCombatData * pCombatData = new SDBCombatData;
	m_BuildActor.m_pCombatData = pCombatData;

	//StatusPart
	m_BuildActor.m_StatusPart.m_pData	= new char[sizeof(SDB_Get_StatusNum_Rsp)];
	m_BuildActor.m_StatusPart.m_Len	  = sizeof(SDB_Get_StatusNum_Rsp);
	*(SDB_Get_StatusNum_Rsp *)m_BuildActor.m_StatusPart.m_pData = SDB_Get_StatusNum_Rsp();

	//DouFaPart
	m_BuildActor.m_pDouFaPart = new SDB_Get_DouFaPartInfo_Rsp;
	*m_BuildActor.m_pDouFaPart = SDB_Get_DouFaPartInfo_Rsp();

	//TaskPart
	m_BuildActor.m_TaskPart.m_pData = new char[SIZE_OF(DB_TaskPartData())];
	m_BuildActor.m_TaskPart.m_Len	= SIZE_OF(DB_TaskPartData());
	m_BuildActor.m_TaskPart.m_bNewUser = true;
	*(DB_TaskPartData *)m_BuildActor.m_TaskPart.m_pData = DB_TaskPartData();

	//ChengJiuPart
	m_BuildActor.m_ChengJiuData.m_pData = new char[SIZE_OF(DB_ChengJiuPart())];
	m_BuildActor.m_ChengJiuData.m_Len	= SIZE_OF(DB_ChengJiuPart());
	*(DB_ChengJiuPart *)m_BuildActor.m_ChengJiuData.m_pData = DB_ChengJiuPart();

	//活动
	m_BuildActor.m_pActivityData = new char[SIZE_OF(SDB_GetActivityPart_Rsp())];
	m_BuildActor.m_ActivityDataLen = SIZE_OF(SDB_GetActivityPart_Rsp());

	*(SDB_GetActivityPart_Rsp*)m_BuildActor.m_pActivityData = SDB_GetActivityPart_Rsp();

	//冷却时间
	m_BuildActor.m_CDTimerDBData.m_pData = new char[SIZE_OF(SDB_CDTimerData())];
	m_BuildActor.m_CDTimerDBData.m_Len   = SIZE_OF(SDB_CDTimerData());
	*(SDB_CDTimerData *)m_BuildActor.m_CDTimerDBData.m_pData = SDB_CDTimerData();

	//帮派PART
	m_BuildActor.m_pSynPart = new SDB_SynPart;

	//法宝PART
	m_BuildActor.m_pTalismanPart = new SDB_TalismanPart;
	
	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(ob);
}

void EnterServerState::GetThingPartDataEnterGame(UINT64 uidUser, TUserID UserID)
{
	GameServer * pGameServer = GameServer::GetInstance();
	if( 0 == pGameServer){
		return;
	}
	OBuffer1k  ob;

	//资源产出型建筑PART
	SDB_GetBuildingData_Req		ReqBuild;
	ReqBuild.Uid_User = uidUser;

	ob << ReqBuild;
	GameServer::GetInstance()->GetDBProxyClient()->Request(UserID,enDBCmd_GetBuildingInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());

	//伏魔洞PART
	SDB_Get_FuMoDongData_Req	ReqFuMoDong;
	ReqFuMoDong.Uid_User = uidUser;

	ob.Reset();
	ob << ReqFuMoDong;
	GameServer::GetInstance()->GetDBProxyClient()->Request(UserID,enDBCmd_GetFuMoDongInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());

	//练功堂PART
	SDB_Get_TrainingHallData_Req ReqTrainingHall;
	ReqTrainingHall.Uid_User = uidUser;

	ob.Reset();
	ob << ReqTrainingHall;
	GameServer::GetInstance()->GetDBProxyClient()->Request(UserID,enDBCmd_GetTrainingHallInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());

	//聚仙楼PART
	SDB_Get_GatherGodHouseData_Req RetGatherGodHouse;
	RetGatherGodHouse.Uid_User = uidUser;

	ob.Reset();
	ob << ReqTrainingHall;
	GameServer::GetInstance()->GetDBProxyClient()->Request(UserID,enDBCmd_GetGatherGodHouseInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());

	//修炼PART
	SDB_Get_XiuLianData_Req		ReqXiuLian;
	ReqXiuLian.Uid_User = uidUser;

	ob.Reset();
	ob << ReqXiuLian;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetXiuLianInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//副本Part
	SDB_Get_FuBenData_Req ReqFuBen;
	ReqFuBen.Uid_User = uidUser;

	ob.Reset();
	ob << ReqFuBen;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetFuBenInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//剑冢PART
	SDB_Get_GodSwordShopData_Req ReqGodSwordShop;
	ReqGodSwordShop.Uid_User = uidUser;

	ob.Reset();
	ob << ReqGodSwordShop;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetGodSwordShopInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//背包PART
	SDB_Get_PacketData_Req ReqPacket;
	ReqPacket.Uid_User = uidUser;

	ob.Reset();
	ob << ReqPacket;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetPacketInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//EquipPart
	SDB_Get_EquipPanelData_Req ReqEquip;
	ReqEquip.Uid_User		= uidUser;

	ob.Reset();
	ob << ReqEquip;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetEquipInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//MagicPart
	SDB_Get_MagicPanelData_Req ReqMagicPanel;
	ReqMagicPanel.Uid_User = uidUser;

	ob.Reset();
	ob << ReqMagicPanel;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetMagicPanelInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//SynMagicPart
	SDB_Get_SynMagicData_Req ReqSynMagic;
	ReqSynMagic.Uid_User = uidUser;

	ob.Reset();
	ob << ReqSynMagic;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetSynMagicInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//CombatPart
	SDB_Get_CombatData_Req ReqCombat;
	ReqCombat.Uid_User = uidUser;

	ob.Reset();
	ob << ReqCombat;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetCombatInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//StatusPart
	SDB_Get_Status_Req ReqStatus;
	ReqStatus.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqStatus;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetStatusInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//DouFaPart
	SDB_Get_DouFaPartInfo_Req ReqDouFa;
	ReqDouFa.m_UserUID = uidUser;

	ob.Reset();
	ob << ReqDouFa;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetDouFaPartInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//TaskPart
	SDB_Get_TaskPartInfo_Req ReqTask;
	ReqTask.m_UserUID = uidUser;

	ob.Reset();
	ob << ReqTask;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetTaskPartInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//ChengJiuPart
	SDB_Get_ChengJiuData_Req ReqChengJiu;
	ReqChengJiu.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqChengJiu;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetChengJiuInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//ActivityPart
	SDB_Get_ActivityData_Req ReqActivity;
	ReqActivity.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqActivity;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetActivityPartInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//冷却时间
	SDB_Get_UserCDTimer_Req ReqCDTimer;
	ReqCDTimer.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqCDTimer;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetCDTimer,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//帮派PART
	SDB_Get_SynPart_Req DBSynPart;
	DBSynPart.m_uidUser = uidUser;

	ob.Reset();
	ob << DBSynPart;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_Get_SynPart,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//法宝PART
	SDB_Get_TalismanPart_Req DBTalisman;
	DBTalisman.m_UIDUser = uidUser;

	ob.Reset();
	ob << DBTalisman;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_Get_TalismanPart,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());
}

//招募角色的PART数据获取
void EnterServerState::GetThingPartDataEmployEnter(UINT64 uidUser, TUserID UserID)
{
	GameServer * pGameServer = GameServer::GetInstance();
	if( 0 == pGameServer){
		return;
	}
	OBuffer1k  ob;

	//EquipPart
	SDB_Get_EquipPanelData_Req ReqEquip;
	ReqEquip.Uid_User		= uidUser;

	ob.Reset();
	ob << ReqEquip;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetEquipInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//MagicPart
	SDB_Get_MagicPanelData_Req ReqMagicPanel;
	ReqMagicPanel.Uid_User = uidUser;

	ob.Reset();
	ob << ReqMagicPanel;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetMagicPanelInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//StatusPart
	SDB_Get_Status_Req ReqStatus;
	ReqStatus.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqStatus;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetStatusInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//CombatPart
	SDB_Get_CombatData_Req ReqCombat;
	ReqCombat.Uid_User = uidUser;

	ob.Reset();
	ob << ReqCombat;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetCombatInfo,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());

	//CDTimerPart
	SDB_Get_UserCDTimer_Req ReqCDTimer;
	ReqCDTimer.m_uidUser = uidUser;

	ob.Reset();
	ob << ReqCDTimer;
	pGameServer->GetDBProxyClient()->Request(UserID,enDBCmd_GetCDTimer,ob.TakeOsb(),pGameServer,m_pSession->GetSessionID());
}

enPlayerSessionState	EnterServerState::GetStateID()
{
	return enEnterSceneServerState;
}

//装载玩家入内存
void		EnterServerState::LoadActor(const UID & UID_Actor)
{
	m_bSelfEnter = false;

	SDB_Get_ActorBasicDataByUID_Req Req;
	Req.Uid_User = UID_Actor.ToUint64();

	OBuffer1k  ob;
	ob.Reset();
	ob << Req;

	GameServer::GetInstance()->GetDBProxyClient()->Request(0,enDBCmd_GetActorBasicDataByUIDInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());
}

//资源产出型建筑PART
void	EnterServerState::HandleGetBuildinData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											   OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam ;
	RspIb >> RspHeader >> OutParam ;

	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());

	//玩家的数据获取
	char * ptr = new char[RspIb.Remain()];

	memcpy(ptr,RspIb.CurrentBuffer(),RspIb.Remain());


	//玩家的数据获取
	m_BuildActor.m_pBuildingData		= ptr;
	m_BuildActor.m_BuildingLen      	= RspIb.Remain();

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//伏魔洞PART
void	EnterServerState::HandleGetFuMoDongData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
												OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_FuMoDongData_Rsp		FuMoDongData_Rsp;
	RspIb >> RspHeader >> OutParam >> FuMoDongData_Rsp;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//玩家的数据获取
	m_BuildActor.m_pFuMoDongData	= new SDB_Get_FuMoDongData_Rsp();
	*m_BuildActor.m_pFuMoDongData	= FuMoDongData_Rsp;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//练功堂PART
void	EnterServerState::HandleGetTrainingHallData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
													OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_TrainingHallData_Rsp	TrainingHall_Rsp;
	RspIb >> RspHeader >> OutParam >> TrainingHall_Rsp;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//玩家的数据获取
	m_BuildActor.m_pTrainingHall	= new SDB_Get_TrainingHallData_Rsp();
	*m_BuildActor.m_pTrainingHall	= TrainingHall_Rsp;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//聚仙楼
void	EnterServerState::HandleGetGatherGodHouseData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
													  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_GatherGodHouseData_Rsp	GatherGodHouse_Rsp;
	RspIb >> RspHeader >> OutParam >> GatherGodHouse_Rsp;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//玩家的数据获取
	m_BuildActor.m_pGatherGodHouseData	= new SDB_Get_GatherGodHouseData_Rsp();
	*m_BuildActor.m_pGatherGodHouseData	= GatherGodHouse_Rsp;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//修炼PART
void EnterServerState::HandleGetXiuLianData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_Get_XiuLianData_Rsp				XiuLianData_Rsp;
	RspIb >> RspHeader >> XiuLianData_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//玩家的数据获取
	INT32 Remain = RspIb.Remain();
	m_BuildActor.m_pXiuLianData		= new char[SIZE_OF(SDB_Get_XiuLianData_Rsp())+Remain];
	memcpy(m_BuildActor.m_pXiuLianData, &XiuLianData_Rsp,SIZE_OF(SDB_Get_XiuLianData_Rsp()));
	if(Remain>0)
	{
		memcpy(m_BuildActor.m_pXiuLianData+SIZE_OF(SDB_Get_XiuLianData_Rsp()),RspIb.CurrentBuffer(),Remain);
	}
	m_BuildActor.m_XiuLianDataSize = SIZE_OF(SDB_Get_XiuLianData_Rsp()) + Remain;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//副本PART
void EnterServerState::HandleGetFuBenData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
										  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_Get_BasicFuBenData_Rsp		BasicFuBenData_Rsp;
	RspIb >> RspHeader >> BasicFuBenData_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( 0 == BasicFuBenData_Rsp.Uid_User){
		HandleEnterGameRsp(enEnterGameRetCode_DBError,RspOb);
		return;
	}

	SDBFuBenData * pFuBenData = new SDBFuBenData();

	pFuBenData->m_CostStoneEnterFuBenNum = BasicFuBenData_Rsp.CostStoneEnterFuBenNum;
	pFuBenData->m_SynWelfareEnterFuBenNum = BasicFuBenData_Rsp.SynWelfareEnterFuBenNum;
	pFuBenData->m_FreeEnterFuBenNum		 = BasicFuBenData_Rsp.FreeEnterFuBenNum;
	pFuBenData->m_FuBenNum				 = RspIb.Remain()/SIZE_OF(SDB_Get_FuBenProgressData_Rsp());
	pFuBenData->m_LastFreeEnterFuBenTime = BasicFuBenData_Rsp.LastFreeEnterFuBenTime;
	pFuBenData->m_LastStoneEnterFuBenTime = BasicFuBenData_Rsp.LastStoneEnterFuBenTime;
	pFuBenData->m_LastSynWelfareEnterFuBenTime = BasicFuBenData_Rsp.LastSynWelfareEnterFuBenTime;
	pFuBenData->m_LastEnterFuBenGodSword  = BasicFuBenData_Rsp.LastEnterFuBenGodSword;
	pFuBenData->m_EnterSynFuBenNum		 = BasicFuBenData_Rsp.EnterSynFuBenNum;
	pFuBenData->m_LastEnterSynFuBenTime	 = BasicFuBenData_Rsp.LastEnterSynFuBenTime;
	pFuBenData->m_EnterGodSwordWorldNum  = BasicFuBenData_Rsp.m_EnterGodSwordWorldNum;
	pFuBenData->m_LastVipEnterFuBenTime  = BasicFuBenData_Rsp.LastVipEnterFuBenTime;
	pFuBenData->m_VipEnterFuBenNum		 = BasicFuBenData_Rsp.VipEnterFuBenNum;
	pFuBenData->m_FinishedFuBen.set((unsigned char*)BasicFuBenData_Rsp.m_FinishedFuBen, sizeof(BasicFuBenData_Rsp.m_FinishedFuBen));
	pFuBenData->m_HardFinishedFuBen.set((unsigned char*)BasicFuBenData_Rsp.m_HardFinishedFuBen, sizeof(BasicFuBenData_Rsp.m_HardFinishedFuBen));

	for( int i = 0; i < pFuBenData->m_FuBenNum && i < MAX_OPEN_FUBEN_NUM; ++i){
				
		SDB_Get_FuBenProgressData_Rsp  FuBenProgressData_Rsp;
		RspIb >> FuBenProgressData_Rsp;

		if( RspIb.Error()){
			TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SFuBenProgress FuBenProgress;
		FuBenProgress.m_FuBenID = FuBenProgressData_Rsp.FuBenID;
		FuBenProgress.m_KillMonsterNum	= FuBenProgressData_Rsp.KillMonsterNum;
		FuBenProgress.m_Level			= FuBenProgressData_Rsp.Level;

		pFuBenData->m_FuBenProgress[i] = FuBenProgress;
	}

	if( BasicFuBenData_Rsp.Uid_User == m_BuildActor.m_pActorBasicData->uid){

		m_BuildActor.m_pFuBenData = pFuBenData;

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}
}

//剑冢PART
void	EnterServerState::HandleGetGodSwordShopData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
													OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_GodSwordShopData_Rsp       GodSwordShop_Rsp;
	RspIb >> RspHeader >> OutParam >> GodSwordShop_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	//if( OutParam.retCode != enDBRetCode_OK){
	//	HandleEnterGameRsp(enEnterGameRetCode(OutParam.retCode),RspOb);
	//	return;
	//}

	//玩家的数据获取
	m_BuildActor.m_pGodSwordShopData  = new SDB_Get_GodSwordShopData_Rsp();
	*m_BuildActor.m_pGodSwordShopData = GodSwordShop_Rsp;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//背包PART
void	EnterServerState::HandleGetPacketData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	RspIb >> RspHeader;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	m_BuildActor.m_PacketData.m_pData = new char[RspIb.Remain()];
	m_BuildActor.m_PacketData.m_nLen  = RspIb.Remain();
	memcpy(m_BuildActor.m_PacketData.m_pData, RspIb.CurrentBuffer(), RspIb.Remain());

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//装备PART
void	EnterServerState::HandleGetEquipPanelData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
												  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_Get_EquipGoodsNum_Rsp		EquipNum_Rsp;
	RspIb >> RspHeader >> EquipNum_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SDBEquipPanel * pEquipPanel = new SDBEquipPanel();

	strncpy(pEquipPanel->m_szFacade,EquipNum_Rsp.m_szFacade,sizeof(pEquipPanel->m_szFacade));

	INT32 nCount = RspIb.Remain()/sizeof(SDB_Get_EquipGoodsData_Rsp);

	for( int i = 0; i < nCount && i < MAX_EQUIP_NUM; ++i)
	{
		SDB_Get_EquipGoodsData_Rsp Rsp;
		RspIb >> Rsp;

		if (RspIb.Error()){

			TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		pEquipPanel->m_GoodsNum = 0;
		pEquipPanel->m_GoodsData[i].m_Location = Rsp.m_Location;
		pEquipPanel->m_GoodsData[i].m_uidGoods = UID(Rsp.uidGoods);
		pEquipPanel->m_GoodsData[i].m_GoodsID  = Rsp.GoodsID;
		pEquipPanel->m_GoodsData[i].m_CreateTime = Rsp.CreateTime;
		pEquipPanel->m_GoodsData[i].m_Number   = Rsp.Number;
		pEquipPanel->m_GoodsData[i].m_Binded	= Rsp.Binded;
		memcpy(&pEquipPanel->m_GoodsData[i].m_TalismanProp, &Rsp.GoodsData, SIZE_OF(SDBTalismanProp()));
	}

	if( EquipNum_Rsp.Uid_User == m_BuildActor.m_pActorBasicData->uid){
		//玩家的数据获取
		m_BuildActor.m_pEquipPanel = pEquipPanel;

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}else{
		//招募角色的数据获取
		for( int i = 0; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
		{
			if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == EquipNum_Rsp.Uid_User){
				m_BuildActor.m_EmployBuildData[i]->m_pEquipPanel = pEquipPanel;

				//检查招募角色所有PART是否都创建成功，成功则创建角色
				Check_Build_Actor(RspOb);
			}
		}
	}

}

//法术栏PART
void	EnterServerState::HandleGetMagicPanelData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
												  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_GetMagicPanelNum_Rsp	MagicNum_Rsp;
	RspIb >> RspHeader >> MagicNum_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SDBMagicPanelData * pMagicPanel = new SDBMagicPanelData();

	pMagicPanel->m_MagicInfoNum = MagicNum_Rsp.MagicNum;

	for( int i = 0; i < MagicNum_Rsp.MagicNum && i < MAX_STUDY_MAGIC_NUM; ++i)
	{
		SDB_GetMagicInfo_Rsp Rsp;
		RspIb >> Rsp;

		if (RspIb.Error()){

			TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		if( 0 != Rsp.Position && MAX_EQUIP_MAGIC_NUM >= Rsp.Position && 0 < Rsp.Position){
			pMagicPanel->m_EquipMagic[Rsp.Position - 1]	 = Rsp.MagicID;
		}
		pMagicPanel->m_MagicInfo[i].m_Level   = Rsp.Level;
		pMagicPanel->m_MagicInfo[i].m_MagicID = Rsp.MagicID;
	}

	if( MagicNum_Rsp.Uid_User == m_BuildActor.m_pActorBasicData->uid){
		//玩家的数据获取
		m_BuildActor.m_pMagicPanelData = pMagicPanel;

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}else{
		//招募角色的数据获取
		for( int i = 0; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
		{
			if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == MagicNum_Rsp.Uid_User){
				m_BuildActor.m_EmployBuildData[i]->m_pMagicPanelData = pMagicPanel;

				//检查招募角色所有PART是否都创建成功，成功则创建角色
				Check_Build_Actor(RspOb);
			}
		}
	}	
}

//帮派技能
void	EnterServerState::HandleGetSynMagicData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
												OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDBSynMagicNumData   SynMagicNum;
	RspIb >> RspHeader >> SynMagicNum;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SDBSynMagicPanelData * pSynMagic = new SDBSynMagicPanelData();

	pSynMagic->m_SynMagicNum = SynMagicNum.m_SynMagicNum;

	for( int i = 0; i < SynMagicNum.m_SynMagicNum && i < MAX_SYNMAGICNUM; ++i)
	{
		SDB_Get_SynMagicData_Rsp SynMagicData;
		RspIb >> SynMagicData;

		if (RspIb.Error()){

			TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		pSynMagic->m_SynMagicData[i].m_SynMagicID =  SynMagicData.m_SynMagicID;
		pSynMagic->m_SynMagicData[i].m_SynMagicLevel = SynMagicData.m_SynMagicLevel;
	}

	//玩家的数据获取
	m_BuildActor.m_pSynMagicPanelData = pSynMagic;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//战斗数据
void	EnterServerState::HandleGetCombatData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											  OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_CombatData_Rsp       CombatData_Rsp;
	RspIb >> RspHeader >> OutParam >> CombatData_Rsp;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		if(!m_bSuccessBuildUser){
			HandleEnterGameRsp(enEnterGameRetCode(OutParam.retCode),RspOb);
			return;
		}
	}

	SDBCombatData * pCombatData = new SDBCombatData();

	pCombatData->m_uidLineup[0] = UID(CombatData_Rsp.uidLineup1);
	pCombatData->m_uidLineup[1] = UID(CombatData_Rsp.uidLineup2);
	pCombatData->m_uidLineup[2] = UID(CombatData_Rsp.uidLineup3);
	pCombatData->m_uidLineup[3] = UID(CombatData_Rsp.uidLineup4);
	pCombatData->m_uidLineup[4] = UID(CombatData_Rsp.uidLineup5);
	pCombatData->m_uidLineup[5] = UID(CombatData_Rsp.uidLineup6);
	pCombatData->m_uidLineup[6] = UID(CombatData_Rsp.uidLineup7);
	pCombatData->m_uidLineup[7] = UID(CombatData_Rsp.uidLineup8);
	pCombatData->m_uidLineup[8] = UID(CombatData_Rsp.uidLineup9);

	if( CombatData_Rsp.Uid_User == m_BuildActor.m_pActorBasicData->uid){
		//玩家的数据获取
		m_BuildActor.m_pCombatData = pCombatData;

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}else{
		//招募角色的数据获取
		for( int i = 0; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
		{
			if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == CombatData_Rsp.Uid_User){
				m_BuildActor.m_EmployBuildData[i]->m_pCombatData = pCombatData;

				//检查招募角色所有PART是否都创建成功，成功则创建角色
				Check_Build_Actor(RspOb);
			}
		}
	}
}

//状态数据
void EnterServerState::HandleGetStatusData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
										   OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_Get_StatusNum_Rsp RspNum;

	RspIb >> RspHeader >> RspNum;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	char * ptr = new char[RspIb.Remain()];
	memcpy(ptr,RspIb.CurrentBuffer(),RspIb.Remain());

	if( RspNum.m_uidUser == m_BuildActor.m_pActorBasicData->uid){

		m_BuildActor.m_StatusPart.m_pData = ptr;
		m_BuildActor.m_StatusPart.m_Len	= RspIb.Remain();

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}else{
		//招募角色的数据获取
		for( int i = 0; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
		{
			if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == RspNum.m_uidUser){
				m_BuildActor.m_EmployBuildData[i]->m_StatusPart.m_pData		= ptr;
				m_BuildActor.m_EmployBuildData[i]->m_StatusPart.m_Len			= RspIb.Remain();

				//检查招募角色所有PART是否都创建成功，成功则创建角色
				Check_Build_Actor(RspOb);
			}
		}	
	}

}


//获得玩家基本数据，并创建玩家
void	EnterServerState::HandleGetActorBasicDataAndPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
														 OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		HandleEnterGameRsp(enEnterGameRetCode(OutParam.retCode),RspOb);
		return;
	}

	SDB_Get_ActorBasicData_Rsp		ActorBasicData_Rsp;
	RspIb >> ActorBasicData_Rsp;

	//先判断玩家是否已存在
	IActor * pActor = GameServer::GetInstance()->GetGameWorld()->FindActor(UID(ActorBasicData_Rsp.uid));

	if(pActor==0)
	{
		m_BuildActor.m_pActorBasicData		= new SDB_Get_ActorBasicData_Rsp();

		*m_BuildActor.m_pActorBasicData	= ActorBasicData_Rsp;

		//得到玩家招募角色数据
		SDB_Get_EmployeeData_Req	ReqEmployee;
		ReqEmployee.Uid_User = ActorBasicData_Rsp.uid;

		OBuffer1k  ob;	
		ob.Reset();
		ob << ReqEmployee;
		GameServer::GetInstance()->GetDBProxyClient()->Request(ActorBasicData_Rsp.UserID,enDBCmd_GetActorEmployInfo,ob.TakeOsb(),GameServer::GetInstance(),m_pSession->GetSessionID());

		if( enDBCmd_GetActorBasicDataInfo == ReqCmd){
			//向数据库请求得到所有part数据
			this->GetThingPartDataEnterGame(ActorBasicData_Rsp.uid, ActorBasicData_Rsp.UserID);
		}else if( enDBCmd_InsertActorBasicDataInfo == ReqCmd){
			m_BuildActor.m_bInitCreate = true;
			//创建新角色时，插入PART数据
			this->GetThingPartDataCreateNewUser(ActorBasicData_Rsp.uid, ActorBasicData_Rsp.UserID);
		}else if( enDBCmd_GetActorBasicDataByUIDInfo == ReqCmd){
			//向数据库请求得到所有part数据
			this->GetThingPartDataEnterGame(ActorBasicData_Rsp.uid, ActorBasicData_Rsp.UserID);		
		}
	}
	else
	{
		KickUser(pActor);
	}


}

//帐号重复登陆，踢掉旧玩家
void EnterServerState::KickUser(IActor* pActor)
{
	Session * pSession = (Session *)pActor->GetSession();

	
	//删除旧的会话
	pSession->KickUser(enKickType_Repeat);

	pActor->SetSession(0); //先置空

	m_pSession->SetActor(pActor);


	TSceneID SceneID(pActor->GetCrtProp(enCrtProp_ActorMainSceneID));

	//pActor->SetCrtProp(enCrtProp_SceneID,SceneID.m_id);
	//pActor->SetCrtProp(enCrtProp_ActorLastSceneID,SceneID.m_id);

	pActor->SetSession(m_pSession); //这句需要放在pActor->SetCrtProp(enCrtProp_SceneID,SceneID.m_id);语句之后，因为此时客户端还没有创建角色

	NoticClientInit(SceneID,pActor->GetUID());

	ActorLogin(pActor);	
}

//斗法数据
void	EnterServerState::HandleGetDouFaData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											 OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam					OutParam;
	SDB_Get_DouFaPartInfo_Rsp   DouFa_Rsp;
	RspIb >> RspHeader >> OutParam;

	SDB_Get_DouFaPartInfo_Rsp * pDouFaPart = new SDB_Get_DouFaPartInfo_Rsp();

	if (OutParam.retCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d line 获取斗法数据错误!",__FUNCTION__,__LINE__);
	}
	else if ( RspIb.Remain() > sizeof(SDB_Get_DouFaPartInfo_Rsp) )
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}
	else if( RspIb.Error() )
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}
	else
	{
		memcpy(pDouFaPart, RspIb.CurrentBuffer(), RspIb.Remain());
	}

	//玩家的数据获取
	m_BuildActor.m_pDouFaPart = pDouFaPart;

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//任务数据
void EnterServerState::HandleGetTaskData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
										 OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	RspIb >> RspHeader;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	m_BuildActor.m_TaskPart.m_pData = new char[RspIb.Remain()];
	memcpy(m_BuildActor.m_TaskPart.m_pData, RspIb.CurrentBuffer(),RspIb.Remain());

	m_BuildActor.m_TaskPart.m_bNewUser = m_BuildActor.m_bInitCreate;

	m_BuildActor.m_TaskPart.m_Len   = RspIb.Remain();

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//成就数据
void EnterServerState::HandleGetChengJiuData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											 OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	RspIb >> RspHeader;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}


	m_BuildActor.m_ChengJiuData.m_pData = new char[RspIb.Remain()];
	memcpy(m_BuildActor.m_ChengJiuData.m_pData, RspIb.CurrentBuffer(), RspIb.Remain());

	m_BuildActor.m_ChengJiuData.m_Len = RspIb.Remain();

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//活动
void EnterServerState::HandleGetActivityData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											 OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	RspIb >> RspHeader;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}


	m_BuildActor.m_pActivityData = new char[RspIb.Remain()];
	memcpy(m_BuildActor.m_pActivityData, RspIb.CurrentBuffer(), RspIb.Remain());

	m_BuildActor.m_ActivityDataLen = RspIb.Remain();

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//冷却时间
void EnterServerState::HandleGetCDTimerData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
											OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_CDTimerActor	CDTimerActor;

	RspIb >> RspHeader >> CDTimerActor;

	if( RspIb.Error()){
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	char * ptr = new char[RspIb.Remain()];
	memcpy(ptr,RspIb.CurrentBuffer(),RspIb.Remain());

	if( CDTimerActor.m_uidActor == m_BuildActor.m_pActorBasicData->uid){

		m_BuildActor.m_CDTimerDBData.m_pData = ptr;
		m_BuildActor.m_CDTimerDBData.m_Len	 = RspIb.Remain();

		//检查所有PART是否都创建成功，成功则创建角色
		Check_Build_Actor(RspOb);
	}else{
		//招募角色的数据获取
		for( int i = 0; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
		{
			if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == CDTimerActor.m_uidActor){
				m_BuildActor.m_EmployBuildData[i]->m_CDTimerDBData.m_pData		= ptr;
				m_BuildActor.m_EmployBuildData[i]->m_CDTimerDBData.m_Len		= RspIb.Remain();

				//检查招募角色所有PART是否都创建成功，成功则创建角色
				Check_Build_Actor(RspOb);
			}
		}	
	}
}

//帮派PART
void	EnterServerState::HandleGetSynPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam			OutParam;
	SDB_SynPart			SynData;
	RspIb >> RspHeader >> OutParam >> SynData;

	m_BuildActor.m_pSynPart = new SDB_SynPart();

	if( RspIb.Error() )
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}
	else
	{
		memcpy(m_BuildActor.m_pSynPart, &SynData, sizeof(SDB_SynPart));
	}

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);
}

//法宝PART
void	EnterServerState::HandleGetTalismanPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam			OutParam;
	SDB_TalismanPart	TalismanData;
	RspIb >> RspHeader >> OutParam >> TalismanData;

	m_BuildActor.m_pTalismanPart = new SDB_TalismanPart();

	if( RspIb.Error() )
	{
		TRACE("<error> %s : %d line DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}
	else
	{
		memcpy(m_BuildActor.m_pTalismanPart, &TalismanData, sizeof(SDB_TalismanPart));
	}

	//检查所有PART是否都创建成功，成功则创建角色
	Check_Build_Actor(RspOb);	
}

//获取玩家招募角色集的数据
void	EnterServerState::HandleGetEmployeeData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
												OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_GetEmployeeNum_Rsp			  Num_Rsp;
	RspIb >> RspHeader >> Num_Rsp;

	if( RspIb.Error()){
		TRACE("HandleDBRetGetUserInfo DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	m_BuildActor.m_nEmployNum = Num_Rsp.EmployeeNum;

	OBuffer1k  ob;

	for( int i = 0; i < Num_Rsp.EmployeeNum && i < MAX_EMPLOY_NUM; ++i)
	{
		SDB_Get_ActorBasicData_Rsp       ActorBasicData_Rsp;
		RspIb >> ActorBasicData_Rsp;

		m_BuildActor.m_EmployBuildData[i] = new SBuild_Employee();
		m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData  = new SDB_Get_ActorBasicData_Rsp();
		*m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData = ActorBasicData_Rsp;

		//获取PART
		this->GetThingPartDataEmployEnter(ActorBasicData_Rsp.uid, DBUserID); //需要用主角的USERID  
	}
}

//检测是否可以创建招募角色，可以的话创建招募角色
void	EnterServerState::CheckBuildEployee(UINT64 Uid_Employee)
{
	int i = 0;
	for( ; i < m_BuildActor.m_nEmployNum && i < MAX_EMPLOY_NUM; ++i)
	{
		if( m_BuildActor.m_EmployBuildData[i]->m_pActorBasicData->uid == Uid_Employee){

			if( m_BuildActor.m_EmployBuildData[i]->IsOK() == true){
				//创建角色
				TSceneID SecenID = GameServer::GetInstance()->GetGameWorld()->GetMainSceneID();
				IThing * pThing = GameServer::GetInstance()->GetGameWorld()->CreateThing(enThing_Class_Actor,SecenID,(char*)m_BuildActor.m_EmployBuildData[i],SIZE_OF(SBuild_Actor()),THING_CREATE_FLAG_DB_DATA);
				if(pThing==0)
				{
					TRACE("<error> %s : %d 创建招募角色失败!",__FUNCTION__,__LINE__);
					return;
				}	

				////////////////////////////////////////////
				IActor * pActor =(IActor *)pThing;

				pActor->NoticClientCreatePrivateThing(pActor->GetUID());

				for(int k=enThingPart_Crt_Basic; k<enThingPart_Crt_Max;k++)
				{
					IThingPart* pThingPart = pActor->GetPart((enThingPart)k);
					if(pThingPart)
					{
						pThingPart->InitPrivateClient();
					}
				}
				/////////////////////////////////////////

				i++;
				if( i == m_BuildActor.m_nEmployNum || i == MAX_EMPLOY_NUM)
				{
					//最后一个雇佣人员
					SS_ActoreCreateContext ActoreCreateCnt;
					ActoreCreateCnt.m_uidActor = m_BuildActor.m_pActorBasicData->uid;
					IActor * pActor =  GameServer::GetInstance()->GetGameWorld()->FindActor(UID(ActoreCreateCnt.m_uidActor));
					if(pActor !=0)
					{
						UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);
						pActor->OnEvent(msgID,&ActoreCreateCnt,SIZE_OF(ActoreCreateCnt));
					}

					//转入游戏态
					m_pSession->GotoState(enGamePlayState,NULL);

					OBuffer4k  ob;
					HandleEnterGameRsp(enEnterGameRetCode_Ok,ob);

					m_BuildActor.Release();

					//全部创建完成，重新返回false,等下次创建
					m_bSuccessBuildUser = false;
				}

				return;
			}

		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩游戏态
GamePlayState::GamePlayState(Session * pSession)
{
	m_pSession = pSession;
}

GamePlayState::~GamePlayState()
{
}


void	GamePlayState::Enter(void *pContext)
{
}

void	GamePlayState::Leave(void * pContext)
{
	//最后存一次盘
	IActor * pActor = m_pSession->Actor();
	if(pActor)
	{
		//掉线，下线要做的一些事
		pActor->LeaveDoSomeThing();

		//pActor->SaveData();
	}
}

//数据到达,
void GamePlayState::OnNetDataRecv(OStreamBuffer & osb)
{	
	// 分发消息
	if(m_pSession->Actor() != NULL)
	{
		IBuffer ib(osb.Buffer(),osb.Size());

		AppPacketHeader Header = *(AppPacketHeader*)osb.Buffer();

		boost::timer timer;

		GameServer::GetInstance()->GetMessageDispatch()->DispatchMessage(m_pSession->Actor(),ib );

		double t = timer.elapsed();
		if(t>0.02)
		{
			TRACE("执行请求[MsgCategory=%d ,Cmd=%d]执行花时 %f 秒",Header.MsgCategory,Header.command,t);
		}
	}
}

void	GamePlayState::OnCloseConnect()
{

	//进入退出态
	m_pSession->GotoState(enLogoutState,0);
}

// nRetCode: 取值于 enDBRetCode
void GamePlayState::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
							OStreamBuffer & ReqOsb,UINT64 userdata)
{
}

enPlayerSessionState	GamePlayState::GetStateID()
{
	return enGamePlayState;
}

void  GamePlayState::LoadActor(const UID & UID_Actor)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//退出游戏态
LogoutState::LogoutState(Session * pSession)
{
	m_pSession = pSession;
}

LogoutState::~LogoutState()
{
}


void	LogoutState::Enter(void *pContext)
{
	//存盘一次
	IActor * pActor = m_pSession->Actor();
	if( pActor != NULL)
	{
		pActor->SaveData();

		//通知数据库用户退出了
		DB_UserExitGame_Req  Req;
		Req.m_ServerID = GameServer::GetInstance()->GetServerID();
		Req.m_UserID   = pActor->GetCrtProp(enCrtProp_ActorUserID);
		Req.m_GodStone = pActor->GetCrtProp(enCrtProp_ActorMoney);
		Req.m_Level	   = pActor->GetCrtProp(enCrtProp_Level);

		OBuffer1k DBOb;
		DBOb << Req;

		GameServer::GetInstance()->RequestDBByActor(pActor,enDBCmd_UserExitGame,DBOb.TakeOsb());

		//发送事件
		SS_ActorLogout ActorLogout ;
		ActorLogout.m_uidActor =  pActor->GetUID();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActorLogout);
		pActor->OnEvent(msgID,&ActorLogout,SIZE_OF(ActorLogout));

	}
	else
	{
		TRACE("<error> %s:%d pActor == NULL",__FUNCTION__,__LINE__);
	}

	GameServer::GetInstance()->OffLineSession(m_pSession->GetSessionID());
}

void	LogoutState::Leave(void * pContext)
{

}

//数据到达,
void LogoutState::OnNetDataRecv(OStreamBuffer & osb)
{
}

void	LogoutState::OnCloseConnect()
{
}

// nRetCode: 取值于 enDBRetCode
void LogoutState::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
						  OStreamBuffer & ReqOsb,UINT64 userdata)
{
}

enPlayerSessionState	LogoutState::GetStateID()
{
	return enLogoutState;
}

void	LogoutState::LoadActor(const UID & UID_Actor)
{
}
