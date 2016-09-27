
#include "TalismanPart.h"
#include "IActor.h"
#include "XDateTime.h"
#include "ITalisman.h"
#include "ThingServer.h"
#include "IGameWorld.h"
#include "IPacketPart.h"
#include "IEquipPart.h"
#include "IBasicService.h"
#include "IConfigServer.h"
#include "IGoodsServer.h"
#include "ITalismanGame.h"
#include "DMsgSubAction.h"


TalismanPart::TalismanPart()
{
	m_pActor = 0;

	m_GameID = 0;

	m_EnterTWNumToday = 0;

	m_LastEnterTWTime = 0;

	m_VipEnterTWNumToday = 0;

	m_LastVipEnterTWTime = 0;

	m_GetGhostSoul = 0;

	m_GetGhostTimes = 0;
}
TalismanPart::~TalismanPart()
{
}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool TalismanPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if( 0 == pMaster || 0 == pContext || nLen < sizeof(SDB_TalismanPart) )
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;

	////////////////////////////////
	//法宝世界数据
	SDB_TalismanPart * pDBTalismanPart = (SDB_TalismanPart *)pContext;

	m_GetGhostSoul = pDBTalismanPart->m_GetGhostSoul;
	m_EnterTWNumToday = pDBTalismanPart->m_EnterTWNumToday;
	m_LastEnterTWTime = pDBTalismanPart->m_LastEnterTWTime;
	m_VipEnterTWNumToday = pDBTalismanPart->m_VipEnterTWNumToday;
	m_LastVipEnterTWTime = pDBTalismanPart->m_LastVipEnterTWTime;

	//占领信息
	m_OccupationInfo.m_Floor = pDBTalismanPart->m_Floor;
	m_OccupationInfo.m_Level = pDBTalismanPart->m_Level;
	m_OccupationInfo.m_OccupationTime = pDBTalismanPart->m_OccupationTime;
	m_OccupationInfo.m_uidMonster = UID(pDBTalismanPart->m_uidMonster);

	if ( m_OccupationInfo.m_OccupationTime > 0 )
	{
		g_pThingServer->GetTalismanMgr().GetUserOccupation(m_pActor->GetUID(), m_OccupationInfo.m_Level, m_OccupationInfo.m_Floor, m_OccupationInfo.m_uidMonster, m_OccupationInfo.m_OccupationTime);

		UINT32 TimeNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_OccupationTimeNum;

		UINT32 TotalOccupationTime = 0;

		if ( CURRENT_TIME() >= m_OccupationInfo.m_OccupationTime + TimeNum )
		{
			TotalOccupationTime = TimeNum;

			//占领结束
			//g_pThingServer->GetTalismanMgr().OverOccupation(m_pActor, m_OccupationInfo.m_uidMonster, m_OccupationInfo.m_Level, m_OccupationInfo.m_Floor);
			this->OverOccupation();
		}
		else
		{
			TotalOccupationTime = CURRENT_TIME() - m_OccupationInfo.m_OccupationTime;

			//设置占领结束定时器
			g_pGameServer->GetTimeAxis()->SetTimer(eTalismanTimerID_Occupation,this,(TimeNum - TotalOccupationTime) * 1000,"TalismanPart::Create");

			//设置获取灵魄定时器
			UINT32 TimeSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_OccupationTimeSpaceGetSoul;

			g_pGameServer->GetTimeAxis()->SetTimer(eTalismanTimerID_GetSoul,this,TimeSpace * 1000,"TalismanPart::Create");
		}

		const STalismanWorldParam *  pTWParam = g_pGameServer->GetConfigServer()->GetTalismanWorldParam(pDBTalismanPart->m_Floor);

		if ( 0 == pTWParam )
		{
			TRACE("<error> %s : %d Line 获取法宝世界配置出错！！层数%d", __FUNCTION__, __LINE__, pDBTalismanPart->m_Floor);
			return false;
		}

		INT32 GetSoul = (TotalOccupationTime / 60 + 0.99999) * pTWParam->m_MinuteGetGhostSoul - m_GetGhostSoul;

		if ( GetSoul > 0 )
		{
			m_pActor->AddCrtPropNum(enCrtProp_GhostSoul, pTWParam->m_MinuteGetGhostSoul);

			m_GetGhostSoul += pTWParam->m_MinuteGetGhostSoul;
		}
	}
	////////////////////////////////

	STalismanWorldRecord * pRecord = (STalismanWorldRecord*)pContext;

	INT32 nCount = nLen/sizeof(STalismanWorldRecord);

	for(int i=0; i<nCount; i++,pRecord++)
	{
		m_mapTalismanRecord[pRecord->m_TalismanWorldID] = *pRecord;
	}

	//历编背包和装备栏，找出正在孕育的法宝
	IPacketPart * pPacketPart  = m_pActor->GetPacketPart();
	if(pPacketPart==0)
	{
		TRACE("<error> %s : %d line 背包part为空!",__FUNCTION__,__LINE__);
		return false;
	}

	INT32 nGoodsNum = pPacketPart->GetGoodsNum();

	for(int i=0; i< nGoodsNum; ++i)
	{
		IGoods * pGoods = pPacketPart->GetGoods(i);
		if(pGoods==0 || pGoods->GetGoodsClass() != enGoodsCategory_Talisman)
		{
			continue;
		}

		ITalisman * pTalisman = (ITalisman * )pGoods;

		if(pTalisman->IsSpawnGoods())
		{
			m_vectSpawnTalisman.push_back(pTalisman->GetUID());
		}
	}

	IEquipPart * pEquipPart = m_pActor->GetEquipPart();

	if(pEquipPart == 0)
	{
		TRACE("<error> %s : %d line 背包part为空!",__FUNCTION__,__LINE__);
		return false;
	}

	for(int i=0; i<MAX_LOAD_TALISMAN_NUM; i++)
	{
		IEquipment * pEquipment = pEquipPart->GetEquipByPos(i+enEquipPos_TalismanOne);
		if(pEquipment==0 || pEquipment->GetGoodsClass() != enGoodsCategory_Talisman)
		{
			continue;
		}

		ITalisman * pTalisman = (ITalisman * )pEquipment;

		if(pTalisman->IsSpawnGoods())
		{
			m_vectSpawnTalisman.push_back(pTalisman->GetUID());
		}
	}

	//注册事件,得到玩家在干什么
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);	

	m_pActor->SubscribeEvent(msgID,this,"TalismanPart::Create");

	return true;
}

