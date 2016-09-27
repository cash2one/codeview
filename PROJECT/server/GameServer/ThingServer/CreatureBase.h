
#ifndef __THINGSERVER_CREATURE_BASE_H__
#define __THINGSERVER_CREATURE_BASE_H__

#include "ICreature.h"
#include "ThingBase.h"
#include "IThingPart.h"
#include "stdlib.h"
#include "IBasicService.h"
#include "IActor.h"


template <typename IBase = ICreature>
class CreatureBase : public TThingBase<IBase>
{
public:
	
	CreatureBase()
	{
		memset(&m_CreatureProp,0,sizeof(m_CreatureProp));
		memset(m_pThingParts,0,sizeof(m_pThingParts));
	}


	virtual ~CreatureBase()
	{
	
	}

	void Release()
	{
	  for(int i=0; i<ARRAY_SIZE(m_pThingParts);i++)
	  {
		 if(m_pThingParts[i] != 0)
		{
			m_pThingParts[i]->Release();
			m_pThingParts[i] = 0;
		}
	  }

	  delete this;
	}

		//获得位置
	virtual XPoint GetLoc()
	{
		XPoint point;
         point.x = GetCrtProp(enCrtProp_PointX);
		 point.y = GetCrtProp(enCrtProp_PointY);

		 return point;
	}

			//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen) { return false;}

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen){return false;}



	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本生物的数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0)
	{
		return SetCrtProp(enPropID,GetCrtProp(enPropID)+nValue,pNewValue,pActor);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本生物的属性，替换原来的值
	// 输  入：数字型属性enPropID，属性值nValue
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor * pActor=0)
	{
		if(enPropID>=enCrtProp_End)
		{
			TRACE("<WARM> %s : %d 意外的生物属性[%d]",__FUNCTION__,__LINE__,enPropID);
			return false;
		}

		
		if(__SetPropValue(enPropID,nValue)==false)
		{
			return false;
		}

		if(s_CreaturePropMapSave[enPropID])
		{
			this->SetSaveFlag(true);
		}
		
		INT32 NewValue = __GetPropValue(enPropID);

		if(pNewValue)
		{
			*pNewValue = NewValue ;
		}
	
		return true;
	}


	//取得本生物的属性

	virtual INT32 GetCrtProp(enCrtProp enPropID)
	{
		if(enPropID>=enCrtProp_End )
		{
			TRACE("<WARM> %s : %d 意外的生物属性[%d]",__FUNCTION__,__LINE__,enPropID);
			return 0;
		}

	
	
		return m_CreatureProp[enPropID];
	}

			//取得本实体的enPartID部件
	virtual IThingPart* GetPart(enThingPart enPartID)
	{
		if(enPartID<enThingPart_Crt_End)
		{
			return m_pThingParts[enPartID];
		}

		return 0;
	}

	//向本实体添加部件pPart。如果部件已存在，则会添加失败
	virtual bool AddPart(IThingPart *pPart) 
	{
		if(pPart==0)
		{
			return false;
		}

		enThingPart enPartID = pPart->GetPartID();

		if(enPartID<enThingPart_Crt_End)
		{
			m_pThingParts[enPartID] = pPart;
			return true;
		}

		return false;

	}

	//删除本实体的enPartID部件，但并不释放这个部件
	virtual bool RemovePart(enThingPart enPartID)
	{
		if(enPartID<enThingPart_Crt_End)
		{
			m_pThingParts[enPartID] = 0;
			return true;
		}

		return false;
	}

protected:

	INT32 __GetPropValue(enCrtProp PropID)
	{
		if(PropID<enCrtProp_End)
		{
			return m_CreatureProp[PropID];
		}
		return 0;
	}

	bool __SetPropValue(enCrtProp PropID,int Value)
	{
		if(PropID<enCrtProp_End)
		{
			m_CreatureProp[PropID] = Value;
			if(m_CreatureProp[PropID] <0)
			{
				m_CreatureProp[PropID] = 0;
			}
			return true;
		}
		return false;
	}

protected:
	int   m_CreatureProp[enCrtProp_End-enCrtProp_Min]; //生物属性

	IThingPart * m_pThingParts[enThingPart_Crt_End];

};














#endif
