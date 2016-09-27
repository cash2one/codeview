
#ifndef __CHATSERVER_CHATSERVER_H__
#define __CHATSERVER_CHATSERVER_H__

#include <boost/asio.hpp>
#include "ISocketSystem.h"
#include "IDBProxyClient.h"
#include "FieldDef.h"
#include <hash_map>


class GameServerProxy;
//配置文件
//配置参数
struct SConfigParam
{
	FIELD_BEGIN();
	FIELD(TServerID,         m_ServerID); //服务器ID
	FIELD(TInt8Array<20>,    m_szIp);   //本服务器监听IP
	FIELD(UINT16 ,           m_Port);   //本服务器监听端口
	FIELD(INT16 ,            m_NetIoThreadNum);  //网络IO线程数
	FIELD(INT16,             m_KeepLiveTime);     //连接心跳超时
	FIELD(INT32,             m_MaxConnNum); //最大连接数
	FIELD(TInt8Array<20> ,   m_szDBIp);  //数据库前端服务器IP
	FIELD(UINT16 ,           m_DBPort); //数据库前端服务器端口
	FIELD(INT16,             m_ServerKeepLiveTime);     //服务器间连接心跳超时

	FIELD_END();
};

struct IBasicService;

//全局配置
extern SConfigParam g_ConfigParam;
extern IBasicService * g_pBasicService;

class ChatServer : public ISocketSink , public IDBProxyClientSink
{
public:
	static ChatServer & GetInstance();

    ChatServer();
	~ChatServer();

	bool Init(const char* szConfigFileName);
	bool Run();

public:

	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
	{
	}
		//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

public:
		virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

		
		//获得关键词
		void HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

public:
	//游戏服断开连接
	void OnGameServerClose(TServerID ServerID);

private:
	void handle_stop(); //停止

	void HandleClientMsg(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob); //处理客户端到达的消息

	void HandleGameServerMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob); //处理游戏服务器到达的消息


	//具体的消息请求

	//登陆请求
	void HandleLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	GameServerProxy * GetGameServerProxy(TServerID ServerID);

	//游戏服登陆
void HandleGameServerLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

private:

	std::hash_map<UINT16,GameServerProxy*>  m_mapServer;

	ISocketSystem * m_pSocketSystem;

	IDBProxyClient * m_pDBProxyClient;
	
	boost::asio::signal_set * m_pSignals_;

};









#endif
