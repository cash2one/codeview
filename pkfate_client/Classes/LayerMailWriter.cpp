#include "LayerMailWriter.h"
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
#include "LayerFriendInvite.h"
#include "FriendControl.h"

LayerMailWriter::LayerMailWriter() :send_user_id(0)
{

}

// on "init" you need to initialize your instance
bool LayerMailWriter::init()
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
	RootNode = CSLoader::createNode("mail/LayerMailWriter.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

	

	text_to = RootNode->getChildByName<TextField*>("text_to");
	if (!text_to)
		return false;
	text_to->setString("");

	Text* label_to = RootNode->getChildByName<Text*>("label_to");
	if (!label_to)
		return false;
	label_to->setString(Language::getStringByKey("Receiver"));

	Text* label_content = RootNode->getChildByName<Text*>("label_content");
	if (!text_to)
		return false;
	label_content->setString(Language::getStringByKey("MailBody"));
	

	Button* btn_title = RootNode->getChildByName<Button*>("btn_title");
	btn_title->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerFriendInvite* layerFriendInvite = LayerFriendInvite::create();
		layerFriendInvite->setFrom(FROMPAGE::WRITE_MAIL_PAGE);
		this->addChild(layerFriendInvite);
	});


	Button * btn_addFriend = RootNode->getChildByName<Button*>("btn_addFriend");

	btn_addFriend->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerFriend * layerFriend = this->getChildByName<LayerFriend*>("LayerFriend");
		if (!layerFriend){
			layerFriend = LayerFriend::create(); //临时创建释放 avoid texture memory overflow
			
			layerFriend->setName("LayerFriend");
			this->addChild(layerFriend, 2);
			OpenWithAlert(layerFriend);
		}
	});
	
	text_content = RootNode->getChildByName<TextField*>("text_content");
	if (!text_content)
		return false;
	if (!(_textContentEditBox = EditBox::create(text_content->getSize(), Scale9Sprite::create())))
		return false;
	_textContentEditBox->setAnchorPoint(text_content->getAnchorPoint());
	_textContentEditBox->setPosition(text_content->getPosition());
	_textContentEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_textContentEditBox->setPlaceholderFontSize(30);
	_textContentEditBox->setInputMode(EditBox::InputMode::ANY);
	_textContentEditBox->setMaxLength(140);
	_textContentEditBox->setDelegate(this);
	RootNode->addChild(_textContentEditBox);

	btnSend = RootNode->getChildByName<Button*>("btn_send");
	//btnSend->setTitleText(Language::getStringByKey("Confirm"));
	btnSend->setEnabled(false);
	btnSend->setBright(false);
	if (!btnSend)
		return false;

	btnSend->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (checkEmailTo() && checkContent()) //检验通过
		{
			funSendMessage();

			this->removeFromParent();
		}
	});

	btnSend->setTitleText(Language::getStringByKey("Send"));

	Button *btBack = RootNode->getChildByName<Button*>("btBack");


	btBack->addClickEventListener([&](Ref *ref){
		this->removeFromParent();
	});

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerMailWriter::onTriggleSetMailTo), "TriggleSetMailTo", nullptr);//监听来自视图的事件

	return true;
}

bool LayerMailWriter::checkEmailTo()
{
	return true;
}
bool LayerMailWriter::checkContent()
{
	return true;
}

void LayerMailWriter::funSendMessage()
{
	
	if (text_to->getString().empty() || text_content->getString().empty() || send_user_id == 0){  return; }

	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::sendFriendMessage(user_id, send_user_id, text_content->getString().c_str()))
	{
		LayerLoading::CloseWithTip("sendFriendMessage net error");
	}
}


void LayerMailWriter::onTriggleSetMailTo(Ref *pSender)
{
	String *mailTo = (String*)pSender;
	if (mailTo){
		FriendData* friendData = FriendControl::GetInstance()->getFriendData(mailTo->intValue());
		if (!friendData) return;
		setSendTo(friendData->nickname, friendData->user_id);
	}
	else{
		setSendTo("", 0);
	}
}

void LayerMailWriter::setSendTo(string sendTo,int send_user_id)
{
	text_to->setString(sendTo);

	this->send_user_id = send_user_id;
}

void LayerMailWriter::onEnter()
{
	Layer::onEnter();
	
}
void LayerMailWriter::onExit()
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Layer::onExit();

}


void LayerMailWriter::editBoxReturn(EditBox* editBox)
{
	if (_textContentEditBox == editBox){
		auto text_content = RootNode->getChildByName<TextField*>("text_content");
		text_content->setString(_textContentEditBox->getText());
		_textContentEditBox->setText("");
	}

	if (checkContent() && checkEmailTo())
	{
		btnSend->setBright(true);
		btnSend->setEnabled(true);
	}
		
}