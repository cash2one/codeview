#include "SceneBjlRoomP.h"
#include "cocostudio/CocoStudio.h"
#include "ui/UIListView.h"
#include "SpriteCmSub.h"
#include "LayerSetting.h"
#include "Settings.h"
#include "ApiGame.h"
#include "ApiBill.h"
#include "BillControl.h"
#include "LayerBjlCalculator.h"
#include "cmd.h"
#include "SoundControl.h"
#include "SceneReports.h"
#include "PKNotificationCenter.h"
#include "LayerBjlHelp.h"
#include "TaskControl.h"
#include "SoundControl.h"
using namespace std;
static int _currentRoomType = 0;
SceneBjlRoomP* SceneBjlRoomP::create(int maxBet)
{
	SceneBjlRoomP *pRet = new(std::nothrow) SceneBjlRoomP();
	if (pRet && pRet->init(maxBet))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}
bool SceneBjlRoomP::init(int maxBet)
{
	if (maxBet <= 0)
		return false;
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}
	//this->setKeypadEnabled(true);
	_maxBet = maxBet;
	_userData = UserControl::GetInstance()->GetUserData();
	_betResultData = nullptr;
	if (!_userData)
		return false;
	_roomData = nullptr;
	auto listener = EventListenerKeyboard::create();
	listener->onKeyReleased = CC_CALLBACK_2(SceneBjlRoomP::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	TimeForShowResult = 5;
	TimeLeft = 99;
	TimeLeftMax = 99;
	_statusGame = STATUSBJLGAME::BETSTART;
	_layerBjlFpS = NULL;

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	CCLOG("Visible size:%.2f,%.2f", visibleSize.width, visibleSize.height);
	/////////////////////////////
	// 3. add your codes below...

	RootNode = CSLoader::createNode("bjl/SceneBjlRoom.csb");
	if (!RootNode)
		return false;
	
	TextTimeLeft = (Text*)RootNode->getChildByName("Text_TimeLeft");
	if (!TextTimeLeft)
		return false;
	TextTimeLeft->setVisible(false);
	if (!(panelCards = RootNode->getChildByName<Layout*>("panelCards")))
		return false;
	if (!(TextPtP = panelCards->getChildByName<TextBMFont*>("TextPtP")))
		return false;
	if (!(TextPtB = panelCards->getChildByName<TextBMFont*>("TextPtB")))
		return false;
	panelCards->setTouchEnabled(false);
	panelCards->setVisible(false);
	TextPtP->setText("");
	TextPtB->setText("");

	auto panelCmArea = RootNode->getChildByName<Layout*>("panelCmArea");
	if (!panelCmArea)
		return false;
	panelCmArea->setTouchEnabled(false);
	lbMyChip = panelCmArea->getChildByName<Text*>("lbMyChip");
	if (!lbMyChip)
		return false;
	lbMyChip->setString(Language::getStringByKey("MyChip"));

	// 加载资源
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistChips.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistCard.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistCardNP.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistSmallCard.plist");
	//计算需要显示的筹码
	_cmInfos.clear();
	auto listener1 = EventListenerTouchOneByOne::create();
	listener1->setSwallowTouches(true);
	listener1->onTouchBegan = CC_CALLBACK_2(SceneBjlRoomP::onCmClick, this);
	int max_bet = maxBet / 1000;
	string strMaxBet;
	CMINFO *cmInfo;
	Sprite *sp;
	for (int i = 0; i < 4; i++)
	{
		strMaxBet = Comm::GetStringFromInt64(max_bet);
		cmInfo = new CMINFO();
		cmInfo->amount = max_bet;
		cmInfo->imgOn = StringUtils::format("chip_%s_on.png", strMaxBet.c_str());
		cmInfo->imgOff = StringUtils::format("chip_%s_off.png", strMaxBet.c_str());
		cmInfo->imgSmall = StringUtils::format("chip_%s_small.png", strMaxBet.c_str());
		_cmInfos.push_back(cmInfo);
		sp = (Sprite *)panelCmArea->getChildByName(StringUtils::format("cm%d", i + 1));
		if (!sp)
			return false;
		_eventDispatcher->addEventListenerWithSceneGraphPriority(i == 0 ? listener1 : listener1->clone(), sp);
		sp->setUserData((void*)cmInfo);
		sp->setSpriteFrame(cmInfo->imgOff);
		_spriteCms.spCms.pushBack(new SpriteCm(cmInfo->amount, sp, cmInfo->imgOn, cmInfo->imgOff));
		max_bet *= 10;
	}

	//隐藏桌面玩家面板
	RootNode->getChildByName("panelPlayers")->setVisible(false);
	auto panelBetArea = RootNode->getChildByName<Layout*>("panelBetArea");
	if (!panelBetArea)
		return false;
	panelBetArea->setTouchEnabled(false);
	auto panelAmountInfos = RootNode->getChildByName<Layout*>("panelAmountInfos");
	if (!panelAmountInfos)
		return false;
	panelAmountInfos->setTouchEnabled(false);
	panelAmountInfos->setZOrder(2);
	//隐藏房间玩家投注总额
	for (auto node : panelAmountInfos->getChildren())
		if (node->getName().find("All") != -1)
			node->setVisible(false);
	auto listener2 = EventListenerTouchOneByOne::create();
	listener2->setSwallowTouches(true);
	listener2->onTouchBegan = CC_CALLBACK_2(SceneBjlRoomP::onCmBet, this);
	sp = (Sprite *)panelBetArea->getChildByName("tbbjlp");
	if (!sp)
		return false;
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener2, sp);
	_spriteBetAreas.addBetArea(BJLBETTYPE::BJLBETP, sp, (Sprite *)panelBetArea->getChildByName("lightP"),
		(Text*)panelAmountInfos->getChildByName("lbAmountP"), panelAmountInfos->getChildByName<Sprite*>("imgBgAmountP"),
		(Sprite *)panelBetArea->getChildByName("p_green"), (Sprite *)panelBetArea->getChildByName("p_red")
		);
	if (!_irregularTest.AddSprite(sp, "bjl/room/BetAreaP.png"))
		return false;		//添加sprite到不规则控制类
	sp = (Sprite *)panelBetArea->getChildByName("tbbjlpp");
	if (!sp)
		return false;
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener2->clone(), sp);
	_spriteBetAreas.addBetArea(BJLBETTYPE::BJLBETPP, sp, (Sprite *)panelBetArea->getChildByName("lightPP"),
		(Text*)panelAmountInfos->getChildByName("lbAmountPP"), panelAmountInfos->getChildByName<Sprite*>("imgBgAmountPP"),
		(Sprite *)panelBetArea->getChildByName("pp_green"), (Sprite *)panelBetArea->getChildByName("pp_red")
		);
	if (!_irregularTest.AddSprite(sp, "bjl/room/BetAreaPP.png"))
		return false;		//添加sprite到不规则控制类
	sp = (Sprite *)panelBetArea->getChildByName("tbbjlbp");
	if (!sp)
		return false;
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener2->clone(), sp);
	_spriteBetAreas.addBetArea(BJLBETTYPE::BJLBETBP, sp, (Sprite *)panelBetArea->getChildByName("lightBP"),
		(Text*)panelAmountInfos->getChildByName("lbAmountBP"), panelAmountInfos->getChildByName<Sprite*>("imgBgAmountBP"),
		(Sprite *)panelBetArea->getChildByName("bp_green"), (Sprite *)panelBetArea->getChildByName("bp_red")
		);
	if (!_irregularTest.AddSprite(sp, "bjl/room/BetAreaBP.png"))
		return false;		//添加sprite到不规则控制类
	sp = (Sprite *)panelBetArea->getChildByName("tbbjlb");
	if (!sp)
		return false;
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener2->clone(), sp);
	_spriteBetAreas.addBetArea(BJLBETTYPE::BJLBETB, sp, (Sprite *)panelBetArea->getChildByName("lightB"),
		(Text*)panelAmountInfos->getChildByName("lbAmountB"), panelAmountInfos->getChildByName<Sprite*>("imgBgAmountB"),
		(Sprite *)panelBetArea->getChildByName("b_green"), (Sprite *)panelBetArea->getChildByName("b_red")
		);
	if (!_irregularTest.AddSprite(sp, "bjl/room/BetAreaB.png"))
		return false;		//添加sprite到不规则控制类
	sp = (Sprite *)panelBetArea->getChildByName("tbbjlt");
	if (!sp)
		return false;
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener2->clone(), sp);
	_spriteBetAreas.addBetArea(BJLBETTYPE::BJLBETT, sp, (Sprite *)panelBetArea->getChildByName("lightT"),
		(Text*)panelAmountInfos->getChildByName("lbAmountT"), panelAmountInfos->getChildByName<Sprite*>("imgBgAmountT"), \
			(Sprite *)panelBetArea->getChildByName("t_green"), (Sprite *)panelBetArea->getChildByName("t_red")
		);
	if (!_irregularTest.AddSprite(sp, "bjl/room/BetAreaT.png"))
		return false;		//添加sprite到不规则控制类

	// Ìí¼Ó°´Å¥ÊÂ¼þ
	_btBet = (Button *)panelCmArea->getChildByName("Button_Bet");
	if (!_btBet)
		return false;
	_btBet->addClickEventListener(CC_CALLBACK_1(SceneBjlRoomP::onButtonBetClicked, this));
	_btBet->setTitleText(Language::getStringByKey("SkipCard"));
	EnableButton(_btBet, true);
	_btCancel = (Button *)panelCmArea->getChildByName("Button_Cancel");
	if (!_btCancel)
		return false;
	_btCancel->setTitleText(Language::getStringByKey("Cancel"));
	EnableButton(_btCancel, false);
	_btCancel->addClickEventListener(CC_CALLBACK_1(SceneBjlRoomP::onButtonCancelClicked, this));
	
	this->addChild(RootNode);
	schedule(schedule_selector(SceneBjlRoomP::funcTimeLeft), 1.0f);
	//¼ÆËãÅÆÃæËõ·ÅÐÅÏ¢
	sp = (Sprite *)panelCards->getChildByName("Panel_init");
	if (!sp)
		return false;
	Rect rect = sp->getBoundingBox();
	_ptCardInit = rect.origin;
	spriteCardB = Sprite::createWithSpriteFrameName("SmallCardHA.png");
	Size sizeCard = spriteCardB->getContentSize();
	_scaleCard0 = rect.size.width / sizeCard.width;
	layerCardP = (Layer *)panelCards->getChildByName("Panel_p");
	if (!layerCardP)
		return false;
	layerCardB = (Layer *)panelCards->getChildByName("Panel_b");
	if (!layerCardB)
		return false;
	rect = layerCardP->getBoundingBox();
	_ptCardP = rect.origin;
	rect = layerCardB->getBoundingBox();
	_ptCardB = rect.origin;
	_widthCard = rect.size.width / 3;
	_scaleCard1 = _widthCard / sizeCard.width;
	sp = (Sprite *)RootNode->getChildByName("Panel_BankerCm");
	if (!sp)
		return false;
	_ptCmLose = sp->getPosition();
	//载入结果显示layer
	_layerResult = LayerBjlResult::create();
	if (!_layerResult)
		return false;
	_layerResult->setVisible(false);
	addChild(_layerResult, 1);
	//user amount
	_textUserAmount = (TextBMFont*)panelCmArea->getChildByName("Text_Amount");
	if (!_textUserAmount)
		return false;
	_textUserAmount->setString("");
	auto panelInfo = RootNode->getChildByName<Layout*>("panelInfo");
	if (!panelInfo)
		return false;
	auto lP = panelInfo->getChildByName<Text*>("lP");
	if (!lP)
		return false;
	lP->setString(Language::getStringByKey("Player"));
	auto lB = panelInfo->getChildByName<Text*>("lB");
	if (!lB)
		return false;
	lB->setString(Language::getStringByKey("Banker"));
	auto lT = panelInfo->getChildByName<Text*>("lT");
	if (!lT)
		return false;
	lT->setString(Language::getStringByKey("Tie"));
	auto lPP = panelInfo->getChildByName<Text*>("lPP");
	if (!lPP)
		return false;
	lPP->setString(Language::getStringByKey("PlayerPair"));
	auto lBP = panelInfo->getChildByName<Text*>("lBP");
	if (!lBP)
		return false;
	lBP->setString(Language::getStringByKey("BankerPair"));
	auto lC = panelInfo->getChildByName<Text*>("lC");
	if (!lC)
		return false;
	lC->setString(Language::getStringByKey("Count"));
	auto lRoomId = panelInfo->getChildByName<Text*>("lRoomId");
	if (!lRoomId)
		return false;
	lRoomId->setString(Language::getStringByKey("PrivateRoom"));
	
	
	if (!(lbP = panelInfo->getChildByName<Text*>("lbP")))
		return false;
	if (!(lbB = panelInfo->getChildByName<Text*>("lbB")))
		return false;
	if (!(lbT = panelInfo->getChildByName<Text*>("lbT")))
		return false;
	if (!(lbPP = panelInfo->getChildByName<Text*>("lbPP")))
		return false;
	if (!(lbBP = panelInfo->getChildByName<Text*>("lbBP")))
		return false;
	if (!(lbC = panelInfo->getChildByName<Text*>("lbC")))
		return false;
	lbRoomId = panelInfo->getChildByName<Text*>("lbRoomId");
	if (!lbRoomId)
		return false;
	lbRoomId->setVisible(false);//私人房间，房间号隐藏了
	_bjlLzSm = new BjlLzSm();
	if (!_bjlLzSm)
		return false;
	if (!_bjlLzSm->init(panelInfo->getChildByName("Node_Lzsm")))
		return false;
	_layerBjlLz = LayerBjlLz::create();
	if (!_layerBjlLz)
		return false;
	_layerBjlLz->setVisible(false);
	_bjlLzSm->_scrollViewLzSm->addClickEventListener(CC_CALLBACK_1(SceneBjlRoomP::ShowLz, this));

	auto btnHide = RootNode->getChildByName<Button*>("btnHide");
	if (!btnHide)
		return false;
	btnHide->setPressedActionEnabled(true);
	NodeGameHideControl = RootNode->getChildByName<Node*>("NodeGameHideControl");
	if (!NodeGameHideControl)
		return false;
	Layout *GameHideBar = NodeGameHideControl->getChildByName<Layout*>("GameHideBar");

	isBarHide = true;

	Vec2 vec = NodeGameHideControl->getPosition();
	btnHide->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		btnHide->setVisible(!btnHide->isVisible());
		if (isBarHide) {
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(950, 0)),
				CallFunc::create([=](){
				isBarHide = false;
			}),
				nullptr));
		}
	});
	
	auto touchListener = EventListenerTouchOneByOne::create();
	//touchListener->setSwallowTouches(true);
	touchListener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!GameHideBar->getBoundingBox().containsPoint(touch->getLocation()))

			if (!isBarHide) {
			isBarHide = true;
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(-950, 0)),
				CallFunc::create([=](){
				btnHide->setVisible(!btnHide->isVisible());
			}),
				nullptr));
			}

		return true;
	};
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(touchListener, GameHideBar);
	// init quit button
	auto btQuit = GameHideBar->getChildByName<Button*>("btQuit");
	if (!btQuit)
		return false;
	btQuit->setPressedActionEnabled(true);
	btQuit->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);  Director::getInstance()->popScene(); });
	// init settings button
	auto btSettings = GameHideBar->getChildByName<Button*>("btSettings");
	if (!btSettings)
		return false;
	btSettings->setPressedActionEnabled(true);
	btSettings->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON); LayerSetting *layer = LayerSetting::create(); this->addChild(layer, 1); });
	// init check button
	auto btCheck = GameHideBar->getChildByName<Button*>("btCheck");
	if (!btCheck)
		return false;
	btCheck->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON); LayerBjlCalculator *layer = LayerBjlCalculator::create(); layer->SetData(_roomData); this->addChild(layer, 1); });
	btCheck->setPressedActionEnabled(true);
	auto btHelp = GameHideBar->getChildByName<Button*>("btHelp");
	if (!btHelp)
		return false;
	btHelp->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON); LayerBjlHelp *layer = LayerBjlHelp::create();  this->addChild(layer, 1); });
	btHelp->setPressedActionEnabled(true);
	//报表
	Button *btForm;
	if (!(btForm = GameHideBar->getChildByName<Button*>("btReport")))
		return false;
	btForm->setPressedActionEnabled(true);
	btForm->addClickEventListener([=](Ref *ref)
	{
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		SceneReports* scene = SceneReports::create();
		scene->setName("SceneReports");
		this->addChild(scene, 2);
	});

	//搓牌
	if (!(_checkFP = GameHideBar->getChildByName<CheckBox*>("cbFP")))
		return false;

	layerCm = Layer::create();
	RootNode->addChild(layerCm, 1);		//添加到rootnode节点，以免盖住投注金额显示
	layerCard = Layer::create();
	this->addChild(layerCard, 1);
	this->addChild(_layerBjlLz, 1);		//路单在上，不修改zorder是因为在android下无效

	//单人房间，不显示聊天窗口。
	auto panelChat = RootNode->getChildByName<Layout*>("panelChat");
	if (!panelChat)
		return false;
	panelChat->setVisible(false);
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoomP::UpdateBalance), "UpdateBalance", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoomP::OnBetCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::SG_BACCARAT_BET_GAME), NULL);

	UpdateBalance(this);
    //初始化时间控件
    lbTime=RootNode->getChildByName<Text*>("lbTime");
    if(!lbTime)
        return false;
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoomP::onCurrentIntervalTaskFinish), "CurrentIntervalTaskFinish", nullptr);

	bool resetSucc = TaskControl::GetInstance()->resetIntervalStart();//重置在线游戏

	intervalTaskLayout = RootNode->getChildByName<Layout*>("interval_task");
	awardTime = intervalTaskLayout->getChildByName<Text*>("award_time");
	btnAward = intervalTaskLayout->getChildByName<Button*>("btn_award");

	btnAward->addClickEventListener([=](Ref* ref){
		btnAward->setEnabled(false);
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneInervalTask");
	});
	btnAward->setPressedActionEnabled(true);
	btnAward->setBright(false);
	btnAward->setEnabled(false);

	intervalTaskWaiting = resetSucc;
	intervalTaskLayout->setVisible(resetSucc);

    this->updateCurrentTime(0);
    schedule(schedule_selector(SceneBjlRoomP::updateCurrentTime), 1.0f);
	
	return true;
}

