#include "LayerFriend.h"
#include "cocostudio/CocoStudio.h"
#include "FriendControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "LayerMailWriter.h"
#include "LayerUserInfo.h"
#include "LayerDuang.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "ShareControl.h"
#endif


FriendSelectMenu::FriendSelectMenu()
{
	_nodeRoot = CSLoader::createNode("friend/FriendSelectMenu.csb");
	addChild(_nodeRoot);
}

FriendSelectMenu::~FriendSelectMenu()
{

}

FriendItem::FriendItem()
{
	_nodeRoot = CSLoader::createNode("friend/FriendItem.csb");
	addChild(_nodeRoot);
	this->setContentSize(Size(1760, 250));
}

void FriendItem::reset()
{
	for (int i = 1; i <= 3; i++){
		Node* itemNode = _nodeRoot->getChildByName(StringUtils::format("node_%d", i));
		itemNode->setVisible(false);
	}
}

void FriendItem::setProperty(FriendData* friendData, int i)
{

	Node* itemNode = _nodeRoot->getChildByName(StringUtils::format("node_%d",i));
	if (itemNode && friendData){
		itemNode->setVisible(true);
		user_id = friendData->user_id;
		nickname = friendData->nickname;
		avatar = friendData->avatar;
		CheckBox* select_box = itemNode->getChildByName<CheckBox*>("select_box");
		select_box->setUserData(friendData);
		//select_box->addEventListenerCheckBox
		///select_box->addEventListenerCheckBox(this, checkboxselectedeventselector(FriendItem::selectedEvent));

		Text* name = itemNode->getChildByName<Text*>("name");
		name->setString(friendData->nickname);
		Sprite* avatar = itemNode->getChildByName<Sprite*>("spAvatar");
		avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(friendData->avatar, 0));

		Node* select_menu = itemNode->getChildByName("select_menu");


		auto listener = EventListenerTouchOneByOne::create();
		//listener->setSwallowTouches(true);
		listener->onTouchBegan = [=](Touch *touch, Event *event){
			if (select_box->getBoundingBox().containsPoint(itemNode->convertToNodeSpace(touch->getLocation()))){
				select_menu->setVisible(true);
			}
			else{
				select_menu->setVisible(false);
			}
			return true;
		};

		auto dispatcher = Director::getInstance()->getEventDispatcher();
		dispatcher->addEventListenerWithSceneGraphPriority(listener, select_box);


		Button *btn_add = select_menu->getChildByName<Button*>("btn_add");
		Button *btn_mail = select_menu->getChildByName<Button*>("btn_mail");
		Button *btn_see = select_menu->getChildByName<Button*>("btn_see");
		Button *btn_del = select_menu->getChildByName<Button*>("btn_del");

		Node *label_add = select_menu->getChildByName<Node*>("label_add");
		Node *label_mail = select_menu->getChildByName<Node*>("label_mail");
		Node *label_see = select_menu->getChildByName<Node*>("label_see");
		Node *label_del = select_menu->getChildByName<Node*>("label_del");

		btn_add->setVisible(false);
		label_add->setVisible(false);
		btn_mail->setVisible(false);
		label_mail->setVisible(false);
		btn_see->setVisible(false);
		label_see->setVisible(false);
		btn_del->setVisible(false);
		label_del->setVisible(false);

		if (!FriendControl::GetInstance()->isFriend(friendData->user_id))
		{
			btn_add->addClickEventListener([=](Ref *ref){
				select_menu->setVisible(false);

				Array* array = Array::create();
				array->addObject(String::create(toString(user_id)));
				string nickname = UserControl::GetInstance()->GetUserData()->nickname;
				array->addObject(String::create(StringUtils::format("%s请求成为您的好友", nickname.c_str())));
				
				PKNotificationCenter::getInstance()->postNotification("TriggleAddFriend", array); //第一次 默认切换
			});
			btn_add->setVisible(true);
			label_add->setVisible(true);
		}
		else{
			btn_mail->addClickEventListener([=](Ref *ref){
				select_menu->setVisible(false);

				LayerMailWriter * mailWriter = this->getChildByName<LayerMailWriter*>("LayerMailWriter");
				if (!mailWriter){
					mailWriter = LayerMailWriter::create(); //临时创建释放 avoid texture memory overflow
					mailWriter->setName("LayerMailWriter");
					mailWriter->setSendTo(this->nickname,this->user_id);
					Director::getInstance()->getRunningScene()->addChild(mailWriter, 2);
				}
			});

			btn_see->addClickEventListener([=](Ref *ref){
				select_menu->setVisible(false);
				LayerOthersInfo *userInfo = LayerOthersInfo::create();

				Director::getInstance()->getRunningScene()->addChild(userInfo, 2);

				userInfo->lookUserInfo(this->user_id,this->nickname,this->avatar);
				
			});

			btn_del->addClickEventListener([=](Ref *ref){
				select_menu->setVisible(false);

				ShowTip(Language::getStringByKey("DelFriendTip"),[=](){
					PKNotificationCenter::getInstance()->postNotification("TriggleDelFriend", String::create(toString(user_id)));
				},nullptr);
				
			});

			btn_mail->setVisible(true);
			label_mail->setVisible(true);
			btn_see->setVisible(true);
			label_see->setVisible(true);
			btn_del->setVisible(true);
			label_del->setVisible(true);
		}
	}
}

