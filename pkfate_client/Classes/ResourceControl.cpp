#include "ResourceControl.h"
#include "PKNotificationCenter.h"
#include "comm.h"
//#include "HttpControl.h"
#include "CurlControl.h"
using namespace cocos2d;

static ResourceControl *m_pInstance = nullptr;

ResourceControl::ResourceControl()
{
	RemoteResources.clear();
}
ResourceControl::~ResourceControl()
{
	if (RemoteResources.size())
	{
		for (auto pRes : RemoteResources)
		{
			delete pRes;
		}
		RemoteResources.clear();
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
ResourceControl* ResourceControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new ResourceControl();
	return m_pInstance;
}
//获取指定资源文件的sha
//@param path - 资源文件路径
//@return 资源文件sha，失败时返回""
string ResourceControl::GetResourceSha(string path)
{
	if (path.empty())
		return "";
	// find local file.
	FileUtils *fileUtils = FileUtils::getInstance();
	std::string fullPath = Comm::GetResDir() + path;
	// sha local file.
	ssize_t size = 0;
	unsigned char *data = fileUtils->getFileData(fullPath, "rb", &size);
	if (data == nullptr || size <= 0)
		return "";
	unsigned char chars[32] = { 0 };
	SHA256(chars, data, size);
	free(data);
	//CCLOG("sha:%s", Char322Char64(chars));
	return string(Char322Char64(chars));
}
//检查本地资源文件
void ResourceControl::CheckResources()
{
	if (RemoteResources.size() <= 0)
	{
		//获取远程资源信息失败，无法检查
		PostNotify(ResourceMsgCode::Error, "get remote resources error.", nullptr);
		return;
	}
	//_updateResources.clear();
	//test
	//string str = GetResourceSha(FileUtils::getInstance()->getWritablePath() + "res/temp/bg1.png");
	//string path = FileUtils::getInstance()->fullPathForFilename("SceneLogin.csb");
	for (auto res : RemoteResources)
	{
		//CCLOG("remote sha:\t%s\r\nlocal sha:\t%s", res->sha.c_str(), GetResourceSha(rootpath + res->path).c_str());
		if (GetResourceSha(res->path) != res->sha)
			_updateResources[res->url] = res;
	}
	//开始更新资源
	auto t = std::thread(CC_CALLBACK_0(ResourceControl::funcDownloadResource, this));
	t.detach();
}
void ResourceControl::funcDownloadResource()
{
	CurlControl *curl = new CurlControl();
	curl->onDone = CC_CALLBACK_1(ResourceControl::onDowloadResourceCallBack, this);
	curl->onProgress = CC_CALLBACK_1(ResourceControl::onDowloadResourceCallBack, this);
	CurlMsg *msg;
	bool ok = true;
	for (auto res : _updateResources)
	{
		PostNotify(ResourceMsgCode::DownloadStart, "", res.second);
		msg = curl->GetUrlMsg(res.second->url.c_str());
		onDowloadResourceCallBack((Ref*)msg);
		if (!msg || msg->status == ResourceMsgCode::Error)
		{
			ok = false;
			break;
		}
	}
	delete curl;
	if (ok)
		PostNotify(ResourceMsgCode::AllDone, "", nullptr);
}
void ResourceControl::onDowloadResourceCallBack(Ref *ref)
{
	// dump data
	CurlMsg *msg = (CurlMsg*)ref;
	if (msg == nullptr)
	{
		PostNotify(ResourceMsgCode::Error, "download error.", nullptr);
		return;
	}
	//找到对应的资源
	RemoteResource *res = _updateResources[msg->url];
	if (res == nullptr)
	{
		PostNotify(ResourceMsgCode::Error, StringUtils::format("download unknown file (url:%s)", msg->url), nullptr);
		return;
	}
	switch (msg->status)
	{
	case CurlMsg::Status::Ok:
	{
		//下载完成
		PostNotify(ResourceMsgCode::DownloadEnd, "", res);
		//下载成功，解压zip文件
		ZipFile *zip = ZipFile::createWithBuffer(msg->data, msg->size);
		if (zip == nullptr)
		{
			PostNotify(ResourceMsgCode::Error, "unzip error.", res);
			return;
		}
		//提取文件名,默认只有一个文件
		string filename = GetFileNameFromPath(res->path);
		//解压
		ssize_t size;
		unsigned char* data = zip->getFileData(filename, &size);
		if (data == nullptr)
		{
			PostNotify(ResourceMsgCode::Error, "uncompress error.", res);
			return;
		}
		string pathroot = Comm::GetResDir();
		if (pathroot.back() == '/')
			pathroot.pop_back();
		string pathreal = pathroot + (res->path[0] == '/' ? res->path : "/" + res->path);
		if (!FileUtils::getInstance()->createDirectory(pathreal.substr(0, pathreal.find_last_of('/'))))
		{
			PostNotify(ResourceMsgCode::Error, "create resources dir error.", res);
			return;
		}
		FILE *fp;
		fp = fopen(pathreal.c_str(), "wb");
		if (fp == nullptr)
		{
			PostNotify(ResourceMsgCode::Error, "create resources file error.", res);
			return;
		}
		size_t wsize = fwrite(data, 1, size, fp);
		free(data);
		fclose(fp);
		if (wsize != size)
		{
			PostNotify(ResourceMsgCode::Error, "write file error.", res);
			return;
		}
		PostNotify(ResourceMsgCode::UnzipEnd, "", res);
	}
		break;
	case CurlMsg::Status::Progress:
		PostNotify(ResourceMsgCode::DownloadProgress, "", res, msg->totalToDownload, msg->size);
		break;
	default:
		PostNotify(ResourceMsgCode::Error, "download error.", res);
		break;
	}
}
void ResourceControl::PostNotify(ResourceMsgCode code, string msg, RemoteResource *res)
{
	PostNotify(code, msg, res, 0, 0);
}
void ResourceControl::PostNotify(ResourceMsgCode code, string msg, RemoteResource *res, int total, int now)
{
	if (res)
	{
		char *p[] = { "AllDone", "DownloadStart", "DownloadEnd", "DownloadProgress", "UnzipEnd", "Error" };
		//CCLOG("%s\t%s\t%s\t%d/%d", p[code], msg.c_str(), res->path.c_str(), now, total);
	}
	PKNotificationCenter::getInstance()->postNotification("checkresources", (Ref*)new ResourceMsg(code, msg, res, total, now));		//通知SceneStart已经返回
}