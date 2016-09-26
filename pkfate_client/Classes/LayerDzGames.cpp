#include "LayerDzGames.h"
#include "cocostudio/CocoStudio.h"
#include "ApiGame.h"
#include "LayerLoading.h"
#include "comm.h"
#include "ui/UIImageView.h"
#include "PKNotificationCenter.h"
#include "ApiBill.h"
#include "cmd.h"
#include "DZControl.h"
#include "SceneDzRoom.h"
#include "BillControl.h"
#include "SoundControl.h"
#include "GuideControl.h"
#include "LayerGuideMask.h"
#include "LayerInviteDzRoom.h"
#include "LayerDuang.h"
#include "LayerSngRule.h"
#include "LayerMall.h"

vector<string> strDZRoomType;
vector<string> strDZRoomTypePath;



SNGRoomItem::SNGRoomItem()
{
	rootNode = CSLoader::createNode("dz/RoomItem.csb");

	rootNode->setPositionX(50);

	master_icon = rootNode->getChildByName<Sprite*>("master_icon");
	sng_label = rootNode->getChildByName<Sprite*>("sng_label");

	condition_text = rootNode->getChildByName<Text*>("condition_text");
	start_time = rootNode->getChildByName<Text*>("start_time");
	start_count = rootNode->getChildByName<Text*>("start_count");
	start_num = rootNode->getChildByName<Text*>("start_num");
	room_num = rootNode->getChildByName<Text*>("room_num");
	room_status = rootNode->getChildByName<Text*>("room_status");


	btn_room_item = rootNode->getChildByName<Button*>("btn_room_item");
	//btn_room_item->setPressedActionEnabled(true);

	btn_room_item->addClickEventListener(CC_CALLBACK_1(SNGRoomItem::btnClickHandler, this));

	btn_room_item->setSwallowTouches(false);

	addChild(rootNode);

	setTouchEnabled(false);
}

void SNGRoomItem::btnClickHandler(Ref * sender)
{

	auto runningScene = Director::getInstance()->getRunningScene();
	LayerSngRule* sngRule = LayerSngRule::create();
	sngRule->handler = [=](){
		enterRoom();
	};

	if (runningScene){
		runningScene->addChild(sngRule);
	}
	
}

void SNGRoomItem::enterRoom()
{
	scheduleOnce([=](float dt){
		DZControl::GetInstance()->current_sng_room_id = sNGRoomStatus->room_id;
		DZControl::GetInstance()->current_sng_type_id = sNGRoomStatus->type_id;

		PKNotificationCenter::getInstance()->postNotification("EnterSNGRoom", String::createWithFormat("%d", sNGRoomStatus->room_id));
		
	}, 0.0f, "enter room");

}

void SNGRoomItem::setProperty(SNGRoomStatus* sNGRoomStatus)
{
	this->sNGRoomStatus = sNGRoomStatus;
	SNGRoomType *roomType = DZControl::GetInstance()->SngRoomTypeInfos[sNGRoomStatus->type_id];
	start_num->setString(toString(roomType->open_players));
	room_num->setString(toString(sNGRoomStatus->room_id));
	string desc = DZControl::GetInstance()->getCurrencyTypeStr(roomType->apply_currency_type);

	condition_text->setString(StringUtils::format("%d张%s", roomType->apply_cost, desc.c_str()));

	if (!sNGRoomStatus->is_running ){
		room_status->setString(StringUtils::format("未进行(%d人)", sNGRoomStatus->players));
	}
	else{
		room_status->setString(StringUtils::format("进行中(%d人)", sNGRoomStatus->players));
	}
	start_count->setString(StringUtils::format("第%d场", sNGRoomStatus->session_num));

	start_time->setString(roomType->open_time);

	string picUrl = StringUtils::format("dz/room/sng_label_%d.png", roomType->reward_amount);
	sng_label->setTexture(Director::getInstance()->getTextureCache()->addImage(picUrl));


	if (roomType->apply_currency_type == CurrencyType::GOLD){

	}
	else if (roomType->apply_currency_type == CurrencyType::GOLD){

	}

	condition_text = rootNode->getChildByName<Text*>("condition_text");
}



