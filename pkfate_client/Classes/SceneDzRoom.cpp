#include "SceneDzRoom.h"
#include "cocostudio/CocoStudio.h"
#include "ui/UIListView.h"
#include "SpriteCmSub.h"
#include "LayerSetting.h"
#include "Settings.h"
#include "ApiGame.h"
#include "ApiBill.h"
#include "BillControl.h"
//#include "LayerBjlCalculator.h"
#include "LayerLoading.h"
#include "cmd.h"
#include "SoundControl.h"
#include "TimeControl.h"
#include "SceneReports.h"
#include "PKNotificationCenter.h"
#include "ui/UITextField.h"
#include "TaskControl.h"
#include "LayerMall.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "SoundControl.h"
#include "LayerDzHelp.h"
#include "GuideControl.h"
#include "LayerGuideMask.h"
#include "LayerInviteDzWait.h"
#include "LayerFriendInvite.h"
#include "LayerFriend.h"
#include "LayerMail.h"
#include "LayerDzGames.h"
#include "LayerDzAddScore.h"
#include "LayerSNGTip.h"

#define MAXCHATRECORD	100		//最大聊天记录数
using namespace std;


const string PokerType[11] = { "ZERO", "HIGH_CARD", "ONE_PAIR", "TWO_PAIR", "THREE_OF_A_KIND", "STRAIGHT", "FLUSH","FULL_HOURSE","FOUR_OF_A_KIND", "STRAIGHT_FLUSH", "ROYAL_FLUSH" };

static float Slip_Max_Y = 690;
static float Slip_Min_Y = 220;
static float Blue_Lenght = 554;
static float Blue_Pos_X = 170;

void BetAddPanel::injectUI(Node * UI)
{
	UI->setVisible(true);
	this->UI = UI;
	bet_amount = UI->getChildByName<Text*>("bet_amount");
	all_light = UI->getChildByName<Sprite*>("all_light");
	btn_x1_2 = UI->getChildByName<Button*>("btn_x1_2");
	btn_x2_3 = UI->getChildByName<Button*>("btn_x2_3");
	btn_x1 = UI->getChildByName<Button*>("btn_x1");
	btn_all = UI->getChildByName<Button*>("btn_all");
	btn_slip = UI->getChildByName<Sprite*>("btn_slip");
	btn_slip->setZOrder(1);
	Sprite * blue = UI->getChildByName<Sprite*>("progress_blue");

	betProgress = CCProgressTimer::create(blue);
	
	betProgress->setPosition(blue->getPosition());

	blue->removeFromParent();
	UI->addChild(betProgress);
	all_light->setVisible(false);

	//设置进度条的模式
	//kCCProgressTimerTypeBar表示条形模式
	betProgress->setType(kCCProgressTimerTypeBar);

	betProgress->setBarChangeRate(ccp(0, 1));

	betProgress->setMidpoint(ccp(0, 0));

	minBet = 0;

	//btn_x1_2->setVisible(false);

	btn_x1_2->addClickEventListener([&](Ref *sender){
		// 进度条变化 ，顶上注额
		if (chip > 0){
			setPercent((gamePoolBet*0.5)*100 / chip );
		}
	});
	//btn_x2_3->setVisible(false);
	btn_x2_3->addClickEventListener([&](Ref *sender){
		if (chip > 0){
			setPercent((gamePoolBet * 2 / 3)*100 / chip );
		}
	});
	//btn_x1->setVisible(false);
	btn_x1->addClickEventListener([&](Ref *sender){
		if (chip > 0){
			setPercent((gamePoolBet)*100 / chip );
		}
	});
	//btn_all->setVisible(false);
	btn_all->addClickEventListener([&](Ref *sender){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		setPercent(100);

	});
	btn_all->setPressedActionEnabled(true);
	//实现拖动
	Node* addBg = UI->getChildByName<Node*>("add_bg");

	auto listener = EventListenerTouchOneByOne::create();

	listener->setSwallowTouches(true);

	isHitSlip = false;

	listener->onTouchBegan = [=](Touch* touch, Event* event){

		Point locationInNode = addBg->convertToNodeSpace(touch->getLocation());
		Size s = addBg->getContentSize();
		Rect rect = Rect(0, 0, s.width, s.height);

		if (!rect.containsPoint(locationInNode))
		{
			setVisible(false);
			return false;
		}

		locationInNode = btn_slip->convertToNodeSpace(touch->getLocation());
		s = btn_slip->getContentSize();
		rect = Rect(0, 0, s.width, s.height);

		if (rect.containsPoint(locationInNode))
		{
			isHitSlip = true;
			return true;
		}
		return false;
	};

	listener->onTouchMoved = [=](Touch* touch, Event* event){

		if (isHitSlip)
		{
			Vec2 p = touch->getLocation();
			float slip_y = touch->getLocation().y;
			if (slip_y > Slip_Max_Y)
			{
				slip_y = Slip_Max_Y;
			}
			else if (slip_y < Slip_Min_Y)
			{
				slip_y = Slip_Min_Y;
			}
			
			int percent = 100* (slip_y - Slip_Min_Y) / (Slip_Max_Y - Slip_Min_Y);

			setPercent(percent);
		}
	};

	listener->onTouchEnded = [=](Touch* touch, Event* event){
		isHitSlip = false;
	};

	UI->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, UI);

	this->chip = 0;
	setPercent(0);

}

void BetAddPanel::setMinBet(int minBet){
	reset();

	DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType
	int ante = DZControl::GetInstance()->getAnte();
	if (minBet < ante){ //处理别人让牌callbet = 0导致的问题 ,不得低于最大盲注
		//minBet = type->ante;

		minBet = ante;
	}

	this->minBet = minBet;
	if (minBet >= this->chip){
		setPercent(100);//all in的情况
	}
	else{
		setPercent(3);
	}
	
}

void BetAddPanel::updateGamePoolBet(int bet)
{
	gamePoolBet = bet;
	//判断携带  禁用按钮
	//btn_x1->setVisible(gamePoolBet<chip);
	//btn_x1_2->setVisible(1/2*gamePoolBet<chip);
	//btn_x2_3->setVisible(2/3*gamePoolBet<chip);
}


int BetAddPanel::getBet()
{
	return addBet;
}

void BetAddPanel::reset()
{
	this->minBet = 0;
	this->percent = 0;
}


void BetAddPanel::setPercent(int percent)
{

	if (this->percent != percent)
	{

		this->percent = percent;

		int slip_y = (Slip_Max_Y - Slip_Min_Y)*percent / 100 + Slip_Min_Y;

		btn_slip->setPositionY(slip_y);

		int blue_percent = 100 * (slip_y - Blue_Pos_X) / Blue_Lenght;
		betProgress->setPercentage(blue_percent);

		int chipN = DzPokerUtils::getChipN(chip,minBet, percent);

		if (percent >= 100)
		{
			all_light->setVisible(true);//动画
			bet_amount->setVisible(false);
			bet_amount->setString(Comm::GetFloatShortStringFromInt64(chipN));
		}
		else
		{
			all_light->setVisible(false);
			bet_amount->setString(Comm::GetFloatShortStringFromInt64(chipN));
			bet_amount->setVisible(true);
		}

		addBet = chipN;

	}
}


void BetAddPanel::updateChip(int chip)
{
	this->chip = chip;
}



bool BetAddPanel::isVisible()
{   
	return UI->isVisible();
}

void BetAddPanel::setVisible(bool is)
{
	UI->setVisible(is);
}




void DzPlayer::injectUI(Node * UI)
{
	this->UI = UI;
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");
	name = seatPanel->getChildByName<Text*>("lbNickName");
	name->setTextColor(ccc4(255, 255, 255,255));
	avatar = seatPanel->getChildByName<Sprite*>("imgAvatar");
	card_left = seatPanel->getChildByName<Sprite*>("card_left");
	card_right = seatPanel->getChildByName<Sprite*>("card_right");

	card_left->setVisible(false);
	card_right->setVisible(false);

	betAmount = gamingPanel->getChildByName<Text*>("lbAmount");
	betAmount2 = seatPanel->getChildByName<Text*>("lbAmount2");
	progressSprite = seatPanel->getChildByName<Sprite*>("progress");
	betAmount2->setZOrder(1);
	setBetVisible(false);


	waitProgress = CCProgressTimer::create(progressSprite);
	waitProgress->setPosition(progressSprite->getPosition());
	seatPanel->removeChild(progressSprite);

	progress_bg = Sprite::create("dz/room/gray.png");
	seatPanel->addChild(progress_bg);
	progress_bg->setPosition(progressSprite->getPosition());

	seatPanel->addChild(waitProgress);
	Sprite* progress_font = Sprite::create("dz/room/progress_font.png");
	seatPanel->addChild(progress_font);
	progress_font->setPosition(progressSprite->getPosition());
	
	waitProgress->setType(kCCProgressTimerTypeRadial);
	waitProgress->setPercentage(100);
	waitProgress->setReverseProgress(true);


	Button * btnSit = UI->getChildByName<Button*>("btSit");
	btnSit->setVisible(false);
	btnSit->addClickEventListener([&](Ref *sender){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		int logic_pos = DZControl::GetInstance()->getLogicPos(pos);
		Api::Game::mo_dzpk_sit(logic_pos);
	});
	btnSit->setPressedActionEnabled(true);
	this->roomPlayer = nullptr;
	this->round = 0;
	this->roundBet = 0;
	CCLOG("DzPlayer::injectUI(Node * UI)  this->roundBet = 0;");
	this->isPlayFlip = false;

	seatPlayer(false);//没有坐人
}


void DzPlayer::updateStatus(int status)
{
	switch (status)
	{
	case TPParticipantStatus::GAMING:
	{
		this->avatar->setColor(ccc3(255, 255, 255));
	}
		break;
	case TPParticipantStatus::LEAVE_ROOM:
	case TPParticipantStatus::LEAVE:
	{
		if (DZControl::GetInstance()->gameProcess == GameProcess::IS_GAMING) //在游戏过程中
		{
			if (roomPlayer->status == TPParticipantStatus::LEAVE)//处在离开状态,暗掉头像
			{
				this->avatar->setColor(ccc3(127, 127, 127));
				name->setString(Language::getStringByKey("Leave"));
			}
		}
		else{
			leave();//离开
		}
	}
		break;
	default:
		break;
	}
}

void DzPlayer::updateAction(int status ,int action)
{
	betStatus = status;
	betAction = action;
	switch (action)
	{
	case BETACTION::CALL:
	{
		betAmount2->setString(Language::getStringByKey("CALL"));

		if (status == TPParticipantStatus::ALL_IN){
			betAmount2->setString("ALL_IN");
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_ALLIN);
		}
		else{
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CALL);
			
		}
	}
		break;
	case BETACTION::RAISE:
	{
		betAmount2->setString(Language::getStringByKey("RAISE"));
		if (status == TPParticipantStatus::ALL_IN){
			betAmount2->setString("ALL_IN");
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_ALLIN);
		}
		else{
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RAISE);
		}
	}
		break;
	case  BETACTION::FOLD:
	{
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_FOLD);
		betAmount2->setString(Language::getStringByKey("FOLD"));

		card_left->setVisible(false);
		card_right->setVisible(false);

		Sprite * pokerBack = Sprite::createWithSpriteFrameName("SmallCardB1.png");//背面
		Size size = Director::getInstance()->getVisibleSize();
		UI->addChild(pokerBack);
		pokerBack->setScale(card_left->getScale());
		pokerBack->setPosition(card_left->getPosition());
		Vec2 p = UI->convertToNodeSpace(ccp(size.width / 2, size.height / 2 + 130));

		pokerBack->runAction(Sequence::create(
			Spawn::create(EaseExponentialOut::create(MoveTo::create(0.8f, p)), ScaleTo::create(0.8f, 0.2), nullptr),
			FadeOut::create(0.3f),
			CallFunc::create([=](){
			pokerBack->removeFromParent();

		}), nullptr));

	}
		break;
	case BETACTION::BIG_BLIND:{
		betAmount2->setString(Language::getStringByKey("BIG_BLIND"));
	}
		break;
	case BETACTION::SMALL_BLIND:{
		betAmount2->setString(Language::getStringByKey("SMALL_BLIND"));
	}
		break;
	case BETACTION::CHECK:{
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CHECK);
		betAmount2->setString(Language::getStringByKey("CHECK"));
	}
		break;
	default:
		break;
	}

	

	
}