void SceneBjlRoomP::onEnter()
{
	Scene::onEnter();
	SoundControl::PlayMusic(BGM::BGM_BACCARAT);
}

void SceneBjlRoomP::onCurrentIntervalTaskFinish(Ref *pSender)
{
	btnAward->stopAllActions();
	btnAward->setBright(false);
	btnAward->setEnabled(false);
	intervalTaskLayout->setVisible(false);

	//播放 金币飞的动画
	String* msg = (String*)pSender;

	Array *array = msg->componentsSeparatedByString("_");
	String* num = (String*)array->objectAtIndex(1);

	if (!num || array->count() < 2 || num->intValue() == 1)//
	{
		return;
	}
	intervalTaskWaiting = true;
	intervalTaskLayout->setVisible(true);

	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	lotteryIcon->setPosition(intervalTaskLayout->getPosition());

	int add = num->intValue();

	lotteryIcon->setScale(0.6);
	lotteryIcon->setCascadeOpacityEnabled(true);

	RootNode->addChild(lotteryIcon);
	lotteryIcon->runAction(Sequence::create(
		Spawn::create(MoveTo::create(1.0f, lbMyChip->getPosition()), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();
		Tips(StringUtils::format(Language::getStringByKey("AddChipsTip"), add));
	})
		, nullptr));
}


