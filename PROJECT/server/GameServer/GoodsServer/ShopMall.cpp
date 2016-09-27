#include "ShopMall.h"
#include "GoodsServer.h"

#include "IActor.h"
#include "GameSrvProtocol.h"
#include "IBasicService.h"

#include "IConfigServer.h"
#include "IPacketPart.h"
#include "DMsgSubAction.h"

#include "IGameWorld.h"
#include "GoodsServer.h"
#include "XDateTime.h"

ShopMall::ShopMall() : m_vecShopMallCnfgByLabel((INT32)enShopMall_Max)
{
}

ShopMall::~ShopMall()
{
}

bool ShopMall::Create()
{
	this->ResetShopMallCnfg();

	//每分钟检测一次
	g_pGameServer->GetTimeAxis()->SetTimer(enShopMallTimer_OneMinute,this,60 * 1000,"ShopMall::Create");

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_ShopMall,this);
}

void ShopMall::Close()
{
	g_pGameServer->GetMessageDispatch()->DelRootMessageSink(enMsgCategory_ShopMall,this);

	OBuffer1k ob;

	for ( int i = 0; i < m_vecShopMallCnfgByLabel.size(); ++i)
	{
		MAP_MALL & mapMall = m_vecShopMallCnfgByLabel[i];

		MAP_MALL::iterator iter = mapMall.begin();

		for ( ; iter != mapMall.end(); ++iter)
		{
			SShopMallCnfg & ShopMallCnfg = iter->second;

			//有份数限制的要更新下
			if ( ShopMallCnfg.m_LeftFen >= 0){
			
				SDB_Update_ShopMallCnfg_Req DBReq;

				DBReq.m_ID		= iter->first;
				DBReq.m_LeftFen = ShopMallCnfg.m_LeftFen;

				ob.Reset();
				ob << DBReq;
				g_pGameServer->GetDBProxyClient()->Request(DBReq.m_ID, enDBCmd_Update_ShopMallCnfg, ob.TakeOsb(), 0, 0);
			}
		}
	}
}

