
#include "IActor.h"
#include "GatherGodHousePart.h"
#include "XDateTime.h"
#include "DBProtocol.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IPacketPart.h"
#include "IGameWorld.h"
#include "IBasicService.h"
#include "DMsgSubAction.h"
#include "ICommonGoods.h"
#include "RandomService.h"
#include "IGoodsServer.h"

GatherGodHousePart::GatherGodHousePart()
{
	m_pActor = 0;

	m_nCDTimeFreeFlush = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_CDTimeFreeFlush;

	m_SpendMoneyFlush  = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_SpendMoneyFlush;

	m_StoneFlushMagicBook = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_StoneFlushMagicBook;
}

GatherGodHousePart::~GatherGodHousePart()
{
}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件

// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool GatherGodHousePart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(0 == pMaster || 0 == pContext || nLen < sizeof(SDBGatherGodHouseData))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	const  SDB_Get_GatherGodHouseData_Rsp * pGatherGodHouse = (SDB_Get_GatherGodHouseData_Rsp *)pContext;

	if(0 == pGatherGodHouse){
		return false;
	}

	m_GatherGodHouseData.m_LastFlushEmployTime = pGatherGodHouse->LastFlushEmployTime;
	m_GatherGodHouseData.m_LastFlushMagicTime  = pGatherGodHouse->LastFlushMagicTime;

	m_GatherGodHouseData.m_idEmployee[0]	= pGatherGodHouse->IdEmployee1;
	m_GatherGodHouseData.m_idEmployee[1]	= pGatherGodHouse->IdEmployee2;
	m_GatherGodHouseData.m_idEmployee[2]	= pGatherGodHouse->IdEmployee3;
	m_GatherGodHouseData.m_idEmployee[3]	= pGatherGodHouse->IdEmployee4;

	m_GatherGodHouseData.m_idMagicBook[0]	= pGatherGodHouse->IdMagicBook1;
	m_GatherGodHouseData.m_idMagicBook[1]	= pGatherGodHouse->IdMagicBook2;
	m_GatherGodHouseData.m_idMagicBook[2]	= pGatherGodHouse->IdMagicBook3;
	m_GatherGodHouseData.m_idMagicBook[3]	= pGatherGodHouse->IdMagicBook4;

	if( m_GatherGodHouseData.m_LastFlushEmployTime == 0 && m_GatherGodHouseData.m_LastFlushMagicTime == 0){

		UINT32 nCurTime = CURRENT_TIME();

		//新建角色时，第一次默认给刷新一次
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SEmployeeDataCnfg * pEmployeeCnfg = g_pGameServer->GetConfigServer()->RandGetEmployData(enFlushType_Free);
			if( 0 == pEmployeeCnfg){
				TRACE("<error> %s : %d 行 刷新招募角色,返回的招募角色配置指针为空！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idEmployee[i]    = pEmployeeCnfg->m_EmployeeID;
		}

		m_GatherGodHouseData.m_LastFlushEmployTime = nCurTime - m_nCDTimeFreeFlush;


		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->RandGetMagicBookCnfg(enFlushType_Free);
			if( 0 == pMagicBookCnfg){
				TRACE("<error> %s : %d 行 刷新法术书,返回的配置指针为0,index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idMagicBook[i] = pMagicBookCnfg->m_MagicBookID;
		}

		m_GatherGodHouseData.m_LastFlushMagicTime = nCurTime - m_nCDTimeFreeFlush;

		//新建角色的特殊处理
		this->NewUserSpecialCal();
	}

	return true;
}

//释放
void GatherGodHousePart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart GatherGodHousePart::GetPartID(void)
{
	return enThingPart_Actor_GatherGodHouse;
}

//取得本身生物
IThing*	GatherGodHousePart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool GatherGodHousePart::OnGetDBContext(void * buf, int &nLen)
{
	if(0 == buf || nLen < sizeof(SDB_Update_GatherGodHouseData_Req))
	{
		return false;
	}

	SDB_Update_GatherGodHouseData_Req * pGatherGodHouseData = (SDB_Update_GatherGodHouseData_Req *)buf;

	if(0 == pGatherGodHouseData){
		return false;
	}

	pGatherGodHouseData->Uid_User		= m_pActor->GetUID().ToUint64();

	pGatherGodHouseData->LastFlushEmployTime	= m_GatherGodHouseData.m_LastFlushEmployTime;
	pGatherGodHouseData->LastFlushMagicTime		= m_GatherGodHouseData.m_LastFlushMagicTime;

	pGatherGodHouseData->IdEmployee1	= m_GatherGodHouseData.m_idEmployee[0];
	pGatherGodHouseData->IdEmployee2	= m_GatherGodHouseData.m_idEmployee[1];
	pGatherGodHouseData->IdEmployee3	= m_GatherGodHouseData.m_idEmployee[2];
	pGatherGodHouseData->IdEmployee4	= m_GatherGodHouseData.m_idEmployee[3];

	pGatherGodHouseData->IdMagicBook1	= m_GatherGodHouseData.m_idMagicBook[0];
	pGatherGodHouseData->IdMagicBook2	= m_GatherGodHouseData.m_idMagicBook[1];
	pGatherGodHouseData->IdMagicBook3	= m_GatherGodHouseData.m_idMagicBook[2];
	pGatherGodHouseData->IdMagicBook4	= m_GatherGodHouseData.m_idMagicBook[3];

	nLen = sizeof(SDB_Update_GatherGodHouseData_Req);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void GatherGodHousePart::InitPrivateClient()
{
}

//玩家下线了，需要关闭该ThingPart
void GatherGodHousePart::Close()
{

}

//保存数据
void  GatherGodHousePart::SaveData()
{
	SDB_Update_GatherGodHouseData_Req Req;

	int nLen = sizeof(SDB_Update_GatherGodHouseData_Req);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID), enDBCmd_UpdateGetGatherGodHouseInfo, ob.TakeOsb(),0,0);
}

