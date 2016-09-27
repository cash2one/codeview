

#ifndef __THINGSERVER_MONSTER_H__
#define __THINGSERVER_MONSTER_H__

#include "IMonster.h"
#include "CreatureBase.h"


class Monster : public CreatureBase<IMonster>
{
	typedef  CreatureBase<IMonster>  Super;
public:
	Monster();
	virtual ~Monster();

public:

		virtual TMsgSourceType GetEventSrcType(){ return enEventSrcType_Monster;}

	virtual enThing_Class GetThingClass(void){ return enThing_Class_Monster; }

			//初始化，
	virtual bool Create(void);

	virtual void Release();




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


	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本生物的数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值，
	//////////////////////////////////////////////////////////////////////////
	virtual bool AddCrtPropNum(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor* pActor=0);
	
	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本生物的属性，替换原来的值
	// 输  入：数字型属性enPropID，属性值nValue
	//         属性同步标志enFlag
	// 返回值：成功pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetCrtProp(enCrtProp enPropID, INT32 nValue,INT32 * pNewValue=0,IActor* pActor=0);

		//恢复气血
	virtual void RecoverBlood(IActor * pActor=0); 


	//取得本生物的属性
	virtual INT32 GetCrtProp(enCrtProp enPropID);

		//获得物理伤害
	virtual INT32  GetPhysicalDamage() ;

	//获得法术伤害
	virtual INT32  GetMagicDamage(); 

	//获得防御值
	virtual INT32  GetDefense();

protected:
		INT32 __GetPropValue(enCrtProp PropID);

		bool __SetPropValue(enCrtProp PropID,int Value);

		//获得血上限
       INT32 GetBloodUp();

private:
	int  m_MonsterProp[enCrtProp_Monster_End-enCrtProp_Monster];

};


#endif

