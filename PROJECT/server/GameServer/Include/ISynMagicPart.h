#ifndef __THINGSERVER_ISYNMAGICPART_H__
#define __THINGSERVER_ISYNMAGICPART_H__

#include "IThingPart.h"
#include "ISynMagic.h"

struct SSynMagicInfo
{
	TSynMagicID					m_SynMagicID;						//帮派技能ID
	UINT8						m_SynMagicLevel;					//帮派技能等级
};

struct ISynMagicPart : public IThingPart
{
	//玩家习得此技能
	virtual void	LearnSynMagicOK(const SynMagicData & MagicData) = 0;

	//把帮派技能的效果加给玩家
	virtual void	UserSynMagicEffect() = 0;

	//打开帮派技能栏
	virtual void	OpenSynMagicPanel() = 0;

	//获得玩家的当前技能等级
	virtual UINT8	GetSynMagicLevel(TSynMagicID SynMagicID) = 0;
};

#endif
