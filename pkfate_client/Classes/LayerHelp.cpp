#include "LayerHelp.h"
#include "Settings.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
#include "GuideControl.h"
#include "SoundControl.h"
using namespace CocosDenshion;


GuideTip::GuideTip() :x_space(40), y_space(40)
{
	tipBg = ImageView::create("img/guide.png");
	tipBg->setAnchorPoint(ccp(0, 0));
	tipBg->setScale9Enabled(true);
	unsigned int w = 90;
	unsigned int h = 60;
	tipBg->setCapInsets(CCRectMake(w, h, 390-w*2, 229-h*2));
	this->addChild(tipBg);
	tipMsg = Text::create();
	tipMsg->setFontName("font/msyhbd.ttf");
	tipMsg->setAnchorPoint(ccp(0, 0));
	tipMsg->setFontSize(30);
	tipMsg->setPosition(ccp(x_space,y_space));
	this->addChild(tipMsg);
}

void GuideTip::setTip(std::string tip)
{
	tipMsg->setText(tip);
	Size size = tipMsg->getContentSize();
	tipBg->setContentSize(CCSizeMake(size.width+2*x_space, size.height+2*y_space));
	this->setContentSize(tipBg->getContentSize());
}

// on "init" you need to initialize your instance
bool LayerHelp::init()
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
	Node* _nodeRoot = CSLoader::createNode("LayerHelp.csb");
	if (!_nodeRoot)
		return false;

	this->addChild(_nodeRoot);

	guidePageView = _nodeRoot->getChildByName<PageView*>("guide_pageView");
	btnExit = _nodeRoot->getChildByName<Button*>("btn_exit");

	if (!guidePageView || !btnExit)
		return false;

	btnExit->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
		this->removeFromParent();
	});

	const unsigned short split = 20;

	slipNode = Node::create();
	unsigned short slipWith = 0;
	for (int i = 0, j = guidePageView->getChildrenCount(); i < j; i++)
	{

		Sprite* point = Sprite::create("img/point_disable.png");
		point->setName(StringUtils::format("point%d", i));
		slipNode->addChild(point);
		point->setAnchorPoint(ccp(0, 0.5));
		point->setPositionX(i*(point->getContentSize().width + split));
		slipWith = slipWith + point->getContentSize().width;
	}

	slipNode->setPosition((guidePageView->getContentSize().width - slipWith) / 2, 100);
	this->addChild(slipNode);

	guidePageView->addEventListenerPageView(this, pagevieweventselector(LayerHelp::pageViewEvent));
	guidePageView->setCustomScrollThreshold(50);
    selectIndex=-1;
	selectPointIndex(0);

	return true;
}




void LayerHelp::selectPointIndex(unsigned short index)
{
	if (index<0||selectIndex == index) return;

	Sprite * lastPoint = slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", selectIndex));
	if (lastPoint)
	{
		lastPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_disable.png"));
	}
	

	Sprite* selectPoint = slipNode->getChildByName<Sprite*>(StringUtils::format("point%d", index));
	if (selectPoint)
	{
		selectPoint->setTexture(Director::getInstance()->getTextureCache()->addImage("img/point_normal.png"));
	}
	selectIndex = index;
}

void LayerHelp::pageViewEvent(cocos2d::Ref *pSender, PageViewEventType type)
{
	switch (type) 
	{
	case cocos2d::ui::PAGEVIEW_EVENT_TURNING:
	{
		PageView* pageView = dynamic_cast<PageView*>(pSender);
	
		selectPointIndex(pageView->getCurPageIndex());
	}
		break;

	default:
		break;
	}
}

































//ImageView* guide = ImageView::create(StringUtils::format("help/%d.jpg", i + 1));
//guide->setAnchorPoint(ccp(0, 0));

//Layout *guideLayout = Layout::create();
//guideLayout->addChild(guide);

//std::vector<GuideData*>*  guideList = GuideControl::GetInstance()->getGuideListByPageIndex(i+1);
//if (guideList&&(*guideList).size() > 0){
//	for (GuideData* guideData : *guideList) //添加引导说明
//	{
//		GuideTip *tip = GuideTip::create();
//		tip->setPosition(guideData->position);
//		tip->setAnchorPoint(guideData->anchor);
//		std::string tipMsg = Language::getStringByKey((guideData->tip).c_str());
//		tip->setTip(tipMsg);
//		guideLayout->addChild(tip);
//	}
//}

//guidePageView->addPage(guideLayout);