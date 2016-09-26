#include "LayerLottery.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "CurlControl.h"
#include "UserControl.h"
#include "MallControl.h"
#include "PKNotificationCenter.h"
#include "LayerLoading.h"
#include "cmd.h"
#include "TaskControl.h"
#include "SoundControl.h"
#include "GuideControl.h"
void LotteryItem::setProperty(unsigned int coin,int coinTag)
{
	this->coin = coin;
	Sprite* lotteryIcon = nullptr;
	
	lotteryIcon = Sprite::create(StringUtils::format("lottery/coin_%d.png",coinTag));
	
	this->addChild(lotteryIcon,1);
	_coinText->setString(StringUtils::format("+%d",coin));
}

unsigned int LotteryItem::getCoin()
{
	return coin;
}

LotteryItem::LotteryItem()
{
	Sprite* normal = Sprite::create("lottery/normal.png");
	this->addChild(normal);
	this->setContentSize(normal->getContentSize());
	
	_coinText = Text::create();
	_coinText->setFontSize(36);
	_coinText->setTextColor(ccc4(194,170,118,255));
	_coinText->setFontName("font/msyhbd.ttf");
	_coinText->setAnchorPoint(ccp(1,0.5));
	_coinText->setPosition(ccp(160, -55));

	this->addChild(_coinText, 2);
}

LotteryItem::~LotteryItem()
{
	
}



bool LayerLottery::init()
{
	if (!Layer::init())
		return false;
	
	_nodeRoot = CSLoader::createNode("LayerLottery.csb");
	if (!_nodeRoot)
		return false;
	addChild(_nodeRoot);
	_light = _nodeRoot->getChildByName<Sprite*>("light");
	_startMask = _nodeRoot->getChildByName<Sprite*>("start_mask");
	_lotteryMask = _nodeRoot->getChildByName<Sprite*>("lottery_mask");
	_btnStart = (Button*)_nodeRoot->getChildByName("btn_start");
	//_lotteryMask->setZOrder(1);
	if (!_light || !_startMask || !_lotteryMask || !_btnStart)
		return false;
	_startMask->setVisible(false);

	_btnStart->addClickEventListener([=](Ref *ref){
		_startMask->setVisible(true);
		
		funcLottery();
		_btnStart->setEnabled(false); // 只能抽一次
	});

	const Point positions[12] = {
		ccp(455, 765), ccp(810, 765), ccp(1160, 765), ccp(1510, 765),
		ccp(1510, 580), ccp(1510, 395), ccp(1510, 210), ccp(1160, 210),
		ccp(810, 210), ccp(455, 210), ccp(455, 395), ccp(455, 580),
	};

	const  int lotteryCoins[12] = {
		2000, 10000, 100, 50, 50000, 200, 1000, 8000, 1500, 20000, 500, 5000
	};

	const int lotteryIcons[12] = { 7, 9, 2, 1, 9, 3, 5, 9, 6, 9, 4, 8 };

	_itemLayer = Node::create();

	_nodeRoot->addChild(_itemLayer);

	for (int i = 0, j = sizeof(positions) / sizeof(positions[1]); i < j; i++)
	{
		LotteryItem *item = LotteryItem::create();
		item->setProperty(lotteryCoins[i], lotteryIcons[i]);
		_itemLayer->addChild(item);
		item->setName(StringUtils::format("lottery%d", i));
		item->setPosition(positions[i]);
	}


	TaskControl::GetInstance()->setLotteryView(this);
	auto eventMaskLayer = CCLayerColor::create(ccc4(0, 0, 0, 1));
	addChild(eventMaskLayer, 1);

	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	//listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerLottery::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();

	
	dispatcher->addEventListenerWithSceneGraphPriority(listener, eventMaskLayer);


	return true;
}


