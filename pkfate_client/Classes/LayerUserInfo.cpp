#include "LayerUserInfo.h"
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
// on "init" you need to initialize your instance
bool LayerUserInfo::init()
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
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerUserInfo.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

    select_avatar = RootNode->getChildByName<Layout*>("select_avatar");

	select_avatar->setVisible(false);
	
	listAvatars = select_avatar->getChildByName<ListView*>("listAvatars");
	
	if (!listAvatars)
		return false;

	label_change_bg = RootNode->getChildByName<TextBMFont*>("label_change_bg");
		

	txtLevel = RootNode->getChildByName<TextBMFont*>("txtLevel");
	nick_name = RootNode->getChildByName<Text*>("nick_name");
	info_scrollview = RootNode->getChildByName<ScrollView*>("info_scrollview");
	label_tip = RootNode->getChildByName<Text*>("label_tip");

	Layout* year_info = RootNode->getChildByName<Layout*>("year_info");

	year_fortune = year_info->getChildByName<Text*>("year_fortune");
	year_winRate = year_info->getChildByName<Text*>("year_winRate");
	year_profit = year_info->getChildByName<Text*>("year_profit");
	avatar = RootNode->getChildByName<Sprite*>("avatar");
	txtCoins = RootNode->getChildByName<TextBMFont*>("txtCoins");

	if (!txtLevel || !nick_name || !label_tip || !info_scrollview
		|| !year_fortune || !year_winRate || !year_profit || !avatar) return false;

	

	label_change = RootNode->getChildByName<Text*>("label_change");

	label_change->setString(Language::getStringByKey("ChangeAvatar"));

	label_tip->setString(Language::getStringByKey("ClickSelectAvatar"));
	label_tip->setVisible(false);

	Text* label_year_fortune = year_info->getChildByName<Text*>("label_year_fortune");

	label_year_fortune->setString(Language::getStringByKey("YearFortuneRank"));

	Text* label_year_winRate = year_info->getChildByName<Text*>("label_year_winRate");

	label_year_winRate->setString(Language::getStringByKey("YearWinRank"));

	Text* label_year_profit = year_info->getChildByName<Text*>("label_year_profit");

	label_year_profit->setString(Language::getStringByKey("YearProfitRank"));

	////bjl

	bjl_node = info_scrollview->getChildByName("bjl_node");

	Text* label_type = bjl_node->getChildByName<Text*>("label_type");
	label_type->setString(Language::getStringByKey("Bacarat"));

	Text* label_total = bjl_node->getChildByName<Text*>("label_total");
	label_total->setString(Language::getStringByKey("TotalRound"));

	Text* label_winRate = bjl_node->getChildByName<Text*>("label_winRate");
	label_winRate->setString(Language::getStringByKey("WinRate"));

	Text* label_profit = bjl_node->getChildByName<Text*>("label_profit");
	label_profit->setString(Language::getStringByKey("Profit"));

	////dz
	dz_node = info_scrollview->getChildByName("dz_node");

	Text* dz_label_type = dz_node->getChildByName<Text*>("label_type");
	dz_label_type->setString(Language::getStringByKey("TexasPoker"));

	Text* dz_label_total = dz_node->getChildByName<Text*>("label_total");
	dz_label_total->setString(Language::getStringByKey("TotalRound"));

	Text* dz_label_winRate = dz_node->getChildByName<Text*>("label_winRate");
	dz_label_winRate->setString(Language::getStringByKey("WinRate"));

	Text* dz_label_profit = dz_node->getChildByName<Text*>("label_profit");
	dz_label_profit->setString(Language::getStringByKey("Profit"));

	auto userData = UserControl::GetInstance()->GetUserData();

	txtLevel->setString(toString(userData->level));
	nick_name->setString(userData->nickname);

	touch_layer = RootNode->getChildByName<Layout*>("touch_layer");


	if (!touch_layer)
		return false;


	SetBalanceData(nullptr);//default


	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerUserInfo::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerUserInfo::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	listAvatars->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(LayerUserInfo::selectedItemEvent, this));



	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerUserInfo::onSetAvatarCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::SET_AVATAR), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerUserInfo::SetBalanceData), "UpdateBalanceUserInfo", NULL);
	
	
	UserControl::GetInstance()->setInfoView(this);


	lookGameData(UserControl::GetInstance()->GetUserData()->user_id);
	return true;
}


void LayerUserInfo::lookUserInfo(int user_id, string nickname, int _avatar)
{
	if (user_id != UserControl::GetInstance()->GetUserData()->user_id){

		avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(_avatar, 0));
		nick_name->setString(nickname);

		txtCoins->setVisible(false);
		label_change_bg->setVisible(false);
		label_change->setVisible(false);
		lookGameData(user_id);
	}
}

