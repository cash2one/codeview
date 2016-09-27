



#include "ISocketSystem.h"
#include <string>
#include "SimpleCrypt.h"
#include "IBasicService.h"
#include <boost/asio.hpp>
#include "ITimeAxis.h"
#include "ILogTrace.h"
#include "IDataBase.h"
#include <vector>
#include <boost/random.hpp>

#include "FieldDef.h"
#include "StringUtil.h"
#include "FileConfig.h"
#include "TDataBase.h"
#include "DBProtocol.h"
#include "IDBProxyClient.h"
#include "TBuffer.h"
#include "IEventServer.h"
#include "ICrashDump.h"
#include "CrashReport.h"

/*#ifdef WIN32
#include "windows.h"
#include "sqlext.h"
#include <odbcinst.h>
#else
#include "sqlcli1.h"
#endif*/



IBasicService * g_pBasicService=NULL;

struct ServerSockSink : public ISocketSink,public ITimerSink
{
	ServerSockSink(ISocketSystem * pSocketSystem,ITimeAxis * pTimeAxis )
	{
		m_pSocketSystem = pSocketSystem;
		m_pTimeAxis = pTimeAxis;

		m_pTimeAxis->SetTimer(1,this,5000);

	}

	//新连接建立,socketid为socket标识,remote为远端ip,act为Listen或Connect函数调用时传送的act
	virtual void OnConnectOK(TSockID socketid,UINT32 remote_ip,UINT16 port,UINT32 act)
	{		
		m_VectConnect.push_back(socketid);		
	}

	//数据到达
	virtual void OnNetDataRecv(TSockID socketid,OStreamBuffer & osb)
	{
		m_pSocketSystem->Send(socketid,osb);
	}

	//连接关闭
	virtual void OnClose(TSockID sockid)
	{
		for(int i=0;i<m_VectConnect.size();i++)
		{
			if(m_VectConnect[i]==sockid)
			{
				m_VectConnect.erase(m_VectConnect.begin()+i);
				break;

			}
		}
	}

	void OnTimer(UINT32 timerID)
	{
		static int count = 0;
		if(++count>2)
		{
			m_pTimeAxis->KillAllTimer(this);

			return;
		}
		if(m_VectConnect.size())
		{
			boost::random::rand48 rnd(::time(0));

			int index = rnd()%m_VectConnect.size();

			char buffer[1024*10];

			int len =  4 + rnd()%(sizeof(buffer)-4);
			*(int*)buffer = len;
			OBuffer ob(len);
			ob.Push(buffer,len);

			m_pSocketSystem->Send(m_VectConnect[index],ob.TakeOsb());
		}
	}

private:
	ISocketSystem * m_pSocketSystem ;
	ITimeAxis * m_pTimeAxis ;
	std::vector<TSockID> m_VectConnect;
};

typedef TArray<int,3> IntArray3;

struct TestSubObj
{
	FIELD_BEGIN();
	FIELD(int,m_i);
	FIELD(TBinArray<3>,m_vect);

	FIELD_END();
};

struct TestObj
{
	FIELD_BEGIN();
	FIELD(int,m_i);
	FIELD(char,m_c);
	FIELD(IntArray3,m_vect);
	FIELD(TestSubObj,m_SubObj);

	FIELD_END();
};

bool TestField()
{
	TestObj obj;
	FileConfig File;
	if(File.Open("./Config.ini")==false)
	{
		return false;
	}

	std::vector<TestObj> vect;

	if(File.Read(obj,"TestObj")==false)
	{
		return false;
	}

	TRACE_OBJ(obj);

	if(File.Write(obj,"TestObj")==false)
	{
		return false;
	}

	return true ;
}



bool TestDB()
{
	IDataBase * pDataBase = CreateDataBase(enDBType_MYSQL);


	if(pDataBase->Connect("centerdb","root","888888","127.0.0.1",0,"gbk")==false)
	{
		std::string strError;
		pDataBase->GetErrorInfo(NULL,strError);
		std::cout << strError;
		return false;
	}


	

	OutParamNull         OutParam;

	DB_User_Info_Record * pResultSet = NULL;
	ResultSetNull * pResultSet2 = NULL;
	int count =0;



	char buff[sizeof(STestVarBinary)+1024] = {0};
	STestVarBinary * pReq = (STestVarBinary * )buff;

	DB_User_Info_Record Record;

	DB_OutParam * pOutParam = 0;
	//ResultSetNull * pOutParam = 0;
	int OutParamCount = 0;

	DB_OutParam * pOutParam2;

	STestStruct2 Result;

	SDBGoodsData * pGoodsData = (SDBGoodsData * )(unsigned char*)Result.m_GoodsData;

	Result.m_GoodsNum = 2;

	for(int i=0; i<Result.m_GoodsNum;i++,pGoodsData++)
	{

	  pGoodsData->m_uidGoods = UID();
	  pGoodsData->m_GoodsID  = i+1;
	  pGoodsData->m_CreateTime = ::time(0);
	  pGoodsData->m_Number = i+1;
	  pGoodsData->m_Binded = i%2;
	}


	for(int i=0; i<10000; i++)
	{
		{
		TDataBase DataBase(pDataBase);
		if(!DataBase.ExecuteMultiResult(Result,pOutParam2,OutParamCount,pResultSet2,count))
		{
			std::string strError;
			int errCode = DataBase.GetErrorInfo(strError);
			std::cout << strError;
			return false;
		}
		else
		{
			if(pOutParam2 != 0)
			{
				delete [] pOutParam2;
				pOutParam2 = 0;
			}
		}
		}
	}

	pDataBase->Close();

	return true;
}

