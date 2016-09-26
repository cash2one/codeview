#include "TaskControl.h"
#include "cJSON.h"
#include "comm.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "ApiTask.h"
#include "ApiGame.h"
#include "LayerRank.h"
#include "LayerLoading.h"
#include "UserControl.h"
#include "LayerTask.h"
#include "ApiBill.h"
#include "TimeControl.h"
#include "LayerLottery.h"
#include "GuideControl.h"

#define TASK_JSON_FILE "static/task.json" 
#define INTERVAL_TIME  10*60
#define INTERVAL_TASK_MAX_NUM 12
static TaskControl *m_pInstance = nullptr;
TaskControl::TaskControl() :_taskView(nullptr), _lotteryView(nullptr), _taskItem(nullptr), _taskId(-3000), _userId(-1), startTime(-1), intervalTurn(0)
{

}

//从服务端的json数据，生成balance data
bool TaskControl::Init()
{  
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onGetTaskListCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_ROOKIE_TASKLIST), nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onGetDailyListCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_DAILY_TASK), nullptr);



	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onDoneRookieTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_ROOKIE_TASK), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onRewardRookieTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_ROOKIE_TASK), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onDoneIntervalTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_INTERVAL_TASK), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onRewardIntervalTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_INTERVAL_TASK), nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onGetRookieAwardClick), "GetRookieAwardClick", nullptr);//监听来自视图的事件
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onTriggerDoneRookieTask), "TriggerDoneRookieTask", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onTriggerDoneIntervalTask), "TriggerDoneInervalTask", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onGetLotteryAwardClick), "GetLotteryAwardClick", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onGetLotteryAwardClick), "TriggerRescue", nullptr);
	

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onDoneBankruptcyTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_BANKRUPTCY_TASK), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(TaskControl::onRewardBankruptcyTaskCallBack), GetMsgTypeString(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_BANKRUPTCY_TASK), nullptr);


	if (taskStaticDic.size() <= 0)
	{
		read(TASK_JSON_FILE); //读取静态文件
	}
	
	return true;
}



void TaskControl::rescue()
{

	if (RescueFailTip) return;
	if (!Api::Task::done_bankruptcy_task(UserControl::GetInstance()->GetUserData()->user_id))
	{
		CCLOG("Task::done_bankruptcy_task error");
		
	}; 
}


void TaskControl::rewardRescue()
{
	if (!Api::Task::reward_bankruptcy_task(UserControl::GetInstance()->GetUserData()->user_id))
	{
		CCLOG("Task::done_bankruptcy_task error");
	}; 
}


void TaskControl::onDoneBankruptcyTaskCallBack(Ref *pSender)
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
			if (dealDoneBankruptcy(msg->data))
			{
				
				ShowTip(StringUtils::format(Language::getStringByKey("RescueTip"), msg->data.c_str()), [=](){
					TaskControl::rewardRescue();
				}, nullptr, true);
			}
			else
			{
				
					//更新数据失败
				CCLOG("[%s]:%s\t%s", "TaskControl::onDoneBankruptcyTaskCallBack", "update userdata error.", msg->data.c_str());
				
			}
		}
		else
		{
			if (msg->code == 7004){
				RescueFailTip = true;
				ShowTip(StringUtils::format(Language::getStringByKey("RescueFailTip")), [=](){

				}, nullptr, true);
			}else
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onDoneBankruptcyTaskCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});

}



bool TaskControl::dealDoneBankruptcy(std::string jsonData)
{
	/*if (jsonData.empty()) //返回数据null
	return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
	return false;

	if (jsonMsg->type != cJSON_Array)
	{
	cJSON_Delete(jsonMsg);
	return false;
	}*/

	return true;
}



