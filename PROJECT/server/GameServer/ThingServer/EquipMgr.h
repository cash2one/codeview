
#ifndef __THINGSERVER_EQUIP_MGR_H__
#define __THINGSERVER_EQUIP_MGR_H__



#include "TBuffer.h"
#include "IMessageDispatch.h"
#include "UniqueIDGenerator.h"
#include "GameSrvProtocol.h"
#include "IEquipment.h"
#include "IConfigServer.h"

enum enStrongerType VC_PACKED_ONE
{
	enStrongerType_Equip = 0,		//装备强化
	enStrongerType_Talisman,		//法宝强化
	enStrongerType_GodSword,		//仙剑强化
}PACKED_ONE;

struct IGoods;
struct IActor;

class EquipMgr : public IMsgRootDispatchSink
{
public:
   EquipMgr();

   virtual ~EquipMgr();

   bool Create();

   void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib); 

private:
	//移除装备
	void RemoveEquip(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//装备强化
	void EquipStronger(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//装备自动强化
	void EquipAutoStronger(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//装备镶嵌
	void EquipInlay(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开法术栏
	void EquipMgr::OpenMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//加载法术
	void EquipMgr::AddMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//移除法术
	void EquipMgr::RemoveMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//升级法术
	void EquipMgr::UpgradeMagic(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//设置参战
	void OnJoinBattle(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	
	//同步了阵形
	void LineupSync(IActor *pActor,UINT8 nCmd, IBuffer & ib);
	
	//升级境界
	void AddLayerLevel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开帮派技能
	void OpenSynMagicPanel(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//打开人物面板
	void ViewActorStatus(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//删除状态
	void DeleteStatus(IActor *pActor,UINT8 nCmd, IBuffer & ib);


	//宝石摘除
	void RemoveGem(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//宝石替换
	void ReplaceGem(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//强化传承
	void StrongInheritance(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	
	void EquipMgr::OpenFacadePanel(IActor *pActor,UINT8 nCmd, IBuffer & ib);      //打开外观栏
	void EquipMgr::SetShowFacade(IActor *pActor,UINT8 nCmd, IBuffer & ib); //设置显示外观



private:
	//装备自动强化
	void __EquipAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req);

	//法宝自动强化
	void __TalismanAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req);

	//仙剑自动强化
	void __GodSwordAutoStronger(IActor *pActor,IGoods * pGoods, CS_EquipAutoStronger_Req & Req);

	//强化装备
	INT32 __StrongerEquip(IActor* pActor,IEquipment * pEquipment,bool bAddExtender);

	//强化法宝
	INT32 __StrongerTalisman(IActor* pActor,IEquipment * pEquipment,bool bAddExtender);

	//强化仙剑
	INT32 __StrongerGodSword(IActor* pActor,IEquipment * pEquipment,bool bAddExtender);

	//得到要强化的物品
	IGoods * __GetStrongerGoods(IActor * pActor, UID uidGoods);

	//检测装备是否可以自动强化
	void  __Check_CanEquipAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode);

	//检测法宝是否可以自动强化
	void  __Check_CanTalismanAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode);

	//检测仙剑是否可以自动强化
	void  __Check_CanGodSwordAutoStronger(IActor *pActor,IEquipment * pEquipment, CS_EquipAutoStronger_Req & Req, enEquipRetCode & EquipRetCode);

	//强化成功后，如果装备在玩家身上，则把属性加上去
	void  __CheckInEquip_AddStrongerProp(IActor *pActor,IEquipment * pEquipment, UINT8 OldStarLevel, UINT8 NewStarLevel);

	//镶嵌成功后，如果装备在玩家身上，则把属性加上去
	void  __CheckInEquip_AddInlayProp(IActor *pActor,IEquipment * pEquipment, TGoodsID OldGemID, TGoodsID NewGemID);

	//宝石摘除,返回结果码
	UINT8  __RemoveGem(IActor * pActor, IEquipment * pEquipment, TGoodsID OldGemID, UINT8 index);

	//支付仙石传承
	UINT8  __MoneyInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment);

	//支付灵石传承
	UINT8  __StoneInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment);

	//普通传承
	UINT8  __OrdinaryInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment);

	//
	void   ClearStartLvAndGem(IActor * pActor, IEquipment * pEquipment);


};












#endif
