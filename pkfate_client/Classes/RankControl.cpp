#include "RankControl.h"
#include "cJSON.h"
#include "comm.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "ApiBill.h"
#include "ApiGame.h"
#include "LayerRank.h"
#include "LayerLoading.h"
#include "BillControl.h"
using namespace Rank;

static RankControl *m_pInstance = nullptr;
RankControl::RankControl() :_rankView(nullptr)
{

}

//从服务端的json数据，生成balance data
bool RankControl::Init()
{  
	_rankConfig["1_1"] = getTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_FORTUNE_DAY_BILLBOARD);
	_rankConfig["1_2"] = getTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_FORTUNE_WEEK_BILLBOARD);
	_rankConfig["1_3"] = getTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_FORTUNE_MONTH_BILLBOARD);
	_rankConfig["1_4"] = getTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_FORTUNE_YEAR_BILLBOARD);

	_rankConfig["2_1"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_PROFIT_DAY_GAMEBOARD);
	_rankConfig["2_2"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_PROFIT_WEEK_GAMEBOARD);
	_rankConfig["2_3"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_PROFIT_MONTH_GAMEBOARD);
	_rankConfig["2_4"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_PROFIT_YEAR_GAMEBOARD);

	_rankConfig["3_1"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_WINRATE_DAY_GAMEBOARD);
	_rankConfig["3_2"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_WINRATE_WEEK_GAMEBOARD);
	_rankConfig["3_3"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_WINRATE_MONTH_GAMEBOARD);
	_rankConfig["3_4"] = getTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_WINRATE_YEAR_GAMEBOARD);


	for (auto config : _rankConfig)
	{ // 监听 排行数据
		CCLOG("config:%s", config.second.c_str());
		PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(RankControl::onGetRankCallBack), config.second, nullptr);
	}

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(RankControl::onRankSwitch), "RankSwitch", NULL);//监听来自视图的事件

	// 定点清除缓存
	const time_t t = time(NULL);
	int bjSecond = t % (24 * 60 * 60)+8*3600;//北京时间秒数
	int bjMinuts = bjSecond / 60;
	int bjHour = (bjMinuts / 60)%24;
	int extraMinuts =  bjMinuts / 60 * 60 -bjMinuts ;//超出的分钟数,精确到分钟
	int leftHour = 24 - bjHour;//差的小时数
	int leftSecond = leftHour * 3600 + extraMinuts * 60; //差的秒数  23:59分  23:58:59 ，误差60-1 = 59秒

	Director::getInstance()->getScheduler()->schedule(schedule_selector(RankControl::clearCacheAtPoint), this, leftSecond, false);
	return true;
}

//定点清理缓存
void RankControl::clearCacheAtPoint(float dt)
{
	Director::getInstance()->getScheduler()->unschedule(schedule_selector(RankControl::clearCacheAtPoint), this);//执行一次，免除连续24小时情况
	if (_rankDic.size()>0)
	{
		for (auto data : _rankDic)
		{
			vector<RankData*> vec = *data.second;
			for (RankData* var : vec)
			{
				delete var;
			}
			vec.clear();
			delete data.second;
		}
		_rankDic.clear();
	}
}

string RankControl::getTypeString(unsigned short type, unsigned short typesub)
{
	char str[10];
	sprintf(str, "%04x%04x", type, typesub);
	string stringStr = str;
	return stringStr;
}

void RankControl::onRankSwitch(Ref *pSender)
{
	//判断类型 

	std::string rankKey = ((String*)pSender)->getCString();

	if (rankKey.empty()){
		return; //do nothings
	}
	if (_rankConfig.find(rankKey) == _rankConfig.end()){
		return; // do nothings
	}
	string rankType = _rankConfig[rankKey];
	if (_rankDic.find(rankType) != _rankDic.end()){ //判断cache
		if (_rankView){
			((LayerRank*)_rankView)->UpdateRankList(_rankDic[rankType]);
		}
	}
	else
	{

		/*if (_rankView&&!_rankView->getChildByName("LayerLoading")){
			auto layerLoading = LayerLoading::create();
			layerLoading->setName("LayerLoading");
			_rankView->addChild(layerLoading);
		}*/

		LayerLoading::Wait();

		unsigned int sgType = 0;
		unsigned int sgSubType = 0; //int 读取 ，防止 sscanf 调用越界
		sscanf(rankType.c_str(), "%04x%04x", &sgType, &sgSubType);

		if (sgType == MSGTYPE_BILL)
		{
			Api::Bill::sg_bill_get_rank(sgSubType);//nocache  req server
		}else if(sgType == MSGTYPE_GAME)
		{
			Api::Game::sg_bill_get_rank(sgSubType);
		}
		else
		{
			// donothings
		}

	
		// 根据类型做对应的处理

	}

}



