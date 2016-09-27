
#ifndef __THINGSERVER_IMAGICPART_H__
#define __THINGSERVER_IMAGICPART_H__

#include "IThingPart.h"

#include "GameSrvProtocol.h"

struct IMagic;

struct IMagicPart : public IThingPart
{
	   //获得玩家已学会的法术
	virtual IMagic * GetMagic(TMagicID MagicID) =0;

	//获取已加载的法术
	virtual IMagic * GetLoadedMagic(UINT8 pos) = 0;

	//玩家学会法术
	virtual IMagic * StudyMagic(TMagicID MagicID) =0;


	//打开法术栏
	virtual void OpenMagicPanel() = 0;

	//升级法术
	virtual void UpgradeMagic(TMagicID MagicID) = 0;

	//加载法术
	virtual void LoadMagic(TMagicID MagicID,UINT8 pos) = 0;

	//卸载法术
	virtual void UnloadMagic(TMagicID MagicID,UINT8 pos) = 0;

	//直接设置法术等级
	virtual bool SetMagicLevel(TMagicID MagicID, UINT8 level) = 0;

};



#endif