SNGRoomItem::~SNGRoomItem()
{

}


// on "init" you need to initialize your instance
bool LayerDzGames::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("dz/LayerDzGames.csb");
	if (!RootNode)
		return false;
	if (!(panelGames = RootNode->getChildByName<Layout*>("panelGames")))
		return false;
	if (!(panelLobby = panelGames->getChildByName<Layout*>("panelLobby")))
		return false;
	if (!(btBack = panelLobby->getChildByName<Button*>("btBack")))
		return false;
	
	if (!(btRoomS = panelLobby->getChildByName<Button*>("btRoomS")))
		return false;
	btRoomS->setTitleText(Language::getStringByKey("Primary"));
	btRoomS->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(DZROOMTYPE::DSmall);
	});
	if (!(btRoomM = panelLobby->getChildByName<Button*>("btRoomM")))
		return false;
	btRoomM->setTitleText(Language::getStringByKey("Middle"));
	btRoomM->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(DZROOMTYPE::DMiddle);
	});
	
	if (!(btRoomB = panelLobby->getChildByName<Button*>("btRoomB")))
		return false;
	btRoomB->setTitleText(Language::getStringByKey("Senior"));
	btRoomB->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(DZROOMTYPE::DBig);
	});

	

	if (!(btNew = panelLobby->getChildByName<Button*>("btNew")))
		return false;
	btNew->setTitleText(Language::getStringByKey("SingleCreate"));
	btNew->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerInviteDzRoom * layerInviteDzRoom = LayerInviteDzRoom::create();
		Director::getInstance()->getRunningScene()->addChild(layerInviteDzRoom,2);
	});

	room_list = panelGames->getChildByName<ScrollView*>("room_list");

	auto room_list_container = room_list->getInnerContainer();

	royal = room_list_container->getChildByName<Button*>("royal");
	normal = room_list_container->getChildByName<Button*>("normal");

	master = room_list_container->getChildByName<Button*>("master");
	elite = room_list_container->getChildByName<Button*>("elite");

	

	normal_big = normal->getChildByName<Text*>("normal_big");
	royal_big = royal->getChildByName<Text*>("royal_big");
	normal_small = normal->getChildByName<Text*>("normal_small");
	royal_small = royal->getChildByName<Text*>("royal_small");
	

	if (!royal || !normal || !normal_small || !royal_small || !royal_big || !normal_big)
		return false;

	royal->addClickEventListener(CC_CALLBACK_1(LayerDzGames::typeSelectHandler, this));
	normal->addClickEventListener(CC_CALLBACK_1(LayerDzGames::typeSelectHandler, this));
	master->addClickEventListener(CC_CALLBACK_1(LayerDzGames::typeSelectHandler, this));
	elite->addClickEventListener(CC_CALLBACK_1(LayerDzGames::typeSelectHandler, this));

	royal->setPressedActionEnabled(true);
	normal->setPressedActionEnabled(true);
	master->setPressedActionEnabled(true);
	elite->setPressedActionEnabled(true);

	enableRoom(false);

	sng_room_panel = RootNode->getChildByName<Layout*>("sng_room_panel");
	sng_room_list = sng_room_panel->getChildByName<ListView*>("sng_room_list");
	sng_room_list->setItemsMargin(650);
	sng_room_panel->setVisible(false);
	
	coin_num = sng_room_panel->getChildByName<Text*>("coin_num");
	gold_num = sng_room_panel->getChildByName<Text*>("gold_num");
	sliver_num = sng_room_panel->getChildByName<Text*>("sliver_num");
	diamond_num = sng_room_panel->getChildByName<Text*>("diamond_num");
	txtVipLevel = sng_room_panel->getChildByName<Text*>("txtVipLevel");
	
	Button* btSnsBack = sng_room_panel->getChildByName<Button*>("btBack");

	btSnsBack->addClickEventListener(CC_CALLBACK_1(LayerDzGames::closeSnsRooms, this));
	btSnsBack->setPressedActionEnabled(true);

	Button* btnBuy = sng_room_panel->getChildByName<Button*>("btnBuy");
	
	btnBuy->addClickEventListener([=](Ref* pSender){
		openMall();
	});

	sng_room_list->setSwallowTouches(false);

	
	this->addChild(RootNode);

	//add button func
	btBack->setPressedActionEnabled(true);
	btBack->addClickEventListener([=](Ref *ref){
		PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::NO)));
		
		LayerDuang::panelStatus();
		LayerDuang::uiGameMenu();
		LayerDuang::dzGamesRoom();
		LayerDuang::bjlGamesRoom();
		LayerDuang::bjlPanelLobby();
		LayerDuang::dzPanelLobby();
	});

	_roomType = (DZROOMTYPE)-1;
	


	
	DZControl::GetInstance()->setDzpkHallView(this);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerDzGames::enterDzpkRoom), "EnterDzpkRoom", NULL);

	PKNotificationCenter::getInstance()->postNotification("ConnectDzpk"); //每次都必须连么？

	
	return true;
}