void TaskControl::onRewardBankruptcyTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();


		if (msg->code == 0)
		{
			if (dealRewardBankruptcyTask(msg->data))
			{
				Tips("领取成功");
			}
			else
			{
				//更新数据失败
				CCLOG("[%s]:%s\t%s", "TaskControl::onRewardBankruptcyTaskCallBack", "update userdata error.", msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onRewardBankruptcyTaskCallBack", "get user info error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool TaskControl::dealRewardBankruptcyTask(std::string jsonData)
{
	/*if (jsonData.empty()) //返回数据null
	return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
	return false;

	if (jsonMsg->type != cJSON_Array)
	{
	cJSON_Delete(jsonMsg);
	return false;
	}*/

	return true;
}


bool TaskControl::resetIntervalStart()
{
	startTime = TimeControl::GetInstance()->GetServerTime();
	//intervalTurn = 1;
	std::string intervalTurn_key = StringUtils::format("%d_IntervalTurn", UserControl::GetInstance()->GetUserData()->user_id);
	intervalTurn = UserDefault::getInstance()->getIntegerForKey(intervalTurn_key.c_str());
	return intervalTurn < INTERVAL_TASK_MAX_NUM;
}

double TaskControl::getIntervalAwardLeftTime()
{
	if (startTime == -1)
	{
		startTime = TimeControl::GetInstance()->GetServerTime();
	}
	double compare = TimeControl::GetInstance()->GetServerTime() - startTime;
	//double left = INTERVAL_TIME*intervalTurn - compare; // 每次轮数不一样算
	double left = INTERVAL_TIME - compare; // 每次轮数都是10分钟
	return left;
}

bool TaskControl::isIntervalAwarding()
{
	return getIntervalAwardLeftTime()<=0;
}


void TaskControl::requestAndCacheTask()
{
	clearTaskList();
	if (_userId != UserControl::GetInstance()->GetUserData()->user_id)
	{
		//startTime = TimeControl::GetInstance()->GetServerTime();
		if (!Api::Task::sg_get_task_list(UserControl::GetInstance()->GetUserData()->user_id))
		{
			CCLOG("Task::sg_get_task_list error");
			//requestAndCacheTask();
		}; //第一次初始化 拉取列表
		if (!Api::Task::sg_get_daily_task_list(UserControl::GetInstance()->GetUserData()->user_id))
		{
			CCLOG("Task::sg_get_task_list error");
			//requestAndCacheTask();
		}; //第一次初始化 拉取列表
	}
	//validateLoginAward();
}

void TaskControl::validateLoginAward()
{
	if (isLoginAward())
	{
		//通知 弹出
		PKNotificationCenter::getInstance()->postNotification("TriggerLoginAward");
	}
	else{
		
	}
}

bool TaskControl::isLoginAward()
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	std::string user_key = StringUtils::format("%d_LoginAwardTime", user_id);
	long loginAwardTime = UserDefault::getInstance()->getIntegerForKey(user_key.c_str());

	if (loginAwardTime <= 0)
	{
		return true;
	}

	time_t serverTime = TimeControl::GetInstance()->GetServerTime();
	struct tm *ptm_st = gmtime(&serverTime);
	if (ptm_st == nullptr) return false;

	char st[100] = { 0 };
	memset(st, 0x0, 100);
	strftime(st, sizeof(st), "%d", ptm_st);

	time_t lat = loginAwardTime;
	struct tm *ptm_lat = gmtime(&lat);
	char tmp_lat[100] = { 0 };
	memset(tmp_lat, 0x0, 100);
	strftime(tmp_lat, sizeof(tmp_lat), "%d", ptm_lat);

	CCLOG("from last loginAward  time:%f hour", (serverTime - loginAwardTime) / 3600);

	if (serverTime - loginAwardTime < 24 * 3600)
	{
		int s_d = String::create(st)->intValue();
		int l_d = String::create(tmp_lat)->intValue();
		if (serverTime >= loginAwardTime &&  s_d - l_d >= 1)
		{
			std::string intervalTurn_key = StringUtils::format("%d_IntervalTurn", user_id);
			UserDefault::getInstance()->setIntegerForKey(intervalTurn_key.c_str(), 0);//登录奖励的时候重置
			return true;
		}
	}
	else
	{
		return true;
	}
	return false;
}


bool TaskControl::isTaskNotDone(int task_id)
{
	for (int i = 0, j = taskList.size(); i < j; i++)
	{
		if (taskList[i]->task_id == task_id)
		{

			return taskList[i]->task_status == 0;
		}
	}

	return false;
}


//PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(taskData->task_id)));
void TaskControl::onTriggerDoneRookieTask(Ref* pSender)
{
	if (GuideControl::GetInstance()->checkInGuide()) return;
	
	if (!pSender) return;
	int task_id = ((String*)pSender)->intValue();
	_taskId = task_id;
	validateTask(_taskId);//验证和处理任务
}

void TaskControl::validateTask(int task_id)//验证和处理任务
{
	if (isTaskNotDone(task_id)){
		_taskId = task_id;
		if (!Api::Task::sg_done_rookie_task(UserControl::GetInstance()->GetUserData()->user_id, task_id))
		{
			CCLOG("Task::sg_done_rookie_task error");
		}; //无需管成功与否
	}
}

void TaskControl::clearTaskList()
{
	if (taskList.size() > 0)
	{
		for (TaskData* taskData : taskList)
		{
			delete taskData;
		}
		taskList.clear();
	}
}

void TaskControl::onDoneRookieTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			if (_taskId != -3000)
			{
				updateTask(_taskId, 1);//更新缓存,按钮可领取
			}
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onDoneRookieTaskCallBack", " error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}

void TaskControl::onGetRookieAwardClick(Ref* pSender)
{
	if (!pSender) return;
	_taskItem = (TaskItem*)pSender;
	/*LayerLoading* layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	_taskView->addChild(layerLoading);*/

	LayerLoading::Wait();

	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	
	if (!Api::Task::sg_reward_rookie_task(user_id, ((TaskItem*)_taskItem)->getTaskData()->task_id)){
		/*layerLoading->SetString(Language::getStringByKey("NetworkError"));
		layerLoading->btCancel->setVisible(true);*/
		LayerLoading::CloseWithTip(Language::getStringByKey("NetworkError"));

		Button* statusBtn = ((TaskItem*)_taskItem)->getStatusBtn();

		statusBtn->setEnabled(true);
		statusBtn->setBright(true);
	}
}



void TaskControl::onGetTaskListCallBack(Ref *pSender)
{
	////SetNickName回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	////使用主线程调用 没有刷新UI。
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{

			InitTaskData(msg->data);
			//验证完成任务 第一次使用PKFATE账号登陆,取巧办法，相当于第一次拉任务列表
			PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(1)));
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onGetTaskListCallBack", "task error.", msg->code, msg->data.c_str());
		}

		msg->release();
	});

}