//进入
void GatherGodHousePart::Enter()
{
	//默认显示招募角色界面
	SyncGatherGodData(enSellItemType_Employee);
}

//查看
void GatherGodHousePart::ViewItem(CS_ViewItem_Req & Req)
{
	SyncGatherGodData(Req.m_ItemType);
}

//刷新
void GatherGodHousePart::FlushItem(CS_FlushItem_Req & Req)
{
	time_t nCurTime = CURRENT_TIME();

	switch((int)Req.m_FlushType)
	{
	case enFlushType_Free:	//免费刷新处理
		{
			if(enSellItemType_Employee == Req.m_ItemType)
			{	
				//刷新招募角色界面
				FlushEmployeeData(enFlushType_Free);
			}
			else if(enSellItemType_Book == Req.m_ItemType) 	
			{	
				//刷新法术书界面
				FlushMagicBookData(enFlushType_Free);
			}
		}
		break;
	case enFlushType_Money: //付费刷新处理
		{
			if(enSellItemType_Employee == Req.m_ItemType)
			{	
				//刷新招募角色界面
				FlushEmployeeData(enFlushType_Money);
			}
			else if(enSellItemType_Book == Req.m_ItemType) //法术书界面
			{	
				//刷新法术书界面
				FlushMagicBookData(enFlushType_Money);
			}
		}
		break;
	}
}

//购买
void GatherGodHousePart::BuyItem(CS_BuyItem_Req & Req)
{
	switch(Req.m_ItemType)
	{
	case enSellItemType_Employee:
		{
			//购买招募角色
			BuyEmployee(Req.m_Index);
		}
		break;
	case enSellItemType_Book:
		{
			//购买法术书
			BuyMagicBook(Req.m_Index);
		}
		break;
	}
}

//获得每次付费刷新需要消耗的仙石
INT32 GatherGodHousePart::GetFlushMoney()
{
	return 10;
}

//刷新招募角色数据
void GatherGodHousePart::FlushEmployeeData(enFlushType FlushType)
{
	if( enFlushType_Free == FlushType){
		//免费刷新招募角色数据
		this->FreeFlushEmployee();
	}else if( enFlushType_Money == FlushType){
		//付费刷新招募角色数据
		this->MoneyFlushEmployee();
	}
}

//刷新法术书数据
void GatherGodHousePart::FlushMagicBookData(enFlushType FlushType)
{
	if( enFlushType_Free == FlushType){
		//免费刷新法术书数据
		this->FreeFlushMagicBook();
	}else if( enFlushType_Money == FlushType){
		//付费刷新法术书数据
		this->MoneyFlushMagicBook();
	}
}