//释放
void TalismanPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart TalismanPart::GetPartID(void)
{
	return enThingPart_Actor_Talisman;
}

//取得本身生物
IThing*		TalismanPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool TalismanPart::OnGetDBContext(void * buf, int &nLen) 
{
	if(buf==0 || nLen<sizeof(SDB_Update_TalismanPart_Req))
	{
		return false;
	}

	SDB_Update_TalismanPart_Req * pDBReq = (SDB_Update_TalismanPart_Req *)buf;

	pDBReq->m_UIDUser = m_pActor->GetUID().ToUint64();
	pDBReq->m_GetGhostSoul = m_GetGhostSoul;
	pDBReq->m_EnterTWNumToday = m_EnterTWNumToday;
	pDBReq->m_LastEnterTWTime = m_LastEnterTWTime;
	pDBReq->m_VipEnterTWNumToday = m_VipEnterTWNumToday;
	pDBReq->m_LastVipEnterTWTime = m_LastVipEnterTWTime;
	pDBReq->m_TLevel = m_OccupationInfo.m_Level;
	pDBReq->m_TFloor = m_OccupationInfo.m_Floor;
	pDBReq->m_uidMonster = m_OccupationInfo.m_uidMonster.ToUint64();
	pDBReq->m_OccupationTime = m_OccupationInfo.m_OccupationTime;
	//if(buf==0 || nLen<sizeof(SDBTalismanData))
	//{
	//	return false;
	//}

	//SDBTalismanData * pDBTalismanData = (SDBTalismanData *)buf;

	//STalismanWorldRecord * pRecord = pDBTalismanData->m_Records;

	//pDBTalismanData->m_RecordNum = 0;

	////nLen -= sizeof(SDBTalismanData);

	//for(MAP_TALISMAN_RECORD::iterator it = m_mapTalismanRecord.begin(); nLen >= sizeof(STalismanWorldRecord) && it !=m_mapTalismanRecord.end();++it,pRecord++)
	//{
	//	*pRecord = (*it).second;
	//	nLen -= sizeof(STalismanWorldRecord);
	//	pDBTalismanData->m_RecordNum++;
	//}


	////nLen -= sizeof(SDBTalismanData) + sizeof(STalismanWorldRecord)*pDBTalismanData->m_RecordNum;

	//nLen -= sizeof(SDBTalismanData);
	

	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void TalismanPart::InitPrivateClient()
{
	g_pThingServer->GetTalismanMgr().Cancel_WaitCombat(m_pActor->GetUID());
}


//玩家下线了，需要关闭该ThingPart
void TalismanPart::Close() 
{
}

//保存数据
void TalismanPart::SaveData()
{
	SDB_Update_TalismanPart_Req DBReq;

	int nLen = sizeof(SDB_Update_TalismanPart_Req);

	if ( !this->OnGetDBContext(&DBReq, nLen) )
	{
		TRACE("<error> %s : %d Line 保存失败！！", __FUNCTION__, __LINE__);
		return;
	}

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_Update_TalismanPart,ob.TakeOsb(),0,0);
}

			//孕育法宝物
 bool  TalismanPart::SpawnTalismanSpawnGoods(UID  uidTalisman)
 {
	 //首先判断玩家是否拥有该法宝

	 IGoods * pGoods = g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,uidTalisman);

	 SC_SpawnTalismanGoods_Rsp Rsp;

	 Rsp.m_uidTalisman = uidTalisman;
	 Rsp.m_Result = enMainUICode_OK;

	 ITalisman * pTalisman = (ITalisman *)pGoods;

	 INT16 MaxSpawnTalismanNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxSpawnTalismanNum;

	 if(pGoods==0)
	 {
		 Rsp.m_Result = enMainUICode_ErrNoGoods;
	 }
	 else if(pGoods->GetGoodsClass() != enGoodsCategory_Talisman )
	 {
		 Rsp.m_Result = enMainUICode_ErrNotTalisman;
	 }
	 else if(GetTalismanSpawnNum() >=MaxSpawnTalismanNum)
	 {
		  Rsp.m_Result = enMainUICode_ErrSpawnNumLimit;
	 }
	 else if(pTalisman->IsSpawnGoods())
	 {
		  Rsp.m_Result = enMainUICode_ErrIsSpawn;
	 }
	 else if(pTalisman->IsInTalisman()==false)
	 {
		  Rsp.m_Result = enMainUICode_ErrNotCanSpawn;
	 }
	 else if(pTalisman->SpawnGoods(m_pActor)==false)
	 {
		  Rsp.m_Result = enMainUICode_ErrSpawnFail;
	 }

	 OBuffer1k ob;

	 ob << SMainUIHeader(enMainUICmd_SpawnTalismanGoods,sizeof(Rsp)) << Rsp;

	 m_pActor->SendData(ob.TakeOsb());

	 //发布事件
	 if( enMainUICode_OK == Rsp.m_Result){
		SS_TalismanSpawn TalismanSpawn;
		TalismanSpawn.m_GoodsID = pGoods->GetGoodsID();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_TalismanSpawn);
		m_pActor->OnEvent(msgID,&TalismanSpawn,sizeof(TalismanSpawn));
	 }

	 return true;
 }



 //获得已在孕育的法宝数量
 INT32 TalismanPart::GetTalismanSpawnNum()
 {
	 INT32 nCount = 0;

	 int i=0;

	 while( i<m_vectSpawnTalisman.size())
	 {
		 ITalisman * pTalisman  = (ITalisman *) g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,m_vectSpawnTalisman[i]);
		 if(pTalisman==0 || pTalisman->IsSpawnGoods()==false)
		 {
			 m_vectSpawnTalisman.erase(m_vectSpawnTalisman.begin()+i);
			 continue;
		 }

		 nCount++;

		 ++i;
	 }
	 return nCount;
 }

		//领取法宝孕育物
