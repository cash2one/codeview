
#ifndef __THIRDPART_DANGLE_PLATFORM_H__
#define __THIRDPART_DANGLE_PLATFORM_H__


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/date_time.hpp>


#include "ISocketSystem.h"

#include "IThirdPart.h"

#include <string>

#include <hash_map>

#include <sstream>


using namespace boost::property_tree;
using namespace boost::gregorian;
using namespace boost;






//命令字
enum enThridPartCmd VC_PACKED_ONE
{
   enThridPartCmd_Login         = 0,   //登陆

   enThridPartCmd_Register      = 1,   //注册

    enThridPartCmd_CommitOrder  = 2,   //提交订单

   enThridPartCmd_Max,

} PACKED_ONE;


//接口ID定义
enum enInterfaceID VC_PACKED_ONE
{
	enInterfaceID_Non        = 0,
	enInterfaceID_ShenZhouFu = 1,  //神州付
	enInterfaceID_JunWang    = 2,     //骏网
	enInterfaceID_ShenZhouXing = 3,  //神州行
	enInterfaceID_LianTong = 4,  //联通
	enInterfaceID_ShengDa = 5,  //盛大
	enInterfaceID_ChengTu = 6,  //征途
	enInterfaceID_DianXin = 7,  //电信
	enInterfaceID_ZhiFuBao = 8,  //支付宝

	enInterfaceID_Max,
};




struct SRequstData
{
	enThridPartCmd  m_ReqCmd;

	UINT32          m_UserID;
	UINT32          m_UserData;
	std::string     m_strReponse;
};

class DangLePlatform : public IThirdPart, public ISocketSink
{
	typedef std::hash_map<UINT64,SRequstData> MAP_REQUST;
public:
	DangLePlatform();

	virtual ~DangLePlatform();


	virtual bool Init(ISocketSystem * pSocketSystem,const char* szRemoteIp,UINT16 Port,const char* AppKey,
		const char* pSecretKey,IThirdPartSink * pSink,const char * szLoginUrl,const char* szRegisterUrl);

	virtual void Release();

	//用户登陆
	virtual bool UserLogin(const char *szUserName,const char* szPassword,UINT32 userdata);

		//用户注册
    bool UserRegister(const char *szUserName,const char* szPassword,UINT32 userdata);

		//设置其他接口URL
	virtual bool SetInterfaceUrl(UINT16  InterfaceID,const char* szInterFaceUrl); 

		//设置平台参数
	virtual bool SetPlatformParam(const char* szMerchantKey,const char*DesKey,int MerchantID,int GameID,int ServerID); 

		//提交订单
	virtual bool CommitOrder(UINT32 UserID,const char* szUserName,UINT8 pcid,const char* szIp,const char* szDesc,const char* szCardNo,const char* szPassword,int amount);

public:
	//数据到达,
	virtual void OnNetDataRecv(TSockID socketid, OStreamBuffer & osb);

	//连接关闭
	virtual void OnClose(TSockID sockid);

private:
	//计算签名sig=MD5(api_key=%s&mid=%s&username=%s&sha256_pwd=%s&secret_key=%s) 
std::string CalculateLoginSig(const char *szUserName,const char* szPassword);

//计算VC vc=MD5(api_key=%s& mid=%s&username=%s&sig=%s)
std::string CalculateLoginVC(const char *szUserName,const char* szSig);

bool HttpRequst(UINT32 UserID,enThridPartCmd nCmd,const char* szUrl,const std::string & strBody,UINT32 userdata);

std::string CalculateRegisterSig(const char *szUserName,const char* szPassword);

//http应答是否接收完整
bool IsReponseComplete(std::string & strReponse,char* & szBody,INT32 & len);

//应答失败
void ReponseFail(TSockID socketid,enThirdPartRetCode RetCode);

//应答成功
void ReponseSuccess(SRequstData & RequstData,const char* szBody,int len);

void OnLoginRet(SRequstData & RequstData,const char* szBody,int len);

void OnRegisterRet(SRequstData & RequstData,const char* szBody,int len);

UINT16 GetRechargeInterfaceID(UINT8 pcid);

std::string GetInterfaceUrl(UINT16 InterfaceID);

template <typename type>
bool GetProperty(const char * szBody,int len,const char * szProperty, type & Value)
{
	if(szBody==0 || szProperty==0)
	{
		return false;
	}
	
	ptree pt;
	std::stringstream stream;

	stream << szBody;
    read_json<ptree>( stream, pt);

	Value = pt.get<type>(szProperty);

	return true;
}

enThirdPartRetCode GetRetCode(int Stauts);

private:
	
	std::string  m_strAppKey;

	std::string  m_strSecret;

	std::string  m_strRemoteIp;

	UINT16       m_RemotePort;

	std::string m_strLoginUrl;

	std::string m_strRegisterUrl;

	std::string m_strMerchantKey ;
	std::string m_strDesKey ;
	int m_MerchantID ;
	int m_GameID ;
	int m_ServerID;


	IThirdPartSink * m_pThirdPartSink;

	ISocketSystem * m_pSocketSystem;


	MAP_REQUST  m_mapRequst;

	typedef std::hash_map<UINT16,std::string> MAP_URL;

	MAP_URL  m_mapUrl;

};



#endif