void LayerUserInfo::lookGameData(int user_id)
{
	PKNotificationCenter::getInstance()->postNotification("TriggerLookFriend", String::create(toString(UserControl::GetInstance()->GetUserData()->user_id)));
}

void  LayerUserInfo::updateGameDatas(map<int, GameData*> tempMap)
{
	int index = 0;
	GameData *bjl_data = tempMap[GAMETYPE::BJL];
	if (bjl_data){
		index += bjl_data->total_round;
		bjl_node->getChildByName<Text*>("total")->setString(toString(bjl_data->total_round));
		bjl_node->getChildByName<Text*>("winRate")->setString(StringUtils::format("%0.1f%%", 100.0*bjl_data->win_round / bjl_data->total_round));
		bjl_node->getChildByName<Text*>("profit")->setString(toString(bjl_data->bonus));
	}

	GameData *dz_data = tempMap[GAMETYPE::DZPK];

	if (dz_data){
		index += dz_data->total_round;
		dz_node->getChildByName<Text*>("total")->setString(toString(dz_data->total_round));
		dz_node->getChildByName<Text*>("winRate")->setString(StringUtils::format("%0.1f%%", 100.0*dz_data->win_round / (dz_data->total_round)));
		dz_node->getChildByName<Text*>("profit")->setString(toString(dz_data->bonus));
	}
	txtLevel->setString(toString(index));
}


//设置balance相关信息
void LayerUserInfo::SetBalanceData(Ref *ref)
{
	
	BalanceData *data = BillControl::GetInstance()->GetBalanceData(0);
	if (data)
	{
		//设置balance
		txtCoins->setString(Comm::GetShortStringFromInt64(data->balance));
		
	}
}




void LayerUserInfo::funcChangeAvatar()
{
	//只设置头像
	int s = UserControl::GetInstance()->GetUserData()->avatar;
	if (UserControl::GetInstance()->GetUserData()->avatar != _avatarId)
	{
		

		LayerLoading::Wait();

		if (!Api::User::SetAvatar(_avatarId)){
			LayerLoading::CloseWithTip("send setAvatar msg net error");
			CCLOG("[%s]:%s", "LayerSetNickName::btEnterClicked", "send setAvatar msg error");
		}
			
	}
	else
	{
		//this->removeFromParent();
	}
		
}



void LayerUserInfo::onEnter()
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
	
	////如果已经存在昵称，显示当前数据，只允许修改头像，且可以退出页面
	UserData *data = UserControl::GetInstance()->GetUserData();
	if (data&&!data->nickname.empty())
	{
		//设置头像
		Sprite*spAvatar = AvatarControl::GetInstance()->GetAvatarSprite(data->avatar);
		if (spAvatar)
		{
			avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(UserControl::GetInstance()->GetUserData()->avatar, 0));
			_avatarId = data->avatar;
		}
		
	}
}


void LayerUserInfo::onSetAvatarCallBack(Ref *pSender)
{
	//SetAvatar回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		/*auto layerLoading = this->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
		{
			layerLoading->removeFromParent();
		}*/

		LayerLoading::Close();

		if (msg->code == 0)
		{
			// SetAvatar ok
			
			Api::User::GetUserInfo();		//申请一次新的数据，scene会自动更新数据
			avatar->setSpriteFrame(AvatarControl::GetInstance()->GetAvatarSpriteName(_avatarId, 0));
			Tips(Language::getStringByKey("ChangeSucc"));
		}
		else
		{
			// SetAvatar fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerUserInfo::onSetAvatarCallBack", "set avatar error.", msg->code, msg->data.c_str());
			switch (msg->code)
			{
			default:
				//lbNickName->setString(Language::getStringByKey("ErrorTry"));
				Tips(Language::getStringByKey("InputError"));
				break;
			}
		}
		msg->release();
	});
}

void LayerUserInfo::selectedItemEvent(Ref *pSender, ListView::EventType type)
{
	switch (type)
	{
	case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_START:
	{
	
		break;
	}
	case cocos2d::ui::ListView::EventType::ON_SELECTED_ITEM_END:
	{
		ListView* listView = static_cast<ListView*>(pSender);
		int avatarId = listView->getItem(listView->getCurSelectedIndex())->getTag();
		CCLOG("select avatar index = %ld", avatarId);
		if (_avatarId != avatarId){
			_avatarId = avatarId;
			funcChangeAvatar();
			
		}
		break;
	}
	default:
		break;
	}
}


void LayerUserInfo::onExit()
{
	//注销消息通知
	UserControl::GetInstance()->setInfoView(nullptr);
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Layer::onExit();
}


