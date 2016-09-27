
#ifndef __THINGSERVER_GODSWORD_H__
#define __THINGSERVER_GODSWORD_H__

#include "IGodSword.h"
#include "Goods.h"
struct IMagic;

class GodSword : public Goods<IGodSword>
{
	typedef Goods<IGodSword> Super;
public:
   GodSword();
   virtual ~GodSword();


public:

		//初始化，
	virtual bool Create(void) ;

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
	virtual bool OnGetPrivateContext(void * buf, int &nLen) ;
	virtual bool OnSetPrivateContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//取得物品的类别（例如：装备、消耗品）
	virtual enGoodsCategory GetGoodsClass(void);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本物品数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	// 输  出：pNewValue返回改变后的新值

	//////////////////////////////////////////////////////////////////////////
	virtual bool AddPropNum(IActor* pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本物品数字型属性，将原来的值替换为nValue
	// 输  入：数字型属性enPropID，新属性值nValue，
	// 输  出：pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetPropNum(IActor* pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0);

	//取得本物品的数字型属性
    virtual bool GetPropNum(enGoodsProp enPropID,int & nValue);

	//获得物品数量
	virtual int GetNumber();


		//获得剑气值
	int  GetSwordkee();

	//获得法术值
    int  GetMagicValue() ;

	  	//获得法术
	virtual IMagic * GetMagic(); 

		//检查生物pCreature是否能装备该项仙剑，返回TRUE表示可以装备
	virtual	bool CanEquip(ICreature *pCreature,UINT8 pos,std::string & strErr);

	//给生物pCreature装备该仙剑，返回TRUE表示装备成功
	virtual bool OnEquip(ICreature *pCreature);

	//给生物pCreature卸载该仙剑，返回TRUE表示卸载成功
	virtual bool UnEquip(ICreature *pCreature);

		//提升仙剑级别
	virtual bool UpgradeSword(IActor * pActor,INT8 value = 1) ;

	//得到是否需要更新到数据库
	virtual bool GetNeedUpdate();

	//设置是否需要更新到数据库
	virtual void SetUpdate(bool bNeedUpdate);

private:
	
	INT32 __GetPropValue(enGoodsProp enPropID);

	void __SetPropValue(enGoodsProp enPropID,INT32 nValue);

	//获得提升一级剑气值
	int  GetSwordkeeUpLevel(int Level);

	//获得提升一级法术值
    int  GetMagicUpLevel(int Level) ;

	//获得最大值
	int GetMaxValue(enGoodsProp enPropID);

	//重新设置剑气值
    void  SetSwordkee(IActor * pActor);

	//重新设置法术值
   void  SetMagicValue(IActor * pActor);


protected:
	INT32 m_GodSwordProp[enGoodsProp_GodSwordEnd-enGoodsProp_GodSword];

	IMagic * m_pMagic;

};







#endif
