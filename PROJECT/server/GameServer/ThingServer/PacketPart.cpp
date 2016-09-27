
#include "IActor.h"

#include "IGameServer.h"
#include "PacketPart.h"

#include "DBProtocol.h"
#include "IGameWorld.h"
#include "IBasicService.h"
#include "ThingServer.h"
#include "IConfigServer.h"
#include "IEquipPart.h"
#include "IGoodsServer.h"
#include "DMsgSubAction.h"
#include "IGodSword.h"

PacketPart::PacketPart()
{
	m_pActor   = 0;     //玩家
	m_Capacity = 0;    //容量

}
PacketPart::~PacketPart()
{
	MAP_SET::iterator iter = m_setGoods.begin();
	for( ; iter != m_setGoods.end(); ++iter)
	{
		const UID & uidGoods = *iter;
		if(uidGoods.IsValid())
		{
			g_pGameServer->GetGameWorld()->DestroyThing(uidGoods);
		}
	}

}


//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool PacketPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster == 0 || pContext == 0 || nLen < sizeof(SDB_Get_PacketData_Rsp))
	{
		return false;
	}

	m_pActor = (IActor*)pMaster;


	SDBPacketData * pDBPacketData = (SDBPacketData*)pContext;

	SDB_Get_PacketData_Rsp * pPacketData = (SDB_Get_PacketData_Rsp *)pDBPacketData->m_pData;

	m_Capacity = pPacketData->Capacity;    //容量

	const SGameConfigParam  & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	if(m_Capacity < ConfigParam.m_PacketInitCapacity)
	{
		m_Capacity = ConfigParam.m_PacketInitCapacity;
	}

	int nNum = (pDBPacketData->m_nLen - sizeof(SDB_Get_PacketData_Rsp)) / sizeof(SDB_Get_PacketGoodsData_Rsp);

	if( nNum < 1){
		return true;
	}

	SDB_Get_PacketGoodsData_Rsp * pGoodsData = (SDB_Get_PacketGoodsData_Rsp *)(pPacketData + 1);

	for( int i = 0; i < nNum; ++i,++pGoodsData)
	{
		if( pGoodsData->GoodsID == INVALID_GOODS_ID){
			continue;
		}

		SDBGoodsData DBGoodsData;
		DBGoodsData.m_Binded	 = pGoodsData->Binded;
		DBGoodsData.m_CreateTime = pGoodsData->CreateTime;
		DBGoodsData.m_GoodsID    = pGoodsData->GoodsID;
		DBGoodsData.m_Location   = pGoodsData->m_Location;
		DBGoodsData.m_Number     = pGoodsData->Number;
		DBGoodsData.m_uidGoods   = UID(pGoodsData->uidGoods);

		memcpy(&DBGoodsData.m_TalismanProp, pGoodsData->GoodsData, sizeof(DBGoodsData.m_TalismanProp));

		m_setGoodsCompare.insert(UID(pGoodsData->uidGoods));

		IGoods * pGoods = (IGoods*)g_pGameServer->GetGameWorld()->CreateThing(enThing_Class_Goods,INVALID_SCENE_ID,(char*)&DBGoodsData,sizeof(SDBGoodsData),THING_CREATE_FLAG_DB_DATA);

		if(0 == pGoods)
		{
			TRACE("<error> %s : %d line 创建物品失败, goodsid = %d", __FUNCTION__,__LINE__,pGoodsData->GoodsID);
			continue;
		}

		//不需要保存到数据库,有变更时才保存
		pGoods->SetUpdate(false);

		this->__AddGoods(pGoods, false);
		//m_setGoods.insert(pGoods->GetUID());
	}

	return true;
}

//释放
void PacketPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart PacketPart::GetPartID(void)
{
	return enThingPart_Actor_Packet;
}

//取得本身生物
IThing*		PacketPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool PacketPart::OnGetDBContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SDB_UpdatePacketPartReq))
	{
		return false;
	}

	SDB_UpdatePacketPartReq * pDBPacketData = (SDB_UpdatePacketPartReq *)buf;

	pDBPacketData->m_Capacity = m_Capacity;

	pDBPacketData->m_GoodsNum = m_setGoods.size();

	pDBPacketData->m_uidUser = m_pActor->GetUID().ToUint64();

	nLen = sizeof(SDB_Update_PacketData_Req);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void PacketPart::InitPrivateClient()
{
	//同步数据
	const SGameConfigParam  & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	int nExtendNum = GetExtendNum();


	SC_PacketData_Sync Rsp;
	Rsp.m_Capacity = m_Capacity;
	Rsp.m_CanExtend = nExtendNum<ConfigParam.m_vectExtendCharge.size();
	if(Rsp.m_CanExtend)
	{
		Rsp.m_Charges = ConfigParam.m_vectExtendCharge[nExtendNum];
	}
	Rsp.m_GoodsNum = 0;

	OBuffer4k obGoods;

	for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end(); ++it)
	{
		UID uidGoods = *it;

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(uidGoods);
		if(pGoods == 0)
		{
			continue;
		}

		//通知客户端创建
		//if(m_pActor->NoticClientCreatePublicThing(uidGoods)==false)
		if(m_pActor->NoticClientCreatePrivateThing(uidGoods) == false)
		{
			continue;
		}

		Rsp.m_GoodsNum++; 

		obGoods << uidGoods;
	}

	OBuffer4k ob;

	ob << Packet_Header(enPacketCmd_Sync,sizeof(Rsp)+obGoods.Size()) << Rsp;

	ob.Push(obGoods.Buffer(),obGoods.Size());


	m_pActor->SendData(ob.TakeOsb());

}


