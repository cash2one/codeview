#include "LayerSetting.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
#include "Settings.h"
#include "SceneLogin.h"
#include "UserControl.h"
#include "AvatarControl.h"
#include "SoundControl.h"
#include "LayerProfile.h"
#include "LayerHelp.h"
#include "LayerPassword.h"
#include "LayerNotice.h"
#include "SoundControl.h"
#include "SocketControl.h"
#include "LayerDuang.h"
#include "GuideControl.h"
#include "PKNotificationCenter.h"
#include "LayerLoading.h"
// on "init" you need to initialize your instance
bool LayerSetting::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerSetting.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

	ScrollView* content_scrollview = RootNode->getChildByName<ScrollView*>("content_scrollview");
	if (!content_scrollview) return false;

	Layout* content = (Layout*)content_scrollview->getChildByName("content");


	Text* label_account = content->getChildByName<Text*>("label_account");
	label_account->setString(Language::getStringByKey("Account"));

	Text* label_select = content->getChildByName<Text*>("label_select");
	label_select->setString(Language::getStringByKey("GameOption"));

	Text* label_other = content->getChildByName<Text*>("label_other");
	label_other->setString(Language::getStringByKey("Other"));
	
	Node* node_1 = content->getChildByName("node_1");

	Button* account_btn = node_1->getChildByName<Button*>("btn");
	account_btn->setSwallowTouches(false);
	
	Text* name = node_1->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("LogOff"));

	account_btn->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CLOSE);
		UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
		SocketControl::GetInstance()->close();
		auto scene = SceneLogin::create();
		Director::getInstance()->replaceScene(scene);
	});
	Node* node_2 = content->getChildByName("node_2");
	Button* pass_btn = node_2->getChildByName<Button*>("btn");
	name = node_2->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("PasswordReset"));
		
	pass_btn->setSwallowTouches(false);
	pass_btn->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	    LayerPassword *passwordLayer = LayerPassword::create();
			
		Director::getInstance()->getRunningScene()->addChild(passwordLayer, 2);
		OpenWithAlert(passwordLayer);
		
		this->removeFromParent();
	});

	if (!UserControl::GetInstance()->GetUserData()){
		pass_btn->setEnabled(false);
		pass_btn->setBright(false);
	}
	

	Node* node_3 = content->getChildByName("node_3");

	name = node_3->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("Music"));


	Node* node_4 = content->getChildByName("node_4");

	name = node_4->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("Sound"));

	Node* node_50 = content->getChildByName("node_5_0");
	name = node_50->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("BacaratGuide"));
	Button* guide_btn = node_50->getChildByName<Button*>("btn");
	guide_btn->setSwallowTouches(false);
	guide_btn->addClickEventListener([=](Ref *ref){
		GuideControl::GetInstance()->finishGuideStepWithType(GAMETYPE::BJL, 0);
		LayerLoading::Wait();
		if (Director::getInstance()->getRunningScene()->getName() != "SceneGame")
		{
			Director::getInstance()->popScene();
		}
		Director::getInstance()->getScheduler()->schedule([=](float dt){
			PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::BJL)));
			LayerLoading::Close();
			this->removeFromParent();
		}, this, 0.0f, 0.0f, 0.5f, false, "triggerSwitchGameType");
	});


	Node* node_5 = content->getChildByName("node_5");
	name = node_5->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("PokerGuide"));

	Button* dz_guide_btn = node_5->getChildByName<Button*>("btn");
	dz_guide_btn->setSwallowTouches(false);
	dz_guide_btn->addClickEventListener([=](Ref *ref){
		GuideControl::GetInstance()->finishGuideStepWithType(GAMETYPE::DZPK, 0);
		LayerLoading::Wait();
		if (Director::getInstance()->getRunningScene()->getName() != "SceneGame")
		{
			Director::getInstance()->popScene();
		}
		Director::getInstance()->getScheduler()->schedule([=](float dt){
			PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::DZPK)));
			LayerLoading::Close();
			this->removeFromParent();
		}, this, 0.0f, 0.0f, 0.5f, false, "triggerSwitchGameType");
		
	});


	Node* node_6 = content->getChildByName("node_6");
	name = node_6->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("GameNotice"));

	Button* notice_btn = node_6->getChildByName<Button*>("btn");
	notice_btn->setSwallowTouches(false);
	notice_btn->addClickEventListener([=](Ref *ref){
		//tongzhi
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerNotice* layerNotice = LayerNotice::create();
		layerNotice->fromSet = true;
		Director::getInstance()->getRunningScene()->addChild(layerNotice, 2);
		OpenWithAlert(layerNotice);
		this->removeFromParent();
	});

	Node* node_7 = content->getChildByName("node_7");
	Button* profile_btn = node_7->getChildByName<Button*>("btn");

	name = node_7->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("Profile"));

	profile_btn->setSwallowTouches(false);
	profile_btn->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerProfile* profileLayer = LayerProfile::create();
		profileLayer->setWithAgree(false);
		Director::getInstance()->getRunningScene()->addChild(profileLayer,2);
		this->removeFromParent();
	});

	Node* node_8 = content->getChildByName("node_8");
	name = node_8->getChildByName<Text*>("name");
	name->setString(Language::getStringByKey("Help"));

	Button* help_btn = node_8->getChildByName<Button*>("btn");
	help_btn->setSwallowTouches(false);
	help_btn->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		LayerHelp* layerHelp = LayerHelp::create();
		Director::getInstance()->getRunningScene()->addChild(layerHelp,2);
		this->removeFromParent();
	});


	sliderMusic = (Slider*)node_3->getChildByName("sliderMusic");
	if (!sliderMusic)
		return false;
	sliderMusic->addEventListener(CC_CALLBACK_2(LayerSetting::SliderEvent, this));
	sliderSound = (Slider*)node_4->getChildByName("sliderSound");
	if (!sliderSound)
		return false;
	sliderSound->addEventListener(CC_CALLBACK_2(LayerSetting::SliderEvent, this));
	btnMuteMusic = (Button*)node_3->getChildByName("btnMuteMusic");
	if (!btnMuteMusic)
		return false;
	btnMuteMusic->addClickEventListener([=](Ref *ref){
		SetMusicMute(btnMuteMusic->isBright());
	});
	btnMuteSound = (Button*)node_4->getChildByName("btnMuteSound");
	if (!btnMuteSound)
		return false;
	btnMuteSound->addClickEventListener([=](Ref *ref){
		SetSoundMute(btnMuteSound->isBright());
	});
	//load current setting
	//load music volume
	sliderMusic->setPercent((int)(SoundControl::GetMusicVolume() * 100));
	//load music mute
	btnMuteMusic->setBright(!SoundControl::GetMusicMute());
	sliderMusic->setBright(!SoundControl::GetMusicMute());
	//load sound volume
	sliderSound->setPercent((int)(SoundControl::GetSoundVolume() * 100));
	//load sound mute
	btnMuteSound->setBright(!SoundControl::GetSoundMute());
	sliderSound->setBright(!SoundControl::GetSoundMute());
	//add layer touch event

	touch_layer = RootNode->getChildByName<Layout*>("touch_layer");
	if (!touch_layer)
		return false;


	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerSetting::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerSetting::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerSetting::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	return true;
}
bool LayerSetting::onTouchBegan(Touch *touch, Event *event){
	if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerSetting::DelayExit), 0.1f);	//延迟调用退出，以免touch事件传送到下层
	return true;
}
void LayerSetting::DelayExit(float dt)
{
	CloseWithAction(this);

	LayerDuang::panelStatus();
	LayerDuang::uiGameMenu();
	LayerDuang::dzGamesRoom();
	LayerDuang::bjlGamesRoom();
	LayerDuang::bjlPanelLobby();
	LayerDuang::dzPanelLobby();
}

