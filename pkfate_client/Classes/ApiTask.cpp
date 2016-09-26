#include "ApiTask.h"
#include "SocketControl.h"
#include "bjlcomm.h"
#include "cmd.h"
using namespace Api;
Task::Task()
{
}
Task::~Task()
{
}



bool Task::sg_get_daily_task_list(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_DAILY_TASK, args);
}

bool Task::sg_get_task_list(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_ROOKIE_TASKLIST, args);
}

bool Task::sg_done_rookie_task(int user_id,int task_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"task_id\":%d}", user_id, task_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_ROOKIE_TASK, args);
}

bool Task::sg_reward_rookie_task(int user_id, int task_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"task_id\":%d}", user_id, task_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_ROOKIE_TASK, args);
}


bool Task::sg_done_interval_task(int user_id, int task_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"task_id\":%d}", user_id, task_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_INTERVAL_TASK, args);
}

bool Task::sg_reward_interval_task(int user_id, int id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"task_id\":%d}", user_id, id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_INTERVAL_TASK, args);
}


bool Task::done_bankruptcy_task(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DONE_BANKRUPTCY_TASK, args);
}


bool Task::reward_bankruptcy_task(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_BANKRUPTCY_TASK, args);
}