#include "SceneGames.h"
#include "LayerSetting.h"
#include "LayerSetNickName.h"
#include "UserControl.h"
#include "BillControl.h"
#include "MailControl.h"
#include "AvatarControl.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "ApiBill.h"
#include "LayerBjlGames.h"
#include "SceneReports.h"
#include "SceneLogin.h"
#include "LayerSetting.h"
#include "PKNotificationCenter.h"
#include "LayerMall.h"
#include "LayerRank.h"
#include "LayerUserInfo.h"
#include "LayerTask.h"
#include "TaskControl.h"
#include "LayerLottery.h"
#include "LayerDzGames.h"
#include "LayerGuideMask.h"
#include "SoundControl.h"
#include "LayerNotice.h"
#include "LayerFriend.h"
#include "LayerMail.h"
#include "ApiGame.h"
#include "LayerPassword.h"
#include "LayerNotice.h"
#include "LayerDuang.h"
#include "LayerLoading.h"
#include "SigninControl.h"
#include "LayerSignin.h"

// on "init" you need to initialize your instance
bool SceneGames::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}
	this->setName("SceneGame");
	isMoved = false;
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("SceneGames.csb");
	if (!RootNode)
		return false;
	panelStatus = (Layout*)RootNode->getChildByName("panelStatus");
	if (!panelStatus)
		return false;
	imgHead = (Sprite*)panelStatus->getChildByName("imgHead");
	if (!imgHead)
		return false;
	txtNickName = panelStatus->getChildByName<Text*>("txtNickName");
	if (!txtNickName)
		return false;
	txtCoins = panelStatus->getChildByName<TextBMFont*>("txtCoins");
	if (!txtCoins)
		return false;
	if (!(txtLevel = panelStatus->getChildByName<TextBMFont*>("txtLevel")))
		return false;
	txtLevel->setString("0");
	imgLevelBar = (LoadingBar*)panelStatus->getChildByName("imgLevelBar");
	if (!imgLevelBar)
		return false;
	imgLevelBar->setPercent(0);
	panelContent = RootNode->getChildByName<Layout*>("panelContent");
	if (!panelContent)
		return false;
	listGames = panelContent->getChildByName<ListView*>("listGames");
	if (!listGames)
		return false;
	//add listview event
	listGames->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(SceneGames::onSelectedRoomEvent, this));
	btnBuy = (Button*)panelStatus->getChildByName("btnBuy");
	if (!btnBuy)
		return false;


//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS||CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	btnBuy->setVisible(true);
	btnBuy->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		PKNotificationCenter::getInstance()->postNotification("ShowMall");
	});
	

