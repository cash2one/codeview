#include "SocketClient.h"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#include <unistd.h>
#endif
static HSocket _socketClient = 0;
SocketClient::SocketClient(void) :
onRecv(nullptr)
{
}

SocketClient::~SocketClient(void)
{
	close();
}
void SocketClient::close(){
	_mutex.lock();
	if (_socketClient>0)
	{
		this->closeConnect(_socketClient);
		this->closeConnect(_socketClient);
		_socketClient = 0;
	}
	_mutex.unlock();
}
bool SocketClient::initClient(int af)
{
    _mutex.lock();
    if (_socketClient>0)
        this->closeConnect(_socketClient);
	_socketClient = socket(af, SOCK_STREAM, 0);
    if (error(_socketClient))
    {
        CCLOG("init client error!");
        _socketClient = 0;
        _mutex.unlock();
        return false;
    }
    _mutex.unlock();
    return true;
}

bool SocketClient::IsConnected()
{
    return _socketClient > 0;
}

bool SocketClient::tcpConnect(const char* ip, int port)
{
	struct hostent *hp;
	struct in_addr in;
	CCLOG("domainName=%s\n", ip);
	hp = gethostbyname(ip);
	if (!hp)
	{
		CCLOG("SocketManager::init -> domain parse failed\n");

		return false;
	}

	memcpy(&in.s_addr, hp->h_addr, 4);
	ip = inet_ntoa(in);
	//strIp = "210.209.116.192";
	//strIp = "113.10.168.157";
	CCLOG("ip=%s port=%d\n", ip, port);

	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ip);

	if (!this->initClient(AF_INET))
	{
		return false;
	}
	CCLOG("debug:init client done.");

	int ret = 0;
	ret = connect(_socketClient, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr));
	if (ret < 0)
	{
		CCLOG("debug:[connectServer] connect ret <0");
		this->closeConnect(_socketClient);
		_socketClient = 0;
		return false;
	}
	return true;
}

bool SocketClient::tcpConnectIP6(const char* ip, int port)
{
	char strIP[100];
	sprintf(strIP, "%s", ip);

	char strPort[100];
	sprintf(strPort, "%d", port);
	struct addrinfo *result;
	struct addrinfo *res;

	struct addrinfo addrCriteria;
	memset(&addrCriteria, 0, sizeof(addrCriteria));
	addrCriteria.ai_family = AF_UNSPEC;
	addrCriteria.ai_socktype = SOCK_STREAM;
	addrCriteria.ai_protocol = IPPROTO_TCP;
	bool isConnectOk = false;

	//判定网络类型    指定特定接口信息  
	int error = getaddrinfo(strIP, strPort, &addrCriteria, &result);
	if (error == 0)
	{
		struct sockaddr_in *sa;
		for (res = result; res != NULL; res = res->ai_next)
		{
			if (AF_INET6 == res->ai_addr->sa_family)
			{
				char buf[128] = {};
				sa = (struct sockaddr_in*)res->ai_addr;
				inet_ntop(AF_INET6, &((reinterpret_cast<struct sockaddr_in6*>(sa))->sin6_addr), buf, 128);

				_socketClient = socket(res->ai_family, res->ai_socktype, 0);
				if (_socketClient == -1) {
					log("error socket create");
					return false;
				}

				struct sockaddr_in6 svraddr;
				memset(&svraddr, 0, sizeof(svraddr)); //注意初始化  
				svraddr.sin6_family = AF_INET6;
				svraddr.sin6_port = htons(port);
				if (inet_pton(AF_INET6, buf, &svraddr.sin6_addr) < 0)
				{
					log("error addr");
				}
				int ret = connect(_socketClient, (struct sockaddr*) &svraddr, sizeof(svraddr));
				if (ret < 0){
					CCLOG("can't connect to %s: %s\n", strIP, strerror(errno));
				}
				else{
					isConnectOk = true;
				}
				//TODO....遇到IPv6就退出  
				break;
			}
			else if (AF_INET == res->ai_addr->sa_family)
			{
				char buf[32] = {};
				sa = (struct sockaddr_in*)res->ai_addr;
				inet_ntop(AF_INET, &sa->sin_addr, buf, 32);

				_socketClient = socket(res->ai_family, res->ai_socktype, 0);
				if (_socketClient == -1) {
					log("error socket create");
					return false;
				}

				struct sockaddr_in svraddr;
				svraddr.sin_family = AF_INET;
				svraddr.sin_addr.s_addr = inet_addr(buf);
				svraddr.sin_port = htons(port);
				int ret = connect(_socketClient, (struct sockaddr*) &svraddr, sizeof(svraddr));

				if (ret < 0){
					CCLOG("can't connect to %s: %s\n", strIP, strerror(errno));
				}
				else{
					isConnectOk = true;
				}
				//TODO.... break除掉  
				//break;  
			}
		}
	}

	freeaddrinfo(result);

	return isConnectOk;
}

