
#include "IActor.h"
#include "GodSwordShopPart.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "XDateTime.h"
#include "IPacketPart.h"
#include "DMsgSubAction.h"
#include "IBasicService.h"
#include "IGoodsServer.h"

GodSwordShopPart::GodSwordShopPart()
{
	m_pActor = 0;

	m_nCDTimeFreeFlush = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_CDFreeFlushTime;

	m_SpendMoneyNumMoneyFlush = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SpendMoneyNumMoneyFlush;
}

GodSwordShopPart::~GodSwordShopPart()
{

}

//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool GodSwordShopPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || enThing_Class_Actor != pMaster->GetThingClass() || 0 == pContext || nLen < sizeof(SDB_Get_GodSwordShopData_Rsp)){
		return false;
	}

	m_pActor = (IActor *)pMaster;

	SDB_Get_GodSwordShopData_Rsp * pDBGodSwordData = (SDB_Get_GodSwordShopData_Rsp *)pContext;

	m_GodSwordShopData.m_LastFlushTime = pDBGodSwordData->LastFlushTime;

	m_GodSwordShopData.m_GodSword[0] = pDBGodSwordData->IdGodSword1;
	m_GodSwordShopData.m_GodSword[1] = pDBGodSwordData->IdGodSword2;
	m_GodSwordShopData.m_GodSword[2] = pDBGodSwordData->IdGodSword3;
	m_GodSwordShopData.m_GodSword[3] = pDBGodSwordData->IdGodSword4;

	if(  0 == m_GodSwordShopData.m_LastFlushTime){
		//第一次进,刷新4把仙剑出来
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->RandGetGodSwordShopCnfg(enFlushType_Free);
			if( 0 == pGodSwordCnfg){
				TRACE("<error> %s : %d 行 刷新仙剑,返回的仙剑配置指针为0！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GodSwordShopData.m_GodSword[i] = pGodSwordCnfg->m_GodSwordID;
		}

		m_GodSwordShopData.m_LastFlushTime = CURRENT_TIME() - m_nCDTimeFreeFlush;

		//新建角色的特殊处理
		this->NewUserSpecialCal();

	}

	return true;
}

//释放
void GodSwordShopPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart GodSwordShopPart::GetPartID(void)
{
	return enThingPart_Actor_GodSwordShop;
}

