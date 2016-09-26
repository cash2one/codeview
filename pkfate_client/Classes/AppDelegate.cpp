#include "AppDelegate.h"
#include "SceneStart.h"
#include "SceneLogin.h"
#include "SoundControl.h"
#include "LayerLang.h"
#include "TimeControl.h"
#include "dzpkcomm.h"
#include "GuideControl.h"
#include "CCVideoPlayer.h"
#include "comm.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "ShareControl.h"
#endif

USING_NS_CC;
AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}
//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
	//set OpenGL context attributions,now can only set six attributions:
	//red,green,blue,alpha,depth,stencil
	GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

	GLView::setGLContextAttrs(glContextAttrs);
}

// If you want to use packages manager to install more packages, 
// don't modify or remove this function
static int register_all_packages()
{
	return 0; //flag for packages manager
}

inline bool AppDelegate::applicationDidFinishLaunching() {
	// initialize director
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	ShareControl::GetInstance()->initSDK();//加入分享
#endif
	
	auto director = Director::getInstance();
	auto glview = director->getOpenGLView();
	if(!glview) {
		glview = GLViewImpl::createWithRect("My Game", Rect(0, 0, 1280, 720), 1.0f);
		director->setOpenGLView(glview);
	}
	//屏幕适配
	glview->setContentScaleFactor(glview->getVisibleSize().width / 1920);
	glview->setDesignResolutionSize(1920, 1080, ResolutionPolicy::SHOW_ALL);
	// turn on display FPS
	//director->setDisplayStats(true);
	
	//UserDefault::getInstance()->setIntegerForKey("LoginType", -1);
	// set FPS. the default value is 1.0/60 if you don't call this
	director->setAnimationInterval(1.0 / 30);
	register_all_packages();
	srand((unsigned)time(0));
	//设置默认语言
	std::string langType = UserDefault::getInstance()->getStringForKey("LangType");
	if (langType.empty())
		UserDefault::getInstance()->setStringForKey("LangType", Application::getInstance()->getCurrentLanguageCode());
	//设置多语言资源目录,必须在设置默认语言之后调用
	LayerLang::SetLangResPath();

	/*
	FileUtils *fileUtils = FileUtils::getInstance();
	std::string fullPath = fileUtils->fullPathForFilename("SceneStart.csb");
	FILE* fp = fopen(fullPath.c_str(), "rb");	
	// sha local file.
	ssize_t size = 0;
	unsigned char *buffer = nullptr;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buffer = (unsigned char*)malloc(size);
	size = fread(buffer, sizeof(unsigned char), size, fp);
	fclose(fp);
	free(buffer);
	unsigned char *data = fileUtils->getFileData(fullPath, "rb", &size);
	return true;*/
	// test
	/*
	FileUtils *_fileUtils = FileUtils::getInstance();
	std::string path = _fileUtils->fullPathForFilename("LayerSetting.zip");
	if (path == "")
		return false;
	ssize_t size = 0;
	unsigned char *data = _fileUtils->getFileDataFromZip(path, "LayerSetting.csb", &size);
	if (data == nullptr || size <= 0)
		return false;
	unsigned char chars[32] = { 0 };
	SHA256(chars, data, size);
	free(data);
	_fileUtils->removeFile(path);
	//remove(path.c_str());
	CCLOG("file secret:%s", Char322Char64(chars));
	
	return true;
	// test end*/
	// create a scene. it's an autorelease object
	/*-- - pokerType2--5---- -
		-- - pokers2-- - D--12-- - 4--
		-- - pokers2-- - D--11-- - 6--
		-- - pokers2-- - S--5-- - 3--
		-- - pokers2-- - H--4-- - 2--
		-- - pokers2-- - H--3-- - 1--
		-- - pokers2-- - C--2-- - 7--*/
	/*-- - pokers2-- - D--10-- - 5--
		-- - pokers2-- - C--9-- - 2--
		-- - pokers2-- - H--8-- - 4--
		-- - pokers2-- - S--7-- - 7--
		-- - pokers2-- - S--6-- - 6--*/

	/*string handCards = "BAM5";
	string publicCards = "S9H9S8H2MK";
	int type = -1;
	map<int, bool> pokerMap;

	DzPokerUtils::validatePokerCards(handCards, publicCards, &type, pokerMap);

	for (auto item : pokerMap){
		CCLOG("%d----", item.first);
	}*/

	//int fchip = DzPokerUtils::getChipN(800, 100);
	//CCLOG("-----------------------%d----whi",DzPokerUtils::getSpaceN(1001));
	//CCLOG("what ---- is  %d", fchip);


	//TimeControl* timeControl = TimeControl::GetInstance();
	//auto scene = SceneStart::create();
	
    //auto scene = Scene::create();

//#if  COCOS2D_DEBUG == 1
//	CCLOG("hello");
//#endif
//
//	CCLOG("%s","hello");
//	OutputDebugString(L"hello");

	TimeControl::GetInstance();//用于拉取时间，不要注释
	// run
	auto scene = SceneStart::create();
	if (scene)
		director->runWithScene(scene);

	
	//string s = "334343.mp4";
	//string t = s.replace(s.find_last_of(".mp4")-3, 5, ".mp3");
	//CCLOG(t.c_str());
//
//    CCVideoPlayer *p = CCVideoPlayer::create("guide/video/t1.mp4");
//    if(p)
//    {
//        p->setPosition(ccp(960,540));
//        p->setContentSize(Size(p->m_width,p->m_height));
//        scene->addChild(p,1000);
//        p->playVideo(true);
//    }

//    //sleep(2);
//    p->stopVideo();
//    p = CCVideoLayer::create("guide/video/n5.mp4");
//    if(p)
//    {
//        p->setPosition(ccp(960,540));
//        p->setContentSize(Size(p->m_width,p->m_height));
//        scene->addChild(p,1000);
//        p->playVideo();
//    }

	//Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
	//	
	//	
	//	
	//	director->getScheduler()->schedule([=](float df){
	//		/*ShowTip(Language::getStringByKey("NotEnoughShouldBuy"), [=](){

	//			
	//		});*/

	//		CCLOG("hlolllo");
	//	}, this, 1.0f, 10000,1.0f,false,"da"); 
	//	
	//});

	//CCLOG("hello you go ");
	//
	////SoundControl::PlayMusic();

	//char * resultS = "{ \"version\":\"1.0.9\", \"status\" : \"0\" }";

	//char *version = "helloll";

	//int a = sizeof(version);

	//bool is = result==nullptr;

	//CCLOG("----------------%d", result);

	////strcpy(result, resultS);
	//cJSON *item;
	//if (!(item = cJSON_Parse(result))){
	//	CCLOG("---GetAppVersion- what-----");
	//	return "";
	//}
	
	/*CCLOG("----------hehe-----%s", filter.c_str());
	string key2 = "周恩来好样的";
	bool isNeed = Language::isNeedFitler(key2);*/

	//auto scene = Scene::create();
	//// run
	//if (scene)
	//	director->runWithScene(scene);


	//static int tindex = 1 ;
	//

	/*CCLOG("----%s", Language::getStringByKey("AddChipsTip"));
	director->getScheduler()->schedule([=](float df){
		Tips(StringUtils::format(Language::getStringByKey("AddChipsTip"), 40));
	}, this, 1.0f,false,"test");*/

	//ShowTip("对不起，您的筹码不足，是否购买筹码后继续", [=](){

	//	//LayerMall * mall = LayerMall::create(); //临时创建释放 avoid texture memory overflow
	//	//mall->setName("LayerMall");
	//	//this->addChild(mall, 2);
	//	//mall->setScale(0.1f);
	//	//mall->runAction(EaseBackOut::create(ScaleTo::create(0.3f, 1)));

	//});

	//director->getScheduler()->schedule([=](float df){
	//	/*ShowTip(Language::getStringByKey("NotEnoughShouldBuy"), [=](){

	//		

	//	});*/

	//	CCLOG("hlolllo");
	//}, this, 1.0f, 10000,1.0f,false,"da"); 
	

	return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
	if (!TimeControl::IS_PLAYING_GAME_IN_ROOM){
		Director::getInstance()->stopAnimation();
	}
	SoundControl::PauseMusic();

	// if you use SimpleAudioEngine, it must be pause
	// SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    

	TimeControl::GetInstance()->RequestServerTime(0.0f);
	Director::getInstance()->startAnimation();

	Director::getInstance()->getScheduler()->schedule([](float tm){
		SoundControl::ResumeMusic();
	}, this, 0, 0, 1.0f, false, "ResumeMusic");//延迟一秒恢复
	
	// if you use SimpleAudioEngine, it must resume here
	// SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
