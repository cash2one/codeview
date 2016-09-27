#include "Trade.h"
#include "RelationServer.h"
#include "IConfigServer.h"
#include "IActor.h"
#include "IPacketPart.h"
#include "IGameWorld.h"
#include "XDateTime.h"
#include "IBasicService.h"
#include "IPacketPart.h"
#include "IMailPart.h"
#include "IEquipment.h"
#include "DMsgSubAction.h"
#include "ISyndicateMgr.h"
#include "ISyndicate.h"
#include "IGoodsServer.h"

Trade::Trade()
{
	m_MaxSellGoodsNum		= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxSellGoodsNum;

	m_SpendStoneSellGoods	= g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SpendStoneSellGoods;

	m_MaxSellTradeGoodsTime = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxSellTradeGoodsTime;

}

Trade::~Trade()
{
}

bool Trade::Create()
{
	//加载交易的所有数据
	this->LoadTradeData();

	//启动定时器
	g_pGameServer->GetTimeAxis()->SetTimer(enTradeTIMER_ID_CheckDel, this, 60 * 1000, "Trade::Create()");	//每分钟检测一次交易物品的寄售时间是否已到

	return g_pGameServer->GetMessageDispatch()->AddRootMessageSink(enMsgCategory_Trade,this);
}

void	Trade::Close()
{
	//销毁定时器
	g_pGameServer->GetTimeAxis()->KillTimer(enTradeTIMER_ID_CheckDel,this);

	//删除交易物品
	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.begin();
	for( ; iter != m_mapTradeGoods.end(); ++iter)
	{
		MAP_TRADEGOODS & mapTradeGoods = iter->second;

		MAP_TRADEGOODS::iterator it = mapTradeGoods.begin();

		for( ; it != mapTradeGoods.end(); ++it)
		{
			STradeGoods & TradeGoods = it->second;

			IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(TradeGoods.m_SDBGoodsData.m_uidGoods);
			if( 0 == pGoods){
				continue;
			}

			g_pGameServer->GetGameWorld()->DestroyThing(TradeGoods.m_SDBGoodsData.m_uidGoods);
		}
	}

	m_mapTradeGoods.clear();
}

//收到MSG_ROOT消息
void	Trade::OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	typedef  void (Trade::* FUNC_PROC)(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	static FUNC_PROC s_funcProc[enTradeCmd_Max] = 
	{
		&Trade::SellItem,
		&Trade::BuyGoods,
		&Trade::ViewTradeGoods,
		&Trade::EnterTrade,
		&Trade::SeeTradeGoodsInfo,
		&Trade::EnterMyTradeGoods,
		&Trade::ViewMyTradeGoods,
		&Trade::CancelMyTradeGoods,
	};

	if(nCmd >= enTradeCmd_Max || 0 == s_funcProc[nCmd])
	{
		 TRACE("<error> %s : %d line 意外的命令字 %d",__FUNCTION__,__LINE__,nCmd);
		 return;
	}

	(this->*s_funcProc[nCmd])(pActor,nCmd, ib);	
}

//IDBProxyClientSink接口
void	Trade::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
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
	case enDBCmd_GetTradeDataListInfo:
		{
			HandleLoadTradeData(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
		}
		break;
	default:
	    {
		   TRACE("意外的DB应答命令 cmd=%d userID = %u nRetCode = %d",ReqCmd,userID,nRetCode);
	    }
	    break;
	}
}

//定时器接口
void Trade::OnTimer(UINT32 timerID)
{
	time_t CurTime = time(0);
	//检测交易物品的寄售时间是否已到，到的则把交易物品返还给卖家
	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.begin();

	for(;iter != m_mapTradeGoods.end(); ++iter)
	{
		MAP_TRADEGOODS & mapTradeGoods = iter->second;

		MAP_TRADEGOODS::iterator it = mapTradeGoods.begin();

		while(it != mapTradeGoods.end())
		{
			STradeGoods & TradeGoods = it->second;

			if(CurTime - TradeGoods.m_Time >= m_MaxSellTradeGoodsTime){
				
				BackSellerGoods(TradeGoods);

				mapTradeGoods.erase(it++);
				continue;
			}
			++it;
		}
	}	
}


