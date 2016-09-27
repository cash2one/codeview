
#ifndef __THINGSERVER_GOODS_H__
#define __THINGSERVER_GOODS_H__

#include "IGoods.h"
#include "ThingBase.h"
#include "GameSrvProtocol.h"
#include "ThingServer.h"
#include "IGameServer.h"
#include "IConfigServer.h"
#include "IActor.h"

template<typename IBase>
class Goods : public TThingBase<IBase>
{
public:
	Goods()
	{
		memset(m_CommonGoodsProp,0,sizeof(m_CommonGoodsProp));
	}

	virtual ~Goods()
	{
	}

public:

	//初始化，
	virtual bool Create(void)
	{
		if(GetGoodsCnfg()==0)
		{
			return false;
		}

		//默认需要更新到数据
		m_bNeedUpdate = true;

		return true;
	}

	virtual void Release()
	{
		delete this;
	}

	//取得本实体对象的类型（大类，例如：玩家角色、NPC、物品等）
	virtual enThing_Class GetThingClass(void)
	{
		return enThing_Class_Goods;
	}

	virtual TMsgSourceType GetEventSrcType()
	{
		return enEventSrcType_Goods;
	}

	//取得配置信息
	const SGoodsCnfg * GetGoodsCnfg()
	{
		return g_pGameServer->GetConfigServer()->GetGoodsCnfg(GetGoodsID());
	}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen)
	{
		if(buf==0 || nLen<sizeof(SGoodsPublicData))
		{
			return false;
		}

		SGoodsPublicData * pPublicData = (SGoodsPublicData*)buf;

		pPublicData->m_GoodsClass = enGoodsCategory_Common;

		pPublicData->m_Binded = (__GetPropValue(enGoodsProp_Bind)!=0);

		pPublicData->m_CreateTime =  __GetPropValue(enGoodsProp_CreateTime);

		pPublicData->m_Number = __GetPropValue(enGoodsProp_Number);

		pPublicData->m_GoodsID = __GetPropValue(enGoodsProp_GoodsID);

		pPublicData->m_uidGoods = GetUID();

		memset(pPublicData->m_EquipProp.m_GemGoodsID,0,sizeof(pPublicData->m_EquipProp.m_GemGoodsID));		

		pPublicData->m_EquipProp.m_Star = 0;

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
	virtual bool OnGetPrivateContext(void * buf, int &nLen)
	{
		if(buf==0 || nLen<sizeof(SGoodsPrivateData))
		{
			return false;
		}

		SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;

		pPrivateData->m_GoodsClass = enGoodsCategory_Common;

		pPrivateData->m_Binded = __GetPropValue(enGoodsProp_Bind);

		pPrivateData->m_CreateTime =  __GetPropValue(enGoodsProp_CreateTime);

		pPrivateData->m_Number = __GetPropValue(enGoodsProp_Number);

		pPrivateData->m_GoodsID = __GetPropValue(enGoodsProp_GoodsID);

		pPrivateData->m_uidGoods = GetUID();

		memset(pPrivateData->m_EquipProp.m_GemGoodsID,0,sizeof(pPrivateData->m_EquipProp.m_GemGoodsID));		

		pPrivateData->m_EquipProp.m_Star = 0;


		nLen = sizeof(SGoodsPrivateData);

		return true;
	}

	virtual bool OnSetPrivateContext(const void * buf, int nLen) 
	{
		if(buf==0 || nLen<sizeof(SGoodsPrivateData))
		{
			return false;
		}


		SGoodsPrivateData * pPrivateData = (SGoodsPrivateData*)buf;

		__SetPropValue(enGoodsProp_Bind,pPrivateData->m_Binded);

		__SetPropValue(enGoodsProp_CreateTime,pPrivateData->m_CreateTime);

		__SetPropValue(enGoodsProp_Number,pPrivateData->m_Number);

		__SetPropValue(enGoodsProp_GoodsID,pPrivateData->m_GoodsID);

		SetUID(pPrivateData->m_uidGoods);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen)
	{
		if(buf==0 || nLen<sizeof(SDBGoodsData))
		{
			return false;
		}

		SDBGoodsData * pPrivateData = (SDBGoodsData*)buf;

		__SetPropValue(enGoodsProp_Bind,pPrivateData->m_Binded);

		__SetPropValue(enGoodsProp_CreateTime,pPrivateData->m_CreateTime);

		__SetPropValue(enGoodsProp_Number,pPrivateData->m_Number);

		__SetPropValue(enGoodsProp_GoodsID,pPrivateData->m_GoodsID);

		SetUID(pPrivateData->m_uidGoods);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen)
	{
		if(buf==0 || nLen<sizeof(SDB_Save_GoodsReq))
		{
			return false;
		}

		SDB_Save_GoodsReq * pDBGoosd  = (SDB_Save_GoodsReq *)buf;

		pDBGoosd->m_Binded				= __GetPropValue(enGoodsProp_Bind);

		pDBGoosd->m_CreateTime			= __GetPropValue(enGoodsProp_CreateTime);

		pDBGoosd->m_Number				= __GetPropValue(enGoodsProp_Number);

		pDBGoosd->m_GoodsID				= __GetPropValue(enGoodsProp_GoodsID);

		pDBGoosd->m_uidGoods			= GetUID().m_uid;

		nLen = sizeof(SDB_Save_GoodsReq);

		return true;
	}

	//获取分类ID
	virtual TGoodsID GetGoodsID()
	{
		return __GetPropValue(enGoodsProp_GoodsID);
	}


	//取得物品的类别（例如：装备、消耗品）
	virtual enGoodsCategory GetGoodsClass(void)
	{
		return enGoodsCategory_Common;
	}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本物品数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	// 输  出：pNewValue返回改变后的新值

	//////////////////////////////////////////////////////////////////////////
	virtual bool AddPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0)
	{
		INT32 nNewValue = 0;
		INT32 nOldValue = 0;

		GetPropNum(enPropID,nOldValue);

		if( SetPropNum(pActor,enPropID,nOldValue+nValue,&nNewValue)==false)
		{
			return false;
		}

		if(pNewValue)
		{
			*pNewValue = nNewValue;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本物品数字型属性，将原来的值替换为nValue
	// 输  入：数字型属性enPropID，新属性值nValue，
	// 输  出：pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0)
	{
		if(enPropID<0 || enPropID>=enGoodsProp_BaseEnd)
		{
			return false;
		}

		__SetPropValue(enPropID,nValue);

		INT32 newValue = __GetPropValue(enPropID);

		if(pNewValue)
		{
			*pNewValue = newValue;
		}

		if(pActor)
		{
			INT32 GoodsPropID = GoodsPropMapID[enPropID];
			if( -1 != GoodsPropID){
				pActor->NoticClientUpdateThing(GetUID(),GoodsPropID,newValue);
			}
		}

		return true;
	}

	//取得本物品的数字型属性
	virtual bool GetPropNum(enGoodsProp enPropID,int & nValue)
	{
		if(enPropID<0 || enPropID>=enGoodsProp_BaseEnd)
		{
			return false;
		}

		nValue = __GetPropValue(enPropID);
		return true;
	}

	//获得物品数量
	virtual int GetNumber()
	{
		return __GetPropValue(enGoodsProp_Number);
	}

	//是否绑定
	virtual bool IsBinded()
	{
		return __GetPropValue(enGoodsProp_Bind);
	}

	//得到是否需要更新到数据库
	virtual bool GetNeedUpdate()
	{
		return m_bNeedUpdate;
	}

	//设置是否需要更新到数据库
	virtual void SetUpdate(bool bNeedUpdate)
	{
		m_bNeedUpdate = bNeedUpdate;
	}

protected:
	INT32 __GetPropValue(enGoodsProp enPropID)
	{
		if(enPropID>=0 && enPropID<enGoodsProp_BaseEnd)
		{
			return m_CommonGoodsProp[enPropID];
		}

		return 0;
	}
	void __SetPropValue(enGoodsProp enPropID,INT32 nValue)
	{
		if(enPropID>=0 && enPropID<enGoodsProp_BaseEnd)
		{
			m_CommonGoodsProp[enPropID] = nValue;

			//属性有变化,需要保存
			m_bNeedUpdate = true;
		}
	}

private:
	int m_CommonGoodsProp[enGoodsProp_BaseEnd];  //

	bool m_bNeedUpdate;	//是否需要更新到数据库

};




#endif
