#define COCOS2D_DEBUG 1

#include "SceneLogin.h"
#include "SceneGames.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "LayerLoading.h"
#include "LayerRegister.h"
#include "LayerLogin.h"
#include "cJSON.h"
#include "Settings.h"
#include "UserControl.h"
#include "LayerSetting.h"
#include "LayerLang.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "LayerProfile.h"
#include "LayerBind.h"
#include "SoundControl.h"

// on "init" you need to initialize your instance
bool SceneLogin::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("login/SceneLogin.csb");
	if (!RootNode)
		return false;
	auto lbVersion = RootNode->getChildByName<Text*>("txtVersion");
	if (!lbVersion)
		return false;
	else
		lbVersion->setString(VERSION);
	btRegister = (Button*)RootNode->getChildByName("btRegister");
	if (!btRegister)
		return false;
	btRegister->addClickEventListener(CC_CALLBACK_1(SceneLogin::btRegisterClicked, this));
	btLoginPkfate = (Button*)RootNode->getChildByName("btLoginPkfate");
	if (!btLoginPkfate)
		return false;
	btLoginPkfate->addClickEventListener(CC_CALLBACK_1(SceneLogin::btLoginPkfateClicked, this));
	btLoginLocal = (Button*)RootNode->getChildByName("btLoginLocal");
	if (!btLoginLocal)
		return false;
	btLoginLocal->addClickEventListener(CC_CALLBACK_1(SceneLogin::btLoginLocalClicked, this));
	auto btSettings = RootNode->getChildByName<Button*>("btSettings");
	if (!btSettings)
		return false;
	btSettings->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);  

		LayerSetting * setting = this->getChildByName<LayerSetting*>("LayerSetting");
		if (!setting){
			LayerSetting * setting = LayerSetting::create(); //临时创建释放 avoid texture memory overflow
			setting->setName("LayerSetting");
			this->addChild(setting, 2);
			OpenWithAlert(setting);
		}
	
	});
	auto btLang = RootNode->getChildByName<Button*>("btLang");
	if (!btLang)
		return false;
	btLang->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);  
	
		LayerLang * lang = this->getChildByName<LayerLang*>("LayerLang");
		if (!lang){
			LayerLang * lang = LayerLang::create(); //临时创建释放 avoid texture memory overflow
			lang->setName("LayerLang");
			this->addChild(lang, 2);
			OpenWithAlert(lang);
		}
	
	});
	this->addChild(RootNode);
	
	EventListenerKeyboard* keyListener = EventListenerKeyboard::create();
	
	

	keyListener->onKeyReleased = CC_CALLBACK_2(SceneLogin::onKeyReleased, this);
	
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, this);
	


	
	return true;
}

void SceneLogin::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event){
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		Director::getInstance()->end();
	}
};

void SceneLogin::onEnter()
{
	Scene::onEnter();
	SoundControl::PlayMusic(BGM::BGM_LOGIN);
	//sp->runAction(Sequence::create(animate, animate->reverse(), nullptr));
	//CCLOG("Local Password:%s",UserDefault::getInstance()->getStringForKey("LocalPassword").c_str());
	//load login type.
	int logintype = UserDefault::getInstance()->getIntegerForKey("LoginType");		//0 pkfate,1 tmp,2 sdk
	std::string username, password;
	switch (logintype)
	{
	case ACCOUNTTYPE::HOST:
		username = UserDefault::getInstance()->getStringForKey("HostUserName");
		password = UserDefault::getInstance()->getStringForKey("HostPassword");
		
		break;
	case ACCOUNTTYPE::TEMP:
	{
		username = UserDefault::getInstance()->getStringForKey("TempUserName");
		password = UserDefault::getInstance()->getStringForKey("TempPassword");
	}
		break;
	case ACCOUNTTYPE::THIRD:
		//暂未开发第三方登录
		return;
	default:
		return;
	}
	if (username == "" || password == "")
	{
		UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);	// 重置自动登录类型
		return;
	}
	//CCLOG("logintype:%d\tusername:%s\tpassword:%s", logintype, username.c_str(), password.c_str());
	// show loading layer
	AutoLogin(username, password);
}

bool SceneLogin::AutoLogin(std::string username, std::string password)
{
	//auto layerLoading = LayerLoading::create();
	//layerLoading->setName("LayerLoading");
	//this->addChild(layerLoading);

	LayerLoading::Wait();
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneLogin::onLoginCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN), NULL);
	// send login msg
	if (!Api::User::Login(username.c_str(), password.c_str()))
	{
		CCLOG("[%s]:%s", "SceneLogin::onEnter", "auto login error");
		PKNotificationCenter::getInstance()->removeObserver(this, GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN));
		/*layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
		layerLoading->btCancel->setVisible(true);*/

		LayerLoading::CloseWithTip();
		return false;
	}
	return true;
}

void SceneLogin::btRegisterClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	auto layerRegister = LayerRegister::create();
	this->addChild(layerRegister);
}
void SceneLogin::btLoginPkfateClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	auto layerLogin = LayerLogin::create();
	this->addChild(layerLogin);
}

