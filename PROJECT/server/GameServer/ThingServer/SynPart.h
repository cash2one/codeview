#ifndef __THINGSERVER_SYNPART_H__
#define __THINGSERVER_SYNPART_H__

#include "IActor.h"
#include "ISynPart.h"

class SyndicateCombat; 
class SynPart : public ISynPart
{
public:
	SynPart();
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
	//获得今天参加帮战的战斗次数
	virtual UINT16	GetJoinCombatNumToday();

	//获得今天最多可参加帮战的战斗次数
	virtual UINT16	GetMaxJoinCombatNumToday();

	//设置今天最多可参加帮战的战斗次数
	virtual void	SetMaxJoinCombatNumToday(UINT16 MaxJoinCombatNum);

	//获得今天得到的声望
	virtual INT32	GetGetNumWeek();

	//增加今天的战斗次数
	virtual void	AddCombatNumToday(INT16 AddNum);

	//增加今天获得的声望
	virtual void	AddGetCredit(INT32 Credit);



private:
	IActor *	m_pActor;



	UINT16		m_JoinCombatNumToday;		//今天参加帮战战斗的次数

	UINT32		m_LastJoinSynWarTime;		//最后一次参加帮战的时间

	UINT16		m_MaxJoinCombatNumToday;	//今天总共可参加的帮战次数

	UINT32		m_LastSetMaxCombatNumTime;		//最后一次设置最多可参加帮战次数的时间

	INT32		m_GetCreditWeek;			//本周获得的声望数

	UINT32		m_LastGetCreditTime;		//最后一次获得声望时间
};

#endif
