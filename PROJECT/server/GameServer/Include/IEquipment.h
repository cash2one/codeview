
#ifndef __THINGSERVER_IEQUIPMENT_H__
#define __THINGSERVER_IEQUIPMENT_H__

#include "IGoods.h"
#include <string>
#include "ICreature.h"
#include "GameSrvProtocol.h"
//装备部位
enum enEquipPos   VC_PACKED_ONE
{
	enEquipPos_Head = 0,   //头部
	enEquipPos_Shoulder,   //肩部
	enEquipPos_Breast ,    //胸部
	enEquipPos_Leg    ,    //腿部
	enEquipPos_Foot  ,     //脚部
	enEquipPos_GodSwordOne, //仙剑1
	enEquipPos_GodSwordTwo, //仙剑2
	enEquipPos_GodSwordThree, //仙剑3

	enEquipPos_TalismanOne, //法宝1
	enEquipPos_TalismanTwo, //法宝2
	enEquipPos_TalismanThree, //法宝3

	enEquipPos_Max,       
} PACKED_ONE;

//最大装备数
#define MAX_LOAD_EQUIPMENT_NUM  5
//最大装载法宝数
#define MAX_LOAD_TALISMAN_NUM   3

//最大装载仙剑数
#define MAX_LOAD_GODSWORD_NUM   3



//装备属性对应人物属性ID
static enCrtProp s_EquipPropMapActorProp [enEquipProp_Max] = 
{
	 enCrtProp_ActorSpiritEqup ,   //灵力
	enCrtProp_ActorShieldEqup ,   //护盾
	enCrtProp_ActorBloodEqup  ,   //气血
	enCrtProp_ActorAvoidEqup  ,    //身法，躲避
};


struct IEquipment : public IGoods
{
	//检查生物pCreature是否能装备该项装备，返回TRUE表示可以装备
	virtual	bool CanEquip(ICreature *pCreature,UINT8 pos,std::string & strErr) = 0;

	//给生物pCreature装备该装备，返回TRUE表示装备成功
	virtual bool OnEquip(ICreature *pCreature) = 0;

	//给生物pCreature卸载该装备，返回TRUE表示卸载成功
	virtual bool UnEquip(ICreature *pCreature) = 0;

	//激活套装属性
	virtual void ActiveSuitProp(IActor* pActor,enGoodsProp PropID){};

   //不激活套装属性
	virtual void InActiveSuitProp(IActor* pActor,enGoodsProp PropID){};



};


#endif