void LayerDzGames::enableRoom(bool enable)
{

	master->setEnabled(enable);
	elite->setEnabled(enable);

	if (enable){
		LayerLoading::Close(true);
		master->setColor(ccc3(255, 255, 255));
		elite->setColor(ccc3(255, 255, 255));
	}
	else{
		master->setColor(ccc3(137, 137, 137));
		elite->setColor(ccc3(137, 137, 137));
	}

}

void LayerDzGames::openMall()
{
	LayerMall * mall = LayerMall::create(); //临时创建释放 avoid texture memory overflow
	mall->setName("LayerMall");
	this->addChild(mall, 2);
	OpenWithAlert(mall);
}


void LayerDzGames::UpdateBalance(Ref *ref)
{
	BalanceData *dataChip = BillControl::GetInstance()->GetBalanceData(0);
	BalanceData *dataGoldCard = BillControl::GetInstance()->GetBalanceData(1);
	BalanceData *dataSilverCard = BillControl::GetInstance()->GetBalanceData(2);
	BalanceData *dataDiamondCard = BillControl::GetInstance()->GetBalanceData(3);
	BalanceData *dataRMB = BillControl::GetInstance()->GetBalanceData(4);

	if (dataChip){
		coin_num->setString(Comm::GetShortStringFromInt64(dataChip->balance));
	}

	if (dataGoldCard){
		gold_num->setString(Comm::GetShortStringFromInt64(dataGoldCard->balance));
	}
	if (dataSilverCard){
		sliver_num->setString(Comm::GetShortStringFromInt64(dataSilverCard->balance));
	}
	if (dataDiamondCard){
		sng_room_panel->getChildByName("iconDiamondCard")->setVisible(true);
		diamond_num->setString(Comm::GetShortStringFromInt64(dataDiamondCard->balance));
	}
	else{
		sng_room_panel->getChildByName("iconDiamondCard")->setVisible(false);
		diamond_num->setString("");
	}
	
	if (dataRMB){
		txtVipLevel->setString(Comm::GetShortStringFromInt64(
			BillControl::GetInstance()->getBetLevel(dataRMB->balance)));
	}

}

void LayerDzGames::typeSelectHandler(Ref* sender)
{
	map<int, DzpkRoomType *> RoomTypeInfos = DZControl::GetInstance()->RoomTypeInfos;
	if (sender == royal){
		selectEnterRoom(DZControl::GetInstance()->RoomTypeInfos[_roomType + 5]);
	}
	else if (sender == normal){
		selectEnterRoom(DZControl::GetInstance()->RoomTypeInfos[_roomType]);
	}
	else if (sender == master){
		selectSngType(CurrencyType::GOLD); //金卡
	}
	else if (sender == elite){
		selectSngType(CurrencyType::SILVER); //银卡
	}
}


