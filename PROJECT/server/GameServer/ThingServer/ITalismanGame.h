
#ifndef __THINGSERVER_ITALISMANGAME_H__
#define __THINGSERVER_ITALISMANGAME_H__

#include "TBuffer.h"
#include <vector>

struct IActor;

//法宝世界游戏
struct ITalismanGame
{
	//启动游戏
	virtual bool Start(std::vector<IActor*> & vectActor) = 0;

	//游戏消息
	virtual void OnMessage(IActor*,UINT8 nSubCmd,IBuffer & ib) = 0;

	//关闭游戏
	virtual void Close() = 0;

	//获得游戏ID
	virtual UINT32 GetGameID() = 0;


};







#endif

