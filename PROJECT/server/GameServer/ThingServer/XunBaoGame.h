
#ifndef __THINGSERVER_XUNBAOGAME_H__
#define __THINGSERVER_XUNBAOGAME_H__

#include "TalismanGame.h"
#include <vector>

//宝物信息
struct SBaoGoodsInfo
{
	UINT16          m_DetectPoint;  //探测点,从零开始编号
	TGoodsID        m_GoodsID;  //宝物
	UINT16          m_PileNum; //叠加数
};


class XunBaoGame : public TalismanGame
{
	typedef TalismanGame Super;
	
public:
	XunBaoGame(UID );
	virtual ~XunBaoGame();

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

private:
	void OnDetect(IActor*,UINT8 nSubCmd,IBuffer & ib);

	

	//计算两个探测点的距离
    INT32 CalculateDistance(INT32 first,INT32 second);

private:

	UINT16          m_CurDetectedNum;  //当前已探测次数
	UINT16          m_ObtainGoodsNum;  //已获得宝物数量
	time_t          m_GameBegin;       //游戏开始时间

	std::vector<SBaoGoodsInfo>  m_vectBaoGoods; //宝物
};









#endif
