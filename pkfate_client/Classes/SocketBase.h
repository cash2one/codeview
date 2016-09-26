﻿#pragma once
#include "cocos2d.h"
#include <list>
#include <thread>
USING_NS_CC;
// 对于windows平台
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "WS2_32.lib")
#define HSocket SOCKET
// 对于android,ios平台
#else
#include <arpa/inet.h>   // for inet_**
#include <netdb.h>   // for gethost**
#include <netinet/in.h>  // for sockaddr_in
#include <sys/types.h>   // for socket
#include <sys/socket.h>  // for socket
#include <unistd.h>
#include <stdio.h>		// for printf
#include <stdlib.h>  // for exit
#include <string.h>  // for bzero
#define HSocket int
#endif 

class SocketBase : public Ref
{
public:
	SocketBase();
	~SocketBase();

protected:
	void closeConnect(HSocket socket);
	bool error(HSocket socket);

protected:
	std::mutex _mutex;
	
private:
	bool _bInitSuccess;
};