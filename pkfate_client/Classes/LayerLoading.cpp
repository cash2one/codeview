#include "LayerLoading.h"
#include "cocostudio/CocoStudio.h"
#include "Settings.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
using namespace CocosDenshion;


// on "init" you need to initialize your instance
bool LayerLoading::init()
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
	RootNode = CSLoader::createNode("LayerLoading.csb");
	if (!RootNode)
		return false;
	txtLoading = RootNode->getChildByName<Text*>("txtLoading");
	if (!txtLoading)
		return false;
	txtLoading = RootNode->getChildByName<Text*>("txtLoading");
	if (!txtLoading)
		return false;
	Text* game_des = RootNode->getChildByName<Text*>("game_des");
	game_des->setText(Language::getStringByKey("LoadingIntroduction"));
	spLoading = RootNode->getChildByName<Sprite*>("spLoading");
	if (!spLoading)
		return false;
	btCancel = RootNode->getChildByName<Button*>("btCancel");
	if (!btCancel)
		return false;
	btCancel->setVisible(false);
	btCancel->addClickEventListener([=](Ref *ref){removeFromParent(); });
	scheduleOnce([=](float dt){btCancel->setVisible(true); }, 5.0f, "ShowButtonCancel");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/communication.plist");
	auto animation = Animation::create();
	for (int i = 0; i < 14; i++)
	{
		char file[50];
		sprintf(file, "communication%02d.png", i);
		//animation->addSpriteFrameWithFile(file);
		SpriteFrame *spf = SpriteFrameCache::getInstance()->getSpriteFrameByName(file);
		animation->addSpriteFrame(spf);
	}
	animation->setDelayPerUnit(1.5f / 14.0f);
	auto action = Animate::create(animation);
	spLoading->runAction(RepeatForever::create(action));
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerLoading::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	this->_isSteady = false;
	

	return true;
}
bool LayerLoading::onTouchBegan(Touch *touch, Event *event){
	return true;
}

bool LayerLoading::isSteady()
{
	return this->_isSteady;
}

void LayerLoading::setSteady(bool isSteady)
{
	this->_isSteady = isSteady;
}

void LayerLoading::SetString(std::string msg)
{   
	if (!msg.empty() && msg.size() > 0){
		txtLoading->setVisible(true);
		txtLoading->setString(msg);
	}
	else{
		txtLoading->setVisible(false);
	}
	
}

void LayerLoading::SetTimeout(float timeout)
{
    unschedule("ShowButtonCancel");
	if (timeout == 0){
		btCancel->setVisible(true);
	}
	else{
		scheduleOnce([=](float dt){btCancel->setVisible(true); }, timeout, "ShowButtonCancel");
	}
}

void LayerLoading::Tip(const std::string msg)
{
	SetString(msg);
	SetTimeout(0);
}

void LayerLoading::Wait(const std::string msg, float timeout, bool isSteady)
{

	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene){
		auto layerLoading = runningScene->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading == nullptr){
			layerLoading = LayerLoading::create();
			layerLoading->setName("LayerLoading");
            runningScene->addChild(layerLoading,2008);
		}
		layerLoading->setSteady(isSteady);
		if (!msg.empty()){
			layerLoading->SetString(msg);
		}
		
		layerLoading->SetTimeout(timeout);
	}
	
	
}



void LayerLoading::Stop(const std::string msg)
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene){
		auto layerLoading = runningScene->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading){
			layerLoading->Tip(msg);
		}
	}
}

void LayerLoading::CloseWithTip(const std::string msg,bool force )
{
	LayerLoading::Close();
	if (!msg.empty()){
		Tips(msg);
	}
}

void LayerLoading::Close(bool force)
{

	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene){
		auto layerLoading = runningScene->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading){
			if (force){
				layerLoading->setSteady(false);
			}
			if (!layerLoading->isSteady()){
				layerLoading->removeFromParent();
			}
		}
	}
	
}

void LayerLoading::SetVisible(bool visible)
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene){
		auto layerLoading = runningScene->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading){
			layerLoading->setVisible(visible);
		}
	}
}