void DzPlayer::updateBetInfo(BetInfo* info){
	if (info == nullptr) return;

	updateBet(info->bet_amount,info->round_amount);
	updateChip(info->chip);
	updateAction(info->status,info->action);

	isGameCallEnd = info->action != BETACTION::FOLD && this->round == 3;
	
}

int DzPlayer::getBet(){
	return bet;
}//获取当前round bet金额

void DzPlayer::updateBet(int bet,int roundBet){

	CCLOG("bet:%d <= 0 || this->bet:%d == bet:%d || roundBet:%d<=0", bet, this->bet, bet, roundBet);

	if (bet <= 0 || this->bet == bet || roundBet<=0) return;
	
	int add = bet - this->bet;
	this->bet = bet;
	this->roundBet = roundBet;
	CCLOG(" DzPlayer::updateBet(int bet,int roundBet)  this->roundBet = roundBet; %d", roundBet);
	
	//动画效果
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");

	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	lotteryIcon->setPosition(waitProgress->getPosition());

	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	seatPanel->addChild(lotteryIcon);

	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");

	Node * imgCm = gamingPanel->getChildByName<Sprite*>("imgCm");

	Vec2 p = seatPanel->convertToNodeSpace(gamingPanel->convertToWorldSpace(imgCm->getPosition()));

	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BET);

	lotteryIcon->runAction(Sequence::create(
		Spawn::create(EaseExponentialOut::create(MoveTo::create(0.3f, p)), ScaleTo::create(0.3f, 0.4), nullptr),
		CallFunc::create([=](){
		betAmount->setString(Comm::GetFloatShortStringFromInt64(roundBet));
		lotteryIcon->removeFromParent();
		gamingPanel->setVisible(true);
	})
		, nullptr));
	
}

void DzPlayer::updateChip(int chip){
	this->chip = chip;
	betAmount2->setString(Comm::GetFloatShortStringFromInt64(chip));
}

void DzPlayer::seatPlayer(bool seating){
	isSeat = seating;
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	seatPanel->setVisible(seating);
}

void DzPlayer::leave()
{


	this->roomPlayer = nullptr;
	seatPlayer(false);
}

void DzPlayer::clearSeat()
{
	this->roomPlayer = nullptr;
	seatPlayer(false);
}

void DzPlayer::updatePlayerInfo(RoomPlayer* roomPlayer){
	if (roomPlayer == nullptr) return;
	this->roomPlayer = roomPlayer;

	
	this->avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(roomPlayer->avatar, avatarIndex));
	this->name->setString(roomPlayer->nickname);

	//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType
	//updateChip(type->init_chip);


	
	seatPlayer(true);//坐着

	updateStatus(roomPlayer->status);
	

}


void DzPlayer::setAvatarIndex(short index)
{
	avatarIndex = index;
}

void DzPlayer::startBeting(BetBegin *betBegin)
{//开始投注中

	if (betBegin == nullptr) return;
	betAmount2->setString(Language::getStringByKey("Think"));
	//SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_THINK);
	round = betBegin->round_num;

	//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType
	int interval_time = betBegin->end_time - TimeControl::GetInstance()->GetServerTime();
	//int betSecond = type->bet_seconds;
	int betSecond = DZControl::GetInstance()->getBetSeconds();

	int redTime = betSecond * 1 / 4;
	int yellowTime = betSecond * 1 / 2;
	waitProgress->stopAllActions();
	waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/green.png"));

	waitProgress->setPercentage(interval_time * 100 / betSecond);

	if (interval_time > yellowTime){
		Action* action = Sequence::create(CCProgressTo::create(interval_time - yellowTime, 50), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/yellow.png"));
		}), CCProgressTo::create(yellowTime - redTime, 25), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		}), CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			//over
			setBetVisible(false);
		}), NULL);
		waitProgress->runAction(action);
	}
	else if (interval_time > redTime)
	{
		
		waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/yellow.png"));
		Action* action = Sequence::create(CCProgressTo::create(yellowTime - redTime, 25), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		}), CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			//over
			setBetVisible(false);
		}), NULL);
		waitProgress->runAction(action);
	}
	else{
		
		progressSprite->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		Action* action = Sequence::create(CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			// over
			setBetVisible(false);
		}), NULL);
		CCProgressTo *to = CCProgressTo::create(20, 0);
		waitProgress->runAction(to);
	}
	
}


void DzPlayer::setBetVisible(bool is)
{
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");
	gamingPanel->setVisible(is);
}

void DzPlayer::updateRound(int round){
	if (this->roomPlayer == nullptr) return;
	
	if (round != 0){ // 不为第0轮（手牌盲注轮）
		playBetToPoolAction();
		this->roundBet = 0;
		CCLOG("DzPlayer::updateRound(int round)  this->roundBet = 0;");
	}
	this->round = round;
}


void DzPlayer::playBetToPoolAction()
{
	// bet 收集动画 
	if (bet <= 0 || roundBet <= 0) return;

	int addPoolBet = this->roundBet;
	//动画效果
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");
	Node * imgCm = gamingPanel->getChildByName<Sprite*>("imgCm");


	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	Vec2 p = UI->convertToNodeSpace(gamingPanel->convertToWorldSpace(imgCm->getPosition()));

	lotteryIcon->setPosition(p);
	

	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	UI->addChild(lotteryIcon);

	Vec2 bankerPoint = UI->convertToNodeSpace(ccp(960, 620));

	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_COLLECT);
	lotteryIcon->runAction(Sequence::create(
		DelayTime::create(0.4f),
		CallFunc::create([=](){
		gamingPanel->setVisible(false);
	}),
		Spawn::create(MoveTo::create(0.4f, bankerPoint), ScaleTo::create(0.4f, 0.4), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();
		//抛出自定义事件  
		Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("UpdateGameBetPool", String::create(toString(addPoolBet)));
	})
		, nullptr));
}


void DzPlayer::stopBeting(){
	waitProgress->setPercentage(0);
	waitProgress->stopAllActions();
}

bool DzPlayer::isCallEnd()
{
	/*if (this->roomPlayer != nullptr && betStatus == TPParticipantStatus::ALL_IN){
		return true;
	}
	return isGameCallEnd;*/

	return betStatus != TPParticipantStatus::FOLD_;
}

void DzPlayer::setGameStart()
{
	name->stopAllActions();
	this->bet = 0;
	this->roundBet = 0;
	CCLOG("this->roundBet = 0;  this->roundBet = 0;");
	this->isGameCallEnd = false;

	betStatus = -1;
	betAction = -1;

	if (isSeat) //坐了人
	{
		updateStatus(roomPlayer->status);
		
	}
}

void DzPlayer::flyCards(float delay,float interval)
{
	SoundControl::PlayPokerEffect();
	Sprite * pokerBack = Sprite::createWithSpriteFrameName("SmallCardB1.png");//背面
	Size size = Director::getInstance()->getVisibleSize();
	UI->addChild(pokerBack,2);
	pokerBack->setScale(0.2);
	pokerBack->setVisible(false);
	pokerBack->setPosition(UI->convertToNodeSpace(ccp(size.width / 2, size.height / 2)));
	pokerBack->setOpacity(100);

	Vec2  p = card_left->getPosition();
	if (card_left->isVisible()){
		p = card_right->getPosition();
	}
	
	pokerBack->runAction(Sequence::create(
		DelayTime::create(delay),
		CallFunc::create([=](){
		   pokerBack->setVisible(true);
	    }),
		Spawn::create(EaseExponentialOut::create(MoveTo::create(0.5f, p)), EaseExponentialOut::create(ScaleTo::create(0.5f, card_left->getScale())), FadeIn::create(0.4f), nullptr),
		CallFunc::create([=](){
		pokerBack->removeFromParent();

		if (card_left->isVisible()){
			card_right->setVisible(true);
			card_right->setSpriteFrame("SmallCardB1.png");
		}
		else{
			card_left->setVisible(true);
			card_left->setSpriteFrame("SmallCardB1.png");
			UI->stopAllActions();
			UI->runAction(Sequence::createWithTwoActions(DelayTime::create(interval),CallFunc::create([=](){flyCards(0,0);})));
		}
		
	}), nullptr));

}

void DzPlayer::playFlipCards(Node* pokerFront, float delay )
{

	//float orbitTime = 0.1f;
	//pokerFront->setVisible(false);
	//Sprite * pokerBack = Sprite::createWithSpriteFrameName("SmallCardB1.png");
	//pokerBack->setScale(card_left->getScale());
	//pokerBack->setPosition(pokerFront->getPosition());

	//pokerFront->getParent()->addChild(pokerBack);

	//CCOrbitCamera* orbitBack = CCOrbitCamera::create(orbitTime, 1, 0, 0, 90, 0, 0);
	//
	//pokerBack->runAction(CCSequence::create(DelayTime::create(delay),orbitBack, CCHide::create(), CCCallFunc::create([=](){
	//	pokerFront->runAction(CCSequence::create(CCShow::create(), CCOrbitCamera::create(orbitTime, 1, 0, 270, 90, 0, 0), CallFunc::create([=](){
	//		pokerBack->removeFromParent();
	//
	//	}), NULL));
	//}), NULL));
}

void DzPlayer::win()
{
	//显示 组合

	//名字效果
	name->stopAllActions();
	name->runAction(Sequence::create(
		Repeat::create(Sequence::create(FadeOut::create(0.6), FadeIn::create(0.6), nullptr),3), 
		CallFunc::create([=](){
		if (this->roomPlayer){
			name->setString(roomPlayer->nickname);
		}
	}), nullptr));
}

void DzPlayer::playBetCollectAction(){
	//动画效果
	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");

	Vec2 p = seatPanel->convertToNodeSpace(ccp(960, 620));

	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	lotteryIcon->setPosition(p);
	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	seatPanel->addChild(lotteryIcon);

	Vec2 p2 = waitProgress->getPosition();

	int resultChip = chip;
	lotteryIcon->setVisible(false);

	
	lotteryIcon->runAction(Sequence::create(
		DelayTime::create(2.0f),
		CallFunc::create([=](){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_COLLECT);
		lotteryIcon->setVisible(true);
		Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("UpdateGameBetPool", String::create(toString(-bonus)));
	    }),
		Spawn::create(MoveTo::create(0.6f, p2), ScaleTo::create(0.6f, 0.4), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();

		TextBMFont *addText = TextBMFont::create(StringUtils::format("+%d", bonus), "font/yellow_add.fnt");
		addText->setPosition(betAmount2->getPosition());
		betAmount2->getParent()->addChild(addText);
		addText->setPositionY(addText->getPositionY() - 100);
		addText->setScale(0.8);

		addText->runAction(Sequence::create(
			MoveBy::create(0.8f, ccp(0, 100)),
			FadeOut::create(0.2f),
			CallFunc::create([=](){
			addText->removeFromParent();
			betAmount2->setString(Comm::GetFloatShortStringFromInt64(resultChip));
			CCLOG("betAmount2->setString:%d", resultChip);
		})
			, nullptr));

	})
		, nullptr));
}