bool  TalismanPart::TakeTalismanSpawnGoods(UID  uidTalisman)
{
	 IPacketPart * pPacketPart  = m_pActor->GetPacketPart();

	 //首先判断玩家是否拥有该法宝

	 IGoods * pGoods =  g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,uidTalisman);
	
	 SC_TakeTalismanGoods_Rsp Rsp;

	 Rsp.m_uidTalisman = uidTalisman;
	 Rsp.m_Result = enMainUICode_OK;

	 ITalisman * pTalisman = (ITalisman *)pGoods;

	 if(pGoods==0)
	 {
		 Rsp.m_Result = enMainUICode_ErrNoGoods;
	 }
	 else if(pGoods->GetGoodsClass() != enGoodsCategory_Talisman )
	 {
		 Rsp.m_Result = enMainUICode_ErrNotTalisman;
	 }
	 else if(pTalisman->IsSpawnGoods()==false)
	 {
		  Rsp.m_Result = enMainUICode_ErrIsNotSpawn;
	 }	 
	 else if(pPacketPart->IsFull())
	 {
		  Rsp.m_Result = enMainUICode_ErrNoPacketSpace;
	 }
	 else
	 {
			 //获得孕育物
		 int GoodsID = 0;
		  pTalisman->GetPropNum(enGoodsProp_GestateGoodsID,GoodsID);
		
		 if(pTalisman->IsEndSpawnTime()==false)
		 {
			  Rsp.m_Result = enMainUICode_ErrSpawnNoFinish;
		 }
		 else if(GoodsID == INVALID_GOODS_ID)
		 {
			  Rsp.m_Result = enMainUICode_ErrNoSpawnGoods;
		 }
		 else if(pPacketPart->AddGoods(GoodsID,1)==false)
		 {
			  Rsp.m_Result = enMainUICode_ErrNoPacketSpace;
		 }
		 else
		 {
			 g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_TalismanSpawn,GoodsID,pGoods->GetUID(),1,"领取法宝孕育物");

			 //品质点清零,及育时间
			 pTalisman->SetPropNum(m_pActor,enGoodsProp_BeginGestateTime,0);
			  pTalisman->SetPropNum(m_pActor,enGoodsProp_QualityPoint,0);
			 Rsp.m_gidGoods = GoodsID;
		 }
	 }

	 OBuffer1k ob;

	 ob << SMainUIHeader(enMainUICmd_TakeTalismanGoods,sizeof(Rsp)) << Rsp;

	 m_pActor->SendData(ob.TakeOsb());


	 return true;
}

