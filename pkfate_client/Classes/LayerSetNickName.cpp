#include "LayerSetNickName.h"
#include "AvatarControl.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "UserControl.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "SoundControl.h"

// on "init" you need to initialize your instance
bool LayerSetNickName::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_avatarId = -1;
	_enableExit = false;
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerSetNickName.csb");
	if (!RootNode)
		return false;
	panelSet = RootNode->getChildByName<Layout*>("panelSet");
	if (!panelSet)
		return false;
	auto lbTitle = panelSet->getChildByName<Text*>("lbTitle");
	if (!lbTitle)
		return false;
	lbTitle->setString(Language::getStringByKey("SetNicknameAvatar"));
	listAvatars = panelSet->getChildByName<ListView*>("listAvatars");
	if (!listAvatars)
		return false;
	panelImg = panelSet->getChildByName<Layout*>("panelImg");
	if (!panelImg)
		return false;
	_sizeImgPanel = panelImg->getSize();
	Text *tempText = Text::create();
	tempText->setFontSize(36);
	tempText->setFontName("font/msyhbd.ttf");
	tempText->setTextColor(Color4B::GREEN);
	tempText->setString(StringUtils::format("%s%s", Language::getStringByKey("Select"), Language::getStringByKey("Avatar")));
	tempText->setPosition(Vec2(_sizeImgPanel.width / 2, _sizeImgPanel.height / 2-150));
	panelImg->addChild(tempText);
	auto txtNickName = panelSet->getChildByName<Widget*>("txtNickName");
	if (!txtNickName)
		return false;
	//获取txtNickName位置与大小信息后直接删除，用editBox控件代替
	if (!(_editBoxNickName = EditBox::create(txtNickName->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxNickName->setAnchorPoint(txtNickName->getAnchorPoint());
	_editBoxNickName->setPosition(txtNickName->getPosition());
	txtNickName->removeFromParent();
	_editBoxNickName->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxNickName->setPlaceHolder(Language::getStringByKey("InputNickname"));
	_editBoxNickName->setDelegate(this);
	panelSet->addChild(_editBoxNickName);
	lbNickName = panelSet->getChildByName<Text*>("lbNickName");
	if (!lbNickName)
		return false;
	lbNickName->setZOrder(1);		//将提示信息zorder提升到最前面
	lbNickName->setString("");
	btEnter = panelSet->getChildByName<Button*>("btEnter");
	if (!btEnter)
		return false;
	btEnter->setTitleText(Language::getStringByKey("Confirm"));
	btEnter->addClickEventListener(CC_CALLBACK_1(LayerSetNickName::btEnterClicked, this));
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerSetNickName::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	listAvatars->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(LayerSetNickName::selectedItemEvent, this));
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerSetNickName::onSetNickNameCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::SET_NICKNAME), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerSetNickName::onSetAvatarCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::SET_AVATAR), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerSetNickName::onCheckNickNameCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::CHECK_NICKNAME), NULL);
	
	return true;
}