void DzPlayer::finishGame()
{
	

	Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
	Layout* gamingPanel = seatPanel->getChildByName<Layout*>("gaming_panel");
	gamingPanel->setVisible(false);
	betAmount->setString(toString(0));
	card_left->setVisible(false);
	card_right->setVisible(false);

	waitProgress->stopAllActions();
	waitProgress->setPercentage(0);
	card_left->stopAllActions();
	card_right->stopAllActions();

	Node* pokerTypePic = seatPanel->getChildByName("pokerTypePic");
	if (pokerTypePic){
		pokerTypePic->removeFromParent();
	}
	card_left->getParent()->removeChildByName("light_1");
	card_right->getParent()->removeChildByName("light_2");

	name->stopAllActions();

	if (this->roomPlayer == nullptr) return;

	name->setString(roomPlayer->nickname);
	name->setColor(ccc3(255, 255, 255));

    
    updateStatus(this->roomPlayer->status);
}

void DzPlayer::updateResult(PlayerResult* result)
{
	if (result == nullptr)return;

	playBetToPoolAction(); //播放 到底池动画

	CCLOG("DzPlayer pokeType :%d", result->poker_type);
	handCards = result->hand_cards;
	bonus = result->bonus;
	chip = result->chip;

	if (isCallEnd()){ //跟注到底

		string cardType = handCards.substr(0, 2);
		card_left->setSpriteFrame("SmallCard" + cardType + ".png");
		cardType = handCards.substr(2, 2);
		card_right->setSpriteFrame("SmallCard" + cardType + ".png");
		
		playFlipCards(card_left);
		playFlipCards(card_right,0.1f);
	}

	if (result->isWiner){
		win();//赢钱处理
	}
	else{
		if (this->roomPlayer){
			name->setString(roomPlayer->nickname);
		}
	}

	if (bonus > 0)
	{
		playBetCollectAction();
	}
	else
	{
		betAmount2->setString(Comm::GetFloatShortStringFromInt64(result->chip));
	}
}

void DzPlayer::setPos(short pos){
	this->pos = pos;

	if (pos == 2 || pos == 7){
		avatarIndex = 2;
	}
	else if (pos == 3 || pos == 6){
		avatarIndex = 1;
	}
	else{
		avatarIndex = 0;
	}
}

short DzPlayer::getPos()
{
	return this->pos;
}

string DzPlayer::getHandCards()
{
	return handCards;
}

void DzPlayer::validatePoker(int type,map<int,bool>& pokerMap, bool light)
{
	if (!pokerMap.empty()&& type != -1){
		betAmount2->setString(Language::getStringByKey(PokerType[type].c_str()));
		if (light){
			Vec2 p = card_left->getPosition();
			Layout* seatPanel = UI->getChildByName<Layout*>("seat_panel");
			Sprite* pokerTypePic = Sprite::create(StringUtils::format("dz/room/poker_type_%d.png", type));

			Node* node = Node::create();
			node->setName("pokerTypePic");
			node->setAnchorPoint(ccp(0.5, 1));
			seatPanel->addChild(node);
			node->setPosition(ccp(p.x + card_left->getContentSize().width*0.15, p.y - card_left->getContentSize().height*0.15 - 30));

			Sprite* pokerTypeBg = Sprite::create("dz/room/poker_type_bg.png");
			node->addChild(pokerTypeBg);
			node->addChild(pokerTypePic);

			Action* action = Sequence::create(
				DelayTime::create(0.4f),
				CallFunc::create([=](){
				for (auto item : pokerMap){
					if (item.first == 1){
						//第一张牌高亮
						string lightName = StringUtils::format("dz/room/win_light.png", item.first);
						Sprite *light = Sprite::create(lightName);
						light->setName("light_1");
						light->setScale(3.0 / 4);
						card_left->getParent()->addChild(light);
						light->setPosition(card_left->getPosition());

					}
					else if (item.first == 2){
						//第二张牌高亮
						string lightName = StringUtils::format("dz/room/win_light.png", item.first);
						Sprite *light = Sprite::create(lightName);
						light->setName("light_2");
						light->setScale(3.0 / 4);
						card_right->getParent()->addChild(light);
						light->setPosition(card_right->getPosition());
					}
					else{
						//do nothings
					}
				}
			}), nullptr);
			UI->stopAllActions();
			UI->runAction(action);
		}
	}
}

OperaterPanel::~OperaterPanel()
{
	if (betAddPanel){
		delete betAddPanel;
	}
	if (handPanel){
		delete handPanel;
	}
	
}

void OperaterPanel::injectUI(Node * UI)
{
	this->UI = UI;
	this->autoIndex = 0;

	

	gamingPanel = UI->getChildByName<Layout*>("gaming_panel");
	betAmount = gamingPanel->getChildByName<Text*>("lbAmount");
	betAmount->setString(toString(0));

	user_panel = UI->getChildByName<Layout*>("user_panel");

	avatar = user_panel->getChildByName<Sprite*>("spAvatar");
	name = user_panel->getChildByName<Text*>("name");
	
	betAmount2 = user_panel->getChildByName<Text*>("lbAmount2");
	betAmount2->setZOrder(1);
	progressSprite = user_panel->getChildByName<Sprite*>("progress");

	waitProgress = CCProgressTimer::create(progressSprite);
	waitProgress->setPosition(progressSprite->getPosition());
	user_panel->removeChild(progressSprite);

	progress_bg = Sprite::create("dz/room/gray.png");
	user_panel->addChild(progress_bg);
	progress_bg->setPosition(progressSprite->getPosition());

	user_panel->addChild(waitProgress);
	progress_font = Sprite::create("dz/room/progress_font.png");
	user_panel->addChild(progress_font);
	progress_font->setPosition(progressSprite->getPosition());


	tip = user_panel->getChildByName<Text*>("tip");

	waitProgress->setType(kCCProgressTimerTypeRadial);
	waitProgress->setPercentage(100);
	waitProgress->setReverseProgress(true);
	
	waitMyselfPanel = UI->getChildByName<Node*>("waitMyself");
	waitOthersPanel = UI->getChildByName<Node*>("waitOthers");

	btn1 = waitMyselfPanel->getChildByName<Button*>("btn1");
	btn2 = waitMyselfPanel->getChildByName<Button*>("btn2");
	btn3 = waitMyselfPanel->getChildByName<Button*>("btn3");
	btn4 = waitOthersPanel->getChildByName<CheckBox*>("btn4");
	btn5 = waitOthersPanel->getChildByName<CheckBox*>("btn5");
	btn6 = waitOthersPanel->getChildByName<CheckBox*>("btn6");

	

	betAddPanel = new BetAddPanel;
	betAddPanel->injectUI(UI->getChildByName<Layout*>("add_panel"));
	
	
	handPanel = new HandPanel;
	handPanel->injectUI(UI->getChildByName<Layout*>("hand_panel"));

	
	btn1->addClickEventListener([=](Ref *ref){
		//请求弃牌操作
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		betAct(BETACTION::FOLD);
	});
	
	btn2->addClickEventListener([=](Ref *ref){
		//跟注，请求过牌
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		betAct(BETACTION::CALL);
	});
	
	btn3->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (betAddPanel->isVisible())
		{
			//已经点开再点
			//addPanel 还原
			betAct(BETACTION::RAISE, betAddPanel->getBet()-this->roundBet);
			betAddPanel->setVisible(false);
			//请求 加注
			
		}
		else
		{
			
			betAddPanel->setVisible(true);
			//更改数字
		}
		//加注
		
	});

	btn4->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (this->autoIndex != 1){
			autoSelect(1);
		}
		else{
			this->autoIndex = 0;
		}
	});

	btn5->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (this->autoIndex != 2){
			autoSelect(2);
		}
		else{
			this->autoIndex = 0;
		}
	});

	btn6->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (this->autoIndex != 3){
			autoSelect(3);
		}
		else{
			this->autoIndex = 0;
		}
	});


	btnSit = UI->getChildByName<Button*>("btn_sit");

	btnSit->setVisible(false);
	btnSit->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		int pos = DZControl::GetInstance()->getOneNotSitPos();
		if (pos == 0){
			Tips("not exist remain sit");
			return;
		}

		if (BillControl::GetInstance()->GetBalanceData(0)->balance < DZControl::GetInstance()->getInitChip()){
			CCLOG(" money is not enough!");
			Tips(StringUtils::format(Language::getStringByKey("ConditionEnterDZPK"), DZControl::GetInstance()->getInitChip()));
			return;
		}

		if (!Api::Game::mo_dzpk_sit(pos)){
			Tips("mo_dzpk_sit net error");
		};//坐到位置

	});

	this->roomPlayer = nullptr;
	this->round = 0;
	this->addCount = 0;
	this->roundBet = 0;
	this->autoIndex = 0;
	this->isGaming = false;

	btn1->setPressedActionEnabled(true);
	btn2->setPressedActionEnabled(true);
	btn3->setPressedActionEnabled(true);

	user_panel->setVisible(false);
}

void OperaterPanel::showAddPanel()
{
	

	betAddPanel->setVisible(true);
}

void OperaterPanel::cancelAuto(int autoIndex)
{
	if (this->autoIndex == 0) return;
	CheckBox* btn = waitOthersPanel->getChildByName<CheckBox*>(StringUtils::format("btn%d", autoIndex + 3));
	btn->setSelected(false);
	this->autoIndex = 0;
}

void OperaterPanel::flyCards(float delay,float interval)
{
	SoundControl::PlayPokerEffect();
	Sprite * pokerBack = Sprite::createWithSpriteFrameName("SmallCardB1.png");//背面
	Size size = Director::getInstance()->getVisibleSize();
	UI->getParent()->addChild(pokerBack,1);
	pokerBack->setScale(0.2);
	pokerBack->setPosition(size.width/2,size.height/2);
	pokerBack->setVisible(false);

	pokerBack->runAction(Sequence::create(
		DelayTime::create(delay),
		CallFunc::create([=](){
		pokerBack->setVisible(true);
	   }),
		Spawn::create(EaseExponentialOut::create(MoveTo::create(0.5f, avatar->getPosition())), EaseExponentialOut::create(ScaleTo::create(0.5f, 0.3)), FadeIn::create(0.4f), nullptr),
		CallFunc::create([=](){
		 pokerBack->removeFromParent();

		 if (interval != 0){
			 

			 UI->runAction(Sequence::createWithTwoActions(DelayTime::create(interval), CallFunc::create([=](){flyCards(0, 0); })));
		 }

	    }), nullptr));
}

