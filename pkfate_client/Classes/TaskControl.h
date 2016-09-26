#pragma once
#include "cocos2d.h"

USING_NS_CC;
using namespace std;


	struct TaskStaticData{
		int task_id;
		string task_name;
		int coin;
	};

	struct TaskData{
		int task_id;
		int task_status;
		bool daily;
		TaskStaticData *staticData;
	};


	class TaskControl :public Ref
	{
	public:
		static TaskControl* GetInstance();
		void setTaskView(Node* node);
		void setLotteryView(Node* node);
		vector<TaskData*>& getTaskList(vector<TaskData*> &temp, bool daily = false);
		void requestAndCacheTask();
		bool isTaskNotDone(int task_id); //任务未做
		void validateTask(int task_id);//验证和处理任务
		bool isIntervalAwarding();//
		void validateLoginAward();//检测登录奖励
		bool isLoginAward();//检测是否有登录奖励
		bool isTaskAward();//检测是否有普通任务奖励
		double getIntervalAwardLeftTime();//获取剩余时间
		bool resetIntervalStart();//重置interval task
		void clearTaskList();//清除taskList;
		double startTime;
		unsigned short intervalTurn;// 累计轮数
		bool Init();		//从服务端返回的json数据，生成balance data
		bool RescueFailTip = false;


		void rescue();
		void rewardRescue();

	private:
		TaskControl();
		~TaskControl();
		Node *_lotteryView;
		Node *_taskView;
		Node *_taskItem; //当前点击btn所在taskItem
		int _taskId;// 当前处理的task_id
		int _userId;//当前userId

		vector<TaskData*> taskList;
		map<int, TaskStaticData*> taskStaticDic;
		bool read(string fileName);
		
		void updateTask(int task_id, int status);
		void onTriggerDoneRookieTask(Ref* pSender);
		void onTriggerDoneIntervalTask(Ref* pSender);
		void onGetRookieAwardClick(Ref *pSender);
		void onGetLotteryAwardClick(Ref *pSender); 
		void onGetTaskListCallBack(Ref *pSender);
		void onDoneRookieTaskCallBack(Ref *pSender);
		void onRewardRookieTaskCallBack(Ref *pSender);
		void onDoneIntervalTaskCallBack(Ref *pSender);
		void onRewardIntervalTaskCallBack(Ref *pSender);

		bool InitTaskData(std::string jsonData);
		bool dealDoneIntervalTask(std::string jsonData);
		bool dealRewardIntervalTask(std::string jsonData);

		void onDoneBankruptcyTaskCallBack(Ref *pSender);
		bool dealDoneBankruptcy(std::string jsonData);

		void onRewardBankruptcyTaskCallBack(Ref *pSender);
		bool dealRewardBankruptcyTask(std::string jsonData);


		void onGetDailyListCallBack(Ref *pSender);
		bool dealGetDailyList(std::string jsonData);
		


		void onTriggerRescue(Ref *pSender);
	};

