
#ifndef __THINGSERVER_TALISMANGAME_H__
#define __THINGSERVER_TALISMANGAME_H__

#include "ITalismanGame.h"
#include "ITimeAxis.h"
#include "UniqueIDGenerator.h"
#include "DSystem.h"

struct STalismanWorldCnfg;

//游戏状态
enum enTalismanGameState
{
	enTalismanGameState_Ready = 0, //准备
	enTalismanGameState_Start,
	enTalismanGameState_Over,
	enTalismanGameState_Max,
};

//奇遇奖励类型
enum enRewardType
{
	enRewardType_Goods = 0,			//奖励物品
};

class TalismanGame : public ITalismanGame, public ITimerSink
{
protected:
	enum 
	{
		enTimerID_Ready ,  //准备
		enTimerID_Over,    //游戏结束

		enTimerID_Max,  //最大值

	};
public:
	TalismanGame(UID uidTalisman);

	virtual ~TalismanGame();

public:
	void OnTimer(UINT32 timerID);

public:
		//启动游戏
	virtual bool Start(std::vector<IActor*> & vectActor);

	
	//关闭游戏
	virtual void Close();

	//获得游戏ID
	virtual UINT32 GetGameID(); 

	//子类需要实现的接口

	//初始化客户端
	virtual void NotifyInitClient() = 0;

	//游戏开始
	virtual void GameStart() = 0;


	//游戏结束
	virtual void GameOver() = 0;

	virtual void OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint)=0;


	//完成级别,返回完成级别和获得的品质点
	virtual UINT8 GetFinishLevel(INT32 & QualityPoint) = 0;

protected:
	//给奖励(级别)
	bool GiveFinishLevelAward(INT32 QualityPoint);

	//给奖励(奇遇)
	bool GiveFinishAdventureAward(UINT16 AdventureAwardID);

	//获得游戏状态
	enTalismanGameState GetGameState();

	//法宝界配置
	const STalismanWorldCnfg * GetTalismanWorldCnfg(); 

		//发送消息给客户端
	void SendGameMsgToClient(UINT8 SubCmd,void * buffer,INT32 len);


protected:

	std::vector<IActor*> m_vectActor;

	UINT32        m_GameID;  //游戏ID	

	UID m_uidTalisman;
	//游戏状态
	enTalismanGameState  m_GameState;

	const STalismanWorldCnfg * m_pTalismanWorldCnfg;
};




#endif