//验证 跟的状态
void OperaterPanel::validateCallBet(int call_bet)
{
	if (this->roomPlayer == nullptr||!this->isGaming) return; //没有坐下

	betAddPanel->setVisible(false);//隐藏加注面板

	callBet = call_bet;

	btn3->setEnabled(false);
	btn3->setVisible(false);

	for (auto child : btn2->getChildren()){
		child->setVisible(false);
	}

	for (auto child : btn5->getChildren()){
		child->setVisible(false);
	}
	
	int curTime = TimeControl::GetInstance()->getCurrentUsecTime();
	//CCLOG("this->roundBet:%d\ call_bet:%d ,time:%d", this->roundBet, call_bet, curTime);

	if (this->roundBet >= call_bet){ //可让牌，加注，可自动让牌，可跟任何注，更改按钮显示
		
		btn2->getChildByName<Sprite*>("label_let_pass")->setVisible(true);
		btn5->getChildByName<Sprite*>("label_let_pass")->setVisible(true);
		
		betAddPanel->setMinBet(DzPokerUtils::getMinChip(chip+this->roundBet, call_bet )); //对方跟之后，更新加注信息

		if (addCount < MAX_ROUND_BET_NUM)
		{ //超过3次之后取消 加注按钮
			btn3->setEnabled(true);
			btn3->setVisible(true);
		}
	}
	else{
		if (this->autoIndex == 2){ //取消 默认选择的让牌或者跟,数额变了需要用户重新选择
			
			cancelAuto(this->autoIndex);
		}

		int plus = call_bet - this->roundBet; // 需要加多少

		if (this->chip <= plus)
		{//钱不够 显示all in
			
			btn2->getChildByName<Sprite*>("all")->setVisible(true);
			btn5->getChildByName<Sprite*>("all")->setVisible(true);
			
			btn3->setEnabled(false);
			btn3->setVisible(false);
		}
		else{ //拥有的携带金额大于 增加的

			btn2->getChildByName<Sprite*>("call")->setVisible(true);
			btn5->getChildByName<Sprite*>("call")->setVisible(true);

			Text * gen =  btn2->getChildByName<Text*>("gen");
			Text * gen2 = btn5->getChildByName<Text*>("gen");

			gen->setVisible(true);
			gen2->setVisible(true);
			gen->setString(Comm::GetStringFromInt64(plus));
			gen2->setString(Comm::GetStringFromInt64(plus));

			betAddPanel->setMinBet(DzPokerUtils::getMinChip(chip + this->roundBet, call_bet));
			if (addCount < MAX_ROUND_BET_NUM)
			{
				btn3->setEnabled(true);
				btn3->setVisible(true);
			}
		}
		
	}

}



void OperaterPanel::betAct(int action,int bet){
	
	switch (action)
	{
	case BETACTION::FOLD:
	{
		Api::Game::mo_dzpk_tp_bet(round, BETACTION::FOLD, bet);
		// 动画弃牌
	}
		break;
	case BETACTION::CALL:
	{
		Api::Game::mo_dzpk_tp_bet(round, BETACTION::CALL, bet);
	}
		break;
	case BETACTION::RAISE:
	{
		Api::Game::mo_dzpk_tp_bet(round, BETACTION::RAISE, bet);
	}
		break;

	default:
		break;
	}

	stopBeting();
	
}

void OperaterPanel::updateStatus(int status)
{
	switch (status)
	{
	case TPParticipantStatus::GAMING:
	{
		UI->setVisible(true);
	}
		break;
	case TPParticipantStatus::LEAVE_ROOM:
	case TPParticipantStatus::LEAVE:
	{
		
		//leave();//离开
		
	}
		break;
	default:
		break;
	}
}

void OperaterPanel::updateAction(int status,int action)
{
	betStatus = status;
	betAction = action;
	switch (action)
	{
	case BETACTION::CALL:
	{
		tip->setString(Language::getStringByKey("CALL"));
		if (status == TPParticipantStatus::ALL_IN){
			betAmount2->setString("ALL_IN");
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_ALLIN);
			disableOperator();
		}
		else{
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CALL);
			
		}
	}
		break;
	case BETACTION::RAISE:
	{
		addCount++; //加注次数增加
		tip->setString(Language::getStringByKey("RAISE"));
		if (status == TPParticipantStatus::ALL_IN){
			betAmount2->setString("ALL_IN");
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_ALLIN);
			disableOperator();
		}
		else{
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RAISE);
		}
	}
		break;
	case  BETACTION::FOLD:
	{
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_FOLD);
		tip->setString(Language::getStringByKey("FOLD"));
		disableOperator();
		handPanel->hide();
	}
		break;
	case BETACTION::BIG_BLIND:{
		tip->setString(Language::getStringByKey("BIG_BLIND"));
	}
		break;
	case BETACTION::SMALL_BLIND:{
		tip->setString(Language::getStringByKey("SMALL_BLIND"));
	}
		break;
	case BETACTION::CHECK:{
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CHECK);
		tip->setString(Language::getStringByKey("CHECK"));
	}
		break;

	
		break;
	default:
		break;
	}

	
	

}


void OperaterPanel::updateBetInfo(BetInfo* info){
	if (info == nullptr) return;
	
	updateBet(info->bet_amount,info->round_amount);
	updateChip(info->chip);
	updateAction(info->status,info->action);

	//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType];

	betAddPanel->updateGamePoolBet(0);
	betAddPanel->updateChip(info->chip + info->round_amount);
	

	isGameCallEnd = info->action != BETACTION::FOLD && this->round == 3;

	
}



void OperaterPanel::updateBet(int bet,int roundBet){
	if (bet <= 0 || this->bet == bet || roundBet <= 0) return;

	this->bet = bet;
	this->roundBet = roundBet;
	//CCLOG("OperaterPanel::updateBet(int bet,int roundBet) this->roundBet = roundBet;  %d", roundBet);

	int add = bet - this->bet;
	
	//动画效果
	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	lotteryIcon->setPosition(waitProgress->getPosition());
	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	UI->addChild(lotteryIcon);

	Layout* gamingPanel = UI->getChildByName<Layout*>("gaming_panel");

	Node * imgCm = gamingPanel->getChildByName<Sprite*>("imgCm");

	Vec2 p = UI->convertToNodeSpace(gamingPanel->convertToWorldSpace(imgCm->getPosition()));

	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BET);

	lotteryIcon->runAction(Sequence::create(
		Spawn::create(EaseExponentialOut::create(MoveTo::create(0.3f, p)), ScaleTo::create(0.3f, 0.4), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();
		betAmount->setString(Comm::GetFloatShortStringFromInt64(roundBet));
		gamingPanel->setVisible(true);
	})
		, nullptr));

}

void OperaterPanel::updateChip(int chip){
	this->chip = chip;
	
	betAmount2->setString(Comm::GetFloatShortStringFromInt64(chip));
}

void OperaterPanel::leave()
{
	
	this->roomPlayer = nullptr;
	btnSit->setVisible(true);
	gamingPanel->setVisible(false);
	user_panel->setVisible(false);
}

void OperaterPanel::clearSeat()
{
	this->roomPlayer = nullptr;
	user_panel->setVisible(false);
}


bool OperaterPanel::isMe()
{
	if (roomPlayer == nullptr) return false;
	return roomPlayer->user_id == UserControl::GetInstance()->GetUserData()->user_id;
}

void OperaterPanel::updatePlayerInfo(RoomPlayer* roomPlayer)
{
	if (roomPlayer == nullptr) return;

	this->roomPlayer = roomPlayer;

	if (!isMe()){
		handPanel->hide();
		disableOperator();
	}

	btnSit->setVisible(false);
	this->avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(roomPlayer->avatar, 0));
	this->name->setString(roomPlayer->nickname);
	

	user_panel->setVisible(true);
	user_panel->setCascadeOpacityEnabled(true);

	user_panel->runAction(FadeIn::create(0.5f));
	
	tip->setString("");

	updateStatus(roomPlayer->status);
	
}


void OperaterPanel::startBeting(BetBegin *betBegin)
{//开始投注中
	if (betBegin == nullptr) return;
	round = betBegin->round_num;

	if (this->autoIndex != 0){ //检测是否有选自动

		switch (this->autoIndex)
		{
		case 1:
		{
			if (callBet - roundBet > 0){//需要跟，自动放弃
				Api::Game::mo_dzpk_tp_bet(betBegin->round_num, BETACTION::FOLD, 0);//自动弃牌
			}
			else
			{
				Api::Game::mo_dzpk_tp_bet(betBegin->round_num, BETACTION::CALL, 0);//让牌，相当于跟注0
			}

		}
			break;
		case 2:
		{
			Api::Game::mo_dzpk_tp_bet(betBegin->round_num, BETACTION::CALL, 0);//跟或让 一样
		}
			break;
		case 3:
		{
			Api::Game::mo_dzpk_tp_bet(betBegin->round_num, BETACTION::CALL, 0);//跟任何注
		}
			break;
		default:
			break;
		}

		waitMyselfPanel->setVisible(false);
		waitOthersPanel->setVisible(false);

		return;
	}
	waitMyself();

	// 没有自动就切换自己面板，开始倒计时
	

	DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[1]; // test  roomType
	int interval_time = betBegin->end_time - TimeControl::GetInstance()->GetServerTime();
	int betSecond = type->bet_seconds;

	int redTime = betSecond * 1 / 4;
	int yellowTime = betSecond * 1 / 2;
	waitProgress->stopAllActions();
	waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/green.png"));
	waitProgress->setPercentage(100);

	
	if (interval_time > yellowTime){
		Action* action = Sequence::create(CCProgressTo::create(interval_time - yellowTime, 50), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/yellow.png"));
		}), CCProgressTo::create(yellowTime - redTime, 25), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		}), CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			//over
			setBetVisible(false);
		}), NULL);
		waitProgress->runAction(action);
	}
	else if (interval_time > redTime)
	{
		waitProgress->setPercentage(50);
		waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/yellow.png"));
		Action* action = Sequence::create(CCProgressTo::create(yellowTime - redTime, 25), CallFunc::create([&](){
			waitProgress->getSprite()->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		}), CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			//over
			setBetVisible(false);
		}), NULL);
		waitProgress->runAction(action);
	}
	else{
		waitProgress->setPercentage(25);
		progressSprite->setTexture(Director::getInstance()->getTextureCache()->addImage("dz/room/red.png"));
		Action* action = Sequence::create(CCProgressTo::create(redTime, 0), CallFunc::create([&](){
			// over
			setBetVisible(false);
		}), NULL);
		CCProgressTo *to = CCProgressTo::create(20, 0);
		waitProgress->runAction(to);
	}


	waitMyself();//显示自己等待面板
}

void OperaterPanel::setBetVisible(bool is)
{
	Layout* gamingPanel = UI->getChildByName<Layout*>("gaming_panel");
	gamingPanel->setVisible(is);
}

bool OperaterPanel::isCallEnd()
{
	if (this->roomPlayer != nullptr && betStatus == TPParticipantStatus::ALL_IN){
		return true;
	}
	return isGameCallEnd;
}

void OperaterPanel::setGameStart()
{
	

	tip->stopAllActions();
	tip->setString("");

	waitProgress->stopAllActions();
	waitProgress->setPercentage(0);

	name->stopAllActions();
	isDisableOperator = false;//解禁
	waitOthersPanel->setVisible(false);
	waitMyselfPanel->setVisible(false);
	betAddPanel->setVisible(false);

	handPanel->setGameStart();
	setBetVisible(false);
	
	cancelAuto(this->autoIndex);

	this->autoIndex = 0;
	this->round = 0;
	this->addCount = 0;
	this->roundBet = 0;
	this->isGaming = false;
	this->isGameCallEnd = false;
	this->bet = 0;
	betStatus = -1;
	betAction = -1;

	betAddPanel->updateChip(this->chip + this->roundBet);//重置这一轮开始的chip
	
}

string OperaterPanel::getHandCards()
{
	return handCards;
}

void OperaterPanel::updateHandCards(string handCards)
{
	if (handCards.empty())return;
	this->handCards = handCards;
	handPanel->show(handCards);
}

