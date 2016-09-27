#ifndef __THINGSERVER_SYNMAGICPART_H__
#define __THINGSERVER_SYNMAGICPART_H__

#include "ISynMagicPart.h"
#include <vector>
#include <hash_map>
#include "ISynMagic.h"

struct IActor;

class SynMagicPart : public ISynMagicPart
{
public:
	SynMagicPart();
	~SynMagicPart();

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
	virtual IThing*		GetMaster(void);

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
	virtual void Close();

	//保存数据
	virtual void SaveData();
public:
	//玩家习得此技能
	virtual void	LearnSynMagicOK(const SynMagicData & MagicData);  

	//把帮派技能的效果加给玩家
	virtual void	UserSynMagicEffect();

	//打开帮派技能栏
	virtual void	OpenSynMagicPanel();

	//获得玩家的当前技能等级
	virtual UINT8	GetSynMagicLevel(TSynMagicID SynMagicID);


private:
	IActor *					m_pActor;

	std::hash_map<TSynMagicID, SSynMagicInfo>		m_mapSynMagic;	//已经习得的帮派技能
};


#endif
