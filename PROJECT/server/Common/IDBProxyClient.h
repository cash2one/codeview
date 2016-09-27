
#ifndef __DBPROXYSERVER_IDBPROXYCLIENT_H__
#define __DBPROXYSERVER_IDBPROXYCLIENT_H__

#include "BclHeader.h"
#include "DBProtocol.h"
#include "TBuffer.h"

#ifndef BUILD_DBPROXY_CLIENT_DLL
#ifdef _MSC_VER
#  pragma comment(lib,"DBProxyClient.lib")
#endif
#endif

struct ISocketSystem;

// 前置机回调接口
// 一定要保证返回时可以回调到正确的指针
struct IDBProxyClientSink
{
	// 前置机回调
	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0) = 0;
};



struct IDBProxyClient
{
	virtual void Release() = 0;

	virtual bool Init(ISocketSystem * pSocketSystem,const char * szIp,int port) = 0;

	virtual void Close()=0;

	// 发送DB请求包
	// userdata: 不透明数据，一般调用者用来标示每个请求
	// ReqCmd: 请求ID
	// ReqOb  请求数据
	// userID: 发出本请求的用户ID, 用于取模进行负载均衡
	// pDBProxyClientSink: 请求返回回调接口,如果为NULL，表示不关心结果，如玩家数据存盘，
	virtual bool    Request(unsigned int userID,enDBCmd ReqCmd, OStreamBuffer & ReqOb, 
		IDBProxyClientSink *pDBProxyClientSink=NULL,UINT64 userdata=0) = 0;

};

BCL_API IDBProxyClient * CreateDBProxyClient();

#endif

