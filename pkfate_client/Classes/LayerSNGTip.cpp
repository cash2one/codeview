#include "LayerSNGTip.h"
#include "cocostudio/CocoStudio.h"
#include "SoundControl.h"

bool LayerSNGTip::init()
{
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	/*	载入cocos studio 资源
	*/
	rootNode = CSLoader::createNode("dz/LayerSNGTip.csb");
	if (!rootNode)
		return false;

	addChild(rootNode);
	msgTip = rootNode->getChildByName<Text*>("tip");

	btn = rootNode->getChildByName<Button*>("btn");

	btn_confirm = rootNode->getChildByName<Button*>("btn_confirm");

	btn_cancel = rootNode->getChildByName<Button*>("btn_cancel");

	return true;
}

void LayerSNGTip::set(string tip,string label,bool only)
{
	msgTip->setString(tip);

	if (only){
		Node * node = rootNode->getChildByName<Node*>(label);
		if (node){
			node->setVisible(true);
		}
		btn->addClickEventListener(CC_CALLBACK_1(LayerSNGTip::btnClickHandler, this));
	}
	else{
		btn->setVisible(false);
		btn_confirm->setVisible(true);
		btn_cancel->setVisible(true);

		btn_confirm->addClickEventListener(CC_CALLBACK_1(LayerSNGTip::btnClickHandler, this));
		btn_cancel->addClickEventListener(CC_CALLBACK_1(LayerSNGTip::btnClickHandler, this));
	}
	
}

void LayerSNGTip::show()
{
	auto runningScene = Director::getInstance()->getRunningScene();

	if (runningScene){
		runningScene->addChild(this);
	}
}

void LayerSNGTip::btnClickHandler(Ref * sender)
{
	if (sender == btn){
		if (handler)
		{
			handler();
		}
	}
	else if (sender == btn_confirm)
	{
		if (handler)
		{
			handler();
		}
	}
	else if (sender == btn_cancel){
		if (cancelHandler)
		{
			cancelHandler();
		}
	}
	
	this->removeFromParent();
}