
#include "IActor.h"
#include "Talk.h"
#include "IGameServer.h"
#include "RelationServer.h"
#include "IBasicService.h"
#include "IGameWorld.h"
#include "SendWorldTalk.h"
#include "IGoods.h"
#include "SendSynTalk.h"
#include "ISyndicateMgr.h"
#include "SendWorldViewGoods.h"
#include "SendZhaoMuSynMember.h"
#include "IConfigServer.h"
#include <sstream>
#include "DMsgSubAction.h"
#include "SendWorldNotice.h"
#include "IKeywordFilter.h"
#include "ICDTimerPart.h"
#include "IChengJiuPart.h"
#include "XDateTime.h"
#include "ITeamPart.h"

Talk::Talk()
{
}

Talk::~Talk()
{

}

bool	Talk::Create()
{
	m_WorldTalkCDTimerID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_WorldTalkCDTimerID;
	m_SynTalkCDTimerID	 = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SynTalkCDTimerID;
	m_WorldViewGoodsCDTimerID = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_WorldViewGoodsTimerID;

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Talk,this);
}
void	Talk::Close()
{
	MAP_TALKSYSMSG::iterator iter = m_mapTalkSysMap.begin();

	for( ; iter != m_mapTalkSysMap.end(); ++iter)
	{
		STalkSysMsg & TalkSysMsg = iter->second;

		if( TalkSysMsg.m_bUseTimer){
			g_pGameServer->GetTimeAxis()->KillTimer(TalkSysMsg.m_MsgID, this);
		}
	}
}

void	Talk::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef  void (Talk::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enTalkCmd_Max] = 
	{
		&Talk::PrivateTalk,
		&Talk::WorldTalk,
		&Talk::ViewItem,
		&Talk::SynTalk,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&Talk::ViewGoodsSuperLink,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&Talk::TeamTalk,
		NULL,
		&Talk::ViewSynInfo,
	};

	
	 if(nCmd >= enTalkCmd_Max || 0 == s_funcProc[nCmd])
	 {
		  TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	 }

	 (this->*s_funcProc[nCmd])(pActor,nCmd, ib);
}

//私人聊天
void	Talk::PrivateTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_PrivateTalk_Req Req;

	ib >> Req;

	SC_PrivateTalk_Rsp Rsp;

	SC_PrivateTalkRet_Rsp RspRet;
	RspRet.m_TalkRetCode = enTalkRetCode_OK;

	IActor * pDestUser = g_pGameServer->GetGameWorld()->FindActor(UID(Req.m_DestUserUID));
	if( 0 == pDestUser){
		//玩家不存在或没有此玩家
		RspRet.m_TalkRetCode = enTalkRetCode_ErrorUser;
	}else if( this->IsDontTalk(pActor)){
		//你已被禁言
		RspRet.m_TalkRetCode = enTalkRetCode_DontTalk;
	}else{
		Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(Rsp.m_szSendUserName, pActor->GetName(), sizeof(Rsp.m_szSendUserName));

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if( 0 == pChengJiuPart){
			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		strncpy(Rsp.m_szTitleName, strTitleName.c_str(), sizeof(Rsp.m_szTitleName));

		//关键字过滤		
		g_pGameServer->GetKeywordFilter()->Filter(Req.m_TalkContent);

		strncpy(Rsp.m_szTalkContent, Req.m_TalkContent, sizeof(Rsp.m_szTalkContent));
	}

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_Private, sizeof(RspRet)) << RspRet;
	pActor->SendData(ob.TakeOsb());

	if( enTalkRetCode_OK == RspRet.m_TalkRetCode){

		ob.Reset();
		ob << TalkHeader(enTalkCmd_SycPrivate, sizeof(SC_PrivateTalk_Rsp)) << Rsp;

		OBuffer1k ob2;
		ob2 << TalkHeader(enTalkCmd_SycPrivate, sizeof(SC_PrivateTalk_Rsp)) << Rsp;

		pDestUser->SendData(ob.TakeOsb());
		pActor->SendData(ob2.TakeOsb());
	}
}