//把物品寄售
void	Trade::SellItem(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_SellGoods_Req	Req;
	ib >> Req;

	if ( ib.Error()){

		TRACE("<error> %s : %d 行 客户端消息长度有误！！", __FUNCTION__, __LINE__);
		return;
	}
	
	if ( this->CanSellItem(pActor, Req) == false){

		return;
	}
	
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if ( 0 == pPacketPart){

		return;
	}
	
	IGoods * pGoods = pPacketPart->GetGoods(Req.m_UidGoods);
	if ( 0 == pGoods){
		return;
	}

	//入库操作
	SDB_Insert_TradeData_Req   InsertReq;

	InsertReq.Price			 = Req.m_Price;
	InsertReq.SubClass		 = pGoods->GetGoodsCnfg()->m_SubClass;
	InsertReq.uid_User		 = pActor->GetUID().m_uid;
	InsertReq.TradeLabel	 = pGoods->GetGoodsCnfg()->m_GoodsClass;
	InsertReq.Time			 = CURRENT_TIME();
	strncpy(InsertReq.szSellerName, pActor->GetName(), sizeof(InsertReq.szSellerName));

	//从背包里面把要寄售的物品删除
	SDBGoodsData DBGoodsData;

	if ( !this->RemoveGoods(pActor, pGoods, Req.m_GoodsNum, DBGoodsData)){

		TRACE("<error> %s : %d 行 物品从背包中移除失败！！,物品ID = %d", __FUNCTION__, __LINE__, pGoods->GetGoodsID());
		return;
	}

	//普通物品要区分下所在标签，有普通和宝石标签
	if ( InsertReq.TradeLabel == (UINT8)enGoodsCategory_Common){

		if ( enGoodsSecondType_Gem == InsertReq.SubClass){

			InsertReq.TradeLabel = (UINT8)enTradeLabel_Gem;
		}
	}

	if ( InsertReq.TradeLabel < enTradeLabel_Common || InsertReq.TradeLabel >= enTradeLabel_Max){

		return;
	}

	bool bVipFree = pActor->GetVipValue(enVipType_bFreeSellGoods);

	if ( !bVipFree){
		
		pActor->AddCrtPropNum(enCrtProp_ActorStone, -m_SpendStoneSellGoods);
	}

	//上步有可能把原来的pGoods给删除了，所以pGoods不能再用了
	pGoods = 0;
	InsertReq.uid_Goods	 = DBGoodsData.m_uidGoods.ToUint64();

	SDBGoodsData * pDBGoodsData =(SDBGoodsData *)&InsertReq.GoodsData;
	*pDBGoodsData = DBGoodsData;

	OBuffer1k ob;
	ob << InsertReq;
	g_pGameServer->GetDBProxyClient()->Request(pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_InsertTradeInfo, ob.TakeOsb(), 0, 0);


	//把物品加到交易物品map中去
	STradeGoods	  TradeGoods;
	TradeGoods.m_uidSeller	  = pActor->GetUID();
	strncpy(TradeGoods.m_szSellerName, pActor->GetName(), sizeof(TradeGoods.m_szSellerName));
	TradeGoods.m_Price		  = InsertReq.Price;
	TradeGoods.m_SubClass     = InsertReq.SubClass;
	TradeGoods.m_Time	      = InsertReq.Time;
	TradeGoods.m_SDBGoodsData = DBGoodsData;

	this->AddToTradeGoodsList((enGoodsCategory)InsertReq.TradeLabel, TradeGoods);

	SC_TradeResult_Rsp Rsp;

	ob.Reset();
	ob << TradeHeader(enTradeCmd_NeedSell, sizeof(SC_TradeResult_Rsp)) << Rsp; 
	pActor->SendData(ob.TakeOsb());
}

//获得玩家的寄售物品集合
void	Trade::GetUserTradeGoods(std::vector<STradeGoods> & vectTradeGoods, const UID & uid_User)
{
	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.begin();

	for( ; iter != m_mapTradeGoods.end(); ++iter)
	{
		MAP_TRADEGOODS & mapTradeGoods = iter->second;

		MAP_TRADEGOODS::iterator it = mapTradeGoods.begin();
		
		for( ; it != mapTradeGoods.end(); ++it){
			if(it->second.m_uidSeller == uid_User){
				vectTradeGoods.push_back(it->second);
			}	
		}
	}
}

