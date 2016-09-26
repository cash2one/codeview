#include "SceneBjlRoom.h"
#include "cocostudio/CocoStudio.h"
#include "ui/UIListView.h"
#include "SpriteCmSub.h"
#include "LayerSetting.h"
#include "Settings.h"
#include "ApiGame.h"
#include "ApiBill.h"
#include "BillControl.h"
#include "LayerBjlCalculator.h"
#include "LayerLoading.h"
#include "cmd.h"
#include "SoundControl.h"
#include "TimeControl.h"
#include "SceneReports.h"
#include "PKNotificationCenter.h"
#include "ui/UITextField.h"
#include "LayerBjlHelp.h"
#include "TaskControl.h"
#include "LayerMall.h"
#include "SoundControl.h"
#include "LayerGuideMask.h"
#include "LayerFriend.h"
#include "LayerMail.h"

#define MAXCHATRECORD	100		//最大聊天记录数
using namespace std;
static int _tempBetType = 0;			//for lz test

static int _currentRoomType = 0;
SceneBjlRoom* SceneBjlRoom::create(RoomInfoBjl *roomInfo)
{
	SceneBjlRoom *pRet = new(std::nothrow) SceneBjlRoom();
	if (pRet && pRet->init(roomInfo))
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
bool SceneBjlRoom::init(RoomInfoBjl *roomInfo)
{
	if (!roomInfo || !roomInfo->type)
		return false;
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}



	//this->setKeypadEnabled(true);
	_fpEnable = false;
	_roomInfo = roomInfo;
	_roomData = nullptr;
	_userAmount = BillControl::GetInstance()->GetBalanceData(0)->balance;
	_seatId = 0;		//进入房间时无座
	_myBetInfoNoSeat = { 0 };
	_userData = UserControl::GetInstance()->GetUserData();
	if (!_userData)
		return false;
	auto listener = EventListenerKeyboard::create();
	listener->onKeyReleased = CC_CALLBACK_2(SceneBjlRoom::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	TimeForShowResult = 5;
	TimeLeftMax = 99;
	_statusGame = STATUSBJLGAME::BETEND;
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	CCLOG("Visible size:%.2f,%.2f", visibleSize.width, visibleSize.height);
	/////////////////////////////
	// 3. add your codes below...

	int curTime = TimeControl::GetInstance()->getCurrentUsecTime();

	RootNode = CSLoader::createNode("bjl/SceneBjlRoom.csb");
	
	int timeConsuming = TimeControl::GetInstance()->getCurrentUsecTime() - curTime;


	if (!RootNode)
		return false;

	TextTimeLeft = RootNode->getChildByName<TextBMFont*>("Text_TimeLeft");
	if (!TextTimeLeft)
		return false;
	TextTimeLeft->setString("");
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

	panelCmArea = RootNode->getChildByName<Layout*>("panelCmArea");
	if (!panelCmArea)
		return false;
	panelCmArea->setTouchEnabled(false);
	lbMyChip = panelCmArea->getChildByName<Text*>("lbMyChip");
	if (!lbMyChip)
		return false;
	lbMyChip->setString(Language::getStringByKey("MyChip"));

	auto btnBuy = panelCmArea->getChildByName<Button*>("btnBuy");

	btnBuy->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerMall* layerMall = LayerMall::create();
		this->addChild(layerMall, 1);
	});

	// 加载资源
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistChips.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistCard.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistCardNP.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistSmallCard.plist");
	//计算需要显示的筹码
	_cmInfos.clear();
	auto listener1 = EventListenerTouchOneByOne::create();
	listener1->setSwallowTouches(true);
	listener1->onTouchBegan = CC_CALLBACK_2(SceneBjlRoom::onCmClick, this);
	int max_bet = roomInfo->type->max_bet / 1000;
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

	//初始化时间控件
	lbTime = RootNode->getChildByName<Text*>("lbTime");
	if (!lbTime)
		return false;

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
	schedule(schedule_selector(SceneBjlRoom::updateCurrentTime), 1.0f);



	//初始化桌面玩家控制类
	if (!(_tableControl = BjlTableControl::create(RootNode->getChildByName("panelPlayers"))))
		return false;
	//初始化有座玩家聊天控制类
	if (!(_sitChatControl = BjlSitChatControl::create(_tableControl->RootNode->getChildByName("nodeSitChat"))))
		return false;
	auto panelBetArea = RootNode->getChildByName<Layout*>("panelBetArea");
	if (!panelBetArea)
		return false;
	panelBetArea->setTouchEnabled(false);
	auto panelAmountInfos = RootNode->getChildByName<Layout*>("panelAmountInfos");
	if (!panelAmountInfos)
		return false;
	panelAmountInfos->setTouchEnabled(false);
	panelAmountInfos->setZOrder(2);
	auto listener2 = EventListenerTouchOneByOne::create();
	listener2->setSwallowTouches(true);
	listener2->onTouchBegan = CC_CALLBACK_2(SceneBjlRoom::onCmBet, this);
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
	//设置全部玩家投注控件
	if (!(lbAmountPAll = panelAmountInfos->getChildByName<Text*>("lbAmountPAll")))
		return false;
	if (!(lbAmountPPAll = panelAmountInfos->getChildByName<Text*>("lbAmountPPAll")))
		return false;
	if (!(lbAmountBAll = panelAmountInfos->getChildByName<Text*>("lbAmountBAll")))
		return false;
	if (!(lbAmountBPAll = panelAmountInfos->getChildByName<Text*>("lbAmountBPAll")))
		return false;
	if (!(lbAmountTAll = panelAmountInfos->getChildByName<Text*>("lbAmountTAll")))
		return false;

	if (!(imgBgAmountPAll = panelAmountInfos->getChildByName<Sprite*>("imgBgAmountPAll")))
		return false;
	if (!(imgBgAmountPPAll = panelAmountInfos->getChildByName<Sprite*>("imgBgAmountPPAll")))
		return false;
	if (!(imgBgAmountBAll = panelAmountInfos->getChildByName<Sprite*>("imgBgAmountBAll")))
		return false;
	if (!(imgBgAmountBPAll = panelAmountInfos->getChildByName<Sprite*>("imgBgAmountBPAll")))
		return false;
	if (!(imgBgAmountTAll = panelAmountInfos->getChildByName<Sprite*>("imgBgAmountTAll")))
		return false;


	setTotalBetInfo(nullptr);
	//隐蔽bet,cancel按钮
	panelCmArea->getChildByName("Button_Bet")->setVisible(false);
	panelCmArea->getChildByName("Button_Cancel")->setVisible(false);

	this->addChild(RootNode);
	schedule(schedule_selector(SceneBjlRoom::funcTimeLeft), 1.0f);
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
	_textUserAmount->setString(Comm::GetFloatShortStringFromInt64(_userAmount));
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
	lRoomId->setString(Language::getStringByKey("RoomId"));


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
	lbRoomId->setString(toString(_roomInfo->id));
	_bjlLzSm = new BjlLzSm();
	if (!_bjlLzSm)
		return false;
	if (!_bjlLzSm->init(panelInfo->getChildByName("Node_Lzsm")))
		return false;
	_layerBjlLz = LayerBjlLz::create();
	if (!_layerBjlLz)
		return false;
	_layerBjlLz->setVisible(false);

	_bjlLzSm->_scrollViewLzSm->addClickEventListener(CC_CALLBACK_1(SceneBjlRoom::ShowLz, this));
	auto btnHide = RootNode->getChildByName<Button*>("btnHide");
	if (!btnHide)
		return false;

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
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(1230, 0)),
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
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(-1230, 0)),
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
	btSettings->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	LayerSetting * setting = this->getChildByName<LayerSetting*>("LayerSetting");
	if (!setting){
		LayerSetting * setting = LayerSetting::create(); //临时创建释放 avoid texture memory overflow
		setting->setName("LayerSetting");
		this->addChild(setting, 1);
		OpenWithAlert(setting);
	}

	});
	// init check button
	auto btCheck = GameHideBar->getChildByName<Button*>("btCheck");
	if (!btCheck)
		return false;
	btCheck->setPressedActionEnabled(true);
	btCheck->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	LayerBjlCalculator * calculator = this->getChildByName<LayerBjlCalculator*>("LayerBjlCalculator");
	if (!calculator){
		LayerBjlCalculator * calculator = LayerBjlCalculator::create(); //临时创建释放 avoid texture memory overflow
		calculator->setName("LayerBjlCalculator");
		calculator->SetData(_roomData);
		this->addChild(calculator, 1);
		OpenWithAlert(calculator);
	}

	});

	auto btHelp = GameHideBar->getChildByName<Button*>("btHelp");
	if (!btHelp)
		return false;
	btHelp->setPressedActionEnabled(true);
	btHelp->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	LayerBjlHelp * help = this->getChildByName<LayerBjlHelp*>("LayerBjlHelp");
	if (!help){
		LayerBjlHelp * help = LayerBjlHelp::create(); //临时创建释放 avoid texture memory overflow
		help->setName("LayerBjlHelp");
		this->addChild(help, 1);
		OpenWithAlert(help);
	}
	});

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
	Button* btnFriend;
	btnFriend = GameHideBar->getChildByName<Button*>("btnFriend");
	btnFriend->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerFriend * layerFriend = this->getChildByName<LayerFriend*>("LayerFriend");
		if (!layerFriend){
			layerFriend = LayerFriend::create(); //临时创建释放 avoid texture memory overflow
			layerFriend->setName("LayerFriend");
			this->addChild(layerFriend, 2);
		}
	});
	btnFriend->setPressedActionEnabled(true);
	Button* btnMail;
	btnMail = GameHideBar->getChildByName<Button*>("btnMail");
	btnMail->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerMail * mail = this->getChildByName<LayerMail*>("LayerMail");
		if (!mail){
			mail = LayerMail::create(); //临时创建释放 avoid texture memory overflow
			mail->setName("LayerMail");
			this->addChild(mail, 2);
		}
	});
	btnMail->setPressedActionEnabled(true);
	//搓牌
	if (!(_checkFP = GameHideBar->getChildByName<CheckBox*>("cbFP")))
		return false;

	//添加其他玩家投注显示层
	auto layerPlayerCm = Layer::create();
	RootNode->addChild(layerPlayerCm, 1);
	Rect rectCm = layerPlayerCm->getBoundingBox();
	//  获取玩家筹码位置数组
	vector<Vec2> ptPlayers;
	for (int i = 0; i < 9; i++)
	{
		if (i == 0)
		{
			//无座玩家,统一为右下角
			ptPlayers.push_back(Vec2(visibleSize.width, 0));
		}
		else
		{
			//有座玩家
			ptPlayers.push_back(_tableControl->_playerPanels[i]->RootNode->convertToWorldSpace(_tableControl->_playerPanels[i]->imgCm->getPosition()));
		}
	}
	//获取 投注区域数组,顺序不可错
	Vec2 ptTemp;
	vector<Rect> rectBetAreas;
	SpriteBetArea* betArea = _spriteBetAreas.getBetArea(BJLBETP);
	Rect rectArea = betArea->spriteArea->getBoundingBox();
	float scaleRect = 0.1f;
	rectArea.origin += Vec2(rectArea.size.width*scaleRect, rectArea.size.height*scaleRect);
	rectArea.size = rectArea.size - Size(rectArea.size.width*scaleRect * 2, rectArea.size.height*scaleRect * 2);
	rectBetAreas.push_back(rectArea);
	betArea = _spriteBetAreas.getBetArea(BJLBETPP);
	rectArea = betArea->spriteArea->getBoundingBox();
	rectArea.origin += Vec2(rectArea.size.width*scaleRect, rectArea.size.height*scaleRect);
	rectArea.size = rectArea.size - Size(rectArea.size.width*scaleRect * 2, rectArea.size.height*scaleRect * 2);
	rectBetAreas.push_back(rectArea);
	betArea = _spriteBetAreas.getBetArea(BJLBETB);
	rectArea = betArea->spriteArea->getBoundingBox();
	rectArea.origin += Vec2(rectArea.size.width*scaleRect, rectArea.size.height*scaleRect);
	rectArea.size = rectArea.size - Size(rectArea.size.width*scaleRect * 2, rectArea.size.height*scaleRect * 2);
	rectBetAreas.push_back(rectArea);
	betArea = _spriteBetAreas.getBetArea(BJLBETBP);
	rectArea = betArea->spriteArea->getBoundingBox();
	rectArea.origin += Vec2(rectArea.size.width*scaleRect, rectArea.size.height*scaleRect);
	rectArea.size = rectArea.size - Size(rectArea.size.width*scaleRect * 2, rectArea.size.height*scaleRect * 2);
	rectBetAreas.push_back(rectArea);
	betArea = _spriteBetAreas.getBetArea(BJLBETT);
	rectArea = betArea->spriteArea->getBoundingBox();
	rectArea.origin += Vec2(rectArea.size.width*scaleRect, rectArea.size.height*scaleRect);
	rectArea.size = rectArea.size - Size(rectArea.size.width*scaleRect * 2, rectArea.size.height*scaleRect * 2);
	rectBetAreas.push_back(rectArea);
	if (!(_playerCmControl = BjlCmControl::create(layerPlayerCm, &_cmInfos, ptPlayers, _ptCmLose, rectBetAreas, roomInfo->type->reward_seconds)))
		return false;




	//本人筹码显示层
	layerCm = Layer::create();
	RootNode->addChild(layerCm, 1);		//添加到rootnode节点，以免盖住投注金额显示
	layerCard = Layer::create();
	this->addChild(layerCard, 1);
	this->addChild(_layerBjlLz, 1);		//路单在上，不修改zorder是因为在android下无效

	//多人房间，默认显示聊天窗口
	auto panelChat = RootNode->getChildByName<Layout*>("panelChat");
	if (!panelChat)
		return false;
	if (!(_listChat = panelChat->getChildByName<ListView*>("listChat")))
		return false;
	TextField* txtMsg = panelChat->getChildByName<TextField*>("txtMsg");
	if (!txtMsg)
		return false;

	//获取txtMsg位置与大小信息后直接删除，用editBox控件代替
	if (!(_editBoxMsg = EditBox::create(txtMsg->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxMsg->setAnchorPoint(txtMsg->getAnchorPoint());
	_editBoxMsg->setPosition(txtMsg->getPosition());
	txtMsg->removeFromParent();
	_editBoxMsg->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxMsg->setPlaceHolder(Language::getStringByKey("InputChatMsg"));
	_editBoxMsg->setDelegate(this);
	panelChat->addChild(_editBoxMsg);
	auto btSend = panelChat->getChildByName<Button*>("btSend");
	if (!btSend)
		return false;
	btSend->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON); sendChatMsg(); });
	// register callback func




	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onEnterRoomCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_EnterRoom), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onHistoryDataCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_HistoryData), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onBeginBetCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_BeginBet), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onShowCardsCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_ShowCards), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRoundOverCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoundOver), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onCutCardCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_CutCard), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onNewRoomPlayerCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_NewRoomPlayer), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onDelRoomPlayerCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_DelRoomPlayer), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRoomPlayersCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomPlayers), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onChatCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RecvChat), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onSitClicked), "BjlRoomPlayerSit", nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onSitCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RecvSit), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvRoomBetCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RecvRoomBet), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvRoomBetTotalCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RecvRoomBetTotal), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvRoomBetProfitCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_BetProfit), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvRoomAwardCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Award), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvBetCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Bet), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onRecvFpCallBack), "BjlFpMsg", nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::UpdateBalance), "UpdateBalance", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::onCurrentIntervalTaskFinish), "CurrentIntervalTaskFinish", nullptr);
	//NotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneBjlRoom::OnBetCallBack), GetMsgTypeString(MSGTYPE_BjlGameSg, MSGTYPE_BjlGameSg_Bet), NULL);
	UpdateBalance(this);
	// 翻牌界面
	if (!(_layerBjlFpM = LayerBjlFpM::create()))
		return false;
	_layerBjlFpM->_checkFP = _checkFP;
	_layerBjlFpM->setVisible(false);
	this->addChild(_layerBjlFpM, 1);
	// 刷新服务器时间，再次同步游戏时间
	TimeControl::GetInstance()->RequestServerTime(0.0f);
	

	return true;
}





