#include "HttpControl.h"
#include "PKNotificationCenter.h"
using namespace cocos2d;

static HttpControl *m_pInstance = nullptr;
HttpControl::HttpControl()
{
}
HttpControl::~HttpControl()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
HttpControl* HttpControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new HttpControl();
	return m_pInstance;
}
void HttpControl::GetUrl(const char* url,const char* tag)
{
	HttpRequest* request = new (std::nothrow) HttpRequest();
	request->setUrl(url);
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback(CC_CALLBACK_2(HttpControl::onHttpRequestCompleted, this));
	request->setTag(tag);
	HttpClient::getInstance()->send(request);
	request->release();
}
void HttpControl::onHttpRequestCompleted(HttpClient *sender, HttpResponse *response)
{
	if (!response)
	{
		return;
	}
	const char *tag = response->getHttpRequest()->getTag();
	if (strlen(tag) <= 0)
		return;
	if (!response->isSucceed())
	{
		CCLOG("response failed");
		CCLOG("error buffer: %s", response->getErrorBuffer());
		PKNotificationCenter::getInstance()->postNotification(tag, nullptr);
		return;
	}

	/*
	// dump data
	std::vector<char> *buffer = response->getResponseData();
	std::string data;
	data.assign(buffer->begin(), buffer->end());
	NotificationCenter::getInstance()->postNotification(tag, (Ref*)data.c_str());
	*/
	PKNotificationCenter::getInstance()->postNotification(tag, (Ref*)response->getResponseData());
}