//插入到寄售集合中去
void	Trade::AddToTradeGoodsList(enGoodsCategory GoodsCategory, const STradeGoods & TradeGoods)
{
	std::hash_map<UINT8, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.find((UINT8)GoodsCategory);

	if(iter == m_mapTradeGoods.end()){
		MAP_TRADEGOODS	mapTradeGoods;
		mapTradeGoods[TradeGoods.m_SDBGoodsData.m_uidGoods] = TradeGoods;
		m_mapTradeGoods[(enGoodsCategory)GoodsCategory] = mapTradeGoods;
	}else{
		MAP_TRADEGOODS &  mapTradeGoods = iter->second;
		mapTradeGoods[TradeGoods.m_SDBGoodsData.m_uidGoods] = TradeGoods;	
	}
}

//把物品从玩家身上移走
bool	Trade::RemoveGoods(IActor * pActor, IGoods * pGoods, INT16 GoodsNum, SDBGoodsData & GoodsData)
{
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return false;
	}

	int nPropNum = 0;
	pGoods->GetPropNum(enGoodsProp_Number, nPropNum);

	SDB_Save_GoodsReq GoodsDB;
	int nLen = sizeof(GoodsDB);

	if(1 == pGoods->GetGoodsCnfg()->m_PileNum || nPropNum == GoodsNum){
		//当为不可叠加物品 或 移除数刚好等于背包物品的叠加数时，获得DB数据
		pGoods->OnGetDBContext(&GoodsDB, nLen);

		pPacketPart->RemoveGoods(pGoods->GetUID());

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Trade,GoodsDB.m_GoodsID,UID(GoodsDB.m_uidGoods),GoodsNum,"物品拿去寄售");

	}else{
		//否则为可叠加的普通物品，先删除指定数量的物品，再创建指定数量的物品
		TGoodsID GoodsID   = pGoods->GetGoodsID();
		UID		 GoodsUID  = pGoods->GetUID();
		pPacketPart->DestroyGoods(GoodsID, GoodsNum);

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Trade,GoodsID,GoodsUID,GoodsNum,"物品拿去寄售");

		SCreateGoodsContext  GoodsCnt;
		GoodsCnt.m_Binded  = false;
		GoodsCnt.m_GoodsID = GoodsID;
		GoodsCnt.m_Number  = GoodsNum;

		IGoods * pGoodsNew = g_pGameServer->GetGameWorld()->CreateGoods(GoodsCnt);
		
		if( 0 == pGoodsNew){
			return false;
		}

		pGoodsNew->OnGetDBContext(&GoodsDB, nLen);
	}

	GoodsData.m_uidGoods = UID(GoodsDB.m_uidGoods);
	GoodsData.m_GoodsID  = GoodsDB.m_GoodsID;
	GoodsData.m_CreateTime = GoodsDB.m_CreateTime;
	GoodsData.m_Number   = GoodsDB.m_Number;
	GoodsData.m_Binded   = GoodsDB.m_Binded;

	memcpy(&GoodsData.m_TalismanProp, &GoodsDB.GoodsData, sizeof(GoodsData.m_TalismanProp));

	return true;
}

//加载交易数据
void	Trade::LoadTradeData()
{
	SDB_Get_TradeDataList_Req Req;
	Req.nValue = 24;

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(TRADEID, enDBCmd_GetTradeDataListInfo, ob.TakeOsb(), this, 0);
}

