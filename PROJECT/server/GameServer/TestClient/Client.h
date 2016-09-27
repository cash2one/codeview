#ifndef __TESTCLIENT_CLIENT_H__
#define __TESTCLIENT_CLIENT_H__

#include "ISocketSystem.h"
//#include "ICmdBase.h"
#include <vector>
#include "DSystem.h"
#include "TestClient.h"
#include "OStreamBuffer.h"
#include <hash_map>
#include "UniqueIDGenerator.h"
#include "ITimeAxis.h"

struct ICmdBase;

class Client : public ISocketSink, public ITimerSink
{
	enum enTimerID
	{
		enTimerID_Msg,		//发消息,一定时间发一条
	};
public:
	Client(TUserID UserID, const char * pszTicket);
	~Client();

	bool Init();

	//进入游戏
	void EnterGame(const char* szIP,UINT16 port);

	//发送数据
	bool SendData(OStreamBuffer & osb);

	//随机执行命令
	void RandomExecutionCmd();
	
	virtual void OnTimer(UINT32 timerID);

	//获得玩家的UID值
	const UID & GetUserUID();

	//是否已进入游戏服
	bool IsLogin();

public:
	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act);

	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

private:
	//进游戏服消息操作
	void __HandleEneterGame(UINT8 nCmd, IBuffer & ib);

	//游戏世界的消息操作
	void __HandleGameWorld(UINT8 nCmd, IBuffer & ib);

	//开始
	void __Start();

private:
	TSockID		m_SockID;

	TUserID		m_UserID;

	char        m_szTicket[TICKET_LEN];  //登陆游戏需要用到的票

	//命令集合
	std::hash_map<UINT8/*服消息分类*/, ICmdBase *> m_mapCmd;

	UID			m_uidUser;				//玩家的UID

	bool		m_bWaitTimer;			//是否要等待定时器

	bool		m_bFirst;
};

#endif