bool TaskControl::InitTaskData(std::string jsonData)
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

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	cJSON *task_id, *status;
	
	

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Array
			|| !(task_id = cJSON_GetArrayItem(item, 0)) || task_id->type != cJSON_Number
			|| !(status = cJSON_GetArrayItem(item, 1)) || status->type != cJSON_Number
			){

			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			TaskData *taskData = new TaskData();
			taskData->task_id = task_id->valueint;
			taskData->task_status = status->valueint;
			taskData->daily = false;
			if (taskStaticDic.find(taskData->task_id) != taskStaticDic.end())
			{
				taskData->staticData = taskStaticDic[taskData->task_id];
			}
			taskList.push_back(taskData);
			//处理vip level
		}
	}
	PKNotificationCenter::getInstance()->postNotification("TriggerLotteryTask");//第一次拉取

	if (_taskView)
	{
		((LayerTask*)_taskView)->UpdateTaskList();
	}

	cJSON_Delete(jsonMsg);
	return true;
}

bool TaskControl::isTaskAward()//检测是否有普通任务奖励
{
	if (taskList.size() <= 0) return false;
	for (auto task : taskList){
		if (task->task_status == 1) return true;
	}
	return false;
}

void TaskControl::onRewardRookieTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		//auto layerLoading = _taskView->getChildByName<LayerLoading*>("LayerLoading");
		LayerLoading::Close();
		if (msg->code == 0)
		{
			/*if (layerLoading){
				layerLoading->removeFromParent();
			}*/
			if (_taskItem){
				
				Tips(StringUtils::format(Language::getStringByKey("AddChipsTip"), ((TaskItem*)_taskItem)->getTaskData()->staticData->coin));
				updateTask(((TaskItem*)_taskItem)->getTaskData()->task_id, 2);//更新缓存,领完奖励删除任务
			}

			Api::Bill::GET_BALANCE();		//刷新用户余额

		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onRewardRookieTaskCallBack", " error.", msg->code, msg->data.c_str());
			/*layerLoading->SetString(Language::getStringByKey("GetAwardError"));
			layerLoading->btCancel->setVisible(true);*/
			LayerLoading::CloseWithTip(Language::getStringByKey("GetAwardError"));
		}
		msg->release();
	});
}



void TaskControl::onGetLotteryAwardClick(Ref* pSender)
{
	if (!Api::Task::sg_done_interval_task(UserControl::GetInstance()->GetUserData()->user_id, 1000+1))
	{
		if (_lotteryView)
		{
			((LayerLottery*)_lotteryView)->lotteryError();
		}
		CCLOG("Task::sg_done_interval_task error");
	}; 
}


