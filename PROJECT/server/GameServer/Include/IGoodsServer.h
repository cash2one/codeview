


#ifndef __GOODSSERVER_IGOODSSERVER_H__
#define __GOODSSERVER_IGOODSSERVER_H__

#include "DSystem.h"

#include "IGameServer.h"

#include <vector>


#ifndef BUILD_GOODSSERVER_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"GoodsServer.lib")
#endif
#endif

struct IGoods;
struct IActor;
struct IEquipment;

struct IGoodsServer
{
	//释放
	virtual void Release(void) = 0;

	virtual void Close()=0;

	//使用物品
	virtual bool UseGoods(IActor * pActor,UID uidGoods,UID uidTarget = UID(),INT32 nNum=1, UINT8 Index = 0)=0;

	//根据掉落ID，给玩家掉落物品
	virtual bool GiveUserDropGoods(IActor * pActor, UINT16 DropID,std::vector<TGoodsID> & vectGoods) = 0;

	//判断玩家是否拥有该物品,包括装备栏，背包栏
	virtual IGoods * GetGoodsFromPacketOrEquipPanel(IActor * pActor,UID uidGoods) = 0;

	//判断一件装备是装备哪个角色身上
	virtual IActor * GetActorOnEquip(IActor * pActor, IEquipment * pEquipment) = 0;

	//获得冷却接口
	virtual ICDTimeMgr * GetCDTimeMgr() = 0;

	//记录物品日志
	virtual void Save_GoodsLog(TUserID UserID,enGameGoodsType type,enGameGoodsChanel chanel,TGoodsID GoodsID,UID uidGoods,UINT16 GoodsNum,const char * pszDesc) = 0;

	//放置商城配置信息
	virtual void Push_ShopMallCnfg(const SDB_Get_ShopMallCnfg & DBShopCnfg) = 0;

	//得到掉落的物品ID集合,返回掉落的物品数量
	virtual void GetDropGoodsID(UINT16 DropID, std::vector<TGoodsID> & vectGoods) = 0;

};

BCL_API  IGoodsServer * CreateGoodsServer(IGameServer *pServerGlobal);



#endif