//组队聊天
void	Talk::TeamTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_TeamTalk_Req Req;

	ib >> Req;

	SC_TeamTalk_Rsp Rsp;

	SC_TeamTalkRet_Rsp RspRet;
	RspRet.m_TalkRetCode = enTalkRetCode_OK;

	ITeamPart * pTeamPart = pActor->GetTeamPart();

	if ( 0 == pTeamPart ){
		return;
	}
	IActor * pDestUser = pTeamPart->GetTeamMember();
	if( 0 == pDestUser){
		//没有队友
		RspRet.m_TalkRetCode = enTalkRetCode_ErrorNoTeamMember;
	}else if( this->IsDontTalk(pActor)){
		//你已被禁言
		RspRet.m_TalkRetCode = enTalkRetCode_DontTalk;
	}else{
		Rsp.m_SendUserUID = pActor->GetUID().ToUint64();
		Rsp.m_SendUserVipLevel = pActor->GetCrtProp(enCrtProp_TotalVipLevel);
		strncpy(Rsp.m_szSendUserName, pActor->GetName(), sizeof(Rsp.m_szSendUserName));

		IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
		if( 0 == pChengJiuPart){
			return;
		}

		std::string strTitleName = pChengJiuPart->GetTitle();

		strncpy(Rsp.m_szTitleName, strTitleName.c_str(), sizeof(Rsp.m_szTitleName));

		//关键字过滤		
		g_pGameServer->GetKeywordFilter()->Filter(Req.m_TalkContent);

		strncpy(Rsp.m_szTalkContent, Req.m_TalkContent, sizeof(Rsp.m_szTalkContent));
	}

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_Team, sizeof(RspRet)) << RspRet;
	pActor->SendData(ob.TakeOsb());

	if( enTalkRetCode_OK == RspRet.m_TalkRetCode){

		ob.Reset();
		ob << TalkHeader(enTalkCmd_SycTeam, sizeof(SC_TeamTalk_Rsp)) << Rsp;

		OBuffer1k ob2;
		ob2 << TalkHeader(enTalkCmd_SycTeam, sizeof(SC_TeamTalk_Rsp)) << Rsp;

		pDestUser->SendData(ob.TakeOsb());
		pActor->SendData(ob2.TakeOsb());
	}
}


