#ifndef __THINGSERVER_MIJINGMGR_H__
#define __THINGSERVER_MIJINGMGR_H__


#include "IMessageDispatch.h"



#include "TBuffer.h"
//#include "UniqueIDGenerator.h"
#include "GameSrvProtocol.h"
#include "IConfigServer.h"




struct ICommonGoods;


struct IActor;
struct ITalisman;

class MiJingMgr : public IMsgRootDispatchSink, public IEventListener
{
public:
   MiJingMgr();

   virtual ~MiJingMgr();

   bool Create();

   void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib); 

	virtual void OnEvent(XEventData & EventData);

private:

	//打开秘境界面
	void OpenMiJing(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//强化传承
	void StrongInheritance(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//法宝附灵
	void AddGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//灵件摘除
	void RemoveGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//灵件升级
	void UpGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//升级声望技能
	void CreditMagicUp(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//装备制作
	void EquipMake(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//剑诀升级
	void DamageUp(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//支付仙石传承
	UINT8  __MoneyInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment);

	//普通传承
	UINT8  __OrdinaryInheritance(IActor * pActor, IEquipment * pSrcEquipment, IEquipment * pDesEquipment);

	//清除装备星级
	void   ClearStartLvAndGem(IActor * pActor, IEquipment * pEquipment);

	//强化成功后，如果装备在玩家身上，则把属性加上去
	void  __CheckInEquip_AddStrongerProp(IActor *pActor,IEquipment * pEquipment, UINT8 OldStarLevel, UINT8 NewStarLevel);

	//镶嵌成功后，如果装备在玩家身上，则把属性加上去
	void  __CheckInEquip_AddInlayProp(IActor *pActor,IEquipment * pEquipment, TGoodsID OldGemID, TGoodsID NewGemID);

	//附灵成功后，如果法宝在玩家身上，则把属性加上去
	void  __CheckInTalisman_AddGhostProp(IActor *pActor,ITalisman * pTalisman, UINT8 OldLevel, TGoodsID OldGhostID, UINT8 NewLevel, TGoodsID NewGhostID);


	//灵件摘除,返回结果码
	UINT8  __RemoveGhost(IActor * pActor, ITalisman * pTalisman, TGoodsID OldGhostID);

	//灵件替换
	void ReplaceGhost(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//金剑诀升级
	INT32 GoldDamageUp(IActor* pActor,IActor * pTargetActor);

	//木剑诀升级
	INT32 WoodDamageUp(IActor* pActor,IActor * pTargetActor);

	//水剑诀升级
	INT32 WaterDamageUp(IActor* pActor,IActor * pTargetActor);

	//火剑诀升级
	INT32 FireDamageUp(IActor* pActor,IActor * pTargetActor);

	//土剑诀升级
	INT32 SoilDamageUp(IActor* pActor,IActor * pTargetActor);

	//爆击升级
	INT32 CritLevelUp(IActor* pActor,IActor * pTargetActor);

	//坚韧升级
	INT32 TenacityLevelUp(IActor* pActor,IActor * pTargetActor);

	//命中升级
	INT32 HitLevelUp(IActor* pActor,IActor * pTargetActor);

	//回避升级
	INT32 DodgeLevelUp(IActor* pActor,IActor * pTargetActor);

	//法术回复升级
	INT32 MagicCDLevelUp(IActor* pActor,IActor * pTargetActor);

	//灵件升级结果码
    INT32 __UpGhost(IActor* pActor,ICommonGoods * pCommonGoods);

};





#endif
