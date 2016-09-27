
#ifndef __GAMESERVER_ISESSION_H__
#define __GAMESERVER_ISESSION_H__

#include "ISocketSystem.h"
#include "UniqueIDGenerator.h"

struct IActor;

//会话状态
enum enPlayerSessionState
{
	enEnterSceneServerState,//准备进入场景服
	enGamePlayState,//正常游戏状态
	enLogoutState,//准备退出游戏状态
	enSwitchServerState, //切换服务器状态
	enPlayerSessionState_Max,
};

static const char *		g_sessionStateName[]=
{
	"进入态",
	"游戏态",
	"退出态",
	"换线",
};
//连接会话接口
struct ISession : public ISocketSink
{
	//关闭会话
	virtual	void KickUser( UINT8 KickType) = 0;

	//给玩家客户端发送数据
	virtual bool SendData(OStreamBuffer & osb) = 0;

	
	//切换到退出状态,
	virtual	void	Logout()=0;
    

	//返回角色的会话状态
	virtual enPlayerSessionState GetCurrentStateID() = 0;


	virtual	UINT32		GetSessionID()=0;

	//是否在线
	virtual bool IsOnLine() = 0;

	//装载玩家入内存
	virtual bool LoadActor(const UID & uidActor) = 0;

	//设置通讯套接字
	virtual void SetSocket(TSockID socketid) = 0;
	
};








#endif