void OperaterPanel::autoSelect(int autoIndex)
{
	
	this->autoIndex = autoIndex;
	for (int i = 1; i <= 3; i++)
	{

		CheckBox* btn = waitOthersPanel->getChildByName<CheckBox*>(StringUtils::format("btn%d", i + 3));


		if (i != autoIndex){
			btn->setSelected(false);
		}

	}

}
//下完注后, 没轮到自己，可自动 让或弃 自动让牌（没到自己有人加注 改成跟）  跟任何注
void OperaterPanel::waitMyself()
{
	if (!isMe()||isDisableOperator||!isGaming) return;

	waitMyselfPanel->setVisible(true);
	waitOthersPanel->setVisible(false);
}

void OperaterPanel::waitOthers()
{
	if (!isMe() || isDisableOperator || !isGaming) return;

	betAddPanel->setVisible(false);
	waitMyselfPanel->setVisible(false);
	waitOthersPanel->setVisible(true);
}

void OperaterPanel::disableOperator()
{
	isDisableOperator = true;
	waitMyselfPanel->setVisible(false);
	waitOthersPanel->setVisible(false);
}

void OperaterPanel::stopBeting(){
	waitProgress->setPercentage(0);
	waitProgress->stopAllActions();
	waitMyselfPanel->setVisible(false);
	waitOthersPanel->setVisible(false);
}

void OperaterPanel::setPos(short pos)
{
	this->pos = pos;
}

short OperaterPanel::getPos()
{
	return this->pos;
}


void OperaterPanel::playFlipCards(Node* pokerFront, float delay)
{

}


void OperaterPanel::playBetCollectAction(){
	//动画效果
	Layout* gamingPanel = UI->getChildByName<Layout*>("gaming_panel");
	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");

	Vec2 p = UI->convertToNodeSpace(ccp(960, 620));
	lotteryIcon->setPosition(p);
	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	UI->addChild(lotteryIcon);

	Vec2 p2 = waitProgress->getPosition();

	int resultChip = chip;
	lotteryIcon->setVisible(false);

	
	lotteryIcon->runAction(Sequence::create(
		DelayTime::create(2.0f),
		CallFunc::create([=](){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_COLLECT);
		lotteryIcon->setVisible(true);
		Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("UpdateGameBetPool", String::create(toString(-bonus)));
	    }),
		Spawn::create(MoveTo::create(0.6f, p2), ScaleTo::create(0.6f, 0.4), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();
		TextBMFont *addText = TextBMFont::create(StringUtils::format("+%d", bonus), "font/yellow_add.fnt");
		addText->setPosition(betAmount2->getPosition());
		betAmount2->getParent()->addChild(addText);
		addText->setPositionY(addText->getPositionY() - 100);
		addText->setScale(0.8);
		addText->runAction(Sequence::create(
			MoveBy::create(0.8f, ccp(0, 100)),
			FadeOut::create(0.2f),
			CallFunc::create([=](){
			addText->removeFromParent();
			betAmount2->setString(Comm::GetFloatShortStringFromInt64(resultChip));
		})
			, nullptr));

	})
		, nullptr));
}

void OperaterPanel::win()
{
	// 效果 闪动结果
	tip->stopAllActions();
	CCAction *action = RepeatForever::create(Sequence::create(FadeIn::create(0.6), FadeOut::create(0.6), nullptr));
	tip->runAction(action);


	//you_win 显示
	Sprite * youWin = UI->getChildByName<Sprite*>("you_win");
	youWin->setVisible(true);
	youWin->setOpacity(100);
	youWin->setScale(0.1f);
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_WIN);

	youWin->runAction(Sequence::create(
		Spawn::create(EaseBackOut::create(ScaleTo::create(0.4f, 1)),
		FadeIn::create(0.4f), nullptr),
		DelayTime::create(1.0f),
		FadeOut::create(0.6f),
		CallFunc::create([=]{
		youWin->setVisible(false);
	}),
		nullptr));
}

void OperaterPanel::leaveTipHandler()
{
	int ante = DZControl::GetInstance()->getAnte();
	int init_chip = DZControl::GetInstance()->getInitChip();
	if (chip < ante)//没钱自动离场
	{

		if (DZControl::GetInstance()->isSNG){
			LayerSNGTip * sngTip = LayerSNGTip::create();
			sngTip->set(Language::getStringByKey("SNSNoMoneyLeaveTip"), "label_confirm");
			sngTip->handler = [=](){
				DZControl::GetInstance()->closeRoom();
			};
			sngTip->show();
		}
		else{
			if (BillControl::GetInstance()->GetBalanceData(0)->balance <init_chip){
				ShowTip(Language::getStringByKey("NotEnoughShouldBuy"), [=](){

					LayerMall * mall = LayerMall::create(); //临时创建释放 avoid texture memory overflow
					mall->setName("LayerMall");

					auto runningScene = Director::getInstance()->getRunningScene();
					if (runningScene){
						runningScene->addChild(mall, 2);
					}
					OpenWithAlert(mall);
				});
			}
			else{
				ShowTip(Language::getStringByKey("EnoughShouldSit"), [=](){
					CCLOG(" go to click sit");
				}, nullptr, true);
			}
		}

	}
}


void OperaterPanel::finishGame()
{
	if (roomPlayer == nullptr) return;
	
	if (roomPlayer->status == TPParticipantStatus::LEAVE)
	{
		leaveTipHandler();
		leave();
	}
	

	//Layout* gamingPanel = UI->getChildByName<Layout*>("gaming_panel");
	//gamingPanel->setVisible(false);
	betAmount->setString(toString(0));

	waitProgress->stopAllActions();
	waitProgress->setPercentage(0);

	if (roomPlayer){
		name->setString(roomPlayer->nickname);
	}
	name->setColor(ccc3(255, 255, 255));
	tip->setOpacity(255);
	tip->stopAllActions();
	tip->setString("");
	handPanel->hide();
	disableOperator();
}

void OperaterPanel::updateResult(PlayerResult* result)
{
	if (result == nullptr)return;
	playBetToPoolAction(); //播放 到底池动画
	//隐藏操作面板
	waitOthersPanel->setVisible(false);
	waitMyselfPanel->setVisible(false);
	betAddPanel->setVisible(false);

	handCards = result->hand_cards;
	bonus = result->bonus;
	chip = result->chip;

	if (result->isWiner&&isMe()){
		win();//赢钱处理
	}

	if (bonus > 0)
	{
		playBetCollectAction();
	}
	else{
		betAmount2->setString(Comm::GetFloatShortStringFromInt64(result->chip));
	}
	
}


void OperaterPanel::validatePoker(int type, map<int, bool>& pokerMap, bool light)
{
	if (type!=-1&& !pokerMap.empty()){
		tip->setString(Language::getStringByKey(PokerType[type].c_str()));
		if (light){
			handPanel->lightCard(pokerMap);
		}
		//提示
	}
}


int OperaterPanel::getBet()
{
	return bet;
}

void OperaterPanel::playBetToPoolAction(){
	// bet 收集动画 
	if (bet <= 0 || roundBet <= 0) return;

	int addPoolBet = this->roundBet;

	//动画效果
	Layout* gamingPanel = UI->getChildByName<Layout*>("gaming_panel");
	Node * imgCm = gamingPanel->getChildByName<Sprite*>("imgCm");


	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	Vec2 p = UI->convertToNodeSpace(gamingPanel->convertToWorldSpace(imgCm->getPosition()));

	lotteryIcon->setPosition(p);
	lotteryIcon->setScale(0.2);
	lotteryIcon->setCascadeOpacityEnabled(true);

	UI->addChild(lotteryIcon);

	Vec2 bankerPoint = UI->convertToNodeSpace(ccp(960, 620));

	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_COLLECT);
	lotteryIcon->runAction(Sequence::create(
		DelayTime::create(0.4f),
		CallFunc::create([=](){
		gamingPanel->setVisible(false);
	}),
		Spawn::create(MoveTo::create(0.4f, bankerPoint), ScaleTo::create(0.4f, 0.4), nullptr),
		CallFunc::create([=](){
		lotteryIcon->removeFromParent();
		//抛出自定义事件  
		Director::getInstance()->getEventDispatcher()->dispatchCustomEvent("UpdateGameBetPool", String::create(toString(addPoolBet)));
	})
		, nullptr));
}



void OperaterPanel::updateRound(int round){
	if (this->roomPlayer == nullptr) return;

	if (round != 0){ // 不为第0轮（手牌盲注轮）

		playBetToPoolAction();

		this->roundBet = 0;
		callBet = 0;//跟注 重置
		this->addCount = 0;

		betAddPanel->reset();
		//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType
		int ante = DZControl::GetInstance()->getAnte();
		betAddPanel->setMinBet(DzPokerUtils::getMinChip(chip+this->roundBet, ante));

		cancelAuto(this->autoIndex);
	}

	this->round = round;
}

void OperaterPanel::hideHand()
{
	handPanel->hide();
}

void HandPanel::injectUI(Node * UI)
{
	this->UI = UI;
	card_left = UI->getChildByName<Sprite*>("card_left");
	card_right = UI->getChildByName<Sprite*>("card_right");

	left_light = UI->getChildByName<Sprite*>("left_light");
	right_light = UI->getChildByName<Sprite*>("right_light");
	two_light = UI->getChildByName<Sprite*>("two_light");


	isHandHide = true;
}

void HandPanel::lightCard(map<int, bool>& pokerMap)
{
	if (pokerMap.empty())return;
	bool lightLeft = pokerMap.find(1) != pokerMap.end();
	bool lightRight = pokerMap.find(2) != pokerMap.end();
	
	if (lightLeft&&lightRight){
		two_light->setVisible(true);
	}
	else if (lightLeft){
		left_light->setVisible(true);
	}
	else if (lightRight){
		right_light->setVisible(true);
	}
}

bool HandPanel::isHide()
{
	return isHandHide;
}

void HandPanel::hide()
{
	isHandHide = true;
	UI->setCascadeOpacityEnabled(true);
	UI->runAction(Sequence::create(
		Spawn::create(MoveBy::create(0.4f, ccp(0, 0)), FadeOut::create(0.4f), nullptr),
		CallFunc::create([=](){
		UI->setVisible(false);
	}), nullptr));
}

void HandPanel::show(string handCards)
{
	if (handCards.empty())return;
	isHandHide = false;
	// 效果
	UI->setVisible(true);
	string left = handCards.substr(0, 2);
	string right = handCards.substr(2, 2);
	card_left->setSpriteFrame("SmallCard" + left + ".png");
	card_right->setSpriteFrame("SmallCard" + right + ".png");

	UI->setCascadeOpacityEnabled(true);
	UI->setOpacity(20);
	UI->setPositionY(UI->getPositionY() - 200);
	UI->runAction(Sequence::create(
		Spawn::create(MoveBy::create(0.4f, ccp(0,200)), FadeIn::create(0.4f), nullptr),
		CallFunc::create([=](){
	}), nullptr));
}

void HandPanel::setGameStart()
{
	UI->setVisible(false);
	
	UI->getChildByName<Text*>("shadow")->setVisible(false);

	card_left->setOpacity(255);

	card_right->setOpacity(255);

	two_light->setVisible(false);
	left_light->setVisible(false);
	right_light->setVisible(false);

	Sprite* shadow_left = UI->getChildByName<Sprite*>("shadow_left");
	if (shadow_left){
		shadow_left->setVisible(false);
	}

	Sprite* shadow_right = UI->getChildByName<Sprite*>("shadow_right");
	if (shadow_right){
		shadow_right->setVisible(false);
	}
	
}

void HandPanel::throwCards()
{
	card_left->setOpacity(180);

	card_right->setOpacity(180);

	UI->getChildByName<Text*>("shadow")->setVisible(true);
}


