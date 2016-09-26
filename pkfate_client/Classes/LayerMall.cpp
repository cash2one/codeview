#include "LayerMall.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "CurlControl.h"
#include "UserControl.h"
#include "MallControl.h"
#include "PKNotificationCenter.h"
#include "SoundControl.h"
#include "TaskControl.h"
#include "ApiBill.h"
#include "LayerLoading.h"
LayerMall::LayerMall() :_selectItem(nullptr), _light(nullptr), _pBtnMall(nullptr),
_pBtnConversion(nullptr), _pBtnVip(nullptr), _pTagPitch(nullptr),
_pTxtNumChip(nullptr), _pTxtNumGoldCard(nullptr), _pTxtNumSilverCard(nullptr),
_pTxtNumDiamondCard(nullptr), _pBtnLottery(nullptr), _pTxtVipLevel(nullptr),
_pTxtVipLevel2(nullptr), _pLoadingBarVip(nullptr)
{

}
bool LayerMall::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("LayerMall.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;
	_light = Sprite::create("mall/light.png");
	//_light->setPosition(ccp(-15,25));
	_light->setPosition(ccp(0, 0));
	_light->retain();
	_btExit = (Button*)_nodeRoot->getChildByName("Button_Exit");
	_btExit->addClickEventListener(CC_CALLBACK_1(LayerMall::onButtonExitClicked, this));
	
	Layout* layoutMall = _nodeRoot->getChildByName<Layout*>("layoutMall");
	if (!layoutMall)
		return false;

	ScrollView * productList = (ScrollView*)layoutMall->getChildByName("productList");

	map<int, ProductData*> products = MallControl::GetInstance()->GetProducts();

	//To obtain language types
	std::string langType = UserDefault::getInstance()->getStringForKey("LangType");

	for (int i = 0; i < products.size(); i++)
	{
		Node* site = (Node*)productList->getChildByName(StringUtils::format("product%d", i));
		Node* goods = CSLoader::createNode("mall/product_mall_node.csb");
		if (!goods || !site)
			return false;
		Text *coin_num = (Text*)goods->getChildByName("coin_num");
		TextBMFont *price = (TextBMFont*)goods->getChildByName("price");
		Sprite *productIcon = (Sprite*)goods->getChildByName("product_icon");
		if (!coin_num || !price || !productIcon)
			return false;
		ProductData * productData = products[i + 1];
		Sprite *moneyIcon;

		if (langType == "zh")
		{
			price->setPositionX(price->getPositionX() + 55);
			price->setAnchorPoint(ccp(0, 0.5));
			moneyIcon = Sprite::create("mall/rmb.png");
			price->setString(StringUtils::format("%d", (int)productData->price));
			moneyIcon->setPosition(ccp(price->getPositionX() -
				moneyIcon->getContentSize().width,
				price->getPositionY() - 5));
		}
		else if (langType == "en")
		{
			price->setPositionX(price->getPositionX() + 90);
			price->setAnchorPoint(ccp(1, 0.5));
			moneyIcon = Sprite::create("mall/usd.png");
			price->setString(StringUtils::format("$%.2f", productData->price));
			moneyIcon->setPosition(ccp(price->getPositionX() + 20, price->getPositionY() - 5));
		}
		goods->addChild(moneyIcon);
		site->addChild(goods);
		coin_num->setString(Comm::GetFormatThousandsSeparatorFromInt64(productData->coin));
		Button *productBtn = (Button*)goods->getChildByName("product_btn");
		productBtn->setUserObject(String::create(productData->productId));
		productBtn->addTouchEventListener(this, toucheventselector(LayerMall::touchHandler));
	}

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
	dispatcher->addEventListenerWithSceneGraphPriority(listener, touch_layer);

	MallControl::GetInstance()->setMallView(this);
	UpdateBalance(nullptr);

	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_SELL);

	_pBtnMall = _nodeRoot->getChildByName<Button*>("btnMall");
	_pBtnConversion = _nodeRoot->getChildByName<Button*>("btnConversion");
	_pBtnVip = _nodeRoot->getChildByName<Button*>("btnVip");
	_pBtnLottery = _nodeRoot->getChildByName<Button*>(" btnLottery");
	_pTagPitch = _nodeRoot->getChildByName<Sprite*>("spTagPitch");
	_pLayoutMall = _nodeRoot->getChildByName<Layout*>("layoutMall");
	_pLayoutConversion = _nodeRoot->getChildByName<Layout*>("layoutConversion");
	_pLayoutVIP = _nodeRoot->getChildByName<Layout*>("layoutVIP");
	_pTxtNumChip = _nodeRoot->getChildByName<Text*>("txtNumChip");
	_pTxtNumGoldCard = _nodeRoot->getChildByName<Text*>("txtNumGoldCard");
	_pTxtNumSilverCard = _nodeRoot->getChildByName<Text*>("txtNumSilverCard");
	_pTxtNumDiamondCard = _nodeRoot->getChildByName<Text*>("txtNumDiamondCard");
	_pTxtVipLevel = _nodeRoot->getChildByName<Text*>("txtVipLevel");
	_pIconDiamondCard = _nodeRoot->getChildByName<ImageView*>("icon_diamond_card");

	if (!_pBtnMall || !_pBtnConversion || !_pBtnVip || !_pTagPitch||
		!_pLayoutVIP || !_pLayoutConversion || !_pLayoutMall || !_pTxtNumChip ||
		!_pTxtNumGoldCard || !_pTxtNumSilverCard || !_pTxtNumDiamondCard || 
		!_pBtnLottery || !_pTxtVipLevel || !_pIconDiamondCard)
		return false;

	_pLoadingBarVip = _pLayoutVIP->getChildByName<LoadingBar*>("loadingBarVIP");
	_pTxtVipLevel2 = _pLayoutVIP->getChildByName<Text*>("txtVipLevel");
	_pTxtVipLevelUpPrompt = _pLayoutVIP->getChildByName<Text*>("txtVipLevelUpPrompt");
	_pTextNumVipPercent = _pLoadingBarVip->getChildByName<Text*>("textNumVipPercent");

	Node* pNode0 = _pLayoutConversion->getChildByName<Button*>("product0");
	Node* pNode1 = _pLayoutConversion->getChildByName<Button*>("product1");
	if (!pNode0 || !pNode1)
		return false;

	_pBtnConversionProduct1 = pNode0->getChildByName<Button*>("product_btn1");
	_pBtnConversionProduct2 = pNode1->getChildByName<Button*>("product_btn2");

	if (!_pLoadingBarVip || !_pTxtVipLevel2 || !_pBtnConversionProduct1 ||
		!_pBtnConversionProduct2 || !_pTxtVipLevelUpPrompt || !_pTextNumVipPercent)
		return false;

	_pBtnMall->addClickEventListener(CC_CALLBACK_1(LayerMall::onTagButtonClicked, this));
	_pBtnConversion->addClickEventListener(CC_CALLBACK_1(LayerMall::onTagButtonClicked, this));
	_pBtnVip->addClickEventListener(CC_CALLBACK_1(LayerMall::onTagButtonClicked, this));

	_pBtnLottery->addClickEventListener([=](Ref *ref){
		TaskControl::GetInstance()->validateLoginAward();
	});
	
	if (!TaskControl::GetInstance()->isLoginAward())
	{
		_pBtnLottery->setVisible(false);
	}

	_pBtnConversionProduct1->addTouchEventListener(this, toucheventselector(LayerMall::touchHandler));
	_pBtnConversionProduct2->addTouchEventListener(this, toucheventselector(LayerMall::touchHandler));

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerMall::onHideBtnLotteryNotification), "HideBtnLottery", nullptr);

	PKNotificationCenter::getInstance()->postNotification("ListBalanceNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceGoldCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceSilverCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceDiamondCardNotification");
	PKNotificationCenter::getInstance()->postNotification("GetAllCoodsNotification");
	PKNotificationCenter::getInstance()->postNotification("GetBalanceRMBNotification");
	return true;
}