//玩家下线了，需要关闭该ThingPart
void PacketPart::Close()
{
}

//保存数据
void PacketPart::SaveData()
{
	//保存PART数据
	SDB_UpdatePacketPartReq Req;

	int nLen = sizeof(SDB_Save_GoodsReq);

	if( false == this->OnGetDBContext(&Req, nLen)){
		return;
	}

	OBuffer1k ob;
	ob << Req;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdatePacketPacketPart,ob.TakeOsb(),0,0);

	//保存单个物品数据
	//先比较,删除物品
	MAP_SET::iterator iter = m_setGoods.begin();
	for( ; iter != m_setGoods.end(); ++iter)
	{
		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(*iter);
		if( 0 == pGoods){
			continue;
		}

		MAP_SET::iterator itCompare = m_setGoodsCompare.find(*iter);
		if( itCompare == m_setGoodsCompare.end()){
			//新加入的,要更新
			this->SaveGoodsToDB(pGoods);
			continue;
		}

		if( pGoods->GetNeedUpdate()){
			//新加入的,要更新
			this->SaveGoodsToDB(pGoods);
		}

		m_setGoodsCompare.erase(itCompare);
	}

	iter = m_setGoodsCompare.begin();
	for( ; iter != m_setGoodsCompare.end(); ++iter)
	{
		//剩下的删除
		SDB_Delete_GoodsReq DeleteEquip;
		DeleteEquip.m_uidUser = m_pActor->GetUID().ToUint64();
		DeleteEquip.m_uidGoods = (*iter).ToUint64();
		DeleteEquip.m_Pos	  = 0;

		OBuffer1k ob;
		ob << DeleteEquip;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteGoods,ob.TakeOsb(),0,0);		
	}

	m_setGoodsCompare.clear();

	iter = m_setGoods.begin();
	for( ; iter != m_setGoods.end(); ++iter)
	{
		m_setGoodsCompare.insert(*iter);
	}
}

bool PacketPart::IsFull()
{
	return m_setGoods.size() == m_Capacity;
}

//获得背包物品
IGoods * PacketPart::GetGoods(UID uidGoods)
{
	if(m_setGoods.find(uidGoods) == m_setGoods.end())
	{
		return 0;
	}
	return g_pGameServer->GetGameWorld()->GetGoods(uidGoods);
}

//获得背包数量
INT32    PacketPart::GetGoodsNum()
{
	return m_setGoods.size();
}

//获得背包当前容量
INT32   PacketPart::GetCapacity()
{
	return this->m_Capacity;
}

//获得空格数
INT32 PacketPart::GetSpace()
{
	return m_Capacity - m_setGoods.size();
}

//获得背包指定物品的数量
INT32 PacketPart::HaveGoodsNum(TGoodsID GoodsID)
{
	INT32 nNum = 0;

	for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end(); ++it)
	{
		IGoods * pGoods = GetGoodsFromGameWorld(*it);
		if(pGoods && pGoods->GetGoodsID() == GoodsID)
		{
			nNum += pGoods->GetNumber();
		}
	}

	return nNum;
}

