#include "LayerFriendInvite.h"
#include "cocostudio/CocoStudio.h"
#include "FriendControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "ApiUser.h"
#include "DZControl.h"
#include "LayerLoading.h"

FriendInviteItem::FriendInviteItem()
{
	_nodeRoot = CSLoader::createNode("friend/FriendInviteItem.csb");
	addChild(_nodeRoot);
	this->setContentSize(Size(1760, 250));
	
}


void FriendInviteItem::reset()
{
	for (int i = 1; i <= 3; i++){
		Node* itemNode = _nodeRoot->getChildByName(StringUtils::format("node_%d", i));
		itemNode->setVisible(false);
	}
}

CheckBox* FriendInviteItem::setProperty(FriendData* friendData, int i)
{


	Node* itemNode = _nodeRoot->getChildByName(StringUtils::format("node_%d",i));
	if (itemNode && friendData){
	
		itemNode->setVisible(true);

		CheckBox* select_box = itemNode->getChildByName<CheckBox*>("select_box");
		select_box->setName(toString(friendData->user_id));
		
		select_box->addEventListenerCheckBox(this, checkboxselectedeventselector(FriendInviteItem::selectedEvent));

		Text* name = itemNode->getChildByName<Text*>("name");
		name->setString(friendData->nickname);
		Sprite* avatar = itemNode->getChildByName<Sprite*>("spAvatar");
		avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(friendData->avatar, 0));
		return select_box;
	}
	return nullptr;
}


void FriendInviteItem::selectedEvent(cocos2d::Ref *pSender, CheckBoxEventType type)
{
	switch (type) {
	case cocos2d::ui::CHECKBOX_STATE_EVENT_SELECTED:
	{
		currentClickCheckBox = (CheckBox*)pSender;
		PKNotificationCenter::getInstance()->postNotification("SelectFriend",this ); //第一次 默认切换

		break;
	}
	case cocos2d::ui::CHECKBOX_STATE_EVENT_UNSELECTED:
	{	
		PKNotificationCenter::getInstance()->postNotification("SelectFriend", this ); //第一次 默认切换
	}
	break;
	default:
		break;
	}
}

FriendInviteItem::~FriendInviteItem()
{
}


LayerFriendInvite::LayerFriendInvite() :from(0), select_user_id(0), lastClickCheckBox(nullptr){

}

bool LayerFriendInvite::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("friend/LayerFriendInvite.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;

	friend_panel = _nodeRoot->getChildByName<Layout*>("friend_panel");


	if ( !friend_panel ){
		return false;
	}

	friend_tip = friend_panel->getChildByName<Text*>("friend_tip");
	friend_tip->setString(Language::getStringByKey("NoFriend"));
	btnInvite = friend_panel->getChildByName<Button*>("btn_invite");

	btnInvite->setEnabled(false);
	btnInvite->setBright(false);

	btnInvite->setTitleText(Language::getStringByKey("Confirm"));

	btnInvite->addClickEventListener([&](Ref *ref){
		btnInvite->setEnabled(false);
		btnInvite->setBright(false);
		funcInvite();
	});

	friendList = (ListView*)friend_panel->getChildByName("friend_list");
	//mailList->setBackGroundColorOpacity(0);
	friendList->setItemsMargin(20);


	Button *btBack = _nodeRoot->getChildByName<Button*>("btBack");


	btBack->addClickEventListener([&](Ref *ref){
		this->removeFromParent();
	});

	FriendControl::GetInstance()->setFriendView(this); //添加视图

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerFriendInvite::onSelectFriend), "SelectFriend", nullptr);//监听来自视图的事件

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerFriendInvite::onUpdateFriendList), "updateFriendList", nullptr);//监听来自视图的事件
	PKNotificationCenter::getInstance()->postNotification("TriggleGetFriendList", nullptr); //第一次 默认切换
	
	return true;
}