//购买招募角色
void GatherGodHousePart::BuyEmployee(UINT8	nIndex)
{
	if( nIndex >= FLUSH_NUM){
		TRACE("<error> %s : %d 行　聚仙楼参数错误，索引为 = %d", __FUNCTION__, __LINE__, nIndex);
		return;
	}

	SC_BuyItem_Rsp Rsp;
	Rsp.m_Result = enGatherGodCode_NoItem;

	if( 0 != m_GatherGodHouseData.m_idEmployee[nIndex]){

		const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(m_GatherGodHouseData.m_idEmployee[nIndex]);

		if( 0 == pEmployeeDataCnfg){
			TRACE("<error> %s : %d 行 获取招募角色配置信息出错！！招募角色ID = %d", __FUNCTION__, __LINE__, m_GatherGodHouseData.m_idEmployee[nIndex]);
			return;
		}

		do
		{
			INT32 NeedStone = pEmployeeDataCnfg->m_Price - pEmployeeDataCnfg->m_Price * m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney) / 100.0f + 0.9999;

			if(m_pActor->GetCrtProp(enCrtProp_ActorStone) < NeedStone){
				//玩家身上的灵石不够
				Rsp.m_Result = enGatherGodCode_NoStone;
				break;
			}

			//当前的招募角色数
			UINT8 nCount = 0;
			//是否存在相同名字的角色
			bool bExit = false;
			
			for( int k = 0; k < MAX_EMPLOY_NUM; ++k)
			{
				IActor * pEmployee = m_pActor->GetEmployee(k);
				if( 0 != pEmployee){
					++nCount;

					// fly add	20121106
					if( 0 == strcmp(pEmployee->GetName(), (const char *)g_pGameServer->GetGameWorld()->GetLanguageStr(pEmployeeDataCnfg->m_NameLanguageID)))
					{
						//已有相同名字招募角色
						Rsp.m_Result = enGatherGodCode_ErrName;

						bExit = true;
						break;
					}											
//					if( 0 == strcmp(pEmployee->GetName(), (const char *)pEmployeeDataCnfg->m_szName))
//					{
						//已有相同名字招募角色
//						Rsp.m_Result = enGatherGodCode_ErrName;

//						bExit = true;
//						break;
//					}
					continue;
				}
			}

			if( bExit){
				break;
			}
			
			//if( nCount >= MAX_EMPLOY_NUM){
			//	Rsp.m_Result = enGatherGodCode_NumLimit;
			//	break;
			//}
			if ( nCount >= g_pGameServer->GetConfigServer()->GetCanEmployNum(m_pActor->GetCrtProp(enCrtProp_Level)) ){
				Rsp.m_Result = enGatherGodCode_NumLimit;
				break;			
			}

			IActor * pEmployee = g_pGameServer->GetGameWorld()->CreateEmploy(pEmployeeDataCnfg->m_EmployeeID, m_pActor);
			if( 0 == pEmployee){
				TRACE("<error> %s:%d 创建招募角色失败, EmployeeID = %d", __FUNCTION__, __LINE__, pEmployeeDataCnfg->m_EmployeeID);
				return;
			}
			
			m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -NeedStone);

			m_GatherGodHouseData.m_idEmployee[nIndex] = 0;
			Rsp.m_Result = enGatherGodCode_OK;
		}
		while(0);
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Buy,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGatherGodCode_OK == Rsp.m_Result){
		SyncGatherGodData(enSellItemType_Employee);
	}
}

