
#ifndef __THINGSERVER_COMMONGOODS_H__
#define __THINGSERVER_COMMONGOODS_H__

#include "ICommonGoods.h"
#include "Goods.h"

class CommonGoods : public Goods<ICommonGoods>
{
	typedef Goods<ICommonGoods> Super;
public:
	CommonGoods();
	virtual ~CommonGoods();


public:

	virtual bool OnSetDBContext(const void * buf, int nLen);

		//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);


			//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen);
	virtual bool OnSetPrivateContext(const void * buf, int nLen);


	//获得三级类型
	virtual enGoodsThreeType  GetGoodsThreeType();

	//获得扩展数据
	virtual const void * GetExtendData();

	//获得扩展数据长度
	virtual int  GetExtendLen();

	//设置扩展数据
	virtual bool SetExtendLen(const void* pData,int len);

	//得到是否需要更新到数据库
	virtual bool GetNeedUpdate();

	//设置是否需要更新到数据库
	virtual void SetUpdate(bool bNeedUpdate);

private:


	SDBCommonGoodsProp m_CommonGoodsProp;
};




#endif