//取得本身生物
IThing*		GodSwordShopPart::GetMaster(void)
{
	return (IThing * )m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool		GodSwordShopPart::OnGetDBContext(void * buf, int &nLen)
{
	if(0 == buf || nLen < sizeof(SDB_Update_GodSwordShopData_Req))
	{
		return false;
	}

	SDB_Update_GodSwordShopData_Req * pDBGodSwordData = (SDB_Update_GodSwordShopData_Req *)buf;

	pDBGodSwordData->Uid_User	= m_pActor->GetUID().ToUint64();

	pDBGodSwordData->LastFlushTime  = m_GodSwordShopData.m_LastFlushTime;

	pDBGodSwordData->IdGodSword1 = m_GodSwordShopData.m_GodSword[0];
	pDBGodSwordData->IdGodSword2 = m_GodSwordShopData.m_GodSword[1];
	pDBGodSwordData->IdGodSword3 = m_GodSwordShopData.m_GodSword[2];
	pDBGodSwordData->IdGodSword4 = m_GodSwordShopData.m_GodSword[3];
	
	nLen = sizeof(SDB_Update_GodSwordShopData_Req);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void		GodSwordShopPart::InitPrivateClient()
{
}

//玩家下线了，需要关闭该ThingPart
void		GodSwordShopPart::Close()
{
}

//保存数据
void		GodSwordShopPart::SaveData()
{
	SDB_Update_GodSwordShopData_Req Req;

	int nLen = sizeof(SDB_Update_GodSwordShopData_Req);

	if( this->OnGetDBContext(&Req, nLen) == false){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateGodSwordShopInfo,ob.TakeOsb(),0,0);
}

//进入剑冢
void		GodSwordShopPart::EnterGodSwordShop()
{
	this->SyncGodSwordShop();
}

//刷新剑冢
void		GodSwordShopPart::FlushGodSwordShop(enFlushType FlushType)
{
	if( enFlushType_Free == FlushType){
		//免费刷新
		this->FreeFlush();
	}else if( enFlushType_Money == FlushType){
		//付费刷新
		this->MoneyFlush();
	}
}

//购买仙剑
void		GodSwordShopPart::BuyGodSword(UINT8 nIndex)
{
	IPacketPart * pPacketPart = m_pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	if( nIndex >= FLUSH_NUM){
		TRACE("<error> %s : %d 行 仙剑配置信息获取出错！！位置索引 = %d", __FUNCTION__, __LINE__, nIndex);
		return;
	}

	SC_GodSwordShopRetCode_Rsp Rsp;
	Rsp.enRetCode = enGodSwordShopRetCode_ErrorNoGoods;
	
	if( m_GodSwordShopData.m_GodSword[nIndex] != 0){

		const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->GetGodSwordShopCnfg(m_GodSwordShopData.m_GodSword[nIndex]);
		if( 0 == pGodSwordCnfg){
			TRACE("<error> %s : %d 行 仙剑配置信息获取出错！！仙剑ID = %d", __FUNCTION__, __LINE__, m_GodSwordShopData.m_GodSword[nIndex]);
			return;
		}

		INT32 nNeedStone = pGodSwordCnfg->m_Stone - pGodSwordCnfg->m_Stone * m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney) / 100.0f + 0.9999;

		//购买的仙剑直接绑定
		if( false == pPacketPart->CanAddGoods(pGodSwordCnfg->m_GodSwordID, 1,true)){
			//玩家背包已满
			Rsp.enRetCode = enGodSwordShopRetCode_FullPacket;
		}else if( m_pActor->GetCrtProp(enCrtProp_ActorStone) < nNeedStone){
			//灵石不足
			Rsp.enRetCode = enGodSwordShopRetCode_ErrorStone;		
		}else{
			//扣灵石
			if( false == m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -nNeedStone)){
				TRACE("<error> %s : %d 行 扣灵石失败！！", __FUNCTION__, __LINE__);
				return;
			}

			//购买的仙剑直接绑定

			//仙剑放到玩家背包
			pPacketPart->AddGoods(pGodSwordCnfg->m_GodSwordID,1,true);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Buy,pGodSwordCnfg->m_GodSwordID,UID(),1,"剑冢购买仙剑");

			//被买走的仙剑转置0
			m_GodSwordShopData.m_GodSword[nIndex] = 0;

			Rsp.enRetCode = enGodSwordShopRetCode_OK;	
		}
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_BuyGodSword, sizeof(SC_GodSwordShopRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGodSwordShopRetCode_OK == Rsp.enRetCode){
		//同步
		this->SyncGodSwordShop();

		//发布购买仙剑事件
		SS_BuyGoodsContext BuyGoodsContext;
		BuyGoodsContext.m_GoodsID  = m_GodSwordShopData.m_GodSword[nIndex];
		BuyGoodsContext.m_Category = enGoodsCategory_GodSword;
		BuyGoodsContext.m_SubClass = 0;
		BuyGoodsContext.m_ThreeClass = 0;
		BuyGoodsContext.m_BuyPlaceType = enBugPlaceType_GodSword;
		BuyGoodsContext.m_MoneyType  = enMoneyType_Stone;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_BuyGoods);
		m_pActor->OnEvent(msgID,&BuyGoodsContext,sizeof(BuyGoodsContext));
	}
}

