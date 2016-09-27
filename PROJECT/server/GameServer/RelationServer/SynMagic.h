#ifndef __RELATIONSERVER_SYNMAGIC_H__
#define __RELATIONSERVER_SYNMAGIC_H__

#include "ISynMagic.h"
#include <hash_map>

struct IActor;
class SyndicateMgr;

class SynMagic : public ISynMagic
{
public:
	SynMagic();
	~SynMagic();

	bool Create(SyndicateMgr * pSyndicateMgr);

public:
	//查看帮派技能
	void	ViewSynMagic(IActor * pActor);

	//学习帮派技能
	void	LearnSynMagic(IActor * pActor, TSynMagicID SynMagicID);

private:
	//得到帮派技能数据
	SynMagicData * GetSynMagicData(TSynMagicID SynMagicID, UINT8 SynMagicLevel);

private:
	typedef std::hash_map<UINT8/*SynMagicLevel*/, SynMagicData>			MAPSYNMAGIC;

	std::hash_map<TSynMagicID, MAPSYNMAGIC>								m_mapSynMagicData;	//帮派技能数据

	SyndicateMgr *														m_pSyndicateMgr;
};


#endif