void TaskControl::onTriggerDoneIntervalTask(Ref* pSender)
{
	String* va = (String*)pSender;

	if (!va || va->intValue() <= 1000+6){
		if (!isIntervalAwarding()) return; //

		//int inteval_task_id = intervalTurn + 1;  //轮数+1
		int inteval_task_id = 1000 + 2;  //轮数+1

		if (!Api::Task::sg_done_interval_task(UserControl::GetInstance()->GetUserData()->user_id, inteval_task_id))
		{
			CCLOG("TaskControl::sg_done_interval_task error");
		}
	}
	else{
		this->_taskId = va->intValue();

		if (!Api::Task::sg_done_interval_task(UserControl::GetInstance()->GetUserData()->user_id, this->_taskId))
		{
			CCLOG("TaskControl::sg_done_interval_task error");
		}
	}
}

//完成时段任务
void TaskControl::onDoneIntervalTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			dealDoneIntervalTask(msg->data);
		}
		else
		{
			if (_lotteryView)
			{
				_lotteryView->removeFromParent();
			}
			else
			{
				startTime = TimeControl::GetInstance()->GetServerTime();//startTime 重置 
				PKNotificationCenter::getInstance()->postNotification("CurrentIntervalTaskFinish", String::create("0_0"));
			}
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onDoneIntervalTaskCallBack", " error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}


bool TaskControl::dealDoneIntervalTask(string jsonData)
{
	if (jsonData.empty())
		return false;

	cJSON *jsonMsg, *id;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array || !(id = cJSON_GetArrayItem(jsonMsg, 0)) || id->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	if (id->valueint <= 1000 + 6){

		if (!Api::Task::sg_reward_interval_task(UserControl::GetInstance()->GetUserData()->user_id, id->valueint)) // 直接领奖
		{
			if (_lotteryView)
			{
				((LayerLottery*)_lotteryView)->lotteryError();
			}


			CCLOG("Task::sg_reward_interval_task error");
		}

	}
	else{
		updateTask(id->valueint, 1);//更新缓存,领完奖励删除任务
	}

	cJSON_Delete(jsonMsg);
	return true;
}


void TaskControl::onRewardIntervalTaskCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			dealRewardIntervalTask(msg->data);
			//刷新用户余额
		}
		else//异常情况
		{
			if (msg->code == 7001){
				int user_id = UserControl::GetInstance()->GetUserData()->user_id;
				double serverTime = TimeControl::GetInstance()->GetServerTime();
				std::string user_key = StringUtils::format("%d_LoginAwardTime", user_id);
				UserDefault::getInstance()->setIntegerForKey(user_key.c_str(), serverTime);   //注释测试
			}

			if (_lotteryView)
			{
				_lotteryView->removeFromParent();
			}
			else
			{
				startTime = TimeControl::GetInstance()->GetServerTime();//startTime 重置 
				PKNotificationCenter::getInstance()->postNotification("CurrentIntervalTaskFinish", String::create("0_0"));
			}
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onRewardIntervalTaskCallBack", " error.", msg->code, msg->data.c_str());

		}
		msg->release();
	});
}

bool TaskControl::dealRewardIntervalTask(string jsonData)
{
	if (jsonData.empty())
		return false;

	cJSON *jsonMsg, *bonus;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (!(bonus = jsonMsg) || bonus->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}


	if (_lotteryView){//抽奖
		((LayerLottery*)_lotteryView)->lottery(bonus->valueint);
		int user_id = UserControl::GetInstance()->GetUserData()->user_id;
		double serverTime = TimeControl::GetInstance()->GetServerTime();
		std::string user_key = StringUtils::format("%d_LoginAwardTime", user_id);
		UserDefault::getInstance()->setIntegerForKey(user_key.c_str(), serverTime);   //注释测试
	}
	else{
			std::string intervalTurn_key = StringUtils::format("%d_IntervalTurn", UserControl::GetInstance()->GetUserData()->user_id);
			intervalTurn = UserDefault::getInstance()->getIntegerForKey(intervalTurn_key.c_str());
			intervalTurn = intervalTurn + 1;
			UserDefault::getInstance()->setIntegerForKey(intervalTurn_key.c_str(), intervalTurn);
			startTime = TimeControl::GetInstance()->GetServerTime();//startTime 重置 
			String* msg = String::createWithFormat("%d_%d", intervalTurn < INTERVAL_TASK_MAX_NUM ? 0 : 1, bonus->valueint);
			PKNotificationCenter::getInstance()->postNotification("CurrentIntervalTaskFinish", msg);
		
		//动画效果 coin 分发
	}
	Api::Bill::GET_BALANCE();

	cJSON_Delete(jsonMsg);
	return true;
}