void LayerDzGames::selectSngType(int type)
{
	if (DZControl::GetInstance()->SngRoomTypeInfos.size() <= 0){
		Tips("wait sng room type init");
		return;
	}
	
	DZControl::GetInstance()->gameRoomType = type;
	LayerLoading::Wait();
	if (!Api::Game::mo_sng_apply_start(type)){
		LayerLoading::CloseWithTip(" net error");
	}
}

void LayerDzGames::updateSngRoom(SNGRoomStatus* sNGRoomStatus)
{
	Vector<Node*> vec = sng_room_list->getChildren();
	for (auto item : vec){
		SNGRoomItem * sNGRoomItem = (SNGRoomItem *)item;
		sNGRoomItem->setProperty(sNGRoomStatus);
	}
}


void LayerDzGames::closeSnsRooms(Ref* pSender)
{
	sng_room_panel->setVisible(false);
	sng_room_list->removeAllItems();
	panelGames->setVisible(true);
	Api::Game::mo_dzpk_leave_sng_game();
}


void LayerDzGames::updateSngRooms(map<int, SNGRoomStatus*> & map)
{
	panelGames->setVisible(false);
	if (map.size() <= 0) return;
	sng_room_panel->setVisible(true);
	sng_room_list->removeAllItems();
	for (auto item : map){
		
		SNGRoomStatus* sNGRoomStatus = item.second;
		SNGRoomType *roomType = DZControl::GetInstance()->SngRoomTypeInfos[sNGRoomStatus->type_id];
		if (roomType&&roomType->apply_currency_type == DZControl::GetInstance()->gameRoomType){
			SNGRoomItem * sNGRoomItem = new SNGRoomItem();
			sNGRoomItem->setProperty(sNGRoomStatus);
			sng_room_list->pushBackCustomItem(sNGRoomItem);
		}
	}

	SNGRoomItem * sNGRoomItem = new SNGRoomItem();//添加一个默认的,占格
	sng_room_list->pushBackCustomItem(sNGRoomItem);


	sng_room_list->jumpToLeft();
	sng_room_list->refreshView();
}



void LayerDzGames::enterDzpkRoom(Ref* pSender)
{
	auto scene = SceneDzRoom::create();
	scene->joinGame();
	
	Director::getInstance()->pushScene(TransitionPageTurn::create(0.3f, scene, true));
}


void LayerDzGames::updateRoomsTypes(){
	//LayerDuang::intoDzGamesRoom();
	// 无特定房间 先这样吧。
	showRooms(DZROOMTYPE::DSmall);

	if (DZControl::GetInstance()->roomKind == DZROOMKIND::DZ_GAME_JOIN_CREATE_ROOM){ //直接进入
		PKNotificationCenter::getInstance()->postNotification("TriggerJoinCreateRoom", nullptr);
	}
	else{
		
	}
	if (GuideControl::GetInstance()->checkInGuide()){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N1, Director::getInstance()->getRunningScene());
	}
}


void LayerDzGames::selectEnterRoom(DzpkRoomType* roomType)
{
	if (roomType == nullptr ){
		Tips("room is not init!");
		return;
	} 
	
	if (BillControl::GetInstance()->GetBalanceData(0)->balance < roomType->init_chip){
		CCLOG(" money is not enough!");
	
		Tips(StringUtils::format(Language::getStringByKey("ConditionEnterDZPK"), roomType->init_chip));

		return;
	}

	DZControl::GetInstance()->setRoomInfo(roomType->type_id, DZROOMKIND::DZ_GAME_ROOM);
	
	scheduleOnce([=](float dt){
		PKNotificationCenter::getInstance()->postNotification("JoinGame", String::createWithFormat("%d", roomType->type_id));
	}, 0.0f, "enter room");

}



