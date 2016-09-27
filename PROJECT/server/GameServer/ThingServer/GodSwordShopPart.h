#ifndef __THINGSERVER_GODSWORDSHOP_H__
#define __THINGSERVER_GODSWORDSHOP_H__

#include "IGodSwordShopPart.h"
#include "BclHeader.h"
#include "DSystem.h"
#include "IThingPart.h"

struct IActor;

struct SGodSwordShopData
{
	SGodSwordShopData(){
		memset(this, 0, sizeof(*this));
	}
	UINT64		m_LastFlushTime;
	TGoodsID	m_GodSword[FLUSH_NUM];
};

class GodSwordShopPart : public IGodSwordShopPart
{
public:
	GodSwordShopPart();
	~GodSwordShopPart();

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
	//进入剑冢
	virtual void  EnterGodSwordShop();
	//刷新剑冢
	virtual void  FlushGodSwordShop(enFlushType FlushType);
	//购买仙剑
	virtual void  BuyGodSword(UINT8 nIndex);
	//同步剑冢
	virtual void  SyncGodSwordShop();

		//获得剩余刷新仙剑时间
	virtual INT32 GetRemainFlushTime(); 

private:
	//免费刷新
	void  FreeFlush();
	//付费刷新
	void  MoneyFlush();

	//新建角色的特殊处理
	void  NewUserSpecialCal();

private:
	IActor *			m_pActor;

	SGodSwordShopData	m_GodSwordShopData;			//剑冢数据

	INT16				m_nCDTimeFreeFlush;			//剑冢免费刷新时间(秒)

	INT16				m_SpendMoneyNumMoneyFlush;	//付费刷新一次多少仙石
};

#endif