void SceneBjlRoom::onCurrentIntervalTaskFinish(Ref *pSender)
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

SceneBjlRoom::~SceneBjlRoom()
{
	// remove callback func
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_roomData)
		delete _roomData;
	for (auto player : _roomPlayers)
		delete player.second;
	for (auto cmInfo : _cmInfos)
		delete cmInfo;
}
void SceneBjlRoom::onEnter()
{
	Scene::onEnter();
	SoundControl::PlayMusic(BGM::BGM_BACCARAT);
	/*auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	layerLoading->SetString("loading...");
	this->addChild(layerLoading);*/

	LayerLoading::Wait();

	Api::Game::mo_baccarat_enter(_roomInfo->id);

	addGuide();
}
void SceneBjlRoom::onExit()
{
	Api::Game::mo_baccarat_leave();
	Scene::onExit();
}

void SceneBjlRoom::sendChatMsg()
{
	if (Api::Game::mo_baccarat_chat(_editBoxMsg->getText()))
		_editBoxMsg->setText("");
}
void SceneBjlRoom::editBoxReturn(EditBox* editBox)
{
	if (_editBoxMsg == editBox)
		sendChatMsg();
}
void SceneBjlRoom::onSitClicked(Ref *pSender)
{
	BjlTablePlayerPanel *panel = (BjlTablePlayerPanel *)pSender;
	Api::Game::mo_baccarat_sit(panel->num);

	
}
void SceneBjlRoom::onChatCallBack(Ref *pSender)
{
	//聊天回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		ChatData chatData;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitChatData(msg->data, &chatData) && (_roomPlayers.find(chatData.id) != _roomPlayers.end()))
		{
			//成功
			//检测是否是有座玩家聊天消息，如果是，则显示聊天泡泡
			int sid = _tableControl->GetSitIdFromUserId(chatData.id);
			if (sid > 0)
				_sitChatControl->ShowChat(sid, chatData.msg);
			//将聊天信息加入聊天列表
			BjlRoomPlayer *player = _roomPlayers[chatData.id];
			float widthTotal = _listChat->getContentSize().width;
			string nickname;
			Color4B color;
			if (player->id == 0)
			{
				//系统消息
				nickname = "系统";
				color = Color4B::RED;
			}
			else if (player->id == _userData->user_id)
			{
				//本人消息
				nickname = "我";
				color = Color4B::GREEN;
			}
			else
			{
				nickname = player->nickname;
				color = Color4B(0x00, 0xcc, 0xff, 0xff);
			}
			Text *lbNickname = Text::create(nickname + ":", "", 28);
			lbNickname->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
			lbNickname->setTextColor(color);
			float widthNickname = lbNickname->getContentSize().width;
			Text *lbMsg = Text::create(chatData.msg, "", 28);
			lbMsg->setTextAreaSize(Size(widthTotal - widthNickname, 0));
			lbMsg->ignoreContentAdaptWithSize(false);
			lbMsg->setTextHorizontalAlignment(TextHAlignment::LEFT);
			lbMsg->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
			float heightMsg = lbMsg->getContentSize().height;
			auto layout = Layout::create();
			layout->addChild(lbNickname);
			layout->addChild(lbMsg);
			layout->setContentSize(Size(widthTotal, heightMsg));
			lbNickname->setPosition(Vec2(0.0f, heightMsg));
			lbMsg->setPosition(Vec2(widthNickname, 0.0f));
			_listChat->pushBackCustomItem(layout);
			while (_listChat->getItems().size() > MAXCHATRECORD)
				_listChat->removeItem(0);
			_listChat->scrollToBottom(0.5f, false);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onChatCallBack", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onEnterRoomCallBack(Ref *pSender)
{
	//进入游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		//auto layerLoading = this->getChildByName<LayerLoading*>("LayerLoading");
		LayerLoading::Close();
		if (msg->code == 0)
		{
			//成功进入,等等服务端推送数据
			if (_roomInfo->id != atoi(msg->data.c_str()))
			{
				CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onEnterRoomCallBack", "send roomid != recv roomid", msg->code, msg->data.c_str());
				/*if (layerLoading)
					layerLoading->SetString("enter room fail");*/
				LayerLoading::CloseWithTip("enter room fail");
				scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
				return;
			}
			/*if (layerLoading)
				layerLoading->SetString("loading...");*/
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onEnterRoomCallBack", "enter room error.", msg->code, msg->data.c_str());
			/*if (layerLoading)
				layerLoading->SetString("enter room fail");*/

			LayerLoading::CloseWithTip("enter room fail");
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onHistoryDataCallBack(Ref *pSender)
{
	//游戏房间历史数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		//auto layerLoading = this->getChildByName<LayerLoading*>("LayerLoading");

		LayerLoading::Close();

		if (msg->code == 0)
		{
			//成功获取
			// init historydata
			BjlRoomData *data = GameControlBjl::GetInstance()->InitRoomHistoryData(msg->data);
			if (data)
			{
				//成功
				/*if (layerLoading)
					layerLoading->removeFromParent();*/

				SetRoomData(data);
			}
			else
			{
				//解析历史数据失败
				CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onHistoryDataCallBack", "process history data fail.", msg->code, msg->data.c_str());
				/*if (layerLoading)
					layerLoading->SetString("process history data fail.");*/
				LayerLoading::CloseWithTip("process history data fail.");
				scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
			}
		}
		else
		{
			//失败,返回大厅
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onHistoryDataCallBack", "get history data fail.", msg->code, msg->data.c_str());
			/*if (layerLoading)
				layerLoading->SetString("get history data fail.");*/
			LayerLoading::CloseWithTip("get history data fail.");
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::SetRoomData(BjlRoomData *data)
{
	if (data == nullptr)
		return;
	if (_roomData)
		delete _roomData;
	_roomData = data;
	_cardDatas = "";
	//清除历史数据
	_bjlLzSm->clearLz();
	_layerBjlLz->clearLz();
	_lzDatas.clear();
	//lbRoomId->setString(_roomData->uuid);
	//初始化路单
	for (int i = 0; i < _roomData->history_rounds.size(); i++)
	{
		_cardDatas.append(_roomData->history_rounds[i].data);
		if (i > 0)
		{
			if (!_pmInfos.SetCards(_roomData->history_rounds[i].data.c_str()))
			{
				CCLOG("SceneBjlRoom::SetRoomData\tset cards error");
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


void SceneBjlRoom::onBeginBetCallBack(Ref *pSender)
{
	//开始投注回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		double time;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitBeginBetData(msg->data, _roomData, &time))
		{
			//成功获取
			if (GuideControl::GetInstance()->checkInGuide()){
				_timeBetEnd = TimeControl::GetInstance()->GetServerTime()+15;
			}
			else{
				_timeBetEnd = time;
			}
			
			double remainTime = _timeBetEnd - TimeControl::GetInstance()->GetServerTime();
			_statusGame = STATUSBJLGAME::BETSTART;
			char str[10];
			sprintf(str, "%.0f", remainTime);
			TextTimeLeft->setString(str);
			TextTimeLeft->setVisible(true);
			//清空桌面，开始投注
			_playerCmControl->clear();
			layerCm->removeAllChildren();
			layerCard->removeAllChildren();
			_spriteBetAreas.clearBet();
			checkAndSetAllAreaBetStatus();//检测
			setTotalBetInfo(nullptr);
			_layerResult->setVisible(false);
			panelCards->setVisible(false);
			TextPtP->setText("");
			TextPtB->setText("");
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onBeginBetCallBack", "get beginbet data fail.", msg->code, msg->data.c_str());
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onShowCardsCallBack(Ref *pSender)
{
	//牌局结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BjlRoomDataRound round;
		double time;
		bool flip = true;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitShowCardsData(msg->data, &round, &time, &flip))
		{
			//成功获取
			_roomData->history_rounds.push_back(round);		//添加到历史数据中
			funcFP(round.data.c_str(), flip);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onShowCardsCallBack", "get showcards data fail.", msg->code, msg->data.c_str());
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onAwardCallBack(Ref *pSender)
{
}

void SceneBjlRoom::onRoundOverCallBack(Ref *pSender)
{
	//牌局结束回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		double time;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitRoundOverData(msg->data, &_roomData->history_rounds[0], &time))
		{
			//成功
			//TimeLeft = (int)(time - TimeControl::GetInstance()->GetServerTime());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRoundOverCallBack", "get roundover data fail.", msg->code, msg->data.c_str());
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onCutCardCallBack(Ref *pSender)
{
	//切牌回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BjlRoomDataRound round;
		double time;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitCutCardData(msg->data, &round, &time))
		{
			//成功
			//TimeLeft = (int)(time - TimeControl::GetInstance()->GetServerTime());
			SetRoomData(new  BjlRoomData());		//重置牌局数据
			_roomData->uuid = round.seed;		//调用成功返回时，seed为uuid
			_roomData->secret = round.secret;
			round.num = 0;
			round.seed = "";
			_roomData->history_rounds.push_back(round);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRoundOverCallBack", "get roundover data fail.", msg->code, msg->data.c_str());
			scheduleOnce([](float dt){Director::getInstance()->popScene(); }, 1.0f, "delayExit");
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onNewRoomPlayerCallBack(Ref *pSender)
{
	//新玩家进入房间回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		string data = "[" + msg->data + "]";		//将数据放入数组中，以便用InitRoomPlayers函数统一处理
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitRoomPlayers(data, &_roomPlayers))
		{
			//成功
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onNewRoomPlay", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onDelRoomPlayerCallBack(Ref *pSender)
{
	//玩家离开房间回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功
			int id = atoi(msg->data.c_str());
			delete _roomPlayers[id];
			_roomPlayers.erase(id);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onNewRoomPlay", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onSitCallBack(Ref *pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		vector<BjlRoomSitInfo> sitInfos;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitSitPlayers(msg->data, &sitInfos))
		{
			//成功
			for (auto sitInfo : sitInfos)
			{
				if (sitInfo.userid == 0)
				{
					//玩家离开座位
					_tableControl->SetPlayerOnSeat(sitInfo.pos, nullptr);
					if (_seatId == sitInfo.pos)
						_seatId = 0;		//如果与本人座位相同，则表示本人离开座位
				}
				else
				{
					auto playerIt = _roomPlayers.find(sitInfo.userid);
					if (playerIt == _roomPlayers.end())
						continue;		//找不到玩家信息
					BjlRoomPlayer *player = playerIt->second;
					player->balance = sitInfo.balance;
					_tableControl->SetPlayerOnSeat(sitInfo.pos, player);
					if (sitInfo.userid == _userData->user_id)
						_seatId = sitInfo.pos;		//如果与本人ID相同，则表示本人坐在此位置
				}
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onSitCallBack", "get sit data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onRecvBetCallBack(Ref *pSender)
{
	//投注消息回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		int64_t balance;
		int num;
		BJLBETINFO betInfo;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitBetResultInfo(msg->data, &balance, &num, &betInfo))
		{
			//成功
			//如果当前本人无座，则需要更新本人投注数组，以便在收到无座玩家投注消息时，将本人投注从中扣除
			//如果当前本人有座，无须处理，因为在收到有座玩家消息时，如果发现与本人座位号相同，直接忽略数据
			_userAmount = balance;

			if (_seatId == 0)
			{
				_myBetInfoNoSeat.Add(&betInfo);
			}
		}
		else
		{
			


			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRecvBetCallBack", "get bet result fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏

		LayerLoading::Close(true);
	});
}
void SceneBjlRoom::onRecvRoomBetCallBack(Ref *pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		map<int, BJLBETINFO*> betDatas;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitBjlRoomBet(msg->data, &betDatas))
		{
			//成功
			for (auto betData : betDatas)
			{
				if (betData.first == 0)
				{
					//检查是否有无座玩家投注记录
					betData.second->SafeSub(&_myBetInfoNoSeat);
					_myBetInfoNoSeat = { 0 };
				}
				else
				{
					//检查是否与玩家座位ID相同
					if (betData.first == _seatId)
						*(betData.second) = { 0 };
				}
			}
			_playerCmControl->bet(&betDatas);
			for (auto betData : betDatas)
			{
				delete betData.second;
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRecvRoomBetCallBack", "get room bet data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onRecvRoomBetProfitCallBack(Ref *pSender)
{
	//收到房间玩家盈亏消息，此消息表示开始派奖,pos:0为本人，1－8为有座玩家
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (_layerBjlFpM->isVisible()){
			//关闭翻牌界面
			_layerBjlFpM->setVisible(false);
			_layerBjlFpM->clear();
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
		}
		vector<BJLBETPROFIT> betProfits;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitBetProfits(msg->data, &betProfits))
		{

			//成功
			for (auto betProfit : betProfits)
			{
				if (betProfit.pos == 0)
				{
					//本人余额更新
					_userAmount = betProfit.balance;
					_textUserAmount->setString(Comm::GetFloatShortStringFromInt64(_userAmount));
				}
				else
					_tableControl->SetPlayerProfit(betProfit.pos, betProfit.balance);
			}
			//TimeLeft = _roomInfo->type->reward_seconds;		//time to show result
			//显示大赢家
			_tableControl->ShowBigWinner((float)_roomInfo->type->reward_seconds);

			doAward();
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRecvRoomBetProfitCallBack", "get room bet profit data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onRecvRoomAwardCallBack(Ref *pSender){
	//收到房间牌局输赢结果消息
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (_layerBjlFpM->isVisible()){
			//打开翻牌界面中所有的牌面
			for (int i = 0; i < _pmInfos.Player.count; i++)
			{
				_layerBjlFpM->setCardStatus(i, 2);
			}
			for (int i = 0; i < _pmInfos.Banker.count; i++)
			{
				_layerBjlFpM->setCardStatus(i + 3, 2);
			}
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onRecvRoomBetTotalCallBack(Ref *pSender)
{
	//房间玩家总投注数据回调函数
	if (pSender == nullptr)
		return;

	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BJLBETINFO betInfo;
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitBjlBetInfo(msg->data, &betInfo))
		{
			//成功
			setTotalBetInfo(&betInfo);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRecvRoomBetTotalCallBack", "get room bet total data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::onRoomPlayersCallBack(Ref *pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		for (auto player : _roomPlayers)
			delete player.second;
		_roomPlayers.clear();		//清除旧的玩家数据
		if (msg->code == 0 && GameControlBjl::GetInstance()->InitRoomPlayers(msg->data, &_roomPlayers))
		{
			//成功
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onNewRoomPlay", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void SceneBjlRoom::UpdateBalance(Ref *ref)
{
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BalanceData *balance = BillControl::GetInstance()->GetBalanceData(0);
		if (!balance)
			return;
		_userAmount = (int64_t)balance->balance;
		_textUserAmount->setString(Comm::GetFloatShortStringFromInt64(_userAmount));
	});
}
void SceneBjlRoom::ShowLz(Ref* ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	_layerBjlLz->setVisible(true);
}
// 翻牌相关

CMINFO* SceneBjlRoom::getCurrentCMInfo()
{
	Sprite *sp = _spriteCms.getCurrentCmSprite();
	if (!sp) return nullptr;
	CMINFO *cmInfo = (CMINFO*)sp->getUserData();
	return cmInfo;
}

bool SceneBjlRoom::isAreaCanBet(SpriteBetArea *betArea, CMINFO *cmInfo)
{
	//检测是否走出限额
	int64_t maxBet = _roomInfo->type->max_bet;

	if (betArea->type&(BJLBETTYPE::BJLBETPP | BJLBETTYPE::BJLBETBP | BJLBETTYPE::BJLBETT)) {
		//对子，和，限额为正常限额的1/10
		maxBet /= 10;
	}
	
	string temps = Comm::DeleteCharcter(_textUserAmount->getString(), ",");
	int64_t temp = atoll(temps.c_str());
	
	if (cmInfo->amount > temp) {
		return false;
	}

	//最小投注限额为最大限额的1/100
	int64_t minBet = maxBet / 100;
	if ((betArea->amount + cmInfo->amount) > maxBet || cmInfo->amount < minBet) {
		//超出投注限额
		return false;
	}

	//out of time
	double timeLeft = _timeBetEnd - TimeControl::GetInstance()->GetServerTime();
	if (timeLeft <= 0)
		return false;

	return true;
}

void SceneBjlRoom::checkAndSetAreaBetStatus(SpriteBetArea *betArea, CMINFO *cmInfo)
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


void SceneBjlRoom::checkAndSetAllAreaBetStatus()
{

	CMINFO *cmInfo = getCurrentCMInfo();// 空情况已处理
	Vector<SpriteBetArea*> betAreas = _spriteBetAreas.betAreas;
	for (SpriteBetArea* betArea : betAreas)
	{
		checkAndSetAreaBetStatus(betArea, cmInfo);
	}

}

//选中筹码
bool SceneBjlRoom::onCmClick(Touch* touch, Event* event)
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

//投注
bool SceneBjlRoom::onCmBet(Touch* touch, Event* event)
{
	if (_statusGame != STATUSBJLGAME::BETSTART)
		return false;

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

		

		if (!isAreaCanBet(betArea, cmInfo))
		{
			return false;
		}

		//添加子筹码
		SpriteCmSub *scs = SpriteCmSub::create(cmInfo->imgSmall, _spriteCms._curCm->amount, true, sp->getPosition(), _ptCmLose);
		layerCm->addChild(scs);
		scs->doBet(point);
		//add to betareas control class
		_spriteBetAreas.addBet(target, scs);

		checkAndSetAreaBetStatus(betArea, cmInfo);//增加之后再检测下状态, 个人投注区总额，无需同步其他玩家，忽略投注未成功情况。

		////Bet buffer
		//BjlBet bet;
		//bet.m_type = betArea->type;
		//bet.m_amount = cmInfo->amount;
		//bet.m_gameNum = _roomData->inning_num;
		//GameControlBjl::GetInstance()->Bet(bet);

		//投注
		if (!Api::Game::mo_baccarat_bet(BjlBetTypeToString(betArea->type), cmInfo->amount, _roomData->inning_num))
		{
			CCLOG("SceneBjlRoom::onCmBet\tbet error.");
		}
		else
		{	
			//投注成功
			//SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BET);
			if (_roomInfo->type->max_bet <= 10000){
				//验证完成任务 首次在中级房进行游戏
				PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(8)));
			}
			else if (_roomInfo->type->max_bet <= 10000 * 10){
				//验证完成任务 首次在中级房进行游戏
				PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(9)));
			}
			else{
				//验证完成任务 首次在高级房进行游戏
				PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(10)));
			}

			_curBet += cmInfo->amount;
			
			string temps = Comm::DeleteCharcter(_textUserAmount->getString(), ",");
			int64_t temp = atoll(temps.c_str()) - cmInfo->amount;
			_textUserAmount->setString(Comm::GetFloatShortStringFromInt64(temp));
		}

		return true;
	}
	return false;
}



void SceneBjlRoom::funcTimeLeft(float d)
{
	switch (_statusGame)
	{
	case BETSTART:
	{
		double timeLeft = _timeBetEnd - TimeControl::GetInstance()->GetServerTime();

		////Bet Buffer
		//if ((int)timeLeft % 2 == 0)
		//{
		//	bool succeed = GameControlBjl::GetInstance()->SyncBet();
		//	
		//	if (succeed)
		//	{
		//		GameControlBjl::GetInstance()->FinishBetTask(_roomInfo);
		//	}
		//}
		
		if (timeLeft > 0)
		{
			char str[10];
			sprintf(str, "%.0f", timeLeft);
			TextTimeLeft->setText(str);
		}
		else
		{
			if (!GuideControl::GetInstance()->checkInGuide()){
				_statusGame = STATUSBJLGAME::BETEND;
				checkAndSetAllAreaBetStatus();
			}
		}

	}
		break;
	case BETEND:
		break;
	case FPSTART:
		break;
	case FPEND:
		break;
	default:
		break;
	}
}

//翻牌结束
void SceneBjlRoom::actFpDone()
{
	TextPtP->setString(StringUtils::format("%d", _pmInfos.Player.point));
	TextPtB->setString(StringUtils::format("%d", _pmInfos.Banker.point));

	SoundControl::PlayPoint(_pmInfos.Player.point, _pmInfos.Banker.point);
	//正常牌局
	_statusGame = STATUSBJLGAME::FPEND;
}
//派奖
void SceneBjlRoom::doAward()
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

		//验证完成任务 第一次买和胜利
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(5)));
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
		//验证完成任务 第一次买庄对或者闲对胜利
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(4)));
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

		//验证完成任务 第一次买庄对或者闲对胜利
		PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(4)));
	}
	else
	{
		amount = _spriteBetAreas.getBetAmount(BJLBETTYPE::BJLBETBP);
		if (amount < 0)
			return;
		_layerResult->addAmount(-amount);
		_spriteBetAreas.setLoseArea(BJLBETTYPE::BJLBETBP);
	}
	_playerCmControl->award(lzStatus);
	_userAmount += (int64_t)_layerResult->getAmount();
	
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
		

		SoundControl::PlayEffect(cocos2d::random() % 2 ? "sound/win0.mp3" : "sound/win1.mp3");
	}
	else if (_layerResult->getAmount() < 0)
		SoundControl::PlayEffect("sound/lose0.mp3");

	if (GuideControl::GetInstance()->checkInGuide()){
		
		GuideControl::GetInstance()->injectHander(11, "finish_guide", [=](){
			GuideControl::GetInstance()->finishGuideStep();
			Director::getInstance()->popScene();
		});
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N11, RootNode);
	}
}
void SceneBjlRoom::actFp(Node* node, BJLFPINDEX n)
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
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLB1));
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
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLB2));
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
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLB3));
		else
			func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoom::actFpDone, this));
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
		func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLP2));
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
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLP3));
		else if (_pmInfos.Banker.count == 3)
			func = CallFuncN::create(CC_CALLBACK_1(SceneBjlRoom::actFp, this, BJLFPINDEX::BJLB3));
		else
			func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoom::actFpDone, this));
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
		func = CallFuncN::create(CC_CALLBACK_0(SceneBjlRoom::actFpDone, this));
		sp->runAction(Sequence::create(Spawn::create(MoveTo::create(_timeFP / 2, Vec2(_ptCardB.x + _widthCard * 2, _ptCardB.y)), ScaleTo::create(_timeFP / 2, _scaleCard1), nullptr), func, nullptr));

		break;
	default:
		break;
	}
}
//开始发牌,flip人工翻牌时，等待fpbegin消息
void SceneBjlRoom::funcFP(const char* cards, bool flip)
{
	TextTimeLeft->setVisible(false);
	_statusGame = STATUSBJLGAME::FPSTART;
	// 分析牌面
	if (!_pmInfos.SetCards(cards))
		return;
	_cardDatas.append(cards);
	panelCards->setVisible(true);
	TextPtP->setString("");
	TextPtB->setString("");
	if (!flip)
		actFp(this, BJLFPINDEX::BJLP1);		//自动翻牌
}
void SceneBjlRoom::setTotalBetInfo(BJLBETINFO *betInfo)
{
	if (!betInfo)
	{
		lbAmountPAll->setString("");
		lbAmountPPAll->setString("");
		lbAmountBAll->setString("");
		lbAmountBPAll->setString("");
		lbAmountTAll->setString("");

		imgBgAmountPAll->setVisible(false);
		imgBgAmountPPAll->setVisible(false);
		imgBgAmountBAll->setVisible(false);
		imgBgAmountBPAll->setVisible(false);
		imgBgAmountTAll->setVisible(false);
	}
	else
	{
		if (betInfo->p>0)
			imgBgAmountPAll->setVisible(true);
		if (betInfo->pp > 0)
			imgBgAmountPPAll->setVisible(true);
		if (betInfo->b > 0)
			imgBgAmountBAll->setVisible(true);
		if (betInfo->bp > 0)
			imgBgAmountBPAll->setVisible(true);
		if (betInfo->t > 0)
			imgBgAmountTAll->setVisible(true);

		lbAmountPAll->setString(betInfo->p > 0 ? Comm::GetFloatShortStringFromInt64(betInfo->p) : "");
		lbAmountPPAll->setString(betInfo->pp > 0 ? Comm::GetFloatShortStringFromInt64(betInfo->pp) : "");
		lbAmountBAll->setString(betInfo->b > 0 ? Comm::GetFloatShortStringFromInt64(betInfo->b) : "");
		lbAmountBPAll->setString(betInfo->bp > 0 ? Comm::GetFloatShortStringFromInt64(betInfo->bp) : "");
		lbAmountTAll->setString(betInfo->t > 0 ? Comm::GetFloatShortStringFromInt64(betInfo->t) : "");
	}
}
// ¼üÎ»ÏìÓ¦º¯ÊýÔ­ÐÍ
void SceneBjlRoom::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	log("Key with keycode %d released", keyCode);
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{

		if (GuideControl::GetInstance()->checkInGuide()) return;

		SceneReports* report = this->getChildByName<SceneReports*>("SceneReports");
		if (report){
			report->removeFromParent();
		}
		else{
			if (_layerBjlLz->isVisible())
				_layerBjlLz->setVisible(false);
			else if (_layerBjlFpM->isVisible())
				_layerBjlFpM->setVisible(false);
			else
				Director::getInstance()->popScene();
		}
		
	}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

