#include "SigninControl.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "comm.h"
#include "ApiSignin.h"
#include "LayerLoading.h"
#include "UserControl.h"
#include "LayerSignin.h"
static SigninControl *m_pInstance = nullptr;

bool initSigninRewards(cJSON *jsonSigninRewards, std::vector<SRewards> & vec)
{
	if (jsonSigninRewards->type != cJSON_Array)
	{
		return false;
	}

	cJSON *jsonRewardsType, *jsonNumRewards;
	if (!(jsonRewardsType = cJSON_GetArrayItem(jsonSigninRewards, 0)) || jsonRewardsType->type != cJSON_Number
		|| !(jsonNumRewards = cJSON_GetArrayItem(jsonSigninRewards, 1)) || jsonNumRewards->type != cJSON_Number
		)
	{
		return false;
	}

	SRewards rewards;
	rewards.byRewardsType = (unsigned char)(jsonRewardsType->valueint);
	rewards.numRewards = jsonNumRewards->valueint;

	vec.push_back(rewards);

	return true;
}

bool initAllSigninRewards(std::string strAllSigninRewards, SigninControl::SigninRewardsMap & map)
{
	cJSON *jsonAllSigninRewards;
	if (!(jsonAllSigninRewards = cJSON_Parse(strAllSigninRewards.c_str())))
		return false;
	if (jsonAllSigninRewards->type != cJSON_Object)
	{
		cJSON_Delete(jsonAllSigninRewards);
		return false;
	}

	map.clear();

	int sizeAllJsonSigninRewards = cJSON_GetArraySize(jsonAllSigninRewards);
	for (int i = 1; i <= sizeAllJsonSigninRewards; i++)
	{
		char szKeySigninRewards[8];
		itoa(i, szKeySigninRewards, 10);
		cJSON *jsonTodaySigninRewards = cJSON_GetObjectItem(jsonAllSigninRewards, szKeySigninRewards);
		if (jsonTodaySigninRewards->type == cJSON_Array)
		{
			int sizeTodayJsonRewards = cJSON_GetArraySize(jsonTodaySigninRewards);
			for (int j = 0; j < sizeTodayJsonRewards; j++)
			{
				initSigninRewards(cJSON_GetArrayItem(jsonTodaySigninRewards, j), map[i-1]);
			}
		}
	}
	return true;
}

bool initObtainmentRewards(int day, SigninControl::SigninRewardsMap & map)
{
	if (day <= 1 || map.size() == 0)
		return false;

	if (day > 1)
	{
		for (int i = 0; i < day - 1; i++)
		{
			for (int j = 0; j < map[i].size(); j++)
			{
				map[i][j].bTake = true;
			}
		}
	}
	return true;
}

bool initDoneSigninData(std::string strDoneSigninData, SigninControl::SigninRewardsMap & map, int &day)
{
	cJSON *jsonDoneSigninData;
	if (!(jsonDoneSigninData = cJSON_Parse(strDoneSigninData.c_str())))
		return false;
	if (jsonDoneSigninData->type != cJSON_Array)
	{
		cJSON_Delete(jsonDoneSigninData);
		return false;
	}

	cJSON *jsonSigninDay = cJSON_GetArrayItem(jsonDoneSigninData, 0);
	if (jsonSigninDay->type == cJSON_Number)
	{
		day = jsonSigninDay->valueint;
	}

	initObtainmentRewards(day, map);

	cJSON *jsonIsTake = cJSON_GetArrayItem(jsonDoneSigninData, 2);
	bool isTake;
	if (jsonIsTake->type == cJSON_False)
	{
		isTake = false;
	}
	else if (jsonIsTake->type == cJSON_True)
	{
		isTake = true;
	}

	cJSON *jsonTodayRewards = cJSON_GetArrayItem(jsonDoneSigninData, 1);
	if (jsonTodayRewards->type == cJSON_Array)
	{
		int sizeRewards = cJSON_GetArraySize(jsonTodayRewards);
		for (int j = 0; j < sizeRewards; j++)
		{
			cJSON *jsonRewards = cJSON_GetArrayItem(jsonTodayRewards, j);
			if (jsonRewards->type == cJSON_Array)
			{
				unsigned char byRewardsTypeTemp = cJSON_GetArrayItem(jsonRewards, 0)->valueint;
				int numRewardsTemp = cJSON_GetArrayItem(jsonRewards, 1)->valueint;
				if (byRewardsTypeTemp == map[day-1][j].byRewardsType &&
					numRewardsTemp == map[day-1][j].numRewards)
				{
					map[day-1][j].bTake = isTake;
				}
			}
		}
	}

	return true;
}

