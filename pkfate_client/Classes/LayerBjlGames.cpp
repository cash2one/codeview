#include "LayerBjlGames.h"
#include "cocostudio/CocoStudio.h"
#include "ApiGame.h"
#include "LayerLoading.h"
#include "comm.h"

#include "SceneBjlRoom.h"
#include "ui/UIImageView.h"
#include "PKNotificationCenter.h"
#include "ApiBill.h"
#include "cmd.h"
#include "SoundControl.h"
#include "LayerGuideMask.h"
#include "LayerDuang.h"
vector<string> strRoomType;
vector<string> strRoomTypePath;


BJLRoomItem::BJLRoomItem()
{
	rootNode = CSLoader::createNode("bjl/RoomItem.csb");

	rootNode->setPositionX(50);

	Button * btn_room_item = rootNode->getChildByName<Button*>("btn_room_item");

	btn_room_item->addClickEventListener(CC_CALLBACK_1(BJLRoomItem::btnClickHandler, this));

	btn_room_item->setSwallowTouches(false);

	addChild(rootNode);

	setTouchEnabled(false);
}

void BJLRoomItem::btnClickHandler(Ref * sender)
{
	LayerLoading::Wait("enter...");
	scheduleOnce([=](float dt){
		auto scene = SceneBjlRoom::create(this->roomInfoBjl);
		Director::getInstance()->pushScene(scene);
		LayerLoading::Close();
	}, 0.0f, "enter room");
}

void BJLRoomItem::enterRoom()
{
	scheduleOnce([=](float dt){
		

	//	PKNotificationCenter::getInstance()->postNotification("EnterSNGRoom", String::createWithFormat("%d", sNGRoomStatus->room_id));

	}, 0.0f, "enter room");

}

void BJLRoomItem::setProperty(RoomInfoBjl* roomInfoBjl)
{
	this->roomInfoBjl = roomInfoBjl;
	Sprite* imgRoomType = rootNode->getChildByName<Sprite*>("imgRoomType");

	imgRoomType->setTexture(Director::getInstance()->getTextureCache()->addImage(strRoomTypePath[roomInfoBjl->roomType]));
	CCLOG(strRoomTypePath[roomInfoBjl->roomType].c_str());
	auto sp = new spRoomInfo();
	this->sp = sp;
	sp->txtId = rootNode->getChildByName<Text*>("txtId");
	sp->txtId->setString(StringUtils::format("#%d", roomInfoBjl->id));
	sp->txtCount = rootNode->getChildByName<Text*>("txtCount");
	sp->lz = new BjlLzSm();
	if (!sp->lz->init(rootNode->getChildByName<Node*>("layoutLzsm")))
	{
		CCLOG("[%s]:init Node_Lzsm error", "LayerBjlGames::updateRoomsInfo");
		exit(-1);
	}
	sp->lz->setScrollEnable(false);
	rootNode->setUserData(roomInfoBjl);
	
	//添加等待动画
	Sprite *sprite = Sprite::create("start/img/CheckVer.png");
	sprite->setName("loading");
	sprite->runAction(RepeatForever::create(RotateBy::create(1, 359)));
	Size size = sp->lz->_scrollViewLzSm->getContentSize();
	sprite->setPosition(size.width / 2, size.height / 2);
	sp->lz->_scrollViewLzSm->addChild(sprite);
	//设置当前房间人数
	sp->txtCount->setString(StringUtils::format("%3d/%3d", roomInfoBjl->count, roomInfoBjl->type->players_limit));
}



BJLRoomItem::~BJLRoomItem()
{

}


