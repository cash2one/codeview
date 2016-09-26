#pragma once
//#include <map>
#include "comm.h"
#include "SocketClient.h"
#include "cJSON.h"
#include "MessageQueue.h"

class SocketControl:Ref
{
public:	
	~SocketControl();
	void OnMessage(SOCKETDATA *data);
	void OnDisconnected();
    void close();
	//bool SendMsg(std::string type, std::string data);
	//bool SendMsg(std::string type, std::string args);
	bool SendMsg(unsigned short type, unsigned short typesub, std::string data);
	static SocketControl *GetInstance();
private:
	SocketControl();
	
	MessageQueue* respQueue;
	void addMessageToReceiveQueue(RECVMSG* m);
	RECVMSG* getAndRemoveMessageFromReceiveQueue();
	void dispatchResponseCallbacks(float delta);

	bool init();
	bool isMessageEmpty;
	std::mutex socket_resp_mutex; /*初始化响应互斥锁*/
	SocketClient *mSocket;
	bool CheckConnect();
};
