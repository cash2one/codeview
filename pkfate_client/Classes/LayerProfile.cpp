#include "LayerProfile.h"
#include "Settings.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
#include "SoundControl.h"
using namespace CocosDenshion;

// on "init" you need to initialize your instance
bool LayerProfile::init()
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
	Node* _nodeRoot = CSLoader::createNode("LayerProfile.csb");
	if (!_nodeRoot)
		return false;

	this->addChild(_nodeRoot);

	btnCancel = _nodeRoot->getChildByName<Button*>("btn_cancel");

	btnOk = _nodeRoot->getChildByName<Button*>("btn_ok");

	btnAgree = _nodeRoot->getChildByName<Button*>("btn_agree");

	if (!btnAgree || !btnCancel || !btnOk)
		return false;

	btnOk->addClickEventListener([=](Ref *ref){
		//
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CONFIRM);
		this->removeFromParent();
	});

	btnOk->setTitleText(Language::getStringByKey("Confirm"));
	
	btnCancel->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CLOSE);
		this->removeFromParent();
	});
	
	btnCancel->setTitleText(Language::getStringByKey("Cancel"));

	btnAgree->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CONFIRM);
		this->_profileCallback();
		this->removeFromParent();
	});

	btnAgree->setTitleText(Language::getStringByKey("Agree"));

	ScrollView * textScrollView = (ScrollView*)_nodeRoot->getChildByName("text_scrollView");


	Sprite* privacy = Sprite::create("img/privacy.png");
	privacy->setAnchorPoint(ccp(0, 0));
	
	textScrollView->addChild(privacy);
	textScrollView->setInnerContainerSize(CCSizeMake(1600, privacy->getContentSize().height));
	
	btnOk->setVisible(false);
	return true;
}

void LayerProfile::addProfileCallback(const std::function<void()> &callback)
{
	this->_profileCallback = callback;
}

void LayerProfile::setWithAgree(bool withAgree)
{
	btnAgree->setVisible(withAgree);
	btnCancel->setVisible(withAgree);
	btnOk->setVisible(!withAgree);
}


//void LayerProfile::onButtonExitClicked(Ref *ref){
//	this->setCascadeOpacityEnabled(true);
//	this->runAction(Sequence::create(
//		Spawn::create(EaseBackIn::create(ScaleTo::create(0.2f, 0.2f)),
//		FadeOut::create(0.2f), nullptr),
//		CallFunc::create([&]{this->removeFromParent(); }), nullptr));
//
//}