void LayerMall::UpdateBalance(Ref *ref)
{
	BalanceData *dataChip = BillControl::GetInstance()->GetBalanceData(0);
	BalanceData *dataGoldCard = BillControl::GetInstance()->GetBalanceData(1);
	BalanceData *dataSilverCard = BillControl::GetInstance()->GetBalanceData(2);
	BalanceData *dataDiamondCard = BillControl::GetInstance()->GetBalanceData(3);
	BalanceData *dataRMB = BillControl::GetInstance()->GetBalanceData(4);
	if (!dataChip || !dataGoldCard || !dataSilverCard || !dataDiamondCard || !dataRMB)
		return;

	if (!_pTxtNumChip || !_pTxtNumGoldCard || !_pTxtNumSilverCard || 
		!_pTxtNumDiamondCard || !_pTxtVipLevel || !_pTxtVipLevel2 || 
		!_pLoadingBarVip || !_pIconDiamondCard || !_pTxtVipLevelUpPrompt)
		return;

	_pTxtNumChip->setString(Comm::GetShortStringFromInt64(dataChip->balance));
	_pTxtNumGoldCard->setString(Comm::GetShortStringFromInt64(dataGoldCard->balance));
	_pTxtNumSilverCard->setString(Comm::GetShortStringFromInt64(dataSilverCard->balance));
	_pTxtNumDiamondCard->setString(Comm::GetShortStringFromInt64(dataDiamondCard->balance));
	
	//hide icon
	if (dataDiamondCard->balance <= 0)
	{
		_pTxtNumDiamondCard->setVisible(false);
		_pIconDiamondCard->setVisible(false);
	}
	else
	{
		_pTxtNumDiamondCard->setVisible(true);
		_pIconDiamondCard->setVisible(true);
	}

	_pTxtVipLevel->setString(Comm::GetShortStringFromInt64(
		BillControl::GetInstance()->getBetLevel(dataRMB->balance)));
	_pTxtVipLevel2->setString(Comm::GetShortStringFromInt64(
		BillControl::GetInstance()->getBetLevel(dataRMB->balance)));

	const int RMB_SCORE = 100;
	int level = BillControl::GetInstance()->getBetLevel(dataRMB->balance*RMB_SCORE);
	vector<int64_t> *levels = &BillControl::GetInstance()->_levelConfigs;
	int64_t numNetxLevelBet = levels->at(level + 1) - levels->at(level);
	int64_t numCurrentLevelBet = dataRMB->balance*RMB_SCORE - levels->at(level);
	float percent = (float)numCurrentLevelBet / (float)numNetxLevelBet * 100;
	_pLoadingBarVip->setPercent(percent);
	std::string strTempPrompt = StringUtils::format("(您当前的VIP等级为VIP%d,还差%dVIP额度可升到VIP", level, (numNetxLevelBet - numCurrentLevelBet)/100);
	std::string strPrompt = StringUtils::format("%s%d)", strTempPrompt.c_str(),level + 1);
	_pTxtVipLevelUpPrompt->setString(strPrompt);

	std::string strNumVipPercent = StringUtils::format("%d", percent);
	_pTextNumVipPercent->setPosition(ccp(
		_pLoadingBarVip->getContentSize().width*(percent / 100.00)+10,
		_pTextNumVipPercent->getPosition().y));
	_pTextNumVipPercent->setString(strNumVipPercent);
}

