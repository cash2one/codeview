

#include "DangLePlatform.h"
#include "ICryptService.h"
#include "IBasicService.h"

static const char * s_HttpHethod[enThridPartCmd_Max] = 
{
	"GET",
	"POST",
};

DangLePlatform::DangLePlatform()
{
    m_pThirdPartSink = NULL;

	m_pSocketSystem = NULL;

	m_MerchantID =0;
	m_GameID =0 ;
	m_ServerID =0;

}

DangLePlatform::~DangLePlatform()
{
}


bool DangLePlatform::Init(ISocketSystem * pSocketSystem,const char* szRemoteIp,UINT16 Port,const char* szAppKey,
						  const char* pSecretKey,IThirdPartSink * pSink,const char * szLoginUrl,const char* szRegisterUrl)
{
	this->m_strAppKey = szAppKey;
	this->m_pSocketSystem = pSocketSystem;
	this->m_strSecret = pSecretKey;
	this->m_pThirdPartSink = pSink;
	m_strRemoteIp = szRemoteIp;

	m_RemotePort = Port,

	m_strLoginUrl = szLoginUrl;

	m_strRegisterUrl = szRegisterUrl;

	return true;
}

void DangLePlatform::Release()
{
	delete this;
}

//用户登陆
bool DangLePlatform::UserLogin(const char *szUserName,const char* szPassword,UINT32 userdata)
{
	//GET http://connect.d.cn/connect/json/member/login HTTP/1.0\r\nContent-Length:22\r\n\r\napi_key=123456&sig=EADDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD&mid=&username=jxcq&vc=FFFFFFFFFFFFFFFFFFFFFFFFFF

	std::string strSig = CalculateLoginSig(szUserName,szPassword);

	std::string strVC = CalculateLoginVC(szUserName,strSig.c_str());

	std::ostringstream ost;
	
	ost << "api_key=" << m_strAppKey << "&sig=" <<  strSig  << "&mid=" << "" << "&username=" << szUserName << "&vc=" << strVC;

	std::string  strBody = ost.str();

	return HttpRequst(0,enThridPartCmd_Login,m_strLoginUrl.c_str(),strBody,userdata);
}

bool  DangLePlatform::HttpRequst(UINT32 UserID,enThridPartCmd nCmd,const char* szUrl,const std::string & strBody,UINT32 userdata)
{
	std::ostringstream ost;
	ost << s_HttpHethod[nCmd] << szUrl << " HTTP/1.0\r\nContent-Length:" << strBody.length() << "\r\n\r\n" << strBody;

	TSockID SockID = m_pSocketSystem->Connect(this->m_strRemoteIp.c_str(),m_RemotePort,this);

	if(SockID.IsInvalid())
	{
		TRACE("<error> %s : %d 连接[%s]失败!",__FUNCTION__,__LINE__,szUrl);
		return false;
	}

	std::string strReq = ost.str();

    OBuffer1k ob;
	ob << strReq;
	if(m_pSocketSystem->Send(SockID,ob.TakeOsb())==false)
	{
		TRACE("<error> %s : %d 发送数据到[%s]失败!",__FUNCTION__,__LINE__,szUrl);
		return false;
	}

    SRequstData       Data;
	Data.m_ReqCmd = nCmd;
	Data.m_UserData = userdata;
	Data.m_UserID   = UserID;

	m_mapRequst[SockID.ToUint64()] = Data;

	return true;

}



//计算签名sig=MD5(api_key=%s&mid=%s&username=%s&sha256_pwd=%s&secret_key=%s) 
std::string DangLePlatform::CalculateLoginSig(const char *szUserName,const char* szPassword)
{
	std::ostringstream ost;
	
	ost << "api_key=" << m_strAppKey << "&mid=" << "" << "&username=" << szUserName << "&sha256_pwd=" << szPassword << "&secret_key=" << m_strSecret;

	IBasicService * pBasicService = ::GetBasicService();

	std::string str  = ost.str();
	
	if(pBasicService)
	{
		TMD5 md5 = pBasicService->GetCryptService()->CalculateMD5((const UINT8*)str.c_str(),str.length());

		return md5.toString();
	}

	return "";	
}

//计算VC vc=MD5(api_key=%s& mid=%s&username=%s&sig=%s)
std::string DangLePlatform::CalculateLoginVC(const char *szUserName,const char* szSig)
{
	std::ostringstream ost;
	
	ost << "api_key=" << m_strAppKey << "&mid=" << "" << "&username=" << szUserName << "&sig=" << szSig;

	IBasicService * pBasicService = ::GetBasicService();

	std::string str  = ost.str();
	
	if(pBasicService)
	{
		TMD5 md5 = pBasicService->GetCryptService()->CalculateMD5((const UINT8*)str.c_str(),str.length());

		return md5.toString();
	}

	return "";	
}

