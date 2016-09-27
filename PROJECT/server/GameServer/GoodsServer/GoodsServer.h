
#ifndef __GOODSSERVER_GOODSSERVER_H__
#define __GOODSSERVER_GOODSSERVER_H__

#include "IGoodsServer.h"
#include "ShopMall.h"
#include "GoodsUseHandler.h"
#include "CDTime.h"
#include "IRelationServer.h"
#include "IDBProxyClient.h"
#include <set>

extern IGameServer * g_pGameServer;

class GoodsServer : public IGoodsServer,public IGMCmdHandler, public IDBProxyClientSink
{
public:
	GoodsServer();
	virtual ~GoodsServer();

	bool Create();
public:
		//释放
	virtual void Release(void);

	virtual void Close();

public:
		//使用
	virtual bool UseGoods(IActor * pActor,UID uidGoods,UID uidTarget = UID(),INT32 nNum=1, UINT8 Index = 0);

	//根据掉落ID，给玩家掉落物品
	virtual bool GiveUserDropGoods(IActor * pActor, UINT16 DropID,std::vector<TGoodsID> & vectGoods);

		//判断玩家是否拥有该物品,包括装备栏，背包栏
	virtual IGoods* GetGoodsFromPacketOrEquipPanel(IActor * pActor,UID uidGoods);

	//判断一件装备是装备哪个角色身上
	virtual IActor * GetActorOnEquip(IActor * pActor, IEquipment * pEquipment);

	//获得冷却接口
	virtual ICDTimeMgr * GetCDTimeMgr();

	virtual void OnHandleGMCmd(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam); 

	//得到物品品质对应颜色
	virtual enColor GetGoodsQualityColor(IGoods * pGoods);

	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	//记录物品日志
	virtual void Save_GoodsLog(TUserID UserID,enGameGoodsType type,enGameGoodsChanel chanel,TGoodsID GoodsID,UID uidGoods,UINT16 GoodsNum,const char * pszDesc);

	//放置商城配置信息
	virtual void Push_ShopMallCnfg(const SDB_Get_ShopMallCnfg & DBShopCnfg);

	//得到掉落的物品ID集合,返回掉落的物品数量
	virtual void GetDropGoodsID(UINT16 DropID, std::vector<TGoodsID> & vectGoods);

private:
	//通过GM命令增加物品
	void OnHandleGMCmdAddGoods(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	//通过GM命令增加仙石
	void OnHandleGMCmdSetMoney(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	//通过GM命令增加灵石
	void OnHandleGMCmdSetStone(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	//通过GM命令增加礼券
	void OnHandleGMCmdSetTicket(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	//需要确认的则发送消息,弹出确认框
	void SendConfirmation(IActor * pActor,UID uidGoods,UID uidTarget,INT32 nNum, enPacketRetCode PacketRetCode);

	void HandleGetNeedLogGoods(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

private:
	ShopMall			m_ShopMall;
	GoodsUseHandler		m_GoodsUseHandler; //物品使用

	CDTimeMgr			m_CDTimeMgr;

	std::set<TGoodsID>	m_mapNeedLogGoodsID;
};









#endif
