#include "SceneStart.h"
#include "SceneLogin.h"
#include "cocostudio/CocoStudio.h"
#include "Settings.h"
#include "LayerLoading.h"
#include "ResourceControl.h"
#include "comm.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "SoundControl.h"
//#include "TimeControl.h"
//#include "LayerBjlFp.h"
#define DISABLECHECKRESOURCES
#define UPDATEURL "http://pkfate.com/"
// on "init" you need to initialize your instance
bool SceneStart::init()
{
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	memset(_strPct, 0, 5);
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("start/SceneStart.csb");
	if (!RootNode)
		return false;
	panelInfo = RootNode->getChildByName<Layout*>("panelInfo");
	if (!panelInfo)
		return false;
	//panelInfo->setVisible(false);
	
	imgInformationBluePanel = panelInfo->getChildByName<Sprite*>("imgInformationBluePanel");
	if (!imgInformationBluePanel)
		return false;
	imgInformationRedPanel = panelInfo->getChildByName<Sprite*>("imgInformationRedPanel");
	if (!imgInformationRedPanel)
		return false;
	imgCheckVer = panelInfo->getChildByName<Sprite*>("imgCheckVer");
	if (!imgCheckVer)
		return false;
	txtLoading = panelInfo->getChildByName<Text*>("txtLoading");
	if (!txtLoading)
		return false;
	txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "正在检测游戏版本" : "Checking version ...");
	txtPct = panelInfo->getChildByName<Text*>("txtPct");
	if (!txtPct)
		return false;
	btUpdate = RootNode->getChildByName<Button*>("btUpdate");
	btUpdate->setTitleText(Language::getStringByKey("Update"));
	if (!btUpdate)
		return false;
	btUpdateLater = RootNode->getChildByName<Button*>("btUpdateLater");
	if (!btUpdateLater)
		return false;
	btUpdateLater->setTitleText(Language::getStringByKey("Later"));
	auto lbVersion = RootNode->getChildByName<Text*>("txtVersion");
	if (!lbVersion)
		return false;
	lbVersion->setString(VERSION);
	currentNum = 0;
	totalNum = 100;
	this->addChild(RootNode);
	return true;
}
void SceneStart::onEnter()
{
	Scene::onEnter();
	
	HideLogo();//隐藏掉logo
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneStart::onGetServerInfoCallBack), "getserverinfo", NULL);
	imgCheckVer->runAction(RepeatForever::create(RotateBy::create(1, 359)));
	this->scheduleOnce([=](float dt){
		panelInfo->setVisible(true);
#ifdef DISABLECHECKRESOURCES
		this->LoadResources();
#else
		getServerInfoCount = 1;
		Settings::GetInstance()->GetServerInfo();
#endif
	}, 1, "ShowInfoPanel");
}
void SceneStart::onExit()
{
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Scene::onExit();
}
void SceneStart::onGetServerInfoCallBack(Ref *pSender)
{
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		imgCheckVer->setVisible(false);
		txtLoading->setString("");
		if (Settings::GetInstance()->ServerInfo.size() <= 0)
		{
			if (getServerInfoCount > 2){ //获取三次 如果不成功则弹出提示信息

				//获取服务器信息失败，无法进入游戏
				txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "获取信息失败，\n请检查网络" : "Get info error,\nplease check network.");
				string error = UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "获取信息失败，请检查网络,重启游戏" : "Get info error,please check network,restart game.";

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
				ShowTip(error,[=](){
					Director::getInstance()->end();
				},nullptr,true);

#elif(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
				ShowTip(error,[=](){
					Director::getInstance()->end();
			}, nullptr, true);
#else
				ShowTip(error, [=](){
					Director::getInstance()->end();
				}, nullptr, true);

#endif
			}
			else{
				getServerInfoCount = getServerInfoCount + 1;
				CCLOG("getServerInfoCount:%d", getServerInfoCount);
				Settings::GetInstance()->GetServerInfo();
			}
			
			return;
		}
		//检测版本
		std::string ver = Settings::GetInstance()->ServerInfo["ver"];
		std::string minver = Settings::GetInstance()->ServerInfo["minver"];
		int ret = 0;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS||CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
		ret = ver.compare(VERSION);
#endif
		if (ret <= 0)
		{
			//最新版本，检查资源
			CheckResources();
			//警告,本地版本大于远程版本
			if (ret < 0)
				CCLOG("[%s]:%s", "SceneStart::onGetServerInfoCallBack", "local ver > server ver");
		}
		else
		{
			//有新版本,再比较minver
			ret = minver.compare(VERSION);
			if (ret > 0)
			{
				Size visibleSize = Director::getInstance()->getVisibleSize();
				//minver > local ver,强制更新
				imgInformationBluePanel->setVisible(false);
				imgInformationRedPanel->setVisible(true);
				txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "版本过低，请升级！" : "Version is too old,\n please upgrade.");
				btUpdate->setPositionX(visibleSize.width / 2);
				btUpdate->addClickEventListener([=](Ref* ref){
					Application::getInstance()->openURL(UPDATEURL);
				});
				btUpdate->setVisible(true);
			}
			else
			{
				//minver < local ver < ver.建议更新
				Size visibleSize = Director::getInstance()->getVisibleSize();
				txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "有新版本，是否需要升级" : "New version is published,\n need upgrade?");
				btUpdate->addClickEventListener([=](Ref* ref){
					Application::getInstance()->openURL(UPDATEURL);
				});
				btUpdate->setVisible(true);
				btUpdateLater->addClickEventListener([=](Ref* ref){
					CheckResources();
				});
				btUpdateLater->setVisible(true);
			}
		}
	});
}