//判断能否增加
bool PacketPart::CanAddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded)
{
	const SGoodsCnfg* pGoodsCnfg =  g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	if(pGoodsCnfg == 0)
	{
		TRACE("<error> %s : %d 不能获取物品配置信息,GoodsID=%d", __FUNCTION__,__LINE__,GoodsID);
		return false;
	}

	//叠加上限
	INT32 nPileLimit = pGoodsCnfg->m_PileNum < 1 ? 1 : pGoodsCnfg->m_PileNum;

	//背包空格数
	INT32 Space = GetSpace();

	if((nNum+nPileLimit-1)/nPileLimit <= Space)
	{
		return true;
	}

	//需要空格数
	INT32 NeedSpace = nNum /nPileLimit;

	if(NeedSpace > Space)
	{
		return false;
	}

	//余下可以叠加到其他物品部份
	INT32 Left = nNum%nPileLimit;

	//是否获取绑定
	if( pGoodsCnfg->m_BindType == enBindType_Get)
	{
		bBinded=true;		
	}

	IGameWorld * pGameWorld = g_pGameServer->GetGameWorld();
	//偿试叠加
	for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end(); ++it)
	{
		IGoods * pGoods = pGameWorld->GetGoods((*it));

		if(pGoods==0 || pGoods->GetGoodsID()!=GoodsID)
		{
			continue;
		}

		if((pGoods->IsBinded()==bBinded) && pGoods->GetNumber()<nPileLimit)
		{
			if(pGoods->GetNumber()+Left<=nPileLimit)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}

//获得背包物品
IGoods * PacketPart::GetGoods(INT32 Index)
{
	if(Index>=m_setGoods.size())
	{
		return 0;
	}

	MAP_SET::iterator it = m_setGoods.begin();

	std::advance(it,Index);

	if(it != m_setGoods.end())
	{
		return GetGoods((*it));
	}

	return 0;

}

//出售物品
void PacketPart::SellGoods(UID uidGoods, INT32 nNum)
{
	IGoods * pGoods = this->GetGoods(uidGoods);
	if( 0 == pGoods){
		return;
	}

	TGoodsID GoodsID= pGoods->GetGoodsID();

	SC_SellGoods_Rsp Rsp;

	if( nNum > pGoods->GetNumber()){
		Rsp.m_Result = enPacketRetCode_NoNum;
	}else if(pGoods->GetGoodsCnfg()->m_GoodsLevel == 0){
		//该物品无法出售
		Rsp.m_Result = enPacketRetCode_ErrNotSell;
	}else{
		//获得出售价格
		INT32  SellPrice = nNum * pGoods->GetGoodsCnfg()->m_SellPrice;

		if( pGoods->GetNumber() == nNum){
			this->DestroyGoods(uidGoods);
		}else{
			//删除指定个数
			this->DestroyGoods(uidGoods, nNum);
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Sell,GoodsID,uidGoods,nNum,"出售物品给系统");

		m_pActor->AddCrtPropNum(enCrtProp_ActorStone, SellPrice);

		Rsp.m_Price = SellPrice;
	}

	OBuffer1k ob;
	ob << Packet_Header(enPacketCmd_SellGoods,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());
}

//是否可以批量增加物品
bool PacketPart::CanAddGoods(const SAddGoodsInfo * pGoodsInfo,INT32 nNum) 
{
	if(pGoodsInfo ==0 || nNum==0)
	{
		TRACE("<error> %s : %d line 需要增加的物品数量为零!",__FUNCTION__,__LINE__);
		return true;
	}
	//先把同类物品加在一起

	std::vector<SAddGoodsInfo> vect;

	for(int i=0; i<nNum; i++)
	{
		bool bBinded = pGoodsInfo[i].m_bBinded;
		if(bBinded == false)
		{
			//是否该物品为获取绑定
			const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(pGoodsInfo[i].m_GoodsID);

			if(pGoodsCnfg==0)
			{
				TRACE("<error> %s : %d line 找不到物品配置信息GoodsID=%d !",__FUNCTION__,__LINE__,pGoodsInfo[i].m_GoodsID);
				continue;
			}

			if(pGoodsCnfg->m_BindType == enBindType_Get)
			{
				bBinded = true;
			}
		}

		bool bOk = false;

		for(int j=0; j<vect.size();j++)
		{
			if(vect[j].m_GoodsID == pGoodsInfo[i].m_GoodsID
				&& vect[j].m_bBinded == bBinded)
			{
				vect[j].m_nNum += pGoodsInfo[i].m_nNum;
				bOk = true;
				break;
			}
		}

		if( !bOk){
			vect.push_back(pGoodsInfo[i]);
		}
	}

	INT32 ReservedSpace = 0;

	for(int i=0; i<vect.size();i++)
	{
		if(CanAddGoods(vect[i].m_GoodsID,vect[i].m_nNum,vect[i].m_bBinded,ReservedSpace)==false)
		{
			return false;
		}
	}

	return true;
}

//是否可以批量增加物品,需要预留空间
bool  PacketPart::CanAddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded,INT32 & ReservedSpace)
{
	const SGoodsCnfg* pGoodsCnfg =  g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	if(pGoodsCnfg == 0)
	{
		TRACE("<error> %s : %d 不能获取物品配置信息,GoodsID=%d", __FUNCTION__,__LINE__,GoodsID);
		return false;
	}

	//叠加上限
	INT32 nPileLimit = pGoodsCnfg->m_PileNum < 1 ? 1 : pGoodsCnfg->m_PileNum;

	//背包空格数
	INT32 Space = GetSpace();

	//需要的空间
	INT32 nNeedSpace = 0;

	if(nPileLimit>1) //可叠加
	{
		//查找是否有同类的物品存在(GoodsID和绑定性质都相同)
		for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end();++it)
		{
			IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods((*it));
			if(pGoods==0 
				|| pGoods->GetGoodsID() != GoodsID
				)
			{
				continue;
			}

			INT32 bGoodsBinded = 0;

			pGoods->GetPropNum(enGoodsProp_Bind,bGoodsBinded);
			if((bool)bGoodsBinded != bBinded)
			{
				continue;
			}

			INT32 nNumber = pGoods->GetNumber();
			if(nNumber<nPileLimit)
			{
				nNum -= nPileLimit-nNumber;
				if(nNum <= 0)
				{
					return true;
				}				
				break;
			}			
		}
	
		//计算需要空间
		nNeedSpace = (nNum+nPileLimit-1)/nPileLimit;
		
	}
	else
	{
		nNeedSpace = nNum;
	}	

	ReservedSpace += nNeedSpace;

	if(Space < ReservedSpace)
	{
		return false;
	}

	return true;
}

//是否可以批量增加物品，vectGoods格式为:GoodsID,nNum,GoodsID,nNum,.......
bool PacketPart::CanAddGoods(const std::vector<UINT16> & vectGoods)
{
	std::vector<SAddGoodsInfo> vect;

	for(int i=0; i < vectGoods.size() /2 ; i++)
	{
		SAddGoodsInfo Info;
		Info.m_GoodsID = vectGoods[i*2];
		Info.m_nNum   = vectGoods[i*2+1];

		vect.push_back(Info);
	}

	if(vect.size()==0)
	{
		TRACE("<error> %s : %d line 增加物品为空!",__FUNCTION__,__LINE__);
		return false;
	}

	return CanAddGoods(&vect[0],vect.size());
}

//重置所有背包中的仙剑副本进度
void  PacketPart::ResetGodSwordFuBen()
{
	MAP_SET::iterator iter = m_setGoods.begin();

	int nKillNpcNum = 0;
	int nFuBenFloor = 0;

	for( ; iter != m_setGoods.end(); ++iter)
	{
		IGoods * pGoods = this->GetGoods(*iter);
		if( 0 == pGoods){
			continue;
		}

		if( pGoods->GetGoodsClass() != enGoodsCategory_GodSword){
			continue;
		}

		IGodSword * pGodSword = (IGodSword *)pGoods;

		pGodSword->GetPropNum(enGoodsProp_KillNpcNum, nKillNpcNum);
		if( nKillNpcNum > 0){
			//设为0
			pGodSword->SetPropNum(m_pActor, enGoodsProp_KillNpcNum, 0);
			nKillNpcNum = 0;
		}

		pGodSword->GetPropNum(enGoodsProp_FuBenLevel, nFuBenFloor);
		if( nFuBenFloor > 0){
			//设为0
			pGodSword->SetPropNum(m_pActor, enGoodsProp_FuBenLevel, 0);
			nFuBenFloor = 0;
		}
	}
}