//
void	Trade::HandleLoadTradeData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error())
	{
		TRACE("HandleLoadTradeData DB前端应答长度有误 cmd=%d userID = %u len=%d",ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	for(int i = 0; i < OutParam.retCode; ++i){
	
		SDB_Get_TradeDataList_Rsp Rsp;
		RspIb >> Rsp;

		if(Rsp.GoodsCategory < enGoodsCategory_Common || Rsp.GoodsCategory >= enGoodsCategory_Max){
			continue;
		}
		STradeGoods TradeGoods;

		TradeGoods.m_uidSeller				= UID(Rsp.uid_User);
		strncpy(TradeGoods.m_szSellerName, Rsp.szSellerName, sizeof(TradeGoods.m_szSellerName));
		TradeGoods.m_Price					= Rsp.Price;
		TradeGoods.m_SDBGoodsData			= *((SDBGoodsData *)&Rsp.GoodsData);
		TradeGoods.m_SubClass				= Rsp.SubClass;
		TradeGoods.m_Time					= Rsp.Time;

		std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.find(Rsp.GoodsCategory);

		if(iter == m_mapTradeGoods.end())
		{
			MAP_TRADEGOODS  mapTradeGoods;
			mapTradeGoods[TradeGoods.m_SDBGoodsData.m_uidGoods] = TradeGoods;
			m_mapTradeGoods[Rsp.GoodsCategory] = mapTradeGoods;
		}else{
			MAP_TRADEGOODS & mapTradeGoods = iter->second;
			mapTradeGoods[TradeGoods.m_SDBGoodsData.m_uidGoods] = TradeGoods;
		}

		g_pGameServer->GetGameWorld()->CreateThing(enThing_Class_Goods,INVALID_SCENE_ID,(char*)&TradeGoods.m_SDBGoodsData,sizeof(SDBGoodsData),THING_CREATE_FLAG_DB_DATA);
	}
}

//购买物品
void	Trade::BuyGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_BuyGoods_Req Req;

	ib >> Req;
	
	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误！！", __FUNCTION__, __LINE__);
		return;
	}

	if(Req.m_TradeLabel >= enTradeLabel_Max){
		return;
	}

	STradeGoods * pTradeGoods = this->GetTradeGoods((enTradeLabel)Req.m_TradeLabel, Req.m_UidGoods);
	if( 0 == pTradeGoods){
		TRACE("<error> %s : %d 行 找不到交易物品!!标签＝%d,物品UID＝%s",  __FUNCTION__, __LINE__, Req.m_TradeLabel, Req.m_UidGoods.ToString());
		return;
	}

	if( !pTradeGoods->m_SDBGoodsData.m_uidGoods.IsValid()){
		return;
	}

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(pTradeGoods->m_SDBGoodsData.m_uidGoods);
	if(0 == pGoods){
		TRACE("<error> %s : %d line 创建物品失败, goodsid = %d", __FUNCTION__,__LINE__,pTradeGoods->m_SDBGoodsData.m_uidGoods);
		return;
	}

	SC_TradeResult_Rsp Rsp;

	if(pActor->GetCrtProp(enCrtProp_ActorMoney) <  pTradeGoods->m_Price){
		//玩家钱不够
		Rsp.m_ResultRetCode = enTradeRetCode_NoMoney;
	}else{
		pActor->AddCrtPropNum(enCrtProp_ActorMoney, -pTradeGoods->m_Price);

		g_pGameServer->GetGameWorld()->Save_GodStoneLog(pActor->GetCrtProp(enCrtProp_ActorUserID), pTradeGoods->m_Price, pActor->GetCrtProp(enCrtProp_ActorMoney), "交易中购买物品");

		//把卖物品的钱通过邮件发给卖家
		this->SendSellerMoney(pTradeGoods->m_uidSeller, pTradeGoods->m_Price, pTradeGoods->m_SDBGoodsData.m_GoodsID);

		if(pPacketPart->CanAddGoods(pTradeGoods->m_SDBGoodsData.m_GoodsID, pTradeGoods->m_SDBGoodsData.m_Number) == false){

			//背包已满，物品通过邮件发送给玩家
			Rsp.m_ResultRetCode = enTradeRetCode_PacketFull;
			this->SendBuyerGoods(pActor->GetUID(), pGoods, pTradeGoods->m_Price);

			//销毁物品
			//g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
		}else{
			//加入玩家的背包
			pPacketPart->AddGoods(pGoods->GetUID());

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Trade,pGoods->GetGoodsID(),pGoods->GetUID(),pGoods->GetNumber(),"交易中购买物品");
		}

		//发布事件
		SS_SellSuccess SellSuccess;
		SellSuccess.m_GoodsID = pTradeGoods->m_SDBGoodsData.m_GoodsID;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_SellSuccess);
		pActor->OnEvent(msgID,&SellSuccess,sizeof(SellSuccess));

		//把该物品从交易物品中删除
		this->DeleteTradeGoods((enTradeLabel)Req.m_TradeLabel, Req.m_UidGoods);

		//刷新
		this->ViewTradeGoods(pActor, enTradeLabel(Req.m_TradeLabel));
	}

	OBuffer1k ob;
	ob << TradeHeader(enTradeCmd_BuyGoods, sizeof(SC_TradeResult_Rsp)) << Rsp; 
	pActor->SendData(ob.TakeOsb());
}


