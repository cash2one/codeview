

#ifndef __THINGSERVER_XIULIANGAME_H__
#define __THINGSERVER_XIULIANGAME_H__

#include <hash_map>
#include "TalismanGame.h"
#include "IConfigServer.h"
#include "GameSrvProtocol.h"

//描述气团信息
struct SAirMassInfo
{
	UINT32             m_AirMassID;
	UINT64             m_EndTime;  //存活到期时间
	enAirMassType      m_AirMassType;  //气团类型  
};

class XiuLianGame : public TalismanGame
{
	typedef TalismanGame Super;
	enum 
	{
		enTimerID_AirMass =  Super::enTimerID_Max +1,  //产生气团
	};
public:
	XiuLianGame(UID );
	virtual ~XiuLianGame();

public:

		void OnTimer(UINT32 timerID);

			//游戏消息
	virtual void OnMessage(IActor*,UINT8 nSubCmd,IBuffer & ib);


		//子类需要实现的接口

	//初始化客户端
	virtual void NotifyInitClient();

	//游戏开始
	virtual void GameStart();

	//游戏结束
	virtual void GameOver();

	//完成级别,返回完成级别和获得的品质点
	virtual UINT8 GetFinishLevel(INT32 & QualityPoint);

	virtual void OnNoticClientGameOver(UINT8 FinishLevel,UINT32 AdventureAwardID, INT32 QualityPoint);

private:
	//产生气团
    void  GenerateAirMass();

	void OnHitAirMass(IActor*,UINT8 nSubCmd,IBuffer & ib);  //击打气团

	void OnAirMassEndPoint(IActor*,UINT8 nSubCmd,IBuffer & ib);  //击打气团

	SAirMassInfo * GetAirMassInfo(UINT32 AirMassID);

	//预先生成时间轴的第几阶段的气团类型集合
	void  GenerateAirMassVect();

private:
	
	INT32         m_HitWhiteAirMassNum;  //击中的白气团数
	INT32         m_HitBlackAirMassNum;  //击中的黑气团数
	INT32         m_TotalWhiteAirMassNum; //总共的白气团数
	INT32         m_TotalBlackAirMassNum; //总共的黑气团数

	typedef  std::hash_map<UINT32,SAirMassInfo> MAP_AIRMASS;
	
	MAP_AIRMASS  m_mapAirMass;  //存活的气团

	std::vector<UINT64>  m_vectLastGenerateTime; //各地点最后产生气团时间

	std::vector<UINT16>  m_vectTimeShaft;		//时间轴,云团时间比例

	UINT16				 m_TimeSection;			//时间轴的下个阶段,比如：0~10秒为第0阶段，11~20秒第1阶段

	std::vector<enAirMassType> m_vectAirMassType;	//这个阶段的气团类型集合，按云团时间比例预先生成，后面直接从这随机取

};





#endif