//增加物品到背包
bool PacketPart::AddGoods(UID uidGoods,bool bNotifyClientCreate /*=true*/)
{
	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(uidGoods);

	if(pGoods==0)
	{
		return false;
	}

	TGoodsID GoodsID = pGoods->GetGoodsID();
	INT32 nNum = pGoods->GetNumber();
	bool goodsBind = pGoods->IsBinded();

	if(CanAddGoods(GoodsID,nNum,goodsBind)==false)
	{
		return false;
	}

	const SGoodsCnfg* pGoodsCnfg = pGoods->GetGoodsCnfg();

	//判断是否需要绑定
	if(pGoodsCnfg->m_BindType == enBindType_Get)
	{
		goodsBind = true;
	}

	//叠加上限
	INT32 nPileLimit = pGoodsCnfg->m_PileNum < 1 ? 1 : pGoodsCnfg->m_PileNum;

	//先考虑能否叠加
	if(nPileLimit>1)
	{		
		IGameWorld * pGameWorld = g_pGameServer->GetGameWorld();

		for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end();++it)
		{
			IGoods * pGoods2 = pGameWorld->GetGoods((*it));
			if(pGoods2 == 0 ||pGoods2->GetGoodsID()!=GoodsID)
			{
				continue;
			}

			INT32 Number =  pGoods2->GetNumber();

			if(pGoods2->IsBinded()==goodsBind && Number<nPileLimit)
			{
				INT32 size = std::min(nPileLimit-Number,nNum);
				pGoods2->SetPropNum(m_pActor,enGoodsProp_Number,Number+size);
				nNum -= size;

				this->SaveGoodsToDB(pGoods2);

				if(nNum==0)
				{
					break;
				}

				pGoods->AddPropNum(m_pActor, enGoodsProp_Number, -nNum);
			}
		}
	}

	if(nNum>0)
	{
		//if(m_setGoods.insert(uidGoods).second == false)
		//{
		//	return false;
		//}
		if ( !this->__AddGoods(pGoods, true) )
			return false;

		if(bNotifyClientCreate)
		{
			m_pActor->NoticClientCreatePrivateThing(pGoods->GetUID());
		}

		SC_PacketAddGoods Rsp;

		Rsp.m_uidGoods = uidGoods;

		OBuffer1k ob;

		ob << Packet_Header(enPacketCmd_AddGoods,sizeof(Rsp)) << Rsp;

		m_pActor->SendData(ob.TakeOsb());

		if(pGoods->GetNumber() != nNum)
		{
			pGoods->SetPropNum(m_pActor,enGoodsProp_Number,nNum);
		}

		if(goodsBind && pGoods->IsBinded() != goodsBind)
		{
			pGoods->SetPropNum(m_pActor,enGoodsProp_Bind,true);
		}
	}

	return true;
}


