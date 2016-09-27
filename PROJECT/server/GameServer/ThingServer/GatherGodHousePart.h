
#ifndef __THINGSERVER_GATHERGODHOUSE_PART_H__
#define __THINGSERVER_GATHERGODHOUSE_PART_H__

#include "IGatherGodHousePart.h"

struct IActor;

//聚仙楼数据
struct SGatherGodHouseData
{
	UINT32 m_LastFlushEmployTime; //最后一次刷新角色时间

	TEmployeeID m_idEmployee[FLUSH_NUM];  //记录玩家可以招募角色的角色ID,4个

	UINT32 m_LastFlushMagicTime;  //最后一次刷新法术书时间

	TMagicBookID m_idMagicBook[FLUSH_NUM]; //记录玩家可以购买法术书的ID
};

class GatherGodHousePart : public IGatherGodHousePart
{
public:
	GatherGodHousePart();

	virtual ~GatherGodHousePart();

public:

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

	//进入
	virtual void Enter();

	//查看
	virtual void ViewItem(CS_ViewItem_Req & Req);

	//刷新
	virtual void FlushItem(CS_FlushItem_Req & Req);

	//购买
	virtual void BuyItem(CS_BuyItem_Req & Req);

		//获得刷新招幕人员余下时间
	virtual INT32 GetFlushEmployRemainTime(); 

		//获得刷新法术书余下时间
	virtual INT32 GetFlushMagicBookRemainTime();

	//自动刷新招募角色
	virtual void  AutoFlushEmployee(UINT16 FlushNum, INT32 Aptitude);

private:
	//获得每次付费刷新需要消耗的仙石
	INT32 GetFlushMoney();

	//刷新招募角色数据
	void FlushEmployeeData(enFlushType FlushType);

	//刷新法术书数据
	void FlushMagicBookData(enFlushType FlushType);

	//购买招募角色,nIndex是第几个
	void BuyEmployee(UINT8 nIndex);

	//购买法术书
	void BuyMagicBook(UINT8 nIndex);

	//同步聚仙楼数据
    void SyncGatherGodData(enSellItemType SellItemType);

	//免费刷新招募角色
	void FreeFlushEmployee();

	//付费刷新招募角色
	void MoneyFlushEmployee();

	//免费刷新法术书
	void FreeFlushMagicBook();

	//付费刷新法术书
	void MoneyFlushMagicBook();

	//新建角色的特殊处理
	void NewUserSpecialCal();

private:
	IActor*	m_pActor;

	SGatherGodHouseData	m_GatherGodHouseData;

	INT16	m_nCDTimeFreeFlush;	  //聚仙楼免费刷新时间(秒)

	UINT16  m_SpendMoneyFlush;	  //聚仙楼付费刷新一次多少仙石
	UINT16	m_StoneFlushMagicBook;//聚仙楼付费刷新法术书一次多少灵石
};



#endif