//#endif
	btnSetting = (Button*)panelStatus->getChildByName("btnSettings");
	if (!btnSetting)
		return false;
	btnSetting->setPressedActionEnabled(true);
	btnSetting->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerSetting * setting = this->getChildByName<LayerSetting*>("LayerSetting");
		if (!setting){
			LayerSetting * setting = LayerSetting::create(); //临时创建释放 avoid texture memory overflow
			setting->setName("LayerSetting");
			this->addChild(setting, 2);
			OpenWithAlert(setting);
		}

	});
	btnTask = (Button*)panelStatus->getChildByName("btnTask");
	if (!btnTask)
		return false;
	btnTask->setPressedActionEnabled(true);
	btnTask->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerTask * task = this->getChildByName<LayerTask*>("LayerTask");
		if (!task){
			LayerTask * task = LayerTask::create(); //临时创建释放 avoid texture memory overflow
			task->setName("LayerTask");
			this->addChild(task, 2);
			OpenWithAlert(task);
		}

	});
	
	//报表
	btnReport = panelStatus->getChildByName<Button*>("btnReport");
	if (!btnReport)
		return false;
	btnReport->setPressedActionEnabled(true);
	btnReport->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		SceneReports* scene = SceneReports::create();
		scene->setName("SceneReports");
		this->addChild(scene, 1);
	});
	
	//排行
	btnRank = (Button*)panelStatus->getChildByName("btnRank");
	if (!btnRank)
		return false;
	btnRank->setPressedActionEnabled(true);
	btnRank->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		showRank(nullptr);
	});

	//好友
	btnFriend = (Button*)panelStatus->getChildByName("btnFriend");
	if (!btnFriend)
		return false;
	btnFriend->setPressedActionEnabled(true);
	btnFriend->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerFriend * layerFriend = this->getChildByName<LayerFriend*>("LayerFriend");
		if (!layerFriend){
			layerFriend = LayerFriend::create(); //临时创建释放 avoid texture memory overflow
			layerFriend->setName("LayerFriend");
			this->addChild(layerFriend, 2);
		}
	});


	//邮件
	btnMail = (Button*)panelStatus->getChildByName("btnMail");
	if (!btnMail)
		return false;
	btnMail->setPressedActionEnabled(true);
	btnMail->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerMail * mail = this->getChildByName<LayerMail*>("LayerMail");
		if (!mail){
			mail = LayerMail::create(); //临时创建释放 avoid texture memory overflow
			mail->setName("LayerMail");
			this->addChild(mail, 2);
		}
	});


	btnQuit = (Button*)panelStatus->getChildByName("btnQuit");
	if (!btnQuit)
		return false;
	btnQuit->setPressedActionEnabled(true);
	btnQuit->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CLOSE);
		UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
        SocketControl::GetInstance()->close();
		auto scene = SceneLogin::create();
		Director::getInstance()->replaceScene(scene);
	});


	this->addChild(RootNode);
	

	//初始化billcontrol以便注册回调函数，再申请levelcofnig
	if (!BillControl::GetInstance() || !Api::Bill::GET_LEVEL_CONFIG())
		return false;
	
	// Make sprite imgHead touchable
	auto listener1 = EventListenerTouchOneByOne::create();
	listener1->setSwallowTouches(true);
	listener1->onTouchBegan = [=](Touch* touch, Event* event){
		auto target = static_cast<Sprite*>(event->getCurrentTarget());
		Vec2 locationInNode = target->convertToNodeSpace(touch->getLocation());
		Size s = target->getContentSize();
		Rect rect = Rect(0, 0, s.width, s.height);
		if (rect.containsPoint(locationInNode))
			return true;
		else
			return false;
	};
	listener1->onTouchEnded = [=](Touch* touch, Event* event){
		/*LayerSetNickName *layer = LayerSetNickName::create();*/
		LayerUserInfo *userInfo = this->getChildByName<LayerUserInfo*>("LayerUserInfo");
		if (!userInfo){
			userInfo = LayerUserInfo::create();
			if (userInfo){
				userInfo->setName("LayerUserInfo");
				this->addChild(userInfo, 1);
				OpenWithAlert(userInfo);
			}
		}
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, imgHead);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::UpdateSceneGameInfo), "UpdateSceneGameInfo", NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::SetUserInfo), "UpdateUserInfo", NULL);
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::triggerSwitchGameType), "triggerSwitchGameType", NULL);


	//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS||CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::showMall), "ShowMall", NULL);
	//#endif

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::triggerLoginAward), "TriggerLoginAward", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::triggerRequestTask), "TriggerRequestTask", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::triggerRequestNotice), "TriggerRequestNotice", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::TriggerLotteryTask), "TriggerLotteryTask", NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::TriggerShowNotice), "TriggerShowNotice", NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::triggerMailUnRead), "TriggerMailUnRead", NULL);
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::onOpenSigninUINotification), "OpenSigninUINotification", NULL);

	SigninControl::GetInstance()->requestSigninList();

	//判断是否设置昵称
	if (UserControl::GetInstance()->GetUserData()->nickname.empty() || UserControl::GetInstance()->GetUserData()->nickname == "null")
	{
		LayerSetNickName *layerSetNickName = LayerSetNickName::create();
		
		//TaskControl::GetInstance()->requestAndCacheTask();//设置名字退出后拉取任务,验证登录奖励
		if (layerSetNickName)
			this->addChild(layerSetNickName);
	}
	else
	{
		SetUserInfo(nullptr);
		
		PKNotificationCenter::getInstance()->postNotification("DoneSigninTaskNotification");
	}
	
	currentPanel = nullptr;

	EventListenerKeyboard* keyListener = EventListenerKeyboard::create();

	keyListener->onKeyReleased = CC_CALLBACK_2(SceneGames::onKeyReleased, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);
	
	UserControl::GetInstance()->requestGameIndex();
	UserControl::GetInstance()->requestGames();
	BillControl::GetInstance()->requestBalance();
	MailControl::GetInstance()->requestMail();//请求邮件
	FriendControl::GetInstance()->requestFriend();//请求好友

	return true;
}


void SceneGames::UpdateSceneGameInfo(Ref* ref)
{
	addGames();
}


void SceneGames::triggerMailUnRead(Ref *ref)
{

	int count = MailControl::GetInstance()->getUnReadMailCount();
	btnMail->getChildByName("warn")->setVisible(count > 0);
	Text* textCount = btnMail->getChildByName<Text*>("count");
	textCount->setString(toString(count));
	textCount->setVisible(count > 0);
}

