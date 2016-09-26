#include "LayerRegister.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "LayerLoading.h"
#include "cJSON.h"
#include "UserControl.h"
#include "SceneGames.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "Settings.h"
#include "LayerProfile.h"
#include "SoundControl.h"
// on "init" you need to initialize your instance
bool LayerRegister::init()
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
	RootNode = CSLoader::createNode("login/LayerRegister.csb");
	if (!RootNode)
		return false;
	panelRegister = RootNode->getChildByName<Layout*>("panelRegister");
	if (!panelRegister)
		return false;
	//auto lbTitle = panelRegister->getChildByName<Text*>("lbTitle");
	//if (!lbTitle)
	//	return false;
	//lbTitle->setString(StringUtils::format("PKFate %s", Language::getStringByKey("Register")));
	auto txtUserName = panelRegister->getChildByName<Widget*>("txtUserName");
	if (!txtUserName)
		return false;
	Text* errorMsg = panelRegister->getChildByName<Text*>("errorMsg");
	if (!errorMsg)
		return false;
	if (!(_editBoxUserName = EditBox::create(txtUserName->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxUserName->setAnchorPoint(txtUserName->getAnchorPoint());
	_editBoxUserName->setPosition(txtUserName->getPosition());
	txtUserName->removeFromParent();
	_editBoxUserName->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxUserName->setPlaceHolder(Language::getStringByKey("InputUsername"));
	_editBoxUserName->setDelegate(this);
	panelRegister->addChild(_editBoxUserName);
	lbUserName = panelRegister->getChildByName<Text*>("lbUserName");
	if (!lbUserName)
		return false;
	lbUserName->setZOrder(1);		//将提示信息zorder提升到最前面
	//lbUserName = errorMsg;
	imgCheckUserName = panelRegister->getChildByName<Sprite*>("imgCheckUserName");
	if (!imgCheckUserName)
		return false;
	auto txtPassword = panelRegister->getChildByName<Widget*>("txtPassword");
	if (!txtPassword)
		return false;
	if (!(_editBoxPassword = EditBox::create(txtPassword->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxPassword->setAnchorPoint(txtPassword->getAnchorPoint());
	_editBoxPassword->setPosition(txtPassword->getPosition());
	txtPassword->removeFromParent();
	_editBoxPassword->setPlaceHolder(Language::getStringByKey("InputPassword"));
	_editBoxPassword->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxPassword->setInputFlag(EditBox::InputFlag::PASSWORD);
	_editBoxPassword->setDelegate(this);
	panelRegister->addChild(_editBoxPassword);
	lbPassword = panelRegister->getChildByName<Text*>("lbPassword");
	if (!lbPassword)
		return false;
	lbPassword->setZOrder(1);		//将提示信息zorder提升到最前面
	//lbPassword = errorMsg;
	imgCheckPassword = panelRegister->getChildByName<Sprite*>("imgCheckPassword");
	if (!imgCheckPassword)
		return false;
	auto txtPassword1 = panelRegister->getChildByName<Widget*>("txtPassword1");
	if (!txtPassword1)
		return false;
	if (!(_editBoxPassword1 = EditBox::create(txtPassword1->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxPassword1->setAnchorPoint(txtPassword1->getAnchorPoint());
	_editBoxPassword1->setPosition(txtPassword1->getPosition());
	txtPassword1->removeFromParent();
	_editBoxPassword1->setPlaceHolder(Language::getStringByKey("InputPassword1"));
	_editBoxPassword1->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxPassword1->setInputFlag(EditBox::InputFlag::PASSWORD);
	_editBoxPassword1->setDelegate(this);
	panelRegister->addChild(_editBoxPassword1);
	lbPassword1 = panelRegister->getChildByName<Text*>("lbPassword1");
	if (!lbPassword1)
		return false;
	lbPassword1->setZOrder(1);		//将提示信息zorder提升到最前面
	//lbPassword1 = errorMsg;
	imgCheckPassword1 = panelRegister->getChildByName<Sprite*>("imgCheckPassword1");
	if (!imgCheckPassword1)
		return false;
	btRegister = panelRegister->getChildByName<Button*>("btRegister");
	if (!btRegister)
		return false;
	btRegister->addClickEventListener(CC_CALLBACK_1(LayerRegister::btRegisterClicked, this));
	
	auto btBack = RootNode->getChildByName<Button*>("btn_back");
	if (!btBack)
		return false;
	btBack->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN); this->removeFromParent(); });
	
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerRegister::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerRegister::onRegisterCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::REGISTER), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerRegister::onLoginCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerRegister::btExistUser), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::EXIST_USER), NULL);
	return true;
}
LayerRegister::~LayerRegister(){
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
void LayerRegister::btRegisterClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	if (!CheckUserName() || !CheckPassword() || !CheckPassword1())
		return;
	LayerProfile* profileLayer = LayerProfile::create();
	this->addChild(profileLayer);
	profileLayer->addProfileCallback([=](void){
		funcRegister();
	});
	
}
void LayerRegister::funcRegister()
{
	std::string username = _editBoxUserName->getText();
	std::string password = _editBoxPassword->getText();
	std::string parentid = _editBoxPassword1->getText();
	unsigned char temp[32];
	SHA256(temp, (unsigned char*)password.c_str(), password.length());
	password = Char322Char64(temp);
	//开始注册
	// load loading layer
	/*auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	this->addChild(layerLoading);*/
	
	LayerLoading::Wait();
	int idParent = atoi(parentid.c_str());
	// send register msg
	if (!Api::User::Register(ACCOUNTTYPE::HOST, username.c_str(), password.c_str(), idParent, GetRegisterChannel()))
	{
		CCLOG("[%s]:%s", "LayerRegister::btRegisterClicked", "user register error");
		/*layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
		layerLoading->btCancel->setVisible(true);*/

		LayerLoading::CloseWithTip("user register net error");

		return;
	}
	// save username,password for auto login
	UserDefault::getInstance()->setStringForKey("HostUserName", username);
	UserDefault::getInstance()->setStringForKey("HostPassword", password);
}
void LayerRegister::onRegisterCallBack(Ref *pSender)
{
	//注册结果回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//注册成功，且已经登录
			cJSON *jsonData = cJSON_Parse(msg->data.c_str());
			if (jsonData == nullptr)
			{
				CCLOG("[%s]:%s", "LayerRegister::onRegisterCallBack", "reg - get data can't json");
			}
			else
			{
				cJSON *jsonDataUserName = cJSON_GetObjectItem(jsonData, "username");
				if (jsonDataUserName == nullptr)
				{
					CCLOG("[%s]:%s", "LayerRegister::onRegisterCallBack", "reg - get data can't find username");
				}
				else
				{
					cJSON *jsonDataUserId = cJSON_GetObjectItem(jsonData, "user_id");
					if (jsonDataUserId == nullptr)
					{
						CCLOG("[%s]:%s", "LayerRegister::onRegisterCallBack", "reg - get data can't find userId");
						return;
					}
					else
					{
						// successed.
						int userId = jsonDataUserId->valueint;
						std::string userName = jsonDataUserName->valuestring;
						std::string password = UserDefault::getInstance()->getStringForKey("HostPassword");
						//login games
						UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::HOST);
						// send login msg
						if (!Api::User::Login(userName.c_str(), password.c_str()))
							CCLOG("[%s]:%s", "LayerRegister::onRegisterCallBack", "user login error");
					}
				}
				cJSON_Delete(jsonData);
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerRegister::onRegisterCallBack", "reg error.", msg->code, msg->data.c_str());
			/*auto layerLoading = this->getChildByName("LayerLoading");
			if (layerLoading)
				layerLoading->removeFromParent();*/

			LayerLoading::Close();

			if (msg->code == 2013)
				lbUserName->setString(Language::getStringByKey("UsernameAlreadyExists"));
			else
				lbUserName->setString(StringUtils::format("%s:[%d]%s", Language::getStringByKey("Error"), msg->code,msg->data.c_str()));
		}
		//清除msg数据，以免内存泄漏
		msg->release();	
	});
}
void LayerRegister::onLoginCallBack(Ref *pSender)
{
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

		LayerLoading::Wait();

		if (msg->code == 0)
		{
			// login ok
			// init userdata
			if (!UserControl::GetInstance()->InitUserData(msg->data))
			{
				CCLOG("[%s]:%s\t%s", "SceneLogin::onLoginCallBack", "init userdata error.", msg->data.c_str());
			}
			//enter games
			auto scene = SceneGames::create();
			Director::getInstance()->replaceScene(scene);
		}
		else
		{
			// login fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLoginCallBack", "login error.", msg->code, msg->data.c_str());
			// 重置自动登录类型
			UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
		}
		msg->release();
	});
}
bool LayerRegister::onTouchBegan(Touch *touch, Event *event)
{
	if (!panelRegister->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerRegister::DelayExit), 0.1f);	//延迟调用退出，以免touch事件传送到下层
	return true;
}
void LayerRegister::DelayExit(float dt)
{
	this->removeFromParent();
}
bool LayerRegister::CheckUserName()
{
	std::string username = _editBoxUserName->getText();
	if (username.empty())
	{
		lbUserName->setString(Language::getStringByKey("UsernameCanNotEmpty"));
		imgCheckUserName->setTexture("login/img/RegisterWrong.png");
		imgCheckUserName->setVisible(true);
		return false;
	}
	if (username.length() < 6 || username.length() > 32)
	{
		lbUserName->setString(Language::getStringByKey("UsernameLenError"));
		imgCheckUserName->setTexture("login/img/RegisterWrong.png");
		imgCheckUserName->setVisible(true);
		return false;
	}
	if (!Comm::RegexIsMatch(username.c_str(), "^[A-Za-z0-9_]{6,32}$"))
	{
		lbUserName->setString(Language::getStringByKey("UsernameInvalid"));
		return false;
	}

	lbUserName->setString("");
	imgCheckUserName->setTexture("login/img/RegisterRight.png");
	imgCheckUserName->setVisible(true);
	return true;
}
bool LayerRegister::CheckPassword()
{
	std::string password = _editBoxPassword->getText();
	if (password.empty())
	{
		lbPassword->setString(Language::getStringByKey("PasswordCanNotEmpty"));
		imgCheckPassword->setTexture("login/img/RegisterWrong.png");
		imgCheckPassword->setVisible(true);
		return false;
	}
	if (password.length()<6)
	{
		lbPassword->setString(Language::getStringByKey("PasswordLenError"));
		imgCheckPassword->setTexture("login/img/RegisterWrong.png");
		imgCheckPassword->setVisible(true);
		return false;
	}
	lbPassword->setString("");
	imgCheckPassword->setTexture("login/img/RegisterRight.png");
	imgCheckPassword->setVisible(true);

	return true;
}
bool LayerRegister::CheckPassword1()
{
	//if (strcmp(_editBoxPassword->getText(), _editBoxPassword1->getText()) != 0)
	//{
	//	lbPassword1->setString(Language::getStringByKey("Password1Error"));
	//	imgCheckPassword1->setTexture("login/img/RegisterWrong.png");
	//	imgCheckPassword1->setVisible(true);
	//	return false;
	//}

	lbPassword1->setString("");
	imgCheckPassword1->setTexture("login/img/RegisterRight.png");
	imgCheckPassword1->setVisible(true);
	return true;
}
void LayerRegister::editBoxReturn(EditBox* editBox)
{
	if (_editBoxUserName == editBox)
	{
		//CheckUserName();

		std::string username = _editBoxUserName->getText();
		if (!Api::User::ExistUser(username.c_str()))
			CCLOG("exist user error");
	}else if (_editBoxPassword == editBox)
		CheckPassword();
	else if (_editBoxPassword1 == editBox)
	{
		//密码框输入确认后，直接登录
		if (CheckPassword1() && CheckPassword() && CheckUserName())
		{
			LayerProfile* profileLayer = LayerProfile::create();
			this->addChild(profileLayer);
			profileLayer->addProfileCallback([=](void)
			{
				funcRegister();
			});
		}
	}
}

void LayerRegister::btExistUser(Ref *ref)
{
	//SetNickName回调函数
	if (ref == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)ref;
	msg->retain();
	//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			if (strcmp(msg->data.c_str(), "true") == 0)
			{
				lbUserName->setString(Language::getStringByKey("UsernameAlreadyExists"));
				imgCheckUserName->setTexture("login/img/RegisterWrong.png");
				imgCheckUserName->setVisible(true);
			}
			else
			{
				CheckUserName();
			}
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerRegister::ExistUser", "ExistUser error.", msg->code, msg->data.c_str());
			switch (msg->code)
			{
			default:
				lbUserName->setString(Language::getStringByKey("ErrorTry"));
				break;
			}
		}
		msg->release();
	});


}