//增加物品到背包
bool  PacketPart::AddGoods(TGoodsID GoodsID,INT32 nNum,bool bBinded)
{
	if(CanAddGoods(GoodsID,nNum,bBinded)==false)
	{
		return false;
	}

	const SGoodsCnfg* pGoodsCnfg =  g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	//叠加上限
	INT32 nPileLimit = pGoodsCnfg->m_PileNum < 1 ? 1 : pGoodsCnfg->m_PileNum;

	//判断是否需要绑定
	if(pGoodsCnfg->m_BindType == enBindType_Get)
	{
		bBinded = true;
	}

	//先考虑能否叠加
	if(nPileLimit>1)
	{		
		IGameWorld * pGameWorld = g_pGameServer->GetGameWorld();

		for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end();++it)
		{
			IGoods * pGoods2 = pGameWorld->GetGoods((*it));
			if(pGoods2 == 0 ||pGoods2->GetGoodsID()!=GoodsID)
			{
				continue;
			}

			INT32 Number =  pGoods2->GetNumber();

			if(pGoods2->IsBinded()==bBinded && Number<nPileLimit)
			{
				INT32 size = std::min(nPileLimit-Number,nNum);
				pGoods2->SetPropNum(m_pActor,enGoodsProp_Number,Number+size);
				nNum -= size;

				this->SaveGoodsToDB(pGoods2);

				if(nNum==0)
				{
					break;
				}
			}
		}
	}

	SCreateGoodsContext  GoodsCnt;
	GoodsCnt.m_Binded = bBinded;
	GoodsCnt.m_GoodsID = GoodsID;
	GoodsCnt.m_Binded = bBinded;

	while(nNum>0)
	{
		int size = std::min(nNum,nPileLimit);
		GoodsCnt.m_Number = size;

		IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(GoodsCnt);

		if(pGoods==0)
		{
			return false;
		}

		//if(m_setGoods.insert(pGoods->GetUID()).second == false)
		if( !this->__AddGoods(pGoods, true) )
		{
			g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
			return false;
		}

		//通知客户端创建
		m_pActor->NoticClientCreatePrivateThing(pGoods->GetUID());

		nNum -= size;

		SC_PacketAddGoods Rsp;

		Rsp.m_uidGoods = pGoods->GetUID();

		OBuffer1k ob;

		ob << Packet_Header(enPacketCmd_AddGoods,sizeof(Rsp)) << Rsp;

		m_pActor->SendData(ob.TakeOsb());
	}


	return true;
}
//从背包移除物品
bool PacketPart::RemoveGoods(UID uidGoods)
{
	MAP_SET::iterator it = m_setGoods.find(uidGoods);
	if(it == m_setGoods.end())
	{
		return false;
	}

	m_setGoods.erase(it);	

	SC_PacketRemoveGoods Rsp;
	Rsp.m_uidGoods = uidGoods;

	OBuffer1k ob;

	ob << Packet_Header(enPacketCmd_RemoveGoods,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	//从数据库移除
	this->DBDeleteGoods(uidGoods);

	return true;
}

//销毁物品
bool PacketPart::DestroyGoods(UID uidGoods) 
{
	if(RemoveGoods(uidGoods)==false)
	{
		return false;
	}

	g_pGameServer->GetGameWorld()->DestroyThing(uidGoods);

	return true;
}

//销毁物品,返回销毁数量
INT32 PacketPart::__DestroyGoods(const SGoodsCnfg* pGoodsCnfg,INT32 nNum,bool bBinded)
{
	INT32 old_num = nNum;

	//叠加上限
	INT32 nPileLimit = pGoodsCnfg->m_PileNum < 1 ? 1 : pGoodsCnfg->m_PileNum;

	if(nPileLimit>1) //可叠加
	{
		//叠加未满的物品
		for(MAP_SET::iterator it = m_setGoods.begin();it != m_setGoods.end() && nNum>0; )
		{
			IGoods * pGoods = GetGoodsFromGameWorld(*it);
			if(pGoods != 0  && pGoods->GetGoodsID()==pGoodsCnfg->m_GoodsID)
			{
				if(bBinded==false || pGoods->IsBinded())
				{
					INT32 Number = pGoods->GetNumber();
					if(Number < nPileLimit || nNum>nPileLimit)
					{
						if(nNum >=Number)
						{
							//整个销毁
							//MAP_SET::iterator itDel = it;
							UID uidGoods = *it;

							++it;

							this->DestroyGoods(uidGoods);
							//m_setGoods.erase(itDel);

							//this->DBDeleteGoods(uidGoods);

							////要通知客户端，物品销毁了
							//SC_PacketRemoveGoods Rsp;
							//Rsp.m_uidGoods = uidGoods;

							//OBuffer1k ob;
							//ob << Packet_Header(enPacketCmd_RemoveGoods,sizeof(Rsp)) << Rsp;
							//m_pActor->SendData(ob.TakeOsb());
						
							//g_pGameServer->GetGameWorld()->DestroyThing(uidGoods);
							nNum -= Number;									
							continue;
						}
						else
						{
							pGoods->SetPropNum(m_pActor,enGoodsProp_Number,Number-nNum);

							this->SaveGoodsToDB(pGoods);

							return old_num;
						}
					}					
				}
			}

			++it;
		}	
	}

	for(MAP_SET::iterator it = m_setGoods.begin();it != m_setGoods.end() && nNum>0; )
	{
		IGoods * pGoods = GetGoodsFromGameWorld(*it);
		if(pGoods != 0 && pGoods->GetGoodsID()==pGoodsCnfg->m_GoodsID)
		{
			if(bBinded==false || pGoods->IsBinded())
			{
				INT32 Number = pGoods->GetNumber();
				if(nNum >=Number)
				{
					//整个销毁
					//MAP_SET::iterator itDel = it;

					UID uidGoods = *it;

					++it;
					/*m_setGoods.erase(itDel);*/

					this->DestroyGoods(uidGoods);

					//this->DBDeleteGoods(uidGoods);

					////要通知客户端，物品销毁了
					//SC_PacketRemoveGoods Rsp;
					//Rsp.m_uidGoods = uidGoods;

					//OBuffer1k ob;
					//ob << Packet_Header(enPacketCmd_RemoveGoods,sizeof(Rsp)) << Rsp;
					//m_pActor->SendData(ob.TakeOsb());
				
					//g_pGameServer->GetGameWorld()->DestroyThing(uidGoods);
					nNum -= Number;									
					continue;
				}
				else
				{
					pGoods->SetPropNum(m_pActor,enGoodsProp_Number,Number-nNum);

					this->SaveGoodsToDB(pGoods);

					return old_num;
				}
			}
		}

		++it;
	}


	return old_num - nNum;

}


//销毁物品
bool PacketPart::DestroyGoods(TGoodsID GoodsID,INT32 nNum)
{	
	if(HaveGoods(GoodsID,nNum)==false)
	{
		return false;
	}

	const SGoodsCnfg* pGoodsCnfg =  g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);

	nNum -= __DestroyGoods(pGoodsCnfg,nNum,true);

	if(nNum>0)
	{
		nNum -= __DestroyGoods(pGoodsCnfg,nNum,false);
	}


	return (nNum==0);
}

//扩充背包容量
bool PacketPart::ExtendCapacity()
{
	SC_PacketExtend_Rsp Rsp;

	Rsp.m_Result = enPacketRetCode_OK;

	const SGameConfigParam  & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	int nExtendNum = GetExtendNum();

	//判断是否还可以扩展
	if(nExtendNum>=ConfigParam.m_vectExtendCharge.size())
	{
		Rsp.m_Result = enPacketRetCode_ExtendNumLimit;		
	}
	else
	{
		int nNeedCharge = ConfigParam.m_vectExtendCharge[nExtendNum];

		//判断钱是否够
		if( m_pActor->GetCrtProp(enCrtProp_ActorMoney) < nNeedCharge)
		{
			Rsp.m_Result = enPacketRetCode_ErrNoMoneyExtender;
		}
		else
		{
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney,-nNeedCharge);

			g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), nNeedCharge, m_pActor->GetCrtProp(enCrtProp_ActorMoney), "扩展背包容量");

			//
			m_Capacity += ConfigParam.m_PacketExtendSize;

			Rsp.m_Capacity = m_Capacity;
			if(nExtendNum+1>=ConfigParam.m_vectExtendCharge.size())
			{
				Rsp.m_CanExtend = false;
				Rsp.m_Charges   = 0;
			}
			else
			{
				Rsp.m_CanExtend = true;
				Rsp.m_Charges = ConfigParam.m_vectExtendCharge[nExtendNum+1];
			}	

			//发布事件
			SS_PacketExtend  PacketExtend;
			PacketExtend.m_bCanExtend = Rsp.m_CanExtend;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_PacketExtend);
			m_pActor->OnEvent(msgID,&PacketExtend,sizeof(PacketExtend));
		}
	}

	OBuffer1k ob;

	ob << Packet_Header(enPacketCmd_Extend,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	return Rsp.m_Result == enPacketRetCode_OK;
}

