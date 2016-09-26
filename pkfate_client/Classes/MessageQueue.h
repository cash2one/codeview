#pragma once

#include "comm.h"
#include <queue> 


class MessageQueue
{
public:
	void push(RECVMSG* message);
	RECVMSG* pop();
	MessageQueue();
	~MessageQueue();
	
public:
	std::queue<RECVMSG*> messageQueue;
};

