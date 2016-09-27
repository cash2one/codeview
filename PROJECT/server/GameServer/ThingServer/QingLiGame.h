#ifndef __THINGSERVER_QINGLIGAME_H__
#define __THINGSERVER_QINGLIGAME_H__

#include "TalismanGame.h"
#include "GameSrvProtocol.h"
#include <hash_map>
#include <vector>

struct SDiShuInfo
{	
	SDiShuInfo() : m_QingLiDiShuType(enQingLiDiShuType_Max), m_DiShuID(0){}
	enQingLiDiShuType m_QingLiDiShuType;	//地鼠类型
	UINT32			  m_DiShuID;			//地鼠ID
};

//在地图上地鼠的信息
struct SInMapDiShuInfo
{
	enQingLiDiShuType m_QingLiDiShuType;	//地鼠类型
	UINT32			  m_DiShuID;			//地鼠ID
	UINT8			  m_Pos;				//出现位置
	UINT8			  m_HitNum;				//被击打次数
	UINT32			  m_BornTime;			//刷新出来的时间
};

//可击打的位置信息
struct SPosInfo
{
	SPosInfo() : m_LastFlushTime(0), m_QingLiDiShuType(enQingLiDiShuType_Max){}
	UINT32				m_LastFlushTime;		//最后刷新出现地鼠的时间
	enQingLiDiShuType	m_QingLiDiShuType;		//地鼠类型
};

class QingLiGame : public TalismanGame
{
	typedef TalismanGame Super;
	enum 
	{
		enTimerID_BornDiShu =  Super::enTimerID_Max + 1,  //通知客户端产生地鼠
		enTimerID_IsDeleteTimer,							//判断地鼠是否删除 
	};
public:
	QingLiGame(UID uidTalisman);
	virtual ~QingLiGame();

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
	//创建游戏中的所有地鼠
	bool __CreateAllDiShu();

	//通知客户端产生一个地鼠
	void __NoticeClientCreateDiShu();

	//验证客户端发来的打中地鼠消息
	void __OnHitDiShuMass(IActor*,UINT8 nSubCmd,IBuffer & ib);

	//通知客户端删除地鼠
	void __NoticeClientDeleteDiShu(UINT32 DiShuID);

private:
	UINT16		m_HitDiShuGuaiNum;	//打中的地鼠怪个数
	UINT16		m_HitDiShuJingNum;	//打中的地鼠精个数
	UINT16		m_HitShuJingNum;	//打中的树精个数

	INT32		m_GetExp;			//获得的经验

	typedef std::hash_map<UINT32/*地鼠ID*/, SDiShuInfo> MAP_DISHUINFO;

	MAP_DISHUINFO	m_mapDiShuInfo;					//游戏开始时就把所有地鼠创建,每次直接从这里随机抽取就好了

	typedef std::hash_map<UINT32/*地鼠ID*/, SInMapDiShuInfo> MAP_INMAPDISHUINFO;
	MAP_INMAPDISHUINFO   m_mapNowDiShu;				//目前地图上的地鼠，即还在地图上的地鼠

	UINT16			m_CanHitNum;					//可击打位置数

	typedef std::vector<SPosInfo>					MAP_POSINFO;
	MAP_POSINFO		m_mapPosInfo;					//可击打位置记录
};


#endif
