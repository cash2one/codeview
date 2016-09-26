#include "LayerBind.h"
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

// on "init" you need to initialize your instance
bool LayerBind::init()
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
	RootNode = CSLoader::createNode("login/LayerBind.csb");
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
	btRegister->addClickEventListener(CC_CALLBACK_1(LayerBind::btRegisterClicked, this));
	
	auto btBack = RootNode->getChildByName<Button*>("btn_back");
	if (!btBack)
		return false;
	btBack->addClickEventListener([=](Ref *ref){this->removeFromParent(); });
	
	this->addChild(RootNode);
	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerBind::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	// register callback func
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBind::onRegisterCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::BIND), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBind::onLoginCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::LOGIN), NULL);
	return true;
}
LayerBind::~LayerBind(){
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
void LayerBind::btRegisterClicked(Ref *ref)
{
	if (!CheckUserName() || !CheckPassword() || !CheckPassword1())
		return;
	LayerProfile* profileLayer = LayerProfile::create();
	this->addChild(profileLayer);
	profileLayer->addProfileCallback([=](void){
		funcRegister();
	});
	
}
void LayerBind::funcRegister()
{
	std::string username = _editBoxUserName->getText();
	std::string password = _editBoxPassword->getText();
	unsigned char temp[32];
	SHA256(temp, (unsigned char*)password.c_str(), password.length());
	password = Char322Char64(temp);
	//开始注册
	// load loading layer
	//auto layerLoading = LayerLoading::create();
	//layerLoading->setName("LayerLoading");
	//this->addChild(layerLoading);

	LayerLoading::Wait();
	
	// send register msg
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	if (!Api::User::Bind(user_id,username.c_str(), password.c_str()))
	{
		CCLOG("[%s]:%s", "LayerBind::btRegisterClicked", "user bind error");
	/*	layerLoading->SetString(UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "网络异常，请检查" : "Network is not working, pls check.");
		layerLoading->btCancel->setVisible(true);*/

		LayerLoading::CloseWithTip("user bind net error");
		return;
	}
	// save username,password for auto login
	UserDefault::getInstance()->setStringForKey("HostUserName", username);
	UserDefault::getInstance()->setStringForKey("HostPassword", password);
}
void LayerBind::onRegisterCallBack(Ref *pSender)
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
			
				
					
			// successed.
			std::string userName = UserDefault::getInstance()->getStringForKey("HostUserName");
			std::string password = UserDefault::getInstance()->getStringForKey("HostPassword");
			//login games
			UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::HOST);

			UserDefault::getInstance()->setStringForKey("TempUserName", userName);
			UserDefault::getInstance()->setStringForKey("TempPassword", password);

			int user_id = UserControl::GetInstance()->GetUserData()->user_id;
			UserDefault::getInstance()->setBoolForKey(toString(user_id), true);
			// send login msg
			if (!Api::User::Login(userName.c_str(), password.c_str()))
				CCLOG("[%s]:%s", "LayerBind::onRegisterCallBack", "user login error");
					
			
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBind::onRegisterCallBack", "reg error.", msg->code, msg->data.c_str());
			/*auto layerLoading = this->getChildByName("LayerLoading");
			if (layerLoading)
				layerLoading->removeFromParent();*/

			LayerLoading::Close();

			if (msg->code == 2003)
				lbUserName->setString(Language::getStringByKey("UsernameAlreadyExists"));
			else if(msg->code = 2013){
				lbUserName->setString(Language::getStringByKey("UsernameAlreadyExists"));
			}
			else{
				lbUserName->setString(StringUtils::format("%s:%d", Language::getStringByKey("Error"), msg->code));
			}
				
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
void LayerBind::onLoginCallBack(Ref *pSender)
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
			UserDefault::getInstance()->setIntegerForKey(UserControl::GetInstance()->GetUserData()->username.c_str(), UserControl::GetInstance()->GetUserData()->user_id);
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
bool LayerBind::onTouchBegan(Touch *touch, Event *event)
{
	if (!panelRegister->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerBind::DelayExit), 0.1f);	//延迟调用退出，以免touch事件传送到下层
	return true;
}
void LayerBind::DelayExit(float dt)
{
	this->removeFromParent();
}
bool LayerBind::CheckUserName()
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
bool LayerBind::CheckPassword()
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
bool LayerBind::CheckPassword1()
{
	if (strcmp(_editBoxPassword->getText(), _editBoxPassword1->getText()) != 0)
	{
		lbPassword1->setString(Language::getStringByKey("Password1Error"));
		imgCheckPassword1->setTexture("login/img/RegisterWrong.png");
		imgCheckPassword1->setVisible(true);
		return false;
	}
	lbPassword1->setString("");
	imgCheckPassword1->setTexture("login/img/RegisterRight.png");
	imgCheckPassword1->setVisible(true);
	return true;
}
void LayerBind::editBoxReturn(EditBox* editBox)
{
	if (_editBoxUserName == editBox)
		CheckUserName();
	else if (_editBoxPassword == editBox)
		CheckPassword();
	else if (_editBoxPassword1 == editBox)
	{
		//密码框输入确认后，直接登录
		if (CheckPassword1() && CheckPassword() && CheckUserName())
		{
			LayerProfile* profileLayer = LayerProfile::create();
			this->addChild(profileLayer);
			profileLayer->addProfileCallback([=](void){
				funcRegister();
			});
		}
	}
}