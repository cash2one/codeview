#include "LayerBjlHelp.h"
#include "Settings.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
#include "GuideControl.h"
#include "LayerHelp.h"
#include "SoundControl.h"
using namespace CocosDenshion;




// on "init" you need to initialize your instance
bool LayerBjlHelp::init()
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
	Node* _nodeRoot = CSLoader::createNode("bjl/LayerBjlHelp.csb");
	if (!_nodeRoot)
		return false;
	

	this->addChild(_nodeRoot);

	optPanel = _nodeRoot->getChildByName<Layout*>("opt_panel");
	if (!optPanel)
		return false;
	rulePanel = _nodeRoot->getChildByName<Layout*>("rule_panel");
	if (!rulePanel)
		return false;

	bg = _nodeRoot->getChildByName<Sprite*>("bg");
	btnOpt = _nodeRoot->getChildByName<Button*>("btn_opt");
	btnRule = _nodeRoot->getChildByName<Button*>("btn_rule");

	if (!bg || !btnOpt || !btnRule)
		return false;
   
	
	chooseOptPanel(true);

	btnOpt->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		chooseOptPanel(true);
	});

	btnRule->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		chooseOptPanel(false);
	});

	btnOpt_guidePageView = optPanel->getChildByName<PageView*>("guide_pageView");
	btnRule_guidePageView = rulePanel->getChildByName<PageView*>("guide_pageView");
	btnExit = _nodeRoot->getChildByName<Button*>("btn_exit");

	if (!btnOpt_guidePageView || !btnExit || !btnRule_guidePageView)
		return false;

	btnExit->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
		this->removeFromParent();
	});

	const unsigned short split = 20;

	btnOpt_slipNode = Node::create();
	unsigned short slipWith = 0;
	for (int i = 0, j = btnOpt_guidePageView->getChildrenCount(); i < j; i++)
	{
		

		Sprite* point = Sprite::create("img/point_disable.png");
		point->setName(StringUtils::format("point%d", i));
		btnOpt_slipNode->addChild(point);
		point->setAnchorPoint(ccp(0, 0.5));
		point->setPositionX(i*(point->getContentSize().width + split));
		slipWith = slipWith + point->getContentSize().width;
	}

	btnOpt_slipNode->setPosition((btnOpt_guidePageView->getContentSize().width - slipWith) / 2, 10);
	optPanel->addChild(btnOpt_slipNode);

	btnOpt_guidePageView->setCustomScrollThreshold(40);
	btnOpt_guidePageView->addEventListenerPageView(this, pagevieweventselector(LayerBjlHelp::btnOpt_pageViewEvent));
	btnOpt_selectIndex = -1;
	selectOptPointIndex(0);


	btnRule_slipNode = Node::create();
	
	for (int i = 0, j = btnRule_guidePageView->getChildrenCount(); i < j; i++)
	{


		Sprite* point = Sprite::create("img/point_disable.png");
		point->setName(StringUtils::format("point%d", i));
		btnRule_slipNode->addChild(point);
		point->setAnchorPoint(ccp(0, 0.5));
		point->setPositionX(i*(point->getContentSize().width + split));
		slipWith = slipWith + point->getContentSize().width;
	}

	btnRule_slipNode->setVisible(btnRule_guidePageView->getChildrenCount() >= 2);

	btnRule_slipNode->setPosition((btnRule_guidePageView->getContentSize().width - slipWith) / 2, 10);
	rulePanel->addChild(btnRule_slipNode);
	btnRule_guidePageView->setCustomScrollThreshold(40);
	btnRule_guidePageView->addEventListenerPageView(this, pagevieweventselector(LayerBjlHelp::btnRule_pageViewEvent));
	btnRule_selectIndex = -1;
	selectRulePointIndex(0);
	

	Layout* layoutVS = _nodeRoot->getChildByName<Layout*>("layoutVS");
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerBjlHelp::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, layoutVS);



	return true;
}



void LayerBjlHelp::chooseOptPanel(bool isOpt)
{
	btnRule->setBright(isOpt);
	btnRule->setEnabled(isOpt);
	rulePanel->setVisible(!isOpt);
	btnRule->getChildByName("label_normal")->setVisible(isOpt);
	btnRule->getChildByName("label_select")->setVisible(!isOpt);

	btnOpt->setBright(!isOpt);
	btnOpt->setEnabled(!isOpt); //default 选择
	optPanel->setVisible(isOpt);
	btnOpt->getChildByName("label_normal")->setVisible(!isOpt);
	btnOpt->getChildByName("label_select")->setVisible(isOpt);
}

void LayerBjlHelp::selectOptPointIndex(unsigned short index)
{
	if (index<0 || btnOpt_selectIndex == index) return;

	Sprite * lastPoint = btnOpt_slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", btnOpt_selectIndex));
	if (lastPoint)
	{
		lastPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_disable.png"));
	}


	Sprite* selectPoint = btnOpt_slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", index));
	if (selectPoint)
	{
		selectPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_normal.png"));
	}
	btnOpt_selectIndex = index;
	
}

void LayerBjlHelp::selectRulePointIndex(unsigned short index)
{
	if (index<0 || btnRule_selectIndex == index) return;

	Sprite * lastPoint = btnRule_slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", btnRule_selectIndex));
	if (lastPoint)
	{
		lastPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_disable.png"));
	}


	Sprite* selectPoint = btnRule_slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", index));
	if (selectPoint)
	{
		selectPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_normal.png"));
	}
	btnRule_selectIndex = index;
}

void LayerBjlHelp::btnOpt_pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type)
{
	switch (type) 
	{
	case cocos2d::ui::PAGEVIEW_EVENT_TURNING:
	{
		PageView* pageView = dynamic_cast<PageView*>(pSender);
	
		selectOptPointIndex(pageView->getCurPageIndex());
	}
		break;

	default:
		break;
	}
}

void LayerBjlHelp::btnRule_pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type)
{
	switch (type)
	{
	case cocos2d::ui::PAGEVIEW_EVENT_TURNING:
	{
		PageView* pageView = dynamic_cast<PageView*>(pSender);

		selectRulePointIndex(pageView->getCurPageIndex());
	}
		break;

	default:
		break;
	}
}



bool LayerBjlHelp::onTouchBegan(Touch *touch, Event *event){
	if (!bg->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce([=](float){

		CloseWithAction(this);

	}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	return true;
}