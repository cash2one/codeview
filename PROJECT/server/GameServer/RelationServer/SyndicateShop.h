#ifndef __RELATIONSERVER_SYNDICATESHOP_H__
#define __RELATIONSERVER_SYNDICATESHOP_H__

#include <vector>
#include <hash_map>
#include "GameSrvProtocol.h"

class SyndicateMgr;

struct SSynGoodsData
{
	TGoodsID	m_GoodsID;				//物品ID
	INT32		m_Contribution;			//购买需要的贡献值
};

struct IActor;

class SyndicateShop
{
public:
	SyndicateShop();
	~SyndicateShop();

	bool Create(SyndicateMgr * pSyndicateMgr);
	void Close();

public:
	//查看帮派商铺
	void	ViewSynShop(IActor * pActor, enSynShopLabel SynShopLabel);

	//购买帮派物品
	void	BuySynGoods(IActor * pActor, TGoodsID GoodsID);

private:
	SyndicateMgr *											m_pSyndicateMgr;

	typedef std::hash_map<TGoodsID, SSynGoodsData>			MAPSYNGOODS;

	std::hash_map<UINT8/*enSynShopLabel*/,  MAPSYNGOODS>	m_mapSynGoods;

};

#endif