//同步剑冢
void		GodSwordShopPart::SyncGodSwordShop()
{
	time_t tCurrentTime = CURRENT_TIME();

	SC_SynGodSwordShop_Rsp Rsp;

	if( (tCurrentTime - m_GodSwordShopData.m_LastFlushTime) > m_nCDTimeFreeFlush){
		Rsp.m_RemainCDTime = 0;
	}else{
		Rsp.m_RemainCDTime = m_GodSwordShopData.m_LastFlushTime + m_nCDTimeFreeFlush - tCurrentTime;
	}
	
	Rsp.m_PriceFlush = m_SpendMoneyNumMoneyFlush;

	Rsp.m_MoneyDes	 = m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney);

	for( int i = 0; i < FLUSH_NUM; ++i)
	{
		Rsp.m_GodSword[i].m_GodSwordID = m_GodSwordShopData.m_GodSword[i];

		const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->GetGodSwordShopCnfg(m_GodSwordShopData.m_GodSword[i]);
		if( 0 == pGodSwordCnfg){
			continue;
		}

		Rsp.m_GodSword[i].m_Price	   = pGodSwordCnfg->m_Stone;

		//发送配置信息给客户端
		m_pActor->SendGoodsCnfg(m_GodSwordShopData.m_GodSword[i] );
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_SyncGodSwordShop, sizeof(SC_SynGodSwordShop_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//获得剩余刷新仙剑时间
INT32 GodSwordShopPart::GetRemainFlushTime() 
{
		time_t tCurrentTime = CURRENT_TIME();
		INT32 RemainTime = m_nCDTimeFreeFlush - (tCurrentTime - m_GodSwordShopData.m_LastFlushTime);

		if(RemainTime<0)
		{
			RemainTime = 0;
		}

		return  RemainTime;
}

//免费刷新
void		GodSwordShopPart::FreeFlush()
{
	time_t tCurrentTime = CURRENT_TIME();

	SC_GodSwordShopRetCode_Rsp Rsp;
	Rsp.enRetCode = enGodSwordShopRetCode_OK;

	if( GetRemainFlushTime()>0){
		//免费刷新的CD时间还没到
		Rsp.enRetCode = enGodSwordShopRetCode_ErrorCDTime;
	}else{
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->RandGetGodSwordShopCnfg(enFlushType_Free);
			if( 0 == pGodSwordCnfg){
				TRACE("<error> %s : %d 行 刷新仙剑,返回的仙剑配置指针为0！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GodSwordShopData.m_GodSword[i] = pGodSwordCnfg->m_GodSwordID;
		}

		m_GodSwordShopData.m_LastFlushTime = tCurrentTime;
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_FlushGodSwordShop, sizeof(SC_GodSwordShopRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGodSwordShopRetCode_OK == Rsp.enRetCode){
		//同步剑冢
		this->SyncGodSwordShop();
	}
}
//付费刷新
void		GodSwordShopPart::MoneyFlush()
{
	SC_GodSwordShopRetCode_Rsp Rsp;
	Rsp.enRetCode = enGodSwordShopRetCode_OK;

	if( m_pActor->GetCrtProp(enCrtProp_ActorMoney) < m_SpendMoneyNumMoneyFlush && m_pActor->GetCrtProp(enCrtProp_ActorTicket) < m_SpendMoneyNumMoneyFlush){
		//仙石或礼卷不足，不能付费刷新
		Rsp.enRetCode = enGodSwordShopRetCode_ErrorMoney;
	}else{
		//刷新4把仙剑出来
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->RandGetGodSwordShopCnfg(enFlushType_Money);
			if( 0 == pGodSwordCnfg){
				TRACE("<error> %s : %d 行 刷新仙剑,返回的仙剑配置指针为0！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GodSwordShopData.m_GodSword[i] = pGodSwordCnfg->m_GodSwordID;
		}

		if( m_pActor->GetCrtProp(enCrtProp_ActorTicket) >= m_SpendMoneyNumMoneyFlush){
			//优先扣礼卷
			m_pActor->AddCrtPropNum(enCrtProp_ActorTicket, -m_SpendMoneyNumMoneyFlush);
		}else{
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, -m_SpendMoneyNumMoneyFlush);

			g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), m_SpendMoneyNumMoneyFlush, m_pActor->GetCrtProp(enCrtProp_ActorMoney), "剑冢刷新仙剑");
		}
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_FlushGodSwordShop, sizeof(SC_GodSwordShopRetCode_Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGodSwordShopRetCode_OK == Rsp.enRetCode){
		//同步剑冢
		this->SyncGodSwordShop();
	}
}

//新建角色的特殊处理
void  GodSwordShopPart::NewUserSpecialCal()
{
	//1号位置的仙剑必然是新手金剑
	TGoodsID GodSwordID = 15000;

	//2号位置的仙剑必然是新手土剑
	TGoodsID GodSwordID2 = 15044;

	const SGodSwordCnfg * pGodSwordCnfg = g_pGameServer->GetConfigServer()->GetGodSwordShopCnfg(GodSwordID);
	if( 0 == pGodSwordCnfg){
		TRACE("<warning> %s : %d 行 仙剑配置信息获取失败！！仙剑ID = %d", __FUNCTION__, __LINE__, GodSwordID);
	}else{
		m_GodSwordShopData.m_GodSword[0] = GodSwordID;
	}

	pGodSwordCnfg = 0;
	pGodSwordCnfg = g_pGameServer->GetConfigServer()->GetGodSwordShopCnfg(GodSwordID2);
	if( 0 == pGodSwordCnfg){
		TRACE("<warning> %s : %d 行 仙剑配置信息获取失败！！仙剑ID = %d", __FUNCTION__, __LINE__, GodSwordID);
	}else{
		m_GodSwordShopData.m_GodSword[1] = GodSwordID2;
	}
}