// on "init" you need to initialize your instance
bool LayerBjlGames::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	strRoomType =
	{
		"",		//roomtype从1开始，所以增加一个空字符串做为数组0的索引值
		StringUtils::format("%s - %s", Language::getStringByKey("Baccarat"), Language::getStringByKey("RoomPrimary")),
		StringUtils::format("%s - %s", Language::getStringByKey("Baccarat"), Language::getStringByKey("RoomMiddle")),
		StringUtils::format("%s - %s", Language::getStringByKey("Baccarat"), Language::getStringByKey("RoomHigh"))
	};
	strRoomTypePath =
	{
		"",		//roomtype从1开始，所以增加一个空字符串做为数组0的索引值
		"bjl/rooms/RoomTypePrimary.png",
		"bjl/rooms/RoomTypeMiddle.png",
		"bjl/rooms/RoomTypeHigh.png"
	};
	layerBjlNew = nullptr;
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("bjl/LayerBjlGames.csb");
	if (!RootNode)
		return false;
	if (!(panelGames = RootNode->getChildByName<Layout*>("panelGames")))
		return false;
	if (!(panelLobby = panelGames->getChildByName<Layout*>("panelLobby")))
		return false;
	if (!(btBack = panelLobby->getChildByName<Button*>("btBack")))
		return false;
	if (!(btNew = panelLobby->getChildByName<Button*>("btNew")))
		return false;
	if (!(btRoomS = panelLobby->getChildByName<Button*>("btRoomS")))
		return false;
	btRoomS->setTitleText(Language::getStringByKey("Primary"));
	btRoomS->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(BJLROOMTYPE::Small);
	});
	if (!(btRoomM = panelLobby->getChildByName<Button*>("btRoomM")))
		return false;
	btRoomM->setTitleText(Language::getStringByKey("Middle"));
	btRoomM->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(BJLROOMTYPE::Middle);
	});

	if (!(btRoomB = panelLobby->getChildByName<Button*>("btRoomB")))
		return false;
	btRoomB->setTitleText(Language::getStringByKey("Senior"));
	btRoomB->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(BJLROOMTYPE::Big);
	});
	btNew->setTitleText(Language::getStringByKey("SingleGame"));

	if (!(listRooms = panelGames->getChildByName<ListView*>("listRooms")))
		return false;
	listRooms->setItemsMargin(650);
	listRooms->removeAllChildren();		//清空列表
	this->addChild(RootNode);

	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerBjlGames::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	//add listview event
	listRooms->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(LayerBjlGames::onSelectedRoomEvent, this));
	//add button func
	btBack->setPressedActionEnabled(true);
	btBack->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
		PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::NO)));

		LayerDuang::panelStatus();
		LayerDuang::uiGameMenu();
	});


	btNew->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRooms(BJLROOMTYPE::New);
	});
	_roomType = (BJLROOMTYPE)-1;
	//register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onConnectCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Connect), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onLoginCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_Login), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onEnterRoomCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_EnterRoom), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onMsgRoomInfo), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomsInfo), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onMsgRoomTypesInfo), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomTypesInfo), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onRoomUpdate), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomInfoUpdate), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::onGetRoomResultsCallBack), GetMsgTypeString(MSGTYPE_BJLGAMEMO, MSGTYPE_BjlGameMo_RoomResults), nullptr);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBjlGames::triggerShowRoom), "triggerShowRoom", NULL);

	/*auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	Director::getInstance()->getRunningScene()->addChild(layerLoading);*/

	LayerLoading::Wait();

	//connect bjl rooms.
	Api::Game::mo_baccarat_connect();

	return true;
}




void LayerBjlGames::triggerShowRoom(Ref *pSender)
{
	showRooms(BJLROOMTYPE::Small);
}



void LayerBjlGames::onSelectedRoomEvent(Ref *pSender, ListView::EventType type)
{
	if (type != ListView::EventType::ON_SELECTED_ITEM_END)
		return;
	LayerLoading::Wait("enter...");
	scheduleOnce([=](float dt){
		auto layout = listRooms->getItem(listRooms->getCurSelectedIndex());
		RoomInfoBjl *room = (RoomInfoBjl *)layout->getUserData();
		CCLOG("#%d is clicked!", room->id);
		auto scene = SceneBjlRoom::create(room);
		Director::getInstance()->pushScene(scene);
		LayerLoading::Close();
	}, 0.0f, "enter room");
	/*Api::Game::mo_baccarat_leave();
	Api::Game::mo_baccarat_enter(layout->getTag());*/
}


bool LayerBjlGames::onTouchBegan(Touch *touch, Event *event){
	if (!panelGames->getBoundingBox().containsPoint(touch->getLocation()))
		return false;
	return true;
}


void LayerBjlGames::onEnter()
{
	Layer::onEnter();
	if (GameControlBjl::GetInstance()->RoomTypeInfos.size() > 0)
	{
		if (_roomType == BJLROOMTYPE::New)
			showRooms(BJLROOMTYPE::Small);
		else
			showRooms(_roomType);
	}
	addGuide();
}


void LayerBjlGames::onExit()
{
	Layer::onExit();
}


