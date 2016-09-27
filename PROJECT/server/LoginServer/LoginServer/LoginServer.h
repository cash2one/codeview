
#ifndef __LOGINSERVER_LOGINSERVER_H__
#define __LOGINSERVER_LOGINSERVER_H__

#include "ISocketSystem.h"
#include "IDBProxyClient.h"
#include <boost/asio.hpp>
#include "FieldDef.h"
#include "LoginSrvProtocol.h"
#include "IThirdPart.h"
#include <hash_map>

struct IBasicService;

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
	FIELD(UINT8              ,m_GamePlatform);          //游戏平台

	FIELD_END();
};

//保存第三方数据
struct SThirdPartReq
{
	UINT8       m_nCmd;
	TSockID     m_SockID;
};


//全局配置
extern SConfigParam g_ConfigParam;
extern IBasicService * g_pBasicService;

class LoginServer : public ISocketSink , public IDBProxyClientSink,public IThirdPartSink
{
public:
	static LoginServer & GetInstance();

    LoginServer();
	~LoginServer();

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
	 	//登陆返回
	virtual void OnLoginRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata);


	//注册返回
	virtual void OnRegisterRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata);

public:
		virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

		//登陆DB应答
		void HandleDBRetGetUserInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

		//注册DB应答
		void HandleDBRetInsertUserInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);


		//获得关键词
		void HandleDBRetGetKeywordInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

		//获取系统公告
		void HandleDBRetGetServerInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

		//插入节点信息
		void HandleDBRetInsertPointInfo(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

public:
	//游戏服断开连接
	void OnGameServerClose(TServerID ServerID);

		//游戏服汇报状态
	void HandleReportStateReq(TServerID ServerID,enServerState State);

private:
	void handle_stop(); //停止

	void HandleClientMsg(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob); //处理客户端到达的消息

	void HandleGameServerMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob); //处理游戏服务器到达的消息

	void HandleWebServerMsg(TSockID socketid,UINT8 nCmd,IBuffer & ib, OBuffer4k & ob); //处理WEB服务器到达的消息


	//具体的消息请求

	//登陆请求
	void HandleLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	//登陆成功应答
	void HandleLoginSuccessRsp(TSockID socketid,TUserID UserID,UINT8 IdentityStatus,INT32 OnlineTime,OBuffer4k & ob);

	//登陆失败应答
	void HandleLoginFailRsp(TSockID socketid,UINT8 retCode,OBuffer4k & ob);

	//Push游戏服务器列表到OBuffer
	void PushGameServerList(TSockID socketid,OBuffer4k & ob);


	////////////////////////////////////////////////////////////////////////////////////
	//注册请求
	void HandleRegisterReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	//注册成功应答
	void HandleRegisterSuccessRsp(TSockID socketid,TUserID UserID,OBuffer4k & ob);

	//注册失败应答
	void HandleRegisterFailRsp(TSockID socketid,UINT8 retCode,OBuffer4k & ob);

	////////////////////////////////////////////////////////////////////////////////////////////
	//获取服务器状态
	void HandleGetServerStateReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	//请求检查版本号
	void HandleCheckVersion(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);
		//请求检查版本号
	void HandleCheckVersionEx(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	//请求检查版本号
void HandleCheckVersion_i(TSockID socketid,UINT8 nCmd,CS_CheckVersionEx_Req & Req,OBuffer4k & ob);

//处理客户端发上来的节点问题
void HandleSendPoint(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

//获得安装包路径
std::string LoginServer::GetInstallPackageUrl(UINT32 PaltformID);

	
	/////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////
	//来自游戏服的消息

	//游戏服登陆
	void HandleGameServerLoginReq(TSockID socketid,UINT8 nCmd, IBuffer & ib,OBuffer4k & ob);

	bool GetGameServerInfoIndex(TServerID ServerID,int & index);


private:
	//不是线程安全
	 const char* IpToStr(UINT32 Ip); 

	UINT32 IpToUINT(const char* szIp);

	void GetKeyWorldInfo();
	//获得服务器信息
    void GetServerInfo();

	//登陆请求数据库
void LoginReqToDB(TSockID socketid,CS_Login_Req & LoginReq);

//向数据库注册
void RegisterToDB(TSockID socketid,CS_Register_Req & RegisterReq);

enLoginRetCode GetRetCode(enThirdPartRetCode RetCode);

//发送节点的返回
void SendPointBack(TSockID socketid,UINT32 Vid);


private:

	std::vector<SGameServerInfo> m_vectServerInfo; //游戏服列表

	ISocketSystem * m_pSocketSystem;

	IDBProxyClient * m_pDBProxyClient;

	IThirdPart   * m_pThirdPartProxy;
	
	boost::asio::signal_set * m_pSignals_;

	std::string       m_strPublicNotic;  //系统公告
	std::string       m_strUpdateDesc;   //更新说明
	std::string       m_strLastVersion;  //最新版本号
	std::string       m_strMinVersion;   //最小版本号
	std::string       m_strResUrl; //资源下载url

	std::string          m_strServiceTel;
	std::string          m_strServiceEmail;
	std::string          m_strServiceQQ;

	//各个平台安装包下载地址
	typedef std::hash_map<UINT32,std::string>  MAP_INSTALLPACKAGE;
	MAP_INSTALLPACKAGE  m_mapInstallPackageUrl;

};




#endif
