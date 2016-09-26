#include "LayerDzAddScore.h"
#include "Settings.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
#include "GuideControl.h"
#include "LayerHelp.h"
#include "SoundControl.h"
#include "DZControl.h"
#include "ApiGame.h"
#include "BillControl.h"
using namespace CocosDenshion;




// on "init" you need to initialize your instance
bool LayerDzAddScore::init()
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
	Node* _nodeRoot = CSLoader::createNode("dz/LayerDzAddScore.csb");
	if (!_nodeRoot)
		return false;
	

	this->addChild(_nodeRoot);
	text_num = _nodeRoot->getChildByName<TextField*>("text_num");

	label_des = _nodeRoot->getChildByName<Text*>("label_des");
	label_change = _nodeRoot->getChildByName<Text*>("label_change");
	
	btn_left = _nodeRoot->getChildByName<Button*>("btn_left");
	btn_right = _nodeRoot->getChildByName<Button*>("btn_right");

	btn_confirm = _nodeRoot->getChildByName<Button*>("btn_confirm");

	btn_left->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		changeCount(-1);

	});

	btn_right->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		changeCount(1);
	});

	btn_confirm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
	});
	btn_confirm->setPressedActionEnabled(true);
	Layout *touch_layer;
	if (!(touch_layer = _nodeRoot->getChildByName<Layout*>("touch_layer")))
		return false;

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
			this->scheduleOnce([=](float dt){

			CloseWithAction(this);

		}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
		return true;
	};

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	int buyTimes = DZControl::GetInstance()->buyChipTimes;
	if (buyTimes <= 0){
		label_des->setString(Language::getStringByKey("LackExchange"));
	}
	count = 0;
	BalanceData *data = BillControl::GetInstance()->GetBalanceData(DZControl::GetInstance()->gameRoomType);
	if (data){
		totalCount = data->balance;
	}
	else{
		totalCount = 0;
	}

	btn_confirm->addClickEventListener(CC_CALLBACK_1(LayerDzAddScore::btnClickHandler, this));

	return true;
}

void LayerDzAddScore::changeCount(int add)
{
	
	int buyTimes = DZControl::GetInstance()->buyChipTimes;
	if (buyTimes <= 0){
		Tips("buy time < 0");
		return;
	}

	if (count <= 0 && add <0){
		return;
	}

	if(count>= count>=totalCount && add >0){
		return;
	}

	count = count + add;

	text_num->setString(toString(count));

	label_change->setString(StringUtils::format(Language::getStringByKey("addStoreTip"), count * 1000));
}

void LayerDzAddScore::btnClickHandler(Ref * sender)
{
	Api::Game::mo_sng_buy_chip(count);
	this->removeFromParent();
}

