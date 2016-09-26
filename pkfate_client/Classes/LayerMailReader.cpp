#include "LayerMailReader.h"
#include "AvatarControl.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "UserControl.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "LayerLoading.h"
#include "AvatarControl.h"
#include "BillControl.h"
#include "SocketControl.h"
#include "SceneLogin.h"
#include "SoundControl.h"
#include "LayerFriend.h"
#include "LayerLoading.h"
#include "SceneDzRoom.h"
#include "LayerMailWriter.h"
// on "init" you need to initialize your instance
bool LayerMailReader::init()
{
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("mail/LayerMailReader.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

	

	text_title = RootNode->getChildByName<Text*>("text_title");
	if (!text_title)
		return false;
	from = RootNode->getChildByName<Text*>("from");
	if (!from)
		return false;
	
	text_content = RootNode->getChildByName<TextField*>("text_content");
	if (!text_content)
		return false;
	

	btnConfirm = RootNode->getChildByName<Button*>("btn_confirm");
	if (!btnConfirm)
		return false;
	btnConfirm->setVisible(false);

	btnCancel = RootNode->getChildByName<Button*>("btn_cancel");
	if (!btnCancel)
		return false;
	btnCancel->setVisible(false);


	Layout *touch_layer;
	if (!(touch_layer = RootNode->getChildByName<Layout*>("touch_layer")))
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
	//
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	return true;
}



void LayerMailReader::setMessageData(MessageData* messageData)
{
	this->messageData = messageData;
	text_title->setString(messageData->title);
	text_content->setString(messageData->content->content);

	int type = messageData->content->type;

	from->setString(this->messageData->sender_nickname);

	switch (type)
	{
		case::NoticeMsgSubType::USER_MSG_ADD_FRIEND_REQ://添加好友
		{
			btnCancel->setTitleText(Language::getStringByKey("Refuse"));
			btnCancel->setVisible(true);
			btnCancel->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
				this->messageData->status = NOTICE_MSG_STATUS::DELETED;
				refuseFriend();
			});
		
			btnConfirm->setTitleText(Language::getStringByKey("Agree"));
			btnConfirm->setVisible(true);
			btnConfirm->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
				this->messageData->status = NOTICE_MSG_STATUS::DELETED;
				agreeFriend();

			});

			
		}
			break;
		case::NoticeMsgSubType::FRIEND_INVITE_GAME: //邀请游戏
		{
			btnConfirm->setTitleText(Language::getStringByKey("AgreeInvite"));
			btnConfirm->setVisible(true);
			btnConfirm->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

				if (GuideControl::GetInstance()->isGuideFinished(GAMETYPE::DZPK)){
					this->messageData->status = NOTICE_MSG_STATUS::DELETED;
					agreeInvite();
				}
				else{
					Tips("please finish dzpk tech");
				}
			
			});
		
		}
			break;

		case::NoticeMsgSubType::FRIEND_SEND_MSG: //交流信息
		{
			btnConfirm->setTitleText(Language::getStringByKey("Reply"));

			btnConfirm->setVisible(FriendControl::GetInstance()->isFriend(this->messageData->content->user_id));

			btnConfirm->addClickEventListener([&](Ref *ref){
				reply();
			});

		}
			break;
		case::NoticeMsgSubType::USER_MSG_DEL_FRIEND: //删除好友
		{
			btnConfirm->setTitleText(Language::getStringByKey("Confirm"));

			btnConfirm->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
				this->messageData->status = NOTICE_MSG_STATUS::DELETED;
				this->removeFromParent();
			});

		}
			break;

		case::NoticeMsgSubType::USER_MSG_ADD_FRIEND_RSP: //好友添加回应
		{
			btnConfirm->setTitleText(Language::getStringByKey("Confirm"));

			btnConfirm->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
				this->messageData->status = NOTICE_MSG_STATUS::DELETED;
				this->removeFromParent();
			});

		}
			break;

		default:
		{
			btnConfirm->setTitleText(Language::getStringByKey("Confirm"));

			btnConfirm->addClickEventListener([&](Ref *ref){
				SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
				this->removeFromParent();
			});
		}
			break;
	}

	CCLOG("--reader---NoticeMsgSubType-----:%d", type);
}


void LayerMailReader::reply()
{

	LayerMailWriter * mailWriter = this->getChildByName<LayerMailWriter*>("LayerMailWriter");
	if (!mailWriter){
		mailWriter = LayerMailWriter::create(); //临时创建释放 avoid texture memory overflow
		mailWriter->setName("LayerMailWriter");
		mailWriter->setSendTo(this->messageData->sender_nickname, this->messageData->sender_id);
		Director::getInstance()->getRunningScene()->addChild(mailWriter, 2);
	}

	this->removeFromParent();
}


void LayerMailReader::agreeInvite()
{

	MailData* mailData = this->messageData->content;

	DZControl::GetInstance()->setRoomInfo(mailData->game_type, DZROOMKIND::DZ_GAME_JOIN_CREATE_ROOM, mailData->token);

	this->removeFromParent();
	
	if (DZControl::GetInstance()->isHallView()){
		PKNotificationCenter::getInstance()->postNotification("TriggerJoinCreateRoom", nullptr);
	}
	else{
		PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::DZPK)));
	}
}



void LayerMailReader::agreeFriend()
{
	Array* array = Array::create();
	array->addObject(String::create(toString(this->messageData->content->user_id)));
	string nickname = UserControl::GetInstance()->GetUserData()->nickname;
	array->addObject(String::create(StringUtils::format(Language::getStringByKey("AgreeFriendContentFormat"), nickname.c_str())));
	PKNotificationCenter::getInstance()->postNotification("TriggleAgreeAddFriend", array); //第一次 默认切换
	this->removeFromParent();
}


void LayerMailReader::refuseFriend()
{
	Array* array = Array::create();
	array->addObject(String::create(toString(this->messageData->content->user_id)));
	string nickname = UserControl::GetInstance()->GetUserData()->nickname;
	array->addObject(String::create(""));
	PKNotificationCenter::getInstance()->postNotification("TriggleDenyFriend", array); //第一次 默认切换
	this->removeFromParent();
}


void LayerMailReader::onEnter()
{
	Layer::onEnter();
	
}

void LayerMailReader::finishRead()
{
	if (messageData->status == NOTICE_MSG_STATUS::UNREAD){
		messageData->status = NOTICE_MSG_STATUS::READED;
		PKNotificationCenter::getInstance()->postNotification("ReadMessage", messageData); //
	}
	else if (messageData->status == NOTICE_MSG_STATUS::DELETED){
		PKNotificationCenter::getInstance()->postNotification("ReadMessage", messageData);
	}
	
}

void LayerMailReader::onExit()
{

	finishRead();

	//注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Layer::onExit();
}