void SceneGames::TriggerLotteryTask(Ref *ref)
{
	btnTask->getChildByName("warn")->setVisible(TaskControl::GetInstance()->isTaskAward());
}

void SceneGames::triggerRequestTask(Ref *ref)
{
	TaskControl::GetInstance()->requestAndCacheTask();//拉取任务,验证登录奖励
}

void SceneGames::triggerRequestNotice(Ref *ref)
{
	MailControl::GetInstance()->requestNotice();//拉取通知
}


void SceneGames::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event){
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{

		if (GuideControl::GetInstance()->checkInGuide()) return;

		SceneReports* report = this->getChildByName<SceneReports*>("SceneReports");

		if (report){
			report->removeFromParent();
		}
		else{
			if (currentPanel){
				PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::NO)));
			}

			else{
				UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
				SocketControl::GetInstance()->close();
				auto scene = SceneLogin::create();
				Director::getInstance()->replaceScene(scene);
			}
		}

	}
};


SceneGames::~SceneGames()
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	
}
void SceneGames::onEnter()
{
	Scene::onEnter();
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneGames::SetBalanceData), "UpdateBalance", NULL);
	LayerLoading::Wait("", 5.0f, true);
	Api::Bill::GET_BALANCE();		//每次进入时，刷新用户余额
	
	SoundControl::PlayMusic(BGM::BGM_HALL);

	//if Baccarat Game don't play effects
	LayerBjlGames *pLayerBjlGames = dynamic_cast<LayerBjlGames*>(getCurrentPanel());
	if (pLayerBjlGames != NULL)
		return;
	
	if (listGames->getChildrenCount() > 0){
		playAction();
	}
}


void SceneGames::rescue(float dt)
{
	TaskControl::GetInstance()->rescue();
}

void SceneGames::playAction()
{
	LayerDuang::uiBorderAppear(panelStatus);
	LayerDuang::uiGameMenu();
	LayerDuang::dzGamesRoom();
	LayerDuang::bjlGamesRoom();
	LayerDuang::bjlPanelLobby();
	LayerDuang::dzPanelLobby();
}

void SceneGames::triggerLoginAward(Ref* pSender)
{
	LayerLottery* layerLottery = LayerLottery::create();
	this->addChild(layerLottery,2);
}

void SceneGames::TriggerShowNotice(Ref* pSender)
{
	if (MailControl::GetInstance()->isLoginNotice()){
		LayerNotice* layerNotice = LayerNotice::create();
		this->addChild(layerNotice);
	}
}

void SceneGames::onExit(){
	
	Scene::onExit();
}
//设置balance相关信息
void SceneGames::showRank(Ref *ref)
{
	LayerRank * rank = LayerRank::create(); //临时创建释放 avoid texture memory overflow
	this->addChild(rank, 2);
	OpenWithAlert(rank);
}