//购买法术书
void GatherGodHousePart::BuyMagicBook(UINT8 nIndex)
{
	SC_BuyItem_Rsp Rsp;
	Rsp.m_Result = enGatherGodCode_NoItem;
	
	if( nIndex >= FLUSH_NUM){
		TRACE("<error> %s : %d 行　聚仙楼参数错误，索引为 = %d", __FUNCTION__, __LINE__, nIndex);
		return;
	}

	if( 0 != m_GatherGodHouseData.m_idMagicBook[nIndex]){

		const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->GetMagicBookCnfg(m_GatherGodHouseData.m_idMagicBook[nIndex]);
		if( 0 == pMagicBookCnfg){
			TRACE("<error> %s : %d 找不到法术书配置,法术书ID = %d", __FUNCTION__,__LINE__,m_GatherGodHouseData.m_idMagicBook[nIndex]);
			return;
		}

		INT32 NeedStone = pMagicBookCnfg->m_Price - pMagicBookCnfg->m_Price * m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney) / 100.0f + 0.9999;

		if( m_pActor->GetCrtProp(enCrtProp_ActorStone) < NeedStone){
			//玩家身上的灵石不够
			Rsp.m_Result = enGatherGodCode_NoStone;

		}else if( false == m_pActor->GetPacketPart()->AddGoods(pMagicBookCnfg->m_MagicBookID,1,true)){
			//背包已满
			Rsp.m_Result = enGatherGodCode_PacketFull;

		}else{

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Buy,pMagicBookCnfg->m_MagicBookID,UID(),1,"聚仙楼购买法术书");

			m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -NeedStone);

			m_GatherGodHouseData.m_idMagicBook[nIndex] = 0;
			Rsp.m_Result = enGatherGodCode_OK;
			
			//发布事件
			SS_BuyGoodsContext BuyGoodsContext;
			BuyGoodsContext.m_GoodsID	= pMagicBookCnfg->m_MagicBookID;
			BuyGoodsContext.m_Category	= enGoodsCategory_Common;
			BuyGoodsContext.m_SubClass	= enGoodsSecondType_Common;
			BuyGoodsContext.m_ThreeClass = enGoodsThreeType_MagicBook;
			BuyGoodsContext.m_MoneyType  = enMoneyType_Money;
			BuyGoodsContext.m_BuyPlaceType = enBugPlaceType_GodGather;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_BuyGoods);
			m_pActor->OnEvent(msgID,&BuyGoodsContext,sizeof(BuyGoodsContext));
		}
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Buy, sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( Rsp.m_Result == enGatherGodCode_OK){
		SyncGatherGodData(enSellItemType_Book);
	}
}

//同步聚仙楼数据
void GatherGodHousePart::SyncGatherGodData(enSellItemType SellItemType)
{
	switch(SellItemType)
	{
	case enSellItemType_Employee:
		{
			//显示剩余可招募角色数据
			SC_EmployeeData_Sync Rsp;

			Rsp.NextFlushRemainTime = 0;
			Rsp.m_PriceFlush = m_SpendMoneyFlush;
			Rsp.m_MoneyDes	 = m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney);

			for(int i = 0; i < FLUSH_NUM; ++i)
			{
				Rsp.m_EmployeeData[i].m_EmployeeID	  = m_GatherGodHouseData.m_idEmployee[i];

				const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(m_GatherGodHouseData.m_idEmployee[i]);
				if( 0 == pEmployeeDataCnfg){
					continue;
				}

				Rsp.m_EmployeeData[i].m_Price		  = pEmployeeDataCnfg->m_Price;

				//发送招募角色配置信息
				m_pActor->SendEmployeeCnfg(pEmployeeDataCnfg->m_EmployeeID);
			}
			time_t nCurTime = CURRENT_TIME();
			if(nCurTime - m_GatherGodHouseData.m_LastFlushEmployTime > m_nCDTimeFreeFlush){
				Rsp.NextFlushRemainTime = 0;
			}else{
				Rsp.NextFlushRemainTime = m_GatherGodHouseData.m_LastFlushEmployTime + m_nCDTimeFreeFlush - nCurTime;
			}

			OBuffer1k ob;
			ob << BuildingHeader(enBuildingCmd_SyncEmployee, sizeof(SC_EmployeeData_Sync)) << Rsp;
			m_pActor->SendData(ob.TakeOsb());
		
		}
		break;
	case enSellItemType_Book:
		{
			//显示剩余可购买法术书
			SC_MagicBookData_Sync Rsp;

			Rsp.NextFlushRemainTime = 0;
			Rsp.m_PriceFlush        = m_StoneFlushMagicBook;
			Rsp.m_MoneyDes			= m_pActor->GetVipValue(enVipType_BuySwordEmployMagicDesMoney);

			for(int i = 0; i < FLUSH_NUM; ++i)
			{
				Rsp.m_MagicBookData[i].m_MagicBookID   = m_GatherGodHouseData.m_idMagicBook[i];

				const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->GetMagicBookCnfg(m_GatherGodHouseData.m_idMagicBook[i]);
				if( 0 == pMagicBookCnfg){
					continue;
				}

				Rsp.m_MagicBookData[i].m_PriceStone	   = pMagicBookCnfg->m_Price;

				//发送配置数据给客户端
				m_pActor->SendGoodsCnfg(pMagicBookCnfg->m_MagicBookID);
			}

			time_t nCurTime = CURRENT_TIME();
			if(nCurTime - m_GatherGodHouseData.m_LastFlushMagicTime > m_nCDTimeFreeFlush){
				Rsp.NextFlushRemainTime = 0;
			}else{
				Rsp.NextFlushRemainTime = m_GatherGodHouseData.m_LastFlushMagicTime + m_nCDTimeFreeFlush - nCurTime;
			}

			OBuffer1k ob;
			ob << BuildingHeader(enBuildingCmd_SyncBook, sizeof(Rsp)) << Rsp;
			m_pActor->SendData(ob.TakeOsb());
		
		}
		break;
	}
}

		//获得刷新招幕人员余下时间
