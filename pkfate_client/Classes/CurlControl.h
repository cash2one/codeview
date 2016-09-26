#pragma once

#include "cocos2d.h"
USING_NS_CC;;
#define CURLCALLFUNC std::function<void(Ref*)>
class CurlMsg :public Ref
{
public:
	enum Status{ Ok, Progress, Error };
	const char *url;
	Status status;
	size_t totalToDownload;
	size_t size;	//data 当前的size，即 nowDownload
	int pct;			//下载百分比
	char *data;		//在状态Ok时，必须free，否则会内存泄露，其他时间为不完整数据
	CURLCALLFUNC onProgress;		//处理进度函数
	CURLCALLFUNC onDone;		//完成后回调函数
	CurlMsg();
	~CurlMsg();
	double timeoutDownload;		//超时的下载数
	short errorTimes;			//出错次数
};
class CurlControl// :public std::enable_shared_from_this < CurlControl >
{
public:
	CURLCALLFUNC onProgress;		//处理进度函数
	CURLCALLFUNC onDone;		//完成后回调函数
	CurlMsg *GetUrlMsg(const char* url);		//提交请求，返回消息结果
	void GetUrl(const char* url);		//提交get请求,tag为返回时的消息名称
	void GetUrlAsync(const char* url);		//异步提交get请求,tag为返回时的消息名称,存在同步问题，以后再解决
	CurlControl();
	~CurlControl();
};