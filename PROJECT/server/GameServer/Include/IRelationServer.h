
#ifndef __RELATIONSERVER_IRELATIONSERVER_H__
#define __RELATIONSERVER_IRELATIONSERVER_H__


#include "DSystem.h"

#include "IGameServer.h"

#ifndef BUILD_RELATIONSERVER_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"RelationServer.lib")
#endif
#endif

struct ISyndicateMgr;
struct ITalk;
struct ISystemMsg;

enum enGMCmd VC_PACKED_ONE
{
	enGMCmd_AddGoods   = 0,  //增加物品,格式:/items,goodsid,num,starlevel
	enGMCmd_SetLv	   = 1,  //设置等级,格式:/lv,num  招募角色则要: /lv,num,名字
	enGMCmd_Money	   = 2,  //给仙石,格式:/money,num
	enGMCmd_Stone	   = 3,  //给灵石,格式:/stone,num
	enGMCmd_Att		   = 4,  //设置属性,格式:/att,type,num 招募角色则要:/att,type,num 名字
	enGMCmd_SynProp	   = 5,  //设置当前帮派等级,格式:/syn,propID, num   propID: 1为等级，2为贡献，3为经验  
	enGMCmd_Sk		   = 6,  //设置仙剑灵气,格式:/sk,num
	enGMCmd_Cmprs	   = 7,  //资源快速成熟,格式:/cmprs,建筑名称
	enGMCmd_StudyMagic = 8,	 //学习法术,格式:/magic,MagicID
	enGMCmd_MagicLvUp  = 9,	 //法术等级设置,格式:/magiclv,num
	enGMCmd_Ticket	   = 10, //给礼券,格式:/ticket,num
	enGMCmd_GetTask	   = 11, //得到任务,格式:/task,taskID
	enGMCmd_FinishTask = 12, //完成任务,格式:/taskok,taskID
	enGMCmd_ClearAccellNum  = 13,  //清除今天挂机加速使用次数,格式:/accell
	enGMCmd_ClearCDTime= 14, //清除玩家所有的CD时间:/clearCD
	enGMCmd_ChangeGoodsProp = 15,  //改变物品属性:/itemsatt,pos,propID,num
	enGMCmd_SetTime    = 16,         //设置服务器虚拟时间 /settime,month,day,hour,minute,不带参数，设置服务器时间与标准时间一至
	enGMCmd_GetTime     = 17,        //获取服务器当前虚拟时间/gettime
	enGMCmd_ClearTrainNum = 18,		 //清除练功堂次数限制
	enGMCmd_ClearFuBenNum = 19,		//清除副本次数限制
	enGMCmd_ChallengeNum  = 20,		//清除挑战次数
	enGMCmd_SetVipLevel   = 21,		//设置VIP等级

	enGMCmd_Max,
} PACKED_ONE;

enum enAttType VC_PACKED_ONE
{
	enAttType_Spirit		= 1,	//灵力
	enAttType_Shield		= 2,	//护盾
	enAttType_Blood			= 3,	//气血
	enAttType_Avoid			= 4,	//身法
	enAttType_ActorNimbus	= 5,	//灵气
	enAttType_Exp			= 6,	//经验
	enAttType_Honor			= 7,	//荣誉
	enAttType_Aptitude		= 8,	//资质
	enAttType_Credit		= 9,	//声望
	enAttType_PolyNimbus	= 10,	//聚灵气
	enAttType_GhostSoul		= 11,	//灵魄
	enAttType_ActorCrit		= 12,	//爆击
	enAttType_ActorTenacity	= 13,	//坚韧
	enAttType_ActorHit		= 14,	//命中
	enAttType_ActorDodge	= 15,	//回避
	enAttType_MagicCD		= 16,	//法术回复
	enAttType_GoldDamage	= 17,	//金剑诀伤害
	enAttType_WoodDamage	= 18,	//木剑诀伤害
	enAttType_WaterDamage	= 19,	//水剑诀伤害
	enAttType_FireDamage	= 20,	//火剑诀伤害
	enAttType_SoilDamage	= 21,	//土剑诀伤害

	enAttType_Max,
} PACKED_ONE;

//颜色
enum enColor VC_PACKED_ONE
{
	enColor_White = 0,		//白色
	enColor_Ret,			//红色
	enColor_Yellow,			//黄色
	enColor_Blue,			//蓝色
	enColor_Green,			//绿色
	enColor_Violet,			//紫色
	enColor_Orange,			//橙色

	enColor_Max,

}PACKED_ONE;

//颜色对应标签
static const char * szColor[enColor_Max] =
{
	"/0",		//白色
	"/1",		//红色
	"/2",		//黄色
	"/3",		//蓝色
	"/4",		//绿色
	"/5",		//紫色
	"/6",		//橙色
};

//GM命令处理者
struct IGMCmdHandler
{
	virtual void OnHandleGMCmd(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam) = 0;
};

struct IRelationServer
{
	//释放
	virtual void Release(void) = 0;

	virtual void Close()=0;

	virtual ISyndicateMgr * GetSyndicateMgr() = 0;

	//帮派招募成员
	virtual void	ZhaoMuSynMember(IActor * pActor) = 0;

	//注册GM命令处理者
	virtual bool RegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler) = 0;

	//取消注册GM命令处理者
	virtual bool UnRegisterGMCmdHanler(enGMCmd Cmd, IGMCmdHandler * Handler) = 0;

	//分发GM命令
	virtual void DispatchGMMessage(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam) = 0;

	virtual ITalk * GetTalk() = 0;

	//右上角的提示悬浮框
	virtual void  SendTipBox(IActor * pActor, const char * pszTip) = 0;

	virtual ISystemMsg * GetSystemMsg() = 0;

	//刷新帮战排行
	virtual void WarEndFlushRank() = 0;
};

BCL_API  IRelationServer * CreateRelationServer(IGameServer *pServerGlobal);




#endif

