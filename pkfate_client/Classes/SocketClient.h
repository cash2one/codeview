#pragma once
#include "SocketBase.h"
#include "comm.h"
class SocketClient : public SocketBase
{
public:
	SocketClient(void);
	~SocketClient(void);

	bool connectServer(const char* serverIP, unsigned short port);
	bool sendMessage(unsigned short type, unsigned short typesub, unsigned short len, const char* msg);
	bool sendMessage(unsigned short type, unsigned short typesub, const char* data);
	bool IsConnected();

	bool tcpConnect(const char* ip, int port);

	bool tcpConnectIP6(const char* ip, int port);

	std::function<void(SOCKETDATA *data)> onRecv;
	std::function<void()> onDisconnect;
	void close();
private:
	bool initClient(int af = AF_INET);
	void recvMessage();
};