//是否可以用该道具提升品质
bool TalismanPart::CanUpgradeQualityWithProp( ITalisman * pTalisman,TGoodsID gidProp,INT32 & QualityPoint,INT32 & CanUseNum)
{
	std::vector<TGoodsID>  vect = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_vectUpgradeTalismanQualityProp;

	for(int i=0; i<vect.size()/3; i++)
	{
		if(vect[i*3] == gidProp)
		{
			QualityPoint = vect[i*3 + 1];

			CanUseNum = vect[i*3+2];

			if(CanUseNum == 1)
			{
				//不可以使用多次，判断是否已使用了
				INT32 bUseSpawnProp = 0;

				pTalisman->GetPropNum(enGoodsProp_bUseSpawnProp,bUseSpawnProp);

				if(bUseSpawnProp==1)
				{
					//已使用了,不能再使用
					CanUseNum = 0;
				}

			}						
			return true;
		}
	}


	return false;
}


		//提升品质点
bool  TalismanPart::UpgradeTalismanQuality(UID  uidTalisman,TGoodsID gidProp)
{
	IPacketPart * pPacketPart  = m_pActor->GetPacketPart();

	INT32  QualityPoint = 0;
	
	INT32  CanUseNum = 0;

	 //首先判断玩家是否拥有该法宝

	 IGoods * pGoods =  g_pGameServer->GetGoodsServer()->GetGoodsFromPacketOrEquipPanel(m_pActor,uidTalisman);
	
	 SC_UpgradeTalismanQuality_Rsp Rsp;

	 Rsp.m_Result = enMainUICode_OK;

	 ITalisman * pTalisman = (ITalisman *)pGoods;

	 if(pGoods==0)
	 {
		 Rsp.m_Result = enMainUICode_ErrNoGoods;
	 }
	 else if(pGoods->GetGoodsClass() != enGoodsCategory_Talisman )
	 {
		 Rsp.m_Result = enMainUICode_ErrNotTalisman;
	 }
	 else if(pTalisman->IsSpawnGoods()==false)
	 {
		  Rsp.m_Result = enMainUICode_ErrIsNotSpawn;
	 }	 
	 else if(pTalisman->IsEndSpawnTime())
	 {
		  Rsp.m_Result = enMainUICode_ErrSpawnTimeEnd;
	 }
	 else if(CanUpgradeQualityWithProp(pTalisman,gidProp,QualityPoint,CanUseNum)==false)
	 {		
		 Rsp.m_Result = enMainUICode_ErrErrProp;
	 }
	 else if(CanUseNum == 0)
	 {
		  Rsp.m_Result = enMainUICode_ErrUseNumLimit;
	 }
	 else if(pPacketPart->DestroyGoods(gidProp,1)== false)
	 {
		  Rsp.m_Result = enMainUICode_ErrNoGoods;
	 }
	 else if(pTalisman->UpgradeQuality(m_pActor,QualityPoint)==false)
	 {
		  Rsp.m_Result = enMainUICode_ErrUpgradeFail;
	 }
	 else if(--CanUseNum == 0)
	 {
		 //记录已使用道具
		 pTalisman->SetPropNum(0,enGoodsProp_bUseSpawnProp,1);

		 g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Other,gidProp,UID(),1,"提升法宝品质点消耗道具");
	 }

	 OBuffer1k ob;

	 ob << SMainUIHeader(enMainUICmd_UpgradeTalismanQuality,sizeof(Rsp)) << Rsp;

	 m_pActor->SendData(ob.TakeOsb());

	 return true;
}


