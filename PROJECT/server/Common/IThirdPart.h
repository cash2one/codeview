
#ifndef __THIRDPART_ITHIRDPART_H__
#define __THIRDPART_ITHIRDPART_H__


#include "DSystem.h"

#ifndef BUILD_THIRDPARTPLATFORM_DLL
#  pragma comment(lib,"ThirdPartPlatform.lib")
#endif

struct ISocketSystem;




enum enThirdPartRetCode VC_PACKED_ONE
{	
	enThirdPartRetCode_OK = 0,  //成功
	enThirdPartRetCode_ErrUnknow, //未知错误
	enThirdPartRetCode_Close   , //连接断开
	enThirdPartRetCode_UserNotExist, //用户名不存在
	enThirdPartRetCode_ErrPwd, //密码错误
	enThirdPartRetCode_UserExit, //用户名已存在

	enThirdPartRetCode_Max,
}; 


//回调
struct IThirdPartSink
{
		//登陆返回
	virtual void OnLoginRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata){};


	//注册返回
	virtual void OnRegisterRet(enThirdPartRetCode RetCode,const char* szAccessSecret,UINT32 userdata){};
};

struct IThirdPart
{
	virtual bool Init(ISocketSystem * pSocketSystem,const char* szRemoteIp,UINT16 Port,const char* AppKey, 

		const char* pSecretKey,IThirdPartSink * pOSink,const char * szLoginUrl,const char* szRegisterUrl) = 0;

	virtual void Release() = 0;

	//用户登陆
	virtual bool UserLogin(const char *szUserName,const char* szPassword,UINT32 userdata) = 0;

	//用户注册
    virtual bool UserRegister(const char *szUserName,const char* szPassword,UINT32 userdata) = 0;

	//设置其他接口URL
	virtual bool SetInterfaceUrl(UINT16  InterfaceID,const char* szInterFaceUrl) = 0;

	//设置平台参数
	virtual bool SetPlatformParam(const char* szMerchantKey,const char*DesKey,int MerchantID,int GameID,int ServerID) = 0;

	//提交订单
	virtual bool CommitOrder(UINT32 UserID,const char* szUserName,UINT8 pcid,const char* szIp,const char* szDesc,const char* szCardNo,const char* szPassword,int amount) = 0;

};

BCL_API IThirdPart * CreateThirdPartProxy(UINT8 type);

#endif