//获得已扩充容量次数
INT16  PacketPart::GetExtendNum()
{
	const SGameConfigParam  & ConfigParam = g_pGameServer->GetConfigServer()->GetGameConfigParam();

	int num = (m_Capacity-ConfigParam.m_PacketInitCapacity) /ConfigParam.m_PacketExtendSize;

	return num;
}

//使用      Index为第几个状态组确认使用
bool PacketPart::UseGoods(UID uidGoods,UID uidTarget,INT32 nNum, UINT8 Index)
{
	return g_pGameServer->GetGoodsServer()->UseGoods(m_pActor,uidGoods,uidTarget,nNum, Index);
}

//丢弃
bool PacketPart::DiscardGoods(UID uidGoods)
{
	SC_PacketDiscardGoods_Rsp Rsp;

	Rsp.m_Result = enPacketRetCode_OK;

	TGoodsID GoodsID  = 0;
	UINT16	 GoodsNum = 0;

	IGoods * pGoods = this->GetGoods(uidGoods);
	if( 0 == pGoods)
	{
		Rsp.m_Result = enPacketRetCode_NoGoods;
	}else
	{
		GoodsID = pGoods->GetGoodsID();
		GoodsNum = pGoods->GetNumber();
	}

	if ( Rsp.m_Result == enPacketRetCode_OK){

		if( DestroyGoods(uidGoods)==false)
		{
			Rsp.m_Result = enPacketRetCode_NoGoods;
		}else
		{
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Sell,GoodsID,uidGoods,GoodsNum,"丢弃物品");
		}
	}


	OBuffer1k ob;

	ob << Packet_Header(enPacketCmd_DiscardGoods,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	return Rsp.m_Result == enPacketRetCode_OK;

}

//装备
bool PacketPart::Equip(UID uidGoods,UID uidTarget,UINT8 pos)
{
	SC_PacketEquip_Rsp Rsp;

	Rsp.m_Result = enPacketRetCode_OK;

	IGoods * pGoods = 0;

	//目标
	IActor * pTarget = m_pActor;
	if(m_pActor->GetUID() != uidTarget)
	{
		pTarget = m_pActor->GetEmployee(uidTarget);
	}

	if(pTarget==0)
	{
		Rsp.m_Result = enPacketRetCode_NoTarget;
	}
	else if((pGoods=GetGoods(uidGoods))==0)
	{
		Rsp.m_Result = enPacketRetCode_NoGoods;
	}
	else if(pGoods->GetGoodsCnfg()->m_UsedLevel>pTarget->GetCrtProp(enCrtProp_Level))
	{
		Rsp.m_Result = enPacketRetCode_LevelLimit;
	}	
	else if(pGoods->GetGoodsClass() != enGoodsCategory_Equip
		&& pGoods->GetGoodsClass() != enGoodsCategory_GodSword
		&& pGoods->GetGoodsClass() != enGoodsCategory_Talisman)
	{
		Rsp.m_Result = enPacketRetCode_UnEquip;
	}
	else
	{
		//移到装备栏	
		IEquipPart * pEquipPart = pTarget->GetEquipPart();


		if(pGoods->GetGoodsClass()==enGoodsCategory_GodSword
			|| pGoods->GetGoodsClass()==enGoodsCategory_Talisman)
		{
			//判断是否已有相同的仙剑或法宝存在了
			for(int i=enEquipPos_GodSwordOne; i<=enEquipPos_TalismanThree;i++)
			{
				IEquipment * pEquipTemp =  pEquipPart->GetEquipByPos(i);
				if(pEquipTemp == 0)
				{
					continue;
				}

				if(pEquipTemp->GetGoodsID() == pGoods->GetGoodsID())
				{
					if( i!=pos ) //位置不同，不是替换
					{
						Rsp.m_Result = enPacketRetCode_ErrSameGodSwrod;
					}
					break;
				}
			}
		}

		if(Rsp.m_Result == enPacketRetCode_OK)
		{
			if(false == RemoveGoods(uidGoods))
			{
				Rsp.m_Result = enPacketRetCode_NoGoods;
			}
			else
			{
				IGoods * pGoodsOld = pEquipPart->GetEquipByPos(pos);
				if(pGoodsOld)
				{
					pEquipPart->RemoveEquip(pGoodsOld->GetUID());

				}

				if(pEquipPart->AddEquip(pGoods->GetUID(),pos)==false)
				{
					if(AddGoods(uidGoods)==false)
					{
						TRACE("<error> %s : %d line 增加背包物品失败 goodsid = %d",__FUNCTION__,__LINE__,pGoods->GetGoodsID());
					}

					Rsp.m_Result = enPacketRetCode_UnEquip;
				}
			}
		}

	}

	OBuffer1k ob;

	ob << Packet_Header(enPacketCmd_Equip,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	if( enPacketRetCode_OK == Rsp.m_Result){
		//发布事件
		SS_OnEquip OnEquip;
		OnEquip.m_Pos = pos;
		OnEquip.m_GoodsID = pGoods->GetGoodsID();
		OnEquip.m_Category = pGoods->GetGoodsClass();

		UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_OnEquip);
		m_pActor->OnEvent(msgID,&OnEquip,sizeof(OnEquip));
	}

	return true;
}