//#define PAY_TEST 
void LayerMall::touchHandler(Ref* sender, ui::Widget::TouchEventType type){
	
	//_selectItem->addChild(_light);
	//_light->release();
	
	switch (type)
	{
	case ui::Widget::TouchEventType::BEGAN:
	{
		auto* productItem = ((Button*)sender)->getParent();
		_light->removeFromParentAndCleanup(false);
		if (_selectItem&& productItem != _selectItem){
			
			_selectItem->runAction(EaseBackOut::create(ScaleTo::create(0.1f, 1)));
		}
		_selectItem = productItem;
		_selectItem->addChild(_light);
		productItem->runAction(EaseBackIn::create(ScaleTo::create(0.1f, 1.02f)));
	}
		break;
	case ui::Widget::TouchEventType::MOVED:
		
		break;
	case ui::Widget::TouchEventType::ENDED:
	{
	
		_light->removeFromParentAndCleanup(false);
		if (_selectItem){
			_selectItem->runAction(EaseBackOut::create(ScaleTo::create(0.1f, 1)));
		}

		//no cost prompt string
		const string strNoCostPromptArray[] = { "筹码不足,无法购买", "金卡不足,无法购买", "银卡不足,无法购买", "钻石卡不足,无法购买" };

		if (((Button*)sender)->getUserData() != nullptr)
		{
			SProductConversion *product = (SProductConversion*)((Button*)sender)->getUserData();
			int user_id = UserControl::GetInstance()->GetUserData()->user_id;
				BalanceData *pBalanceData = BillControl::GetInstance()->GetBalanceData(product->byCostCurrencyType);
				if (pBalanceData->balance < product->dCostAmount)
				{
					Tips(strNoCostPromptArray[product->byCostCurrencyType]);
				}
				else
				{
					Api::Bill::BuyGoods(user_id, product->nId, product->nGoodsAmount);
					LayerLoading::Wait();
				}
				break;
		}
		PKNotificationCenter::getInstance()->postNotification("SendPay", ((Button*)sender)->getUserObject());
	}
		//发起支付
		break;
	case ui::Widget::TouchEventType::CANCELED:
	{
		auto* productItem = ((Button*)sender)->getParent();
		productItem->runAction(EaseBackIn::create(ScaleTo::create(0.1f, 1)));
		_light->removeFromParentAndCleanup(false);
		_selectItem = nullptr;
	}
		break;
	default:
		break;
	}
}




