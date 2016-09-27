
#include "WebInterface.h"
#include "ThingServer.h"
#include "WebCmd.h"
#include "IBasicService.h"
#include "ITalk.h"
#include "IActor.h"
#include "ISession.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "IMailPart.h"
#include "XDateTime.h"
#include "IKeywordFilter.h"
#include "ISystemMsg.h"
#include "IEventServer.h"
#include "IGoodsServer.h"
#include "IGoodsServer.h"

WebInterface::WebInterface()
{
}

WebInterface::~WebInterface()
{
}

bool WebInterface::Create()
{
	this->LoadDontTalk();

	this->LoadSealNo();

	this->LoadSysMsg();

	this->LoadRechargeForward();

	this->LoadFirstRechargeForward();
	
	//先订阅玩家创建的消息
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return false;
	}
	pEnventServer->AddListener(this, msgID, enEventSrcType_Actor,0,"WebInterface::Create");

	return true;
}

void WebInterface::Close()
{
	IEventServer * pEnventServer = g_pGameServer->GetEventServer();
	if( 0 == pEnventServer){
		return;
	}

	pEnventServer->RemoveAllListener(this);
}


//收到MSG_ROOT消息
void WebInterface::OnRecv(UINT8 nCmd, IBuffer & ib)
{
	typedef void (WebInterface::* FUNC_PROC)(UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enWebSrvCmd_Max] = 
	{
		&WebInterface::DontTalk,
		&WebInterface::SealNo,
		&WebInterface::GiveGoods,
		&WebInterface::ChangeKeyword,
		&WebInterface::AddResource,
		&WebInterface::BackSysMail,
		&WebInterface::SysMsg,
		NULL,
		NULL,
		&WebInterface::VersionUpdate,
		&WebInterface::ServiceInfo,
		&WebInterface::Pay,
		&WebInterface::ShopMall,
		&WebInterface::RechargeForward,
		&WebInterface::ReloadMonsterCnfg,
		&WebInterface::ChangeActivity,
		&WebInterface::ChangeMultipExp,
		&WebInterface::ReloadFirstRechargeCnfg,

	};

	if(nCmd >= ARRAY_SIZE(s_funcProc) || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(nCmd, ib);
}

// nRetCode: 取值于 enDBRetCode
void WebInterface::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
						   OStreamBuffer & ReqOsb,UINT64 userdata )
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_Get_DontTalk:
			{
				HandleDontTalk(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   case enDBCmd_Get_SealNo:
		   {
				HandleSealNo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_GiveResource:
		   {
			   HandleAddResource(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_AllDontTalk:
		   {
			   HandleLoadDontTalk(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_AllSealNo:
		   {
			   HandleLoadSealNo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_GiveGoods:
		   {
			   HandleGiveGoods(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_BackSysMail:
		   {
			   HandleBackSysMail(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_Version:
		   {
			   HandleVersionUpdate(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_ServiceData:
		   {
			   HandleServiceInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_SysMsg:
		   {
			   HandleSysMsg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_AllSysMsg:
		   {
			   HandleAllSysMsg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_AllPayData:
		   {
			   HandleGetAllPayData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_PayData:
		   {
			   HandleGetPayData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_OneGoodsShopCnfg:
		   {
			   HandleGetGoodsShopCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetRechargeForward:
		   {
			   HandleGetRechargeForward(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetRechargeForwardCnfg:
		   {
			   HandleGetRechargeForwardCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetTotalRecharge:
		   {
			   HandleGetTotalRecharge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetMultipExpInfo:
		   {
				HandleGetMultipExpInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_FirstRechargeCnfg:
		   {
				HandleGetFirstRechargeCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_Get_IsGetFirstRechargeForward:
		   {
				HandleIsGetFirstRecharge(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}
}

void	WebInterface::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ActoreCreate);

	if( msgID == EventData.m_MsgID){
		//玩家创建，检测是否有未充值数据

		SS_ActoreCreateContext * pActoreCreateCnt = (SS_ActoreCreateContext *)EventData.m_pContext;
		if( 0 == pActoreCreateCnt){
			return;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(pActoreCreateCnt->m_uidActor));
		if( 0 == pActor){
			return;
		}

		SDB_Get_AllPayData_Req DBReq;

		DBReq.m_UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);
		DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(DBReq.m_UserID, enDBCmd_Get_AllPayData, ob.TakeOsb(), this, pActor->GetUID().ToUint64());
	}
}

//禁言
void WebInterface::DontTalk(UINT8 nCmd, IBuffer & ib)
{
	SS_DontTalk_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_DontTalk_Req DBReq;
	DBReq.m_ID = Req.m_ID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Get_DontTalk, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleDontTalk(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_DontTalk       DBDontTalk;
	RspIb >> RspHeader >> OutParam >> DBDontTalk;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	g_pGameServer->GetRelationServer()->GetTalk()->DontTalk_Push(DBDontTalk);	
}

//封号
void WebInterface::SealNo(UINT8 nCmd, IBuffer & ib)
{
	SS_SealNo_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_SealNo_Req DBReq;
	DBReq.m_ID = Req.m_ID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Get_SealNo, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleSealNo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_SealNo       DBSealNo;
	RspIb >> RspHeader >> OutParam >> DBSealNo;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	g_pGameServer->SealNo_Push(DBSealNo);

	//判断是否在线，在线则赐下线
	IActor * pTarget = g_pGameServer->GetGameWorld()->GetUserByUserID(DBSealNo.m_UserID);
	if( 0 == pTarget){
		return;
	}

	ISession * pSession = pTarget->GetSession();
	if( 0 == pSession){
		return;
	}

	pSession->KickUser(enKickType_SealNo);
}

//发放物品
void WebInterface::GiveGoods(UINT8 nCmd, IBuffer & ib)
{
	SS_GoodsMgr_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_GiveGoods_Req DBReq;

	DBReq.m_GiveGoodsMailID = Req.m_MailID;
	DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_GiveGoodsMailID, enDBCmd_Get_GiveGoods, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleGiveGoods(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_GiveThingMail	DBMail;
	RspIb >> RspHeader >> DBMail;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(DBMail.m_RetCode != enDBRetCode_OK){
		return;
	}

	UINT8 OperatorType = 0;

	UINT8 nNum = RspIb.Remain() / sizeof(SDB_GiveGoods);

	if( nNum <= 0){
		TRACE("<error> %s : %d 行 发送物品的DB物品返回个数为0", __FUNCTION__, __LINE__);
		return;
	}

	//给玩家发增加或者扣除的物品
	std::vector<UINT16>	vectGoods;

	for( int i = 0; i < nNum; ++i)
	{
		SDB_GiveGoods DBGiveGoods;

		RspIb >> DBGiveGoods;

		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		OperatorType = DBGiveGoods.m_OperateType;

		vectGoods.push_back(DBGiveGoods.m_GoodsID);
		vectGoods.push_back(DBGiveGoods.m_GoodsNum);
	}


	switch(DBMail.m_TargetType)
	{
	case 0:
		{
			//所有玩家
			if( OperatorType == 0){
				this->GiveGoodsToAllUser(vectGoods, DBMail.m_ThemeText, DBMail.m_ContentText);
			}
		}
		break;
	case 1:
		{
			//指定玩家
			std::vector<TUserID> vectUserID;
			//获得指定玩家
			this->GetVectValue(DBMail.TargetList, vectUserID);

			for( int i = 0; i < vectUserID.size(); ++i)
			{
				if( OperatorType == 0){
					//给玩家增加物品
					this->GiveGoodsToUser(vectUserID[i], vectGoods, DBMail.m_ThemeText, DBMail.m_ContentText);
				}else{
					//扣除玩家指定物品
					IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(vectUserID[i]);
					if( 0 != pActor){
						//在线扣除物品处理
						this->OnLine_RemoveGoods(pActor, vectGoods, DBMail.m_ThemeText, DBMail.m_ContentText);
					}else{
						//离线扣除物品处理
						this->OffLine_RemoveGoods(vectUserID[i], vectGoods, DBMail.m_ThemeText, DBMail.m_ContentText);
					}
				}
			}
		}
		break;
	default:
		TRACE("<error> %s : %d 行 发放物品的目标玩家类型错误！！玩家类型 = %d", __FUNCTION__, __LINE__, DBMail.m_TargetType);
		break;
	}
}

//给玩家发物品
void WebInterface::GiveGoodsToUser(TUserID UserID, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText)
{
	SDB_Insert_SysMailByUserID Req;

	Req.UserID = UserID;
	strncpy(Req.ThemeText, ThemeText, sizeof(Req.ThemeText));
	strncpy(Req.ContentText, ContentText, sizeof(Req.ContentText));
	Req.Time = CURRENT_TIME();

	SDBGoodsData * pDBGoodsData = (SDBGoodsData *)&Req.GoodsData;

	std::vector<IGoods *> vectMailGoods;

	for( int k = 0; k + 1 < vectGoods.size(); k += 2)
	{
		std::vector<IGoods *> TmpVectGoods = g_pGameServer->GetGameWorld()->CreateGoods(vectGoods[k], vectGoods[k + 1], true);

		for( int n = 0; n < TmpVectGoods.size(); ++n)
		{
			if( 0 == TmpVectGoods[n]){
				continue;
			}
			vectMailGoods.push_back(TmpVectGoods[n]);
		}
	}

	for( int i = 0; i < vectMailGoods.size() && i < MAX_MAIL_GOODS_NUM; ++i)
	{
		IGoods * pGoods = vectMailGoods[i];
		if( 0 == pGoods){
			continue;
		}

		SDB_Save_GoodsReq GoodsDB;
		int nLen = sizeof(GoodsDB);

		pGoods->OnGetDBContext(&GoodsDB, nLen);

		pDBGoodsData->m_uidGoods = UID(GoodsDB.m_uidGoods);
		pDBGoodsData->m_GoodsID  = GoodsDB.m_GoodsID;
		pDBGoodsData->m_CreateTime = GoodsDB.m_CreateTime;
		pDBGoodsData->m_Number   = GoodsDB.m_Number;
		pDBGoodsData->m_Binded   = GoodsDB.m_Binded;
		memcpy(&pDBGoodsData->m_TalismanProp, &GoodsDB.GoodsData, sizeof(pDBGoodsData->m_TalismanProp));

		++pDBGoodsData;

		g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
	}

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Insert_SysMailByUserID, ob.TakeOsb(), 0, 0);

	//如果玩家在线，则增加玩家邮件数量
	IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(UserID);
	if( 0 == pActor){
		return;
	}

	IMailPart * pMailPart = pActor->GetMailPart();
	if( 0 != pMailPart){
		pMailPart->AddMailNum(1);
	}
}

//扣除玩家物品
void WebInterface::OnLine_RemoveGoods(IActor * pActor, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	INT32 RealNum = 0;	//真实扣除的物品数量

	for(int k = 0; k + 1 < vectGoods.size(); k += 2)
	{
		INT32 nGoodsNum = pPacketPart->HaveGoodsNum(vectGoods[k]);

		if( nGoodsNum < vectGoods[k + 1]){

			RealNum = nGoodsNum;

			//先扣除这些
			pPacketPart->DestroyGoods(vectGoods[k], RealNum);

			//从装备栏中扣除
			IEquipPart * pEquipPart = pActor->GetEquipPart();
			if( 0 == pEquipPart){
				return;
			}

			for( int i = 0; i < enEquipPos_Max; ++i)
			{
				IEquipment * pEquipment = pEquipPart->GetEquipByPos(i);
				if( 0 == pEquipment){
					return;
				}

				if( pEquipment->GetGoodsID() == vectGoods[k]){
					//删除
					pEquipPart->RemoveEquip(pEquipment->GetUID(), true);
					
					++RealNum;
					if( RealNum >= vectGoods[k + 1]){
						break;
					}
				}
			}

		}else{
			RealNum = vectGoods[k + 1];

			pPacketPart->DestroyGoods(vectGoods[k], RealNum);
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_SystemDes,vectGoods[k],UID(),RealNum,"后台扣除玩家物品");
	}


	//写系统邮件
	SWriteSystemData SysMailData;

	SysMailData.m_DestUID = pActor->GetUID();
	strncpy(SysMailData.m_szThemeText, ThemeText, sizeof(SysMailData.m_szThemeText));
	strncpy(SysMailData.m_szContentText, ContentText, sizeof(SysMailData.m_szContentText));

	g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData);
}

//离线玩家扣除玩家物品处理
void WebInterface::OffLine_RemoveGoods(TUserID UserID, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText)
{
	SDB_OffLine_RemoveGoods_Req Req;
		
	Req.m_UserID   = UserID;

	OBuffer1k ob;

	for( int i = 0; i + 1 < vectGoods.size(); i += 2)
	{

		Req.m_GoodsID  = vectGoods[i];
		Req.m_GoodsNum = vectGoods[i + 1];

		ob.Reset();
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_OffLine_RemoveGoods, ob.TakeOsb(), 0, 0);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(UserID,enGameGoodsType_Use,enGameGoodsChanel_SystemDes,Req.m_GoodsID,UID(),Req.m_GoodsNum,"后台离线扣除玩家物品");
	}

	//写系统邮件说明
	this->WriteUserSysMail(UserID, ThemeText, ContentText);
}

//字符串中得到数值参数
void WebInterface::GetVectValue(char * pszValue, std::vector<UINT32> & vectValue)
{
	char * pszValue2 = pszValue;

	for( ; *pszValue; )
	{
		if( *pszValue == ','){
			*pszValue = '\0';

			vectValue.push_back(atoi(pszValue2));

			++pszValue;

			pszValue2 = pszValue;
		}else{
			++pszValue;
		}	
	}

	//验证最后一项
	if( atoi(pszValue2) == 0){
		return;
	}

	vectValue.push_back(atoi(pszValue2));
}

//给所有玩家发物品
void WebInterface::GiveGoodsToAllUser(std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText)
{
	SWriteSystemData WriteSysMail;

	WriteSysMail.m_DestUID = UID();
	strncpy(WriteSysMail.m_szThemeText, ThemeText, sizeof(WriteSysMail.m_szThemeText));
	strncpy(WriteSysMail.m_szContentText, ContentText, sizeof(WriteSysMail.m_szContentText));

	std::vector<IGoods *> vectMailGoods;

	for( int k = 0; k + 1 < vectGoods.size(); k += 2)
	{
		std::vector<IGoods *> TmpVectGoods = g_pGameServer->GetGameWorld()->CreateGoods(vectGoods[k], vectGoods[k + 1], true);

		for( int n = 0; n < TmpVectGoods.size(); ++n)
		{
			vectMailGoods.push_back(TmpVectGoods[n]);
		}
	}

	for( int  i = 0; i < vectMailGoods.size() && i < MAX_MAIL_GOODS_NUM; ++i)
	{
		IGoods * pGoods = vectMailGoods[i];
		if( 0 == pGoods){
			continue;
		}

		SDB_Save_GoodsReq GoodsDB;
		int nLen = sizeof(GoodsDB);

		pGoods->OnGetDBContext(&GoodsDB, nLen);

		WriteSysMail.m_SDBGoodsData[i].m_uidGoods = UID(GoodsDB.m_uidGoods);
		WriteSysMail.m_SDBGoodsData[i].m_GoodsID  = GoodsDB.m_GoodsID;
		WriteSysMail.m_SDBGoodsData[i].m_CreateTime = GoodsDB.m_CreateTime;
		WriteSysMail.m_SDBGoodsData[i].m_Number   = GoodsDB.m_Number;
		WriteSysMail.m_SDBGoodsData[i].m_Binded   = GoodsDB.m_Binded;

		memcpy(&WriteSysMail.m_SDBGoodsData[i].m_TalismanProp, &GoodsDB.GoodsData, sizeof(WriteSysMail.m_SDBGoodsData[i].m_TalismanProp));

		g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
	}

	g_pGameServer->GetGameWorld()->WriteAllUserSysMail(WriteSysMail);
}

//增加资源
void WebInterface::AddResource(UINT8 nCmd, IBuffer & ib)
{
	SS_ResourceMgr_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_GiveResource_Req DBReq;

	DBReq.m_GiveResourceMailID = Req.m_MailID;
	DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_GiveResourceMailID, enDBCmd_Get_GiveResource, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleAddResource(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_GiveThingMail DBMail;
	SDB_GiveResource	  DBResource;
	RspIb >> RspHeader >> DBMail >> DBResource;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(DBMail.m_RetCode != enDBRetCode_OK){
		return;
	}

	switch(DBMail.m_TargetType)
	{
	case 0:
		{
			//发给所有玩家
			if( DBResource.m_OperateType == 0){
				//增加资源
				this->AddAllUserResource(DBResource.m_MoneyNum, DBResource.m_StoneNum, DBResource.m_TicketNum,DBResource.m_PolyNimbusNum, DBMail.m_ThemeText, DBMail.m_ContentText);
			}
		}
		break;
	case 1:
		{
			//针对指定玩家
			std::vector<TUserID> vectUserID;
			//获得指定玩家
			this->GetVectValue(DBMail.TargetList, vectUserID);

			for( int i = 0; i < vectUserID.size(); ++i)
			{
				if( DBResource.m_OperateType == 0){
					//增加资源
					this->AddResource(vectUserID[i], DBResource.m_MoneyNum, DBResource.m_StoneNum, DBResource.m_TicketNum, DBResource.m_PolyNimbusNum, DBMail.m_ThemeText, DBMail.m_ContentText);
				}else{
					IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(vectUserID[i]);
					if( 0 == pActor){
						//离线扣除资源
						this->OffLine_DescResource(vectUserID[i], DBResource.m_MoneyNum, DBResource.m_StoneNum, DBResource.m_TicketNum, DBResource.m_PolyNimbusNum, DBMail.m_ThemeText, DBMail.m_ContentText);
					}else{
						//在线扣除资源
						this->OnLine_DesResource(pActor, DBResource.m_MoneyNum, DBResource.m_StoneNum, DBResource.m_TicketNum, DBResource.m_PolyNimbusNum, DBMail.m_ThemeText, DBMail.m_ContentText);
					}
				}
			}
		}
		break;
	default:
		TRACE("<error> %s : %d 行 发放资源的目标玩家类型错误！！玩家类型 = %d", __FUNCTION__, __LINE__, DBMail.m_TargetType);
		break;
	}
}

//在线扣除资源
void WebInterface::OnLine_DesResource(IActor * pActor, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText)
{
	INT32 CurMoney  = pActor->GetCrtProp(enCrtProp_ActorMoney);
	INT32 CurStone  = pActor->GetCrtProp(enCrtProp_ActorStone);
	INT32 CurTicket = pActor->GetCrtProp(enCrtProp_ActorTicket);
	INT32 CurPolyNimbus = pActor->GetCrtProp(enCrtProp_ActorPolyNimbus);	//聚灵气

	nMoney = CurMoney - nMoney > 0 ? nMoney : CurMoney;
	nStone = CurStone - nStone > 0 ? nStone : CurStone;
	nTicket = CurTicket - nTicket > 0 ? nTicket : CurTicket;
	nPolyNimbus = CurPolyNimbus - nPolyNimbus > 0 ? nPolyNimbus : CurPolyNimbus;

	pActor->AddCrtPropNum(enCrtProp_ActorMoney, -nMoney);

	g_pGameServer->GetGameWorld()->Save_GodStoneLog(pActor->GetCrtProp(enCrtProp_ActorUserID), nMoney, pActor->GetCrtProp(enCrtProp_ActorMoney), "后台扣除玩家仙石",enGodStoneChanel_Manage);

	pActor->AddCrtPropNum(enCrtProp_ActorStone, -nStone);
	pActor->AddCrtPropNum(enCrtProp_ActorTicket, -nTicket);
	pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, -nPolyNimbus);

	//发系统邮件说明
	SWriteSystemData SysMailData;

	SysMailData.m_DestUID = pActor->GetUID();
	strncpy(SysMailData.m_szThemeText, ThemeText, sizeof(SysMailData.m_szThemeText));
	strncpy(SysMailData.m_szContentText, ContentText, sizeof(SysMailData.m_szContentText));

	g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData);
}

//离线扣除资源
void WebInterface::OffLine_DescResource(TUserID UserID, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText)
{
	SDB_OffLine_DescRes_Req DBReq;

	DBReq.m_UserID = UserID;
	DBReq.m_Money  = nMoney;
	DBReq.m_Stone  = nStone;
	DBReq.m_Ticket = nTicket;
	DBReq.m_PolyNimbus = nPolyNimbus;	//聚灵气

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_OffLine_DescRes, ob.TakeOsb(), 0, 0);

	g_pGameServer->GetGameWorld()->Save_GodStoneLog(UserID, nMoney, 0, "后台离线扣除玩家仙石,目前仙石数未知",enGodStoneChanel_Manage);

	//发系统邮件说明
	this->WriteUserSysMail(UserID, ThemeText, ContentText);
}

//增加资源
void WebInterface::AddResource(TUserID UserID, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText)
{
	//发系统邮件
	SDB_Insert_SysMailByUserID ReqMail;

	ReqMail.UserID = UserID;
	ReqMail.Stone  = nStone;
	ReqMail.Money  = nMoney;
	ReqMail.Ticket = nTicket;
	ReqMail.PolyNimbus = nPolyNimbus; //聚灵气
	strncpy(ReqMail.ThemeText, ThemeText, sizeof(ReqMail.ThemeText));
	strncpy(ReqMail.ContentText, ContentText, sizeof(ReqMail.ContentText));

	ReqMail.Time = CURRENT_TIME();

	OBuffer1k ob;
	ob << ReqMail;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Insert_SysMailByUserID, ob.TakeOsb(), 0, 0);

	//如果玩家在线，则增加玩家邮件数量
	IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(UserID);
	if( 0 == pActor){
		return;
	}

	IMailPart * pMailPart = pActor->GetMailPart();
	if( 0 != pMailPart){
		pMailPart->AddMailNum(1);
	}
}

//给所有玩家增加资源
void WebInterface::AddAllUserResource(INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText)
{
	SWriteSystemData WriteSysMail;

	WriteSysMail.m_DestUID = UID();
	WriteSysMail.m_Money   = nMoney;
	WriteSysMail.m_Stone   = nStone;
	WriteSysMail.m_Ticket  = nTicket;
	WriteSysMail.m_PolyNimbus  = nPolyNimbus;	//聚灵气
	strncpy(WriteSysMail.m_szThemeText, ThemeText, sizeof(WriteSysMail.m_szThemeText));
	strncpy(WriteSysMail.m_szContentText, ContentText, sizeof(WriteSysMail.m_szContentText));

	g_pGameServer->GetGameWorld()->WriteAllUserSysMail(WriteSysMail);
}

//后台系统邮件
void WebInterface::BackSysMail(UINT8 nCmd, IBuffer & ib)
{
	SS_MailMgr_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_BackSysMail_Req DBReq;

	DBReq.m_ID = Req.m_MailID;
	DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Get_BackSysMail, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleBackSysMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_BackSysMail       BackSysMail;
	RspIb >> RspHeader >> OutParam >> BackSysMail;

	if(RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
	}

	if( OutParam.retCode != enDBRetCode_OK){
		return;
	}

	switch(BackSysMail.m_TargetType)
	{
	case 0:
		{
			//写给所有玩家
			this->WriteAllUserSysMail(BackSysMail.m_ThemeText, BackSysMail.m_ContentText);
		}
		break;
	case 1:
		{
			//写给指定玩家
			std::vector<TUserID> vectUserID;
			//获得指定玩家
			this->GetVectValue(BackSysMail.m_TargetList, vectUserID);

			for( int i = 0; i < vectUserID.size(); ++i)
			{
				this->WriteUserSysMail(vectUserID[i], BackSysMail.m_ThemeText, BackSysMail.m_ContentText, true);
			}
		}
		break;
	case 3:
		{
			//写给所有帮派成员
		}
		break;
	}
}

//给所有玩家写文本系统邮件
void WebInterface::WriteAllUserSysMail(const char * pszThemeText, const char * pszContentText)
{
	SWriteSystemData WriteSysMail;

	WriteSysMail.m_DestUID = UID();
	strncpy(WriteSysMail.m_szThemeText, pszThemeText, sizeof(WriteSysMail.m_szThemeText));
	strncpy(WriteSysMail.m_szContentText, pszContentText, sizeof(WriteSysMail.m_szContentText));

	g_pGameServer->GetGameWorld()->WriteAllUserSysMail(WriteSysMail);	
}

//用UserID给玩家写文本系统邮件
void WebInterface::WriteUserSysMail(TUserID UserID, const char * pszThemeText, const char * pszContentText, bool bAddMailNum)
{
	//发系统邮件
	SDB_Insert_SysMailByUserID ReqMail;

	ReqMail.UserID = UserID;
	strncpy(ReqMail.ThemeText, pszThemeText, sizeof(ReqMail.ThemeText));
	strncpy(ReqMail.ContentText, pszContentText, sizeof(ReqMail.ContentText));

	ReqMail.Time = CURRENT_TIME();

	OBuffer1k ob;
	ob << ReqMail;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Insert_SysMailByUserID, ob.TakeOsb(), 0, 0);

	if( bAddMailNum){
		//检测玩家是否在线，在线则增加邮件数量
		IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(UserID);
		if( 0 == pActor){
			return;
		}

		IMailPart * pMailPart = pActor->GetMailPart();
		if( 0 == pMailPart){
			return;
		}

		pMailPart->AddMailNum();
	}
}


//得到所有禁言
void WebInterface::LoadDontTalk()
{
	SDB_AllDontTalk_Req Req;
	Req.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(Req.m_ServerID , enDBCmd_Get_AllDontTalk, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleLoadDontTalk(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	int nNum = RspIb.Remain() / sizeof(SDB_DontTalk);

	for( int i = 0; i < nNum; ++i)
	{
		SDB_DontTalk DBDontTalk;
		RspIb >> DBDontTalk;

		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		g_pGameServer->GetRelationServer()->GetTalk()->DontTalk_Push(DBDontTalk);	
	}
}

//得到所有封号
void WebInterface::LoadSealNo()
{
	SDB_GetAllSealNo_Req Req;
	Req.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(Req.m_ServerID , enDBCmd_Get_AllSealNo, ob.TakeOsb(), this, 0);	
}

void WebInterface::HandleLoadSealNo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	int nNum = RspIb.Remain() / sizeof(SDB_SealNo);

	for( int i = 0; i < nNum; ++i)
	{
		SDB_SealNo DBSealNo;

		RspIb >> DBSealNo;

		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		g_pGameServer->SealNo_Push(DBSealNo);
	}
}

//版本更新
void WebInterface::VersionUpdate(UINT8 nCmd, IBuffer & ib)
{
	SS_Version_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_Get_Version_Req DBReq;

	DBReq.m_ID = Req.m_ID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(Req.m_ID, enDBCmd_Get_Version, ob.TakeOsb(), this, 0);	
}

void WebInterface::HandleVersionUpdate(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_Version DBVersion;
	RspIb >> RspHeader >> OutParam >> DBVersion;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	std::string strLastVersion = (char *)DBVersion.m_LastVersion;
	std::string strMinVersion = (char *)DBVersion.m_MinVersion;
	std::string strInstallPackageUrl = (char *)DBVersion.m_InstallPackageUrl;

	g_pGameServer->SetVersionInfo(strLastVersion, strMinVersion, strInstallPackageUrl);
}

//客服信息
void WebInterface::ServiceInfo(UINT8 nCmd, IBuffer & ib)
{
	SS_ServiceInfo_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_Get_ServiceData_Req DBReq;

	DBReq.m_ID = Req.m_ID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(Req.m_ID, enDBCmd_Get_ServiceData, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleServiceInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ServiceData DBService;
	RspIb >> RspHeader >> OutParam >> DBService;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	std::string strServiceTel = (char *)DBService.m_szServiceTel;
	std::string strServiceEmail = (char *)DBService.m_szServiceEmail;
	std::string strServiceQQ = (char *)DBService.m_szServiceQQ;

	g_pGameServer->SetServiceInfo(strServiceTel, strServiceEmail, strServiceQQ);
}

//更改关键字
void WebInterface::ChangeKeyword(UINT8 nCmd, IBuffer & ib)
{
	g_pGameServer->ReGetKeywordFilter();
}

//系统消息
void WebInterface::SysMsg(UINT8 nCmd, IBuffer & ib)
{
	SS_MessageMgr_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_Get_SysMsg_Req DBReq;

	DBReq.m_SysMsgID = Req.m_MsgID;


	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_SysMsgID, enDBCmd_Get_SysMsg, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleSysMsg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_SysMsgInfo		DBSysMsg;
	RspIb >> RspHeader >> OutParam >> DBSysMsg;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	if( DBSysMsg.m_MsgType == 0){
		//聊天框
		g_pGameServer->GetRelationServer()->GetTalk()->TalkSysMsg_Push(DBSysMsg);

	}else if( DBSysMsg.m_MsgType == 1){
		//主界面
		g_pGameServer->GetRelationServer()->GetSystemMsg()->HouTaiMsg_Push(DBSysMsg);
	}
}

//得到所有系统消息
void WebInterface::LoadSysMsg()
{
	SDB_Get_AllSysMsg_Req DBReq;

	DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;


	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ServerID, enDBCmd_Get_AllSysMsg, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleAllSysMsg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	INT32 nNum = RspIb.Remain() / sizeof(SDB_SysMsgInfo);

	for( int i = 0; i < nNum; ++i)
	{
		SDB_SysMsgInfo	DBSysMsg;
		RspIb >> DBSysMsg;

		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		if( DBSysMsg.m_MsgType == 0){
			//聊天框
			g_pGameServer->GetRelationServer()->GetTalk()->TalkSysMsg_Push(DBSysMsg);

		}else if( DBSysMsg.m_MsgType == 1){
			//主界面
			g_pGameServer->GetRelationServer()->GetSystemMsg()->HouTaiMsg_Push(DBSysMsg);
		}
	}
}

//支付
void WebInterface::Pay(UINT8 nCmd, IBuffer & ib)
{
	SS_Pay_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	if( Req.m_bOk){
		//成功
		SS_Pay_Ok Pay_OK;
		ib >> Pay_OK;

		if( ib.Error()){
			TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return;
		}

		SDB_Get_PayData_Req DBReq;

		DBReq.m_ID = Pay_OK.m_ID;
		DBReq.m_ServerID = g_pGameServer->GetConfigParam().m_ServerID;

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Get_PayData, ob.TakeOsb(), this, 0);
	}else{
		//失败
		SS_Pay_Error Pay_Error;
		ib >> Pay_Error;

		if( ib.Error()){
			TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
			return;
		}

		IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(Pay_Error.m_UserID);
		if( 0 == pActor){
			return;
		}

		SC_Recharge_Rsp Rsp;

		Rsp.m_Result = enShopMallRetCode_ErrorRecharge;

		SC_Recharge_Error Error;

		strncpy(Error.m_szErrorInfo, Pay_Error.m_szErrorInfo, sizeof(Error.m_szErrorInfo));

		OBuffer1k ob;
		ob << ShopMallHeader(enShopMallCmd_Recharge, sizeof(Rsp) + sizeof(Error)) << Rsp << Error;
		pActor->SendData(ob.TakeOsb());

		TRACE("玩家 <%s> 充值失败,因为：%s！！！！", pActor->GetName(), Pay_Error.m_szErrorInfo);
	}

}

void WebInterface::HandleGetPayData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_PayData	DBPay;
	RspIb >> RspHeader >> OutParam >> DBPay;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->GetUserByUserID(DBPay.m_UserID);
	if( 0 == pActor){
		return;
	}

	//支付充值
	if( !this->PayMoneyToUser(pActor, DBPay)){
		TRACE("<error> %s : %d 行 充值失败！！", __FUNCTION__, __LINE__);
		return;
	}
}

void WebInterface::HandleGetAllPayData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	INT32 nNum = RspIb.Remain() / sizeof(SDB_Get_PayData);

	for( int i = 0; i < nNum; ++i)
	{
		SDB_Get_PayData DBReq;

		RspIb >> DBReq;

		if(RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			continue;
		}

		//支付充值
		this->PayMoneyToUser(pActor, DBReq);
	}
}

//支付仙石给玩家
bool WebInterface::PayMoneyToUser(IActor * pActor, const SDB_Get_PayData & PayData)
{
	if( pActor->GetCrtProp(enCrtProp_ActorUserID) != PayData.m_UserID){
		TRACE("<error> %s : %d 行 支付充值时，玩家不对！！玩家ID = %d,充值玩家ID = %d", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_ActorUserID), PayData.m_UserID);
		return false;
	}

	//获得的仙石数量
	INT32 Money = PayData.m_GodStone;

	if( Money < 0){
		TRACE("<error> %s : %d 行 玩家充值的仙石数为负数%d！！充值金额=%d,玩家ID=%d", __FUNCTION__, __LINE__, Money, PayData.m_Money, PayData.m_UserID);
		return false;
	}

	if( !pActor->AddCrtPropNum(enCrtProp_ActorMoney, Money)){
		TRACE("<error> %s : %d 行 玩家充值，增加玩家仙石数错误！！，玩家已有仙石=%d，充值仙石=%d，玩家ID=%d！！", __FUNCTION__, __LINE__, pActor->GetCrtProp(enCrtProp_ActorMoney), Money, PayData.m_UserID);
		return false;
	}

	pActor->AddCrtPropNum(enCrtProp_Recharge, Money);

	//看看是否可升级VIP等级
	UINT8 OldVipLv = pActor->GetCrtProp(enCrtProp_VipLevel);

	UINT8 NewVipLv = g_pGameServer->GetConfigServer()->GetCanVipLevel(pActor->GetCrtProp(enCrtProp_Recharge));

	if ( NewVipLv > OldVipLv){
		
		pActor->AddCrtPropNum(enCrtProp_VipLevel, NewVipLv - OldVipLv);
	}

	pActor->SaveActorProp();

	g_pGameServer->GetGameWorld()->Save_GodStoneLog(pActor->GetCrtProp(enCrtProp_ActorUserID), Money, pActor->GetCrtProp(enCrtProp_ActorMoney), "玩家充值,获得仙石",enGodStoneChanel_Recharge,enGodStoneType_Get);

	TRACE("玩家 <%s> 成功充值%d元，增加仙石%d！！！！", pActor->GetName(), PayData.m_Money, PayData.m_GodStone);

	OBuffer1k ob;

	//充值返利
	if ( this->IsInRechargeBackTime() )
	{
		//单次充值的充值返利
		const SRechargeBack * pOneBack = this->GetOneRecharge(PayData.m_Money / 10);

		if ( 0 != pOneBack )
		{
			this->GetRechargeForword(pActor, pOneBack, PayData.m_GodStone);
		}
	}

	//支付数据移到支付历史表
	SDB_RemoveToPayLog_Req DBReq;

	DBReq.m_ID = PayData.m_ID;

	ob.Reset();
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(PayData.m_Money, enDBCmd_RemoveToPayLog, ob.TakeOsb(), 0, 0);

	//检测是否可以获得某时间起的首次充值奖励
	SDB_Get_IsGetFirstRechargeForward DBFirstRecharge;

	DBFirstRecharge.m_UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);
	DBFirstRecharge.m_uidUser = pActor->GetUID().ToUint64();
	DBFirstRecharge.m_level  = pActor->GetCrtProp(enCrtProp_Level);

	ob.Reset();
	ob << DBFirstRecharge;
	g_pGameServer->GetDBProxyClient()->Request(PayData.m_Money, enDBCmd_Get_IsGetFirstRechargeForward, ob.TakeOsb(), this, Money);

	//if ( PayData.m_Money >= m_FirstRechargeCnfg.m_MinRecharge && (m_FirstRechargeCnfg.m_RateGodStone > 0 || m_FirstRechargeCnfg.m_RateTicket > 0) )
	//{
	//	if ( pActor->GetCrtProp(enCrtProp_Recharge) <= 0 )
	//	{
	//		//没充过的肯定得给
	//		this->GetFirstRechargeForward(pActor->GetUID(),Money);
	//	}
	//	else
	//	{
	//		//充过值的可能开始时间改变，检测下是否可以奖励
	//		SDB_Get_IsGetFirstRechargeForward DBFirstRecharge;

	//		DBFirstRecharge.m_UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);
	//		DBFirstRecharge.m_uidUser = pActor->GetUID().ToUint64();

	//		ob.Reset();
	//		ob << DBFirstRecharge;
	//		g_pGameServer->GetDBProxyClient()->Request(PayData.m_Money, enDBCmd_Get_IsGetFirstRechargeForward, ob.TakeOsb(), this, Money);
	//	}
	//}

	////插入首次充值表，如果表里还没有他的记录的话

	//发送客户端消息
	SC_Recharge_Rsp Rsp;

	Rsp.m_Result = enShopMallRetCode_OK;

	SC_Recharge_Ok OkRsp;

	OkRsp.m_Amount = PayData.m_Money;
	OkRsp.m_GodStone = PayData.m_GodStone;

	ob.Reset();
	ob << ShopMallHeader(enShopMallCmd_Recharge, sizeof(Rsp) + sizeof(OkRsp)) << Rsp << OkRsp;
	pActor->SendData(ob.TakeOsb());

	//发出事件
	SS_PayMoney PayEvent;

	PayEvent.m_nMoney = PayData.m_Money;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_PayMoney);
	pActor->OnEvent(msgID,&PayEvent,sizeof(PayEvent));

	SS_UpVipLevel VipLevel;

	VipLevel.m_uidUser  = pActor->GetUID().ToUint64();
	VipLevel.m_bChange  = (NewVipLv > OldVipLv);
	VipLevel.m_vipLevel = pActor->GetCrtProp(enCrtProp_VipLevel);

	msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Up_VipLevel);
	pActor->OnEvent(msgID,&PayEvent,sizeof(VipLevel));

	return true;
}

//商城
void WebInterface::ShopMall(UINT8 nCmd, IBuffer & ib)
{
	SS_ShopMall Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	SDB_Get_OneGoodsShopCnfg_Req DBReq;

	DBReq.m_ID = Req.m_ID;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Get_OneGoodsShopCnfg, ob.TakeOsb(), this, 0);
}

//充值返利修改
void WebInterface::RechargeForward(UINT8 nCmd, IBuffer & ib)
{
	this->LoadRechargeForward();
}

void  WebInterface::HandleGetGoodsShopCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ShopMallCnfg  GoodsShopCnfg;
	RspIb >> RspHeader >> OutParam >> GoodsShopCnfg;

	if ( RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK){

		return;
	}

	g_pGameServer->GetGoodsServer()->Push_ShopMallCnfg(GoodsShopCnfg);
}

//得到充值返利信息
void WebInterface::LoadRechargeForward()
{
	MEM_ZERO(&m_RechargeBackCnfg);
	m_vecOneRechare.clear();
	m_vecTotalRechare.clear();

	SDB_GetRechargeForwardCnfg Req;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(10, enDBCmd_GetRechargeForwardCnfg, ob.TakeOsb(), this, 0);
}

void WebInterface::HandleGetRechargeForward(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		return;
	}

	INT32 num = RspIb.Remain() / sizeof(SDB_RechareForward);

	for ( int i = 0; i < num; ++i )
	{
		SDB_RechareForward Info;

		RspIb >> Info;

		if ( RspIb.Error())
		{
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		SRechargeBack Data;

		Data.m_CalType	 = Info.m_CalType;
		Data.m_ForwardType = Info.m_ForwardType;
		Data.m_GodStone	 = Info.m_GodStone;
		Data.m_MoneyNum	 = Info.m_MoneyNum;
		Data.m_Stone	 = Info.m_Stone;
		Data.m_PolyNimbus= Info.m_PolyNimbus;
		Data.m_Ticket	 = Info.m_Ticket;

		this->GetVectValue((char *)Info.m_Goods, Data.m_vecGoods);

		if ( Data.m_CalType == (UINT8)enCalType_One )
		{
			m_vecOneRechare.push_back(Data);
		}
		else if ( Data.m_CalType == (UINT8)enCalType_Total )
		{
			m_vecTotalRechare.push_back(Data);
		}
	}
}

const SRechargeBack * WebInterface::GetOneRecharge(INT32 OneRechargeNum)
{
	int MoneyNum = 0;
	int index	 = -1;

	for ( int i = 0; i < m_vecOneRechare.size(); ++i )
	{
		const SRechargeBack & Info = m_vecOneRechare[i];

		if ( OneRechargeNum >= Info.m_MoneyNum && MoneyNum < Info.m_MoneyNum )
		{
			MoneyNum = Info.m_MoneyNum;
			index = i;
		}
	}

	if ( index >= 0 && index < m_vecOneRechare.size() )
		return &m_vecOneRechare[index];

	return 0;
}

const SRechargeBack * WebInterface::GetTotalRecharge(INT32 TotalRechargeNum)
{
	int MoneyNum = 0;
	int index	 = -1;

	for ( int i = 0; i < m_vecTotalRechare.size(); ++i )
	{
		const SRechargeBack & Info = m_vecTotalRechare[i];

		if ( TotalRechargeNum >= Info.m_MoneyNum && MoneyNum < Info.m_MoneyNum )
		{
			MoneyNum = Info.m_MoneyNum;
			index = i;
		}		
	}

	if ( index >= 0 && index < m_vecTotalRechare.size() )
		return &m_vecTotalRechare[index];

	return 0;
}

void WebInterface::GetRechargeForword(IActor * pActor, const SRechargeBack * pBack, INT32 RechargeNum, INT32 TotalRechargeNum)
{
	if ( 0 == pActor || 0 == pBack )
		return;

	INT32 AddGodStone = 0;

	INT32 AddTicket = 0;

	INT32 AddStone = 0;

	INT32 AddPolyNimbus = 0;

	if ( pBack->m_ForwardType == enForwardType_Fix )
	{
		AddGodStone = pBack->m_GodStone;
		AddTicket   = pBack->m_Ticket;
		AddStone	= pBack->m_Stone;
		AddPolyNimbus = pBack->m_PolyNimbus;
	}
	else if ( pBack->m_ForwardType == enForwardType_Scale )
	{
		if ( pBack->m_CalType == enCalType_One )
		{
			AddGodStone = RechargeNum * pBack->m_GodStone / 100;
			AddTicket = RechargeNum * pBack->m_Ticket / 100;
			AddStone = RechargeNum * pBack->m_Stone / 100;
			AddPolyNimbus = RechargeNum * pBack->m_PolyNimbus / 100;
		}
		else if ( pBack->m_CalType == enCalType_Total )
		{
			AddGodStone = TotalRechargeNum * pBack->m_GodStone / 100;
			AddTicket = TotalRechargeNum * pBack->m_Ticket / 100;
			AddStone = TotalRechargeNum * pBack->m_Stone / 100;
			AddPolyNimbus = TotalRechargeNum * pBack->m_PolyNimbus / 100;
		}
	}
	

	SWriteSystemData MailData;

	MailData.m_DestUID = pActor->GetUID();
	MailData.m_Money = AddGodStone;
	MailData.m_Ticket = AddTicket;
	MailData.m_Stone = AddStone;
	MailData.m_PolyNimbus = AddPolyNimbus;

	strncpy(MailData.m_szThemeText, m_RechargeBackCnfg.m_szMailTheme, sizeof(MailData.m_szThemeText));
	strncpy(MailData.m_szContentText, m_RechargeBackCnfg.m_szMailContent, sizeof(MailData.m_szContentText));

	//物品奖励
	std::vector<IGoods *> vecGoods;

	for ( int i = 0; i + 1 < (pBack->m_vecGoods).size(); i += 2)
	{
		SCreateGoodsContext GoodsCnt;

		GoodsCnt.m_Binded = true;
		GoodsCnt.m_GoodsID = (pBack->m_vecGoods)[i];
		GoodsCnt.m_Number = (pBack->m_vecGoods)[i + 1];

		IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsCnt);

		if ( 0 != pGoods )
		{
			vecGoods.push_back(pGoods);
		}
	}

	g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);
}

void WebInterface::HandleGetRechargeForwardCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK)
	{
		return;
	}

	SDB_RechargeForwardCnfg Data;

	RspIb >> Data;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	m_RechargeBackCnfg.m_BeginTime = Data.m_BeginTime;
	m_RechargeBackCnfg.m_EndTime   = Data.m_EndTime;
	strncpy(m_RechargeBackCnfg.m_szMailTheme, Data.m_szMailTheme, sizeof(m_RechargeBackCnfg.m_szMailTheme));
	strncpy(m_RechargeBackCnfg.m_szMailContent, Data.m_szMailContent, sizeof(m_RechargeBackCnfg.m_szMailContent));

	SDB_GetRechargeForward Req;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(10, enDBCmd_GetRechargeForward, ob.TakeOsb(), this, 0);
}

//是否在充值返利期间
bool	WebInterface::IsInRechargeBackTime()
{
	UINT32 CurTime = CURRENT_TIME();

	return ( CurTime >= m_RechargeBackCnfg.m_BeginTime && CurTime <= m_RechargeBackCnfg.m_EndTime );
}

void	WebInterface::HandleGetTotalRecharge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));

	if ( 0 == pActor )
		return;

	SDB_TotalRecharge Data;

	RspIb >> Data;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	const SRechargeBack * pTotalBack = this->GetTotalRecharge(Data.m_TotalRecharge / 10);

	UINT32 ReadTotalRechare = Data.m_TotalRecharge + DBUserID/*这个在调用时，是传递的充值金额数*/;

	const SRechargeBack * pRealTotalBack = this->GetTotalRecharge(ReadTotalRechare / 10);

	if ( pTotalBack != pRealTotalBack && 0 != pRealTotalBack )
	{
		this->GetRechargeForword(pActor, pRealTotalBack, 0, ReadTotalRechare);
	}
}

//重新加载数据库中的怪物配置
void WebInterface::ReloadMonsterCnfg(UINT8 nCmd, IBuffer & ib)
{
	g_pThingServer->GetMainUIMgr().ReloadXTMonsterCnfg();
}

//修改活动配置
void WebInterface::ChangeActivity(UINT8 nCmd, IBuffer & ib)
{
	SS_Activity Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d Line 后台数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return;
	}

	switch(Req.m_enActivity)
	{
	case enActivity_Del:
		{
			g_pThingServer->GetActivityMgr().DeleteActivity(Req.m_ID);
		}
		break;
	case enActivity_Add:
		{
			g_pThingServer->GetActivityMgr().AddActivity(Req.m_ID);
		}
		break;
	case enActivity_Change:
		{
			g_pThingServer->GetActivityMgr().ChangeActivity(Req.m_ID);
		}
		break;
	}
}

//多倍经验变化
void WebInterface::ChangeMultipExp(UINT8 nCmd, IBuffer & ib)
{
	SDB_GetMultipExpInfo_Req DBReq;

	DBReq.m_ServerID = g_pGameServer->GetServerID();

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ServerID, enDBCmd_GetMultipExpInfo, ob.TakeOsb(), this, 0);
}

//重新加载某时间起首次充值奖励
void WebInterface::ReloadFirstRechargeCnfg(UINT8 nCmd, IBuffer & ib)
{
	this->LoadFirstRechargeForward();
}

void WebInterface::HandleGetMultipExpInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_MultipExpInfo MultipExpInfo;
	RspIb >> RspHeader >> OutParam >> MultipExpInfo;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK)
	{
		TRACE("<warning> %s : %d Line 获取多倍经验返回结果码%d", __FUNCTION__, __LINE__, OutParam.retCode);
		return;
	}

	g_pGameServer->SetMultipExpInfo(MultipExpInfo.m_fMultipExp,MultipExpInfo.m_MinMultipExpLv,MultipExpInfo.m_MaxMultipExpLv,MultipExpInfo.m_MultipExpBeginTime,MultipExpInfo.m_MultipExpEndTime);
}

void WebInterface::HandleGetFirstRechargeCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_FirstRechargeCnfg FirstRechargeCnfg;
	RspIb >> RspHeader >> OutParam >> FirstRechargeCnfg;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK)
	{
		TRACE("<warning> %s : %d Line 首次充值奖励配置表没有配置", __FUNCTION__, __LINE__);
		return;
	}

	m_FirstRechargeCnfg.m_RateTicket = FirstRechargeCnfg.m_RateTicket;
	m_FirstRechargeCnfg.m_RateGodStone = FirstRechargeCnfg.m_RateGodStone;
	m_FirstRechargeCnfg.m_MinRecharge  = FirstRechargeCnfg.m_MinRecharge;
	strncpy(m_FirstRechargeCnfg.m_szMailTheme,FirstRechargeCnfg.m_szMailTheme, sizeof(m_FirstRechargeCnfg.m_szMailTheme));
	strncpy(m_FirstRechargeCnfg.m_szMailContent,FirstRechargeCnfg.m_szMailContent,sizeof(m_FirstRechargeCnfg.m_szMailContent));
}

void WebInterface::HandleIsGetFirstRecharge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_FirstRechargeRet data;
	RspIb >> RspHeader >> OutParam >> data;

	if ( RspIb.Error())
	{
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK)
	{
		return;
	}

	UID uidUser = UID(data.m_uidUser);

	if ( !uidUser.IsValid() )
		return;

	INT32 PayMoney = DBUserID;

	if ( PayMoney >= m_FirstRechargeCnfg.m_MinRecharge && (m_FirstRechargeCnfg.m_RateGodStone > 0 || m_FirstRechargeCnfg.m_RateTicket > 0) )
	{
		this->GetFirstRechargeForward(uidUser,userdata);
	}
}