//合成
bool PacketPart::Compose(TGoodsID gidOutput,INT16 OutputNum)
{
	const SGoodsComposeCnfg * pGoodsComposeCnfg = g_pGameServer->GetConfigServer()->GetGoodsComposeCnfg(gidOutput);

	SC_PacketCompose_Rsp Rsp;
	Rsp.m_Result = enPacketRetCode_OK;


	IPacketPart * pPacketPart = m_pActor->GetPacketPart();


	if(pGoodsComposeCnfg == 0)
	{
		TRACE("<error> %s : %d line 该物品不能合成 goodsid = %d",__FUNCTION__,__LINE__,gidOutput);
		Rsp.m_Result = enPacketRetCode_NoCompose;
	}
	else if( 0 == pPacketPart)
	{
		Rsp.m_Result =  enPacketRetCode_Error;
	}
	else if(CanAddGoods(gidOutput,OutputNum)==false)
	{
		Rsp.m_Result = enPacketRetCode_NoSpace;
	}
	else if( m_pActor->GetCrtProp(enCrtProp_ActorMoney) < pGoodsComposeCnfg->m_Charge)
	{
		Rsp.m_Result =  enPacketRetCode_NoMoney;
	}
	else
	{
		for(int i=0; i+1 < pGoodsComposeCnfg->m_vectMaterial.size(); i += 2)
		{
			if(!pPacketPart->HaveGoods(pGoodsComposeCnfg->m_vectMaterial[i], pGoodsComposeCnfg->m_vectMaterial[i+1] * OutputNum)){
				Rsp.m_Result =  enPacketRetCode_NoMaterial;
				break;
			}
		
		}

		if(Rsp.m_Result == enPacketRetCode_OK){
			//扣除
			m_pActor->AddCrtPropNum(enCrtProp_ActorMoney, -pGoodsComposeCnfg->m_Charge);
			g_pGameServer->GetGameWorld()->Save_GodStoneLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID), pGoodsComposeCnfg->m_Charge, m_pActor->GetCrtProp(enCrtProp_ActorMoney), "物品合成");

			for(int i=0; i+1 < pGoodsComposeCnfg->m_vectMaterial.size(); i += 2){
				pPacketPart->DestroyGoods(pGoodsComposeCnfg->m_vectMaterial[i], pGoodsComposeCnfg->m_vectMaterial[i+1] * OutputNum);
				g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_vectMaterial[i],UID(),pGoodsComposeCnfg->m_vectMaterial[i+1] * OutputNum,"物品合成扣除材料");
			
			}

			//给玩家合成的物品
			pPacketPart->AddGoods(pGoodsComposeCnfg->m_GoodsID,OutputNum,true);
			g_pGameServer->GetGoodsServer()->Save_GoodsLog(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Compose,pGoodsComposeCnfg->m_GoodsID,UID(),1,"合成物品");
		}

	}

	OBuffer1k ob;

	ob << Packet_Header(enPacketCmd_Compose,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	return Rsp.m_Result == enPacketRetCode_OK;
}

//是否有指定数量的物品
bool  PacketPart::HaveGoods(TGoodsID GoodsID,INT32 nNum)
{
	for(MAP_SET::iterator it = m_setGoods.begin(); it != m_setGoods.end(); ++it)
	{
		IGoods * pGoods = GetGoodsFromGameWorld(*it);
		if(pGoods && pGoods->GetGoodsID() == GoodsID)
		{
			nNum -= pGoods->GetNumber();
			if(nNum<=0)
			{
				return true;
			}
		}
	}

	return false;
}

IGoods * PacketPart::GetGoodsFromGameWorld(UID uidGoods)
{
	return g_pGameServer->GetGameWorld()->GetGoods(uidGoods);
}



//加入已发送客户端物品配置记录
void PacketPart::InsertSendGoodsCnfg(TGoodsID GoodsID)
{
	m_GoodsCnfg.insert(GoodsID);
}

//是否已经发送过物品配置记录
bool PacketPart::IsAlreadySend(TGoodsID GoodsID)
{
	std::hash_set<TGoodsID>::iterator iter = m_GoodsCnfg.find(GoodsID);
	if( iter == m_GoodsCnfg.end()){
		return false;
	}

	return true;
}

//清空已发送客户端的所有配置记录
void PacketPart::ClearSendCnfg()
{
	g_pGameServer->GetEventServer()->RemoveAllListener(this);
	m_GoodsCnfg.clear();
	m_setThing.clear();
	m_EmployeeCnfg.clear();
	m_MagicLevelCnfg.clear();
	m_SynMagicCnfg.clear();
}

//加入已发送客户端招募角色配置记录
void PacketPart::InsertSendEmployeeCnfg(TActorID EmployeeID)
{
	m_EmployeeCnfg.insert(EmployeeID);
}

//是否已经发送过招募角色配置记录
bool PacketPart::IsAlreadySendEmployeeCnfg(TActorID EmployeeID)
{
	std::hash_set<TActorID>::iterator iter = m_EmployeeCnfg.find(EmployeeID);
	if( iter == m_EmployeeCnfg.end()){
		return false;
	}

	return true;
}

