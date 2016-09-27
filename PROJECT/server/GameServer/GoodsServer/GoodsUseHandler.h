
#ifndef __GOODSSERVER_GOODSUSEHANDLER_H__
#define __GOODSSERVER_GOODSUSEHANDLER_H__

#include "UniqueIDGenerator.h"
#include <string>

struct SGoodsUseCnfg;
struct IActor;
struct IGoods;

//物品使用函数原型
typedef INT32  (*GOODSUSE_FUNC)(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index);


//学习法术书
INT32 API_StudyMagicBook(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//打开礼包
INT32 API_OpenGiftPecket(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//法宝邀请令
INT32 API_OpenTalismanInvitation(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//人物卡
INT32 API_OpenActorCard(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//资源卡
INT32 API_OpenResourceCard(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//增加人物属性
INT32 API_AddActorProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//增加好友度
INT32 API_AddFriendRelation(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//增加物品属性
INT32 API_AddGoodsProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//增加帮派属性
INT32 API_AddGangProp(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//给玩家加状态
INT32 API_AddStatus(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//刷新任务
INT32 API_FlushTask(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//更改招募角色名称
INT32 API_ChangeEmployeeName(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//全服务公告
INT32 API_BroadcastToGameWorld(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//打开随机礼包
INT32 API_OpenRandomGiftPacket(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//物品合成
INT32 API_GoodsCompose(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

//需要单独做特殊处理的物品
INT32 API_SpecialTreatment(IActor* pActor,IGoods * pGoods,const SGoodsUseCnfg * pGoodsUseCnfg,UID uidTarget,INT32 & UseNum, std::string & strUserDesc, UINT8 Index = 0);

enum enGoodsUseHandlerType
{
	enGoodsUseHandlerType_OpenGiftPecket               = 1 , //打开礼包
	enGoodsUseHandlerType_OpenTalismanInvitation       = 2,   //法宝邀请令
	enGoodsUseHandlerType_OpenActorCard                = 3,   //人物卡
	enGoodsUseHandlerType_OpenResourceCard             = 4,   //资源卡
	enGoodsUseHandlerType_AddActorProp                 = 5,   //增加人物属性
	enGoodsUseHandlerType_AddFriendRelation            = 6,   //增加好友度
	enGoodsUseHandlerType_AddGoodsProp                 = 7,   //增加物品属性
	enGoodsUseHandlerType_AddGangProp                  = 8,   //增加帮派属性
	//enGoodsUseHandlerType_AddBuildingOutput            = 9,   //增加建筑产量
	//enGoodsUseHandlerType_AddTwoXLNimbus               = 10,  //双人修炼增加双倍灵气
	//enGoodsUseHandlerType_AddAloneXLNimbusAndSword     = 11,  //独自修炼增加双倍灵气及剑气
	enGoodsUseHandlerType_TimeNumAddAbility			   = 9,	  //一段时间内增加玩家的某种能力
	enGoodsUseHandlerType_FlushTask                    = 10,  //刷新任务
	enGoodsUseHandlerType_ChangeEmployeeName           = 11,  //更改招募角色名称
	enGoodsUseHandlerType_BroadcastToGameWorld         = 12,  //全服务公告
	enGoodsUseHandlerType_OpenRandomGiftPecket		   = 13,  //打开随机礼包
	enGoodsUseHandlerType_ComposeGoods				   = 14,  //合成物品
	enGoodsUseHandlerType_StudyMagicBook               = 15,  //学习法术书
	enGoodsUseHandlerType_SpecialTreatment			   = 16,  //需要单独做特殊处理的物品
	enGoodsUseHandlerType_Max,                               

};

static  GOODSUSE_FUNC s_FuncArray[enGoodsUseHandlerType_Max] = 
{
	NULL,

    //打开礼包
    API_OpenGiftPecket,

   //法宝邀请令
    API_OpenTalismanInvitation,

   //人物卡
    API_OpenActorCard,

    //资源卡
    API_OpenResourceCard,

   //增加人物属性
    API_AddActorProp,

   //增加好友度
    API_AddFriendRelation,

   //增加物品属性
    API_AddGoodsProp,

   //增加帮派属性
    API_AddGangProp,

	//一段时间内增加玩家的某种能力
	API_AddStatus,

    //刷新任务
    API_FlushTask,

    //更改招募角色名称
    API_ChangeEmployeeName,

    //全服务公告
    API_BroadcastToGameWorld,

	//打开随机礼包
	API_OpenRandomGiftPacket,

	//物品合成
	API_GoodsCompose,

	//学习法术书
    API_StudyMagicBook,

	//需要单独做特殊处理的物品
	API_SpecialTreatment,
};



class GoodsUseHandler
{
public:
	GoodsUseHandler();

	~GoodsUseHandler();

	bool Create();

	GOODSUSE_FUNC GetHandleFunc(enGoodsUseHandlerType Type);

private:
  
};





#endif

