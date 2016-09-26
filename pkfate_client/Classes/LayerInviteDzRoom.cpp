#include "LayerInviteDzRoom.h"
#include "cocostudio/CocoStudio.h"
#include "FriendControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "ApiGame.h"
#include "LayerLoading.h"
#include "DZControl.h"
#include "BillControl.h"
LayerInviteDzRoom::LayerInviteDzRoom() :currentType(0), currentIndex(0)
{

}

bool LayerInviteDzRoom::init()
{
	if (!Layer::init())
		return false;
	
	_nodeRoot = CSLoader::createNode("friend/LayerInviteDzRoom.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;



	btn_confirm = _nodeRoot->getChildByName<Button*>("btn_confirm");
	if (!btn_confirm)return false;

	btn_royal = _nodeRoot->getChildByName<Button*>("btn_royal");
	if (!btn_royal)return false;

	btn_normal = _nodeRoot->getChildByName<Button*>("btn_normal");
	if (!btn_normal)return false;

	text_blind = _nodeRoot->getChildByName<Text*>("text_blind");
	text_take = _nodeRoot->getChildByName<Text*>("text_take");

	take_slider = _nodeRoot->getChildByName<Slider*>("take_slider");

	if (!text_blind || !text_take||!take_slider) return false;

	btn_royal->addClickEventListener([&](Ref *ref){
		if (this->currentType == 0){
			selectType(1);
		}
		else{
			selectType(0);
		}
	});

	btn_normal->addClickEventListener([&](Ref *ref){
		if (this->currentType == 1){
			selectType(0);
		}
		else{
			selectType(1);
		}
	});
	
	btn_confirm->addClickEventListener([&](Ref *ref){
		createRoom();
	});


	Button *btBack = _nodeRoot->getChildByName<Button*>("btBack");


	btBack->addClickEventListener([&](Ref *ref){
		this->removeFromParent();
	});

	take_slider->addEventListener(CC_CALLBACK_2(LayerInviteDzRoom::SliderEvent, this));


	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		return true;
	};
	listener->onTouchEnded = [=](Touch *touch, Event *event){
		int index = floorl(take_slider->getPercent() / 25.0) + 1;
		clickSelect(index);
	};

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, take_slider);
	


	Button *btn_point = nullptr;

	for (int i = 0; i < 5; i++)
	{
		btn_point = _nodeRoot->getChildByName<Button*>(StringUtils::format("btn_point_%d",i));
		
		btn_point->addClickEventListener([&](Ref *ref){
			string s = ((Node*)ref)->getName();
			clickSelect(atoi(s.substr(s.size() - 1).c_str())+1);
		});
		btn_point->setSwallowTouches(false);
	}

	selectType(0);
	clickSelect(1);
	
	return true;
}


void LayerInviteDzRoom::clickSelect(int index)
{
	take_slider->setPercent((index - 1) * 25);
	selectIndex(index);

	Node *point = nullptr;

	for (int i = 0; i < 5; i++)
	{
		point = _nodeRoot->getChildByName<Node*>(StringUtils::format("point_%d", i));
		point->setVisible(i!=index -1);
	}
}


void LayerInviteDzRoom::onCreateRoomSucc(Ref* pSender)
{
	CCLOG("CreateRoomSucc  =====> alert wait page");
}


void LayerInviteDzRoom::createRoom()
{

	if (BillControl::GetInstance()->GetBalanceData(0)->balance <this->select_roomType->init_chip){

		Tips(StringUtils::format(Language::getStringByKey("ConditionEnterDZPK"), this->select_roomType->init_chip));

		return;
	}

	DZControl::GetInstance()->setRoomInfo(select_roomType->type_id, DZROOMKIND::DZ_GAME_SINGLE_CREATE_ROOM);

	PKNotificationCenter::getInstance()->postNotification("TriggerCreateGame", String::create(toString(this->select_roomType->type_id)));
	
	this->removeFromParent();
}


void LayerInviteDzRoom::SliderEvent(Ref *pSender, Slider::EventType type)
{
	
	switch (type)
	{
	case Slider::EventType::ON_PERCENTAGE_CHANGED:
	{
		Slider* slider = static_cast<Slider*>(pSender);

		int index = floorl(slider->getPercent() / 25.0)+1;

		selectIndex(index);

		//clickSelect(index);
		
	}
		break;
	default:
		break;
	}
}

void LayerInviteDzRoom::selectIndex(short index)
{
	if (currentIndex != index){

		currentIndex = index;

		int type = this->currentType == 0 ? this->currentIndex : this->currentIndex + 5;

		DzpkRoomType * roomTypeInfo = DZControl::GetInstance()->RoomTypeInfos[type];
		
		this->select_roomType = roomTypeInfo;

		text_blind->setString(StringUtils::format("%d/%d", roomTypeInfo->ante / 2, roomTypeInfo->ante));

		text_take->setString(toString(roomTypeInfo->init_chip));
	}
}


void LayerInviteDzRoom::selectType(short type)
{
	this->currentType = type;

	

	if (type == 1){// 皇家
		btn_normal->setColor(ccc3(100, 100, 100));
		btn_royal->setColor(ccc3(255, 255, 255));
		btn_royal->getChildByName("gou_select")->setVisible(true);
		btn_royal->getChildByName("gou_unselect")->setVisible(false);
		btn_normal->getChildByName("gou_select")->setVisible(false);
		btn_normal->getChildByName("gou_unselect")->setVisible(true);
	}
	else{
		btn_normal->setColor(ccc3(255, 255, 255));
		btn_royal->setColor(ccc3(100, 100, 100));
		btn_royal->getChildByName("gou_select")->setVisible(false);
		btn_royal->getChildByName("gou_unselect")->setVisible(true);
		btn_normal->getChildByName("gou_select")->setVisible(true);
		btn_normal->getChildByName("gou_unselect")->setVisible(false);
	}
}


LayerInviteDzRoom::~LayerInviteDzRoom()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void LayerInviteDzRoom::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
