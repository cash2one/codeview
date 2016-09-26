#include "MessageQueue.h"

using namespace std;

//本messagequeu非线程安全
RECVMSG* MessageQueue::pop()
{ 	
	RECVMSG* message = nullptr;
	if(this->messageQueue.size()>0)
	{
		message = this->messageQueue.front();
		this->messageQueue.pop();		
	}
	
	
	return message;
}



void MessageQueue::push(RECVMSG* m)
{ 
	this->messageQueue.push(m);	
}

MessageQueue::~MessageQueue()
{
}

MessageQueue::MessageQueue()
{
}