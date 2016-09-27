

#ifndef __THINGSERVER_IGOODS_H__
#define __THINGSERVER_IGOODS_H__

#include "IThing.h"

struct SGoodsCnfg;

struct IActor;

//物品种类
enum enGoodsCategory VC_PACKED_ONE
{
	enGoodsCategory_Common =0 ,      //普通物品
	enGoodsCategory_Equip =1,		//1 = 人物装备	
	enGoodsCategory_GodSword,        //仙剑
	enGoodsCategory_Talisman,          //法宝

	enGoodsCategory_Max,				//最大物品种类
} PACKED_ONE;

//普通物品二级分类
enum enGoodsSecondType VC_PACKED_ONE
{
	enGoodsSecondType_Common  = 0,			//普通
	enGoodsSecondType_Gem	  = 1,			//宝石
	enGoodsSecondType_Task	  = 2,			//任务物品
	enGoodsSecondType_Compose = 3,			//合成

	enGoodsSecondType_Max,					//二级分类最大物品种类
} PACKED_ONE;

//普通物品三级分类
/*0:普通消耗品
1:仙丹
2:法术
3:礼包
4:邀请令
5:人物卡
6:资源
7:特殊
8:分组消耗品
9:一般
13:灵件*/






//法宝二级分类
enum enTalismanType VC_PACKED_ONE
{
	enTalismanType_Out =0 ,      //外世界
	enTalismanType_In =1,		//1内世界	
	enTalismanType_Max,				//最大
} PACKED_ONE;

enum enGoodsProp 
{
	enGoodsProp_GoodsID = 0,		//0  = 物品ID
	enGoodsProp_Number,			//3  = 数量
	enGoodsProp_Bind,		     //4  = 绑定标志
	enGoodsProp_CreateTime,     // 6 = 生命期
	enGoodsProp_CommonParam,     //7通用扩展参数

	enGoodsProp_BaseEnd,
	
	//以下为装备专用
	enGoodsProp_Equip = 20,
	enGoodsProp_StarLevel,      //9  = 物品的星级
	enGoodsProp_Gem_ID1, //孔子1所镶嵌宝石 物品 ID
	enGoodsProp_Gem_ID2, //孔子2所镶嵌宝石 物品 ID
	enGoodsProp_Gem_ID3, //孔子3所镶嵌宝石 物品 ID
	enGoodsProp_ActiveSuit1, //激活套装属性1
	enGoodsProp_ActiveSuit2, //激活套装属性2
	enGoodsProp_EquipEnd,

	enGoodsProp_GodSword =100, //仙剑
	enGoodsProp_SwordLevel=101,    //仙剑级别
	enGoodsProp_Nimbus=102,         //灵气
	enGoodsProp_SecretLevel=103,    //剑诀等级  	
	enGoodsProp_FuBenLevel=104,       //副本等级
	enGoodsProp_KillNpcNum=105,       //杀怪数量
	enGoodsProp_LastEnterTime=106,    //最后一次进入副本时间
	enGoodsProp_MagicValue=107,       //法术伤害
	enGoodsProp_SwordkeeValue=108,    //剑气值
	enGoodsProp_GodSwordEnd,

	enGoodsProp_Talisman = 130, //法宝
	enGoodsProp_QualityPoint = 131,   //品质点
	enGoodsProp_GestateGoodsID = 132, //孕育的物品ID
	enGoodsProp_BeginGestateTime = 133,    //开始孕育时间	
	enGoodsProp_EnterNum =134,       //当天进入法宝世界次数
	enGoodsProp_MagicLevel = 135,       //法术级别
	enGoodsProp_bUseSpawnProp = 136,   //是否已使用过孕育丹
	enGoodsProp_GhostGoodsID = 137,   //法宝上的灵件的物品ID
	enGoodsProp_GhostLevel = 138,		//灵件等级

	enGoodsProp_TalismanEnd,

	enGoodsProp_Max				//最大值

};

//物品基本属性枚举ID映射物品私有属性排序编号
static INT32 GoodsPropMapID[] =
{
	1,3,4,-1,-1,
};

//装备属性枚举ID映射装备私有属性排序编号
static INT32 EquipmentPropMapID[] =
{
	6,7,8,9,10,11,
};

//仙剑属性枚举ID映射仙剑私有属性排序编号
static INT32 GodSwordPropMapID[] = 
{
	7,6,8,12,11,-1,9,10,
};

//法宝属性枚举ID映射法宝私有属性排序编号
static INT32 TalismanPropMapID[] = 
{
	6,7,8,12,13,-1,9,10,
};


struct IGoods : public IThing
{
	//取得配置信息
	virtual const SGoodsCnfg * GetGoodsCnfg() = 0;


	//获取分类ID
	virtual TGoodsID GetGoodsID() = 0;


	//取得物品的类别（例如：装备、消耗品）
	virtual enGoodsCategory GetGoodsClass(void) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：改变本物品数字型属性，将原来的值增加nValue
	// 输  入：数字型属性enPropID，属性值nValue（为负表示减去一个值），
	// 输  出：pNewValue返回改变后的新值

	//////////////////////////////////////////////////////////////////////////
	virtual bool AddPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：设置本物品数字型属性，将原来的值替换为nValue
	// 输  入：数字型属性enPropID，新属性值nValue，
	// 输  出：pNewValue返回改变后的新值
	//////////////////////////////////////////////////////////////////////////
	virtual bool SetPropNum(IActor * pActor,enGoodsProp enPropID, int nValue,int * pNewValue=0) = 0;

	//取得本物品的数字型属性
    virtual bool GetPropNum(enGoodsProp enPropID,int & nValue)=0;

	//获得物品数量
	virtual int GetNumber() = 0;

	//是否绑定
	virtual bool IsBinded() = 0;

	//得到是否需要更新到数据库
	virtual bool GetNeedUpdate() = 0;

	//设置是否需要更新到数据库
	virtual void SetUpdate(bool bNeedUpdate) = 0;
};


#endif

