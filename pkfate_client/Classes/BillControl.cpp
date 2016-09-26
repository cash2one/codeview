#include "BillControl.h"
#include "cJSON.h"
#include "comm.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "ApiBill.h"
#include "LayerLoading.h"
#include "UserControl.h"

static BillControl *m_pInstance = nullptr;
BillControl::BillControl()
{
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_BALANCE), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetLevelConfigCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_LEVEL_CONFIG), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceCallBack), GetMsgTypeString(MSGTYPE_TIME, MSGCMD_TIME::BALANCE_UPDATE_NOTIFY), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onListBalanceCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::LIST_BALANCE), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onBuyGoodsCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::BUY_GOODS), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceStateCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::GET_BALANCE_STAT), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onListBalanceNotification), "ListBalanceNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceNotification), "GetBalanceNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceGoldCardNotification), "GetBalanceGoldCardNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceSilverCardNotification), "GetBalanceSilverCardNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceDiamondCardNotification), "GetBalanceDiamondCardNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceRMBNotification), "GetBalanceRMBNotification", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(BillControl::onGetBalanceStateNotification), "GetBalanceStateNotification", nullptr);
}

BillControl::~BillControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_balanceData.size()>0)
	{
		for (auto data : _balanceData)
		{
			delete data.second;
		}
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}


BillControl* BillControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new BillControl();
	return m_pInstance;
}


void BillControl::requestCardBalance()
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (_balanceData.find(1) == _balanceData.end()){
		if (!Api::Bill::Get_Balance(user_id, 1)){
			LayerLoading::CloseWithTip("GET_BALANCE net error");//金卡
		}
	}

	if (_balanceData.find(2) == _balanceData.end()){
		if (!Api::Bill::Get_Balance(user_id, 2)){
			LayerLoading::CloseWithTip("GET_BALANCE net error");//银卡
		}
	}

	if (_balanceData.find(3) == _balanceData.end()){
		if (!Api::Bill::Get_Balance(user_id, 3)){
			LayerLoading::CloseWithTip("GET_BALANCE net error");//钻石卡
		}
	}

	if (_balanceData.find(4) == _balanceData.end()){
		if (!Api::Bill::Get_Balance(user_id, 4)){
			LayerLoading::CloseWithTip("GET_BALANCE net error");//钻石卡
		}
	}
}

void  BillControl::requestBalance()
{
	requestCardBalance();

	if (!Api::Bill::GET_BALANCE()){
		LayerLoading::CloseWithTip("GET_BALANCE net error");
	}
}

void BillControl::onGetBalanceCallBack(Ref *pSender)
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
			if (InitBalanceData(msg->data))
			{
				//更新成功，通知其他进程
				PKNotificationCenter::getInstance()->postNotification("UpdateBalance",nullptr);
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "BillControl::onGetBalanceCallBack", "update balance data error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "BillControl::onGetBalanceCallBack", "get balance data error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
//从服务端的json数据，生成balance data
bool BillControl::InitBalanceData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON *create_time, *update_time, *balance, *user_id, *currency_type;
	if (jsonMsg->type != cJSON_Object
		|| (update_time = cJSON_GetObjectItem(jsonMsg, "update_time")) == nullptr || update_time->type != cJSON_String
		|| (balance = cJSON_GetObjectItem(jsonMsg, "balance")) == nullptr || balance->type != cJSON_Number
		|| (user_id = cJSON_GetObjectItem(jsonMsg, "user_id")) == nullptr || user_id->type != cJSON_Number
		|| (currency_type = cJSON_GetObjectItem(jsonMsg, "currency_type")) == nullptr || currency_type->type != cJSON_Number
		|| (create_time = cJSON_GetObjectItem(jsonMsg, "create_time")) == nullptr || create_time->type != cJSON_String
		)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	/*
	cJSON *withdraw = cJSON_GetObjectItem(data, "withdraw");
	if (withdraw == nullptr || withdraw->type != cJSON_Number)
		continue;*/
	BalanceData *balancedata = _balanceData[currency_type->valueint];
	if (balancedata == nullptr)
	{
		balancedata = new BalanceData();
		_balanceData[currency_type->valueint] = balancedata;
	}
	
	balancedata->update_time = update_time->valuestring;
	balancedata->create_time = create_time->valuestring;
	balancedata->balance = balance->valuedouble;
	balancedata->user_id = user_id->valueint;
	balancedata->currency_type = currency_type->valueint;

	cJSON_Delete(jsonMsg);
	return true;
}

BalanceData* BillControl::GetBalanceData(int currency_type)
{
	return _balanceData[currency_type];
}