#define COMPATIBLE_IP6 //兼容IP6

bool SocketClient::connectServer(const char* serverIP, unsigned short port)
{
	serverIP = "113.10.168.157";//for test

#ifdef COMPATIBLE_IP6
	if(!tcpConnectIP6(serverIP, port)) return false; //采用域名
#else
	if (!tcpConnect(serverIP, port)) return false;
#endif


#ifdef SO_NOSIGPIPE
	int set = 1;//SIGPIPE make client shut down 
	if (setsockopt(_socketClient, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int)) < 0)
	{
		CCLOG("setsockopt() failed: SO_NOSIGPIPE\n");
		return false;
	}
#endif
    
    std::thread recvThread(&SocketClient::recvMessage, this);
    recvThread.detach();
    return true;
}

void SocketClient::recvMessage()
{
    SOCKETDATA data;
    char recvBuf[BUFSIZE];
    int ret, pos;
    bool isError = false;
    while (true)
    {
        ret = recv(_socketClient, (char*)&data, 8, MSG_WAITALL);
        if (ret!=8)
        {
            CCLOG("recv head error: errno: %d", errno);
            break;
        }
        /*
         data.flag = ntohs(data.flag);
         if (data.flag != 0xFF)
         {
         CCLOG("recv head error, flas != 0xFF");
         break;
         }
         data.type = ntohs(data.type);
         if (data.type != 0x01)
         {
         CCLOG("recv head error, type != 1");
         break;
         }
         data.len = ntohl(data.len);
         if (data.len <= 0)
         {
         CCLOG("recv head error, len <= 0");
         break;
         }*/
        data.type = ntohs(data.type);
        data.typesub = ntohs(data.typesub);
        data.code = ntohs(data.code);
        data.len = ntohs(data.len);
        data.content = "";
        pos = 0;
        while (pos < data.len)
        {
            ret = recv(_socketClient, recvBuf, min(data.len - pos, BUFSIZE), 0);
            if (ret <= 0)
            {
                CCLOG("recv content error, ret=%d", ret);
                isError = true;
                break;
            }
            data.content.append(recvBuf, ret);
            pos += ret;
        }
        if (isError)
            break;
        CCLOG("recv:\n{\n\ttype:%d\n\ttypesub:%d\n\tcode:%d\n\tlen:%d\n\tdata:%s\n}", data.type, data.typesub, data.code, data.len, data.content.c_str());
        if (onRecv != nullptr)
        {
            onRecv(&data);
        }
    }
    _mutex.lock();
    if (_socketClient > 0){
        this->closeConnect(_socketClient);
        _socketClient = 0;
        if (onDisconnect)
            onDisconnect();
    }
    _mutex.unlock();
}
bool SocketClient::sendMessage(unsigned short type,unsigned short typesub, unsigned short len, const char* msg)
{
    if (len > 0 && (!msg || len > (BUFSIZ - 8)))
        return false;
    SOCKETDATA data;
    char sendBuf[BUFSIZE];
    data.type = htons(type);
    data.typesub = htons(typesub);
    data.code = 0;
    data.len = htons(len);
    memcpy(sendBuf, &data, 8);
    if (len > 0)
        memcpy(sendBuf + 8, msg, len);
    int ret;
    ret = send(_socketClient, sendBuf, len + 8, 0);
    if (ret != len + 8)
    {
        return false;
    }
    CCLOG("send:\n{\n\ttype:%d\n\ttypesub:%d\n\tcode:%d\n\tlen:%d\n\tdata:%s\n}", type, typesub, 0, len, msg);
    return true;
}
bool SocketClient::sendMessage(unsigned short type, unsigned short typesub, const char* data)
{
    return sendMessage(type, typesub, strlen(data), data);
}
