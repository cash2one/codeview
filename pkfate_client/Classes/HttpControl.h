#pragma once
#include "cocos2d.h"
#include "network/HttpClient.h"
using namespace cocos2d::network;
class HttpControl
{
protected:
	void onHttpRequestCompleted(HttpClient *sender, HttpResponse *response);
public:
	static HttpControl* GetInstance();
	void GetUrl(const char* url, const char* tag);		//提交get请求,tag为返回时的消息名称
private:
	HttpControl();
	~HttpControl();
};