bool LayerLottery::onTouchBegan(Touch *touch, Event *event){
	if (_enableQuit){
		this->scheduleOnce([=](float){this->removeFromParent(); }, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	}
	return true;
}

void LayerLottery::lotteryError()
{
	//LayerLoading *layerLoading = getChildByName<LayerLoading*>("LayerLoading");
	//if (layerLoading)
	//{
	//	layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
	//	layerLoading->btCancel->setVisible(true);
	//}
	//_startMask->setVisible(false);
	//_btnStart->setEnabled(true); // 开启抽奖状态

	_enableQuit = true;
}


void LayerLottery::funcLottery()
{
	// load loading layer
	/*auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	this->addChild(layerLoading);*/

	PKNotificationCenter::getInstance()->postNotification("GetLotteryAwardClick");
	
}

void LayerLottery::lottery(unsigned int bonus)
{
	_enableQuit = false;
	unsigned short index = getIndexOfLottery(bonus);
	if (index > 0)
	{
		lotteryAction(index);
	}
}

void LayerLottery::onLotteryCallBack(Ref *pSender)
{
	// 根据结果 运转抽奖 停下

	// 调用刷新balance
}

//index 为ui定义的index，后端传递的需要转换
void LayerLottery::lotteryAction(unsigned short index)
{
	if (index <= 0) return;

	unschedule("LotterySchedule");
	order = 0;
	add = 0;
	step = 1;
	minus = 6;
	minusStep = 1;
	addIndex = 12 * 2 + index - minus; //转2圈，缓冲minus
	
	schedule([&](float dt){
		lotterySchedule(dt);
	}, 0.1f, "LotterySchedule");
	
}

int LayerLottery::getIndexOfLottery(unsigned int bonus)
{
	Vector<Node*> childs = _itemLayer->getChildren();
	for (int i = 0,j=childs.size(); i < j; i++)
	{
		LotteryItem * item = (LotteryItem *)childs.at(i);
		if (item->getCoin() == bonus)
		{
			return i+1;
		}
	}
	return 0;
}


void LayerLottery::lotterySchedule(float dt)
{
	if (add%step == 0)
	{
		LotteryItem* lotteryItem = _itemLayer->getChildByName<LotteryItem*>(StringUtils::format("lottery%d", order % 12));
		_light->retain();
		_light->removeFromParent();
		lotteryItem->addChild(_light);
		_light->setPosition( 0,  0);
		_light->setVisible(true);
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_REWARD);
		if (order >= addIndex)
		{
			//减速
			step = step + minusStep;//线性 minus减速
			if (step > minus*minusStep){
				unschedule("LotterySchedule"); //结束 发放奖励
				lotteryFinish(order % 12);
			}
		}
		add = 0;//归0
		order = order + 1;
	}
	add = add + 1;
}

void LayerLottery::lotteryFinish(unsigned short index)
{
	_lotteryMask->setVisible(true);
	LotteryItem* lotteryItem = _itemLayer->getChildByName<LotteryItem*>(StringUtils::format("lottery%d", index));
	unsigned int coin = lotteryItem->getCoin();

	Sprite* lotteryIcon = nullptr;
	if (coin < 1000)
	{
		lotteryIcon = Sprite::create("lottery/coin_1.png");
	}
	else if (coin<10000)
	{
		lotteryIcon = Sprite::create("lottery/coin_2.png");
	}
	else{
		lotteryIcon = Sprite::create("lottery/coin_2.png");
	}
	this->addChild(lotteryIcon);
	lotteryIcon->setPosition(960, 540);
	Text *coinText = Text::create();
	coinText->setPosition(ccp(lotteryIcon->getContentSize().width/2, 0));
	coinText->setFontSize(48);
	coinText->setTextColor(ccc4(194, 170, 118, 255));
	coinText->setFontName("font/msyhbd.ttf");
	lotteryIcon->addChild(coinText);
	coinText->setString(StringUtils::format("+%d", coin));
	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);
	
	CCAction *action = RepeatForever::create(Sequence::create(FadeOut::create(1), FadeIn::create(1), nullptr));
	_light->runAction(action);

	lotteryIcon->runAction(Sequence::create(
		EaseBackOut::create(ScaleTo::create(0.5f, 2.0f)),
		CCDelayTime::create(0.5f),
		CallFunc::create([&]{
				  _enableQuit = true;
	    }), nullptr));
}

LayerLottery::~LayerLottery()
{
	unscheduleAllSelectors();
	//// remove callback func
	TaskControl::GetInstance()->setLotteryView(nullptr);

	PKNotificationCenter::getInstance()->postNotification("HideBtnLottery", nullptr);
}

LayerLottery::LayerLottery() :_enableQuit(false)
{

  
}

void LayerLottery::onButtonExitClicked(Ref *ref){
	CloseWithAction(this);
	
}