void LayerFriendInvite::onSelectFriend(Ref *ref)
{
	
	FriendInviteItem* friendInviteItem = (FriendInviteItem*)ref;
	if (!friendInviteItem) return;
	
	switch (from)
	{
	case FROMPAGE::WAIT_INVITE_PAGE:
	{

	}
		break;
	case FROMPAGE::WRITE_MAIL_PAGE:
	{
		if (this->lastClickCheckBox != nullptr){
			this->lastClickCheckBox->setSelected(false);
		}

		this->lastClickCheckBox = friendInviteItem->currentClickCheckBox;

		select_user_id = atoi(friendInviteItem->currentClickCheckBox->getName().c_str());

		if (this->lastClickCheckBox->isSelected()){
			PKNotificationCenter::getInstance()->postNotification("TriggleSetMailTo", String::create(toString(select_user_id))); //第一次 默认切换
		}
		else{
			PKNotificationCenter::getInstance()->postNotification("TriggleSetMailTo", nullptr); //第一次 默认切换
		}
		
	}
		break;

	default:break;
	}
}
void LayerFriendInvite::setFrom(FROMPAGE from)
{
	this->from = from;
	switch (from)
	{
		case FROMPAGE::WAIT_INVITE_PAGE :
		{
			btnInvite->setTitleText(Language::getStringByKey("InviteFriend"));
		}
			 break;
		case FROMPAGE::WRITE_MAIL_PAGE :
		{
			btnInvite->setTitleText(Language::getStringByKey("Confirm"));
		}
			 break;

		default:break;
	}
	
}

void LayerFriendInvite::funcInvite()
{
	switch (from)
	{
	case FROMPAGE::WAIT_INVITE_PAGE:
	{
		//写邀请邮件
		writeInviteMail();
		this->removeFromParent();
	}
		break;
	case FROMPAGE::WRITE_MAIL_PAGE:
	{
		this->removeFromParent();
	}
		break;

	default:break;
	}
}

void LayerFriendInvite::writeInviteMail()
{
	LayerLoading::Wait();
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	string nickname = UserControl::GetInstance()->GetUserData()->nickname;
	string remark = nickname+ "请您一起来玩德州扑克";
	for (CheckBox* box : boxList){

		if (!box->isSelected()) continue;

		int to_user_id = atoi(box->getName().c_str());
		int gameType = DZControl::GetInstance()->gameRoomType;
		string gameToken = DZControl::GetInstance()->gameToken;
		
		if (!Api::User::sendFriendInvite(user_id, to_user_id, gameType, gameToken.c_str(), remark.c_str()))
		{
			CCLOG(" sendFriendInvite net error");
		}
	}
	Tips("邀请邮件已发送");
	LayerLoading::Close();
}

void LayerFriendInvite::onUpdateFriendList(Ref* pSender)
{
	vector<FriendData*> list = FriendControl::GetInstance()->getFriendList();
	updateFriendList(list);
}


void LayerFriendInvite::updateFriendList(vector<FriendData*> list)
{
	boxList.clear();

	for (int i = 0, length = list.size(); i < length; ){  //数据已顺序排列
		FriendInviteItem *item = (FriendInviteItem *)friendList->getItem(i);// list item 重复利用
		if (item == nullptr)
		{
			item = new FriendInviteItem();// list item 重复利用
			friendList->pushBackCustomItem(item);//少了加
		}
		item->reset();

		boxList.push_back(item->setProperty(list[i], 1));
		

		if (i + 1 < length){
			boxList.push_back(item->setProperty(list[i + 1], 2));
		}

		if (i + 2 < length)
		{
			boxList.push_back(item->setProperty(list[i + 2], 3));
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

	btnInvite->setEnabled(list.size() > 0);
	btnInvite->setBright(list.size() > 0);
	friend_tip->setVisible(list.size() <= 0);
}

LayerFriendInvite::~LayerFriendInvite()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void LayerFriendInvite::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
