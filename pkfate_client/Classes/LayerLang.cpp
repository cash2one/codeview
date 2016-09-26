#include "LayerLang.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
#include "SceneLogin.h"
#include "SoundControl.h"
#include "AudioEngine.h"

// on "init" you need to initialize your instance
bool LayerLang::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerLang.csb");
	if (!RootNode)
		return false;
	if (!(panelSetting = RootNode->getChildByName<Layout*>("panelSetting")))
		return false;
	//auto lbTitle = panelSetting->getChildByName<Text*>("lbTitle");
	//if (!lbTitle)
	//	return false;
	/*lbTitle->setString(Language::getStringByKey("LangSetting"));*/
	if (!(btZh = panelSetting->getChildByName<Button*>("btZh")))
		return false;
	if (!(btEn = panelSetting->getChildByName<Button*>("btEn")))
		return false;
	this->addChild(RootNode);
	// add button event
	btZh->addClickEventListener(CC_CALLBACK_1(LayerLang::btClicked, this));
	btEn->addClickEventListener(CC_CALLBACK_1(LayerLang::btClicked, this));
	
	//load current lang
	setButtonStatus(UserDefault::getInstance()->getStringForKey("LangType") == "zh");
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerLang::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerLang::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerLang::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}
bool LayerLang::onTouchBegan(Touch *touch, Event *event){
	if (!panelSetting->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce([=](float dt){
		CloseWithAction(this);
	
	}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	return true;
}

void LayerLang::onTouchMoved(Touch *touch, Event *event){

}

void LayerLang::onTouchEnded(Touch* touch, Event* event){

}

void LayerLang::setButtonStatus(bool isZh)
{
	btZh->setHighlighted(isZh);
	btZh->setEnabled(!isZh);
	btEn->setHighlighted(!isZh);
	btEn->setEnabled(isZh);
}
//根据当前语言，设置搜索多语言资源路径
void LayerLang::SetLangResPath()
{
	std::vector<std::string> paths;
	std::string langDir = UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "reszh" : "resen";
	//开始添加资源目录，必须按照此优先级顺序添加，以便用更新资源替代程序自带资源
	
	//paths.push_back(Comm::GetResDir() + "res");	//添加下载资源通用目录
	//paths.push_back(Comm::GetResDir() + langDir);	//添加下载资源多语言目录

	string channel = GetRegisterChannel();

	
	if (channel.find("pkfate_") != string::npos)
	{
		paths.push_back(StringUtils::format("channel/%s/res", "jiajiale"));
		paths.push_back(StringUtils::format("channel/%s/%s", "jiajiale", langDir.c_str()));
	}

	paths.push_back("res");			//添加程序自带资源目录
	paths.push_back(langDir);		//添加程序自带资源多语言目录
	FileUtils::getInstance()->setSearchPaths(paths);
}


void LayerLang::btClicked(Ref *ref)
{
	bool isZh = (ref == btZh);
	setButtonStatus(isZh);
	UserDefault::getInstance()->setStringForKey("LangType", isZh ? "zh" : "en");
	SetLangResPath();
	experimental::AudioEngine::uncacheAll();		//清空声音缓存，以免播放其他语种音效
	SoundControl::PlayPassMusic();						//重新播放背景音乐
	//重新载入登录界面
	Director::getInstance()->replaceScene(SceneLogin::create());
}