SceneBjlRoomP::~SceneBjlRoomP()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_roomData)
		delete _roomData;
	if (_betResultData)
		delete _betResultData;
}
void SceneBjlRoomP::SetRoomData(BjlRoomData *data)
{
	if (data == nullptr)
		return;
	if (_roomData)
		delete _roomData;
	_roomData = data;
	_cardDatas = "";
	//lbRoomId->setString(_roomData->uuid);
	//初始化路单
	for (int i = 0; i < _roomData->history_rounds.size(); i++)
	{
		_cardDatas.append(_roomData->history_rounds[i].data);
		if (i > 0)
		{
			if (!_pmInfos.SetCards(_roomData->history_rounds[i].data.c_str()))
			{
				CCLOG("SceneBjlRoomP::SetRoomData\tset cards error");
				exit(-1);
			}
			int lzStatus = 0;
			if (_pmInfos.Player.point > _pmInfos.Banker.point)
			{
				//player win
				lzStatus |= BJLBETTYPE::BJLBETP;
			}
			else if (_pmInfos.Player.point < _pmInfos.Banker.point)
			{
				//banker win
				lzStatus |= BJLBETTYPE::BJLBETB;
			}
			else
			{
				//tie
				lzStatus |= BJLBETTYPE::BJLBETT;
			}
			if (_pmInfos.Player.cards[0][1] == _pmInfos.Player.cards[1][1])
			{
				//player pair
				lzStatus |= BJLBETTYPE::BJLBETPP;
			}
			if (_pmInfos.Banker.cards[0][1] == _pmInfos.Banker.cards[1][1])
			{
				//banker pair
				lzStatus |= BJLBETTYPE::BJLBETBP;
			}
			//更新缩略路图
			_bjlLzSm->addLz(lzStatus);
			_layerBjlLz->addLz(lzStatus);
			_lzDatas.push_back(lzStatus);		//添加到历史数据中
		}
	}
	//更新局数统计信息
	lbB->setString(toString(_layerBjlLz->_countB));
	lbP->setString(toString(_layerBjlLz->_countP));
	lbT->setString(toString(_layerBjlLz->_countT));
	lbBP->setString(toString(_layerBjlLz->_countBP));
	lbPP->setString(toString(_layerBjlLz->_countPP));
	lbC->setString(toString(_layerBjlLz->_count));
}
void SceneBjlRoomP::UpdateBalance(Ref *ref)
{
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BalanceData *balance = BillControl::GetInstance()->GetBalanceData(0);
		if (!balance)
			return;
		_userAmount = balance->balance;
        
		_textUserAmount->setString(Comm::GetFloatShortStringFromInt64(_userAmount));
	});
}
void SceneBjlRoomP::ShowLz(Ref* ref)
{
	_layerBjlLz->setVisible(true);
}
void SceneBjlRoomP::onButtonBetClicked(Ref* ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	EnableButton(_btBet, false);
	EnableButton(_btCancel, false);
	/*
	_statusGame = STATUSBJLGAME::FPSTART;
	char str[13];
	memset(str, 0, 13);
	for (int i = 0; i < 6; i++)
	{
		int n = cocos2d::random(1, 13);
		if (n < 10)
			sprintf(str + i * 2, "H%d", n);
		else if (n == 10)
			sprintf(str + i * 2, "HT");
		else if (n == 11)
			sprintf(str + i * 2, "HJ");
		else if (n == 12)
			sprintf(str + i * 2, "HQ");
		else if (n == 13)
			sprintf(str + i * 2, "HK");
	}
	funcFP(str);		//funcFP("H7H3H7H7H7H7");*/
	int b, p, t, b_p, p_p;
	if ((b = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETB)) < 0	||
		(p = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETP)) < 0 ||
		(t = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETT)) < 0 ||
		(b_p = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETBP)) < 0 ||
		(p_p = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETPP)) < 0)
	{
		return;
	}
	if (this->_userAmount < b + p + t + b_p + p_p) {
		return;
	}


		
	Api::Game::sg_baccarat_bet_game(_roomData->uuid.c_str(), _roomData->inning_num, 0, b, p, t, b_p, p_p);
}
//投注回调函数
void SceneBjlRoomP::OnBetCallBack(Ref *pSender)
{  

	

	//get user info结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{

			//成功
			/*"bonus":	0,
		"next_game_num":	2,
		"secret":	"7ac384bd6e955467375aa7414df1dec790502add26b8e482ddcd751f5efd5b11",
		"seed":	"92e239a1fddf11e659b305660c604efaf86bbe11540a8651a1d2892e01d62985",
		"next_secret":	"8dc8050a8e0fe5d96984dd73e0930824e953077917879c33d509d3c01c5824fd",
		"is_end":	false,
		"balance":	{
			"outpour":	0,
			"update_time":	1436569818,
			"user_id":	87,
			"commission":	0,
			"bonus":	0,
			"tax":	0,
			"inpour":	0,
			"freeze":	0,
			"currency_type":	0,
			"create_time":	1436317815,
			"deposit":	10000,
			"withdraw":	0,
			"rebate":	0,
			"balance":	9700,
			"bet":	300
		},
		"data":	"HKCQC6C3HJ"*/


			

			if (_betResultData)
				delete _betResultData;
			_betResultData = GameControlBjl::GetInstance()->InitBetResultData(msg->data);
			if (_betResultData)
			{


				_statusGame = STATUSBJLGAME::FPEND;
				//layerCm->removeAllChildren();	//Çå¿Õ³ïÂë²ã
				//layerCard->removeAllChildren();	//Çå¿Õ·¢ÅÆ²ã
				

				//开始翻牌
				funcFP(_betResultData->data.c_str());
                //_spriteBetAreas.clearBet();		//ÖØÖÃ¼ÆËãÇø
                //checkAndSetAllAreaBetStatus();
			}
			else
				CCLOG("[%s]:%s\t%s", "SceneBjlRoomP::OnBetCallBack", "init bet data error.", msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoomP::OnBetCallBack", "get bet data error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoomP::onButtonCancelClicked(Ref* ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	_spriteBetAreas.clearBet();		//清空投注区管理类数据
	checkAndSetAllAreaBetStatus();// 检测设置状态
	layerCm->removeAllChildren();	//清空桌面筹码
	_btBet->setTitleText(Language::getStringByKey("SkipCard"));
	EnableButton(_btBet, true);
	EnableButton(_btCancel, false);
}


CMINFO* SceneBjlRoomP::getCurrentCMInfo()
{
	Sprite *sp = _spriteCms.getCurrentCmSprite();
	if (!sp) return nullptr;
	CMINFO *cmInfo = (CMINFO*)sp->getUserData();
	return cmInfo;
}

bool SceneBjlRoomP::isAreaCanBet(SpriteBetArea *betArea, CMINFO *cmInfo)
{
	//检测是否走出限额
	int64_t maxBet = _maxBet; // 跟 SceneBjlRoom.cpp不一样

	if (betArea->type&(BJLBETTYPE::BJLBETPP | BJLBETTYPE::BJLBETBP | BJLBETTYPE::BJLBETT)) {
		//对子，和，限额为正常限额的1/10
		maxBet /= 10;
	}

	if (cmInfo->amount > this->_userAmount) { // 注意 ，跟之前代码对比，添加了验证。
		return false;
	}

	//最小投注限额为最大限额的1/100
	int64_t minBet = maxBet / 100;
	if ((betArea->amount + cmInfo->amount) > maxBet || cmInfo->amount < minBet) {
		//超出投注限额
		return false;
	}
	return true;
}

void SceneBjlRoomP::checkAndSetAreaBetStatus(SpriteBetArea *betArea, CMINFO *cmInfo)
{
	if (_statusGame != STATUSBJLGAME::BETSTART || cmInfo == nullptr){
		betArea->setSelectStatus(0);
		return;
	}
	if (isAreaCanBet(betArea, cmInfo))
	{
		betArea->setSelectStatus(1);
	}
	else
	{
		betArea->setSelectStatus(2);
	}
}


void SceneBjlRoomP::checkAndSetAllAreaBetStatus()
{

	CMINFO *cmInfo = getCurrentCMInfo();// 空情况已处理
	Vector<SpriteBetArea*> betAreas = _spriteBetAreas.betAreas;
	for (SpriteBetArea* betArea : betAreas)
	{
		checkAndSetAreaBetStatus(betArea, cmInfo);
	}

}

//选中筹码
bool SceneBjlRoomP::onCmClick(Touch* touch, Event* event)
{
	auto target = static_cast<Sprite*>(event->getCurrentTarget());
	if (target == _spriteCms.getCurrentCmSprite())
		return false;
	Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
	Size s = target->getContentSize();
	Rect rect = Rect(0, 0, s.width, s.height);
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BACCARATE_SELECT);
	if (rect.containsPoint(locationInNode))
	{
		//³ïÂëµã»÷ÊÂ¼þ
		//log("cm place began... x = %f, y = %f", locationInNode.x, locationInNode.y);
		if (_spriteCms.getCurrentCmSprite())
			_spriteCms._curCm->SetSelected(false);
		_spriteCms.setCurrentCm(target);
		_spriteCms._curCm->SetSelected(true);


		// 处理
		if (_statusGame == STATUSBJLGAME::BETSTART)
		{
			checkAndSetAllAreaBetStatus();

		}

		return true;
	}
	return false;
}


//Í¶·Å³ïÂë

bool SceneBjlRoomP::onCmBet(Touch* touch, Event* event)
{
	if (_statusGame != STATUSBJLGAME::BETSTART)
		return false;
	//SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BET);
	auto target = static_cast<Sprite*>(event->getCurrentTarget());
	Vec2 point = touch->getLocation();
	Vec2 locationInNode = target->convertToNodeSpace(point);
	Size s = target->getContentSize();
	Rect rect = Rect(0, 0, s.width, s.height);
	//if (rect.containsPoint(locationInNode))
	if (_irregularTest.HitTest(target, point))
	{
		
		//log("cm place began... x = %f, y = %f", locationInNode.x, locationInNode.y);
		Sprite *sp = _spriteCms.getCurrentCmSprite();
		if (!sp)
			return false;		//没有选中任何筹码
		CMINFO *cmInfo = (CMINFO*)sp->getUserData();
		if (!cmInfo)
			return false;
		SpriteBetArea *betArea = _spriteBetAreas.getBetArea(target);
		if (!betArea)
			return false;
		//检测是否走出限额
		//int64_t maxBet = _maxBet;
		//if (betArea->type&(BJLBETTYPE::BJLBETPP | BJLBETTYPE::BJLBETBP | BJLBETTYPE::BJLBETT))
		//	maxBet /= 10;			//对子，和，限额为正常限额的1/10
		//int64_t minBet = maxBet / 100;		//最小投注限额为最大限额的1/100
		//if ((betArea->amount + cmInfo->amount) > maxBet
		//	|| cmInfo->amount < minBet)
		//	return false;		//超出投注限额
		if (!isAreaCanBet(betArea, cmInfo))
		{
			return false;
		}
		//添加子筹码
		SpriteCmSub *scs = SpriteCmSub::create(cmInfo->imgSmall, _spriteCms._curCm->amount, sp->getScale(), sp->getPosition(), _ptCmLose);
		layerCm->addChild(scs);
		scs->doBet(point);
		//add to betareas control class
		_spriteBetAreas.addBet(target, scs);
		checkAndSetAllAreaBetStatus();
		//set confirm,cancel button status
		_btBet->setTitleText(Language::getStringByKey("Confirm"));
		EnableButton(_btBet, true);
		EnableButton(_btCancel, true);

		//验证完成任务 首次创建单人房间并且完成一局游戏
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(6)));

		//-------------------------------for lz test
		//_tempBetType |= _spriteBetAreas.getBetArea(target)->type;
		return true;
	}
	return false;
}

