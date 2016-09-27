
#ifndef __THINGSERVER_ITALISMAN_PART_H__
#define __THINGSERVER_ITALISMAN_PART_H__

#include "IThingPart.h"
#include "TBuffer.h"
#include "ICreature.h"

struct OccupationInfo
{
	UINT8		m_Level;
	UINT8		m_Floor;
	UID			m_uidMonster;
	UINT32		m_OccupationTime;	//占领时间
};


//灵件属性对应人物属性ID
static enCrtProp s_GhostPropMapActorProp [enGhostProp_Max] = 
{
	enCrtProp_ActorSpiritEqup ,   //灵力
	enCrtProp_ActorShieldEqup ,   //护盾
	enCrtProp_ActorAvoidEqup  ,   //身法
	enCrtProp_ActorBloodEqup  ,   //气血
	
};

struct ITalismanPart : public IThingPart
{
	//玩家是否可以进入法宝世界
	virtual bool CanEnterTalismanWorld(TTalismanWorldID TalismanWorldID) = 0;

	//进入法宝世界
	virtual bool EnterTalismanWorld(CS_TalismanWorldEnter_Req & Req) = 0;

	//退出法宝世界
	virtual void LeaveTalismanWorld(UINT32 GameID) = 0;

	//游戏消息
	virtual void GameMessage(CSC_TalismanWorldGame & Req,IBuffer & ib) = 0;

		//孕育法宝物
	virtual bool SpawnTalismanSpawnGoods(UID  uidTalisman) = 0;

		//领取法宝孕育物
	virtual bool TakeTalismanSpawnGoods(UID  uidTalisman) = 0;

		//提升品质点
	virtual bool UpgradeTalismanQuality(UID  uidTalisman,TGoodsID gidProp) = 0;

	//占领
	virtual bool Occupation(UID uidMonster, UINT8	Level, UINT8 floor) = 0;

	//占领结束
	virtual void OverOccupation(bool bBeaten = false) = 0;

	//得到占领信息
	virtual const OccupationInfo * GetOccupationInfo() = 0; 

	//掉线，下线要做的一些事
	virtual void  LeaveDoSomeThing() = 0;

	//检测法宝世界次数
	virtual bool Check_EnterTalismanWorldNum() = 0;

	//打开法宝世界
	virtual void OpenTalismanWorld() = 0;

	//得到今天进入法宝世界次数
    virtual UINT8	GetEnterTWNumToday() = 0;

	  //得到今天vip进入法宝世界次数
   virtual UINT8	GetVipEnterTWNumToday() = 0;

   //增加进入法宝世界次数
   virtual void		AddEnterNum() = 0;

   //设置下的法宝世界场景
   virtual void		SetTWSceneID(TSceneID SceneID) = 0;

   //清除法宝世界的次数
   virtual void		ClearTWEnterNum() = 0;
};




#endif