void CardPublicPanel::injectUI(Node * UI)
{
	this->UI = UI;
	tip = UI->getChildByName<Text*>("tip");
	imgCm = UI->getChildByName<Node*>("imgCm");
	imgBgAmount = UI->getChildByName<Node*>("imgBgAmount");
	poolLabel = UI->getChildByName<Text*>("pool_label");
	left_time = UI->getChildByName<Text*>("left_time");
	imgCm->setVisible(false);
	imgBgAmount->setVisible(false);
	poolLabel->setVisible(false);
	poolLabel->setString(Language::getStringByKey("BetPool"));
	
	bet = 0;
	gamePoolBet = 0;
	isPlayFlip = false;
	isPlayNumAction = false;
	fee = 1;
	//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType

	float fee_rate = DZControl::GetInstance()->getFeeRate();
	//if (type != nullptr){
		//fee = 1 + type->fee_rate;
		fee = 1 + fee_rate;
	//}

	Director::getInstance()->getEventDispatcher()->addCustomEventListener("UpdateGameBetPool", [=](EventCustom* evt){
		onUpdateGamePoolBet(evt);
	});

	finishGame();
	tip->setString(Language::getStringByKey("WaitStart"));
}

void CardPublicPanel::playNumAction()
{
	if (bet == gamePoolBet || isPlayNumAction)return;

	isPlayNumAction = true;
	
	Director::getInstance()->getScheduler()->schedule([=](float tm){
	  if (bet == gamePoolBet){
		  isPlayNumAction = false;
		  Director::getInstance()->getScheduler()->unschedule("playNumAction", UI);
	  }
	  else{
		  int cx = gamePoolBet - bet;
		  int step = cx > 0 ? 1 : -1; //暂时步长度用1;
		  if (abs(cx) > 30){
			  bet = gamePoolBet - 30 * step;
		  }

		  bet = bet + step;
		  updateBet(bet);
	  }
	}, UI,0, false, "playNumAction");
}

void CardPublicPanel::updateBet(int bet){
	if (bet <= 0){
		tip->setString("");
		imgCm->setVisible(false);
		imgBgAmount->setVisible(false);
		poolLabel->setVisible(false);
	}
	else{
		imgCm->setVisible(true);
		imgBgAmount->setVisible(true);
		poolLabel->setVisible(true);
		tip->setString(Comm::GetFloatShortStringFromInt64(bet));
	}
}



void CardPublicPanel::onUpdateGamePoolBet(EventCustom* evt)
{
	String* addBetStr = (String*)evt->getUserData();
	if (addBetStr == nullptr) return;
	
	int addBet = addBetStr->intValue();
	if (addBet == 0){
		return;
	}
	
	if (addBet < 0){
		addBet = addBet*fee;
	}
	gamePoolBet = gamePoolBet + addBet;
	if (gamePoolBet < 0){
		gamePoolBet = 0;
	}

	imgCm->setVisible(true);
	imgBgAmount->setVisible(true);
	poolLabel->setVisible(true);

	playNumAction();

	//更新底池
	//tip->setTextColor(ccc4(30, 225, 132,255));
}

string CardPublicPanel::getPublicCards()
{
	return publicCards;
}



void CardPublicPanel::updateCards(RoundInfo * info)
{
	if (info == nullptr || info->round_num <1) return;
	
	
	publicCards = info->public_cards;
	switch (info->round_num)
	{
		case 1:
		{

			for (int i = 0; i < 3; i++)
			{
				Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", i+1));
				if (!card->isVisible()){
					string cardType = publicCards.substr(i * 2, 2);
					card->setSpriteFrame("SmallCard" + cardType + ".png");
					card->setVisible(true);
					playFlipCards(card, 0.02*i);
				}
			}
	
		}
			break;
		case 2:
		{
			for (int i = 0; i < 3; i++)
			{
				Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", i + 1));
				if (!card->isVisible()){
					string cardType = publicCards.substr(i * 2, 2);
					card->setSpriteFrame("SmallCard" + cardType + ".png");
					card->setVisible(true);
				}
			}

			string cardType = publicCards.substr(3 * 2, 2);
			Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", 3 + 1));
			card->setSpriteFrame("SmallCard" + cardType + ".png");
			card->setVisible(true);
			playFlipCards(card);
		}
			break;
		case 3:
		{
			for (int i = 0; i < 4; i++)
			{
				
				Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", i + 1));
				if (!card->isVisible()){
					string cardType = publicCards.substr(i * 2, 2);
					card->setSpriteFrame("SmallCard" + cardType + ".png");
					card->setVisible(true);
				}
			}

			string cardType = publicCards.substr(4 * 2, 2);
			Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", 4 + 1));
			card->setSpriteFrame("SmallCard" + cardType + ".png");
			card->setVisible(true);
			playFlipCards(card);
		}
			break;
		default:
			break;
	}

	
}

void CardPublicPanel::playFlipCards(Node* pokerFront, float delay )
{

	float orbitTime = 0.1f;
	pokerFront->setVisible(false);
	Sprite * pokerBack = Sprite::createWithSpriteFrameName("SmallCardB1.png");
	pokerBack->setScale(0.4);
	pokerBack->setPosition(pokerFront->getPosition());

	pokerFront->getParent()->addChild(pokerBack);

	CCOrbitCamera* orbitBack = CCOrbitCamera::create(orbitTime, 1, 0, 0, 90, 0, 0);

	pokerBack->runAction(CCSequence::create(DelayTime::create(delay), orbitBack, CCHide::create(), CCCallFunc::create([=](){
		pokerBack->removeFromParent();
		pokerFront->runAction(CCSequence::create(CCShow::create(), CCOrbitCamera::create(orbitTime, 1, 0, 270, 90, 0, 0), CallFunc::create([=](){
			
		}), NULL));
	}), NULL));
}


void CardPublicPanel::validatePoker(int type, map<int, bool>& pokerMap, bool light)
{

	if (!hasValidate&&type != -1 && !pokerMap.empty()){
		hasValidate = true;
		if (light){

			for (int i = 1; i <= 5; i++){
				Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", i));
				if (pokerMap.find(i + 2) != pokerMap.end()){

					Sprite *light = Sprite::create("dz/room/win_light.png");
					card->getParent()->addChild(light);
					light->setPosition(card->getPosition());
					cardExtras.push_back(light);
					light->setOpacity(0);
				}
				else{
					Sprite * shadow = Sprite::create("dz/room/shadow2.png");
					card->getParent()->addChild(shadow);
					shadow->setPosition(card->getPosition());
					cardExtras.push_back(shadow);
					shadow->setOpacity(0);
				}
			}

			Action* action = Sequence::create(
				DelayTime::create(0.4f),
				CallFunc::create([=](){

				for (auto node : cardExtras){
					node->runAction(FadeIn::create(0.2f));
				}

			}), nullptr);
			//UI->stopAllActions();
			UI->runAction(action);
		}
	}
}

void CardPublicPanel::finishGame()
{
	for (int i = 1; i <= 5; i++)
	{
		Sprite * card = UI->getChildByName<Sprite*>(StringUtils::format("card_%d", i));
		card->setVisible(false);
	}
	actionFlips.clear();

	for (auto node : cardExtras){
		node->removeFromParent();
	}
	cardExtras.clear();

	imgCm->setVisible(false);
	imgBgAmount->setVisible(false);
	poolLabel->setVisible(false);

	tip->setString(Language::getStringByKey("WaitStart"));

	/*if (DZControl::GetInstance()->getCurrentPlayersNum() > 1){
		int ctime = DZControl::GetInstance()->getBetSeconds() / 2;
		left_time->setString(toString(ctime));
		left_time->setVisible(true);
		startLeftTime(ctime);
	}*/
}


void CardPublicPanel::startLeftTime(int seconds)
{
	left = seconds;
	
	Director::getInstance()->getScheduler()->schedule([=](float dt){
		timeLeftHandler(dt);
	}, UI, 1.0f, false, "playTimeLeftAction");
}

void CardPublicPanel::timeLeftHandler(float dt)
{
	if (left <= 0){
		left_time->setString("");
		left_time->setVisible(false);
		Director::getInstance()->getScheduler()->unschedule("playTimeLeftAction", UI);
		return;
	}
	left = left - 1;
	left_time->setString(toString(left));
}

void CardPublicPanel::setGameStart()
{

	tip->setTextColor(ccc4(255, 255, 255,255));
	tip->setString("");
	imgCm->setVisible(false);
	imgBgAmount->setVisible(false);
	poolLabel->setVisible(false);
	hasValidate = false;
	bet = 0;
	gamePoolBet = 0;
}

CardPublicPanel::~CardPublicPanel(){
	Director::getInstance()->getEventDispatcher()->removeCustomEventListeners("UpdateGameBetPool");
	actionFlips.clear();
	cardExtras.clear();
}

void SceneDzRoom::updateSitInfos()
{
	for (auto item : dzPlayerMap){
		Player* player = item.second;
		if (player->roomPlayer){
			player->clearSeat();
		}
	}

	map<int, RoomSitInfo*> sitInfos = DZControl::GetInstance()->sitInfos;
	for (auto item:sitInfos)
	{
		updateSitInfo(item.second);
	}
}

void SceneDzRoom::updateSitInfo(RoomSitInfo* sitInfo)
{
	if (sitInfo == nullptr) return;
	

	if (DZControl::GetInstance()->RoomPlayers.find(sitInfo->user_id) == DZControl::GetInstance()->RoomPlayers.end())
	{
		CCLOG("updateSitInfo, can`t find roomPlayer, plz check");
		return;
	}

	int ui_pos = DZControl::GetInstance()->getUIPos(sitInfo->pos);

	if (dzPlayerMap.find(ui_pos) == dzPlayerMap.end())
	{
		CCLOG("updateSitInfo error cant find  pos:%d,ui_pos:%d",sitInfo->pos,ui_pos);
		return;
	}
	Player *player = dzPlayerMap[ui_pos];

	RoomPlayer* roomPlayer = DZControl::GetInstance()->RoomPlayers[sitInfo->user_id];//roomPlayer会在下轮比赛开始前检测清除
	if (player != nullptr)
	{
		player->updatePlayerInfo(roomPlayer);
	}

	if (roomPlayer->user_id == UserControl::GetInstance()->GetUserData()->user_id)
	{
		btnStand->setEnabled(roomPlayer->status != TPParticipantStatus::LEAVE);
		btnStand->setBright(roomPlayer->status != TPParticipantStatus::LEAVE);

		if (!isForceLeave &&roomPlayer->status == TPParticipantStatus::LEAVE){ //没钱强制离场时候,会提示后处理
			operaterPanel->leave();
		}
		else{
			isForceLeave = false;
		}
	}
}

//第二个参数是参与者
void SceneDzRoom::updateRoomInfo(RoundInfo* roundInfo, vector<BetInfo*> &vec, BetBegin* betBegin)
{
	//roundInfo  大盲，小盲，公共牌 轮数
	//游戏已经开始的 处理
	updateRoundInfo(roundInfo);
	int totalPoolBet = 0;
	for (BetInfo* betInfo : vec){ //对于 每个坐着的
		Player* player = getPlayer(betInfo->user_id);
		if (player){
			updateBetInfo(betInfo);
			totalPoolBet = totalPoolBet + betInfo->bet_amount - betInfo->round_amount;
		}
	}
	
	updateBetBegin(betBegin);
	cardPublicPanel->updateBet(totalPoolBet);
}