void SceneBjlRoomP::funcTimeLeft(float d)
{
	static char str[10];
	switch (_statusGame)
	{
	case BETSTART:
		break;
	case BETEND:
		break;
	case FPSTART:
		break;
	case FPEND:
		if (TimeLeft-- <= 0)
		{
			//set confirm,cancel button status
			_statusGame = STATUSBJLGAME::BETSTART;
			TimeLeft = TimeLeftMax;
			layerCm->removeAllChildren();	//Çå¿Õ³ïÂë²ã
			layerCard->removeAllChildren();	//Çå¿Õ·¢ÅÆ²ã
			_spriteBetAreas.clearBet();		//ÖØÖÃ¼ÆËãÇø
			checkAndSetAllAreaBetStatus();
			_layerResult->setVisible(false);
			panelCards->setVisible(false);
			TextPtP->setText("");
			TextPtB->setText("");
			_btBet->setTitleText(Language::getStringByKey("SkipCard"));
			EnableButton(_btBet, true);
		}
		break;
	default:
		break;
	}
}

//翻牌结束
void SceneBjlRoomP::actFpDone()
{
	TextPtP->setString(StringUtils::format("%d", _pmInfos.Player.point));
	TextPtB->setString(StringUtils::format("%d", _pmInfos.Banker.point));
	SoundControl::PlayPoint(_pmInfos.Player.point, _pmInfos.Banker.point);
	doAward();
	//派奖完成,更新数据
	if (_betResultData->is_end)
	{
		//整局牌结束，开始新的牌局
	}
	else
	{
		//添加到历史数据
		BjlRoomDataRound round;
		round.num = _roomData->inning_num;
		round.secret = _betResultData->secret;
		round.seed = _betResultData->seed;
		round.data = _betResultData->data;
		_roomData->history_rounds.push_back(round);
		//设置新一局牌的信息
		_roomData->inning_num = _betResultData->next_game_num;
		_roomData->inning_secret = _betResultData->next_secret;
		
	}
	//在投注返回的数据中，已经包含了用户最新余额，此处只需要手动更新一次即可
	//将数据由object改为数组，以便InitBalanceData函数调用
	if (BillControl::GetInstance()->InitBalanceData(StringUtils::format("[%s]", _betResultData->balance.c_str())))
		PKNotificationCenter::getInstance()->postNotification("UpdateBalance");

	if (_btBet->getTitleText() != Language::getStringByKey("SkipCard"))
	{
		//正常牌局
		//_layerResult->setVisible(true);
		TimeLeft = TimeForShowResult;		//time to show result
	}
	else
	{
		//飞牌
		TimeLeft = TimeForShowResult / 2;		//time to show result
	}
	_statusGame = STATUSBJLGAME::FPEND;
}
//派奖
void SceneBjlRoomP::doAward()
{
	//显示结果界面
	_layerResult->resetAmount();
	float amount;
	int lzStatus = 0;
	if (_pmInfos.Player.point > _pmInfos.Banker.point)
	{
		//player win
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETP);
		if (amount < 0)
			return;
		_layerResult->addAmount(amount);
		_spriteBetAreas.setWinArea(BJLBETTYPE::BJLBETP);
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETB);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETB);
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETT);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETT);
		lzStatus |= BJLBETTYPE::BJLBETP;
	}
	else if (_pmInfos.Player.point < _pmInfos.Banker.point)
	{
		//banker win
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETB);
		if (amount < 0)
			return;
		_layerResult->addAmount(amount*0.95f);
		_spriteBetAreas.setWinArea(BJLBETTYPE::BJLBETB);
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETP);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETP);
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETT);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETT);
		lzStatus |= BJLBETTYPE::BJLBETB;
	}
	else
	{
		//tie
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETT);
		if (amount < 0)
			return;
		_layerResult->addAmount(amount*8.0f);
		_spriteBetAreas.setWinArea(BJLBETTYPE::BJLBETT);
		_spriteBetAreas.setTieArea(BJLBETTYPE::BJLBETP);
		_spriteBetAreas.setTieArea(BJLBETTYPE::BJLBETB);
		lzStatus |= BJLBETTYPE::BJLBETT;
	}
	if (_pmInfos.Player.cards[0][1] == _pmInfos.Player.cards[1][1])
	{
		//player pair
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETPP);
		if (amount < 0)
			return;
		_layerResult->addAmount(amount*11.0f);
		_spriteBetAreas.setWinArea(BJLBETTYPE::BJLBETPP);
		lzStatus |= BJLBETTYPE::BJLBETPP;
	}
	else
	{
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETPP);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETPP);
	}
	if (_pmInfos.Banker.cards[0][1] == _pmInfos.Banker.cards[1][1])
	{
		//banker pair
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETBP);
		if (amount < 0)
			return;
		_layerResult->addAmount(amount*11.0f);
		_spriteBetAreas.setWinArea(BJLBETTYPE::BJLBETBP);
		lzStatus |= BJLBETTYPE::BJLBETBP;
	}
	else
	{
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETBP);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETBP);
	}
	_userAmount += _layerResult->getAmount();
	if (_betResultData->bonus != _layerResult->_amountWin)
		CCLOG("SceneBjlRoomP::doAward\terror:client bonus != server bonus");
	_textUserAmount->setText(toString(_userAmount));
	//更新缩略路图
	_bjlLzSm->addLz(lzStatus);
	_lzDatas.push_back(lzStatus);		//添加到历史数据中	
	_layerBjlLz->addLz(lzStatus);
	//更新局数统计信息
	lbB->setString(toString(_layerBjlLz->_countB));
	lbP->setString(toString(_layerBjlLz->_countP));
	lbT->setString(toString(_layerBjlLz->_countT));
	lbBP->setString(toString(_layerBjlLz->_countBP));
	lbPP->setString(toString(_layerBjlLz->_countPP));
	lbC->setString(toString(_layerBjlLz->_count));
	//播放胜利音效
	if (_layerResult->getAmount() > 0)
	{
		//Settings::GetInstance()->SoundVolume
		//AudioEngine::play2d(cocos2d::random() % 2 ? "sound/win0.mp3" : "sound/win1.mp3", false, 1.0f);
		SoundControl::PlayEffect(cocos2d::random() % 2 ? "sound/win0.mp3" : "sound/win1.mp3");
	}
	else if (_layerResult->getAmount() < 0)
		SoundControl::PlayEffect("sound/lose0.mp3");
    
    Api::Bill::GET_BALANCE();
}
void SceneBjlRoomP::actFp(Node* node, BJLFPINDEX n)
{
	char chars[20];
	Sprite* sp;
	CallFuncN *func;
	SoundControl::PlayPokerEffect();
	switch (n)
	{
	case BJLP1:
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Player.cards[0]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLB1));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP, _ptCardP), ScaleTo::create(_timeFP, _scaleCard1), nullptr), func, nullptr));
		break;
	case BJLP2:
		//show b1
		sprintf(chars, "%d", _pmInfos.Banker.points[0]);
		TextPtB->setText(chars);
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Player.cards[1]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLB2));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP, Vec2(_ptCardP.x + _widthCard, _ptCardP.y)), ScaleTo::create(_timeFP, _scaleCard1), nullptr), func, nullptr));
		break;
	case BJLP3:
		//show b2
		sprintf(chars, "%d", (_pmInfos.Banker.points[0] + _pmInfos.Banker.points[1]) % 10);
		TextPtB->setText(chars);
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Player.cards[2]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		if (_pmInfos.Banker.count == 3)
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLB3));
		else
			func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoomP::actFpDone, this));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP, Vec2(_ptCardP.x + _widthCard * 2, _ptCardP.y)), ScaleTo::create(_timeFP, _scaleCard1), nullptr), func, nullptr));
		break;
	case BJLB1:
		//show p1
		sprintf(chars, "%d", _pmInfos.Player.points[0]);
		TextPtP->setText(chars);
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Banker.cards[0]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLP2));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP / 2, _ptCardB), ScaleTo::create(_timeFP / 2, _scaleCard1), nullptr), func, nullptr));
		break;
	case BJLB2:
		//show p2
		sprintf(chars, "%d", (_pmInfos.Player.points[0] + _pmInfos.Player.points[1]) % 10);
		TextPtP->setText(chars);
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Banker.cards[1]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		if (_pmInfos.Player.count == 3)
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLP3));
		else if (_pmInfos.Banker.count == 3)
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoomP::actFp, this, BJLFPINDEX::BJLB3));
		else
			func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoomP::actFpDone, this));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP / 2, Vec2(_ptCardB.x + _widthCard, _ptCardB.y)), ScaleTo::create(_timeFP / 2, _scaleCard1), nullptr), func, nullptr));
		break;
	case BJLB3:
		//show p
		TextPtP->setText(toString(_pmInfos.Player.point));
		//show b2
		sprintf(chars, "%d", (_pmInfos.Banker.points[0] + _pmInfos.Banker.points[1]) % 10);
		TextPtB->setText(chars);
		sprintf(chars, "%d", (_pmInfos.Banker.points[0] + _pmInfos.Banker.points[1]) % 10);
		TextPtB->setText(chars);
		sprintf(chars, "SmallCard%s\.png", _pmInfos.Banker.cards[2]);
		sp = Sprite::createWithSpriteFrameName(chars);
		sp->setScale(_scaleCard0);
		sp->setAnchorPoint(Vec2(0, 0));
		sp->setPosition(_ptCardInit);
		layerCard->addChild(sp);
		func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoomP::actFpDone, this));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP / 2, Vec2(_ptCardB.x + _widthCard * 2, _ptCardB.y)), ScaleTo::create(_timeFP / 2, _scaleCard1), nullptr), func, nullptr));
		break;
	default:
		break;
	}
}
//·¢ÅÆ
void SceneBjlRoomP::funcFP(const char* cards)
{
	//³õÊ¼»¯ÅÆÃæÐÅÏ¢
	if (!_pmInfos.SetCards(cards))
		return;
	_cardDatas.append(cards);
	panelCards->setVisible(true);
	TextPtP->setString("");
	TextPtB->setString("");
	//人工or自动翻牌
	if (_btBet->getTitleText() == Language::getStringByKey("SkipCard") || !_checkFP->isSelected())
		actFp(this, BJLFPINDEX::BJLP1);		//自动
	else
	{
		//人工		
		//init bjlfp layer
		_layerBjlFpS = LayerBjlFpS::create();
		if (!_layerBjlFpS)
			return;
		float amountB, amountP;
		amountP = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETP);
		amountB = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETB);
		if (amountP < 0 || amountB < 0)
			return;		//获取投注额失败
		_layerBjlFpS->setCards(&_pmInfos, amountP >= amountB);
		addChild(_layerBjlFpS, 1);
		_layerBjlFpS->setOnExitCallback([=](){
			_layerBjlFpS = NULL;
			//结束人工翻牌,返回
			TextPtP->setText(toString(_pmInfos.Player.point));
			TextPtB->setText(toString(_pmInfos.Banker.point));
			Sprite *sp;
			char name[20];
			for (int i = 0; i < _pmInfos.Player.count; i++)
			{
				sprintf(name, "SmallCard%s.png", _pmInfos.Player.cards[i]);
				sp = Sprite::createWithSpriteFrameName(name);
				sp->setScale(_scaleCard1);
				sp->setAnchorPoint(Vec2(0, 0));
				sp->setPosition(_ptCardP.x + i*_widthCard, _ptCardP.y);
				layerCard->addChild(sp);
			}
			for (int i = 0; i < _pmInfos.Banker.count; i++)
			{
				sprintf(name, "SmallCard%s.png", _pmInfos.Banker.cards[i]);
				sp = Sprite::createWithSpriteFrameName(name);
				sp->setScale(_scaleCard1);
				sp->setAnchorPoint(Vec2(0, 0));
				sp->setPosition(_ptCardB.x + i*_widthCard, _ptCardB.y);
				layerCard->addChild(sp);
			}
			actFpDone();
		});
	}
	//Sprite* b1 = Sprite::createWithSpriteFrameName(_pmInfos.Banker.cards[0]);
	//Sprite* b2 = Sprite::createWithSpriteFrameName(_pmInfos.Banker.cards[1]);
}
// ¼üÎ»ÏìÓ¦º¯ÊýÔ­ÐÍ
void SceneBjlRoomP::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	log("Key with keycode %d released", keyCode);
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{

		SceneReports* report = this->getChildByName<SceneReports*>("SceneReports");
		if (report){
			report->removeFromParent();
		}
		else{
			if (_layerBjlLz->isVisible())
				_layerBjlLz->setVisible(false);
			else if (_layerBjlFpS)
				_layerBjlFpS->removeFromParent();
			else
				Director::getInstance()->popScene();
		}
		
	}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}