SigninControl::SigninControl() :_pLayerSignin(NULL)
{
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onDoneSigninTaskCallBack), 
		GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DOWN_SIGNIN_TASK), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onRewardSigninTaskCallBack), 
		GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_SIGNIN_TASK), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onGetSigninRewardsCallBack), 
		GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_SIGNIN_REWARDS), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onRewardSigninTaskNotification), "RewardSigninTaskNotification", nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onDoneSigninTaskNotification), "DoneSigninTaskNotification", nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onClearLayerSigninNotification), "ClearLayerSigninNotification", nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onRefreshSigninDayNumberNotification), "RefreshSigninDayNumberNotification", nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SigninControl::onRefreshDayRewardsNotification), "RefreshDayRewardsNotification", nullptr);
}

SigninControl::~SigninControl()
{
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}

SigninControl* SigninControl::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new SigninControl();
	}
	return m_pInstance;
}

void SigninControl::onDoneSigninTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;

	if (msg->code != 0 || !initDoneSigninData(msg->data, _mapSigninRewards, _numSigninDay))
	{
		CCLOG("[%s]:%s\t%s", "SigninControl::onDoneSigninTaskCallBack", "onDoneSigninTaskCallBack error.", msg->data.c_str());
		return;
	}

	//test code _pLayerSignin
	//PKNotificationCenter::getInstance()->postNotification("OpenSigninUINotification", nullptr);

	if (!_mapSigninRewards[_numSigninDay-1][0].bTake)
	{
		PKNotificationCenter::getInstance()->postNotification("OpenSigninUINotification", nullptr);
	}
	else
	{
		PKNotificationCenter::getInstance()->postNotification("TriggerRequestNotice");
	}
}

void SigninControl::onRewardSigninTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;

	//Task already rewarded
	if (msg->code == 7003)
	{
		//test code _pLayerSignin
		//_pLayerSignin->showLayoutObtainment();
		//_pLayerSignin->refreshObtainmentRoot(_mapSigninRewards[_numSigninDay - 1]);
		return;
	}
	
	if (msg->code != 0)
	{
		CCLOG("[%s]:%s\t%s", "SigninControl::onGetSigninRewardsCallBack", "onGetSigninRewardsCallBack error.", msg->data.c_str());
		return;
	}
	else if (msg->code == 0 && _pLayerSignin)
	{
		_pLayerSignin->showLayoutObtainment();
		_pLayerSignin->refreshObtainmentRoot(_mapSigninRewards[_numSigninDay - 1]);
		LayerLoading::Close();
	}
}

void SigninControl::onGetSigninRewardsCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;

	if (msg->code != 0 || !initAllSigninRewards(msg->data, _mapSigninRewards))
	{
		CCLOG("[%s]:%s\t%s", "SigninControl::onGetSigninRewardsCallBack", "onGetSigninRewardsCallBack error.", msg->data.c_str());
		return;
	}
}

void SigninControl::requestSigninList()
{
	if (!Api::Signin::get_signin_rewards())
	{
		LayerLoading::CloseWithTip("GetSigninList net error");
	}
}

void SigninControl::onRewardSigninTaskNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Signin::reward_signin_task(user_id))
	{
		LayerLoading::CloseWithTip("RewardSigninTask net error");
	}
	LayerLoading::Wait();
}

void SigninControl::onDoneSigninTaskNotification(Ref *pSender)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::Signin::done_signin_task(user_id))
	{
		LayerLoading::CloseWithTip("DoneSigninTask net error");
	}
}

void SigninControl::onClearLayerSigninNotification(Ref *pSender)
{
	setLayerSignin(NULL);
}

void SigninControl::onRefreshSigninDayNumberNotification(Ref *pSender)
{
	if (_pLayerSignin == NULL)
		return;

	_pLayerSignin->setSigninDayNumber(_numSigninDay);
}

void SigninControl::setLayerSignin(LayerSignin* pLayerSignin)
{
	_pLayerSignin = pLayerSignin;
}

void SigninControl::onRefreshDayRewardsNotification(Ref *pSender)
{
	if (_pLayerSignin == NULL)
		return;

	_pLayerSignin->refreshDayRewards(_mapSigninRewards);
}