LayerBjlGames::~LayerBjlGames()
{
	Api::Game::mo_baccarat_disconnect();
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void LayerBjlGames::onConnectCallBack(Ref *pSender)
{
	//连接游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功连接,开始登录
			Api::Game::mo_baccarat_login();
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onConnectCallBack", "connect error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


void LayerBjlGames::onLoginCallBack(Ref *pSender)
{
	//登录游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{

		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onConnectCallBack", "connect error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}



void LayerBjlGames::onEnterRoomCallBack(Ref *pSender)
{
	//进入游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功进入
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onEnterRoomCallBack", "login error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


void LayerBjlGames::onMsgRoomTypesInfo(Ref *pSender)
{
	// 获取所有房间类型信息
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功获取所有房间类型信息数据
			if (!GameControlBjl::GetInstance()->InitRoomTypesInfoData(msg->data))
		 		CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onMsgRoomTypesInfo", "init roomtypes info error.", msg->code, msg->data.c_str());
			
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onMsgRoomTypesInfo", "code error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void LayerBjlGames::onMsgRoomInfo(Ref *pSender)
{
	// 获取游戏大厅所有房间信息
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功获取游戏大厅所有房间信息数据
			/*auto layerLoading = Director::getInstance()->getRunningScene()->getChildByName<LayerLoading*>("LayerLoading");
			if (layerLoading)
			layerLoading->removeFromParent();*/
			LayerLoading::Close();
			//get top-bar
			Layout *pLayout = (Layout*)(this->getParent()->getParent()->getChildByName("panelStatus"));

			LayerDuang::uiBorderAppear(pLayout);
			LayerDuang::bjlPanelLobby();

			if (GameControlBjl::GetInstance()->InitRoomsInfoData(msg->data))
			{
				showRooms(BJLROOMTYPE::Small);
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onMsgRoomInfo", "init rooms info error.", msg->code, msg->data.c_str());

			LayerDuang::bjlGamesRoom();

			if (GuideControl::GetInstance()->checkInGuide()){
				LayerGuideMask::GetInstance()->playTechAction(GUIDESTEP::N1, Director::getInstance()->getRunningScene());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onMsgRoomInfo", "code error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
//更新房间信息，id为－1时，更新所有的房间信息 为0时，更新大厅信息
void LayerBjlGames::updateRoomsInfo(int roomId)
{
	auto room = GameControlBjl::GetInstance()->RoomInfos[roomId];
	if (!room)
		return;		//没有找到指定房间数据
	auto sp = spRoomsInfo[roomId];
	if (!sp)
		return;		//没有找到指定房间精灵
	sp->txtCount->setString(StringUtils::format("%3d/%3d", room->count, room->type->players_limit));
}
void LayerBjlGames::onGetRoomResultsCallBack(Ref *pSender)
{
	// 获取游戏结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功进入
			//获取游戏数据
			for (auto room : GameControlBjl::GetInstance()->GetResultsFromData(msg->data))
			{
				spRoomInfo *sp = spRoomsInfo[room.first];
				if (sp)
				{
					auto *spLoading = sp->lz->_scrollViewLzSm->getChildByName("loading");
					if (spLoading)
						spLoading->removeFromParent();
					sp->lz->clearLz();
					vector<int> typeinfos = GameControlBjl::GetInstance()->JsonResults2BetType(room.second);
					for (auto type : typeinfos)
					{
						sp->lz->addLz(type);
					}
					CCLOG("#%d:%s", room.first, room.second.c_str());
				}
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onGetRoomResultsCallBack", "get results error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void LayerBjlGames::onRoomUpdate(Ref *pSender)
{
	// 游戏房间数据更新，一般是指有人数变动
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功获取房间update信息数据
			int id = GameControlBjl::GetInstance()->UpdateRoomInfo(msg->data);
			if (id > 0)
				updateRoomsInfo(id);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onRoomUpdate", "code error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void LayerBjlGames::showRooms(BJLROOMTYPE type)
{
	this->listRooms->setVisible(type != BJLROOMTYPE::New);

	/*if (layerBjlNew)
		this->layerBjlNew->setVisible(type == BJLROOMTYPE::New);
		else{*/
	//当首次点击创建房间时，再进行初始化
	if (type == BJLROOMTYPE::New){
		LayerBjlNew * bjlNew = this->getChildByName<LayerBjlNew*>("LayerBjlNew");
		if (!bjlNew){
			LayerBjlNew * bjlNew = LayerBjlNew::create(); //临时创建释放 avoid texture memory overflow
			bjlNew->setName("LayerBjlNew");
			this->addChild(bjlNew, 1);
			OpenWithAlert(bjlNew);
		}
	}
	//}
	switch (_roomType)
	{
	case Small:
		btRoomS->setEnabled(true);
		btRoomS->setTitleColor(ccc3(43,105,190));
		btRoomS->setHighlighted(false);

		break;
	case Middle:
		btRoomM->setEnabled(true);
		btRoomM->setTitleColor(ccc3(43, 105, 190));
		btRoomM->setHighlighted(false);
		break;
	case Big:
		btRoomB->setEnabled(true);
		btRoomB->setTitleColor(ccc3(43, 105, 190));
		btRoomB->setHighlighted(false);
		break;
	case New:
		btNew->setEnabled(true);
		btNew->setTitleColor(ccc3(43, 105, 190));
		btNew->setHighlighted(false);
		break;
	}
	_roomType = type;
	switch (_roomType)
	{
	case Small:
		btRoomS->setEnabled(false);
		btRoomS->setTitleColor(ccc3(0, 233, 255));
		btRoomS->setHighlighted(true);
		break;
	case Middle:
		btRoomM->setEnabled(false);
		btRoomM->setTitleColor(ccc3(0, 233, 255));
		btRoomM->setHighlighted(true);
		break;
	case Big:
		btRoomB->setEnabled(false);
		btRoomB->setTitleColor(ccc3(0, 233, 255));
		btRoomB->setHighlighted(true);
		break;
	case New:
		btNew->setEnabled(false);
		btNew->setTitleColor(ccc3(0, 233, 255));
		btNew->setHighlighted(true);
		break;
	default:
		return;
	}
	//清空房间列表
	listRooms->removeAllItems();
	//listRooms->scrollToLeft(0.5f, false);
	listRooms->jumpToLeft();
	for (auto info : spRoomsInfo)
	{
		delete info.second;
	}
	spRoomsInfo.clear();
	//如果点击房间类型时，增加房间到列表中
	if (_roomType == BJLROOMTYPE::Small || _roomType == BJLROOMTYPE::Middle || _roomType == BJLROOMTYPE::Big){
		vector<int> ids;
		for (auto item : GameControlBjl::GetInstance()->RoomInfos)
		{
			if (item.second->type->type_id == _roomType){
				item.second->roomType = _roomType;
				BJLRoomItem * bJLRoomItem = new BJLRoomItem();
				bJLRoomItem->setProperty(item.second);
				listRooms->pushBackCustomItem(bJLRoomItem);
				spRoomsInfo[item.second->id] = bJLRoomItem->sp;
			}
		}
		BJLRoomItem * bJLRoomItem = new BJLRoomItem();//添加一个默认的,占格
		listRooms->pushBackCustomItem(bJLRoomItem);

		if (ids.size() > 0)
			Api::Game::mo_baccarat_get_room_results(&ids);
	}


}



void LayerBjlGames::addGuide()
{

	GuideControl::GetInstance()->startWithType(GAMETYPE::BJL);

	if (!GuideControl::GetInstance()->checkInGuide())return;


	GuideControl::GetInstance()->injectHander(1, "kind_room_arrow", [=](){
		
		//primary arrow
		static Node* pArrow = LayerGuideMask::GetInstance()->addArrow(DIRECTION::BOTTOM, ccp(100, 900));

		//middle arrow
		pArrow->scheduleOnce([&](float dt){

			auto action = MoveBy::create(0.3, Vec2(0, -185));
			pArrow->runAction(action);

		}, 1, "arrow_move1");

		//advanced arrow
		pArrow->scheduleOnce([&](float dt){
			auto action = MoveBy::create(0.3, Vec2(0, -180));
			pArrow->runAction(action);
		}, 2, "arrow_move2");
	});

	//GuideControl::GetInstance()->injectHander(1, "kind_room_arrow1", [=](){
	//	LayerGuideMask::GetInstance()->addArrow(DIRECTION::BOTTOM, ccp(100, 800));
	//});

	//GuideControl::GetInstance()->injectHander(1, "seat_number", [=](){
	//	LayerGuideMask::GetInstance()->cleanMask();
	//	LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT, ccp(320, 260));
	//});

	GuideControl::GetInstance()->injectHander(1, "single_room_arrow", [=](){
		LayerGuideMask::GetInstance()->cleanMask();
		Sprite* pArrow = LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT, ccp(320, 260));

	});

	GuideControl::GetInstance()->injectHander(1, "back_arrow", [=](){
		LayerGuideMask::GetInstance()->cleanMask();
		LayerGuideMask::GetInstance()->addArrow(DIRECTION::LEFT, ccp(320, 82));

	});

	GuideControl::GetInstance()->injectHander(1, "middle_arrow", [=](){
		LayerGuideMask::GetInstance()->cleanMask();
	});

	GuideControl::GetInstance()->injectHander(1, "bjl_room_mask", [=](){
		Sprite* roomMask = Sprite::create("guide/room_guide.png");
		LayerGuideMask::GetInstance()->addTo(roomMask);
		roomMask->setName("roomMask");
		roomMask->setPosition(ccp(1055, 450));
		roomMask->setScaleY(1.05);
	});

	GuideControl::GetInstance()->injectHander(1, "jump_to_bjl_room", [=](){



		LayerLoading::Wait();

		scheduleOnce([=](float dt){

			auto layout = listRooms->getItem(1);
			RoomInfoBjl *room = (RoomInfoBjl *)layout->getUserData();

			auto scene = SceneBjlRoom::create(room);
			Director::getInstance()->pushScene(scene);
			LayerLoading::Close();

		}, 0.0f, "enter room");

	});

	
}

ListView* LayerBjlGames::getListRooms()
{
	return listRooms;
}

Layout* LayerBjlGames::getPanelLobby()
{
	return panelLobby;
}