struct STestStruct
{
	FIELD_BEGIN();

	FIELD(int,m_i);
	FIELD(TInt8Array<3>,m_IntArray);
	FIELD(std::vector<int>,m_vect);
	FIELD(std::string,m_str);

	FIELD_END();
};



bool TestStruct()
{
	STestStruct obj;
	TRACE_OBJ(obj);

	std::string str;
	StringUtil::NumberToStr(obj,str);

	StringUtil::StrToNumber(str,obj);

	return true;
}


class TestDBProxyClient : public IDBProxyClientSink
{
public:
	TestDBProxyClient()
	{
		m_pSocketSystem = 0;
		m_pDBProxyClient = 0;
	}

	bool Init(ISocketSystem * pSocketSystem)
	{
		m_pSocketSystem = pSocketSystem;
		if(m_pSocketSystem==NULL)
		{
			return false;
		}

		m_pDBProxyClient = CreateDBProxyClient();
		if(m_pDBProxyClient==NULL)
		{
			return false;
		}

		if(m_pDBProxyClient->Init(m_pSocketSystem,"192.168.1.61",5000)==false)
		{
			return false;
		}


		return RequseUserInfo();
	}

	bool RequseUserInfo()
	{
		DB_Get_User_Info_Req Req;
		strncpy(Req.szUserName,"李小龙",sizeof(Req.szUserName));

		TOBuffer<sizeof(DB_Get_User_Info_Req)> ob;
		ob << Req;

		return m_pDBProxyClient->Request(1,enDBCmd_GetUserInfo,ob.TakeOsb(),this,123);
	}

	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata) 
	{
		IBuffer ib(RspOsb.Buffer(),RspOsb.Size());

		DBRspPacketHeader RspHeader;

		ib >> RspHeader;

		if(ib.Error())
		{
			return;
		}

		switch(ReqCmd)
		{
		case enDBCmd_GetUserInfo:
			{
				NULL;
			}
			break;
		default:
			break;
		}
	}

private:
	IDBProxyClient * m_pDBProxyClient;
	ISocketSystem * m_pSocketSystem;
};

bool TestBuffer()
{
	TestObj Obj;
	int i = 0;
	float f = 0.1;
	char c = 'A';

	char data[256];

	TOBuffer<2> OBuffer2;

	(OBuffer2 << i << f << c << Obj).Push(data,sizeof(data)) << i;	

	TOBuffer<8> OBuffer8;
	(OBuffer8 << i << f << c << Obj).Push(data,sizeof(data)) << i;	

	TOBuffer<64> OBuffer64;
	(OBuffer64 << i  << Obj).Push(data,sizeof(data)) << i << f << c;	

	TOBuffer<128> OBuffer128;
	(OBuffer128 << i << f << c ).Push(data,sizeof(data)) << i << Obj;	

	if(OBuffer2.Error() || OBuffer8.Error() 
		|| OBuffer64.Error()
		|| OBuffer128.Error()) return false;

	IBuffer iBuffer(OBuffer8.Buffer(),OBuffer8.Size());

	(iBuffer >> i >> f >> c >> Obj).Pop(data,sizeof(data)) >> i;	

	if(iBuffer.Error()) return false;

	return true;

}

void TestOBuffer(OBuffer & ob)
{
	return;
}


struct TestEventListener : public IEventListener
{
	virtual void OnEvent(XEventData & EventData)
	{
	}
};

void TestEvent()
{
	TestEventListener EventListener;
}



void main(int argc,char**argv)
{  		


   char * ptr =  new char[200];

	g_pBasicService =GetBasicService();

	IBasicService * pBasicService = g_pBasicService;

	pBasicService->Create();

	ICrashDump * pCrashDump = CreateCrashDump();

	if(pCrashDump != 0)
	{
		pCrashDump->AttachCrashDumpSink(&g_CrashReport);
	}


	ITimeAxis * pTimeAxis= pBasicService->GetTimeAxis();

	ILogTrace * pLogTrace = pBasicService->GetLogTrace();

	if(pLogTrace->Create("./LogFile")==false)
	{
		return;
	}

	if(TestBuffer()==false)
	{
		return ;
	}

	
	if(TestDB()==false)
	{
		return;
	}

	/*pTimeAxis->Create(pBasicService->GetIoService(),1024,10);

	char szIP[] = "127.0.0.1";

	ISocketSystem * pSocketSystem = CreateSocketSystem();

	ServerSockSink SockSink(pSocketSystem,pTimeAxis);

	pSocketSystem->Start(pBasicService->GetIoService(),1000);

	TestDBProxyClient DBProxyClient;
	if(DBProxyClient.Init(pSocketSystem)==false)
	{
		return ;
	}

	if(argc>1)
	{
		TRACE("%s","client start!");


		TSockID socketid = pSocketSystem->Connect(szIP,5001,&SockSink);

		if(socketid.IsInvalid())
		{
			return;
		}		
	}
	else
	{
		TRACE("%s","server start!");

		pSocketSystem->Listen(szIP,5001,&SockSink);
	}	

	pBasicService->GetIoService()->run();

	pSocketSystem->Release();
	*/

	if(pCrashDump != 0)
	{
		pCrashDump->DetachCrashDumpSink(&g_CrashReport);
	}

	

	pBasicService->Release();
}