void LayerDzGames::onEnter()
{
	Layer::onEnter();
	if (DZControl::GetInstance()->RoomTypeInfos.size() > 0)
	{
		showRooms(_roomType);
	}
	UpdateBalance(nullptr);
	addGuide();
}
void LayerDzGames::onExit()
{
	Layer::onExit();
}
LayerDzGames::~LayerDzGames()
{
	
	Api::Game::mo_dzpk_disconnect();
	Api::Game::mo_sng_dzpk_disconnect();
	DZControl::GetInstance()->setDzpkHallView(nullptr);
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}

LayerDzGames::LayerDzGames()
{
}

void LayerDzGames::showRooms(DZROOMTYPE type)
{
	map<int, DzpkRoomType *> RoomTypeInfos = DZControl::GetInstance()->RoomTypeInfos;
	if (RoomTypeInfos.size() <= 0){
		return;
	}

	switch (_roomType)
	{
	case DSmall:
		btRoomS->setEnabled(true);
		btRoomS->setBright(true);
		break;
	case DMiddle:
		btRoomM->setEnabled(true);
		btRoomM->setBright(true);
		break;
	case DBig:
		btRoomB->setEnabled(true);
		btRoomB->setBright(true);
		break;

	}
	_roomType = type;

	switch (_roomType)
	{
	case DSmall:
		btRoomS->setEnabled(false);
		btRoomS->setBright(false);
		
		break;
	case DMiddle:
		btRoomM->setEnabled(false);
		btRoomM->setBright(false);
		
		break;
	case DBig:
		
		btRoomB->setEnabled(false);
		btRoomB->setBright(false);
		break;

	default:
		return;
	}

	DzpkRoomType * roomType = RoomTypeInfos[_roomType + 5];
	if (roomType){
		royal_small->setString(toString(roomType->ante / 2));
		royal_big->setString(toString(roomType->ante));
	}else{
		royal_small->setString("close");
		royal_big->setString("close");
	}
	roomType = RoomTypeInfos[_roomType ];
	if (roomType){
		normal_small->setString(toString(roomType->ante / 2));
		normal_big->setString(toString(roomType->ante));
	}
	else{
		normal_small->setString("close");
		normal_small->setString("close");
	}
}


void LayerDzGames::addGuide()
{
	GuideControl::GetInstance()->startWithType(GAMETYPE::DZPK);

	if (!GuideControl::GetInstance()->checkInGuide())return;


	room_list->getInnerContainer()->runAction(MoveBy::create(0.3f, Vec2(-1280, 0)));

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N1, "jump_to_next", [=](){
		LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N2, Director::getInstance()->getRunningScene());
	});
	
	
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "dz_room_mask", [=](){

		Sprite* roomMask = Sprite::create("guide/room_effect.png");
		LayerGuideMask::GetInstance()->addTo(roomMask);
		roomMask->setName("roomMask");
		roomMask->setPosition(ccp(555, 450));
		
		LayerGuideMask::GetInstance()->textArrow("请点击进入普通场", ccp(570, 890), true, DIRECTION::BOTTOM);
	});


	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "jump_to_dz_room", [=](){
		
		DZControl::GetInstance()->setRoomInfo();

		PKNotificationCenter::getInstance()->postNotification("JoinGame", String::createWithFormat("%d", 1));

	});
	
	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "title_effect", [=](){
		if (RootNode != NULL)
		{
			Layout *layoutT = RootNode->getChildByName<Layout*>("guideTitleEffect");
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

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "room_effect_l", [=](){
		if (RootNode != NULL)
		{
			Layout *layoutT = RootNode->getChildByName<Layout*>("guideRoomL");
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

	GuideControl::GetInstance()->injectHander(GUIDESTEP::N2, "room_effect_r", [=](){
		if (RootNode != NULL)
		{
			Layout *layoutT = RootNode->getChildByName<Layout*>("guideRoomR");
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
}

Node* LayerDzGames::getRoyal()
{
	return royal;
}

Node* LayerDzGames::getNormal()
{
	return normal;
}

Node* LayerDzGames::getMaster()
{
	return master;
}
Node* LayerDzGames::getElite()
{
	return elite;
}

Layout* LayerDzGames::getPanelLobby()
{
	return panelLobby;
}