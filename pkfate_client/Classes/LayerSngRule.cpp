#include "LayerSngRule.h"
#include "cocostudio/CocoStudio.h"
#include "SoundControl.h"
#include "PKNotificationCenter.h"
#include "comm.h"

bool LayerSngRule::init()
{
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	/*	载入cocos studio 资源
	*/
	Node * rootNode = CSLoader::createNode("dz/LayerSngRule.csb");
	if (!rootNode)
		return false;

	addChild(rootNode);

	Layout *touch_layer;
	if (!(touch_layer = rootNode->getChildByName<Layout*>("touch_layer")))
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

	Button* btn_sign_up = rootNode->getChildByName<Button*>("btn_sign_up");

	btn_sign_up->addClickEventListener(CC_CALLBACK_1(LayerSngRule::btnClickHandler, this));

	return true;
}


void LayerSngRule::btnClickHandler(Ref * sender)
{
	if (handler){
		handler();
	}
	this->removeFromParent();
}