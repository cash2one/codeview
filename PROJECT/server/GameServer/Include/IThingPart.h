
#ifndef __THINGSERVER_ITHING_PART_H__
#define __THINGSERVER_ITHING_PART_H__

#include "DSystem.h"
#include "BclHeader.h"

//定义部件ID
enum enThingPart VC_PACKED_ONE
{
	enThingPart_Crt_Basic,			//生物的基础系统
	enThingPart_Crt_Status,         //状态
	enThingPart_Crt_Combat,         //战斗

	enThingPart_Crt_End,

	enThingPart_Actor,              //人物部件开始
    enThingPart_Actor_ResOutPut,    //资源产出部件
	enThingPart_Actor_FuMoDong,     //伏魔洞
	enThingPart_Actor_TrainingHall, //练功堂
	enThingPart_Actor_GatherGodHouse, //聚仙楼
	enThingPart_Actor_Packet,         //背包thingpart
	enThingPart_Actor_Equip,          //装备栏
	enThingPart_Actor_XiuLian,        //修炼
	enThingPart_Actor_Friend,		  //好友
	enThingPart_Actor_Mail,			  //邮件
	enThingPart_Actor_Magic,		  //法术部件
	enThingPart_Actor_FuBen,          //副本
	enThingPart_Actor_SynMagic,		  //帮派技能
	enThingPart_Actor_GodSwordShop,	  //剑冢
	enThingPart_Actor_Talisman,	      //法宝
	enThingPart_Actor_DouFa,          //斗法
	enThingPart_Actor_Task,           //任务
	enThingPart_Actor_ChengJiu,       //成就
	enThingPart_Actor_Activity,       //活动
	enThingPart_Actor_CDTimer,		  //冷却时间
	enThingPart_Actor_Team,			  //组队
	enThingPart_Actor_Syn,			  //帮派
	
	enThingPart_Crt_Max				// 最大值
} PACKED_ONE;  

struct IThing;

struct IThingPart
{

	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen) = 0;

	//释放
	virtual void Release(void) = 0;

	//取得部件ID
	virtual enThingPart GetPartID(void) = 0;

	//取得本身生物
	virtual IThing*		GetMaster(void) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen) = 0;

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient() =0;
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close() = 0;

	//保存数据
	virtual void SaveData() = 0;

protected:

	virtual	~IThingPart(){}


};














#endif

