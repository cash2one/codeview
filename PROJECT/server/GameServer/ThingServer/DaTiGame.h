#ifndef __THINGSERVER_DATIGAME_H__
#define __THINGSERVER_DATIGAME_H__

#include "TalismanGame.h"
#include <vector>
#include "BclHeader.h"
#include "IConfigServer.h"

enum enTiMuType VC_PACKED_ONE
{
	enTiMuType_JXCQ,			//剑仙传奇知识
	enTiMuType_Game,			//游戏知识
	enTiMuType_XianXia,			//仙侠知识
	enTiMuType_ZheXue,			//哲学知识
	enTiMuType_NJJZW,			//脑筋急转弯

	enTiMuType_Max,
}PACKED_ONE;

//题目信息
struct STiMuInfo
{
	UINT16		m_TiMuID;		//题目ID
	enTiMuType	m_TiMuType;		//题目类型
};

class DaTiGame : public TalismanGame
{
	typedef TalismanGame Super;
	enum 
	{
		enTimerID_TimeOut =  Super::enTimerID_Max + 1,  //答题超时时间
	};
public:
	DaTiGame(UID uidTalisman);
	virtual ~DaTiGame();

public:
	void OnTimer(UINT32 timerID);

	//游戏消息
	virtual void OnMessage(IActor*,UINT8 nSubCmd,IBuffer & ib);

	//初始化客户端
	virtual void NotifyInitClient();

	//游戏开始
	virtual void GameStart();

	//游戏结束
	virtual void GameOver();

	virtual void OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint);

	//完成级别,返回完成级别和获得的品质点
	virtual UINT8 GetFinishLevel(INT32 & QualityPoint);

private:
	//用随机方法获得玩家要答的所有题目
	bool	__GetAllTiMu();

	//发送下一道题给客户端题目信息
	bool	__SendNextTiMuData();

	//得到题目信息
	const STiMuData * __GetTiMuData(STiMuInfo TiMuInfo);

	//答题
	void	__UserDaTi(IActor*,UINT8 nSubCmd,IBuffer & ib);

	//得到游戏奖励
	void	__GetGameAward();

private:
	std::vector<STiMuInfo> m_vectTiMu;	//玩家要回答的所有题目

	UINT16				   m_TotalRightTiMuNum;	//回答正解的问题数

	UINT16				   m_NowTiMuIndex;		//当前正在回答的问题索引(m_vectTiMu索引)

	UINT32				   m_GameBeginTime;		//游戏开始时间
};

#endif
