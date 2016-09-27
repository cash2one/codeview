
#ifndef __THINGSERVER_THINGSERVER_H__
#define __THINGSERVER_THINGSERVER_H__

#include "IThingServer.h"
#include "IGameServer.h"
#include "BuildingMgr.h"
#include "IEquipment.h"
#include "PacketMgr.h"
#include "EquipMgr.h"
#include "XiuLianMgr.h"
#include "FriendMgr.h"
#include "MailMgr.h"
#include "MainUIMgr.h"
#include "TalismanMgr.h"
#include "IRelationServer.h"
#include "TaskMgr.h"
#include "DouFaMgr.h"
#include "DMsgSubAction.h"
#include "ChengJiuMgr.h"
#include "ActivityMgr.h"
#include "WebInterface.h"
#include "TeamMgr.h"
#include "MiJingMgr.h"

extern IGameServer * g_pGameServer;

class ThingServer :public IThingServer,public IGMCmdHandler             
{
public:
	ThingServer();

	virtual ~ThingServer();

	bool Create();

	virtual void Close();

		//装载玩家入内存
	virtual bool LoadActor(UID uidActor); 

	virtual void OnHandleGMCmd(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam);

	//判断指定事件现场是否与向量一一毕配
	bool IsEqual(XEventData & EventData,UINT16 EeventID,const std::vector<INT32> & vectParam);

	//接收后台发来的消息
	virtual void WebOnRecv(UINT8 nCmd, IBuffer & ib);

public:
		//释放
	virtual void Release(void);

	virtual IGameWorld * GetGameWorld();

	XiuLianMgr & GetXiuLianMgr();

	BuildingMgr & GetBuildingMgr();

	TalismanMgr & GetTalismanMgr();

	TaskMgr & GetTaskMgr();

	FriendMgr & GetFriendMgr();

	MailMgr &	GetMailMgr();

	DouFaMgr &  GetDouFaMgr();

	TeamMgr &   GetTeamMgr();

	MainUIMgr & GetMainUIMgr();

	ActivityMgr & GetActivityMgr();

	//GM命令，设置等级
	void OnHandleGMCmdSetLv(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，设置部分属性
	void OnHandleGMCmdSetAtt(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，设置当前帮派等级
	void OnHandleGMCmdSetSynProp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，设置仙剑灵气
	void OnHandleGMCmdSetSK(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，资源快速成熟
	void OnHandleGMCmdSetCmprs(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，学会法术
	void OnHandleGMCmdStudyMagic(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，设置法术等级
	void OnHandleGMCmdMagicLvUp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，得到任务
	void OnHandleGMCmdGetTask(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，完成任务
	void OnHandleGMCmdFinishTask(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，清除今天挂机加速使用次数
	void OnHandleGMCmdClearAccellNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//GM命令，改变物品属性
	void OnHandleGMCmdChangeGoodsProp(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//清除玩家所在CD时间
	void OnHandleGMCmdClearCDTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
	//清除练功堂次数限制
	void OnHandleGMClearTrainNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);

	//清除副本次数限制
	void OnHandleGMClearFuBenNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);

	//设置服务器时间
	void OnHandleGMCmdSetTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);

		//获得服务器时间
	void OnHandleGMCmdGetTime(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);

	//清除挑战次数
	void OnHandleGMCmdChallengeNum(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);

	//设置VIP等级
	void OnHandleGMCmdVipLevel(IActor * pActor, enGMCmd Cmd, std::vector<std::string> & vectParam);
public:
	//杂项函数
	//获得装备具有的属性值，为零表示不具有该属性
    INT32 GetEquipProp(TGoodsID GoodsID,enEquipProp PropID);

private:
	//发送服务器时间给客户端
void SendServerTimeToClient(IActor * pActor);


private:
	IGameWorld * m_pGameWorld;
	BuildingMgr  m_BuildingMgr;
	PacketMgr    m_PacketMgr;
	EquipMgr     m_EquipMgr;
	XiuLianMgr   m_XiuLianMgr;
	FriendMgr	 m_FriendMgr;
	MailMgr		 m_MailMgr;
	MainUIMgr    m_MainUIMgr;
	TalismanMgr  m_TalismanMgr;
	TaskMgr      m_TaskMgr;
	DouFaMgr	 m_DouFaMgr;
	ChengJiuMgr  m_ChengJiuMgr;
	ActivityMgr  m_ActivityMgr;
	WebInterface m_WebInterface;
	TeamMgr		 m_TeamMgr;
	MiJingMgr	 m_MiJingMgr;

};

extern ThingServer * g_pThingServer;

#endif