//根据物品UID值，获取物品的STradeGoods结构
STradeGoods * Trade::GetTradeGoods(enTradeLabel TradeLabel, const UID & uid_Goods)
{
	if(enTradeLabel_Head == TradeLabel || enTradeLabel_Shoulder == TradeLabel || enTradeLabel_Breast == TradeLabel || enTradeLabel_Leg == TradeLabel || enTradeLabel_Foot == TradeLabel){
		TradeLabel = enTradeLabel_Equip;
	}
	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.find((UINT8)TradeLabel);

	if(iter == m_mapTradeGoods.end()){
		return 0;
	}

	MAP_TRADEGOODS & map_TradeGoods = iter->second;
	
	MAP_TRADEGOODS::iterator it = map_TradeGoods.find(uid_Goods);

	if( it != map_TradeGoods.end())
	{
		return &(it->second);
	}

	return 0;
}

//把该物品从交易物品中删除
void	Trade::DeleteTradeGoods(enTradeLabel TradeLabel, const UID uid_Goods)
{
	if(enTradeLabel_Head == TradeLabel || enTradeLabel_Shoulder == TradeLabel || enTradeLabel_Breast == TradeLabel || enTradeLabel_Leg == TradeLabel || enTradeLabel_Foot == TradeLabel){
		TradeLabel = enTradeLabel_Equip;
	}

	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.find((UINT8)TradeLabel);
	if(iter == m_mapTradeGoods.end()){
		return;
	}

	MAP_TRADEGOODS & map_TradeGoods = iter->second;

	MAP_TRADEGOODS::iterator it = map_TradeGoods.find(uid_Goods);
	if( it == map_TradeGoods.end()){
		return;
	}

	STradeGoods & TradeGoods = it->second;

	//数据库删除
	SDB_Delete_TradeData_Req Req;
	
	Req.uid_Goods  = uid_Goods.ToUint64();
	Req.uid_Seller = TradeGoods.m_uidSeller.ToUint64();

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(TRADEID, enDBCmd_DeleteTradeDataInfo, ob.TakeOsb(), 0, 0);

	map_TradeGoods.erase(it);
}

//查看市场指定标签中的物品
void	Trade::ViewTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_ViewTradeGoods_Req Req;
	ib >> Req;

	this->ViewTradeGoods(pActor, Req.m_TradeLabel);
}

//查看市场指定标签中的物品
void	Trade::ViewTradeGoods(IActor *pActor, enTradeLabel TradeLabel)
{
	if(enTradeLabel_Common > TradeLabel || enTradeLabel_Max <= TradeLabel){
		return;
	}
	//如果是查看装备中的子分类，则直接从装备标签中用子分类来筛选
	enEquipPos enSubClass = enEquipPos_Max;
	if(TradeLabel >= enTradeLabel_Head){
		enSubClass = (enEquipPos)(TradeLabel - enTradeLabel_Head);
		TradeLabel = enTradeLabel_Equip;
	}

	SC_ViewTradeGoodsNum_Rsp Rsp;
	
	OBuffer2k ob;
	OBuffer2k ob2;

	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.find((UINT8)TradeLabel);

	if(iter == m_mapTradeGoods.end()){
		Rsp.m_LabelTradeGoodsNum = 0;
		ob << TradeHeader(enTradeCmd_ViewTradeGoods, sizeof(Rsp)) << Rsp;
	}else{
		MAP_TRADEGOODS & mapTradeGoods = iter->second;

		//不要用while，尽量用for,防止死循环
		for( MAP_TRADEGOODS::iterator it = mapTradeGoods.begin(); it != mapTradeGoods.end();++it)
		{
			STradeGoods & TradeGoods = it->second;

			pActor->NoticClientCreatePrivateThing(TradeGoods.m_SDBGoodsData.m_uidGoods);

			if(TradeLabel == enTradeLabel_Equip && enSubClass != enEquipPos_Max){
				if(TradeGoods.m_SubClass != (UINT8)enSubClass){
					continue;
				}
			}

			++Rsp.m_LabelTradeGoodsNum;

			ViewGoodsSellerInfo_Rsp RspInfo;
			strncpy(RspInfo.m_szSellerName, TradeGoods.m_szSellerName, sizeof(RspInfo.m_szSellerName));
			RspInfo.m_UidGoods = TradeGoods.m_SDBGoodsData.m_uidGoods;
			RspInfo.m_Price	   = TradeGoods.m_Price;

			ob2.Push(&RspInfo, sizeof(RspInfo));
			
		}
	}

	ob << TradeHeader(enTradeCmd_ViewTradeGoods, sizeof(Rsp) + ob2.Size()) << Rsp << ob2;

	pActor->SendData(ob.TakeOsb());
}