void SceneDzRoom::updateBetBegin(BetBegin* betBegin)
{
	//更新新的投注人
	if (betBegin == nullptr) return;

	Player *player = getPlayer(betBegin->user_id);
	if (player != nullptr)
	{
		if (betingPlayer != nullptr)
		{
			betingPlayer->stopBeting();
		}

		betingPlayer = player;

		if (player->getPos() != 1){
			operaterPanel->waitOthers();
		}
		player->startBeting(betBegin);
	}

}

void SceneDzRoom::updateFlopInfo(string info)
{
	if (info.empty()) return; 
	//更新flopinfo 手牌
	operaterPanel->updateHandCards(info);
}

void SceneDzRoom::updateRoundInfo(RoundInfo * info)
{
	if (info == nullptr) return;
	//更新roundInfo
	cardPublicPanel->updateCards(info);

	for (auto item : dzPlayerMap){
		item.second->updateRound(info->round_num);
	}

	if (info->round_num != 0)//
	{
		map<int, bool> pokerMap;
		int type = -1;
		DzPokerUtils::validatePokerCards(operaterPanel->getHandCards(), info->public_cards, &type, pokerMap);
		operaterPanel->validatePoker(type, pokerMap,false);
	}

}

Player* SceneDzRoom::getPlayer(int user_id)
{
	if (user_id == 0) return nullptr;

	if (DZControl::GetInstance()->RoomPlayers.find(user_id) == DZControl::GetInstance()->RoomPlayers.end()){
		return nullptr;
	}

	RoomPlayer *roomPlayer = DZControl::GetInstance()->RoomPlayers[user_id];

	if (roomPlayer->pos == 0){
		CCLOG("sitInfo pos :0");
		return nullptr;
	}
	int ui_pos = DZControl::GetInstance()->getUIPos(roomPlayer->pos);

	if (dzPlayerMap.find(ui_pos) == dzPlayerMap.end())
	{
		return nullptr;
	}
	return  dzPlayerMap[ui_pos];
}

void SceneDzRoom::updateBetInfo(BetInfo* info)
{   
	if (info == nullptr) return;
	Player *player = getPlayer(info->user_id);
	if (player != nullptr)
	{
		int addBet = info->bet_amount - player->getBet();
		player->updateBetInfo(info);

		//不是弃牌的时候，验证跟牌状态
		if (info->status != TPParticipantStatus::FOLD_)
		{
			operaterPanel->validateCallBet(info->round_amount);
		}
	}
}

void SceneDzRoom::updateGameResults(vector<PlayerResult*>& vec)
{
	for (PlayerResult* playerResult : vec){
		updateGameResult(playerResult);
	}
	//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType

}

void SceneDzRoom::finishGame()
{
	DZControl::GetInstance()->gameProcess = GameProcess::IS_WAITING; //推送游戏结果后进入等待

	cardPublicPanel->finishGame();

	for (auto item : dzPlayerMap){
		item.second->finishGame();
	}
}

void SceneDzRoom::updateGameResult(PlayerResult* playerResult)
{
	if (playerResult == nullptr) return;

	if (betingPlayer != nullptr)
	{
		betingPlayer->stopBeting();//停掉
	}

	Player *player = getPlayer(playerResult->user_id);

	if (player != nullptr)
	{
		player->updateResult(playerResult);

		if (player->isCallEnd()){//一直跟到结束

			int type = -1;
			map<int, bool> pokerMap;
			DzPokerUtils::validatePokerCards(playerResult->hand_cards, cardPublicPanel->getPublicCards(), &type, pokerMap);
			

			if (playerResult->isWiner){ //赢家的显示公共牌
				player->validatePoker(type, pokerMap, true);
				cardPublicPanel->validatePoker(type, pokerMap, true);
			}
			else{
				player->validatePoker(type, pokerMap, false);
			}
		}

		if (player->roomPlayer!=nullptr&&player->roomPlayer->user_id == UserControl::GetInstance()->GetUserData()->user_id){

			//DzpkRoomType* type = DZControl::GetInstance()->RoomTypeInfos[DZControl::GetInstance()->gameRoomType]; //  roomType
			operaterPanel->updateChip(playerResult->chip);
			if (playerResult->chip < DZControl::GetInstance()->getAnte()){
				isForceLeave = true;
			}
			
		}
	}

	
}


void SceneDzRoom::setGameStart(bool gameStart)
{
	this->stopAllActions();
	//sfinishGame();

	betingPlayer = nullptr;//当前投注人
	for (auto item : dzPlayerMap){
		item.second->setGameStart();
    }
	operaterPanel->setGameStart();
	cardPublicPanel->setGameStart();
	

	if (!gameStart) return; //做默认初始化

		

	DZControl::GetInstance()->gameProcess = GameProcess::IS_GAMING; //推送游戏开始后进入游戏
	

	CCLOG("---------------new game start--------------");
	
	map<int, RoomSitInfo*> sitInfos = DZControl::GetInstance()->sitInfos;
	vector<Player*> seatPlayers;
	for (auto item : sitInfos)
	{
		RoomSitInfo * sitInfo = item.second;
		Player *player = getPlayer(sitInfo->user_id);
		if (player != nullptr&&player->roomPlayer!=nullptr)
		{
			seatPlayers.push_back(player);
		}
	}
	//-------------------发牌动画-----------------------
	for (int i = 0, j = seatPlayers.size(); i < j; i++){
		Player* player = seatPlayers[i];
		player->flyCards(i*0.12f, (seatPlayers.size()-1)*0.12f);
	}
	if (operaterPanel->roomPlayer != nullptr){
		operaterPanel->isGaming = true;
	}
}


void SceneDzRoom::updatePlayerChip(ChipInfo *chipInfo)
{
	Player * player = getPlayer(chipInfo->user_id);

	if (player){
		player->updateChip(chipInfo->chip);
	}
}


bool SceneDzRoom::init()
{
	if (!Scene::init())
	{
		return false;
	}
	isForceLeave = false;
	auto listener = EventListenerKeyboard::create();
	listener->onKeyReleased = CC_CALLBACK_2(SceneDzRoom::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

	RootNode = CSLoader::createNode("dz/SceneDzRoom.csb");
	if (!RootNode)
		return false;
	addChild(RootNode);
	Layout * panelPlayers = RootNode->getChildByName<Layout*>("panelPlayers");
	if (!panelPlayers)
		return false;

	// 组装玩家UI
	for (size_t i = 2; i <= 7; i++)
	{
		Layout* dzPlayerUI = panelPlayers->getChildByName<Layout*>(StringUtils::format("panelPlayer%d", i));
		DzPlayer* dzPlayer = new DzPlayer;
		dzPlayer->injectUI(dzPlayerUI);
		dzPlayer->setPos(i);
		dzPlayerMap[i] = dzPlayer;
	}
	
	operaterPanel = new OperaterPanel;
	Layout* operatorPanel = RootNode->getChildByName<Layout*>("operator_panel");
	operaterPanel->injectUI(operatorPanel);
	operaterPanel->setPos(1);
	dzPlayerMap[1] = operaterPanel;

	cardPublicPanel = new CardPublicPanel;
	cardPublicPanel->injectUI(RootNode->getChildByName<Layout*>("public_panel"));

	


	leaveTip = RootNode->getChildByName<Text*>("leave_tip");
	leaveTip->setVisible(false);

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistSmallCard.plist");

	setGameStart(false);//设置默认状态

	infoPanel = operatorPanel->getChildByName<Layout*>("info_panel");

	auto btnBuy = infoPanel->getChildByName<Button*>("btnBuy");
	spAddBg = infoPanel->getChildByName<Sprite*>("spAddBg");

	btnBuy->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerMall * mall = LayerMall::create(); //临时创建释放 avoid texture memory overflow
		mall->setName("LayerMall");
		this->addChild(mall, 2);
		OpenWithAlert(mall);
	});

	txtCoins = infoPanel->getChildByName<TextBMFont*>("txtCoins");

	//初始化时间控件
	lbTime=RootNode->getChildByName<Text*>("lbTime");
	if(!lbTime)
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
	schedule(schedule_selector(SceneDzRoom::updateCurrentTime), 1.0f);
    
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
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(1260, 0)),
				CallFunc::create([=](){
				isBarHide = false;
			}),
				nullptr));
		}
	});

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);
	touchListener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!GameHideBar->getBoundingBox().containsPoint(touch->getLocation()))
		
		if (!isBarHide) {
			isBarHide = true;
			NodeGameHideControl->runAction(Sequence::create(MoveTo::create(0.5f, vec + Vec2(-1260, 0)),
				CallFunc::create([=](){
				btnHide->setVisible(!btnHide->isVisible());
			}),
				nullptr));
		}
		
		return true;
	};

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(touchListener, GameHideBar);

	btnStand = GameHideBar->getChildByName<Button*>("btn_stand");
	btnStand->addClickEventListener([=](Ref* ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (Api::Game::mo_dzpk_sit(0)){

		};//坐到0位置,相当于离开
	});
	btnStand->setPressedActionEnabled(true);
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
	auto btHelp = GameHideBar->getChildByName<Button*>("btHelp");
	if (!btHelp)
		return false;
	btHelp->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerDzHelp * help = this->getChildByName<LayerDzHelp*>("LayerDzHelp");
		if (!help){
			LayerDzHelp * help = LayerDzHelp::create(); //临时创建释放 avoid texture memory overflow
			help->setName("LayerDzHelp");
			this->addChild(help, 1);
			OpenWithAlert(help);
		}
	});
	btHelp->setPressedActionEnabled(true);
	auto btQuit = GameHideBar->getChildByName<Button*>("btQuit");
	if (!btQuit)
		return false;
	btQuit->addClickEventListener([=](Ref *ref){ 
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN); 
		
		if (DZControl::GetInstance()->isSNG){
			

			LayerSNGTip * sngTip = LayerSNGTip::create();
			

			sngTip->set(Language::getStringByKey("SNSLeaveTip"), "label_get_mall",false);
			sngTip->handler = [=](){
				Director::getInstance()->popScene();
			};
			sngTip->show();

		}
		else{
			ShowTip(Language::getStringByKey("WantLeaveRoom"), [=](){
				Director::getInstance()->popScene();
			});
		}
	});
	btQuit->setPressedActionEnabled(true);
	auto btSettings = GameHideBar->getChildByName<Button*>("btSettings");
	if (!btSettings)
		return false;
	btSettings->setPressedActionEnabled(true);
	btSettings->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerSetting * setting = this->getChildByName<LayerSetting*>("LayerSetting");
		if (!setting){
			LayerSetting * setting = LayerSetting::create(); //临时创建释放 avoid texture memory overflow
			setting->setName("LayerSetting");
			this->addChild(setting, 1);
			OpenWithAlert(setting);
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
		this->addChild(scene, 1);
	});

	//邀请
	if (!(btInvite = GameHideBar->getChildByName<Button*>("btInvite")))
		return false;
	btInvite->setEnabled(false);
	btInvite->setBright(false);
	btInvite->setPressedActionEnabled(true);

	//显示聊天窗口
	panelChat = RootNode->getChildByName<Layout*>("panelChat");
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
	btSend->setPressedActionEnabled(true);

	auto btnChat = RootNode->getChildByName<Button*>("btn_chat");
	btnChat->setTouchEnabled(false);
	auto btnChat_t = RootNode->getChildByName<Button*>("btn_chat_t");
	chatWarn = btnChat->getChildByName<Sprite*>("warn");
	chatWarn->setVisible(false);
	btnChat_t->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		chatWarn->setVisible(false);
		panelChat->setVisible(!panelChat->isVisible());
	; });

	//积分兑换
	btn_add_score = RootNode->getChildByName<Button*>("btn_add_score");

	btn_add_score->addClickEventListener([=](Ref* sender){
		LayerDzAddScore * layerDzAddScore = LayerDzAddScore::create();
		this->addChild(layerDzAddScore);
	});

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneDzRoom::UpdateBalance), "UpdateBalance", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneDzRoom::onCurrentIntervalTaskFinish), "CurrentIntervalTaskFinish", nullptr);
	UpdateBalance(this);
	
	// 刷新服务器时间，再次同步游戏时间
	TimeControl::GetInstance()->RequestServerTime(0.0f);

	return true;
}


