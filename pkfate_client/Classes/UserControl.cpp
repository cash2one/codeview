#include "UserControl.h"
#include "cJSON.h"
#include "comm.h"
#include "cmd.h"
#include "PKNotificationCenter.h"
#include "ApiGame.h"
#include "ApiUser.h"
#include "LayerLoading.h"
#include "LayerUserInfo.h"
#include "SocketControl.h"
#include "SceneLogin.h"
#include "ApiBill.h"

static UserControl *m_pInstance = nullptr;

UserControl::UserControl()
	:_userData(nullptr), infoView(nullptr)
{
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onGetUserInfoCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::GET_USER_INFO), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onGetGameLevelConfigCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::GAME_LEVEL_CONFIG), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onGetGameIndexCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::COUNT_GAME_INDEXES), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onGetGameStatCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_GAME_STAT), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onGetAvaliableGameCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_CHANNEL_AVALIABLE_GAMES), NULL);
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onTriggerLookFriend), "TriggerLookFriend", nullptr);//监听

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(UserControl::onRepeatLoginCallBack), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::REPEAT_LOGIN_NOTIFY), NULL);
	
	

}



void UserControl::onTriggerLookFriend(Ref* pSender)
{
	String* userID = (String*)pSender;
	if (!userID) return;
	LayerLoading::Wait();
	if (!Api::Game::get_game_stat(userID->intValue()))
	{
		LayerLoading::CloseWithTip("get_game_stat net error");
	}
}


UserControl::~UserControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (m_pInstance != nullptr)
	{
		if (m_pInstance->_userData)
			delete m_pInstance->_userData;
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
UserControl* UserControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new UserControl();
		
    }
		
	return m_pInstance;
}


//根据等级获取indexs
int UserControl::getIndexsByLevel(int level)
{
	if (level < 1 ){
		CCLOG(" level out of scope");
		return 1;
	}
	if (levelIndexs.size() == 0){
		return 1;
	}

	if (level > levelIndexs.size()){//满级
		
		return levelIndexs[levelIndexs.size()-1];
	}
	return levelIndexs[level-1];
}

//根据indexs 获取等级
int UserControl::getLevelByIndexs(int indexs)
{
	if (levelIndexs.size() <= 0) return 0;

	if (indexs <= levelIndexs[0]){
		return 1;
	}
	//从小到大顺序
	for (int i = 0, j = levelIndexs.size(); i < j; i++)
	{
		if (indexs < levelIndexs[i]){
			return i;
		}
	}
	
	return levelIndexs.size();
}


void UserControl::requestGameIndex()
{
	if (Api::Game::game_level_config()){
		if (!Api::Game::count_game_indexes()){
			CCLOG("Api::Game::count_game_indexes error");
		}
	}
	else{
		CCLOG("Api::Game::game_level_config error");
	}
	
}

void UserControl::requestGames()
{
	//int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	//LayerLoading::Wait();
	//if (!Api::Game::get_avaliable_games(user_id))
	//{
	//	LayerLoading::Close();
	//}

	if (!Api::Game::get_channel_avaliable_games(GetRegisterChannel()))
	{
		LayerLoading::Close();
	}
}

std::vector<GAMEINFO*> & UserControl::getGameInfos()
{
	return gameInfos;
}

