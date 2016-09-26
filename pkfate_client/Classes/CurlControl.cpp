#include "CurlControl.h"
#include "stdio.h"
#include "stdlib.h"
#include "curl/curl.h"
#include <memory>
USING_NS_CC;
CurlMsg::CurlMsg()
	: url(nullptr)
	, status(Status::Error)
	, totalToDownload(0)
	, size(0)
	, data(nullptr)
	, errorTimes(0)
	, timeoutDownload(0)
{

}
CurlMsg::~CurlMsg()
{
	if (data)
		free(data);
}
static CurlControl *m_pInstance = nullptr;
CurlControl::CurlControl()
	:onDone(nullptr)
	, onProgress(nullptr)
{
}
CurlControl::~CurlControl()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	CurlMsg *msg = (CurlMsg*)userp;

	msg->data = (char*)realloc(msg->data, msg->size + realsize + 1);
	if (msg->data == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	memcpy(&(msg->data[msg->size]), contents, realsize);
	msg->size += realsize;
	msg->data[msg->size] = 0;
	return realsize;
}
int DownloadProgressFunc(CurlMsg* msg, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	if (totalToDownload == 0)
		return 0;
	if (msg->totalToDownload == 0)
		msg->totalToDownload = totalToDownload;
	int pct = (int)((double)msg->size / msg->totalToDownload * 100);
	if (pct!=msg->pct)
	{
		msg->pct = pct;
		msg->status = CurlMsg::Status::Progress;
		//创建新的数据，以免出现多线程同步问题，同时可以避免使用同步锁
		CurlMsg msg1;
		msg1.url = msg->url;
		msg1.totalToDownload = msg->totalToDownload;
		msg1.pct = msg->pct;	
		msg1.size = msg->size;
		//size,data数据不需要，因为此时数据不完整
		msg1.status = CurlMsg::Status::Progress;		//必须指定，不能复制msg数据，因为有可能会改变状态
		if (msg->onProgress)
			msg->onProgress(&msg1);
		/*
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->onProgress&&msg->status == CurlMsg::Status::Progress)
		{
		//创建新的数据，以免出现多线程同步问题，同时可以避免使用同步锁
		CurlMsg msg1;
		msg1.url = msg->url;
		msg1.totalToDownload = msg->totalToDownload;
		msg1.nowDownloaded = msg->nowDownloaded;
		//size,data数据不需要，因为此时数据不完整
		msg1.status = CurlMsg::Status::Progress;		//必须指定，不能复制msg数据，因为有可能会改变状态
		msg->onProgress(msg);
		}
		});*/
	}
	return 0;
}
void CurlControl::GetUrlAsync(const char* url)
{
	auto t = std::thread(&CurlControl::GetUrl, this, url);
	t.detach();
}
void CurlControl::GetUrl(const char* url)
{
	//std::weak_ptr<CurlControl> ptr = shared_from_this();
	if (!onDone)
		return;
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	CurlMsg *msg = new CurlMsg();
	msg->url = url;
	msg->onDone = onDone;
	msg->onProgress = onProgress;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)msg);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		if (onProgress)
		{
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)msg);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DownloadProgressFunc);
		}
		while (true)
		{
			res = curl_easy_perform(curl);
			if (res == 0)
			{
				long retcode = 0;
				res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
				if (res == CURLE_OK && (retcode == 200 || (retcode == 206 && msg->timeoutDownload > 0)))
				{
					//成功获取数据,不释放memory数据，待调用后释放
					msg->status = CurlMsg::Status::Ok;
				}
				else
					msg->status = CurlMsg::Status::Error;
				//double length = 0;
				//res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
				//CCLOG("curl data[%d]\nresponse code:%d\n:%s", msg->size, retcode, msg->data);
				break;
			}
			else if (res == CURLE_PARTIAL_FILE || res == CURLE_RECV_ERROR || res == CURLE_OPERATION_TIMEDOUT)
			{
				//CURLE_PARTIAL_FILE (18)
				//CURLE_RECV_ERROR (56)
				//CURLE_OPERATION_TIMEDOUT (28)		超时，应付卡住的情况
				if (res == CURLE_OPERATION_TIMEDOUT&&(long)msg->timeoutDownload != msg->size)
				{
					msg->timeoutDownload = (double)msg->size;		//正常超时
				}
				else
					msg->errorTimes++;
				if (msg->errorTimes > 3)
				{
					msg->status = CurlMsg::Status::Error;
					break;
				}
				curl_easy_setopt(curl, CURLOPT_RESUME_FROM, msg->size);
				continue;
			}
			else
			{
				msg->status = CurlMsg::Status::Error;
				break;
			}
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	else
	{
		msg->status = CurlMsg::Status::Error;
	}
	msg->onDone(msg);		//调用回调函数后，再销毁数据
	msg->release();
}
CurlMsg* CurlControl::GetUrlMsg(const char* url)
{
	//std::weak_ptr<CurlControl> ptr = shared_from_this();
	if (!onDone)
		return nullptr;
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	CurlMsg *msg = new CurlMsg();
	msg->url = url;
	msg->onDone = onDone;
	msg->onProgress = onProgress;
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)msg);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		if (onProgress)
		{
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
			curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)msg);
			curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DownloadProgressFunc);
		}
		while (true)
		{
			res = curl_easy_perform(curl);
			if (res == 0)
			{
				long retcode = 0;
				res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retcode);
				if (res == CURLE_OK && (retcode == 200 || (retcode == 206 && msg->timeoutDownload > 0)))
				{
					//成功获取数据,不释放memory数据，待调用后释放
					msg->status = CurlMsg::Status::Ok;
				}
				else
					msg->status = CurlMsg::Status::Error;
				//double length = 0;
				//res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
				//CCLOG("curl data[%d]\nresponse code:%d\n:%s", msg->size, retcode, msg->data);
				break;
			}
			else if (res == CURLE_PARTIAL_FILE || res == CURLE_RECV_ERROR || res == CURLE_OPERATION_TIMEDOUT)
			{
				//CURLE_PARTIAL_FILE (18)
				//CURLE_RECV_ERROR (56)
				//CURLE_OPERATION_TIMEDOUT (28)		超时，应付卡住的情况
				if (res == CURLE_OPERATION_TIMEDOUT && (long)msg->timeoutDownload != msg->size)
				{
					msg->timeoutDownload = (double)msg->size;		//正常超时
				}
				else
					msg->errorTimes++;
				if (msg->errorTimes > 3)
				{
					msg->status = CurlMsg::Status::Error;
					break;
				}
				curl_easy_setopt(curl, CURLOPT_RESUME_FROM, msg->size);
				continue;
			}
			else
			{
				msg->status = CurlMsg::Status::Error;
				break;
			}
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	else
	{
		msg->status = CurlMsg::Status::Error;
	}
	//msg->onDone(msg);		//调用回调函数后，再销毁数据
	//msg->release();
	return msg;
}