//收到MSG_ROOT消息
void ShopMall::OnRecv(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	typedef void (ShopMall::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enShopMallCmd_Max] = 
	{
		&ShopMall::Enter,
		&ShopMall::ChangeLabel,
		&ShopMall::BuyItem,

		NULL,
		NULL,

		&ShopMall::OpenVipView,
	};

	if(nCmd >= enShopMallCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

//进入
void ShopMall::Enter(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	//进入默认查看为热卖商店
	const MAP_MALL * pvecShopMallCnfg = this->GetShopMallCnfgByLable(enShopMall_Hot);

	if ( 0 == pvecShopMallCnfg){
		
		return;
	}

	SC_ShopMallData_Rsp Rsp;

	OBuffer4k ob;

	MAP_MALL::const_iterator iter = (*pvecShopMallCnfg).begin();

	for ( ; iter != (*pvecShopMallCnfg).end(); ++iter){

		const SShopMallCnfg & ShopMallCnfg = iter->second;

		if ( !ShopMallCnfg.m_bCanSell){
			
			continue;
		}

		SShopMallData ShopMallData;

		ShopMallData.m_id		 = iter->first;
		ShopMallData.m_idGoods   = ShopMallCnfg.m_GoodsID;
		ShopMallData.m_GoodsNum	 = ShopMallCnfg.m_Amount;
		ShopMallData.m_MoneyType = ShopMallCnfg.m_MoneyType;
		ShopMallData.m_MoneyNum  = ShopMallCnfg.m_MoneyNum;

		++Rsp.m_nGoodsNum;
		
		ob << ShopMallData;

		//发配置信息给客户端
		pActor->SendGoodsCnfg(ShopMallCnfg.m_GoodsID);
	}

	OBuffer4k ob2;
	ob2 << ShopMallHeader(enShopMallCmd_Enter, sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){
		
		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}

//切换标签显示
void ShopMall::ChangeLabel(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_ChangeLaber_Req Req;
	
	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	if ( enShopMall_Hot > Req.m_nLaber || enShopMall_Max <= Req.m_nLaber){

		TRACE("<error> %s : %d Line 客户端发送数据有误,标签不在范围内!!标签 = %d !", __FUNCTION__,__LINE__,Req.m_nLaber);
		return;
	}

	const MAP_MALL * pvecShopMallCnfg = this->GetShopMallCnfgByLable(Req.m_nLaber);

	if ( 0 == pvecShopMallCnfg){
		
		return;
	}

	SC_ShopMallData_Rsp Rsp;

	OBuffer4k ob;

	MAP_MALL::const_iterator iter = (*pvecShopMallCnfg).begin();

	for ( ; iter != (*pvecShopMallCnfg).end(); ++iter){

		const SShopMallCnfg & ShopMallCnfg = iter->second;

		if ( !ShopMallCnfg.m_bCanSell){
			
			continue;
		}

		SShopMallData ShopMallData;

		ShopMallData.m_id		 = iter->first;
		ShopMallData.m_idGoods   = ShopMallCnfg.m_GoodsID;
		ShopMallData.m_GoodsNum	 = ShopMallCnfg.m_Amount;
		ShopMallData.m_MoneyType = ShopMallCnfg.m_MoneyType;
		ShopMallData.m_MoneyNum  = ShopMallCnfg.m_MoneyNum;
		ShopMallData.m_Index	 = ShopMallCnfg.m_index;

		++Rsp.m_nGoodsNum;
		
		ob << ShopMallData;

		//发配置信息给客户端
		pActor->SendGoodsCnfg(ShopMallCnfg.m_GoodsID);
	}

	OBuffer4k ob2;
	ob2 << ShopMallHeader(enShopMallCmd_ChangeLabel, sizeof(Rsp) + ob.Size()) << Rsp;

	if ( ob.Size() > 0){
		
		ob2 << ob;
	}

	pActor->SendData(ob2.TakeOsb());
}

//购买物品
void ShopMall::BuyItem(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	CS_BuyItemShop_Req Req;

	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d Line 客户端数据包长度有误 len = %d !", __FUNCTION__,__LINE__,ib.Capacity());
		return ;
	}

	if ( enShopMall_Hot > Req.m_Label || enShopMall_Max <= Req.m_Label){

		TRACE("<error> %s : %d Line 客户端发送数据有误,标签不在范围内!!标签 = %d !", __FUNCTION__,__LINE__,Req.m_Label);
		return;
	}

	const MAP_MALL * pvecShopMallCnfg = this->GetShopMallCnfgByLable(Req.m_Label);

	if ( 0 == pvecShopMallCnfg){

		return;
	}

	MAP_MALL::const_iterator iter = (*pvecShopMallCnfg).find(Req.m_ID);

	SC_BuyShopMall_Rsp Rsp;

	enMoneyType MoneyType = enMoneyType_Max;
	TGoodsID	GoodsID   = 0;

	do
	{
		if ( iter == (*pvecShopMallCnfg).end()){
			
			Rsp.m_Result = enShopMallRetCode_NoItem;
			break;
		}
	
		const SShopMallCnfg & ShopMallCnfg = iter->second;

		GoodsID = ShopMallCnfg.m_GoodsID;

		if ( g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxBuyGoodsNum < ShopMallCnfg.m_Amount * Req.m_GoodsNum){
			
			Rsp.m_Result = enShopMallRetCode_NumLimit;
			break;
		}

		if ( !ShopMallCnfg.m_bCanSell){
			
			Rsp.m_Result = enShopMallRetCode_NoBuy;
			break;
		}

		if ( ShopMallCnfg.m_LeftFen != -1 && ShopMallCnfg.m_LeftFen < Req.m_GoodsNum){

			Rsp.m_Result = enShopMallRetCode_OverGoods;
			break;
		}

		IPacketPart * pPacketPart = pActor->GetPacketPart();

		if ( 0 == pPacketPart){
			
			return;
		}

		if ( !pPacketPart->CanAddGoods(ShopMallCnfg.m_GoodsID, ShopMallCnfg.m_Amount * Req.m_GoodsNum, ShopMallCnfg.m_bBinded)){
			
			Rsp.m_Result = enShopMallRetCode_PacketFull;
			break;
		}

		INT32 MoneyNum = ShopMallCnfg.m_MoneyNum * Req.m_GoodsNum;

		//买物品
		switch ( ShopMallCnfg.m_MoneyType)
		{
		case enShopMoneyType_GodStone:
			{
				if ( pActor->GetCrtProp(enCrtProp_ActorMoney) < MoneyNum){
					
					Rsp.m_Result = enShopMallRetCode_NoMoney;
					break;
				}

				pActor->AddCrtPropNum(enCrtProp_ActorMoney, -MoneyNum);

				MoneyType = enMoneyType_Money;
			}
			break;
		case enShopMoneyType_Ticket:
			{
				if ( pActor->GetCrtProp(enCrtProp_ActorTicket) < MoneyNum){

					Rsp.m_Result = enShopMallRetCode_NoTicket;
					break;
				}

				pActor->AddCrtPropNum(enCrtProp_ActorTicket, -MoneyNum);

				MoneyType = enMoneyType_Ticket;
			}
			break;
		case enShopMoneyType_Stone:
			{
				if ( pActor->GetCrtProp(enCrtProp_ActorStone) < MoneyNum){

					Rsp.m_Result = enShopMallRetCode_NoStone;
					break;
				}

				pActor->AddCrtPropNum(enCrtProp_ActorStone, -MoneyNum);

				MoneyType = enMoneyType_Stone;
			}
			break;
		case enShopMoneyType_Honor:
			{
				if ( pActor->GetCrtProp(enCrtProp_ActorHonor) < MoneyNum){

					Rsp.m_Result = enShopMallRetCode_NoHonor;
					break;
				}

				pActor->AddCrtPropNum(enCrtProp_ActorHonor, -MoneyNum);

				MoneyType = enMoneyType_Honor;
			}
			break;
		case enShopMoneyType_Credit:
			{
				if ( pActor->GetCrtProp(enCrtProp_ActorCredit) < MoneyNum){

					Rsp.m_Result = enShopMallRetCode_NoCredit;
					break;
				}

				pActor->AddCrtPropNum(enCrtProp_ActorCredit, -MoneyNum);

				MoneyType = enMoneyType_Credit;
			}
			break;
		default:
			TRACE("<error> %s : %d Line 配置表中的货币类型错误!!,货币类型=%d", __FUNCTION__, __LINE__, ShopMallCnfg.m_MoneyType);
			return;
		}

		if ( Rsp.m_Result != enShopMallRetCode_OK){
			
			break;
		}

		pPacketPart->AddGoods(ShopMallCnfg.m_GoodsID, ShopMallCnfg.m_Amount * Req.m_GoodsNum, ShopMallCnfg.m_bBinded);

		if ( ShopMallCnfg.m_LeftFen > 0){
			
			//该物品有数量限制
			SShopMallCnfg * pShopMallCnfg = const_cast<SShopMallCnfg *>(&ShopMallCnfg);

			if ( 0 != pShopMallCnfg){
				
				--pShopMallCnfg->m_LeftFen;
			}
		}

		//日志
		SDB_Insert_ShopMallLog_Req DBReq;

		DBReq.m_BuyNum = Req.m_GoodsNum;
		DBReq.m_ShopMallID = Req.m_ID;
		DBReq.m_UserID = pActor->GetCrtProp(enCrtProp_ActorUserID);

		OBuffer1k ob;
		ob << DBReq;
		g_pGameServer->GetDBProxyClient()->Request(DBReq.m_UserID, enDBCmd_Insert_ShopMallLog, ob.TakeOsb(), 0, 0);

	}while(0);

	OBuffer1k ob;
	ob << ShopMallHeader(enShopMallCmd_Buy, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());

	//发布事件
	if ( enShopMallRetCode_OK == Rsp.m_Result){

		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

		if ( 0 == pGoodsCnfg){
			return;
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Buy,GoodsID,UID(),Req.m_GoodsNum,"商场买物品");

		//发布事件
		SS_BuyGoodsContext BuyGoodsContext;

		BuyGoodsContext.m_GoodsID	= GoodsID;
		BuyGoodsContext.m_Category	= pGoodsCnfg->m_GoodsClass;
		BuyGoodsContext.m_SubClass	= pGoodsCnfg->m_SubClass;
		BuyGoodsContext.m_ThreeClass = pGoodsCnfg->m_ThirdClass;
		BuyGoodsContext.m_BuyPlaceType = enBugPlaceType_ShopMall;
		BuyGoodsContext.m_MoneyType  = MoneyType;
		
		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_BuyGoods);
		pActor->OnEvent(msgID,&BuyGoodsContext,sizeof(BuyGoodsContext));
	}

}

//重新设置商场配置信息
void ShopMall::ResetShopMallCnfg()
{
	for ( int i = 0; i < m_vecShopMallCnfgByLabel.size(); ++i)
	{
		MAP_MALL & mapMall = m_vecShopMallCnfgByLabel[i];

		mapMall.clear();
	}

	SDB_Get_ShopMallCnfg_Req DBReq;

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(5, enDBCmd_Get_AllShopMallCnfg, ob.TakeOsb(), this, 0);
}

// nRetCode: 取值于 enDBRetCode
void ShopMall::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	if ( nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	switch(ReqCmd)
	{
	case enDBCmd_Get_AllShopMallCnfg:
		{
			HandleGetShopMallCnfg(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
		{
		   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	    }
		break;
	}
}

void ShopMall::OnTimer(UINT32 timerID)
{
	if ( timerID == enShopMallTimer_OneMinute){
		
		UINT32 nCurTime = CURRENT_TIME();

		for ( int i = 0; i < m_vecShopMallCnfgByLabel.size(); ++i)
		{
			MAP_MALL & mapMall = m_vecShopMallCnfgByLabel[i];

			MAP_MALL::iterator iter = mapMall.begin();

			for ( ; iter != mapMall.end(); ++iter)
			{
				SShopMallCnfg & ShopGoodsCnfg = iter->second;

				if ( ShopGoodsCnfg.m_bCanSell){
					
					if ( (nCurTime >= ShopGoodsCnfg.m_EndTime &&  ShopGoodsCnfg.m_EndTime != 0) ||
						ShopGoodsCnfg.m_LeftFen == 0)
					{
						ShopGoodsCnfg.m_bCanSell = false;
					}
				}else{
					
					if ( (nCurTime >= ShopGoodsCnfg.m_StartTime || ShopGoodsCnfg.m_StartTime == 0) &&
						 (nCurTime < ShopGoodsCnfg.m_EndTime || ShopGoodsCnfg.m_EndTime == 0) &&
						 (ShopGoodsCnfg.m_LeftFen > 0 || ShopGoodsCnfg.m_LeftFen == -1))
					{
						ShopGoodsCnfg.m_bCanSell = true;
					}
				}
			}
		}
	}
}

//得到商场配置信息
void ShopMall::HandleGetShopMallCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if ( RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if ( OutParam.retCode != enDBRetCode_OK){

		return;
	}

	UINT32 nCurTime = CURRENT_TIME();

	INT32 nNum = RspIb.Remain() / sizeof(SDB_Get_ShopMallCnfg);

	for ( int i = 0; i < nNum; ++i)
	{
		SDB_Get_ShopMallCnfg  DBShopMallCnfg;

		RspIb >> DBShopMallCnfg;

		if ( RspIb.Error()){
			
			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__, ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		this->Push_ShopMallCnfg(DBShopMallCnfg);
	}
}

//放进商场配置表
void  ShopMall::Push_ShopMallCnfg(const SDB_Get_ShopMallCnfg & DBShopMallCnfg)
{
	if ( DBShopMallCnfg.m_Type >= enShopMall_Max){
		
		TRACE("<error> %s : %d Line 商场配置信息出错,类型 = %d", __FUNCTION__, __LINE__, DBShopMallCnfg.m_Type);
		return;
	}

	SShopMallCnfg ShopMallCnfg;

	ShopMallCnfg.m_bBinded   = DBShopMallCnfg.m_Binded;
	ShopMallCnfg.m_Amount    = DBShopMallCnfg.m_GoodsNum;
	ShopMallCnfg.m_GoodsID	 = DBShopMallCnfg.m_GoodsID;
	ShopMallCnfg.m_MoneyType = (enShopMoneyType)DBShopMallCnfg.m_MoneyType;
	ShopMallCnfg.m_MoneyNum  = DBShopMallCnfg.m_MoneyNum;
	ShopMallCnfg.m_Type	     = DBShopMallCnfg.m_Type;
	ShopMallCnfg.m_EndTime   = DBShopMallCnfg.m_EndTime;
	ShopMallCnfg.m_StartTime = DBShopMallCnfg.m_StartTime;
	ShopMallCnfg.m_LeftFen   = DBShopMallCnfg.m_LeftFen;

	UINT32 nCurTime = CURRENT_TIME();

	if ( (nCurTime >= ShopMallCnfg.m_StartTime || ShopMallCnfg.m_StartTime == 0) && 
		 (nCurTime < ShopMallCnfg.m_EndTime || ShopMallCnfg.m_EndTime == 0) && 
		 (ShopMallCnfg.m_LeftFen > 0 || ShopMallCnfg.m_LeftFen == -1))
	{	
		ShopMallCnfg.m_bCanSell = true;
	}

	MAP_MALL & Map = m_vecShopMallCnfgByLabel[ShopMallCnfg.m_Type];

	MAP_MALL::iterator iter = Map.find(DBShopMallCnfg.m_ID);

	if ( iter == Map.end() )
	{
		ShopMallCnfg.m_index = Map.size();
	}
	else
	{
		SShopMallCnfg & SMCnfg = iter->second;

		ShopMallCnfg.m_index = SMCnfg.m_index;
	}

	Map[DBShopMallCnfg.m_ID] = ShopMallCnfg;
}

//得到商城抒写商店的物品配置集合
const std::hash_map<UINT32/*id*/, SShopMallCnfg> * ShopMall::GetShopMallCnfgByLable(enShopMallLabel label)
{
	if(label >= m_vecShopMallCnfgByLabel.size()){
		return 0;
	}

	return &m_vecShopMallCnfgByLabel[label];
}

//打开VIP页面
void ShopMall::OpenVipView(IActor *pActor,UINT8 nCmd, IBuffer & ib)
{
	SC_OpenVipView Rsp;

	Rsp.m_TotalRechargeNum = pActor->GetCrtProp(enCrtProp_Recharge);

	OBuffer1k ob;
	ob << ShopMallHeader(enShopMallCmd_OpenVipView, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}