void SceneBjlRoom::updateCurrentTime(float dt)
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
// 收到翻牌消息
void SceneBjlRoom::onRecvFpCallBack(Ref *pSender){
	//翻牌消息回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	if (msg->type != MSGTYPE_BJLGAMEMO)
		return;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		switch (msg->typesub)
		{
		case PUSH_BEGIN_FLIP:
		{
			int playerId, bankerId;
			if (GameControlBjl::InitFpBeginData(msg->data, &playerId, &bankerId))
				onRecvFpBegin(playerId, bankerId);
		}
			break;
		case PUSH_PLAYER_FLIP:
		{
			int status;
			double endTime;
			if (GameControlBjl::InitFpStatusData(msg->data, &status, &endTime))
				onRecvFpStatus(status, endTime);
		}
			break;
		case CARD_SELECT:
		{
			int card;
			if (GameControlBjl::InitFpSelectData(msg->data, &card))
				onRecvFpSelect(card);
		}
			break;
		case CARD_FLIP:
		{
			int x1, y1, x2, y2;
			if (GameControlBjl::InitFpingData(msg->data, &x1, &y1, &x2, &y2))
				onRecvFping(x1, y1, x2, y2);
		}
			break;
		case CARD_OPEN:
		{
			int card;
			if (GameControlBjl::InitFpOpenData(msg->data, &card))
				onRecvFpOpen(card);
		}
			break;
		case PUSH_FLIP:		//中途进入房间时，会收到此消息
		{
			int playerid, bankerid, status;
			double endTime;
			vector<int> cardStatus;
			if (GameControlBjl::InitFpInitData(msg->data, &playerid, &bankerid, &status, &endTime, &cardStatus))
				onRecvFpInit(playerid, bankerid, status, endTime, cardStatus);
		}
			break;
		default:
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onRecvFpCallBack", "unknown subtype.", msg->code, msg->data.c_str());
			break;
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
	return;
}
// 开始翻牌消息
void SceneBjlRoom::onRecvFpBegin(int playerid, int bankerid){
	string playerName, bankerName;
	int playerAvatar = 0, bankerAvatar = 0;
	if (playerid != 0){
		if (_roomPlayers.find(playerid) == _roomPlayers.end())
			return;
		playerName = _roomPlayers[playerid]->nickname;
		playerAvatar = _roomPlayers[playerid]->avatar;
	}
	if (bankerid != 0){
		if (_roomPlayers.find(bankerid) == _roomPlayers.end())
			return;
		bankerName = _roomPlayers[bankerid]->nickname;
		bankerAvatar = _roomPlayers[bankerid]->avatar;
	}
	_layerBjlFpM->setCards(&_pmInfos);		//设置牌面信息
	_layerBjlFpM->setFpBegin(playerid, playerName, playerAvatar, bankerid, bankerName, bankerAvatar, _userData->user_id);

}
// 翻牌状态切换,[0:闲开牌, 1:庄开牌, 2:闲补牌, 3:庄补牌], time:结束时间
void SceneBjlRoom::onRecvFpStatus(int status, double endTime){
	_layerBjlFpM->setFpStatus(status, endTime);
}
// 选牌,[0: p1, 1: p2, 2: p3, 3: b1, 4: b2, 5: b3]
void SceneBjlRoom::onRecvFpSelect(int card){
	_layerBjlFpM->setFpSelect(card);
}

void SceneBjlRoom::onFpSelect(int card){
	return;
}
// 翻牌中
void SceneBjlRoom::onRecvFping(int x1, int y1, int x2, int y2){
	_layerBjlFpM->setFping(x1, y1, x2, y2);
}

void SceneBjlRoom::onFping(int x1, int y1, int x2, int y2){
	return;
}
// 开牌
void SceneBjlRoom::onRecvFpOpen(int card){
	_layerBjlFpM->setFpOpen(card);
}

void SceneBjlRoom::onFpOpen(int card){
	return;
}
// 结束翻牌
void SceneBjlRoom::onFpEnd(){
	return;
}
// 中途进入房间时，翻牌初始化信息
void SceneBjlRoom::onRecvFpInit(int playerid, int bankerid, int status, double endTime, vector<int>cardStatus){
	string playerName, bankerName;
	int playerAvatar = 0, bankerAvatar = 0;
	if (playerid != 0){
		if (_roomPlayers.find(playerid) == _roomPlayers.end())
			return;
		playerName = _roomPlayers[playerid]->nickname;
		playerAvatar = _roomPlayers[playerid]->avatar;
	}
	if (bankerid != 0){
		if (_roomPlayers.find(bankerid) == _roomPlayers.end())
			return;
		bankerName = _roomPlayers[bankerid]->nickname;
		bankerAvatar = _roomPlayers[bankerid]->avatar;
	}
	_layerBjlFpM->setCards(&_pmInfos);		//设置牌面信息
	_layerBjlFpM->setFpInit(playerid, playerName, playerAvatar, bankerid, bankerName, bankerAvatar, _userData->user_id, status, endTime, cardStatus);
	_layerBjlFpM->setVisible(true);
	//_layerBjlFpM->setFpStatus(status, endTime);
}



void SceneBjlRoom::addGuide()
{
	if (!GuideControl::GetInstance()->checkInGuide()) return;

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "sit_arrow", [=](){
		LayerGuideMask::GetInstance()->addArrow(DIRECTION::BOTTOM, ccp(515, 900));
	});


	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "sit_action", [=](){
		Api::Game::mo_baccarat_sit(3);
		LayerGuideMask::GetInstance()->cleanMask();
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N3, RootNode);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N3, "bet_rect_arrow", [=](){

		LayerGuideMask::GetInstance()->cleanMask();
		LayerGuideMask::GetInstance()->addArrow(DIRECTION::RIGHT, ccp(685, 90));
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
	});


	GuideControl::GetInstance()->injectHander(GUIDESTEP::N3, "bet_rect_100", [=](){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BACCARATE_SELECT);
		Sprite* target = (Sprite *)panelCmArea->getChildByName(StringUtils::format("cm%d", 2));

		if (_spriteCms.getCurrentCmSprite())
			_spriteCms._curCm->SetSelected(false);
		_spriteCms.setCurrentCm(target);
		_spriteCms._curCm->SetSelected(true);

		// 处理
		if (_statusGame == STATUSBJLGAME::BETSTART)
		{
			checkAndSetAllAreaBetStatus();
		}
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N4, RootNode);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "small_bet_intro", [=](){

		LayerGuideMask::GetInstance()->addTo(RootNode->getChildByName("circle_guide"));
		LayerGuideMask::GetInstance()->addTo(RootNode->getChildByName("rect_guide_left"));
		LayerGuideMask::GetInstance()->addTo(RootNode->getChildByName("rect_guide_right"));
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "big_bet_intro", [=](){

		LayerGuideMask::GetInstance()->cleanMask();
		LayerGuideMask::GetInstance()->addTo(RootNode->getChildByName("arc_guide_left"));
		LayerGuideMask::GetInstance()->addTo(RootNode->getChildByName("arc_guide_right"));
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "banker_bet_arrow", [=](){
		LayerGuideMask::GetInstance()->cleanMask();

		LayerGuideMask::GetInstance()->addArrow(DIRECTION::BOTTOM, ccp(1410, 510));
	});


	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "banker_bet", [=](){

		//添加子筹码
		SpriteCmSub *scs = SpriteCmSub::create("chip_100_small.png", 100, true, ccp(885,90), _ptCmLose);
		layerCm->addChild(scs);
		scs->doBet(ccp(1410,380));

		Api::Game::mo_baccarat_bet("b", 100, 0);

		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N5, RootNode);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N5, "wait_flip", [=](){
		LayerGuideMask::GetInstance()->cleanVideo();
	});

	//seat number guide
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "seat_number", [=](){
		SceneBjlRoom *pBjlRoom = dynamic_cast<SceneBjlRoom*>(Director::getInstance()->getRunningScene());
		if (pBjlRoom != NULL)
		{
			Node *pNodeT = pBjlRoom->getRootNode();
			Layout *pLayoutT = pNodeT->getChildByName<Layout*>("guide_seat_number");
			float delayTime = 1;
			for (vector<Node*>::iterator it = pLayoutT->getChildren().begin();
				it != pLayoutT->getChildren().end(); ++it)
			{
				ActionInterval *inA = (ActionInterval*)Sequence::create
					(
					DelayTime::create(delayTime),
					Show::create(),
					DelayTime::create(6 - delayTime),
					Hide::create(),
					nullptr
					);
				(*it)->runAction(inA);
				delayTime += 0.3;
			}
			pLayoutT->getChildren();
		}
	});

	//chip area effect
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N3, "chip_area_effect", [=](){
		SceneBjlRoom *pBjlRoom = dynamic_cast<SceneBjlRoom*>(Director::getInstance()->getRunningScene());
		if (pBjlRoom != NULL)
		{
			Node *pNodeT = pBjlRoom->getRootNode();
			Layout *layoutT = pNodeT->getChildByName<Layout*>("guide_chiparea");
			Node *nodeT1 = layoutT->getChildByName<Node*>("chipAreaEffect");
			float delayTime = 0.3;
			int effectNum = 2;
			ActionInterval *inA = (ActionInterval*)Sequence::create
				(
				Blink::create(2, 2),
				Hide::create(),
				nullptr
				);
			nodeT1->runAction(inA);

			Node *nodeT2 = layoutT->getChildByName<Node*>("txtPrompt");
			nodeT2->setVisible(true);
			ActionInterval *inA2 = (ActionInterval*)Sequence::create
				(
				DelayTime::create(10),
				Hide::create(),
				nullptr
				);
			nodeT2->runAction(inA2);
		}
	});

	//bet area effect
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "bet_area_effect", [=](){
		SceneBjlRoom *pBjlRoom = dynamic_cast<SceneBjlRoom*>(Director::getInstance()->getRunningScene());
		if (pBjlRoom != NULL)
		{
			Node *pNodeT = pBjlRoom->getRootNode();
			Layout *layoutT = pNodeT->getChildByName<Layout*>("betAreaEffect");
			ActionInterval *inA = (ActionInterval*)Sequence::create
				(
				Blink::create(2, 2),
				Hide::create(),
				nullptr
				);
			layoutT->runAction(inA);
			LayerGuideMask::GetInstance()->cleanMask();
		}
	});

	LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N2, RootNode);
}

Node* SceneBjlRoom::getRootNode()
{
	return RootNode;
}