void BillControl::onGetLevelConfigCallBack(Ref *pSender)
{
	//get user info结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	if (msg->code == 0)
	{
		//成功
		// init userdata
		if (InitLevelConfigData(msg->data))
		{
			//NotificationCenter::getInstance()->postNotification("UpdateLevelConfig");	//更新成功，通知其他进程
		}
		else
		{
			//更新数据失败
			CCLOG("[%s]:%s\t%s", "BillControl::onGetLevelConfigCallBack", "update level data error.", msg->data.c_str());
		}
	}
	else
	{
		//失败
		CCLOG("[%s]:%s\t(code:%d) - %s", "BillControl::onGetLevelConfigCallBack", "get level data error.", msg->code, msg->data.c_str());
	}
}
bool BillControl::InitLevelConfigData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	int size;
	bool ret = false;
	//cJSON *outpour, *update_time, *balance, *user_id, *currency_type, *bonus, *inpour, *freeze, *commission, *create_time, *deposit, *rebate, *bet;
	if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg))>0)
	{
		_levelConfigs.clear();
		ret = true;
		cJSON *item;
		for (int i = 0; i < size; i++)
		{
			if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Number)
			{
				ret = false;
				break;
			}
			_levelConfigs.push_back((int64_t)item->valuedouble);
		}
	}
	cJSON_Delete(jsonMsg);
	return ret;
}

unsigned int BillControl::getBetLevel(double bet)
{
	vector<int64_t> *levels = &BillControl::GetInstance()->_levelConfigs;
	int size = levels->size();
	bool maxLevel = true;
	int level = 0;
	if (bet > 0 && size > 0)
	{
		
		for (level = 0; level < size; level++)
		{
			if (bet < levels->at(level))
			{
				maxLevel = false;
				level--;			//级别要降低一级，因为没有达到本级要求
				break;
			}
		}
		
	}
	return level;
}

void BillControl::onListBalanceCallBack(Ref *pSender)
{
	//get user info结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
}

void BillControl::onListBalanceNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::List_Balance(user_id))
	{
		LayerLoading::CloseWithTip("ListBalance net error");
	}
}

void BillControl::onGetBalanceNotification(Ref *pSender)
{
	if (!Api::Bill::GET_BALANCE()){
		LayerLoading::CloseWithTip("GET_BALANCE net error");
	}
}
void BillControl::onGetBalanceGoldCardNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::Get_Balance(user_id,1)){
		LayerLoading::CloseWithTip("GET_BALANCE net error");
	}
}
void BillControl::onGetBalanceSilverCardNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::Get_Balance(user_id,2)){
		LayerLoading::CloseWithTip("GET_BALANCE net error");
	}
}
void BillControl::onGetBalanceDiamondCardNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::Get_Balance(user_id,3)){
		LayerLoading::CloseWithTip("GET_BALANCE net error");
	}
}

void BillControl::onGetBalanceRMBNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::Get_Balance(user_id, 4))
	{
		LayerLoading::CloseWithTip("GET_BALANCERMB net error");
	}
}

void BillControl::onBuyGoodsCallBack(Ref *pSender)
{
	//get user info结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;

	PKNotificationCenter::getInstance()->postNotification("GetBalanceNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceGoldCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceSilverCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceDiamondCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetAllCoodsNotification");

	Tips("恭喜,购买成功!");

	LayerLoading::Close();
}

void BillControl::onGetBalanceStateCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	cJSON *jsonMsg, *amount, *business_type, *currency_type;
	jsonMsg = cJSON_Parse(msg->data.c_str());
	if (!jsonMsg || jsonMsg->type != cJSON_Object || 
		(amount = cJSON_GetObjectItem(jsonMsg, "amount")) == nullptr || 
		amount->type != cJSON_Number ||
		(business_type = cJSON_GetObjectItem(jsonMsg, "business_type")) == nullptr ||
		business_type->type != cJSON_Number ||
		(currency_type = cJSON_GetObjectItem(jsonMsg, "currency_type")) == nullptr ||
		currency_type->type != cJSON_Number)
		return;
	string key = StringUtils::format("%d%d", currency_type->valueint, business_type->valueint);
	map<string, int >::iterator l_it = _mapBalanceState.find(key);
	if (l_it != _mapBalanceState.end())
		_mapBalanceState.erase(l_it);
	_mapBalanceState.insert(pair<string, int>(key, amount->valueint));
}

void BillControl::onGetBalanceStateNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Bill::GET_BALANCE_STATE(user_id, 0, 2))
	{
		LayerLoading::CloseWithTip("GET_BALANCERMB net error");
	}
}

int BillControl::GetBalanceStateAmount(string key)
{
	int ret = _mapBalanceState[key];
	return ret;
}