void SceneStart::CheckResources()
{
	LoadResources();
	return;
	btUpdate->setVisible(false);
	btUpdateLater->setVisible(false);
	txtInfoTitle->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "检查资源" : "Check Assets");
	txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "正在检查资源文件" : "Checking assets ...");
	imgCheckVer->setVisible(true);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneStart::onCheckResourcesCallBack), "checkresources", NULL);
	ResourceControl::GetInstance()->CheckResources();
	txtInfoTitle->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "更新资源" : "Update Assets");
	//启动更新百分比控件进程，以免更新过于频繁导致cocos资源释放出错
	schedule([=](float dt){
		txtLoading->setString(_strFileName);
		txtPct->setString(_strPct);			//怀疑更新太过频繁，导致资源释放时出现问题，约300次
		CCLOG("update pct:%d", clock());
	}, 0.1f, "UpdatePct");
}
void SceneStart::onCheckResourcesCallBack(Ref *ref)
{
	ResourceMsg *msg = (ResourceMsg *)ref;
	if (msg == nullptr)
		return;
	switch (msg->code)
	{
	case ResourceMsgCode::AllDone:
		//使用主线程调用
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
			unschedule("UpdatePct");
			//更新完成，开始加载资源
			PKNotificationCenter::getInstance()->removeObserver(this, "checkresources");
			this->LoadResources();
		});
		break;
	case ResourceMsgCode::DownloadStart:
		if (msg->res)
		{
			//txtLoading->setString(GetFileNameFromPath(msg->res->path));
			//txtPct->setString("0%");
			_strFileName = GetFileNameFromPath(msg->res->path);
			sprintf(_strPct, "0%%");
		}
		break;
	case ResourceMsgCode::DownloadEnd:
		if (msg->res)
		{
			//txtLoading->setString(GetFileNameFromPath(msg->res->path));
			//txtPct->setString("100%");
			sprintf(_strPct, "100%%");
			//背景音乐下载完成后，更新之前，需要先释放旧的背景音乐，否则在win32下，将无法更新文件
			//mac系统下，无法释放资源，但是可以更新文件
			if (msg->res->path == "res/sound/background.mp3")
				SoundControl::StopMusic();
		}
		break;
	case ResourceMsgCode::UnzipEnd:
		if (msg->res)
		{
			//背景音乐下载完成后，直接开始播放
			if (msg->res->path == "res/sound/background.mp3"){}
				//SoundControl::PlayMusic();
		}
		break;
	case ResourceMsgCode::DownloadProgress:
		sprintf(_strPct, "%.0f%%", msg->totalToDownload > 0 ? (double)msg->nowDownloaded / msg->totalToDownload * 100 : 0);
		break;
	default:
	{
		//出错
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
			unschedule("UpdatePct");
			auto layer = Layer::create();
			auto lb = Text::create();
			Size size = layer->getContentSize();
			lb->setPosition(Vec2(size.width / 2, size.height / 2));
			lb->setFontSize(48);
			lb->setString("check resources error\npls exit & retry");
			layer->addChild(lb);
			this->addChild(layer);
			PKNotificationCenter::getInstance()->removeObserver(this, "checkresources");
		});
	}
		break;
	}
	delete msg;		//清除数据，以免内存泄露
}
void SceneStart::LoadResources()
{
	//txtInfoTitle->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "载入资源":"Load Assets");
	txtLoading->setString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "正在载入资源":"Loading assets ...");
	txtPct->setString("99%");
	AvatarControl::GetInstance()->InitAvatars();
	//加载HelloWorld.png纹理
	totalNum = 1;
	for (int i = 0; i < totalNum; i++)
	{
		//加载纹理，回调Assets::loadingCallBack
		TextureCache::getInstance()->addImageAsync("img/game_page.png", CC_CALLBACK_1(SceneStart::ImageAsyncCallback, this));
		//TextureCache::getInstance()->addImageAsync("img/avatars/avatar1.plist", CC_CALLBACK_1(SceneStart::ImageAsyncCallback, this));
		//SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/avatars/avatar1.plist");
	}
}
void SceneStart::ImageAsyncCallback(Texture2D *texture)
{
	//currentNum初始值为0，每加载一张纹理值自加1
	++currentNum;
	char tmp[10];
	//求得百分比
	sprintf(tmp, "%d%%", (int)(((float)currentNum / totalNum) * 100));
	//设置标签显示的内容
	txtPct->setText(tmp);
	CCLOG("loading... %s", tmp);
	//如果当前加载的张数跟总数相等则进行场景跳转
	if (currentNum == totalNum)
	{
		//场景跳转
		CCLOG("loaded.");
		auto scene = SceneLogin::create();
		if (scene)
			Director::getInstance()->replaceScene(scene);
	}
}