void FriendItem::selectedEvent(cocos2d::Ref *pSender, CheckBoxEventType type)
{
	switch (type) {
	case cocos2d::ui::CHECKBOX_STATE_EVENT_SELECTED:
	{
		CheckBox * box = (CheckBox *)pSender;

		FriendData* friendData = (FriendData*)box->getUserData();

		PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", String::create(toString(friendData->user_id))); //第一次 默认切换

		break;
	}
	case cocos2d::ui::CHECKBOX_STATE_EVENT_UNSELECTED:
	{	
		PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", nullptr); //第一次 默认切换
	}
	break;
	default:
		break;
	}

}

FriendItem::~FriendItem()
{

}



LayerFriend::LayerFriend():friendList(nullptr)
{

}

bool LayerFriend::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("friend/LayerFriend.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;
	
	invite_panel = _nodeRoot->getChildByName<Layout*>("invite_panel");
	validate_panel = invite_panel->getChildByName<Layout*>("validate_panel");
	handler_panel = invite_panel->getChildByName<Layout*>("handler_panel");
	btn_bind = validate_panel->getChildByName<Button*>("btn_bind");
	btn_send = validate_panel->getChildByName<Button*>("btn_send");
	btn_invite = handler_panel->getChildByName<Button*>("btn_invite");
	friend_panel = _nodeRoot->getChildByName<Layout*>("friend_panel");
	invite_info_panel = _nodeRoot->getChildByName<Layout*>("invite_info_panel"); 

	if (!invite_panel || !friend_panel || !validate_panel || !handler_panel || !btn_bind || !btn_send || !btn_invite || !invite_info_panel)
		return false;

	Node *btnNode = invite_info_panel->getChildByName<Node*>("node_btn_phone");
	if (!btnNode)
		return false;
	btn_phone_invite = btnNode->getChildByName<Button*>("button");
	btnNode = invite_info_panel->getChildByName<Button*>("node_btn_share");
	if (!btnNode)
		return false;
	btn_share_invite = btnNode->getChildByName<Button*>("btn");
	if (!btn_share_invite || !btn_phone_invite)
		return false;

	btn_share_invite->addClickEventListener([&](Ref *ref){
		funcShare();
	});

	btn_phone_invite->addClickEventListener([&](Ref *ref){
		//todo
		if (!UserControl::GetInstance()->isValidPhone())
		{
			invite_panel->setVisible(true);
			validate_panel->setVisible(!UserControl::GetInstance()->isValidPhone());
			invite_info_panel->setVisible(false);
			handler_panel->setVisible(false);
		}
		else
		{
			invite_panel->setVisible(false);
			funcShare();
		}
	});

	text_invite_code = invite_info_panel->getChildByName<Text*>("text_invite_code");
	text_invite_num = invite_info_panel->getChildByName<Text*>("text_invite_num");
	text_invite_bet = invite_info_panel->getChildByName<Text*>("text_invite_bet");
	if (!text_invite_code || !text_invite_num || !text_invite_bet)
		return false;

	UserData *pUserData = UserControl::GetInstance()->GetUserData();
	text_invite_code->setString(StringUtils::format("%d", (int)pUserData->user_id));
	text_invite_num->setString(StringUtils::format("%d", (int)pUserData->children_num));
	text_invite_bet->setString("1000");

	Text* label_warn = validate_panel->getChildByName<Text*>("label_warn");
	label_warn->setString(Language::getStringByKey("MobileValidTip"));

	Text* label_phone = validate_panel->getChildByName<Text*>("label_phone");
	label_phone->setString(Language::getStringByKey("MobileNumber"));

	Text* label_code = validate_panel->getChildByName<Text*>("label_code");
	label_code->setString(Language::getStringByKey("MobileValidCode"));

	btn_bind->setTitleText(Language::getStringByKey("BindPhone"));
	//btn_send->setTitleText(Language::getStringByKey("Send"));
	btn_invite->setTitleText(Language::getStringByKey("Invite"));
	

	btn_search = friend_panel->getChildByName<Button*>("btn_search");


	btn_search->addClickEventListener([&](Ref *ref){
		funcSearch();
		//FriendControl::GetInstance()->fetchOnlineUsers();
	});

	auto search_text = friend_panel->getChildByName<Widget*>("search_text");
	if (!search_text)
	return false;

	if (!(_searchTextEditBox = EditBox::create(search_text->getSize(), Scale9Sprite::create())))
	return false;
	_searchTextEditBox->setAnchorPoint(search_text->getAnchorPoint());
	_searchTextEditBox->setPosition(search_text->getPosition());
	search_text->removeFromParent();
	_searchTextEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_searchTextEditBox->setPlaceholderFontSize(30);
	_searchTextEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_searchTextEditBox->setPlaceHolder(Language::getStringByKey("InputNickName"));
	_searchTextEditBox->setDelegate(this);
	friend_panel->addChild(_searchTextEditBox);

	friend_tip = friend_panel->getChildByName<Text*>("friend_tip");
	friend_tip->setString(Language::getStringByKey("NoFriend"));
	
	
	if (!friend_tip) return false;
	

	auto text_phone = validate_panel->getChildByName<Widget*>("text_phone");
	if (!text_phone)
	return false;

	if (!(_textPhoneEditBox = EditBox::create(text_phone->getSize(), Scale9Sprite::create())))
	return false;
	_textPhoneEditBox->setAnchorPoint(text_phone->getAnchorPoint());
	_textPhoneEditBox->setPosition(text_phone->getPosition());
	text_phone->removeFromParent();
	_textPhoneEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_textPhoneEditBox->setPlaceholderFontSize(30);
	_textPhoneEditBox->setPlaceHolder(Language::getStringByKey("InputMobileNum"));
	_textPhoneEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);

	_textPhoneEditBox->setDelegate(this);
	validate_panel->addChild(_textPhoneEditBox);

	auto text_code = validate_panel->getChildByName<Widget*>("text_code");
	if (!text_code)
	return false;

	if (!(_textCodeEditBox = EditBox::create(text_code->getSize(), Scale9Sprite::create())))
	return false;
	_textCodeEditBox->setAnchorPoint(text_code->getAnchorPoint());
	_textCodeEditBox->setPosition(text_code->getPosition());
	text_code->removeFromParent();
	_textCodeEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_textCodeEditBox->setPlaceholderFontSize(30);
	_textCodeEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_textCodeEditBox->setPlaceHolder(Language::getStringByKey("InputCode"));
	_textCodeEditBox->setDelegate(this);
	validate_panel->addChild(_textCodeEditBox);


	btnFriend = _nodeRoot->getChildByName<Button*>("btn_friend");
	btnInvite = _nodeRoot->getChildByName<Button*>("btn_invite");

	btnFriend->addClickEventListener([&](Ref *ref){
		selectPanel(FriendSelect::FRIEND_PANEL);
	});

	btnInvite->addClickEventListener([&](Ref *ref){
		selectPanel(FriendSelect::INVITE_PANEL);
	});

	friendList = (ListView*)friend_panel->getChildByName("friend_list");
	//mailList->setBackGroundColorOpacity(0);
	friendList->setItemsMargin(20);


	Button *btBack = _nodeRoot->getChildByName<Button*>("btBack");

	selectPanel(FriendSelect::FRIEND_PANEL);
	
	btBack->addClickEventListener([&](Ref *ref){
		this->removeFromParent();

		LayerDuang::panelStatus();
		LayerDuang::uiGameMenu();
		LayerDuang::dzGamesRoom();
		LayerDuang::bjlGamesRoom();
		LayerDuang::bjlPanelLobby();
		LayerDuang::dzPanelLobby();
	});

	btn_invite->addClickEventListener([&](Ref *ref){
		//Tips("邀请功能未开放");
		funcShare();
	});


	btn_send->addClickEventListener([&](Ref *ref){
		PKNotificationCenter::getInstance()->postNotification("TriggleSendPhoneCode", String::create(_textPhoneEditBox->getText()));
	});

	btn_bind->addClickEventListener([&](Ref *ref){
		PKNotificationCenter::getInstance()->postNotification("TriggleBindPhone", String::create(_textCodeEditBox->getText()));
	});


	btn_send->setEnabled(false);
	btn_send->setBright(false);

	btn_bind->setEnabled(false);
	btn_bind->setBright(false);
	
	
	
	FriendControl::GetInstance()->setFriendView(this); //添加视图

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerFriend::onUpdateFriendList), "updateFriendList", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerFriend::onWatchOtherInfo), "WatchOtherInfo", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", nullptr); //第一次 默认切换

	PKNotificationCenter::getInstance()->postNotification("GetBalanceStateNotification");
	
	return true;
}

