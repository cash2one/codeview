#include "SyndicateShop.h"
#include "IActor.h"
#include "ISyndicateMember.h"
#include "SyndicateMgr.h"
#include "RelationServer.h"
#include "IConfigServer.h"
#include "IPacketPart.h"
#include "DMsgSubAction.h"
#include "IGoodsServer.h"

SyndicateShop::SyndicateShop()
{
	m_pSyndicateMgr = 0;
}
SyndicateShop::~SyndicateShop()
{

}

bool	SyndicateShop::Create(SyndicateMgr * pSyndicateMgr)
{
	m_pSyndicateMgr = pSyndicateMgr;

	const std::hash_map<TGoodsID, SSynGoodsCnfg> *	pmapSynGoodsCnfg = g_pGameServer->GetConfigServer()->GetAllSynGoodsCnfg();
	if( 0 == pmapSynGoodsCnfg){
		return false;
	}

	std::hash_map<TGoodsID, SSynGoodsCnfg>::const_iterator iter = pmapSynGoodsCnfg->begin();
	for( ; iter != pmapSynGoodsCnfg->end(); ++iter)
	{
		const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(iter->first);
		if( 0 == pGoodsCnfg){
			continue;
		}

		UINT8 SynGoodsLabel = pGoodsCnfg->m_GoodsClass;
		if( SynGoodsLabel >= (UINT8)enGoodsCategory_Max){
			continue;
		}
		
		SSynGoodsData SynGoodsData;
		SynGoodsData.m_Contribution = (iter->second).m_Contribution;
		SynGoodsData.m_GoodsID		= iter->first;

		std::hash_map<UINT8/*enSynShopLabel*/,  MAPSYNGOODS>::iterator it = m_mapSynGoods.find(SynGoodsLabel);
		if( it == m_mapSynGoods.end()){
			MAPSYNGOODS mapGoods;
			mapGoods[iter->first] = SynGoodsData;
			m_mapSynGoods[SynGoodsLabel] = mapGoods;
		}else{
			MAPSYNGOODS & mapGoods = it->second;
			mapGoods[iter->first] = SynGoodsData;
		}
	}
	return true;
}

void	SyndicateShop::Close()
{
}

//查看帮派商铺
void	SyndicateShop::ViewSynShop(IActor * pActor, enSynShopLabel SynShopLabel)
{
	ISyndicateMember * pSyndicateMember = m_pSyndicateMgr->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSyndicateMember){
		return;
	}

	SC_ViewSynShop_Rsp RspNum;
	OBuffer1k ob;

	std::hash_map<UINT8/*enSynShopLabel*/,  MAPSYNGOODS>::iterator iter = m_mapSynGoods.find((UINT8)SynShopLabel);
	if( iter == m_mapSynGoods.end()){
		RspNum.m_GoodsNum = 0;

		ob << SyndicateHeader(enSyndicateCmd_ViewSynShop, sizeof(RspNum)) << RspNum;
		pActor->SendData(ob.TakeOsb());
		return;
	}

	MAPSYNGOODS & mapGoods = iter->second;

	RspNum.m_GoodsNum = mapGoods.size();

	ob << SyndicateHeader(enSyndicateCmd_ViewSynShop, sizeof(RspNum) + RspNum.m_GoodsNum * sizeof(ViewSynGoods_Rsp)) << RspNum; 

	MAPSYNGOODS::iterator it = mapGoods.begin();
	for( ; it != mapGoods.end(); ++it)
	{
		SSynGoodsData & SynGoods = it->second;

		ViewSynGoods_Rsp Rsp;

		Rsp.m_Contribution = SynGoods.m_Contribution;
		Rsp.m_GoodsID	   = SynGoods.m_GoodsID;
		ob << Rsp;

		pActor->SendGoodsCnfg(SynGoods.m_GoodsID);
	}
	pActor->SendData(ob.TakeOsb());
}

//购买帮派物品
void	SyndicateShop::BuySynGoods(IActor * pActor, TGoodsID GoodsID)
{
	const SGoodsCnfg * pGoodsCnfg = g_pGameServer->GetConfigServer()->GetGoodsCnfg(GoodsID);
	if( 0 == pGoodsCnfg || pGoodsCnfg->m_GoodsClass >= (UINT8)enGoodsCategory_Max){
		return;
	}

	std::hash_map<UINT8/*enSynShopLabel*/,  MAPSYNGOODS>::iterator iter = m_mapSynGoods.find(pGoodsCnfg->m_GoodsClass);
	if( iter == m_mapSynGoods.end()){
		return;
	}

	MAPSYNGOODS & mapGoods = iter->second;

	MAPSYNGOODS::iterator  it = mapGoods.find(GoodsID);
	if( it == mapGoods.end()){
		return;
	}
	SSynGoodsData & SynGoods = it->second;

	ISyndicateMember * pSyndicateMember = m_pSyndicateMgr->GetSyndicateMember(pActor->GetUID());
	if( 0 == pSyndicateMember){
		return;
	}

	SC_Syndicate_Rsp Rsp;

	if( pSyndicateMember->GetContribution() < SynGoods.m_Contribution){
		Rsp.m_SynRetCode = enSynRetCode_NoMoney;
	}else{
		IPacketPart * pPacketPart = pActor->GetPacketPart();
		if( 0 == pPacketPart){
			return;
		}

		if( false == pPacketPart->CanAddGoods(GoodsID, 1)){
			Rsp.m_SynRetCode = enSynRetCode_PacketFull;
		}else{
			//条件都满足，则进行扣帮派贡献和放入玩家背包中
			pSyndicateMember->AddContribution(-SynGoods.m_Contribution);
			pPacketPart->AddGoods(GoodsID, 1,true);

			g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Buy,GoodsID,UID(),1,"购买帮派物品");

			//发布事件
			SS_SynShopBuyGoods SynShopBuyGoods;
			SynShopBuyGoods.m_GoodsID = GoodsID;

			UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_SynShopBuyGoods);
			pActor->OnEvent(msgID,&SynShopBuyGoods,sizeof(SynShopBuyGoods));

			//使用帮贡事件
			SS_UseSynContribution UseSynContribution;
			UseSynContribution.m_UseNum = SynGoods.m_Contribution;

			msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UseSynContribution);
			pActor->OnEvent(msgID,&UseSynContribution,sizeof(UseSynContribution));
		}
	}
	OBuffer1k ob;
	ob << SyndicateHeader(enSyndicateCmd_BuySynGoods, sizeof(Rsp)) << Rsp; 
	pActor->SendData(ob.TakeOsb());
}
