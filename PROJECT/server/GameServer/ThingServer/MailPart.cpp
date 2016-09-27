
#include "IActor.h"
#include "MailPart.h"
#include "GameSrvProtocol.h"
#include "ThingServer.h"
#include "IGameWorld.h"
#include "IBasicService.h"
#include "time.h"
#include "IConfigServer.h"
#include "IPacketPart.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "ISyndicateMember.h"
#include "IMailPart.h"
#include "IGoodsServer.h"

MailPart::MailPart()
{
	m_bLoadData = false;

	m_pActor = 0;

	m_MaxMailNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxMailNum;

	m_HaveMailNum = 0;
}

MailPart::~MailPart()
{
}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功 
//////////////////////////////////////////////////////////////////////////
bool MailPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(0 == pMaster || pMaster->GetThingClass() != enThing_Class_Actor){
		return false;
	}

	m_pActor = (IActor*)pMaster;

	this->LoadMailNum();

	return true;
}

//释放
void MailPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart MailPart::GetPartID(void)
{
	return enThingPart_Actor_Mail;
}

//取得本身生物
IThing *		MailPart::GetMaster(void)
{
	return  (IThing *)m_pActor;
}

//玩家下线了，需要关闭该ThingPart
void	MailPart::Close()
{
}

//保存数据
void	MailPart::SaveData()
{
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool	MailPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void	MailPart::InitPrivateClient()
{
}

//IDBProxyClientSink接口
void	MailPart::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
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
	   case enDBCmd_GetActorUIDByName1:
			{
				HandleWriteMail(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	   case enDBCmd_InsertMailInfo:
		   {
			   HandleAddMailDataList(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetMailListInfo:
		   {
			   HandleLoadMailInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetSectionMail:
		   {
			   HandleLoadSectionMailInfo(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   case enDBCmd_GetMailNum:
		   {
			   HandleGetMailNum(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		   }
		   break;
	   default:
		   {
			   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		   }
		   break;
	}

}

//显示邮件主界面
void	MailPart::ViewMailMain()
{
	SC_ViewMailNum_Rsp RspNum;
	RspNum.m_MailNum = m_mapMailData.size();

	OBuffer1k ob;
	ob << MailHeader(enMailCmd_ViewMailMain, sizeof(SC_ViewMailNum_Rsp) + RspNum.m_MailNum * sizeof(ViewMailMainData_Rsp)) << RspNum;

	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.begin();

	for( ;iter != m_mapMailData.end(); ++iter)
	{
		SMailData & MailData = iter->second;

		ViewMailMainData_Rsp Rsp;

		Rsp.m_MailID = iter->first;
		Rsp.m_Time	 = MailData.m_Time;
		strncpy(Rsp.m_szSendUserName, MailData.m_szSendUserName, sizeof(Rsp.m_szSendUserName));
		strncpy(Rsp.m_szThemeText, MailData.m_szThemeText, sizeof(Rsp.m_szThemeText));

		Rsp.m_MailType = MailData.m_MailType;

		Rsp.m_bRead	   = MailData.m_bRead;

		if( MailData.m_MailType == enMailType_System)
		{
			Rsp.m_bHaveGoods = MailData.IsHaveGoods();
		}
		else if( MailData.m_MailType == enMailType_Syn)
		{
			if( MailData.m_SynID != 0){
				Rsp.m_bHaveSynInvate = true;
			}
		}

		ob << Rsp;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//显示邮件主界面
void	MailPart::ViewMailMain(INT16 nBegin, INT16 nEnd)
{
	//加载邮件数据
	this->LoadMailInfo(nBegin, nEnd);
}

//查看邮件
void	MailPart::ViewMail(TMailID MailID)
{
	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.find(MailID);
	if( iter == m_mapMailData.end()){
		TRACE("<warning> %s : %d 行,找不到邮件，邮件ID = %d", __FUNCTION__, __LINE__, MailID);
		return;
	}

	OBuffer2k ob;

	SMailData & MailData = iter->second;

	SC_ViewMailBasic  MailBasic;

	MailBasic.m_MailID	 = iter->first;
	MailBasic.m_MailType = MailData.m_MailType;
	strncpy(MailBasic.m_szContentText, MailData.m_szContentText, sizeof(MailBasic.m_szContentText));
	strncpy(MailBasic.m_szSendUserName, MailData.m_szSendUserName, sizeof(MailBasic.m_szSendUserName));
	strncpy(MailBasic.m_szThemeText, MailData.m_szThemeText, sizeof(MailBasic.m_szThemeText));

	if( MailBasic.m_MailType == enMailType_Person){
		//私人邮件
		ob << MailHeader(enMailCmd_ViewMail, sizeof(MailBasic)) << MailBasic;
	}else if( MailBasic.m_MailType == enMailType_Syn){
		//帮派邮件
		ViewSynMail SynMail;
		SynMail.m_SynID = MailData.m_SynID;

		ob << MailHeader(enMailCmd_ViewMail, sizeof(MailBasic) + sizeof(SynMail)) << MailBasic << SynMail;
	}else{
		//系统邮件
		ViewSystemMail SystemMail;

		SystemMail.m_Money		= MailData.m_Money;
		SystemMail.m_Stone		= MailData.m_Stone;
		SystemMail.m_Ticket		= MailData.m_Ticket;
		SystemMail.m_PolyNimbus		= MailData.m_PolyNimbus;	//聚灵气

		OBuffer2k ob2;

		for( int i = 0; i < ARRAY_SIZE(MailData.m_SDBGoodsData); ++i)
		{
			if( !MailData.m_SDBGoodsData[i].m_uidGoods.IsValid()){
				continue;
			}

			const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(MailData.m_SDBGoodsData[i].m_GoodsID);
			if( 0 == pGoodsCnfg){
				continue;
			}


			MailGoodsData GoodsData;

			GoodsData.m_ResID = pGoodsCnfg->m_ResID;
			GoodsData.m_Number = MailData.m_SDBGoodsData[i].m_Number;


			// fly add	20121106
			strncpy(GoodsData.m_szGoodsName, g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID), sizeof(GoodsData.m_szGoodsName));
			//strncpy(GoodsData.m_szGoodsName, pGoodsCnfg->m_szName, sizeof(GoodsData.m_szGoodsName));
			GoodsData.m_Quality = pGoodsCnfg->m_Quality;

			ob2 << GoodsData;

			++SystemMail.m_GoodsNum;
		}

		ob << MailHeader(enMailCmd_ViewMail, sizeof(MailBasic) + sizeof(SystemMail) + ob2.Size()) << MailBasic << SystemMail;

		if( ob2.Size() > 0){
			ob << ob2;
		}
	}

	if( !MailData.m_bRead){
		MailData.m_bRead = true;

		this->UpdateMailData(MailData.m_MailID, MailData);
	}
	
	m_pActor->SendData(ob.TakeOsb());
}

//删除邮件
void	MailPart::DeleteMail(TMailID MailID)
{
	SC_MailRetCode_Rsp Rsp;
	Rsp.MailRetCode = enMailRetCode_OK;

	OBuffer1k ob;

	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.find(MailID);

	if( iter == m_mapMailData.end()){
		Rsp.MailRetCode = enMailRetCode_NoFindMail;

	}else{
		//
		SDB_Delete_MailDate_Req Req;

		Req.MailID = MailID;

		ob << Req;

		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteMailInfo, ob.TakeOsb(), 0, 0);

		m_mapMailData.erase(iter);

		//邮件数量减1
		this->AddMailNum(-1);
	}
	
	ob.Reset();
	ob << MailHeader(enMailCmd_DeleteMail, sizeof(SC_MailRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//写邮件
void	MailPart::WriteMail(const SWriteData & WriteData)
{
	SDB_Insert_MailData_Req Req;

	Req.Time   = time(0);

	if(enMailType_Syn == WriteData.m_MailType){
		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return;
		}

		ISyndicate * pSyn = pSynMgr->GetSyndicate(m_pActor->GetUID());
		if( 0 == pSyn){
			return;
		}

		Req.Mail_Type = enMailType_Syn;

		Req.SynID = pSyn->GetSynID();
		strncpy(Req.Name_SendUser, pSyn->GetSynName(), sizeof(Req.Name_SendUser));
	}else{
		Req.Mail_Type = enMailType_Person;

		strncpy(Req.Name_SendUser, m_pActor->GetName(), sizeof(Req.Name_SendUser));
	}

	Req.uid_User = WriteData.m_UidDestUser.ToUint64();

	strncpy(Req.ContentText, WriteData.m_szContentText, sizeof(Req.ContentText));
	strncpy(Req.ThemeText, WriteData.m_szThemeText, sizeof(Req.ThemeText));

	Req.ThemeText[DESCRIPT_LEN_50 - 1] = '\0';
	Req.ContentText[DESCRIPT_LEN_300 - 1] = '\0';

	if( !m_mapInsertMailData.insert(std::hash_map<TActorID, SDB_Insert_MailData_Req>::value_type(m_pActor->GetCrtProp(enCrtProp_ActorUserID), Req)).second){
		return;
	}

	OBuffer1k ob;

	if( 0 == Req.uid_User){
		//如果收件人UID值为0，则需根据名字获得其UID值
		SDB_Get_ActorUIDByName_Req GetActorIDByName_Req;
		strncpy(GetActorIDByName_Req.ActorName, WriteData.m_szDestUserName, sizeof(GetActorIDByName_Req.ActorName));

		ob << GetActorIDByName_Req;

		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetActorUIDByName1, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());
	}else{
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertMailInfo, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());

		SC_MailRetCode_Rsp Rsp;
		Rsp.MailRetCode = enMailRetCode_OK;

		ob.Reset();
		ob << MailHeader(enMailCmd_WriteMail, sizeof(SC_MailRetCode_Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());
	}	
}


void	MailPart::HandleWriteMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Get_ActorUIDByName_Rsp       ActorIDByName_Rsp;
	RspIb >> RspHeader >> OutParam >> ActorIDByName_Rsp;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd = %d userID = %u len = %d",__FUNCTION__,__LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	std::hash_map<TActorID, SDB_Insert_MailData_Req>::iterator iter = m_mapInsertMailData.find(DBUserID);
	if( iter == m_mapInsertMailData.end()){
		TRACE("<warning> %s : %d 行 邮件插入信息没找到！！！", __FUNCTION__,__LINE__);
		return;
	}

	OBuffer1k ob;

	if(enDBMailRetCode_NoUser == OutParam.retCode){
		//不存在此用户
		SC_MailRetCode_Rsp Rsp;
		Rsp.MailRetCode = enMailRetCode_NoUser;

		ob << MailHeader(enMailCmd_WriteMail, sizeof(SC_MailRetCode_Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());
	}else{
		SDB_Insert_MailData_Req Req = iter->second;

		Req.uid_User = ActorIDByName_Rsp.Uid_Actor;
		ob << Req;

		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertMailInfo, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());
	}

	m_mapInsertMailData.erase(iter);
}

void	MailPart::HandleAddMailDataList(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Insert_MailData_Rsp Rsp;
	RspIb >> RspHeader >> OutParam >> Rsp;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	SC_MailRetCode_Rsp RspResult;
	RspResult.MailRetCode = enMailRetCode_OK;

	OBuffer1k ob;

	if(enMailRetCode_WriteError == OutParam.retCode){
		//邮件入库失败
		RspResult.MailRetCode = enMailRetCode_WriteError;
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(Rsp.uid_User));
	if( 0 != pActor){
		//对方在线的话，对方邮件数量加1
		IMailPart * pMailPart = pActor->GetMailPart();
		if( 0 != pMailPart){
			pMailPart->AddMailNum(1);
		}
	}

	ob << MailHeader(enMailCmd_WriteMail, sizeof(SC_MailRetCode_Rsp)) << RspResult;
	m_pActor->SendData(ob.TakeOsb());
}

//拒绝帮派邀请
void	MailPart::RefuseSynInvite(TMailID MailID)
{
	SC_MailRetCode_Rsp Rsp;
	Rsp.MailRetCode = enMailRetCode_OK;

	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.find(MailID);

	if(iter == m_mapMailData.end()){
		Rsp.MailRetCode = enMailRetCode_NoFindMail;
	}else{
		(iter->second).m_SynID = INVALID_SYN_ID;
	}

	OBuffer1k ob;
	ob << MailHeader(enMailCmd_RefuseSyn, sizeof(SC_MailRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
	
	if(Rsp.MailRetCode == enMailRetCode_OK){
		//更新邮件数据到数据库
		this->UpdateMailData(iter->first, iter->second);

		//刷新下
		this->ViewMail(iter->first);
	}
}

//接受帮派邀请
void	MailPart::AcceptSynInvite(TMailID MailID)
{
	SC_MailRetCode_Rsp Rsp;
	Rsp.MailRetCode = enMailRetCode_OK;

	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.find(MailID);

	if(iter == m_mapMailData.end())
	{
		Rsp.MailRetCode = enMailRetCode_NoFindMail;
	}else{
		SMailData & MailData = iter->second;
		///////////////////////////////////////////
		//加入帮派处理
		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			TRACE("<error> %s : %d 行 帮派管理器获取失败！！",__FUNCTION__, __LINE__);
			return;
		}

		if( pSynMgr->MailJoinSyndicate(m_pActor, MailData.m_SynID, Rsp.MailRetCode)){
			MailData.m_SynID = INVALID_SYN_ID;
		}
	}

	OBuffer1k ob;
	ob << MailHeader(enMailCmd_RefuseSyn, sizeof(SC_MailRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( Rsp.MailRetCode == enMailRetCode_OK){
		//刷新下
		this->ViewMail(iter->first);
		//更新邮件数据到数据库
		this->UpdateMailData(iter->first, iter->second);
	}
}

//更新邮件数据到数据库
void	MailPart::UpdateMailData(TMailID MailID, const SMailData & MailData)
{
	SDB_Updata_MailData_Req Req;

	Req.MailID				  = MailID;
	Req.Money				  = MailData.m_Money;
	Req.Stone				  = MailData.m_Stone;
	Req.Ticket				  = MailData.m_Ticket;
	Req.PolyNimbus			  = MailData.m_PolyNimbus;	//聚灵气
	Req.SynID				  = MailData.m_SynID;
	Req.bRead				  = MailData.m_bRead;
	
	memcpy(&Req.GoodsData, MailData.m_SDBGoodsData, sizeof(Req.GoodsData));

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateMailInfo, ob.TakeOsb(), 0, 0);
}

//接受邮件物品
void	MailPart::AcceptMailItem(TMailID MailID)
{
	SC_MailRetCode_Rsp Rsp;
	Rsp.MailRetCode = enMailRetCode_OK;

	OBuffer1k ob;

	std::hash_map<TMailID, SMailData>::iterator iter = m_mapMailData.find(MailID);

	bool bChange = false;

	do
	{
		if(iter == m_mapMailData.end()){
			Rsp.MailRetCode = enMailRetCode_NoFindMail;
			break;
		}

		SMailData & MailData = iter->second;

		if( 0 != MailData.m_Money){
			if( m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, MailData.m_Money)){

				g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), MailData.m_Money, m_pActor->GetCrtProp(enCrtProp_ActorMoney), (char *)MailData.m_szContentText,enGodStoneChanel_Mail, enGodStoneType_Get);

				MailData.m_Money = 0;
				bChange = true;
			}
		}

		if( 0 != MailData.m_Stone){
			if( m_pActor->AddCrtPropNum(enCrtProp_ActorStone, MailData.m_Stone)){
				MailData.m_Stone = 0;
				bChange = true;
			}
		}

		if( 0 != MailData.m_Ticket){
			if( m_pActor->AddCrtPropNum(enCrtProp_ActorTicket, MailData.m_Ticket)){
				MailData.m_Ticket = 0;
				bChange = true;
			}
		}

		//聚灵气
		if( 0 != MailData.m_PolyNimbus){
			if( m_pActor->AddCrtPropNum(enCrtProp_ActorPolyNimbus, MailData.m_PolyNimbus)){
				MailData.m_PolyNimbus = 0;
				bChange = true;
			}
		}

		for( int i = 0; i < ARRAY_SIZE(MailData.m_SDBGoodsData); ++i)
		{
			if( !MailData.m_SDBGoodsData[i].m_uidGoods.IsValid()){
				continue;
			}

			SDBGoodsData & GoodsData = MailData.m_SDBGoodsData[i];

			IGoods * pGoods = (IGoods*)g_pGameServer->GetGameWorld()->CreateThing(enThing_Class_Goods,INVALID_SCENE_ID,(char*)&GoodsData,sizeof(SDBGoodsData),THING_CREATE_FLAG_DB_DATA);
			if( 0 == pGoods){
				TRACE("<error> %s : %d 行 创建物品失败！！　物品ID = %d", __FUNCTION__,__LINE__, GoodsData.m_GoodsID);
				continue;
			}

			if( m_pActor->GetPacketPart()->AddGoods(pGoods->GetUID())){
				memset(&GoodsData, 0, sizeof(SDBGoodsData));
				bChange = true;

				g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Mail,pGoods->GetGoodsID(),pGoods->GetUID(),pGoods->GetNumber(),(char *)MailData.m_szContentText);
			}else{
				Rsp.MailRetCode = enMailRetCode_FullPacket;
				//销毁物品
				g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
				break;
			}
		}
	}
	while(0);

	ob << MailHeader(enMailCmd_RefuseSyn, sizeof(SC_MailRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( bChange){
		//更新邮件数据到数据库
		this->UpdateMailData(MailID, iter->second);

		//刷新
		ViewMail(MailID);
	}
}

//确认删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
void	MailPart::OKDeleteMail(TMailID MailID)
{
	SDB_Delete_MailDate_Req Req;

	Req.MailID = MailID;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_DeleteMailInfo, ob.TakeOsb(), 0, 0);
}

//取消删除(有物品或是帮派邀请邮件时，会提示玩家是否确认删除)
void	MailPart::CancelDeleteMail(TMailID MailID)
{
}

//写邮件
void	MailPart::WriteMail(SDB_Insert_MailData_Req Req)
{
	if( m_mapInsertMailData.insert(std::hash_map<TActorID, SDB_Insert_MailData_Req>::value_type(m_pActor->GetCrtProp(enCrtProp_ActorUserID), Req)).second){
		return;
	}

	OBuffer1k ob;

	//如果收件人UID值为0，则需根据名字获得其UID值
	if( 0 == Req.uid_User){
		SDB_Get_ActorUIDByName_Req GetActorIDByName_Req;

		ob << GetActorIDByName_Req;

		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetActorUIDByName1, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());
	}else{
		ob << Req;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertMailInfo, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());

		SC_MailRetCode_Rsp Rsp;
		Rsp.MailRetCode = enMailRetCode_OK;

		ob.Reset();
		ob << MailHeader(enMailCmd_WriteMail, sizeof(SC_MailRetCode_Rsp)) << Rsp;
		m_pActor->SendData(ob.TakeOsb());
	}	
}

void	MailPart::HandleLoadMailInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
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

	for(int i = 0; i < OutParam.retCode; ++i)
	{
		SMailData MailData;

		SDB_Get_MailDataList_Rsp   Rsp;
		RspIb >> Rsp;

		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			break;
		}

		MailData.m_MailType		   = (enMailType)Rsp.Mail_Type;
		MailData.m_Money		   = Rsp.Money;
		MailData.m_Stone		   = Rsp.Stone;
		MailData.m_SynID		   = Rsp.SynID;
		strncpy(MailData.m_szContentText, Rsp.ContentText, sizeof(MailData.m_szContentText));
		strncpy(MailData.m_szSendUserName, Rsp.Name_SendUser, sizeof(MailData.m_szSendUserName));
		strncpy(MailData.m_szThemeText, Rsp.ThemeText, sizeof(MailData.m_szThemeText));
		MailData.m_Ticket		   = Rsp.Ticket;
		MailData.m_Time			   = Rsp.Time;
		MailData.m_uid_SendUser	   = UID(Rsp.uid_SendUser);
		MailData.m_PolyNimbus	   = Rsp.PolyNimbus;	//聚灵气
		memcpy(MailData.m_SDBGoodsData, &Rsp.GoodsData, sizeof(MailData.m_SDBGoodsData));

		m_mapMailData[Rsp.MailID]  = MailData;
	}
}

//加载指定区间的邮件邮件
void	MailPart::LoadMailInfo(INT16 nBegin, INT16 nEnd)
{
	if( nBegin > nEnd || nBegin < 0 || nEnd < 0){
		TRACE("<error> %s : %d 行 客户端消息错误！！nBegin=%d, nEnd=%d", __FUNCTION__, __LINE__, nBegin, nEnd);
		return;
	}

	SDB_Get_SectionMailData_Req Req;
	Req.uid_User = m_pActor->GetUID().ToUint64();
	Req.nBegin	 = nBegin;
	Req.nNum	 = nEnd - nBegin + 1;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetSectionMail, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());
}

void	MailPart::HandleLoadSectionMailInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());
	IBuffer ReqIb(ReqOb.Buffer(),ReqOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	INT16 nNum = RspIb.Remain() / sizeof(SDB_Get_MailDataList_Rsp);

	if( OutParam.retCode != 0){
		nNum = 0;
	}

	//要发给客户端的
	std::vector<SMailData> vectMailData;

	for(int i = 0; i < nNum; ++i)
	{
		SMailData MailData;

		SDB_Get_MailDataList_Rsp   Rsp;
		RspIb >> Rsp;

		if( RspIb.Error()){
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			break;
		}
		
		MailData.m_MailID		   = Rsp.MailID;
		MailData.m_MailType		   = (enMailType)Rsp.Mail_Type;
		MailData.m_Money		   = Rsp.Money;
		MailData.m_Stone		   = Rsp.Stone;
		MailData.m_PolyNimbus	   = Rsp.PolyNimbus;	//聚灵气
		MailData.m_SynID		   = Rsp.SynID;
		strncpy(MailData.m_szContentText, Rsp.ContentText, sizeof(MailData.m_szContentText));
		strncpy(MailData.m_szSendUserName, Rsp.Name_SendUser, sizeof(MailData.m_szSendUserName));
		strncpy(MailData.m_szThemeText, Rsp.ThemeText, sizeof(MailData.m_szThemeText));
		MailData.m_Ticket		   = Rsp.Ticket;
		MailData.m_Time			   = Rsp.Time;
		MailData.m_uid_SendUser	   = UID(Rsp.uid_SendUser);
		MailData.m_bRead		   = Rsp.bRead;
		memcpy(MailData.m_SDBGoodsData, &Rsp.GoodsData, sizeof(MailData.m_SDBGoodsData));

		vectMailData.push_back(MailData);

		m_mapMailData.insert(MAP_MAILDATA::value_type(MailData.m_MailID, MailData));
	}

	//发指定区间邮件给客户端
	SC_ViewMailNum_Rsp RspNum;
	RspNum.m_HaveMailNum = m_HaveMailNum;
	RspNum.m_MailNum = vectMailData.size();

	OBuffer4k ob;
	ob << MailHeader(enMailCmd_ViewMailMain, sizeof(SC_ViewMailNum_Rsp) + RspNum.m_MailNum * sizeof(ViewMailMainData_Rsp)) << RspNum;

	for( int i = 0; i < vectMailData.size(); ++i)
	{
		ViewMailMainData_Rsp Rsp;

		Rsp.m_MailID   = vectMailData[i].m_MailID;
		Rsp.m_MailType = vectMailData[i].m_MailType;
		strncpy(Rsp.m_szSendUserName, vectMailData[i].m_szSendUserName, sizeof(Rsp.m_szSendUserName));
		strncpy(Rsp.m_szThemeText, vectMailData[i].m_szThemeText, sizeof(Rsp.m_szThemeText));
		Rsp.m_Time	   = vectMailData[i].m_Time;
		Rsp.m_bRead	   = vectMailData[i].m_bRead;

		if( vectMailData[i].IsHaveGoods()){
			Rsp.m_bHaveGoods = true;
		}

		if( vectMailData[i].m_SynID != 0){
			Rsp.m_bHaveSynInvate = true;
		}

		ob << Rsp;
	}

	m_pActor->SendData(ob.TakeOsb());
}

//加载邮件数量
void	MailPart::LoadMailNum()
{
	SDB_Get_MailNum_Req Req;
	Req.m_uidUser = m_pActor->GetUID().ToUint64();

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_GetMailNum, ob.TakeOsb(), &g_pThingServer->GetMailMgr(), m_pActor->GetUID().ToUint64());
}

//加载邮件数量
void	MailPart::HandleGetMailNum(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	SDB_MailNum MailNum;
	RspIb >> RspHeader >> MailNum;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if( MailNum.m_MailNum < 0){
		return;
	}

	m_HaveMailNum = MailNum.m_MailNum;
}

//清除邮件数据
void	MailPart::ClearAllMailData()
{
	m_mapMailData.clear();
}

//增加邮件数量
void	MailPart::AddMailNum(INT16  nAddNum)
{
	m_HaveMailNum += nAddNum;

	if( nAddNum >= 1){
		//有新邮件到,通知客户端
		OBuffer1k ob;
		ob << MailHeader(enMailCmd_NoticeNewMail, 0);
		m_pActor->SendData(ob.TakeOsb());
	}
}