bool LayerUserInfo::onTouchBegan(Touch *touch, Event *event){
	if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation())){
		this->scheduleOnce([=](float){CloseWithAction(this); }, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	}
	else{
		Rect  rect = avatar->getTextureRect();
		Size size = avatar->getContentSize();
		Vec2 nodePoint = avatar->convertToNodeSpace(touch->getLocation());

		if (nodePoint.x<size.width && nodePoint.y<size.height)
		{
			select_avatar->setVisible(true);
			info_scrollview->setVisible(false);
			label_tip->setString("点击选择头像");
			label_tip->setVisible(true);
		}
	}
	return true;
}



bool LayerOthersInfo::init()
{
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	_avatarId = -1;
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerUserInfo.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

	select_avatar = RootNode->getChildByName<Layout*>("select_avatar");

	select_avatar->setVisible(false);

	listAvatars = select_avatar->getChildByName<ListView*>("listAvatars");

	if (!listAvatars)
		return false;

	label_change_bg = RootNode->getChildByName<TextBMFont*>("label_change_bg");


	txtLevel = RootNode->getChildByName<TextBMFont*>("txtLevel");
	nick_name = RootNode->getChildByName<Text*>("nick_name");
	info_scrollview = RootNode->getChildByName<ScrollView*>("info_scrollview");
	label_tip = RootNode->getChildByName<Text*>("label_tip");

	Layout* year_info = RootNode->getChildByName<Layout*>("year_info");

	year_fortune = year_info->getChildByName<Text*>("year_fortune");
	year_winRate = year_info->getChildByName<Text*>("year_winRate");
	year_profit = year_info->getChildByName<Text*>("year_profit");
	avatar = RootNode->getChildByName<Sprite*>("avatar");
	txtCoins = RootNode->getChildByName<TextBMFont*>("txtCoins");

	if (!txtLevel || !nick_name || !label_tip || !info_scrollview
		|| !year_fortune || !year_winRate || !year_profit || !avatar) return false;



	label_change = RootNode->getChildByName<Text*>("label_change");

	label_change->setString(Language::getStringByKey("ChangeAvatar"));

	label_tip->setString(Language::getStringByKey("ClickSelectAvatar"));
	label_tip->setVisible(false);

	Text* label_year_fortune = year_info->getChildByName<Text*>("label_year_fortune");

	label_year_fortune->setString(Language::getStringByKey("YearFortuneRank"));

	Text* label_year_winRate = year_info->getChildByName<Text*>("label_year_winRate");

	label_year_winRate->setString(Language::getStringByKey("YearWinRank"));

	Text* label_year_profit = year_info->getChildByName<Text*>("label_year_profit");

	label_year_profit->setString(Language::getStringByKey("YearProfitRank"));

	////bjl

	bjl_node = info_scrollview->getChildByName("bjl_node");

	Text* label_type = bjl_node->getChildByName<Text*>("label_type");
	label_type->setString(Language::getStringByKey("Bacarat"));

	Text* label_total = bjl_node->getChildByName<Text*>("label_total");
	label_total->setString(Language::getStringByKey("TotalRound"));

	Text* label_winRate = bjl_node->getChildByName<Text*>("label_winRate");
	label_winRate->setString(Language::getStringByKey("WinRate"));

	Text* label_profit = bjl_node->getChildByName<Text*>("label_profit");
	label_profit->setString(Language::getStringByKey("Profit"));

	////dz
	dz_node = info_scrollview->getChildByName("dz_node");

	Text* dz_label_type = dz_node->getChildByName<Text*>("label_type");
	dz_label_type->setString(Language::getStringByKey("TexasPoker"));

	Text* dz_label_total = dz_node->getChildByName<Text*>("label_total");
	dz_label_total->setString(Language::getStringByKey("TotalRound"));

	Text* dz_label_winRate = dz_node->getChildByName<Text*>("label_winRate");
	dz_label_winRate->setString(Language::getStringByKey("WinRate"));

	Text* dz_label_profit = dz_node->getChildByName<Text*>("profit");
	dz_label_profit->setString(Language::getStringByKey("Profit"));

	auto userData = UserControl::GetInstance()->GetUserData();

	txtLevel->setString(toString(userData->level));
	nick_name->setString(userData->nickname);

	touch_layer = RootNode->getChildByName<Layout*>("touch_layer");


	if (!touch_layer)
		return false;


	SetBalanceData(nullptr);//default


	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerUserInfo::onTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerUserInfo::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);



	//listAvatars->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(LayerUserInfo::selectedItemEvent, this));

	UserControl::GetInstance()->setInfoView(this);

	return true;
}


bool LayerOthersInfo::onTouchBegan(Touch *touch, Event *event){
	if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation())){
		this->scheduleOnce([=](float){CloseWithAction(this); }, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	}
	else{
	
	}
	return true;
}