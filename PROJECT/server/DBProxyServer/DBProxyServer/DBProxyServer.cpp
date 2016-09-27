
#include "DBProxyServer.h"
#include "ISocketSystem.h"
#include "ProtocolHeader.h"
#include "ILogTrace.h"
#include <boost/asio.hpp>
#include "TBuffer.h"
#include "ICrashDump.h"
#include "FileConfig.h"
#include <hash_map>



#define CMD_TO_TYPE(cmd,type) m_mapReqHanlder[cmd] = new TRequst<type> ();


#include "DBProtocol.h"


SConfigParam g_ConfigParam;

IBasicService * g_pBasicService=NULL;

bool DBProxyServer::DBRequse(SDBReqData & ReqData)
{
	OBuffer4k ob(ReqData.osbInfo); //由OBuffer接管内存

	IBuffer ib(ob.Buffer(),ob.Size());

	DBReqPacketHeader  ReqHeader;

	ib >> ReqHeader;

	if(ReqHeader.command == enDBCmd_DBInit)
	{
		for(int i=0; i<m_DBSize;i++)
		{
			if( m_vectDB[i].get() !=0)
			{
				m_vectDB[i]->ThreadInit();
			}

		}
		return true;
	}
	
	INT32 RetCode = enDBRetCode_OK;

	TSockID socketid = ReqData.sockID;

	std::hash_map<int,IDBRequst*>::iterator it = m_mapReqHanlder.find(ReqHeader.command);

	if(it == m_mapReqHanlder.end())
	{
		return false;
	}

	IDBRequst * pRequstHanlder = (*it).second;

	if(pRequstHanlder==0)
	{
		return false;
	}

	RetCode = pRequstHanlder->Requst(this,socketid,&ReqHeader,ob,ib.Buffer(),ib.Capacity());

	if(enDBRetCode_OK != RetCode)
	{		
		//复位
	    ob.Reset();

		DBRspPacketHeader Rsp;

		Rsp.m_RequestSN = ReqHeader.m_RequestSN;		
		Rsp.m_UserID = ReqHeader.m_UserID;
		Rsp.m_RetCode = RetCode;

		 Rsp.command = ReqHeader.command;
		 Rsp.m_encryptAlgo = ReqHeader.m_encryptAlgo;


		ob << Rsp;  //分配内存	

		//返回给主线程，由主线程返回给客户端
		PostDBRsp(socketid,ob.TakeOsb());
	}

	return true;
}

//投递应答给主线程
 void DBProxyServer::PostDBRsp(TSockID socketid,OStreamBuffer & sob)
{
	g_pBasicService->GetIoService()->post(boost::bind(&DBProxyServer::HandleRsp,this,socketid,sob.TakeOStreamBufferInfo()));
	
}

 void DBProxyServer::HandleRsp(TSockID socketid,OStreamBuffer  sob)
 {
	 
	if(sob.Size() < sizeof(DBRspPacketHeader))
	 {
		 printf("%s : %d line 应答长度有误 len = %u",__FUNCTION__,__LINE__,sob.Size());
		
		 return;
	 }

	 m_pSocketSystem->Send(socketid,sob);
 }

DBProxyServer * DBProxyServer::Instance()
{
	static DBProxyServer s_Instance_;
	return &s_Instance_;
}

DBProxyServer::DBProxyServer() 
{
	m_pSocketSystem = NULL;
	m_pSignals_ = NULL;
	m_vectDB = 0;
	m_DBSize = 0;

	DB_ALL_CMD_TO_TYPE ; //注册处理者
}

DBProxyServer::~DBProxyServer()
{
	if(m_pSignals_)
	{
		delete m_pSignals_;
		m_pSignals_ = NULL;
	}

	if(m_pSocketSystem)
	{
		m_pSocketSystem->Release();
		m_pSocketSystem = NULL;
	}

	for(int i=0; i<m_DBSize;++i)
	{
		if(m_vectDB[i].get() != 0)
		{
			m_vectDB[i]->Release();
			m_vectDB[i].reset(0);
		}
	}

	for(int i=0; i<m_vectDeque.size();++i)
	{
		if(m_vectDeque[i])
		{
			delete m_vectDeque[i];
		}
	}
	m_vectDeque.clear();

	delete [] m_vectDB;

	m_vectDB = 0;
	m_DBSize = 0;

	for(std::hash_map<int,IDBRequst*>::iterator it = m_mapReqHanlder.begin(); it != m_mapReqHanlder.end(); ++it)
	{
		(*it).second->Release();
	}

	m_mapReqHanlder.clear();


}

