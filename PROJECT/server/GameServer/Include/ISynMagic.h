#ifndef __RELATIONSERVER_ISYNMAGIC_H__
#define __RELATIONSERVER_ISYNMAGIC_H__

#include "DSystem.h"
#include <vector>
#include <string>
#include "BclHeader.h"

struct SynMagicData
{
	SynMagicData(){
		memset(this, 0, sizeof(*this));
	}
	TSynMagicID					m_SynMagicID;						//帮派技能ID
	UINT8						m_SynMagicLevel;					//帮派技能等级
	char						m_szSynMagicName[ACTOR_NAME_LEN];	//帮派技能名称
	std::vector<TEffectID>		m_vectEffect;						//效果列表
	std::string					m_strSynMagicDes;					//帮派技能描述
	INT32						m_NeedContribution;					//要求贡献
	INT32						m_NeedStone;						//要求灵石
	UINT8						m_NeedSynLevel;						//要求帮派等级
};

struct IActor;

struct ISynMagic
{
	//查看帮派技能
	virtual void	ViewSynMagic(IActor * pActor) = 0;

	//学习帮派技能
	virtual void	LearnSynMagic(IActor * pActor, TSynMagicID SynMagicID) = 0;
};

#endif