void LayerSetting::onTouchMoved(Touch *touch, Event *event){

}

void LayerSetting::onTouchEnded(Touch* touch, Event* event){

}
void LayerSetting::SliderEvent(Ref *pSender, Slider::EventType type)
{
	switch (type)
	{
	case Slider::EventType::ON_PERCENTAGE_CHANGED:
	{
		Slider* slider = static_cast<Slider*>(pSender);
		if (slider == sliderMusic)
		{
			SetMusicMute(false);
			SetMusicVolume((float)slider->getPercent() / 100);
			//Settings::GetInstance()->SetMusicVolume(slider->getPercent());
			//CCLOG("Music pec:%d%%\t%.2f", slider->getPercent(), (float)slider->getPercent() / 100);
			//CCLOG("Music vol:%.2f", SimpleAudioEngine::getInstance()->getBackgroundMusicVolume());
		}
		else if (slider == sliderSound)
		{
			SetSoundMute(false);
			SetSoundVolume((float)slider->getPercent() / 100);
			//CCLOG("Effect pec:%d%%\t%.2f", slider->getPercent(), (float)slider->getPercent() / 100);
			//CCLOG("Effect vol:%.2f", SimpleAudioEngine::getInstance()->getEffectsVolume());
		}
	}
		break;

	default:
		break;
	}
}
void LayerSetting::SetMusicMute(bool mute)
{
	btnMuteMusic->setBright(!mute);
	sliderMusic->setBright(!mute);
	SoundControl::SetMusicMute(mute);
}
void LayerSetting::SetMusicVolume(float vol)
{
	SoundControl::SetMusicVolume(vol);
}
void LayerSetting::SetSoundMute(bool mute)
{
	btnMuteSound->setBright(!mute);
	sliderSound->setBright(!mute);
	SoundControl::SetSoundMute(mute);
}
void LayerSetting::SetSoundVolume(float vol)
{
	SoundControl::SetSoundVolume(vol);
}