void RankControl::onGetRankCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();


	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		/*if (_rankView)
		{
			auto layerLoading = _rankView->getChildByName<LayerLoading*>("LayerLoading");
			if (layerLoading)
				layerLoading->removeFromParent();
		}*/
		
		LayerLoading::Close();

		if (msg->code == 0)
		{
			string rankType = getTypeString(msg->type, msg->typesub);
			if (InitRankData(msg->data, rankType)){
				if (_rankView){
					((LayerRank*)_rankView)->UpdateRankList(_rankDic[rankType]);
				}
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "onGetRankCallBack::onGetProfitReportCallBack", "init profit report data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "onGetRankCallBack::onGetProfitReportCallBack", "Get profit report data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}



bool RankControl::InitRankData(std::string jsonData, string rankType)
{

	if (jsonData.empty())
		return false;

	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	vector<RankData*> *rankVec = new vector<RankData*>();

	if (_rankDic.find(rankType) != _rankDic.end())
	{
		rankVec = _rankDic[rankType]; //clear cache
		for (RankData* var : *rankVec)
		{
			delete var;
		}
		(*rankVec).clear();
	}
	{
		_rankDic[rankType] = rankVec;
	}

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	cJSON *username, *user_id, *uuid, *bet_amount, *snapshot_date, *create_time, *avatar, *balance, *nickname, *profit, *winRate;


	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Object
			|| !(username = cJSON_GetObjectItem(item, "username")) || username->type != cJSON_String
			|| !(user_id = cJSON_GetObjectItem(item, "user_id")) || user_id->type != cJSON_Number
			|| !(snapshot_date = cJSON_GetObjectItem(item, "snapshot_date")) || snapshot_date->type != cJSON_String
			//|| !(bet_amount = cJSON_GetObjectItem(item, "bet_amount")) || bet_amount->type != cJSON_Number
			|| !(create_time = cJSON_GetObjectItem(item, "create_time")) || create_time->type != cJSON_String
			|| !(avatar = cJSON_GetObjectItem(item, "avatar")) || avatar->type != cJSON_Number

			|| !(nickname = cJSON_GetObjectItem(item, "nickname")) || nickname->type != cJSON_String
			){

			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			RankData *rankData = new RankData();
			rankData->avatar = avatar->valueint;
			rankData->user_id = user_id->valueint;
			rankData->nickname = nickname->valuestring;
			rankData->username = username->valuestring;

			
			if((balance = cJSON_GetObjectItem(item, "balance")) && balance->type == cJSON_Number)
			{
				rankData->balance = balance->valuedouble;
			}
			if ((profit = cJSON_GetObjectItem(item, "profit")) && profit->type == cJSON_Number)
			{
				rankData->profit = profit->valueint;
			}
			if ((winRate = cJSON_GetObjectItem(item, "winrate")) && winRate->type == cJSON_Number)
			{
				rankData->winRate = winRate->valuedouble*100;
			}
			//int betAmount = bet_amount->valueint; //获取 vip等级
			if ((bet_amount = cJSON_GetObjectItem(item, "bet_amount")) && bet_amount->type == cJSON_Number)
			{
				rankData->vip_level = BillControl::GetInstance()->getBetLevel(bet_amount->valuedouble);
			}
			(*rankVec).push_back(rankData);
			
			//处理vip level
		}
	}


	cJSON_Delete(jsonMsg);
	return true;
}


RankControl::~RankControl()
{
	// remove callback func
	Director::getInstance()->getScheduler()->unschedule(schedule_selector(RankControl::clearCacheAtPoint), this);
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_rankDic.size()>0)
	{
		for (auto data : _rankDic)
		{
			vector<RankData*> vec = *data.second;

            for (RankData* var : vec)
			{
				delete var;
			}
			vec.clear();
			delete data.second;
		}
		_rankDic.clear();
	}

	if (_rankConfig.size() >0){
		for (auto data : _rankConfig)
		{
			//delete data.second;
		}
		_rankConfig.clear();
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}





void RankControl::setRankView(Node* view)
{
	_rankView = view;
}


RankControl* RankControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new RankControl();
		m_pInstance->Init();
	}

	return m_pInstance;
}