LayerMall::~LayerMall()
{
	if (_light != nullptr){
		_light->release();//释放掉发光
	}
	// remove callback func
	MallControl::GetInstance()->setMallView(nullptr);

	PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void LayerMall::onButtonExitClicked(Ref *ref){
	CloseWithAction(this);
	
}

void LayerMall::onTagButtonClicked(Ref *ref)
{
	_pBtnMall->setTitleColor(Color3B(73, 125, 199));
	_pBtnConversion->setTitleColor(Color3B(73, 125, 199));
	_pBtnVip->setTitleColor(Color3B(73, 125, 199));
	_pLayoutVIP->setVisible(false);
	_pLayoutConversion->setVisible(false);
	_pLayoutMall->setVisible(false);

	Button *pBtnClick = dynamic_cast<Button*>(ref);
	if (!pBtnClick)
		return;
	pBtnClick->setTitleColor(Color3B(218, 227, 241));
	_pTagPitch->setPosition(pBtnClick->getPosition());

	if (_pBtnMall == pBtnClick)
	{
		_pLayoutMall->setVisible(true);
	}
	else if (_pBtnConversion == pBtnClick)
	{
		_pLayoutConversion->setVisible(true);
	}
	else if (_pBtnVip == pBtnClick)
	{
		_pLayoutVIP->setVisible(true);
	}
}

void LayerMall::onHideBtnLotteryNotification(Ref *pSender)
{
	if (!_pBtnLottery)
		return;

	_pBtnLottery->setVisible(false);
}

void  LayerMall::onEnterTransitionDidFinish()
{
	Node::onEnterTransitionDidFinish();

	UpdateBalance(nullptr);
}

bool LayerMall::refreshProductConversion()
{
	int size_pc = MallControl::GetInstance()->getProductConversion().size();
	const string nameProductNode[] = { "gold_card_node","silver_card_node"};
	for (int i = 0; i < size_pc; i++)
	{
		Node *product, *site, *productShow;
		Text *num_product, *num_cost;
		Button *btnProduct;
		string nameSiteNode = StringUtils::format("%dsite_node", i);
		site = _pLayoutConversion->getChildByName<Button*>(nameSiteNode);
		if (!site || site->getChildrenCount() > 0)
			return false;
		SProductConversion *pc = &MallControl::GetInstance()->getProductConversion()[i];
		product = CSLoader::createNode("mall/product_conversion_node.csb");
		if (!product)
			return false;
		productShow = product->getChildByName<Node*>(nameProductNode[pc->byGoodsCurrencyType-1]);
		num_cost = product->getChildByName<Text*>("num_cost");
		num_product = product->getChildByName<Text*>("num_product");
		btnProduct = product->getChildByName<Button*>("button_product");
		if (!num_cost || !num_product || !btnProduct || !productShow)
			return false;
		site->addChild(product);
		productShow->setVisible(true);
		num_cost->setString(StringUtils::format("%d", (int)pc->dCostAmount));
		num_product->setString(StringUtils::format("%d", pc->nGoodsAmount));
		btnProduct->addTouchEventListener(this, toucheventselector(LayerMall::touchHandler));
		btnProduct->setUserData(pc);
	}
	return true;
}
