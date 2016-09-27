

#ifndef __BCL_OAUTH2_PROXY_H__
#define __BCL_OAUTH2_PROXY_H__

#include "BclHeader.h"

struct ISocketSystem;

//回调接口
struct IOauth2Sink
{
	virtual void OnOauthRet(const char* szUser,const char* reponse,UINT32 userdata) = 0;
};


struct IOauth2Proxy
{
	//初始化
	virtual bool Init(ISocketSystem * pSocketSystem,const char* AppKey, const char* pSecretKey,IOauth2Sink * pOauth2Sink) = 0;

	//查询
	virtual bool Requset(const char* szUser,const char* requset,UINT32 userdata,IOauth2Sink * pOauth2Sink=0) = 0;

};





#endif