static bool compare(TaskData* a, TaskData* b)
{
	return a->task_status>b->task_status; //升序排列，如果改为return a>b，则为降序
}


vector<TaskData*>& TaskControl::getTaskList(vector<TaskData*> &temp,bool daily)
{


	for (TaskData* taskData : taskList)
	{
		if (taskData->task_status != 2 && taskData->daily == daily)//已完成的不显示
		{
			temp.push_back(taskData);
		}
	}

	sort(temp.begin(), temp.end(), compare);//倒叙
	return temp;
}


void TaskControl::onGetDailyListCallBack(Ref *pSender)
{
	////SetNickName回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	////使用主线程调用 没有刷新UI。
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{

			dealGetDailyList(msg->data);
			//验证完成任务 第一次使用PKFATE账号登陆,取巧办法，相当于第一次拉任务列表
			PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(1)));
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "TaskControl::onGetTaskListCallBack", "task error.", msg->code, msg->data.c_str());
		}

		msg->release();
	});
}

bool TaskControl::dealGetDailyList(std::string jsonData)
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

	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	cJSON *task_id, *status;


	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Array
			|| !(task_id = cJSON_GetArrayItem(item, 0)) || task_id->type != cJSON_Number
			|| !(status = cJSON_GetArrayItem(item, 1)) || status->type != cJSON_Number
			){

			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			TaskData *taskData = new TaskData();
			taskData->task_id = task_id->valueint;
			taskData->task_status = status->valueint;
			taskData->daily = true;
			if (taskStaticDic.find(taskData->task_id) != taskStaticDic.end())
			{
				taskData->staticData = taskStaticDic[taskData->task_id];
			}
			taskList.push_back(taskData);
			//处理vip level
		}
	}
	PKNotificationCenter::getInstance()->postNotification("TriggerLotteryTask");//第一次拉取

	if (_taskView)
	{
		((LayerTask*)_taskView)->UpdateTaskList();
	}

	cJSON_Delete(jsonMsg);
	return true;
}

void TaskControl::updateTask(int task_id, int status)
{
	if (status > 2 || status < 0) return;

	vector<TaskData*>::iterator it;
	for (it = taskList.begin(); it != taskList.end();)
	{
		if ((*it)->task_id == task_id)
		{
			if (status == 2)
			{
				delete (*it);
				it = taskList.erase(it);
			}
			else
			{
				(*it)->task_status = status;
			}

			if (_taskView != nullptr)
			{
				((LayerTask*)_taskView)->UpdateTaskList();//刷新UI
			}

			break;//jump out
		}
		it++;

	}
	PKNotificationCenter::getInstance()->postNotification("TriggerLotteryTask");
}

bool TaskControl::read(string fileName)
{
	std::string jsonData;
	String* content = String::createWithContentsOfFile(fileName);
	if (!content)return false;
	jsonData = content->getCString();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	cJSON *id, *name, *coin;

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Array){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else
		{
			if (!(id = cJSON_GetArrayItem(item, 0)) || id->type != cJSON_Number
				|| !(coin = cJSON_GetArrayItem(item, 1)) || coin->type != cJSON_Number
				|| !(name = cJSON_GetArrayItem(item, 2)) || name->type != cJSON_String
				){
				cJSON_Delete(jsonMsg);
				return false;
			}

			TaskStaticData* taskStaticData = new TaskStaticData();
			taskStaticData->coin = coin->valueint;
			taskStaticData->task_name = name->valuestring;
			taskStaticData->task_id = id->valueint;
			taskStaticDic[taskStaticData->task_id] = taskStaticData;

		}
	}

	cJSON_Delete(jsonMsg);//delete
	return true;
}

TaskControl::~TaskControl()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	
	vector<TaskData*> taskList;
	map<int, TaskStaticData*> taskStaticDic;

	if (taskStaticDic.size()>0)
	{
		for (auto data : taskStaticDic)
		{
			delete data.second;
		}
		taskStaticDic.clear();
	}

	for (TaskData* data : taskList)
	{
		delete data;
	}
	taskList.clear();


	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}




void TaskControl::setTaskView(Node* view)
{
	_taskView = view;
}


void TaskControl::setLotteryView(Node* view)
{
	_lotteryView = view;
}

TaskControl* TaskControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new TaskControl();
		m_pInstance->Init();
	}

	return m_pInstance;
}




