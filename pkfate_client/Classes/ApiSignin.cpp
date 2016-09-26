#include "ApiSignin.h"
#include "cmd.h"
using namespace Api;
Signin::Signin()
{
}

Signin::~Signin()
{
}

bool Signin::done_signin_task(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::DOWN_SIGNIN_TASK, args);
}

bool Signin::reward_signin_task(int user_id)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", user_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::REWARD_SIGNIN_TASK, args);
}

bool Signin::get_signin_rewards()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_TASKLIST, MSGCMD_TASKLIST::GET_SIGNIN_REWARDS, "");
}