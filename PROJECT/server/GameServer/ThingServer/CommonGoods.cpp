
#include "CommonGoods.h"
#include "IBasicService.h"

CommonGoods::CommonGoods() 
{
	m_CommonGoodsProp.m_Ghost.m_GhostLevel = 1;
	memset(&m_CommonGoodsProp,0,sizeof(m_CommonGoodsProp));
}
CommonGoods::~CommonGoods() 
{
}

bool CommonGoods::OnSetDBContext(const void * buf, int nLen) 
{
	if(buf==0 || nLen<sizeof(SDBGoodsData))
	{
		return false;
	}

	if(Super::OnSetDBContext(buf, nLen) == false)
	{
		return false;
	}

	const SDBGoodsData * pDBGoodsData = (const SDBGoodsData *)buf;
	SGhostProp GhostProp;

	GhostProp.m_GhostLevel = pDBGoodsData->m_CommonProp.m_Ghost.m_GhostLevel;
	this->SetExtendLen(&GhostProp,sizeof(GhostProp));
	m_CommonGoodsProp = pDBGoodsData->m_CommonProp;

	
	return true;


}

//////////////////////////////////////////////////////////////////////////
// 描  述：将本实体的数据保存到数据库
// 输  入：保存数据的缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
//////////////////////////////////////////////////////////////////////////
bool CommonGoods::OnGetDBContext(void * buf, int &nLen) 
{
	if(buf==0 || nLen<sizeof(SDB_Save_GoodsReq))
	{
		return false;
	}

	if(Super::OnGetDBContext(buf, nLen) == false)
	{
		return false;
	}

	SDB_Save_GoodsReq * pDBGoodsData = ( SDB_Save_GoodsReq *)buf;

    const SGhostProp * pGhostProp = (SGhostProp *)this->GetExtendData();
	m_CommonGoodsProp.m_Ghost.m_GhostLevel = pGhostProp->m_GhostLevel;
	memcpy(&pDBGoodsData->GoodsData, &m_CommonGoodsProp, sizeof(pDBGoodsData->GoodsData));

	return true;
}

		//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
 bool CommonGoods::OnGetPublicContext(void * buf, int &nLen)
 {
	if(buf==0 || nLen<sizeof(SGoodsPublicData))
	{
		return false;
	}

	if(Super::OnGetPublicContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPublicData * pPublicData = (SGoodsPublicData *)buf;


	memcpy(&pPublicData->m_CommonProp,this->m_CommonGoodsProp.m_ExtendData,sizeof(pPublicData->m_CommonProp));

	nLen = sizeof(SGoodsPublicData);

	return true;
 }

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
bool CommonGoods::OnGetPrivateContext(void * buf, int &nLen)
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnGetPrivateContext(buf,nLen)==false)
	{
		return false;
	}

	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;
    const SGhostProp * pGhostProp = (SGhostProp *)this->GetExtendData();
	pPrivateData->m_CommonProp.m_Ghost.m_GhostLevel = pGhostProp->m_GhostLevel;
	memcpy(&pPrivateData->m_CommonProp,this->m_CommonGoodsProp.m_ExtendData,sizeof(pPrivateData->m_CommonProp));
	nLen = sizeof(SGoodsPrivateData);

	return true;

	
}


bool CommonGoods::OnSetPrivateContext(const void * buf, int nLen)
{
	if(buf==0 || nLen<sizeof(SGoodsPrivateData))
	{
		return false;
	}

	if(Super::OnSetPrivateContext(buf,nLen)==false)
	{
		return false;
	}
		
	SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;
   SGhostProp GhostProp;

   GhostProp.m_GhostLevel = pPrivateData->m_CommonProp.m_Ghost.m_GhostLevel;

   this->SetExtendLen(&GhostProp,sizeof(GhostProp));

	memcpy(this->m_CommonGoodsProp.m_ExtendData,&pPrivateData->m_CommonProp,sizeof(m_CommonGoodsProp.m_ExtendData));

	return true;
}



//获得三级类型
enGoodsThreeType  CommonGoods::GetGoodsThreeType()
{
	return (enGoodsThreeType)GetGoodsCnfg()->m_ThirdClass;
}

//获得扩展数据
const void * CommonGoods::GetExtendData() 
{
	return m_CommonGoodsProp.m_ExtendData;
}


//获得扩展数据长度
int  CommonGoods::GetExtendLen()
{
	return sizeof(m_CommonGoodsProp.m_ExtendData);
}

//设置扩展数据
bool CommonGoods::SetExtendLen(const void* pData,int len)
{
	if(len>sizeof(m_CommonGoodsProp))
	{
		return false;
	}

	memcpy(&m_CommonGoodsProp,pData,len);

	Super::SetUpdate(true);

	return true;
}







//得到是否需要更新到数据库
bool CommonGoods::GetNeedUpdate()
{
	return Super::GetNeedUpdate();
}

//设置是否需要更新到数据库
void CommonGoods::SetUpdate(bool bNeedUpdate)
{
	Super::SetUpdate(bNeedUpdate);
}
