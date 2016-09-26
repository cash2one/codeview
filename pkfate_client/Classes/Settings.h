#pragma once
#include "cocos2d.h"
#include <map>
USING_NS_CC;
#define VERSION	GetAppVersion()
#define PARENTID 0
#define SERVERURL "gateway.pkfate.com"
#define SERVERPORT 7001
class Settings:public Ref
{
private:
	Settings();   //构造函数是私有的
	~Settings();
	void onGetServerInfoCallBack(Ref *ref);
public:
	static Settings * GetInstance();
	std::map<std::string, std::string> ServerInfo;
	void GetServerInfo();		//获取服务端信息
};