//玩家是否可以进入法宝世界
bool TalismanPart::CanEnterTalismanWorld(TTalismanWorldID TalismanWorldID)
{
	MAP_TALISMAN_RECORD::iterator it = m_mapTalismanRecord.find(TalismanWorldID);

	if(it == m_mapTalismanRecord.end())
	{
		return true;
	}

	STalismanWorldRecord & Record  = (*it).second;

	time_t nCurTime = CURRENT_TIME();

	if(XDateTime::GetInstance().IsSameDay(nCurTime,Record.m_LastEnterTime)==false)
	{
		Record.m_EnterNumOfDay = 0;
	}

	if(Record.m_EnterNumOfDay>=1)
	{
		return false;
	}


	return true;
}

//进入法宝世界
bool TalismanPart::EnterTalismanWorld(CS_TalismanWorldEnter_Req & Req)
{
	SC_TalismanWorldEnter_Rsp Rsp;

	Rsp.m_Result = enTalismanWorldRetCode_OK;


	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(Req.m_uidTalisman);

	ITalisman * pTalisman = (ITalisman *)pGoods;

	if(pGoods==0 || pGoods->GetGoodsClass() != enGoodsCategory_Talisman)
	{
		Rsp.m_Result = enTalismanWorldRetCode_ErrNotTalisman;
		
	}
	else if(m_GameID  != 0)
	{
		Rsp.m_Result = enTalismanWorldRetCode_ErrNotTalisman;
	}
	else
	{
		const SGoodsCnfg *  pGoodsCnfg = pTalisman->GetGoodsCnfg();

		const STalismanWorldCnfg * pTalismanWorldCnfg = g_pGameServer->GetConfigServer()->GetTalismanWorldCnfg(pGoodsCnfg->m_SuitIDOrSwordSecretID);

		if(pTalismanWorldCnfg == 0)
		{
			Rsp.m_Result = enTalismanWorldRetCode_ErrNoTalismanWorld;
		}
		else
		{
			ITalismanGame * pTalismanGame = g_pThingServer->GetTalismanMgr().CreateTalismanGame(pTalismanWorldCnfg->m_WorldType,Req.m_uidTalisman);

            if(pTalismanGame == 0)
			{
				Rsp.m_Result = enTalismanWorldRetCode_ErrNoTalismanWorld;
			}
			else if(g_pThingServer->GetTalismanMgr().AddGame(pTalismanGame)==false)
			{
				Rsp.m_Result = enTalismanWorldRetCode_ErrNoTalismanWorld;
			}
			else
			{
				m_GameID = pTalismanGame->GetGameID();
				Rsp.m_GameID = m_GameID;

				std::vector<IActor *> vectActor;

				for(  int i = 0; i < MAX_TALISMAN_GAME_ACTOR_NUM; ++i)
				{
					IActor * pActor = g_pGameServer->GetGameWorld()->FindActor(Req.m_uidActor[i]);
					if( 0 == pActor){
						return false;
					}

					vectActor.push_back(pActor);
				}
				//启动游戏
				pTalismanGame->Start(vectActor);
			}
		}
	}

	OBuffer1k ob;

	ob << TalismanWorldHeader(enTalismanWorldCmd_Enter,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());


	return true;
}

