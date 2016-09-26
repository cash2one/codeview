#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
namespace Api
{
	class Task
	{
	public:
		Task();
		~Task();
		static bool sg_get_task_list(int user_id);
		static bool sg_get_daily_task_list(int user_id);
		static bool sg_done_rookie_task(int user_id, int task_id);
		static bool sg_reward_rookie_task(int user_id, int task_id);
		static bool sg_done_interval_task(int user_id, int task_id);
		static bool sg_reward_interval_task(int user_id, int id);
		static bool done_bankruptcy_task(int user_id);
		static bool reward_bankruptcy_task(int user_id);

	};
}