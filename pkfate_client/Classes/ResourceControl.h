#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
enum ResourceMsgCode{ AllDone, DownloadStart, DownloadEnd, DownloadProgress, UnzipEnd, Error };
struct RemoteResource
{
	std::string path;
	std::string sha;
	std::string url;
};
struct ResourceMsg
{
public:
	ResourceMsgCode code;
	string msg;
	RemoteResource* res;
	int totalToDownload;
	int nowDownloaded;
	ResourceMsg(ResourceMsgCode c, string m, RemoteResource *r)
	{
		code = c;
		msg = m;
		res = r;
		totalToDownload = 0;
		nowDownloaded = 0;
	}
	ResourceMsg(ResourceMsgCode c, string m, RemoteResource *r, int total, int now)
	{
		code = c;
		msg = m;
		res = r;
		totalToDownload = total;
		nowDownloaded = now;
	}
};

class ResourceControl
{
public:
	static ResourceControl* GetInstance();
	string GetResourceSha(string path);
	std::vector<RemoteResource*> RemoteResources;
	void CheckResources();
	void funcDownloadResource();
private:
	ResourceControl();
	~ResourceControl();
	void onDowloadResourceCallBack(Ref *ref);
	void PostNotify(ResourceMsgCode code, string msg, RemoteResource *res);
	void PostNotify(ResourceMsgCode code, string msg, RemoteResource *res, int total, int now);
	map<std::string,RemoteResource*> _updateResources;		//用于存储需要更新的资源文件
};