//加入已发送客户端法术等级配置记录
void PacketPart::InsertSendMagicLevelCnfg(TMagicID	MagicID)
{
	m_MagicLevelCnfg.insert(MagicID);
}

//是否已经发送过法术等级配置记录
bool PacketPart::IsAlreadySendMagicLevelCnfg(TMagicID MagicID)
{
	std::hash_set<TMagicID>::iterator iter = m_MagicLevelCnfg.find(MagicID);
	if( iter == m_MagicLevelCnfg.end()){
		return false;
	}

	return true;
}

void PacketPart::OnEvent(XEventData & EventData)
{
	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThingDestroy);
	if(EventData.m_MsgID == msgID )
	{
		if(EventData.m_len < SIZE_OF(SS_ThingDestroyContext()))
		{
			TRACE("<error> %s : %d line 错误的事件上下文数据长度 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThingDestroy,len = %d 期望的长度为 %d)",__FUNCTION__,__LINE__,EventData.m_len , SIZE_OF(SS_ThingDestroyContext()));
			return;
		}
		SS_ThingDestroyContext * pCnt = (SS_ThingDestroyContext *)EventData.m_pContext;


		if(IsAlreadySendThing(pCnt->m_uidThing))
		{
			//通知客户端删除
			m_pActor->NoticClientDestroyThing(pCnt->m_uidThing);
		}

		IThing * pThing = (IThing *)EventData.m_MsgSource;

		if(pThing)
		{
			pThing->UnsubscribeEvent(msgID,this);
		}
	}
}

//加入已发送客户端创建Thing记录
void PacketPart::InsertSendThing(IThing * pThing )
{
	UID uidThing = pThing->GetUID();

	UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_ThingDestroy);

	pThing->SubscribeEvent(msgID,this,"PacketPart::InsertSendThing[enMsgID_ThingDestroy]");

	m_setThing.insert(uidThing);
}

//是否已发送客户端创建Thing记录
bool PacketPart::IsAlreadySendThing(UID uidThing)
{
	return m_setThing.find(uidThing) != m_setThing.end();
}

//清除已发送给客户端的thing记录
void  PacketPart::ClearSendThing(UID uidThing)
{
	m_setThing.erase(uidThing);
}

//加入已发送客户端帮派技能配置记录
void PacketPart::InsertSendSynMagicCnfg(TSynMagicID SynMagicID)
{	
	m_SynMagicCnfg.insert(SynMagicID);
}

//是否已发送客户端帮派技能配置记录
bool PacketPart::IsAlreadySendSynMagic(TSynMagicID SynMagicID)
{
	SET_SYNMAGIC::iterator iter = m_SynMagicCnfg.find(SynMagicID);
	if( iter != m_SynMagicCnfg.end()){
		return true;
	}

	return false;
}

 //物品保存到数据库
void	 PacketPart::SaveGoodsToDB(IGoods * pGoods)
{
	MAP_SET::iterator iter = m_setGoods.find(pGoods->GetUID());

	if (iter != m_setGoods.end()){
		
		SDB_Save_GoodsReq DBGoodsData;

		int size = sizeof(SDB_Save_GoodsReq);

		if (false == pGoods->OnGetDBContext(&DBGoodsData,size)){
			return;
		}

		DBGoodsData.m_uidUser = m_pActor->GetUID().ToUint64();
		DBGoodsData.m_Pos = 0;
		DBGoodsData.m_Location = 0;

		OBuffer1k ob;
		ob << DBGoodsData;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_SaveGoods,ob.TakeOsb(),0,0);

		//重新设置成不需要保存
		pGoods->SetUpdate(false);
	} else {
		//在装备栏
		IEquipPart * pEquipPart = m_pActor->GetEquipPart();

		if (0 == pEquipPart){
			return;
		}

		if (pEquipPart->SaveGoodsToDB(pGoods)){
			return;
		}
		
		for (int i = 0; i < MAX_EMPLOY_NUM; ++i)
		{
			IActor * pEmployee = m_pActor->GetEmployee(i);

			if (0 == pEmployee){	
				continue;
			}

			IEquipPart * pEquipPart = pEmployee->GetEquipPart();

			if (0 == pEquipPart){
				continue;
			}

			if (pEquipPart->SaveGoodsToDB(pGoods)){
				return;
			}
		}
	}
}

//销毁指定物品
bool	PacketPart::DestroyGoods(UID uidGoods, INT32 nNum)
{
	IGoods * pGoods = this->GetGoods(uidGoods);
	if( 0 == pGoods){
		return false;
	}

	INT32 nCurNum = pGoods->GetNumber();

	if( nNum > nCurNum){
		return false;
	}

	if( nNum < nCurNum){
		pGoods->SetPropNum(m_pActor, enGoodsProp_Number, nCurNum - nNum);

		this->SaveGoodsToDB(pGoods);
	}else{

		if (!this->DestroyGoods(uidGoods)){
			
			return false;
		}
	}

	return true;
}

//数据库删除装备
void	 PacketPart::DBDeleteGoods(UID uidGoods)
{
	SDB_Delete_GoodsReq DeleteEquip;

	DeleteEquip.m_uidUser  = m_pActor->GetUID().ToUint64();
	DeleteEquip.m_uidGoods = uidGoods.ToUint64();
	DeleteEquip.m_Pos	   = 0;

	OBuffer1k ob;
	ob << DeleteEquip;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteGoods,ob.TakeOsb(),0,0);
}

//增加进背包
bool	PacketPart::__AddGoods(IGoods * pGoods, bool bSave)
{
	if ( !m_setGoods.insert(pGoods->GetUID()).second )
		return false;

	if ( bSave )
	{
		this->SaveGoodsToDB(pGoods);	
	}

	return true;
}