//设置balance相关信息
void SceneGames::showMall(Ref *ref)
{   
	LayerMall * mall = this->getChildByName<LayerMall*>("LayerMall");
	if (!mall)
	{
		LayerMall * mall = LayerMall::create(); //临时创建释放 avoid texture memory overflow
		mall->setName("LayerMall");
		this->addChild(mall, 2);
		OpenWithAlert(mall);
	}
}
//设置用户相关信息
void SceneGames::SetUserInfo(Ref *ref)
{
	UserData *data = UserControl::GetInstance()->GetUserData();
	if (data)
	{
		//设置头像
		Sprite *spAvatar = AvatarControl::GetInstance()->GetAvatarSprite(data->avatar);
		if (spAvatar)
		{
			imgHead->setDisplayFrame(spAvatar->getSpriteFrame());
		}
		//设置昵称
		txtNickName->setString(data->nickname);

		
	}
}
//设置balance相关信息
void SceneGames::SetBalanceData(Ref *ref)
{

	BalanceData *balanceData = BillControl::GetInstance()->GetBalanceData(0);

	if (!balanceData) return;

	LayerLoading::Close(true);
	//商城打开时，需要更新商城余额

	if (this->getChildByName("LayerMall"))
		PKNotificationCenter::getInstance()->postNotification("UpdateBalanceMall");
	if (this->getChildByName("LayerUserInfo"))
		PKNotificationCenter::getInstance()->postNotification("UpdateBalanceUserInfo");

	
		//设置balance
		//txtCoins->setString(StringUtils::format("%.0f", data->balance));
	txtCoins->setString(Comm::GetShortStringFromInt64(balanceData->balance));

	UserData *data = UserControl::GetInstance()->GetUserData();
	txtLevel->setString(toString(data->level));
	imgLevelBar->setPercent(data->expRate * 100);

	this->unschedule(schedule_selector(SceneGames::rescue));
	if (balanceData->balance < 200)
	{
		this->scheduleOnce(schedule_selector(SceneGames::rescue), 1.0f);
	}
}
void SceneGames::addGames()
{
	Layout* default_item = Layout::create();
	default_item->setTouchEnabled(true);
	listGames->setItemModel(default_item);
	auto listGameSize=listGames->getContentSize();
	listGames->pushBackDefaultItem();
	//第一个排版距离左边有空白，直接插入一个空白的元素
	auto firstHolder = listGames->getItems().back();
	firstHolder->setSize(Size(60,listGameSize.height));
	std::vector<GAMEINFO*>& gameInfos = UserControl::GetInstance()->getGameInfos();
	for (GAMEINFO *info : gameInfos)
	{
		Sprite *sp = Sprite::create("img/bg_game_in.png");

		Sprite *sp_icon = Sprite::create(StringUtils::format("img/%s_icon.png", info->bgImgUrl));

		Sprite *labelSp = Sprite::create(StringUtils::format("img/label_%s.png", info->bgImgUrl));
		auto itemSize=sp->getContentSize();
		//sp->setAnchorPoint(Vec2(0, 0));
		//宽度增加22是每个项之间的距离
		sp->setPosition(Vec2((itemSize.width+22)/2,listGameSize.height/2+30));
		
		if (labelSp != NULL)
			labelSp->setPosition(Vec2((itemSize.width) / 2, listGameSize.height - 90));
		
		listGames->pushBackDefaultItem();
		auto layout = listGames->getItems().back();
		layout->setUserData(info);
		layout->setSize(Size(itemSize.width+22,listGameSize.height));
		layout->addChild(sp);

		if (labelSp != NULL)
			layout->addChild(labelSp);
	}

	playAction();

}


void SceneGames::onSelectedRoomEvent(Ref *pSender, ListView::EventType type)
{
	
	if (type != ListView::EventType::ON_SELECTED_ITEM_END)
		return;
	auto layout = listGames->getItem(listGames->getCurSelectedIndex());
	GAMEINFO *info = (GAMEINFO*)layout->getUserData();
	if(!info)
		return;

	PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(info->type)));

}


void SceneGames::triggerSwitchGameType(Ref *pSender)
{
	String * type = (String*)pSender;
	if (type == nullptr) return;
	if (currentPanel){ //移除
		currentPanel->removeFromParent();
		currentPanel = nullptr;
	}
	switch (type->intValue())
	{
		case GAMETYPE::NO:
		{
			Api::Bill::GET_BALANCE();//历史遗留
			listGames->setVisible(true);
		}
			break;
		case GAMETYPE::BJL:
		{
			currentPanel = LayerBjlGames::create();
			currentPanel->setName("LayerBjlGames");
		}
			break;
		case GAMETYPE::DZPK:
		{
			currentPanel = LayerDzGames::create();
			currentPanel->setName("LayerDzpkGames");
		}
			break;
		case GAMETYPE::SGJ:
			CCLOG("GAMETYPE::SGJ clicked!");
			break;
		case GAMETYPE::CZD:
			CCLOG("GAMETYPE::CZD clicked!");
			break;
		case GAMETYPE::MORE:
			CCLOG("GAMETYPE::MORE clicked!");
			break;
	  }

	 if (currentPanel){
		    listGames->setVisible(false);
			panelContent->addChild(currentPanel);

			RootNode->getChildByName("company_1")->setVisible(!GuideControl::GetInstance()->checkInGuide());
	 }
	
}

Node* SceneGames::getRootNode()
{
	return RootNode;
}

Layout* SceneGames::getPanelContent()
{
	return panelContent;
}

ListView* SceneGames::getListGames()
{
	return listGames;
}

Node* SceneGames::getCurrentPanel()
{
	return currentPanel;
}

void SceneGames::onOpenSigninUINotification(Ref* ref)
{
	LayerSignin* pLayerSignin = LayerSignin::create();
	this->addChild(pLayerSignin, 2);
	pLayerSignin->setTouchEnabled(true);
	SigninControl::GetInstance()->setLayerSignin(pLayerSignin);
	PKNotificationCenter::getInstance()->postNotification("RefreshSigninDayNumberNotification");
	PKNotificationCenter::getInstance()->postNotification("RefreshDayRewardsNotification");
}