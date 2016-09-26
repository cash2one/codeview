#include "LayerLogin.h"
#include "cocostudio/CocoStudio.h"
#include "ApiUser.h"
#include "LayerLoading.h"
#include "cJSON.h"
#include "UserControl.h"
#include "SceneGames.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "LayerProfile.h"
#include "SoundControl.h"
// on "init" you need to initialize your instance
bool LayerLogin::init()
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
	RootNode = CSLoader::createNode("login/LayerLogin.csb");
	if (!RootNode)
		return false;
	panelLogin = RootNode->getChildByName<Layout*>("panelLogin");
	if (!panelLogin)
		return false;
	/*auto lbTitle = panelLogin->getChildByName<Text*>("lbTitle");
	if (!lbTitle)
		return false;
	lbTitle->setString(StringUtils::format("PKFate %s", Language::getStringByKey("Login")));*/

	errorMsg = panelLogin->getChildByName<Text*>("error_msg");
	if (!errorMsg)
		return false;

	//auto lbUserName = panelLogin->getChildByName<Text*>("lbUserName");
	//if (!lbUserName)
	//	return false;
	//lbUserName->setString(Language::getStringByKey("Username"));
	//auto lbPassword = panelLogin->getChildByName<Text*>("lbPassword");
	//if (!lbPassword)
	//	return false;
	//lbPassword->setString(Language::getStringByKey("Password"));
	auto txtUserName = panelLogin->getChildByName<Widget*>("txtUserName");
	if (!txtUserName)
		return false;
	if (!(_editBoxUserName = EditBox::create(txtUserName->getSize(), Scale9Sprite::create())))
		return false;
	_editBoxUserName->setAnchorPoint(txtUserName->getAnchorPoint());
	_editBoxUserName->setPosition(txtUserName->getPosition());
	txtUserName->removeFromParent();
	_editBoxUserName->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_editBoxUserName->setPlaceHolder(Language::getStringByKey("InputUsername"));
	_editBoxUserName->setDelegate(this);
	panelLogin->addChild(_editBoxUserName);
	//msgUserName = errorMsg;
	if(!(msgUserName = panelLogin->getChildByName<Text*>("msgUserName")))
		return false;
	msgUserName->setZOrder(1);		//将提示信息zorder提升到最前面
	auto txtPassword = panelLogin->getChildByName<Widget*>("txtPassword");
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
	panelLogin->addChild(_editBoxPassword);
	msgPassword = panelLogin->getChildByName<Text*>("msgPassword");
	if (!msgPassword)
		return false;
	msgPassword->setZOrder(1);		//将提示信息zorder提升到最前面
	//msgPassword = errorMsg;
	btLogin = panelLogin->getChildByName<Button*>("btLogin");
	if (!btLogin)
		return false;
	btLogin->addClickEventListener(CC_CALLBACK_1(LayerLogin::btLoginClicked, this));
	auto btBack = RootNode->getChildByName<Button*>("btBack");
	if (!btBack)
		return false;
	btBack->addClickEventListener([=](Ref *ref){SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);  this->removeFromParent(); });
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerLogin::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	string username = UserDefault::getInstance()->getStringForKey("HostUserName");
	if (!username.empty()){
		_editBoxUserName->setText(username.c_str());
	}

	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerLogin::onLoginCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN), NULL);
	return true;
}
LayerLogin::~LayerLogin(){
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
void LayerLogin::btLoginClicked(Ref *ref)
{
	SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
	if (!CheckUserName() || !CheckPassword())
		return;

	LayerProfile* profileLayer = LayerProfile::create();
	this->addChild(profileLayer);
	profileLayer->addProfileCallback([=](void){
		login();
	});

	
}
void LayerLogin::login()
{
	std::string username = _editBoxUserName->getText();
	std::string password = _editBoxPassword->getText();
	unsigned char temp[32];
	SHA256(temp, (unsigned char*)password.c_str(), password.length());
	password = Char322Char64(temp);
	//开始登陆
	// load loading layer
	/*auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	this->addChild(layerLoading);*/

	LayerLoading::Wait();

	// send login msg
	if (!Api::User::Login(username.c_str(), password.c_str()))
	{
		CCLOG("[%s]:%s", "LayerLogin::btLoginClicked", "user login error");
		/*layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
		layerLoading->btCancel->setVisible(true);*/

		LayerLoading::CloseWithTip("user login net error");
		return;
	}
	// save username,password for auto login
	UserDefault::getInstance()->setStringForKey("HostUserName", username);
	UserDefault::getInstance()->setStringForKey("HostPassword", password);
}
void LayerLogin::onLoginCallBack(Ref *pSender)
{
	//登陆结果回调函数
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
			//成功登录
			// init userdata
			if (!UserControl::GetInstance()->InitUserData(msg->data))
			{
				CCLOG("[%s]:%s\t%s", "SceneLogin::onAutoLoginCallBack", "init userdata error.", msg->data.c_str());
			}

			UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::HOST);
			//enter games
			auto scene = SceneGames::create();
			Director::getInstance()->replaceScene(scene);
		}
		else
		{
			//失败
			if (msg->code == 2001)
				msgUserName->setString(Language::getStringByKey("UsernameOrPasswordError"));
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerLogin::onLoginCallBack", "login error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool LayerLogin::onTouchBegan(Touch *touch, Event *event)
{
	/*if (!panelLogin->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerLogin::DelayExit), 0.1f);	//延迟调用退出，以免touch事件传送到下层*/
	return true;
}
bool LayerLogin::CheckUserName()
{
	std::string username = _editBoxUserName->getText();
	if (username.empty())
	{
		msgUserName->setString(Language::getStringByKey("UsernameCanNotEmpty"));
		return false;
	}
	if (username.length() < 6 || username.length() > 32)
	{
		msgUserName->setString(Language::getStringByKey("UsernameLenError"));
		return false;
	}
	if (!Comm::RegexIsMatch(username.c_str(), "^[A-Za-z0-9_]{6,32}$"))
	{
		msgUserName->setString(Language::getStringByKey("UsernameInvalid"));
		return false;
	}
	msgUserName->setString("");
	return true;
}
bool LayerLogin::CheckPassword()
{
	std::string password = _editBoxPassword->getText();
	if (password.empty())
	{
		msgPassword->setString(Language::getStringByKey("PasswordCanNotEmpty"));
		return false;
	}
	if (password.length()<6)
	{
		msgPassword->setString(Language::getStringByKey("PasswordLenError"));
		return false;
	}
	msgPassword->setString("");
	return true;
}
void LayerLogin::editBoxReturn(EditBox* editBox)
{
	if (_editBoxUserName == editBox)
		CheckUserName();
	else if (_editBoxPassword == editBox)
	{
		//密码框输入确认后，直接登录
		if (CheckPassword() && CheckUserName())
		{
			LayerProfile* profileLayer = LayerProfile::create();
			this->addChild(profileLayer);
			profileLayer->addProfileCallback([=](void){
				login();
			});
		}
	}
}