INT32 GatherGodHousePart::GetFlushEmployRemainTime()
{
		time_t tCurrentTime = CURRENT_TIME();
		INT32 RemainTime = m_nCDTimeFreeFlush - (tCurrentTime - m_GatherGodHouseData.m_LastFlushEmployTime );

		if(RemainTime<0)
		{
			RemainTime = 0;
		}

		return  RemainTime;
}

		//获得刷新法术书余下时间
INT32 GatherGodHousePart::GetFlushMagicBookRemainTime()
{
		time_t tCurrentTime = CURRENT_TIME();
		INT32 RemainTime = m_nCDTimeFreeFlush - (tCurrentTime - m_GatherGodHouseData.m_LastFlushMagicTime );

		if(RemainTime<0)
		{
			RemainTime = 0;
		}

		return  RemainTime;
}

//免费刷新
void	GatherGodHousePart::FreeFlushEmployee()
{
	time_t nCurTime = CURRENT_TIME();

	SC_FlushItem_Rsp Rsp; 

	if(GetFlushEmployRemainTime() == 0){
		//刷新4个人物
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SEmployeeDataCnfg * pEmployeeCnfg = g_pGameServer->GetConfigServer()->RandGetEmployData(enFlushType_Free);
			if( 0 == pEmployeeCnfg){
				TRACE("<error> %s : %d 行 刷新招募角色,返回的招募角色配置指针为空！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idEmployee[i]    = pEmployeeCnfg->m_EmployeeID;
		}

		Rsp.m_Result = enGatherGodCode_OK;
		m_GatherGodHouseData.m_LastFlushEmployTime = nCurTime;
	}else{
		Rsp.m_Result = enGatherGodCode_Time;
	}
	
	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Flush,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGatherGodCode_OK == Rsp.m_Result){
		//刷新成功则同步下
		this->SyncGatherGodData(enSellItemType_Employee);
	}
}

//付费刷新
void	GatherGodHousePart::MoneyFlushEmployee()
{
	SC_FlushItem_Rsp Rsp; 

	if( m_pActor->GetCrtProp(enCrtProp_ActorMoney) >= m_SpendMoneyFlush || m_pActor->GetCrtProp(enCrtProp_ActorTicket) >= m_SpendMoneyFlush){
		//刷新4个人物
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SEmployeeDataCnfg * pEmployeeCnfg = g_pGameServer->GetConfigServer()->RandGetEmployData(enFlushType_Money);
			if( 0 == pEmployeeCnfg){
				TRACE("<error> %s : %d 行 刷新招募角色,返回的招募角色配置指针为空！！index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idEmployee[i]    = pEmployeeCnfg->m_EmployeeID;
		}

		Rsp.m_Result = enGatherGodCode_OK;

		if( m_pActor->GetCrtProp(enCrtProp_ActorTicket) >= m_SpendMoneyFlush){
			m_pActor->AddCrtPropNum(enCrtProp_ActorTicket, -m_SpendMoneyFlush);
		}else{
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, -m_SpendMoneyFlush);

			g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), m_SpendMoneyFlush, m_pActor->GetCrtProp(enCrtProp_ActorMoney), "聚仙楼刷新招募角色");
		}

	}else{
		Rsp.m_Result = enGatherGodCode_NoMoney;
	}
	
	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Flush,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGatherGodCode_OK == Rsp.m_Result){
		//刷新成功则同步下
		this->SyncGatherGodData(enSellItemType_Employee);

		//发布购买仙剑事件
		SS_FlushGodHouse FlushGodHouse;
		FlushGodHouse.bMoney = 1;
		FlushGodHouse.bEmployee = 1;

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_FlushGodHouse);
		m_pActor->OnEvent(msgID,&FlushGodHouse,sizeof(FlushGodHouse));
	}
}