//进入交易市场
void	Trade::EnterTrade(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	//默认进入到装备标签界面
	this->ViewTradeGoods(pActor, enTradeLabel_Equip); 
}

//查看交易物品的详细信息
void	Trade::SeeTradeGoodsInfo(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_SeeTradeGoodsInfo_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端数据长度有误！！！", __FUNCTION__, __LINE__);
		return;
	}

	STradeGoods * pTradeGoods = GetTradeGoods(Req.m_TradeLabel, Req.m_uidGoods);
	if(0 == pTradeGoods){
		TRACE("<error> %s : %d 行 找不到交易物品!!标签＝%d,物品UID＝%s",  __FUNCTION__, __LINE__, Req.m_TradeLabel, Req.m_uidGoods.ToString());
		return;
	}

	SDBGoodsData DBGoodsData = pTradeGoods->m_SDBGoodsData;

	SGoodsPublicData GoodsData;
	GoodsData.m_Binded = pTradeGoods->m_SDBGoodsData.m_Binded;
	GoodsData.m_CreateTime = pTradeGoods->m_SDBGoodsData.m_CreateTime;
	GoodsData.m_GoodsID	   = pTradeGoods->m_SDBGoodsData.m_GoodsID;
	GoodsData.m_Number	   = pTradeGoods->m_SDBGoodsData.m_Number;
	GoodsData.m_uidGoods   = pTradeGoods->m_SDBGoodsData.m_uidGoods;
	memcpy(&GoodsData.m_TalismanProp, &pTradeGoods->m_SDBGoodsData.m_TalismanProp, sizeof(GoodsData.m_TalismanProp));

	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pTradeGoods->m_SDBGoodsData.m_GoodsID);
	if( 0 == pGoodsCnfg){
		return;
	}
	GoodsData.m_GoodsClass = pGoodsCnfg->m_GoodsClass;

	int nSize = sizeof(SGoodsPublicData);
	
	OBuffer1k ob;
	ob << TradeHeader(enTradeCmd_GoodsInfo, nSize) << GoodsData;

	pActor->SendData(ob.TakeOsb());
}

//交易物品的寄售时间已到，把卖家的交易物品还给卖家
void	Trade::BackSellerGoods(const STradeGoods & TradeGoods)
{
	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(TradeGoods.m_SDBGoodsData.m_uidGoods);
	if( 0 == pGoods){
		return;
	}

	const SGoodsCnfg * pGoodsCnfg = pGoods->GetGoodsCnfg();
	if( 0 == pGoodsCnfg){
		return;
	}

	SWriteSystemData MailData;

	MailData.m_DestUID = TradeGoods.m_uidSeller;

	std::vector<IGoods *> vecGoods;
	vecGoods.push_back(pGoods);

	// fly add	20121106
	sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10052), (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID));
	strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10017), sizeof(MailData.m_szThemeText));
	//sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), "您撤销了%s物品的出售，请注意接受物品", (char*)pGoodsCnfg->m_szName);
	//strncpy(MailData.m_szThemeText, "撤销出售", sizeof(MailData.m_szThemeText));//需修改
	//发送系统邮件

	g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);

	//数据库删除
	SDB_Delete_TradeData_Req Req;
	
	Req.uid_Goods  = TradeGoods.m_SDBGoodsData.m_uidGoods.ToUint64();
	Req.uid_Seller = TradeGoods.m_uidSeller.ToUint64();

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(TRADEID, enDBCmd_DeleteTradeDataInfo, ob.TakeOsb(), 0, 0);
}

//查看我的摊位
void	Trade::EnterMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
//	ViewMyTradeGoods(pActor, enTradeLabel_Equip);
}

//查看我的摊位指定标签的物品
void	Trade::ViewMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	ViewMyTradeGoods(pActor);
}

