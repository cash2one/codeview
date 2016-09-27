
#ifndef __THINGSERVER_MAGICPART_H__
#define __THINGSERVER_MAGICPART_H__

#include <hash_map>
#include "IMagicPart.h"

struct IActor;
struct IMagic;


class MagicPart : public IMagicPart
{
public:
	MagicPart();

	virtual ~MagicPart();

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


   	//打开法术栏就算
	virtual void OpenMagicPanel();

	//升级法术
	virtual void UpgradeMagic(TMagicID MagicID);

	//加载法术
	virtual void LoadMagic(TMagicID MagicID,UINT8 pos);

	//卸载法术
	virtual void UnloadMagic(TMagicID MagicID,UINT8 pos);

   //获得玩家已学会的法术
	IMagic * GetMagic(TMagicID MagicID);

	//玩家学会法术
	IMagic * StudyMagic(TMagicID MagicID);

		//获取已加载的法术
	virtual IMagic * GetLoadedMagic(UINT8 pos);

	//直接设置法术等级
	virtual bool SetMagicLevel(TMagicID MagicID, UINT8 level);


protected:
   IActor    * m_pActor;
   TMagicID    m_EquipMagic[MAX_EQUIP_MAGIC_NUM]; //法术栏已装备的法术,为零表示该栏为空
   typedef    std::hash_map<TMagicID,IMagic*>  MAP_MAGIC;

   MAP_MAGIC  m_mapMagic;  //已学会的法术列表



};



#endif