//免费刷新法术书
void	GatherGodHousePart::FreeFlushMagicBook()
{
	time_t nCurTime = CURRENT_TIME();

	SC_FlushMagicBook_Rsp Rsp;

	if(GetFlushMagicBookRemainTime()==0){
		//刷新4本法术书
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->RandGetMagicBookCnfg(enFlushType_Free);
			if( 0 == pMagicBookCnfg){
				TRACE("<error> %s : %d 行 刷新法术书,返回的配置指针为0,index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idMagicBook[i] = pMagicBookCnfg->m_MagicBookID;
		}

		Rsp.m_Result = enGatherGodCode_OK;

		m_GatherGodHouseData.m_LastFlushMagicTime = nCurTime;

	}else{
		Rsp.m_Result = enGatherGodCode_Time;
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Flush,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGatherGodCode_OK == Rsp.m_Result){
		//刷新成功则同步下
		SyncGatherGodData(enSellItemType_Book);
	}
}

//付费刷新法术书
void	GatherGodHousePart::MoneyFlushMagicBook()
{
	SC_FlushMagicBook_Rsp Rsp;

	if( m_pActor->GetCrtProp(enCrtProp_ActorStone) >= m_StoneFlushMagicBook){
		//刷新4本法术书
		for( int i = 0; i < FLUSH_NUM; ++i)
		{
			const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->RandGetMagicBookCnfg(enFlushType_Money);
			if( 0 == pMagicBookCnfg){
				TRACE("<error> %s : %d 行 刷新法术书,返回的配置指针为0,index = %d", __FUNCTION__, __LINE__, i);
				continue;
			}

			m_GatherGodHouseData.m_idMagicBook[i] = pMagicBookCnfg->m_MagicBookID;
		}

		Rsp.m_Result = enGatherGodCode_OK;

		m_pActor->AddCrtPropNum(enCrtProp_ActorStone, -m_StoneFlushMagicBook);

	}else{
		Rsp.m_Result = enGatherGodCode_NoStone;
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_Flush,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if( enGatherGodCode_OK == Rsp.m_Result){
		//刷新成功则同步下
		SyncGatherGodData(enSellItemType_Book);
	}
}

//新建角色的特殊处理
void GatherGodHousePart::NewUserSpecialCal()
{
	//玩家首次登陆游戏，聚仙楼刷新的4个角色资质分别为1.4，1.2，1.3，1.1
	const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->RandGetEmployApt(1400, enFlushType_Free);
	if( 0 == pEmployeeDataCnfg){
		TRACE("<warning> %s :  %d 行　随机获取资质1100的招募角色配置信息失败!!", __FUNCTION__, __LINE__);
	}else{
		m_GatherGodHouseData.m_idEmployee[0] = pEmployeeDataCnfg->m_EmployeeID;
	}

	if( FLUSH_NUM < 2){
		return;
	}

	pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->RandGetEmployApt(1200, enFlushType_Free);
	if( 0 == pEmployeeDataCnfg){
		TRACE("<warning> %s :  %d 行　随机获取资质1200的招募角色配置信息失败!!", __FUNCTION__, __LINE__);
	}else{
		m_GatherGodHouseData.m_idEmployee[1] = pEmployeeDataCnfg->m_EmployeeID;
	}

	if( FLUSH_NUM < 3){
		return;
	}

	pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->RandGetEmployApt(1300, enFlushType_Free);
	if( 0 == pEmployeeDataCnfg){
		TRACE("<warning> %s :  %d 行　随机获取资质1300的招募角色配置信息失败!!", __FUNCTION__, __LINE__);
	}else{
		m_GatherGodHouseData.m_idEmployee[2] = pEmployeeDataCnfg->m_EmployeeID;
	}

	if( FLUSH_NUM < 4){
		return;
	}

	pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->RandGetEmployApt(1100, enFlushType_Free);
	if( 0 == pEmployeeDataCnfg){
		TRACE("<warning> %s :  %d 行　随机获取资质1400的招募角色配置信息失败!!", __FUNCTION__, __LINE__);
	}else{
		m_GatherGodHouseData.m_idEmployee[3] = pEmployeeDataCnfg->m_EmployeeID;
	}

	////1号位置的角色必然是从角色ID为41-50之间的任意角色
	//int nRandom = RandomService::GetRandom() % (50 - 41 + 1);

	//TActorID EmployeeID = nRandom = 41 + nRandom;

	//const SEmployeeDataCnfg * pEmployeeDataCnfg = g_pGameServer->GetConfigServer()->GetEmployeeDataCnfg(EmployeeID);
	//if( 0 == pEmployeeDataCnfg){
	//	TRACE("<warning> %s : %d 行 招募角色配置信息获取失败！！，招募角色ID = %d", __FUNCTION__, __LINE__, EmployeeID);
	//}else{
	//	m_GatherGodHouseData.m_idEmployee[0] = EmployeeID;
	//}

	////1号位置的法术书必然是雷电术
	//TGoodsID MagicBookID = 10069;

	//const SMagicBookCnfg * pMagicBookCnfg = g_pGameServer->GetConfigServer()->GetMagicBookCnfg(MagicBookID);
	//if( 0 == pMagicBookCnfg){
	//	TRACE("<warning> %s : %d 行 法术书配置信息获取失败！！，法术书ID = %d", __FUNCTION__, __LINE__, EmployeeID);
	//}else{
	//	m_GatherGodHouseData.m_idMagicBook[0] = MagicBookID;
	//}
}

//自动刷新招募角色
void  GatherGodHousePart::AutoFlushEmployee(UINT16 FlushNum, INT32 Aptitude)
{
	const SAutoFlushEmployee * pAutoEmploy = g_pGameServer->GetConfigServer()->GetAutoFlushEmploy(FlushNum);

	if ( 0 == pAutoEmploy){
		
		TRACE("<error> %s : %d Line 获取不到自动刷新的配置信息！！自动刷新次数为%d", __FUNCTION__, __LINE__, FlushNum);
		return;
	}

	SC_AutoFlushEmploy Rsp;

	INT32 NeedGodStone = pAutoEmploy->m_PerGodStone * pAutoEmploy->m_FlushNum;

	if ( m_pActor->GetCrtProp(enCrtProp_ActorMoney) < NeedGodStone){
		//仙石不足
		Rsp.m_RetCode = enGatherGodCode_NoMoney_AutoFlush;
	}else{
	
		bool bOk = false;

		for ( int num = 0; num < FlushNum; ++num)
		{
			if ( bOk){
				//已刷到
				break;
			}
	
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, -pAutoEmploy->m_PerGodStone);

			for ( int i = 0; i < FLUSH_NUM; ++i)
			{
				const SEmployeeDataCnfg * pEmployCnfg = g_pGameServer->GetConfigServer()->RandGetEmployData(enFlushType_Money);

				if ( 0 == pEmployCnfg){
					
					TRACE("<error> %s : %d Line 付费获取招募角色配置信息出错！！",  __FUNCTION__, __LINE__);
					return;
				}

				m_GatherGodHouseData.m_idEmployee[i] = pEmployCnfg->m_EmployeeID;

				if ( pEmployCnfg->m_Aptitude >= Aptitude){
					
					bOk = true;
					Rsp.m_bOk = true;
					Rsp.m_AptitudeUp = pEmployCnfg->m_Aptitude;
				}
			}

			Rsp.m_RealFlushNum = num + 1;
		}

		Rsp.m_UseGodStone = Rsp.m_RealFlushNum * pAutoEmploy->m_PerGodStone;

		if ( !bOk){
			
			//没刷到指定的资质招募角色，则刷个必然出现的
			INT32 MushAptitude = g_pGameServer->GetConfigServer()->RandGetMushAptitude(FlushNum);

			const SEmployeeDataCnfg * pEmployeeCnfg = g_pGameServer->GetConfigServer()->RandGetEmployApt(MushAptitude, enFlushType_Money);
			
			if ( 0 == pEmployeeCnfg){
				
				TRACE("<error> %s : %d Line 随机获取指定资质角色配置信息出错！！资质%d", __FUNCTION__,  __LINE__, MushAptitude);
				return;
			}

			Rsp.m_AptitudeUp = pEmployeeCnfg->m_Aptitude;
			m_GatherGodHouseData.m_idEmployee[1] = pEmployeeCnfg->m_EmployeeID;
		}

		this->SyncGatherGodData(enSellItemType_Employee);
	}

	OBuffer1k ob;
	ob << BuildingHeader(enBuildingCmd_AutoFlushEmploy,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}