//用户注册
bool DangLePlatform::UserRegister(const char *szUserName,const char* szPassword,UINT32 userdata)
{
	std::string strSig = CalculateRegisterSig(szUserName,szPassword);
	
	std::ostringstream ost;
	
	ost << "api_key=" << m_strAppKey << "&sig=" <<  strSig  << "&username=" << szUserName << "&password=" << szPassword << "&nickname=" << szUserName;

	std::string  strBody = ost.str();

	return HttpRequst(0,enThridPartCmd_Register,m_strRegisterUrl.c_str(),strBody,userdata);

}

//计算签名sig=MD5(api_key=%s&username=%s&password=%s&nickname=%s&secret_key=%s) 
std::string DangLePlatform::CalculateRegisterSig(const char *szUserName,const char* szPassword)
{
	std::ostringstream ost;

	ost << "api_key=" << m_strAppKey << "&username=" << szUserName << "&&password=" << szPassword << "&nickname=" << szUserName << "&secret_key=" << m_strSecret;

	std::string str  = ost.str();

	IBasicService * pBasicService = ::GetBasicService();
	
	if(pBasicService)
	{
		TMD5 md5 = pBasicService->GetCryptService()->CalculateMD5((const UINT8*)str.c_str(),str.length());

		return md5.toString();
	}

	return "";	
}

//数据到达,
void DangLePlatform::OnNetDataRecv(TSockID socketid, OStreamBuffer & osb)
{
	MAP_REQUST::iterator it = m_mapRequst.find(socketid.ToUint64());
	if(it == m_mapRequst.end())
	{
		m_pSocketSystem->ShutDown(socketid);
		return;
	}

	SRequstData & RequstData = (*it).second;

	char * ptr = (char*)osb.Buffer();

	*(ptr+osb.Size()-1) = 0;

	RequstData.m_strReponse += ptr;

	char * szBody = 0;

	INT32 len = 0;
	//需要判断包是否接收完毕了
	bool bComplete = IsReponseComplete(RequstData.m_strReponse,szBody,len);

	if(bComplete)
	{
		if(szBody && len>0)
		{
			ReponseSuccess(RequstData,szBody,len);

		    m_mapRequst.erase(it);

		    m_pSocketSystem->ShutDown(socketid);
		}
	}
	else
	{
		TRACE("<error> %s : %d 解释http 应答错误,[%s]",__FUNCTION__,__LINE__,RequstData.m_strReponse.c_str());
		m_pSocketSystem->ShutDown(socketid);
	}
}

//http应答是否接收完整
bool DangLePlatform::IsReponseComplete(std::string & strReponse,char* & szBody,INT32 & len)
{
	szBody = 0;
	len = 0;

	char * szHeader = (char*)strReponse.c_str();
	char * ptr = strstr(szHeader,"Content-Length:");
	if(ptr==0)
	{
		return true;
	}

	ptr += 15; //Content-Length:

	char * ptr2 = strstr(ptr,"\r\n");

	if(ptr2==0)
	{
		return true;
	}

	char buff[100]={0};

	int contextLne = ptr2-ptr;
	if(contextLne>20)
	{
		TRACE("<error> %s : %d Content-Length[%d]太长了，不能解释!",__FUNCTION__,__LINE__,contextLne);
		return false;
	}
	memcpy(buff,ptr,contextLne);

	contextLne = atoi(buff);

    if(contextLne>1024)
	{
		TRACE("<error> %s : %d Content-Length[%d]太长了，不能解释!",__FUNCTION__,__LINE__,contextLne);
		return false;
	}

	ptr = strstr(ptr2+2,"\r\n\r\n");

	if(ptr==0)
	{
		return true;
	}

	ptr += 4; //\r\n\r\n

	if(strReponse.length()-(ptr-szHeader) < contextLne+1) //留一个\0
	{
		return true;
	}

	szBody = ptr;
	len = contextLne;
	*(szBody+contextLne) = 0;

	return true;	
}

//应答成功
void DangLePlatform::ReponseSuccess(SRequstData & RequstData,const char* szBody,int len)
{
	switch(RequstData.m_ReqCmd)
	{
	case enThridPartCmd_Login:
		{
			OnLoginRet(RequstData,szBody,len);
		}
		break;
	case enThridPartCmd_Register:
		{
			OnRegisterRet(RequstData,szBody,len);
		}
		break;
	}
}

void DangLePlatform::OnLoginRet(SRequstData & RequstData,const char* szBody,int len)
{
	int Status = -1;

	GetProperty(szBody,len,"status",Status);

	enThirdPartRetCode retCode = GetRetCode(Status);

	std::string strAccessSecret;

	if(retCode==enThirdPartRetCode_OK)
	{
		GetProperty(szBody,len,"djtk",strAccessSecret);
	}
	
	this->m_pThirdPartSink->OnLoginRet(retCode,strAccessSecret.c_str(),RequstData.m_UserData);
}

