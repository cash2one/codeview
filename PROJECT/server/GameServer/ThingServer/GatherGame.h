
#ifndef __THINGSERVER_GATHERGAME_H__
#define __THINGSERVER_GATHERGAME_H__

#include "TalismanGame.h"
#include <vector>

//宝物信息
struct SGatherGoodsInfo
{
	UINT16          m_DetectPoint;  //探测点,从零开始编号
	TGoodsID        m_GoodsID;  //宝物
	UINT16          m_GatherNum; //采集次数
};


class GatherGame : public TalismanGame
{
	typedef TalismanGame Super;

	enum enTimerID
	{
		enTimerID_Gather = Super::enTimerID_Max + 1,
	};
	
public:
	GatherGame(UID );
	virtual ~GatherGame();

public:

			//游戏消息
	virtual void OnMessage(IActor*,UINT8 nSubCmd,IBuffer & ib);


		//子类需要实现的接口

	//初始化客户端
	virtual void NotifyInitClient();

	//游戏开始
	virtual void GameStart();

	//游戏结束
	virtual void GameOver();

	virtual void OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint);

	//完成级别
	virtual UINT8 GetFinishLevel(INT32 & QualityPoint);

public:
	void OnTimer(UINT32 timerID);

	void	OnGatherTimer();

private:
	void OnDetect(IActor*,UINT8 nSubCmd,IBuffer & ib);

	void OnGather(IActor*,UINT8 nSubCmd,IBuffer & ib);

	//计算两个探测点的距离
    INT32 CalculateDistance(INT32 first,INT32 second);

	//启动采集定时器
	void StartTimer();

	//停止定时器
	void StopTimer();


private:
	
	INT16          m_MaxGatherNum;  //最大采集次数

	INT16           m_CurGatherIndex;       //当前在采集的宝物在m_vectBaoGoods中的索引

	std::vector<SGatherGoodsInfo>  m_vectBaoGoods; //宝物
};









#endif
