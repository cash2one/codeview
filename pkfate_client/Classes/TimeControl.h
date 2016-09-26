#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;


class TimeControl :Ref
{
public:
	static bool IS_PLAYING_GAME_IN_ROOM;
	static TimeControl* GetInstance();
	double GetServerTime();
	void RequestServerTime(float dt);		//每分钟调用一次，更新服务端时间
	int getCurrentUsecTime();
private:
	double _serverTime;		//服务器时间
	double getCurrentTime();
	TimeControl();
	~TimeControl();
	void onRecvTime(Ref *pSender);
	bool InitTimeData(std::string jsonData, double *clientTime, double *serverTime);
	void UpdateTime(float dt);			//每秒调用，增加时间
	void StopUpdateTime();
	
};