void LayerFriend::refreshInvitePanel()
{
	//validate_panel->setVisible(!UserControl::GetInstance()->isValidPhone());
	//handler_panel->setVisible(UserControl::GetInstance()->isValidPhone());
	selectPanel(FriendSelect::INVITE_PANEL);
}

void LayerFriend::onWatchOtherInfo(Ref* pSender)
{
	LayerOthersInfo* otherInfo = LayerOthersInfo::create();

	this->addChild(otherInfo);
}

void LayerFriend::funcSearch()
{
	string searchText = _searchTextEditBox->getText();

	if (searchText.empty()) return;
	
	PKNotificationCenter::getInstance()->postNotification("TriggleLookupFriend", String::create(searchText)); //第一次 默认切换

}

void LayerFriend::funcShare()
{

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)|| (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	ShareControl::GetInstance()->showShareMenu(nullptr);//加入分享
	return;
#endif
	Tips("platform do not support share ");
}


void  LayerFriend::selectPanel(FriendSelect select)
{
	if (select == FriendSelect::FRIEND_PANEL){
		btnFriend->setEnabled(false);
		btnFriend->setBright(false);
		btnInvite->setEnabled(true);
		btnInvite->setBright(true);
		invite_panel->setVisible(false);
		invite_info_panel->setVisible(false);
		friend_panel->setVisible(true);
	}
	else{
		btnInvite->setEnabled(false);
		btnInvite->setBright(false);
		btnFriend->setEnabled(true);
		btnFriend->setBright(true);
		//invite_panel->setVisible(true);
		invite_info_panel->setVisible(true);
		friend_panel->setVisible(false);

		//validate_panel->setVisible(!UserControl::GetInstance()->isValidPhone());
		//handler_panel->setVisible(UserControl::GetInstance()->isValidPhone());
	}
}