//世界聊天
void	Talk::WorldTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICDTimeMgr * pCDTimeMgr = g_pGameServer->GetCDTimeMgr();
	if( 0 == pCDTimeMgr){
		return;
	}

	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	SC_WorldTalkRet_Rsp Rsp;
	Rsp.m_RetCode = enTalkRetCode_OK;
	OBuffer1k ob;

	if( !pCDTimerPart->IsCDTimeOK(m_WorldTalkCDTimerID)){
		//冷却时间未
		Rsp.m_RetCode = enTalkRetCode_ErrorCDTime;
	}else if( this->IsDontTalk(pActor)){
		//你已被禁言
		Rsp.m_RetCode = enTalkRetCode_DontTalk;
	}else{
		CS_WorldTalk_Req Req;
		
		ib >> Req;

		if( true == this->Check_Exec_GMCmd(pActor, Req.m_TalkContent)){
			return;
		}

		//关键字过滤	
		IKeywordFilter * pKeywordFilter = g_pGameServer->GetKeywordFilter();
		if( 0 != pKeywordFilter){
			pKeywordFilter->Filter(Req.m_TalkContent);
		}

		SendWorldTalk WorldData(pActor->GetName(), Req.m_TalkContent, pActor);

		//得先发送结果码
		ob << TalkHeader(enTalkCmd_WorldTalk, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());

		g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, WorldData);

		//登记冷却时间
		pCDTimerPart->RegistCDTime(m_WorldTalkCDTimerID);

		return;
	}

	ob << TalkHeader(enTalkCmd_WorldTalk, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//点击物品超链接查看物品属性
void	Talk::ViewItem(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICDTimeMgr * pCDTimeMgr = g_pGameServer->GetCDTimeMgr();
	if( 0 == pCDTimeMgr){
		return;
	}

	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	SC_ViewThingSuperLink_Rsp Rsp;
	Rsp.m_RetCode = enTalkRetCode_OK;
	OBuffer1k ob;

	if( !pCDTimerPart->IsCDTimeOK(m_WorldViewGoodsCDTimerID)){
		//冷却时间未
		Rsp.m_RetCode = enTalkRetCode_ErrorCDTime;
	}else if( this->IsDontTalk(pActor)){
		//你已被禁言
		Rsp.m_RetCode = enTalkRetCode_DontTalk;
	}else{
		CS_ViewThingSuperLink_Req Req;
		
		ib >> Req;

		if( ib.Error()){
			TRACE("<error> %s : %d 行 获取客户端数据包长度有误！！", __FUNCTION__, __LINE__);
			return;
		}

		switch(Req.m_SuperLinkType)
		{
		case enSuperLink_Goods:
			{
				ViewGoods GoodsInfo;
				ib >> GoodsInfo;
				if( ib.Error()){
					TRACE("<error> %s : %d 行 获取客户端数据包长度有误！！", __FUNCTION__, __LINE__);
					return;
				}

				IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(GoodsInfo.m_uidGoods);
				if (pGoods == 0){
					
					Rsp.m_RetCode = enTalkRetCode_ErrorGoodsNot;
					ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
					pActor->SendData(ob.TakeOsb());
					return;
				}

				ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
				pActor->SendData(ob.TakeOsb());

				SendWorldViewGoods WorldViewGoods(pActor, GoodsInfo.m_uidGoods);
				g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, WorldViewGoods);
			}
			break;
		case enSuperLink_Magic:
			{
				ViewMagic MagicInfo;
				ib >> MagicInfo;
				if( ib.Error()){
					TRACE("<error> %s : %d 行 获取客户端数据包长度有误！！", __FUNCTION__, __LINE__);
					return;
				}

				IMagicPart *pMagicPart = pActor->GetMagicPart();

				if ( 0 == pMagicPart){
					return;
				}

				IMagic * pMagic = pMagicPart->GetMagic(MagicInfo.m_MagicID);
				if ( 0 == pMagic){

					Rsp.m_RetCode = enTalkRetCode_NotExitMagic;
					ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
					pActor->SendData(ob.TakeOsb());
					return;
				}

				ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
				pActor->SendData(ob.TakeOsb());

				SendWorldViewMagic WorldViewMagic(pActor, MagicInfo.m_MagicID);
				g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, WorldViewMagic);
			}
			break;
		case enSuperLink_ChengJiu:
			{
				ViewChengJiu ChengJiuInfo;
				ib >> ChengJiuInfo;
				if( ib.Error()){
					TRACE("<error> %s : %d 行 获取客户端数据包长度有误！！", __FUNCTION__, __LINE__);
					return;
				}
	
				IChengJiuPart * pChengJiuPart = pActor->GetChengJiuPart();
				if( 0 == pChengJiuPart){
					return;
				}

				ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
				pActor->SendData(ob.TakeOsb());

				UINT32 nFinishTime = pChengJiuPart->GetChengJiuFinishTime(ChengJiuInfo.m_ChengJiuID);

				SendWorldViewChengJiu WorldViewChengJiu(pActor, ChengJiuInfo.m_ChengJiuID, nFinishTime);
				g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, WorldViewChengJiu);
			}
			break;
		}

		//登记冷却时间
		pCDTimerPart->RegistCDTime(m_WorldViewGoodsCDTimerID);

		return;
	}

	ob << TalkHeader(enTalkCmd_WorldViewItem, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//帮派聊天
void	Talk::SynTalk(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	ICDTimeMgr * pCDTimeMgr = g_pGameServer->GetCDTimeMgr();
	if( 0 == pCDTimeMgr){
		return;
	}

	ICDTimerPart * pCDTimerPart = pActor->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return;
	}

	SC_SynTalkRetCode_Rsp Rsp;
	Rsp.RetCode = enTalkRetCode_OK;
	OBuffer1k ob;

	if( !pCDTimerPart->IsCDTimeOK(m_SynTalkCDTimerID)){
		//冷却时间未
		Rsp.RetCode = enTalkRetCode_ErrorCDTime;
	}else if( this->IsDontTalk(pActor)){
		//你已被禁言
		Rsp.RetCode = enTalkRetCode_DontTalk;
	}else{
		CS_SynTalk_Req Req;
		
		ib >> Req;

		//关键字过滤		
		g_pGameServer->GetKeywordFilter()->Filter(Req.m_TalkContent);


		SendSynTalk SSynTalk(Req.m_TalkContent, pActor);

		ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
		if( 0 == pSynMgr){
			return;
		}

		TSynID SynID = pSynMgr->GetUserSynID(pActor->GetUID());
		if( 0 == SynID){
			Rsp.RetCode = enTalkRetCode_ErrorNoSyn;
		}
		//得先发送结果码
		ob << TalkHeader(enTalkCmd_SynTalkRet, sizeof(SC_SynTalkRetCode_Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());

		pSynMgr->VisitAllSynMember(SynID, SSynTalk);

		//登记冷却时间
		pCDTimerPart->RegistCDTime(m_SynTalkCDTimerID);

		return;
	}

	ob << TalkHeader(enTalkCmd_SynTalkRet, sizeof(SC_SynTalkRetCode_Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//检测并处理GM命令,返回值表示是否是GM命令
bool	Talk::Check_Exec_GMCmd(IActor * pActor, const char * pszContent)
{
	const SConfigParam & Param  = g_pGameServer->GetConfigParam();
	if( !Param.m_bOpenControlCmd){
		//服务器没有开启控制台命令
		return false;
	}

	if(*pszContent != '/'){
		//不是GM命令，直接返回
		return false;
	}

	std::stringstream   strstr(pszContent);

	std::string strSubString;
	//第一个是命令
	std::getline(strstr, strSubString, ',');

	const SGMCmdCnfg * pGMCmdCnfg = g_pGameServer->GetConfigServer()->GetGMCmdCnfg(strSubString);
	if( 0 == pGMCmdCnfg){
		//不是GM命令，直接返回
		return false;
	}

	UseControlCmd UseCmd;
	UseCmd.m_pGMCmdCnfg = (SGMCmdCnfg *)pGMCmdCnfg;
	strncpy(UseCmd.m_szPermission, pszContent, sizeof(UseCmd.m_szPermission));

	if( !m_mapUserControlCmd.insert(std::map<UID, UseControlCmd>::value_type(pActor->GetUID(), UseCmd)).second){
		return false;
	}

	//检测是否有权限
	SDB_Get_UserCrlPermission Req;

	Req.m_ActorID = pActor->GetCrtProp(enCrtProp_ActorUserID);

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_Get_UserCrlPermission, ob.TakeOsb(), this, pActor->GetUID().ToUint64());

	return true;
}

//帮派招募成员
void	Talk::ZhaoMuSynMember(IActor *pActor)
{
	SendZhaoMuSynMember ZhaoMuSynMember(pActor);

	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, ZhaoMuSynMember);
}

//传音符全服公告
void	Talk::WorldNotice(IActor * pActor, const char * pszNoticeContext)
{
	SendWorldNotice WorldNotice(pActor, pszNoticeContext);

	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, WorldNotice);
}

//世界频道的系统消息(pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
void	Talk::WorldSystemMsg(const char * pszMsgContext, IActor * pActor, enTalkMsgType enMsgType)
{
	if ( pActor == 0){
		
		this->SendWorldTalkSysMsg(pszMsgContext);

	}else{

		if(enMsgType == enTalkMsgType_UpLevel){

			this->SendUserTalkSysMsg(pActor, pszMsgContext,enTalkChannel_World,enMsgType);

		}else{

			this->SendUserTalkSysMsg(pActor, pszMsgContext);
		}
	

	}
}

//右上角的提示悬浮框
void  Talk::SendTipBox(IActor * pActor, const char * pszTip)
{
	if( 0 == pszTip){
		return;
	}

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_Tip, strlen(pszTip) + 1);
	ob.Push(pszTip, strlen(pszTip) + 1);
	
	pActor->SendData(ob.TakeOsb());
}