void SceneBjlRoomP::addRoomLeaveEvent(ccFuncEventCustom callback)
{
	_eventDispatcher->addCustomEventListener("roomleave", callback);
}

void SceneBjlRoomP::updateCurrentTime(float dt)
{
	unsigned long long timestamp = time(NULL);
	struct tm *ptm = localtime((time_t*)&timestamp);
	char tmp[100] = { 0 };
	memset(tmp, 0x0, 100);
	strftime(tmp, sizeof(tmp), "%H:%M:%S", ptm);
	lbTime->setString(tmp);
	if (timestamp % 600 == 0) //每隔10分钟
	{
		Api::Bill::GET_BALANCE();
	}
	if (intervalTaskWaiting)
	{
		time_t lat = TaskControl::GetInstance()->getIntervalAwardLeftTime();
		if (lat>0)
		{
			struct tm *ptm_lat = localtime(&lat);
			char tmp_lat[100] = { 0 };
			memset(tmp_lat, 0x0, 100);
			strftime(tmp_lat, sizeof(tmp_lat), "%M:%S", ptm_lat);
			awardTime->setString(tmp_lat);
		}
		else
		{
			intervalTaskWaiting = false;
			awardTime->setString("");
			btnAward->setEnabled(true);
			btnAward->setBright(true);
			CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(0.6), FadeOut::create(0.6), nullptr));
			btnAward->runAction(action);
		}
	}
}