void LayerSetNickName::onEnter()
{
	Layer::onEnter();
	//set avatars list
	for (auto avatar : AvatarControl::GetInstance()->GetAvatarsData())
	{
		Sprite *sp = AvatarControl::GetInstance()->GetAvatarSprite(avatar.first);
		if (sp)
		{
			//sp->setScale(2);
			Layout *item = Layout::create();
			item->setTouchEnabled(true);
			Size size = sp->getBoundingBox().size;
			size.width+=30;
			item->setSize(size);
			sp->setPosition(size.width / 2, size.height / 2);
			item->addChild(sp);
			item->setTag(avatar.first);
			listAvatars->pushBackCustomItem(item);
		}
	}
	
	//如果已经存在昵称，显示当前数据，只允许修改头像，且可以退出页面
	UserData *data = UserControl::GetInstance()->GetUserData();
	if (data&&!data->nickname.empty())
	{
		_enableExit = true;
		//设置头像
		Sprite *spAvatar = AvatarControl::GetInstance()->GetAvatarSprite(data->avatar);
		if (spAvatar)
		{
			spAvatar->setPosition(_sizeImgPanel.width / 2, _sizeImgPanel.height / 2);
			panelImg->removeAllChildren();
			panelImg->addChild(spAvatar);
			_avatarId = data->avatar;
		}
		//设置昵称
		_editBoxNickName->setText(data->nickname.c_str());
		_editBoxNickName->setFontColor(Color4B::GRAY);
		_editBoxNickName->setEnabled(false);
	}
}
void LayerSetNickName::onExit()
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Layer::onExit();
	PKNotificationCenter::getInstance()->postNotification("DoneSigninTaskNotification");
}
bool LayerSetNickName::onTouchBegan(Touch *touch, Event *event){
	if (_enableExit&&!panelSet->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce([=](float){this->removeFromParent(); }, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	return true;
}

void LayerSetNickName::selectedItemEvent(Ref *pSender, ListView::EventType type)
{
	switch (type)
	{
	case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_START:
	{
		/*ListView* listView = static_cast<ListView*>(pSender);
		CC_UNUSED_PARAM(listView);
		CCLOG("select child start index = %ld", listView->getCurSelectedIndex());*/
		break;
	}
	case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_END:
	{
		ListView* listView = static_cast<ListView*>(pSender);
		int avatarId = listView->getItem(listView->getCurSelectedIndex())->getTag();
		CCLOG("select avatar index = %ld", avatarId);
		Sprite *spAvatar = AvatarControl::GetInstance()->GetAvatarSprite(avatarId);
		if (spAvatar)
		{
			spAvatar->setPosition(100, _sizeImgPanel.height / 2+420);
			panelImg->removeAllChildren();
			panelImg->addChild(spAvatar);
			_avatarId = avatarId;
		}
		break;
	}
	default:
		break;
	}
}



bool LayerSetNickName::CheckNickName()
{
	_nickname = _editBoxNickName->getText();
	_nickname = replaceAll(_nickname, " ", "");
	_editBoxNickName->setText(_nickname.c_str());
	if (_nickname.empty())
	{
		lbNickName->setString(Language::getStringByKey("NicknameCanNotEmpty"));
		return false;
	}
	if (_nickname.length() < 4 || _nickname.length() > 16)
	{
		lbNickName->setString(Language::getStringByKey("NicknameLenError"));
		return false;
	}
	if (Language::isNeedFitler(_nickname)){
		lbNickName->setString(Language::getStringByKey("FilterWord"));
		return false;
	}

	lbNickName->setString("");
	return true;
}
void LayerSetNickName::btEnterClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	if (_editBoxNickName->isEnabled())
	{
		//设置昵称和头像，一般是首次进入游戏时进行设置
		if (!CheckNickName())
			return;
		if (_avatarId <= 0)
		{
			//红字显示
			Text *temp = (Text*)panelImg->getChildren().at(0);
			if (temp)
				temp->setTextColor(Color4B::RED);
			return;
		}
		if (!Api::User::SetNickName(_nickname.c_str(), _avatarId))
			CCLOG("[%s]:%s", "LayerSetNickName::btEnterClicked", "send setNickName msg error");
	}
	else
	{
		//只设置头像
		if (UserControl::GetInstance()->GetUserData()->avatar != _avatarId)
		{
			if (!Api::User::SetAvatar(_avatarId))
				CCLOG("[%s]:%s", "LayerSetNickName::btEnterClicked", "send setAvatar msg error");
		}
		else
			this->removeFromParent();
	}
}
void LayerSetNickName::onSetNickNameCallBack(Ref *pSender)
{
	//SetNickName回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			// setNickName ok
			this->removeFromParent();
			Api::User::GetUserInfo();		//申请一次新的数据，scene会自动更新数据
		}
		else
		{
			// login fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLoginCallBack", "login error.", msg->code, msg->data.c_str());
			switch (msg->code)
			{
			case 2011:
				lbNickName->setString(Language::getStringByKey("NicknameAlreadySet"));
				break;
			case 2012:
				lbNickName->setString(Language::getStringByKey("NicknameAlreadyExists"));
				break;
			default:
				lbNickName->setString(Language::getStringByKey("ErrorTry"));
				break;
			}
		}
		msg->release();
	});
}
void LayerSetNickName::onSetAvatarCallBack(Ref *pSender)
{
	//SetAvatar回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			// SetAvatar ok
			this->removeFromParent();
			Api::User::GetUserInfo();		//申请一次新的数据，scene会自动更新数据
		}
		else
		{
			// SetAvatar fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerSetNickName::onSetAvatarCallBack", "set avatar error.", msg->code, msg->data.c_str());
			switch (msg->code)
			{
			default:
				lbNickName->setString(Language::getStringByKey("ErrorTry"));
				break;
			}
		}
		msg->release();
	});
}
void LayerSetNickName::editBoxReturn(EditBox* editBox)
{
	if (_editBoxNickName == editBox)
	{
		//检查 昵称
		CheckNickName();		

		if (!Api::User::CheckNickName(_nickname.c_str()))
			CCLOG("CheckNickName error");
	}
}

void LayerSetNickName::onCheckNickNameCallBack(Ref *pSender)
{
	//SetNickName回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		
	//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
				if (strcmp(msg->data.c_str(), "true") != 0)
				{
					lbNickName->setString(Language::getStringByKey("NicknameAlreadyExists"));
				}
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerSetNickName::onCheckNickNameCallBack", "CheckNickName error.", msg->code, msg->data.c_str());
			switch (msg->code)
			{
			default:
				lbNickName->setString(Language::getStringByKey("ErrorTry"));
				break;
			}
		}
		msg->release();
	});
}