void LayerFriend::editBoxReturn(EditBox* editBox)
{
	if (_textPhoneEditBox == editBox){
		
		string phone = _textPhoneEditBox->getText();
		
		if (Comm::RegexIsMatch(phone.c_str(), "^1[0-9]{10}$"))
		{
			btn_send->setEnabled(true);
			btn_send->setBright(true);
		}
		else{
			btn_send->setEnabled(false);
			btn_send->setBright(false);
		}

	}
	 if (_textCodeEditBox == editBox){

		if (btn_send->isEnabled())
		{
			string code = _textCodeEditBox->getText(); //  code 需要正则验证

			if (code.size() > 0){
				btn_bind->setEnabled(true);
				btn_bind->setBright(true);
			}
			else{
				btn_bind->setEnabled(false);
				btn_bind->setBright(false);
			}
		}

	}

}


void LayerFriend::onUpdateFriendList(Ref* pSender)
{
	vector<FriendData*> list = FriendControl::GetInstance()->getFriendList();
	updateFriendList(list);
}

void LayerFriend::updateFriendList(vector<FriendData*> list)
{
	if (!friendList)
		return;

	for (int i = 0, length = list.size(); i < length; ){  //数据已顺序排列
		FriendItem *item = (FriendItem *)friendList->getItem(i);// list item 重复利用
		if (item == nullptr)
		{
			item = new FriendItem();// list item 重复利用
			item->setTouchEnabled(true);
			friendList->pushBackCustomItem(item);//少了加
		}
		item->reset();

		item->setProperty(list[i], 1);

		if (i + 1 < length){
			item->setProperty(list[i+1], 2);
		}

		if (i + 2 < length)
		{
			item->setProperty(list[i + 2], 3);
		}

		i = i + 3;
	}
	int mi = friendList->getChildrenCount() - ceil(list.size()/3.0);// 多出item 数目
	while (mi-- > 0)
	{
		friendList->removeLastItem();

	}

	friendList->refreshView();//记得刷新，更新innerContainer size
	friendList->jumpToTop();

	friend_tip->setVisible(list.size() <= 0);
}

LayerFriend::~LayerFriend()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
	 FriendControl::GetInstance()->setFriendView(nullptr); //释放视图
}

void LayerFriend::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