//退出法宝世界
void TalismanPart::LeaveTalismanWorld(UINT32 GameID)
{
	SC_TalismanWorldLeave_Rsp Rsp;
	Rsp.m_Result  = enTalismanWorldRetCode_OK;

	if(m_GameID != GameID)
	{
		Rsp.m_Result  = enTalismanWorldRetCode_ErrNotInTalismanWorld;
	}
	else
	{
		g_pThingServer->GetTalismanMgr().RemoveGame(m_GameID);
		m_GameID = 0;

		//回到主界面
		m_pActor->ComeBackMainScene();
		
	}

		OBuffer1k ob;

	ob << TalismanWorldHeader(enTalismanWorldCmd_Leave,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());
}


//游戏消息
void TalismanPart::GameMessage(CSC_TalismanWorldGame & Req,IBuffer & ib)
{
    if(m_GameID != Req.m_GameID)
	{
		return;
	}

	ITalismanGame * pTalismanGame = g_pThingServer->GetTalismanMgr().GetTalismanGame(m_GameID);

	if(pTalismanGame != 0)
	{
		pTalismanGame->OnMessage(m_pActor,Req.m_SubCmd,ib);
	}

}

//占领
bool TalismanPart::Occupation(UID uidMonster, UINT8	Level, UINT8 floor)
{
	m_OccupationInfo.m_Level = Level;
	m_OccupationInfo.m_Floor = floor;
 	m_OccupationInfo.m_uidMonster = uidMonster;
	m_OccupationInfo.m_OccupationTime = CURRENT_TIME();


	//设置获取灵魄定时器
	UINT32 TimeSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_OccupationTimeSpaceGetSoul;

	g_pGameServer->GetTimeAxis()->SetTimer(eTalismanTimerID_GetSoul,this,TimeSpace * 1000,"TalismanPart::Occupation");

	//最大占领1小时，设置定时器
	UINT32 TimeNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_OccupationTimeNum;

	g_pGameServer->GetTimeAxis()->SetTimer(eTalismanTimerID_Occupation,this,TimeNum * 1000,"TalismanPart::Occupation");

	return true;
}