bool DBProxyServer::Init(const char* szFileName)
{
	if(szFileName==NULL)
	{
		return false;
	}
	g_pBasicService = GetBasicService();

	if(g_pBasicService==NULL || g_pBasicService->Create()==false)
	{
		return false;
	}

	ICrashDump * pCrashDump = ::CreateCrashDump();

	if(pCrashDump != 0)
	{
		pCrashDump->CrtSetDbgFlag();
	}


	 ILogTrace * pLogTrace = g_pBasicService->GetLogTrace();

	 if(pLogTrace->Create("./DBProxyServer.log")==false)
	 {
		 return false;
	 }

	FileConfig File;
	if(!File.Open(szFileName,"DBProxyServer"))
	{
		TRACE("<error> %s : %d 打开配置文件[%s]失败!",__FUNCTION__,__LINE__,szFileName);
		return false;
	}

	if(File.Read(g_ConfigParam)==false)
	{
		TRACE("<error> %s : %d 读取配置参数失败!",__FUNCTION__,__LINE__);
		return false;
	}

	m_DBSize =g_ConfigParam.m_vectDBConnectInfo.size();
	if(m_DBSize>0)
	{
		m_vectDB = new boost::thread_specific_ptr<IDataBase>[m_DBSize];
	}

	//连接数据库
	for(int i=0; i<g_ConfigParam.m_vectDBConnectInfo.size();++i)
	{
		SDBConnectInfo & ConnInfo = g_ConfigParam.m_vectDBConnectInfo[i];

		IDataBase * pDataBase = CreateDataBase(enDBType_MYSQL);
		if(pDataBase==NULL)
		{
			TRACE("<error> %s : %d创建数据库组件失败!",__FUNCTION__,__LINE__);
			return false;
		}
		if(pDataBase->Connect((char*)ConnInfo.m_DBName,(char*)ConnInfo.m_DBUserName,(char*)ConnInfo.m_DBPassword,(char*)ConnInfo.m_szDBIp,ConnInfo.m_DBPort,"gbk")==false)
		{
            TRACE("<warn> %s : %d 连接数据失败 DBName[%s] UserName[%s] DBPwd[%s] DBIp[%s] DBPort[%d]!",__FUNCTION__,__LINE__,
				(char*)ConnInfo.m_DBName,(char*)ConnInfo.m_DBUserName,(char*)ConnInfo.m_DBPassword,(char*)ConnInfo.m_szDBIp,ConnInfo.m_DBPort);

			pDataBase->Release();

			continue;
		}

		m_vectDB[i].reset(pDataBase);
	}

	
		//创建队列
	for(int i=0;i<g_ConfigParam.m_DBThreadNum;i++)
	{
		ActiveDequeType * pDeque = new ActiveDequeType();
		if(pDeque->Start(boost::bind(&DBProxyServer::DBRequse,this,_1))==false)
		{
			delete pDeque;
			TRACE("<error> %s : %d 启动DB请求队列失败!",__FUNCTION__,__LINE__);
			return false;
		}
		m_vectDeque.push_back(pDeque);

		DBReqPacketHeader  ReqHeader;
		ReqHeader.command = enDBCmd_DBInit;

		OBuffer1k ob;
		ob << ReqHeader;

		SDBReqData ReqData;	
	    ReqData.sockID = TSockID();
		ReqData.osbInfo = ob.TakeOsb().TakeOStreamBufferInfo();

		pDeque->Put(ReqData);

	}

	
	//创建通讯库
	m_pSocketSystem = CreateSocketSystem();
	if(m_pSocketSystem==NULL)
	{
		TRACE("<error> %s : %d创建通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	if(m_pSocketSystem->Start(g_pBasicService->GetIoService(),g_ConfigParam.m_MaxConnNum,g_ConfigParam.m_NetIoThreadNum,0)==false)
	{
		TRACE("<error> %s : %d 初始化通讯库失败!",__FUNCTION__,__LINE__);
		return false;
	}

	if(m_pSocketSystem->Listen(g_ConfigParam.m_szIp,g_ConfigParam.m_Port,this,enPacketHeaderType_Len)==false)
	{
		TRACE("<error> %s : %d 通讯库监听失败 szIp[%s] Port[%d]!",__FUNCTION__,__LINE__,(char*)g_ConfigParam.m_szIp,g_ConfigParam.m_Port);
		return false;
	}

	m_pSignals_ = new boost::asio::signal_set(*g_pBasicService->GetIoService());

	m_pSignals_->add(SIGINT);
   m_pSignals_->add(SIGTERM);

	#if defined(SIGQUIT)
      m_pSignals_->add(SIGQUIT);
     #endif // defined(SIGQUIT)
  
	   m_pSignals_->async_wait(boost::bind(&DBProxyServer::handle_stop, this));

	return true;
}


bool DBProxyServer::Run()
{

	return g_pBasicService->GetIoService()->run();
}

void DBProxyServer::Stop()
{
	for(int i=0; i<m_vectDeque.size();++i)
	{
		if(m_vectDeque[i])
		{
			m_vectDeque[i]->Stop();
		}
	}

	m_pSocketSystem->Stop();

	g_pBasicService->GetIoService()->stop();	

	ICrashDump * pCrashDump = ::CreateCrashDump();

	

}

void DBProxyServer::handle_stop()
{
	Stop();
}

	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
void DBProxyServer::OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
{
}

	//数据到达
void DBProxyServer::OnNetDataRecv(TSockID socketid,  OStreamBuffer & osb )
{
	if(osb.Size() <sizeof(DBReqPacketHeader))
	{
		TRACE("<error> %s : %d 数据请求长度有误 %d < %d!",__FUNCTION__,__LINE__,osb.Size() ,sizeof(DBReqPacketHeader));
		
		return;
	}

	IBuffer ib(osb.Buffer(),osb.Size());

	DBReqPacketHeader ReqHeader ;

	ib >> ReqHeader;

	int index = ReqHeader.m_UserID % m_vectDeque.size();

	SDBReqData ReqData;
	
	ReqData.sockID = socketid;
	ReqData.osbInfo = osb.TakeOStreamBufferInfo();
	m_vectDeque[index]->Put(ReqData);	

}

	//连接关闭
void DBProxyServer::OnClose(TSockID sockid)
{
	TRACE("连接关闭 sockid = %lld" ,sockid.m_id);
}