void UserControl::onGetAvaliableGameCallBack(Ref*pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (InitGetAvaliableGame(msg->data))
			{
				//gameInfos.push_back(new GAMEINFO(GAMETYPE::SGJ, "SGJ", "develop"));
				//gameInfos.push_back(new GAMEINFO(GAMETYPE::CZD, "CZD", "develop"));
				/*gameInfos.push_back(new GAMEINFO(GAMETYPE::SGJ, "SGJ", "doudizhu"));
				gameInfos.push_back(new GAMEINFO(GAMETYPE::CZD, "CZD", "niuniu"));*/
				//gameInfos.push_back(new GAMEINFO(GAMETYPE::MORE, "MORE", "develop"));
				PKNotificationCenter::getInstance()->postNotification("UpdateSceneGameInfo");
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onGetGameLevelConfigCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onGetGameLevelConfigCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool UserControl::InitGetAvaliableGame(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	for (auto item : gameInfos){
		delete item;
	}
	gameInfos.clear();

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_String
			){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{
			GAMEINFO* info = getGameInfoByKey(item->valuestring);
			if (info != nullptr){
				gameInfos.push_back(info);
			}
		}
	}

	cJSON_Delete(jsonMsg);
	return true;
}

GAMEINFO* UserControl::getGameInfoByKey(string key)
{
	if (key == "bacarat"){
		return new GAMEINFO(GAMETYPE::BJL, "BJL", "bacarat");
	}
	if (key == "poker"){
		return new GAMEINFO(GAMETYPE::DZPK, "DZPK", "poker");
	}
	return nullptr;
}



void UserControl::onRepeatLoginCallBack(Ref* pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (InitRepeatLogin(msg->data))
			{
				//更新成功，通知其他进程

				ShowTip(Language::getStringByKey("LoginRepeat"), [=](){
					UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
					SocketControl::GetInstance()->close();
					auto scene = SceneLogin::create();
					Director::getInstance()->replaceScene(scene);
				}, nullptr, true);
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onRepeatLoginCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onRepeatLoginCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool UserControl::InitRepeatLogin(std::string jsonData)
{
	return true;
}




void UserControl::onBalanceUpdateCallBack(Ref* pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (InitBalanceUpdate(msg->data))
			{
			
				char postData[BUFSIZE];
				sprintf(postData, "{\"status\":%d}", 0);

				PKNotificationCenter::getInstance()->postNotification("PayResult", String::create(postData));//验证完成任务  第一次充值筹码
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onRepeatLoginCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onRepeatLoginCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool UserControl::InitBalanceUpdate(std::string jsonData)
{
	return true;
}

void UserControl::onGetGameStatCallBack(Ref*pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			map<int, GameData*> tempMap;
			if (InitGetGameStat(msg->data,tempMap))
			{
				//更新成功，通知其他进程
				((LayerUserInfo*)infoView)->updateGameDatas( tempMap);

				PKNotificationCenter::getInstance()->postNotification("UpdateUserGameData");
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onGetGameLevelConfigCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onGetGameLevelConfigCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool UserControl::InitGetGameStat(std::string jsonData, map<int, GameData*> &tempMap)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item, *bet_amount, *bet_amount_rank, *bonus, *bonus_rank, *game_type, *total_round, *win_round, *valid_bet_amount, *win_rate_rank;

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Object
			|| !(bet_amount = cJSON_GetObjectItem(item, "bet_amount")) || bet_amount->type != cJSON_Number
			|| !(bet_amount_rank = cJSON_GetObjectItem(item, "bet_amount_rank")) || bet_amount_rank->type != cJSON_Number
			|| !(bonus = cJSON_GetObjectItem(item, "bonus")) || bonus->type != cJSON_Number
			|| !(bonus_rank = cJSON_GetObjectItem(item, "bonus_rank")) || bonus_rank->type != cJSON_Number
			|| !(game_type = cJSON_GetObjectItem(item, "game_type")) || game_type->type != cJSON_Number
			|| !(total_round = cJSON_GetObjectItem(item, "total_round")) || total_round->type != cJSON_Number
			|| !(win_round = cJSON_GetObjectItem(item, "win_round")) || win_round->type != cJSON_Number
			|| !(valid_bet_amount = cJSON_GetObjectItem(item, "valid_bet_amount")) || valid_bet_amount->type != cJSON_Number
			|| !(win_rate_rank = cJSON_GetObjectItem(item, "win_rate_rank")) || win_rate_rank->type != cJSON_Number
		){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			GameData * gameData = new GameData();
			gameData->bet_amount = bet_amount->valueint;
			gameData->bet_amount_rank = bet_amount_rank->valueint;
			gameData->bonus = bonus->valueint;
			gameData->bonus_rank = bonus_rank->valueint;
			gameData->game_type = game_type->valueint;
			gameData->total_round = total_round->valueint;
			gameData->win_round = win_round->valueint;
			gameData->valid_bet_amount = valid_bet_amount->valueint;
			gameData->win_rate_rank = win_rate_rank->valueint;

			tempMap[gameData->game_type] = gameData;
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
	
}




void UserControl::onGetGameLevelConfigCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			if (InitLevelConfig(msg->data))
			{
				//更新成功，通知其他进程
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onGetGameLevelConfigCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onGetGameLevelConfigCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

//从服务端的json数据，生成userdata
bool UserControl::InitLevelConfig(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;


	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Number){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{
			levelIndexs.push_back(item->valueint);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}


void UserControl::onGetGameIndexCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//create msg
			if (_userData == nullptr)
				_userData = new UserData();

			int indexs = atoi(msg->data.c_str());

			int level = getLevelByIndexs(indexs);

			_userData->level = level;

			int nextLevelIndex = getIndexsByLevel(level + 1);

			_userData->expRate = indexs / nextLevelIndex;


		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onGetGameIndexCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void UserControl::onGetUserInfoCallBack(Ref *pSender)
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
			// init userdata
			if (InitUserData(msg->data))
			{
				//更新成功，通知其他进程
				PKNotificationCenter::getInstance()->postNotification("UpdateUserInfo");
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "UserControl::onGetUserInfoCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "UserControl::onGetUserInfoCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
//从服务端的json数据，生成userdata
bool UserControl::InitUserData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	//CCLOG(cJSON_Print(jsonMsg));
	cJSON *username = cJSON_GetObjectItem(jsonMsg, "username");
	if (username == nullptr || username->type != cJSON_String)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *status = cJSON_GetObjectItem(jsonMsg, "status");
	if (status == nullptr || status->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *user_id = cJSON_GetObjectItem(jsonMsg, "user_id");
	if (user_id == nullptr || user_id->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *parent_id = cJSON_GetObjectItem(jsonMsg, "parent_id");
	if (parent_id == nullptr || parent_id->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *login_type = cJSON_GetObjectItem(jsonMsg, "login_type");
	if (login_type == nullptr || login_type->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *create_time = cJSON_GetObjectItem(jsonMsg, "create_time");
	if (create_time == nullptr || create_time->type != cJSON_String)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	cJSON *last_login_time, *avatar, *nickname, *mobile, *zone, *email, *login_ip, *token, *json_children_num;
	if (!(last_login_time = cJSON_GetObjectItem(jsonMsg, "last_login_time")) ||
		!(avatar = cJSON_GetObjectItem(jsonMsg, "avatar")) ||
		!(nickname = cJSON_GetObjectItem(jsonMsg, "nickname")) ||
		!(mobile = cJSON_GetObjectItem(jsonMsg, "mobile")) ||
		!(zone = cJSON_GetObjectItem(jsonMsg, "zone")) ||
		!(email = cJSON_GetObjectItem(jsonMsg, "email")) ||
		!(login_ip = cJSON_GetObjectItem(jsonMsg, "login_ip")) ||
		!(json_children_num = cJSON_GetObjectItem(jsonMsg, "children_num")))
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	//create msg
	if (_userData == nullptr)
		_userData = new UserData();

	if ((token = cJSON_GetObjectItem(jsonMsg, "token")) && token->type == cJSON_String)
	{
		_userData->token = token->valuestring;
	}

	// 必选参数
	_userData->username = username->valuestring;
	_userData->status = status->valueint;
	_userData->user_id = user_id->valueint;
	_userData->parent_id = parent_id->valueint;
	_userData->create_time = create_time->valuestring;
	_userData->login_type = login_type->valueint;
	// 可选参数
	_userData->last_login_time = last_login_time->type == cJSON_String ? last_login_time->valuestring : "";
	_userData->zone = zone->type == cJSON_String ? zone->valuestring : "";
	_userData->mobile = mobile->type == cJSON_String ? mobile->valuestring : "";
	_userData->avatar = avatar->type == cJSON_Number ? avatar->valueint : -1;
	_userData->nickname = nickname->type == cJSON_String ? nickname->valuestring : "";
	_userData->email = email->type == cJSON_String ? email->valuestring : "";
	_userData->login_ip = login_ip->type == cJSON_String ? login_ip->valuestring : "";
	_userData->children_num = json_children_num->type == cJSON_Number ? json_children_num->valueint : 0;
	cJSON_Delete(jsonMsg);
	return true;
}


bool UserControl::isValidPhone()
{
	if (_userData){
		return !_userData->mobile.empty();
	}

	return false;
}

UserData* UserControl::GetUserData()
{
	return _userData;
}

void UserControl::setInfoView(Node* view)
{
	this->infoView = view;
}