//撤消我的交易物品
void	Trade::CancelMyTradeGoods(IActor *pActor, UINT8 nCmd,IBuffer & ib)
{
	CS_CancelMyTradeGoods_Req Req;

	ib >> Req;

	if( ib.Error()){
		TRACE("<error> %s : %d 行 客户端消息长度有有误！！", __FUNCTION__, __LINE__);
		return;
	}

	SC_CancelMyTradeGoods_Rsp Rsp;

	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.begin();

	for(; iter != m_mapTradeGoods.end(); ++iter)
	{
		MAP_TRADEGOODS & mapTradeGoods = iter->second;

		MAP_TRADEGOODS::iterator it = mapTradeGoods.find(Req.m_uidGoods);
		if( it == mapTradeGoods.end()){
			continue;
		}

		STradeGoods & TradeGoods = it->second;

		if( TradeGoods.m_uidSeller != pActor->GetUID()){
			return;
		}

		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(TradeGoods.m_SDBGoodsData.m_GoodsID);
		if( 0 == pGoodsCnfg){
			return;
		}

		IPacketPart * pPacketPart = pActor->GetPacketPart();
		if( 0 == pPacketPart){
			return;
		}

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(TradeGoods.m_SDBGoodsData.m_uidGoods);
		if(0 == pGoods){
			TRACE("<error> %s : %d line 创建物品失败, goodsid = %d", __FUNCTION__,__LINE__,TradeGoods.m_SDBGoodsData.m_uidGoods);
			return;
		}

		if( pPacketPart->CanAddGoods(TradeGoods.m_SDBGoodsData.m_GoodsID, TradeGoods.m_SDBGoodsData.m_Number) == false){
			//背包已满，物品通过邮件发送给玩家
			Rsp.m_ResultRetCode = enTradeRetCode_PacketFull;

			SWriteSystemData MailData;

			MailData.m_DestUID = TradeGoods.m_uidSeller;

			std::vector<IGoods *> vecGoods;
			vecGoods.push_back(pGoods);

			// fly add	20121106
			sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10052), (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID));
			strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10017), sizeof(MailData.m_szThemeText));
			//sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), "您撤销了%s物品的出售，请注意接受物品", (char*)pGoodsCnfg->m_szName);						
			//strncpy(MailData.m_szThemeText, "撤销出售", sizeof(MailData.m_szThemeText));
			//发送系统邮件
			g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);
		}else{
			//加入玩家的背包
			pPacketPart->AddGoods(pGoods->GetUID());

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Trade,pGoods->GetGoodsID(),pGoods->GetUID(),pGoods->GetNumber(),"撤消我的交易物品");
		}

		
		
		//数据库删除
		SDB_Delete_TradeData_Req Req;
		
		Req.uid_Goods  = TradeGoods.m_SDBGoodsData.m_uidGoods.ToUint64();
		Req.uid_Seller = TradeGoods.m_uidSeller.ToUint64();

	    	//删除
		mapTradeGoods.erase(it);

		OBuffer1k ob;
		ob << Req;

		g_pGameServer->GetDBProxyClient()->Request(TRADEID, enDBCmd_DeleteTradeDataInfo, ob.TakeOsb(), 0, 0);
		break;
	}

	OBuffer1k ob;
	ob << TradeHeader(enTradeCmd_CancelMyTradeGoods, sizeof(Rsp)) << Rsp;
	pActor->SendData(ob.TakeOsb());
}

//查看我的摊位
void	Trade::ViewMyTradeGoods(IActor *pActor)
{
	SC_ViewMyTradeGoodsNum_Rsp Rsp;
	
	OBuffer2k ob;

	std::hash_map<UINT8/*enTradeLabel*/, MAP_TRADEGOODS>::iterator iter = m_mapTradeGoods.begin();
	for( ; iter != m_mapTradeGoods.end(); ++iter)
	{
		if(iter == m_mapTradeGoods.end()){
			continue;
		}else{
			MAP_TRADEGOODS & mapTradeGoods = iter->second;

			MAP_TRADEGOODS::iterator it = mapTradeGoods.begin();

			for( ;it != mapTradeGoods.end(); ++it)
			{
				STradeGoods & TradeGoods = it->second;

				pActor->NoticClientCreatePrivateThing(TradeGoods.m_SDBGoodsData.m_uidGoods);

				//此时为查看玩家摊位指定标签处的物品
				if(TradeGoods.m_uidSeller != pActor->GetUID()){
					continue;
				}

				++Rsp.m_LabelTradeGoodsNum;

				ViewMyGoodsSellerInfo_Rsp RspInfo;
				RspInfo.m_UidGoods  = TradeGoods.m_SDBGoodsData.m_uidGoods;
				RspInfo.m_Price	    = TradeGoods.m_Price;
				RspInfo.m_TotalTime = m_MaxSellTradeGoodsTime;
				RspInfo.m_LostTime	= CURRENT_TIME() - TradeGoods.m_Time;

				ob.Push(&RspInfo, sizeof(RspInfo));
			}
		}
	}
	OBuffer2k ob2;
	ob2 << TradeHeader(enTradeCmd_ViewMyTradeGoods, sizeof(Rsp) + ob.Size()) << Rsp << ob;
	pActor->SendData(ob2.TakeOsb());
}