enThirdPartRetCode DangLePlatform::GetRetCode(int Stauts)
{
	enThirdPartRetCode RetCode = enThirdPartRetCode_ErrUnknow;
	switch(Stauts)
	{
	case 0:
		RetCode = enThirdPartRetCode_OK;
		break;
	case 101:
        RetCode = enThirdPartRetCode_ErrUnknow;
		break;
	case 311:
	case 601:
		RetCode = enThirdPartRetCode_UserNotExist;
		break;
	case 312:
		RetCode = enThirdPartRetCode_ErrPwd;
		break;
	case 602:
		RetCode = enThirdPartRetCode_UserExit;
		break;
	default:
		break;
	}

	return RetCode;
}

void DangLePlatform::OnRegisterRet(SRequstData & RequstData,const char* szBody,int len)
{
	int Status = -1;

	GetProperty(szBody,len,"status",Status);

	enThirdPartRetCode retCode = GetRetCode(Status);

	std::string strAccessSecret;

	if(retCode==enThirdPartRetCode_OK)
	{
		GetProperty(szBody,len,"djtk",strAccessSecret);
	}
	
	this->m_pThirdPartSink->OnRegisterRet(retCode,strAccessSecret.c_str(),RequstData.m_UserData);
}


//应答失败
void DangLePlatform::ReponseFail(TSockID socketid,enThirdPartRetCode RetCode)
{
	MAP_REQUST::iterator it = m_mapRequst.find(socketid.ToUint64());
	if(it != m_mapRequst.end())
	{
		SRequstData  RequstData = (*it).second;	

		m_mapRequst.erase(it);

		switch(RequstData.m_ReqCmd)
	   {
	     case enThridPartCmd_Login:
		 {
			m_pThirdPartSink->OnLoginRet(RetCode,0,RequstData.m_UserData);
		 }
		 break;
	     case enThridPartCmd_Register:
		 {
			m_pThirdPartSink->OnRegisterRet(RetCode,0,RequstData.m_UserData);
		 }
		 break;
	  }	
	}

	m_pSocketSystem->ShutDown(socketid);	
}

//连接关闭
void DangLePlatform::OnClose(TSockID sockid)
{
	ReponseFail(sockid,enThirdPartRetCode_Close);
}

//设置其他接口URL
bool DangLePlatform::SetInterfaceUrl(UINT16  InterfaceID,const char* szInterFaceUrl) 
{
	if(szInterFaceUrl == 0)
	{
		return false;
	}
	m_mapUrl[InterfaceID] = szInterFaceUrl;
	return true;
}

//设置平台参数
bool DangLePlatform::SetPlatformParam(const char* szMerchantKey,const char*szDesKey,int MerchantID,int GameID,int ServerID)
{
	m_strMerchantKey = szMerchantKey;
	m_strDesKey = szDesKey;
	m_MerchantID = MerchantID;
	m_GameID = GameID;
	m_ServerID = ServerID;

	return true;
}

UINT16 DangLePlatform::GetRechargeInterfaceID(UINT8 pcid)
{
	return pcid+1;
}

std::string DangLePlatform::GetInterfaceUrl(UINT16 InterfaceID)
{
	MAP_URL::iterator it = m_mapUrl.find(InterfaceID);
	if(it == m_mapUrl.end())
	{
		return "";
	}

	return (*it).second;		
}

	//提交订单
bool DangLePlatform::CommitOrder(UINT32 UserID,const char* szUserName,UINT8 pcid,const char* szIp,const char* szDesc,const char* szCardNo,const char* szPassword,int amount)
{
	UINT16 InterfaceID = GetRechargeInterfaceID(pcid);

	std::string strUrl = GetInterfaceUrl(InterfaceID);

	if(strUrl.empty())
	{
		return false;
	}

	//拼串mid=%s&gid=%s&sid=%s&uif=s%&utp=s%&uip=s%&eif=s%&cardno=s%&cardpwd=s%&amount=s%&timestamp=s%&merchantkey=s%

	std::ostringstream ost;

	ost << "mid=" << m_MerchantID << "&gid=" << m_GameID  << "&sid=" << m_ServerID << "&uif=" << szUserName << "&utp=0" 
		<< "&uip=" << szIp << "&eif=" << szDesc << "&cardno=" << szCardNo << "&cardpwd=" << szPassword << "&amount=" <<  amount << "&timestamp=" << ::time(0);

	std::string strOrder = ost.str();

	std::string str = strOrder + "&merchantkey=" + m_strMerchantKey;

	IBasicService * pBasicService = ::GetBasicService();
	
	TMD5 md5 = pBasicService->GetCryptService()->CalculateMD5((const UINT8*)str.c_str(),str.length());

	std::string verstring = md5.toString(false);

	strOrder = strOrder + "&verstring=" + verstring;

	return HttpRequst(UserID,enThridPartCmd_CommitOrder,strUrl.c_str(),strOrder,userdata);;
}