//收到全服公告的消息
void	Talk::CalWorldNotice(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_WorldNotice_Req Req;
	ib >> Req;

	this->WorldNotice(pActor, Req.m_szNoticeContext);
}

//查看物品超链接
void	Talk::ViewGoodsSuperLink(IActor * pActor, UINT8 nCmd, IBuffer & ib)
{
	CS_ViewGoodsSuperLink_Req Req;
	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 获取客户端长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	SC_ViewGoodsSuperLink_Rsp Rsp;
	Rsp.m_RetCode = enTalkRetCode_OK;

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(Req.m_uidGoods);
	if( 0 == pGoods){
		Rsp.m_RetCode = enTalkRetCode_ErrorGoodsNot;
	}else{
		pActor->NoticClientCreatePrivateThing(Req.m_uidGoods);
		Rsp.m_uidGoods = Req.m_uidGoods;
	}

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_ViewGoodsSuperLink, sizeof(SC_ViewGoodsSuperLink_Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

// nRetCode: 取值于 enDBRetCode
void Talk::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_Get_UserCrlPermission:
		{
			HandleUseControlCmd(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
			TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
		}
		break;
	}
}

//加入到禁言中
void	Talk::DontTalk_Push(SDB_DontTalk & DBDontTalk)
{
	m_mapDoutTalk[DBDontTalk.m_UserID] = DBDontTalk;
}

//加入聊天框的系统公告
void	Talk::TalkSysMsg_Push(SDB_SysMsgInfo & DBSysMsg)
{
	if( DBSysMsg.m_State == 0){
		//启用

		//如果已在队列中，则先删除，用新数据来显示
		this->RemoveTalkSysMsg(DBSysMsg.m_SysMsgID);

		STalkSysMsg TalkSysMsg;

		TalkSysMsg.m_MsgID	   = DBSysMsg.m_SysMsgID;
		TalkSysMsg.m_BeginTime = DBSysMsg.m_BeginTime;
		TalkSysMsg.m_EndTime   = DBSysMsg.m_EndTime;
		TalkSysMsg.m_IntervalTime = DBSysMsg.IntervalTime;
		TalkSysMsg.m_bUseTimer = false;

		DBSysMsg.MsgContent[DESCRIPT_LEN_100 - 1] = '\0';

		strncpy(TalkSysMsg.m_szMsgContent, DBSysMsg.MsgContent, sizeof(TalkSysMsg.m_szMsgContent));

		m_mapTalkSysMap[DBSysMsg.m_SysMsgID] = TalkSysMsg;

		this->StartSysMsg(DBSysMsg.m_SysMsgID);

	}else if( DBSysMsg.m_State == 1){
		//停用
		this->RemoveTalkSysMsg(DBSysMsg.m_SysMsgID);
	}
}

void	Talk::OnTimer(UINT32 timerID)
{
	MAP_TALKSYSMSG::iterator iter = m_mapTalkSysMap.find(timerID);
	if( iter == m_mapTalkSysMap.end()){
		return;
	}

	STalkSysMsg & TalkSysMsg = iter->second;

	UINT32 nCurTime = CURRENT_TIME();

	if( !TalkSysMsg.m_bUseTimer && nCurTime >= TalkSysMsg.m_BeginTime && nCurTime < TalkSysMsg.m_EndTime){
		this->StartSysMsg(TalkSysMsg.m_MsgID);
		return;
	}

	if( nCurTime >= TalkSysMsg.m_EndTime){
		this->RemoveTalkSysMsg(TalkSysMsg.m_MsgID);
		return;
	}

	if( nCurTime >= TalkSysMsg.m_BeginTime){
		this->SendWorldTalkSysMsg(TalkSysMsg.m_szMsgContent);
		return;
	}
}

void	Talk::HandleUseControlCmd(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	SDB_Permission DBPermission;
	RspIb >> RspHeader >> OutParam >> DBPermission;

	if(RspIb.Error()){
		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(UID(userdata));
	if( 0 == pActor){
		return;
	}

	std::map<UID, UseControlCmd>::iterator iter = m_mapUserControlCmd.find(pActor->GetUID());
	if( iter == m_mapUserControlCmd.end()){
		return;
	}

	if( OutParam.retCode != enDBRetCode_OK){
		this->WorldSystemMsg("您没有GM权限！！", pActor);
		m_mapUserControlCmd.erase(iter);
		return;
	}

	UseControlCmd & UseCmd = iter->second;

	if( 0 == UseCmd.m_pGMCmdCnfg){

		this->WorldSystemMsg("程序出错！！", pActor);

	}else if( !this->bInPermission(UseCmd.m_pGMCmdCnfg->m_GMCmdID, DBPermission.m_Permission)){

		this->WorldSystemMsg("您没有此命令的权限！！", pActor);

	}else{

		std::string strSubString;

		std::vector<std::string> vectParam;

		std::stringstream strCmd(UseCmd.m_szPermission);

		//第一个是命令，不是参数，先过滤
		std::getline(strCmd, strSubString, ',');

		while( std::getline(strCmd, strSubString, ','))
		{
			vectParam.push_back(strSubString);
		}

		g_pGameServer->GetRelationServer()->DispatchGMMessage(pActor, (enGMCmd)UseCmd.m_pGMCmdCnfg->m_GMCmdID, vectParam);
	}

	m_mapUserControlCmd.erase(iter);
}

//GM命令ID是否在权限中
bool	Talk::bInPermission(UINT8 GMCmdID, char * pszPermission)
{
	char * pGMCmdID = pszPermission;

	for( ; *pszPermission; )
	{
		if( *pszPermission == ','){
			*pszPermission = '\0';

			int CmdID = atoi(pGMCmdID);

			if( CmdID == GMCmdID || CmdID == -1){
				return true;
			}

			++pszPermission;

			pGMCmdID = pszPermission;
		}else{
			++pszPermission;
		}	
	}

	//验证最后一项
	if( atoi(pGMCmdID) == GMCmdID || atoi(pGMCmdID) == -1){
		return true;
	}

	return false;
}

//是否被禁言
bool	Talk::IsDontTalk(IActor * pActor)
{
	std::map<TUserID, SDB_DontTalk>::iterator iter = m_mapDoutTalk.find(pActor->GetCrtProp(enCrtProp_ActorUserID));

	if( iter == m_mapDoutTalk.end()){
		return false;
	}

	SDB_DontTalk & DBDontTalk = iter->second;

	UINT32 nCurTime = CURRENT_TIME();

	if( (0 == DBDontTalk.m_EndTime || nCurTime < DBDontTalk.m_EndTime) && nCurTime >= DBDontTalk.m_BeginTime && (DBDontTalk.m_RemoveTime == 0 || nCurTime < DBDontTalk.m_RemoveTime))
	{
		return true;
	}

	if( (DBDontTalk.m_RemoveTime != 0 && nCurTime >= DBDontTalk.m_RemoveTime) || nCurTime >  DBDontTalk.m_EndTime){
		//禁言时间已到，或者已被解禁言
		m_mapDoutTalk.erase(iter);
	}

	return false;
}

void	Talk::RemoveTalkSysMsg(UINT32	TalkSysMsgID)
{
	MAP_TALKSYSMSG::iterator iter = m_mapTalkSysMap.find(TalkSysMsgID);
	if( iter == m_mapTalkSysMap.end()){
		return;
	}

	STalkSysMsg & TalkSysMsg = iter->second;

	if( TalkSysMsg.m_bUseTimer){
		//停止计时器
		g_pGameServer->GetTimeAxis()->KillTimer(TalkSysMsgID,this);
	}

	m_mapTalkSysMap.erase(iter);
}

//开始定时器
void	Talk::StartSysMsg(UINT32	 TalkSysMsgID)
{
	MAP_TALKSYSMSG::iterator iter = m_mapTalkSysMap.find(TalkSysMsgID);
	if( iter == m_mapTalkSysMap.end()){
		return;
	}

	STalkSysMsg & TalkSysMsg = iter->second;

	if( TalkSysMsg.m_bUseTimer){
		return;
	}
	
	UINT32 nCurTime = CURRENT_TIME();

	if( nCurTime >= TalkSysMsg.m_BeginTime && nCurTime < TalkSysMsg.m_EndTime){
		//发送系统消息
		this->SendWorldTalkSysMsg(TalkSysMsg.m_szMsgContent);
	
		g_pGameServer->GetTimeAxis()->SetTimer(TalkSysMsg.m_MsgID, this, TalkSysMsg.m_IntervalTime * 1000, "Talk::StartSysMsg");
		TalkSysMsg.m_bUseTimer = true;
	} 
}

//显示帮派数据
void	Talk::ViewSynInfo(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ViewSynInfo Req;

	ib >> Req;

	if( ib.Error())
	{
		TRACE("<error> %s : %d 行 获取客户端长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();

	if ( 0 == pSynMgr )
		return;

	ISyndicate * pSyn = pSynMgr->GetSyndicate(Req.m_SynID);

	if ( 0 == pSyn )
		return;

	SC_ViewSynInfo Rsp;

	Rsp.m_SynID = pSyn->GetSynID();
	Rsp.m_SynLevel = pSyn->GetSynLevel();
	Rsp.m_SynMemberNum = pSyn->GetSynMemberNum();
	strncpy(Rsp.m_SynLeaderName, pSyn->GetLeaderName(), sizeof(Rsp.m_SynLeaderName));
	strncpy(Rsp.m_SynName, pSyn->GetSynName(), sizeof(Rsp.m_SynName));

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_ViewSynInfo, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//发送聊天框系统消息
void	Talk::SendUserTalkSysMsg(IActor * pActor, const char * pszMsgContent, enTalkChannel enChannel,enTalkMsgType	enMsgType)
{
	SC_TalkSysMsg Rsp;

	Rsp.m_Channel = enChannel;
	strncpy(Rsp.m_szSysMsg, pszMsgContent, sizeof(Rsp.m_szSysMsg));
	Rsp.m_TalkMsgType = enMsgType;

	OBuffer1k ob;
	ob << TalkHeader(enTalkCmd_SC_TalkSysMsg, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//对世界玩家发送聊天框系统消息
void	Talk::SendWorldTalkSysMsg(const char * pszMsgContent)
{
	SendTalkSysMsg TalkSysMsg(pszMsgContent);
	g_pGameServer->GetGameWorld()->VisitWorldThing(enThing_Class_Actor, TalkSysMsg);
}