//占领结束
void TalismanPart::OverOccupation(bool bBeaten)
{
	//法宝世界占领后多久获取灵魄
	UINT32 TimeSpace = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_OccupationTimeSpaceGetSoul;
	//法宝世界占领时长
	UINT32 TimeNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_OccupationTimeNum;
	//法宝世界占领可以获得灵魄的次数
	UINT16 GetGhostTimes = TimeNum/TimeSpace;
	if(m_GetGhostTimes < GetGhostTimes){
		this->GiveGhostSoul();
	}
	//删除定时器
	g_pGameServer->GetTimeAxis()->KillTimer(eTalismanTimerID_Occupation,this);

	g_pGameServer->GetTimeAxis()->KillTimer(eTalismanTimerID_GetSoul,this);


	//保存被玩家击败时获得的灵魄
	SDB_Update_Occupation_GhostSoul Req;

	Req.m_UidUser = m_pActor->GetUID().ToUint64();
	if(bBeaten == true){
		Req.m_GhostSoul = m_GetGhostSoul;
	}
	else{
		Req.m_GhostSoul = 0;
	}

	OBuffer1k ob1;
	ob1 << Req;
	g_pGameServer->GetDBProxyClient()->Request(RANKID, enDBCmd_UpdateOccupationGhostSoul, ob1.TakeOsb(), 0, 0);

	//先通知客户端更改页面显示
	SC_OpenTalismanWorld Rsp;

	Rsp.m_GetGhostSoul = m_GetGhostSoul;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	Rsp.m_LeftEnterNum = GameParam.m_MaxEnterTWNum - this->GetEnterTWNumToday();

	Rsp.m_VipTotalEnterNum = m_pActor->GetVipValue(enVipType_AddEnterTalismanNum);

	Rsp.m_VipLeftEnterNum = Rsp.m_VipTotalEnterNum - this->GetVipEnterTWNumToday();


	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_Open,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	m_TalismanWorldFuBen.GetPreOccupationInfo(m_pActor);
	

	//清除数据
	m_OccupationInfo.m_Level = 0;
	m_OccupationInfo.m_Floor = 0;
	m_OccupationInfo.m_uidMonster = UID();
	m_OccupationInfo.m_OccupationTime = 0;

	m_GetGhostTimes = 0;
	m_GetGhostSoul = 0;
}

//得到占领信息
const OccupationInfo * TalismanPart::GetOccupationInfo()
{
	return &m_OccupationInfo;
}

//掉线，下线要做的一些事
void  TalismanPart::LeaveDoSomeThing()
{
	g_pThingServer->GetTalismanMgr().Cancel_WaitCombat(m_pActor->GetUID());
}

void  TalismanPart::OnTimer(UINT32 timerID)
{
	if ( eTalismanTimerID_Occupation == timerID )
	{
		//结束占领
		g_pThingServer->GetTalismanMgr().OverOccupation(m_pActor, m_OccupationInfo.m_uidMonster, m_OccupationInfo.m_Level, m_OccupationInfo.m_Floor);
	}
	else if ( eTalismanTimerID_GetSoul == timerID )
	{
		//获取灵魄
		this->GiveGhostSoul();
	}
}

//检测法宝世界次数
bool TalismanPart::Check_EnterTalismanWorldNum()
{
	UINT8 MaxEnterNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxEnterTWNum + m_pActor->GetVipValue(enVipType_AddEnterTalismanNum);

	if ( this->GetEnterTWNumToday() + this->GetVipEnterTWNumToday()>= MaxEnterNum )
		return false;

	return true;
}

