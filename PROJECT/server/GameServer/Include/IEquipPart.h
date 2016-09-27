
#ifndef __THINGSERVER_IEQUIP_PART_H__
#define __THINGSERVER_IEQUIP_PART_H__

#include "IThingPart.h"
#include "IGoods.h"
struct IEquipment;

struct IEquipPart : public IThingPart
{
	//增加装备
	virtual bool AddEquip(UID uidEquip,UINT8 pos) = 0;

	//移除装备 
	virtual bool RemoveEquip(UID uidEquip, bool bDestroyGoods = false) = 0;

	//获得装备数量
	virtual INT32 GetEquipNum() = 0;

	//获得指定部位的装备
	virtual IEquipment * GetEquipByPos(UINT8 pos) = 0;

	
	virtual IEquipment * GetEquipment(UID uidEquip) = 0;

	//重置装备栏中的仙剑副本
	virtual void ResetGodSwordFuBen() = 0;

	//把装备栏的装备全部放回背包，！！！注意：不管背包是否已满
	virtual void RemoveAllEquipToPacket() = 0;

	//发送装备栏数据给别的玩家查看
	virtual void SendEquipPanelData(IActor * pActor) = 0;

	virtual bool SaveGoodsToDB(IGoods * pGoods) = 0;

	//打开外观栏
	virtual void OpenFacadePanel() = 0;

	//设置显示外观
	virtual void SetShowFacade(UINT16 FacadeID) = 0;

	//增加外观
	virtual void AddFacade(UINT16 FacadeID) = 0;

	//移除外观
	virtual void RemoveFacade(UINT16 FacadeID) = 0;

	//获得默认外观
	virtual UINT16 GetDefaultFacade() = 0;

};




#endif

