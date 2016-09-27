
#ifndef __THINGSERVER_ICOMMONGOODS_H__
#define __THINGSERVER_ICOMMONGOODS_H__


#include "IGoods.h"

/*
a)	消耗品
b)	仙丹
c)	法术书
d)	礼包
e)	法宝世界邀请令
f)	人物卡
g)	资源卡
h)	特殊
*/

//普通物品三级分类
enum enGoodsThreeType VC_PACKED_ONE
{
	enGoodsThreeType_Consume          = 0,            //普通消耗品
	enGoodsThreeType_XianDan          = 1,            //仙丹
	enGoodsThreeType_MagicBook        = 2,            //法术书
	enGoodsThreeType_GiftPecket		  = 3,			  //礼包
	enGoodsThreeType_TalismanInvite   = 4,            //法宝邀请令
	enGoodsThreeType_ActorCard        = 5,            //人物卡
	enGoodsThreeType_ResourceCard     = 6,            //资源卡
	enGoodsThreeType_RandomGiftPacket = 7,			  //藏宝图
	enGoodsThreeType_GoodsConsume     = 8,			  //分组消耗品
	enGoodsThreeType_Common			  = 9,			  //一般
	enGoodsThreeType_Special          = 10,           //特殊
	enGoodsThreeType_ChangYinFu		  = 11,			  //传音符
	enGoodsThreeType_ChangeName		  = 12,			  //角色更名卡
	enGoodsThreeType_Ghost			  = 13,			  //灵件
	enGoodsThreeType_Material		  = 14,			  //材料
	enGoodsThreeType_UpAptitude		  = 15,			  //提升资质的物品


	enGoodsThreeType_Max,					//三级分类最大物品种类
} PACKED_ONE;

#pragma pack(push,1)

//针对法宝邀请令的扩展数据结构
struct STalismanInviteData
{
	UID      m_uidTalisman;  //原法宝
};

#pragma pack(pop)


struct ICommonGoods : public IGoods
{
	//获得三级类型
	virtual enGoodsThreeType  GetGoodsThreeType() = 0;

	//获得扩展数据
	virtual const void * GetExtendData() = 0;

	//获得扩展数据长度
	virtual int  GetExtendLen() = 0;

	//设置扩展数据
	virtual bool SetExtendLen(const void* pData,int len) = 0;
};








#endif