void SceneDzRoom::enableExchange(bool enable)
{
	if (!enable){
		btn_add_score->setEnabled(false);
		btn_add_score->setVisible(false);
		return;
	}
	else{
		BalanceData *data = BillControl::GetInstance()->GetBalanceData(DZControl::GetInstance()->gameRoomType);
		if (data && data->balance > 0){
			btn_add_score->setPressedActionEnabled(true);
			btn_add_score->setVisible(true);
			btn_add_score->setVisible(true);
		}
	}
}


void SceneDzRoom::joinGame(){
    
	this->roomType = DZControl::GetInstance()->getCurrentRoomType();

	DZROOMKIND kind = DZControl::GetInstance()->roomKind;


	if (DZControl::GetInstance()->roomKind == DZROOMKIND::DZ_GAME_SINGLE_CREATE_ROOM)
	{

		btInvite->setEnabled(true); //开放邀请按钮
		btInvite->setBright(true);

		btInvite->addClickEventListener([=](Ref *ref)
		{
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

			funcInvite();
		});

		funcWaitInvite();//弹出等待
	}

}

void SceneDzRoom::funcInvite()
{
	LayerFriendInvite * layerFriendInvite = this->getChildByName<LayerFriendInvite*>("LayerFriendInvite");
	if (!layerFriendInvite){
		layerFriendInvite = LayerFriendInvite::create(); 
		layerFriendInvite->setName("LayerFriendInvite");
		layerFriendInvite->setFrom(FROMPAGE::WAIT_INVITE_PAGE);
		this->addChild(layerFriendInvite, 2);
	}
}

void SceneDzRoom::funcWaitInvite()
{
	LayerInviteDzWait * layerInviteDzWait = this->getChildByName<LayerInviteDzWait*>("LayerInviteDzWait");
	if (!layerInviteDzWait){
		layerInviteDzWait = LayerInviteDzWait::create(); 
		layerInviteDzWait->setName("LayerInviteDzWait");
		this->addChild(layerInviteDzWait, 2);
		OpenWithAlert(layerInviteDzWait);
	}
}

void SceneDzRoom::onCurrentIntervalTaskFinish(Ref *pSender)
{
	btnAward->stopAllActions();
	btnAward->setBright(false);
	btnAward->setEnabled(false);
	intervalTaskLayout->setVisible(false);

	//播放 金币飞的动画
	String* msg = (String*)pSender;

	Array *array = msg->componentsSeparatedByString("_");
	String* num = (String*)array->objectAtIndex(1);

	if (!num||array->count() < 2 || num->intValue() == 1)//
	{
		return;
	}
	intervalTaskWaiting = true;
	intervalTaskLayout->setVisible(true);

	Sprite *lotteryIcon = Sprite::create("common/many_coin.png");
	lotteryIcon->setPosition(intervalTaskLayout->getPosition());
	lotteryIcon->setScale(0.6);
	lotteryIcon->setCascadeOpacityEnabled(true);

	RootNode->addChild(lotteryIcon);
	
	int add = num->intValue();
	

    lotteryIcon->runAction(Sequence::create(
	Spawn::create(CCEaseExponentialOut::create(MoveTo::create(0.8f, spAddBg->getPosition())),nullptr),
			CallFunc::create([=](){
			lotteryIcon->removeFromParent();
			Tips(StringUtils::format(Language::getStringByKey("AddChipsTip"), add));
		})
	, nullptr));
}



void SceneDzRoom::closeRoom()
{
	Director::getInstance()->popScene();
}


SceneDzRoom::~SceneDzRoom()
{
	if (cardPublicPanel){
		delete cardPublicPanel;
	}

	for (auto item : dzPlayerMap){
		delete item.second;
	}

	if (DZControl::GetInstance()->isSNG){
		Api::Game::mo_dzpk_leave_sng_game();
	}
	else{
		Api::Game::mo_dzpk_leave();
	}
	LayerLoading::Close();
	DZControl::GetInstance()->isSNG = false;

	DZControl::GetInstance()->setDzpkRoomView(nullptr);
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
}
void SceneDzRoom::onEnter()
{
	Scene::onEnter();

	TimeControl::IS_PLAYING_GAME_IN_ROOM = true;

	SoundControl::PlayMusic(BGM::BGM_POKER);

	DZControl::GetInstance()->setDzpkRoomView(this);

	if (DZControl::GetInstance()->isSNG){

		LayerSNGTip * sngTip = LayerSNGTip::create();

		SNGRoomType *roomType = DZControl::GetInstance()->SngRoomTypeInfos[DZControl::GetInstance()->current_sng_type_id];
	
		string desc = DZControl::GetInstance()->getCurrencyTypeStr(roomType->apply_currency_type);

		string condition_text = StringUtils::format("%d张%s", roomType->apply_cost, desc.c_str());
		string tip = StringUtils::format(Language::getStringByKey("SngSignUpSucc"), condition_text.c_str());

		sngTip->set(tip, "label_confirm");
		sngTip->handler = [=](){

		};
		sngTip->show();
	}

	addGuide();
}
void SceneDzRoom::onExit()
{
	
	TimeControl::IS_PLAYING_GAME_IN_ROOM = false;
	Scene::onExit();
}
void SceneDzRoom::sendChatMsg()
{
	if (Api::Game::mo_dzpk_chat(_editBoxMsg->getText()))
		_editBoxMsg->setText("");
}

void SceneDzRoom::updateChatData(DzChatData* chatData)
{
	//成功
	//检测是否是有座玩家聊天消息，如果是，则显示聊天泡泡
	
	Player* player = getPlayer(chatData->user_id);
	if (player!= nullptr)
	{
		//_sitChatControl->ShowChat(sid, chatData.msg);// 气泡
	}

	RoomPlayer * roomPlayer = DZControl::GetInstance()->RoomPlayers[chatData->user_id];
	
	if (roomPlayer == nullptr) return;

	if (!panelChat->isVisible()){ //没有打开聊天窗口时 显示提醒
		chatWarn->setVisible(true);
	}

	//将聊天信息加入聊天列表
	float widthTotal = _listChat->getContentSize().width;
	string nickname;
	Color4B color;
	if (roomPlayer->user_id == 0)
	{
		//系统消息
		nickname = "系统";
		color = Color4B::RED;
	}
	else if (roomPlayer->user_id == UserControl::GetInstance()->GetUserData()->user_id)
	{
		//本人消息
		nickname = "我";
		color = Color4B::GREEN;
	}
	else
	{
		nickname = roomPlayer->nickname;
		color = Color4B(0x00, 0xcc, 0xff, 0xff);
	}
	Text *lbNickname = Text::create(nickname + ":", "", 28);
	lbNickname->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	lbNickname->setTextColor(color);
	float widthNickname = lbNickname->getContentSize().width;
	Text *lbMsg = Text::create(chatData->msg, "", 28);
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


void SceneDzRoom::editBoxReturn(EditBox* editBox)
{
	if (_editBoxMsg == editBox)
		sendChatMsg();
}


void SceneDzRoom::UpdateBalance(Ref *ref)
{
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		BalanceData *balance = BillControl::GetInstance()->GetBalanceData(0);
		if (!balance)
			return;
		int amount = (int64_t)balance->balance;
		txtCoins->setString(Comm::GetFloatShortStringFromInt64(amount));
	});
}



void SceneDzRoom::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
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

			if (DZControl::GetInstance()->isSNG){
				ShowTip(Language::getStringByKey("SNSLeaveTip"), [=](){
					Director::getInstance()->popScene();
				}, nullptr);
			}
			else{
				ShowTip(Language::getStringByKey("WantLeaveRoom"), [=](){
					Director::getInstance()->popScene();
				});
			}
			
	    }
		
     }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}


void SceneDzRoom::updateCurrentTime(float dt)
{
	unsigned long long timestamp = time(NULL);
	struct tm *ptm = localtime((time_t*)&timestamp);
	char tmp[100] = {0};
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



void SceneDzRoom::addGuide()
{

	if (!GuideControl::GetInstance()->checkInGuide())return;

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N3, "call_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("请点击跟注", ccp(1465, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N3, "call_action", [=](){
		LayerGuideMask::GetInstance()->cleanMask();
		Api::Game::mo_dzpk_tp_bet(1, BETACTION::CALL, 0);//
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N4, this);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N4, "jump_to_next", [=](){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N5, this);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N5, "call_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("请点击让牌按钮", ccp(1465, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N5, "call_action", [=](){
		LayerGuideMask::GetInstance()->cleanMask();
		Api::Game::mo_dzpk_tp_bet(1, BETACTION::CALL, 0);//
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N6, this);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N6, "add_action", [=](){
		operaterPanel->betAddPanel->setVisible(true);
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N7, this);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N6, "add_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("点击加注", ccp(1800, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N7, "add_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("确认加注", ccp(1800, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N7, "add_action", [=](){
		operaterPanel->betAddPanel->setVisible(false);
		Api::Game::mo_dzpk_tp_bet(1, BETACTION::CALL, 0);
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N8, this);
	});



	GuideControl::GetInstance()->injectHander(GUIDESTEP::N8, "jump_to_next", [=](){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N9, this);
	});
	//点击加注
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N9, "add_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("点击加注", ccp(1800, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N9, "add_action", [=](){
		operaterPanel->betAddPanel->setVisible(true);
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N10, this);
	});

	//all in
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N10, "all_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("点击all-in按钮", ccp(1330, 650), false, DIRECTION::RIGHT);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N10, "all_action", [=](){
		operaterPanel->betAddPanel->setPercent(100);
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N11, this);
	});
	//que ren
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N11, "add_arrow", [=](){
		LayerGuideMask::GetInstance()->getTextSay()->setVisible(false);
		LayerGuideMask::GetInstance()->textArrow("确认加注", ccp(1800, 240), true, DIRECTION::BOTTOM);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N11, "add_action", [=](){
		operaterPanel->betAddPanel->setVisible(false);
		Api::Game::mo_dzpk_tp_bet(1, BETACTION::CALL, 0);
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N12, this);
	});

	//que ren
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N12, "add_btn_next", [=](){
		Button* btnNext = RootNode->getChildByName<Button*>("guideNext");
		LayerGuideMask::GetInstance()->setVisible(false);
		ActionInterval *inA2 = (ActionInterval*)Sequence::create
			(
			DelayTime::create(4),
			Show::create(),
			nullptr
			);
		ActionInterval *inA3 = (ActionInterval*)Sequence::create
			(
			DelayTime::create(4),
			Show::create(),
			nullptr
			);
		btnNext->runAction(inA3);
		LayerGuideMask::GetInstance()->runAction(inA2);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N12, "add_action", [=](){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N13, this);
		Button* btnNext = RootNode->getChildByName<Button*>("guideNext");
		btnNext->setVisible(false);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N13, "jump_to_next", [=](){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N14, this);
	});

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N14, "jump_to_next", [=](){
		GuideControl::GetInstance()->finishGuideStep();
		Director::getInstance()->popScene();
	});

	LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N3, this);
}