//打开法宝世界
void TalismanPart::OpenTalismanWorld()
{

	SC_OpenTalismanWorld Rsp;

	Rsp.m_GetGhostSoul = m_GetGhostSoul;

	const SGameConfigParam & GameParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	Rsp.m_LeftEnterNum = GameParam.m_MaxEnterTWNum - this->GetEnterTWNumToday();

	Rsp.m_VipTotalEnterNum = m_pActor->GetVipValue(enVipType_AddEnterTalismanNum);

	Rsp.m_VipLeftEnterNum = Rsp.m_VipTotalEnterNum - this->GetVipEnterTWNumToday();


	if ( !m_OccupationInfo.m_uidMonster.IsValid() ){
		Rsp.m_OccupatLeftTime = 0;
	} else {
		Rsp.m_OccupatLeftTime = m_OccupationInfo.m_OccupationTime + GameParam.m_OccupationTimeNum - CURRENT_TIME();

		const STalismanWorldInfo * pTWInfo = g_pGameServer->GetConfigServer()->GetTalismanWorldInfo(m_OccupationInfo.m_Level, m_OccupationInfo.m_Floor);

		if ( 0 == pTWInfo )
		{
			TRACE("<error> %s : %d Line 获取法宝世界配置失败！！级别%d,层数%d", __FUNCTION__, __LINE__, m_OccupationInfo.m_Level, m_OccupationInfo.m_Floor);
			return;
		}

		const SMapConfigInfo * pMapConfig = g_pGameServer->GetConfigServer()->GetMapConfigInfo(pTWInfo->m_MapID);

		if( 0 == pMapConfig)
		{
			TRACE("<error> %s ; %d 行 获取地图配置数据出错!!地图ID = %d", __FUNCTION__, __LINE__, pTWInfo->m_MapID);
			return;
		}

		strncpy(Rsp.m_PlaceName, g_pGameServer->GetGameWorld()->GetLanguageStr(pMapConfig->m_MapLanguageID), sizeof(Rsp.m_PlaceName));
	}
	
	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_Open,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//得到今天进入法宝世界次数
UINT8	TalismanPart::GetEnterTWNumToday()
{
	if ( !XDateTime::GetInstance().IsSameDay(m_LastEnterTWTime,CURRENT_TIME()) ){
		m_EnterTWNumToday = 0;
	}

	return m_EnterTWNumToday;	
}

//得到今天VIP进入法宝世界次数
UINT8	TalismanPart::GetVipEnterTWNumToday()
{
	if ( !XDateTime::GetInstance().IsSameDay(m_LastVipEnterTWTime,CURRENT_TIME()) ){
		m_VipEnterTWNumToday = 0;
	}

	return m_VipEnterTWNumToday;
}

//增加进入法宝世界次数
void	TalismanPart::AddEnterNum()
{
	UINT8 MaxEnterNum = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_MaxEnterTWNum;

	if ( this->GetEnterTWNumToday() < MaxEnterNum )
	{
		++m_EnterTWNumToday;
		m_LastEnterTWTime = CURRENT_TIME();
	}
	else if ( this->GetVipEnterTWNumToday() < m_pActor->GetVipValue(enVipType_AddEnterTalismanNum) )
	{
		++m_VipEnterTWNumToday;
		m_LastVipEnterTWTime = CURRENT_TIME();
	}
}

//设置下的法宝世界场景
void	TalismanPart::SetTWSceneID(TSceneID SceneID)
{
	m_TWSceneID = SceneID;
}

void	TalismanPart::OnEvent(XEventData & EventData)
{
	if ( 0 == EventData.m_pContext )
		return;

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_Get_ActorDoing);

	if ( msgID == EventData.m_MsgID )
	{
		SS_Get_ActorDoing * pActorDoing = (SS_Get_ActorDoing *)EventData.m_pContext;

		if ( m_pActor->GetCrtProp(enCrtProp_SceneID) == m_TWSceneID.ToID() )
		{
			pActorDoing->m_ActorDoing = enActorDoing_Talisman;
		}
	}
}

//清除法宝世界的次数
void	TalismanPart::ClearTWEnterNum()
{
	m_VipEnterTWNumToday = 0;
	m_EnterTWNumToday = 0;
}

//获取灵魄
void	TalismanPart::GiveGhostSoul()
{
	const STalismanWorldParam *  pTWParam = g_pGameServer->GetConfigServer()->GetTalismanWorldParam(m_OccupationInfo.m_Floor);

	if ( 0 == pTWParam )
	{
		TRACE("<error> %s : %d Line 获取法宝世界配置出错！！层数%d", __FUNCTION__, __LINE__, m_OccupationInfo.m_Floor);
		return;
	}

	m_pActor->AddCrtPropNum(enCrtProp_GhostSoul, pTWParam->m_MinuteGetGhostSoul);

	m_GetGhostSoul += pTWParam->m_MinuteGetGhostSoul;
	m_GetGhostTimes ++ ;

	//通知客户端获得灵魄
	SC_OccupatGetSoul Rsp;

	Rsp.m_GetGhostSoul = m_GetGhostSoul;

	OBuffer1k ob;
	ob << TalismanWorldHeader(enTalismanWorldCmd_SC_OccupatGetSoul,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}
