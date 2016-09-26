#include "TimeControl.h"
#include "cJSON.h"
#include "comm.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "SocketControl.h"

bool TimeControl::IS_PLAYING_GAME_IN_ROOM = false;

static TimeControl *m_pInstance = nullptr;
TimeControl::TimeControl()
{
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TimeControl::onRecvTime), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::RECVTIME), NULL);
	// scheduler
	Director::getInstance()->getScheduler()->schedule(schedule_selector(TimeControl::RequestServerTime), this, 60.0f, CC_REPEAT_FOREVER, 0.0f, false);
	Director::getInstance()->getScheduler()->schedule(schedule_selector(TimeControl::UpdateTime), this, 1.0f, CC_REPEAT_FOREVER, 0.0f, false);
	RequestServerTime(0.0f);
}


void TimeControl::StopUpdateTime()
{
	/*Director::getInstance()->getScheduler()->unschedule(schedule_selector(TimeControl::RequestServerTime));
	Director::getInstance()->getScheduler()->unschedule(schedule_selector(TimeControl::UpdateTime));*/
}

TimeControl::~TimeControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
TimeControl* TimeControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new TimeControl();
	return m_pInstance;
}
double TimeControl::GetServerTime()
{
	return _serverTime;
}
void TimeControl::onRecvTime(Ref *pSender)
{
	//get user info结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功
			// init timedata
			double clientTime, serverTime;
			if (InitTimeData(msg->data, &clientTime, &serverTime))
			{
				//解析成功
				_serverTime = serverTime + (getCurrentTime() - clientTime) / 2;	//将网络延迟也计算到时间里面，以获得精确时间
				//CCLOG("========RecvTime=========\ntime0:%.3f\tstime:%.3f\n===================", getCurrentTime(), _serverTime);
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "TimeControl::onRecvTime", "init time data error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "TimeControl::onRecvTime", "recv time data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
//从服务端的json数据，生成balance data
bool TimeControl::InitTimeData(std::string jsonData, double *clientTime, double *serverTime)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON *time0, *time1;
	if (jsonMsg->type != cJSON_Array || cJSON_GetArraySize(jsonMsg) != 2
		|| !(time0 = cJSON_GetArrayItem(jsonMsg, 0)) || time0->type != cJSON_Number
		|| !(time1 = cJSON_GetArrayItem(jsonMsg, 1)) || time1->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	*clientTime = time0->valuedouble;
	*serverTime = time1->valuedouble;
	cJSON_Delete(jsonMsg);
	return true;
}

double TimeControl::getCurrentTime()
{
	struct timeval tv;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year	 = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
#else
	gettimeofday(&tv, nullptr);
#endif
	double  time = tv.tv_sec + (double)tv.tv_usec / 1000000;
	return time;
}
void TimeControl::UpdateTime(float dt)
{
	_serverTime += dt;
	//CCLOG("=========UpdateTime========\ntime0:%.3f\tstime:%.3f\n===================", getCurrentTime(), _serverTime);
	//CCLOG("server time:%.3f",_serverTime);
}
void TimeControl::RequestServerTime(float dt)
{
	char args[BUFSIZE];
	sprintf(args, "%0.3f", getCurrentTime());
	if (!SocketControl::GetInstance()->SendMsg(MSGTYPE_TIME, MSGCMD_TIME::SENDTIME, args))
		CCLOG("TimeControl::GetServerTime\tsend time to server failed!");
}

int TimeControl::getCurrentUsecTime()
{
	struct timeval nowTimeval;
	gettimeofday(&nowTimeval, NULL);
	//struct tm * tm;
	//time_t time_sec;
	//time_sec = nowTimeval.tv_sec;
	//tm = localtime(&time_sec);
	//int currentTime = nowTimeval.tv_sec * 1000 + nowTimeval.tv_usec / 1000;
	//int nMinute = tm->tm_min;
	//int nSecond = tm->tm_sec;
	//tm->
	//int nHour = tm->tm_hour;

	return nowTimeval.tv_sec * 1000 + nowTimeval.tv_usec / 1000;
}