//给某时间起首次充值的玩家奖励
void WebInterface::GetFirstRechargeForward(UID uidUserr, UINT32 RechargeGodStoneNum)
{
	UINT32 GetGodStone = RechargeGodStoneNum * m_FirstRechargeCnfg.m_RateGodStone / 100.0f + 0.99999;
	UINT32 GetTicket = RechargeGodStoneNum * m_FirstRechargeCnfg.m_RateTicket / 100.0f + 0.99999;

	if ( GetGodStone <= 0 && GetTicket <= 0 )
		return;

	SWriteSystemData SystemMail;

	SystemMail.m_Money = GetGodStone;
	SystemMail.m_Ticket = GetTicket;
	SystemMail.m_DestUID = uidUserr;
	strncpy(SystemMail.m_szThemeText, m_FirstRechargeCnfg.m_szMailTheme, sizeof(SystemMail.m_szThemeText));
	strncpy(SystemMail.m_szContentText, m_FirstRechargeCnfg.m_szMailContent, sizeof(SystemMail.m_szContentText));

	g_pGameServer->GetGameWorld()->WriteSystemMail(SystemMail);
}

//得到某时间首次充值奖励信息
void WebInterface::LoadFirstRechargeForward()
{
	MEM_ZERO(&m_FirstRechargeCnfg);

	SDB_Get_FirstRechargeCnfg ReqFirst;

	OBuffer1k ob;
	ob << ReqFirst;
	g_pGameServer->GetDBProxyClient()->Request(10, enDBCmd_Get_FirstRechargeCnfg, ob.TakeOsb(), this, 0);
}
