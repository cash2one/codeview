#ifndef __TESTCLIENT_TESTCLIENT_H__
#define __TESTCLIENT_TESTCLIENT_H__


#include "ISocketSystem.h"
#include <boost/asio.hpp>
#include "IBasicService.h"
#include "ITimeAxis.h"
#include "FieldDef.h"
#include "LoginSrvProtocol.h"

class Client;

//配置参数
struct SConfigParam
{
	FIELD_BEGIN();

	FIELD(TInt8Array<32>,   m_szLoginSrvIp);		//登陆服务器IP
	FIELD(UINT16,           m_LoginSrvPort);		//登陆服务器端口
	FIELD(TInt8Array<32>,   m_szGameSrvIp);		//游戏服务器IP
	FIELD(UINT16,           m_GameSrvPort);		//游戏服务器端口
	FIELD(UINT32,			m_TimeNumSendMsg);		//一个模拟客户端隔多久发一次消息(毫秒)
	FIELD(INT16,            m_NetIoThreadNum);		//网络IO线程数
	FIELD(INT16,            m_KeepLiveTime);		//连接心跳超时
	FIELD(INT32,            m_MaxConnNum);			//最大连接数

	FIELD_END();
};

class TestClient : public ISocketSink
{
public:
	TestClient();
	~TestClient();

	static TestClient * GetInstance();

	//nCLientNum为同时运行的客户端数
	bool   Init(const char* szFileName, int nCLientNum);

	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

	//随机获取一个10字节名字
	void  RandomGetName(char * szName, int nLen);
public:
	ISocketSystem  *		GetSocketSystem();
	IBasicService  *		GetBasicServer();
	ITimeAxis      *		GetTimeAxis();
	const SConfigParam   *	GetConfigParam();

	virtual ICryptService * GetCryptService();

private:
	//进入游戏
	void  __EnterGame();

private:
	ISocketSystem  *			m_pSocketSystem;
	IBasicService  *			m_pBasicService;
	ITimeAxis      *			m_pTimeAxis;
	boost::asio::signal_set *	m_pSignals_;

	SConfigParam				m_ConfigParam;

	TSockID						m_SockID;

	int							m_ClientNum;		//要同时开启的客户端数

	std::vector<Client *>		m_vectClient;

	std::vector<SGameServerInfo>	m_vectGameServerInfo;	//游戏服务器信息
};

#endif
