
#ifndef __THINGSERVER_EQUIP_PART_H__
#define __THINGSERVER_EQUIP_PART_H__

#include "IEquipPart.h"
#include "IEquipment.h"
#include <set>
#include <hash_map>
#include <vector>

struct IActor;

class EquipPart : public IEquipPart
{
public:
    EquipPart();
	virtual ~EquipPart();

public:

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void) ;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close() ;

	//保存数据
	virtual void SaveData();

		//增加装备
	virtual bool AddEquip(UID uidEquip,UINT8 pos);

	//移除装备 
	virtual bool RemoveEquip(UID uidEquip, bool bDestroyGoods = false);

	//获得装备数量
	virtual INT32 GetEquipNum() ;

	//获得指定部位的装备
	virtual IEquipment * GetEquipByPos(UINT8 pos) ;

	virtual IEquipment * GetEquipment(UID uidEquip);

	//重置装备栏中的仙剑副本
	virtual void ResetGodSwordFuBen();

	//把装备栏的装备全部放回背包，！！！注意：不管背包是否已满
	virtual void RemoveAllEquipToPacket();

	//发送装备栏数据给别的玩家查看
	virtual void SendEquipPanelData(IActor * pActor);

	virtual bool SaveGoodsToDB(IGoods * pGoods);

		//打开外观栏
	virtual void OpenFacadePanel();

	//设置显示外观
	virtual void SetShowFacade(UINT16 FacadeID);

		//增加外观
	virtual void AddFacade(UINT16 FacadeID) ;

	//移除外观
	virtual void RemoveFacade(UINT16 FacadeID);

		//获得默认外观
	virtual UINT16 GetDefaultFacade();

private:
		//激活套装
   void  ActiveSuit(IEquipment * pAddEquip, UINT16 SuitID,UINT8 EquipNum);

   //不激活套装
   void  InActiveSuit(IEquipment * pRemoveEquip, UINT16 SuitID,UINT8 EquipNum);

   //物品保存到数据库,nLocation为在装备栏中的位置
   void	 SaveGoodsToDB(IGoods * pGoods, UINT8 nLocation);

   //增加装备
   bool __AddEquip(UID uidEquip,UINT8 pos, bool bCreateActor = false);

   //数据库删除装备
   void	 DBDeleteGoods(UID uidGoods);

private:
	IActor * m_pActor;
	UID      m_uidEquips[enEquipPos_Max];  //装备列表
	UINT8    m_GoodsNum;   //物品数量

	typedef std::set<UID>	SET_EQUIPCOMPARE;

	SET_EQUIPCOMPARE	    m_setEquipCompare;	//保存时比较	

	typedef std::hash_map<UINT16 /*SuitID*/,UINT8 /*EquipNum*/> MAP_SUIT;   //相同套装已穿了几件

	MAP_SUIT   m_mapSuit;

	std::vector<UINT16>  m_vectFacade; //外观

};




#endif