void SceneLogin::dealLocalLogin()
{
	// show loading layer
	//auto layerLoading = LayerLoading::create();
	//layerLoading->setName("LayerLoading");
	//this->addChild(layerLoading);
	//Api::User::Login("skinsen@gmail.com", "111111");
	//Api::User::Login("66fe23a6-c25b-4d5b-aca6-d60fddcfe083", "111111");

	//bool ok=Api::User::Register(1, NULL, "abcdefg", 0);
	// local login
	// 1.load local userinfo

	LayerLoading::Wait();
	std::string username = UserDefault::getInstance()->getStringForKey("HostUserName");
	std::string password = UserDefault::getInstance()->getStringForKey("HostPassword");
	
	if (username == "" || password == ""){
		username = UserDefault::getInstance()->getStringForKey("TempUserName");
		password = UserDefault::getInstance()->getStringForKey("TempPassword");
	}

	
	if (username == "" || password == "")
	{
		// 2a.request new local account
		// random password
		unsigned char temp[32];
		for (int i = 0; i < 32; i += 8)
		{
			*(unsigned long long*)&temp[i] = ((unsigned long long)rand() ^ ((unsigned long long)rand() << 16) ^ ((unsigned long long)rand() << 32) ^ ((unsigned long long)rand() << 48));
		}
		password = Char322Char64(temp);
		CCLOG("random password:%s", password.c_str());
		// save local password
		UserDefault::getInstance()->setStringForKey("TempPassword", password);
		// register callback func
		PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneLogin::onLocalRegisterCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::REGISTER), NULL);
		// show loading scene
		// send register msg
		if (!Api::User::Register(ACCOUNTTYPE::TEMP, NULL, password.c_str(), PARENTID, GetRegisterChannel()))
		{
			CCLOG("[%s]:%s", "SceneLogin::btLoginLocalClicked", "user register error");
			PKNotificationCenter::getInstance()->removeObserver(this, GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::REGISTER));
			//layerLoading->removeFromParent();

			LayerLoading::CloseWithTip("user register error");
			return;
		}
	}
	else
	{
		// 2b.login with local userinfo
		if (!LocalLogin(username, password))
		{
		/*	layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
			layerLoading->btCancel->setVisible(true);*/

			LayerLoading::CloseWithTip("user login error");
		}
	}
}
void SceneLogin::btLoginLocalClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	LayerProfile* profileLayer = LayerProfile::create();
	this->addChild(profileLayer);
	profileLayer->addProfileCallback([=](void){
		dealLocalLogin();
	});
	
}



bool SceneLogin::LocalLogin(std::string username, std::string password)
{
	if (username.empty() || password.empty())
		return false;
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneLogin::onLoginCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN), NULL);
	UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::TEMP);
	// send login msg
	LayerLoading::Wait();
	if (!Api::User::Login(username.c_str(), password.c_str()))
	{
		CCLOG("[%s]:%s", "SceneLogin::LocalLogin", "user login error");
		PKNotificationCenter::getInstance()->removeObserver(this, GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN));
		return false;
	}
	return true;
}
void SceneLogin::onLoginCallBack(Ref *pSender)
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeObserver(this, GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN));
	//本机登录回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		/*auto layerLoading = this->getChildByName("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();*/

		LayerLoading::Close();
		if (msg->code == 0)
		{
			// login ok
			// init userdata
			if (!UserControl::GetInstance()->InitUserData(msg->data))
			{
				CCLOG("[%s]:%s\t%s", "SceneLogin::onLoginCallBack", "init userdata error.", msg->data.c_str());
			}
			//enter games
			//账号和ID 绑定

			int logintype = UserControl::GetInstance()->GetUserData()->login_type;

				//0 pkfate,1 tmp,2 sdk
			if (logintype == ACCOUNTTYPE::TEMP)
			{
				ShowTip(Language::getStringByKey("NotBindAccount"), [=](){
					//弹出绑定账号界面
					auto layerBind = LayerBind::create();
					this->addChild(layerBind);
				}, [=](){
					auto scene = SceneGames::create();
					Director::getInstance()->replaceScene(scene);
				});
			}
			else{
				auto scene = SceneGames::create();
				Director::getInstance()->replaceScene(scene);
			}
			
		}
		else
		{
			int logintype = UserDefault::getInstance()->getIntegerForKey("LoginType");		//0 pkfate,1 tmp,2 sdk
			if (logintype == ACCOUNTTYPE::TEMP)
			{
				//游客账户登录失败的话，则直接清空旧的游客账号
				UserDefault::getInstance()->setStringForKey("TempUserName", "");
			}
			UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);	// 重置自动登录类型

			Tips(msg->data.c_str());
			// login fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLoginCallBack", "login error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
void SceneLogin::onLocalRegisterCallBack(Ref *pSender)
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeObserver(this, GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::REGISTER));
	//本机注册回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			// local reg ok
			cJSON *jsonData = cJSON_Parse(msg->data.c_str());
			if (jsonData == nullptr)
			{
				CCLOG("[%s]:%s", "SceneLogin::onLocalRegisterCallBack", "get data can't json");
			}
			else
			{
				cJSON *jsonDataUserName = cJSON_GetObjectItem(jsonData, "username");
				if (jsonDataUserName == nullptr)
				{
					CCLOG("[%s]:%s", "SceneLogin::onLocalRegisterCallBack", "get data can't find username");
				}
				else
				{
					cJSON *jsonDataUserId = cJSON_GetObjectItem(jsonData, "user_id");
					if (jsonDataUserId == nullptr)
					{
						CCLOG("[%s]:%s", "SceneLogin::onLocalRegisterCallBack", "get data can't find userId");
						return;
					}
					else
					{
						// successed.
						int userId = jsonDataUserId->valueint;
						std::string userName = jsonDataUserName->valuestring;
						//save username
						UserDefault::getInstance()->setStringForKey("TempUserName", userName);
						std::string password = UserDefault::getInstance()->getStringForKey("TempPassword");
						//login games
						if (!LocalLogin(userName, password))
							CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLocalRegisterCallBack", "temp login error.", msg->code, msg->data.c_str());
					}
				}
				cJSON_Delete(jsonData);
			}
		}
		else
		{
			// local reg fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLocalRegisterCallBack", "temp reg error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}