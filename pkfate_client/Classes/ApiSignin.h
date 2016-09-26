#pragma once
#include "SocketControl.h"
using namespace std;
namespace Api
{
	class Signin
	{
	public:
		Signin();
		~Signin();
		static bool done_signin_task(int user_id);

		static bool reward_signin_task(int user_id);

		static bool get_signin_rewards();
	};
}