//得到最大可寄售数
INT16	Trade::GetMaxSellGoodsNum(IActor * pActor)
{
	ISyndicateMgr * pSynMgr = g_pGameServer->GetSyndicateMgr();
	if( 0 == pSynMgr){
		return m_MaxSellGoodsNum;
	}

	ISyndicate * pSyndicate = pSynMgr->GetSyndicate(pActor->GetUID());
	if( 0 == pSyndicate){
		return m_MaxSellGoodsNum;
	}

	return m_MaxSellGoodsNum + pSyndicate->GetWelfareValue(enWelfare_MaxSellGoodsNum);
}

//是否能够寄售
bool	Trade::CanSellItem(IActor * pActor, const CS_SellGoods_Req & Req)
{
	SC_TradeResult_Rsp	Rsp;

	OBuffer1k ob;

	VECT_TRADEGOODS vectTradeGoods;

	this->GetUserTradeGoods(vectTradeGoods, pActor->GetUID());

	do
	{
		if ( 0 == Req.m_Price ){
			//
			Rsp.m_ResultRetCode = enTradeRetCode_NotZero;
			break;
		} 

		if ( vectTradeGoods.size() >= this->GetMaxSellGoodsNum(pActor)){

			//已达到最大寄售物品数量
			Rsp.m_ResultRetCode = enTradeRetCode_MaxNum;
			break;
		}

		bool bVipFree = pActor->GetVipValue(enVipType_bFreeSellGoods);

		if ( !bVipFree && pActor->GetCrtProp(enCrtProp_ActorStone) < m_SpendStoneSellGoods){
			
			//寄售灵石不足
			Rsp.m_ResultRetCode = enTradeRetCode_NoStone;
			break;
		}

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(UID(Req.m_UidGoods));

		if ( 0 == pGoods){

			return false;
		}

		int nPropNum = 0;
		pGoods->GetPropNum(enGoodsProp_Number, nPropNum);

		if ( nPropNum < Req.m_GoodsNum){

			//输入的物品数量大于背包里的物品数量
			Rsp.m_ResultRetCode = enTradeRetCode_NoGoodsNum;
			break;
		}

	}while(0);

	if ( Rsp.m_ResultRetCode != enTradeRetCode_OK){
		
		ob << TradeHeader(enTradeCmd_NeedSell, sizeof(Rsp)) << Rsp;
		pActor->SendData(ob.TakeOsb());
		return false;
	}

	return true;
}

//把物品通过邮件发送给买家
bool	Trade::SendBuyerGoods(UID uidBuyer, IGoods * pGoods, INT32 Price)
{
	if ( 0 == pGoods ){
		return false;
	}

	const SGoodsCnfg * pGoodsCnfg = pGoods->GetGoodsCnfg();
	if ( 0 == pGoodsCnfg ){
		return false;
	}

	SWriteSystemData MailData;
	MailData.m_DestUID = uidBuyer;

	// fly add	20121106
	strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10015), sizeof(MailData.m_szThemeText));
	sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText), g_pGameServer->GetGameWorld()->GetLanguageStr(10050), Price, (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID));

	std::vector<IGoods *> vecGoods;
	vecGoods.push_back(pGoods);

	//发送系统邮件
	g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);

	return true;	
}

//把钱通过邮件发送给卖家
bool	Trade::SendSellerMoney(UID uidSeller, INT32 Price, TGoodsID GoodsID)
{
	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);
	if ( 0 == pGoodsCnfg ){
		return false;
	}

	SWriteSystemData MailData;
	MailData.m_DestUID = uidSeller;

	//卖家
	MailData.m_Money		   = Price;

	// fly add	20121031

	strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10016), sizeof(MailData.m_szThemeText));
	sprintf_s(MailData.m_szContentText, sizeof(MailData.m_szContentText),g_pGameServer->GetGameWorld()->GetLanguageStr(10051), (char*)g_pGameServer->GetGameWorld()->GetLanguageStr(pGoodsCnfg->m_GoodsLangID), Price);

	//发送系统邮件
	g_pGameServer->GetGameWorld()->WriteSystemMail(MailData);

	return true;
}
