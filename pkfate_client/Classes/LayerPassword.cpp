#include "LayerPassword.h"
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
bool LayerPassword::init()
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
	RootNode = CSLoader::createNode("LayerPassword.csb");
	if (!RootNode)
		return false;
	this->addChild(RootNode);

	Layout* passPanel = RootNode->getChildByName<Layout*>("pass_panel");


	touch_layer = RootNode->getChildByName<Layout*>("touch_layer");


	if (!touch_layer)
		return false;

	txtCoins = passPanel->getChildByName<TextBMFont*>("txtCoins");
	if (!txtCoins)
		return false;
	if (!(txtLevel = passPanel->getChildByName<TextBMFont*>("txtLevel")))
		return false;
	auto userData = UserControl::GetInstance()->GetUserData();
	txtLevel->setString(toString(userData->level));

	avatar = passPanel->getChildByName<Sprite*>("avatar");
	if (!avatar) return false;
	SetBalanceData(nullptr);//default
	Text* nick_name = passPanel->getChildByName<Text*>("nick_name");
	nick_name->setString(userData->nickname);

	passPanel->getChildByName<Text*>("pass_label1")->setText(Language::getStringByKey("OldPassWord"));
	passPanel->getChildByName<Text*>("pass_label2")->setText(Language::getStringByKey("NewPassWord"));
	passPanel->getChildByName<Text*>("pass_label3")->setText(Language::getStringByKey("ConfirmPassWord"));

	auto oldPass = passPanel->getChildByName<Widget*>("old_pass");
	if (!oldPass)
		return false;
	oldPassCheck = passPanel->getChildByName<Sprite*>("old_pass_check");
	if (!oldPassCheck)
		return false;
	if (!(_oldPassEditBox = EditBox::create(oldPass->getSize(), Scale9Sprite::create())))
		return false;
	_oldPassEditBox->setAnchorPoint(oldPass->getAnchorPoint());
	_oldPassEditBox->setPosition(oldPass->getPosition());
	oldPass->removeFromParent();
	_oldPassEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_oldPassEditBox->setPlaceholderFontSize(30);
	_oldPassEditBox->setPlaceHolder(Language::getStringByKey("InputPassword"));
	_oldPassEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_oldPassEditBox->setInputFlag(EditBox::InputFlag::PASSWORD);
	_oldPassEditBox->setDelegate(this);
	passPanel->addChild(_oldPassEditBox);
	
	auto newPass = passPanel->getChildByName<Widget*>("new_pass");
	if (!newPass)
		return false;
	newPassCheck = passPanel->getChildByName<Sprite*>("new_pass_check");
	if (!newPassCheck)
		return false;
	if (!(_newPassEditBox = EditBox::create(newPass->getSize(), Scale9Sprite::create())))
		return false;
	_newPassEditBox->setAnchorPoint(newPass->getAnchorPoint());
	_newPassEditBox->setPosition(newPass->getPosition());
	newPass->removeFromParent();
	_newPassEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_newPassEditBox->setPlaceholderFontSize(30);
	_newPassEditBox->setPlaceHolder(Language::getStringByKey("InputPassword"));
	_newPassEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_newPassEditBox->setInputFlag(EditBox::InputFlag::PASSWORD);
	_newPassEditBox->setDelegate(this);
	passPanel->addChild(_newPassEditBox);

	auto confirmPass = passPanel->getChildByName<Widget*>("confirm_pass");
	if (!confirmPass)
		return false;
	confirmPassCheck = passPanel->getChildByName<Sprite*>("confirm_pass_check");
	if (!confirmPassCheck)
		return false;
	if (!(_confirmPassEditBox = EditBox::create(confirmPass->getSize(), Scale9Sprite::create())))
		return false;
	_confirmPassEditBox->setAnchorPoint(confirmPass->getAnchorPoint());
	_confirmPassEditBox->setPosition(confirmPass->getPosition());
	confirmPass->removeFromParent();
	_confirmPassEditBox->setPlaceholderFontName("font/msyhbd.ttf");
	_confirmPassEditBox->setPlaceholderFontSize(30);
	_confirmPassEditBox->setPlaceHolder(Language::getStringByKey("InputPassword1"));
	_confirmPassEditBox->setInputMode(EditBox::InputMode::SINGLE_LINE);
	_confirmPassEditBox->setInputFlag(EditBox::InputFlag::PASSWORD);
	_confirmPassEditBox->setDelegate(this);
	passPanel->addChild(_confirmPassEditBox);

	btnConfirm = passPanel->getChildByName<Button*>("btn_confirm");
	btnConfirm->setTitleText(Language::getStringByKey("Confirm"));
	btnConfirm->setEnabled(false);
	btnConfirm->setBright(false);
	if (!btnConfirm)
		return false;

	btnConfirm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		if (CheckOldPassword() && CheckNewPassword() && CheckConfirmPassword()) //检验通过
		{

			funcChangePass();

		}
	});

	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerPassword::onTouchBegan, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);



	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerPassword::onUpdatePasswordCallBack), GetMsgTypeString(MSGTYPE_USER, MSGCMD_USER::UPDATE_PASSWORD), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerPassword::SetBalanceData), "UpdateBalanceUserInfo", NULL);
	return true;
}


//设置balance相关信息
void LayerPassword::SetBalanceData(Ref *ref)
{
	
	BalanceData *data = BillControl::GetInstance()->GetBalanceData(0);
	if (data)
	{
		//设置balance
		txtCoins->setString(Comm::GetShortStringFromInt64(data->balance));
		
	}
}

void LayerPassword::funcChangePass()
{
	std::string oldPass = _oldPassEditBox->getText();
	std::string newPass = _newPassEditBox->getText();
	unsigned char oldTemp[32];
	SHA256(oldTemp, (unsigned char*)oldPass.c_str(), oldPass.length());
	oldPass = Char322Char64(oldTemp);

	unsigned char newTemp[32];
	SHA256(newTemp, (unsigned char*)newPass.c_str(), newPass.length());
	newPass = Char322Char64(newTemp);

	//开始注册
	// load loading layer

	LayerLoading::Wait();

	// send register msg
	if (!Api::User::UpdatePassword(UserControl::GetInstance()->GetUserData()->user_id, oldPass.c_str(), newPass.c_str()))
	{
		CCLOG("[%s]:%s", "LayerRegister::btRegisterClicked", "user register error");

		LayerLoading::CloseWithTip("user register net error");

		return;
	}
	
}






void LayerPassword::onEnter()
{
	Layer::onEnter();
	//set avatars list

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
void LayerPassword::onExit()
{
	//注销消息通知
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	Layer::onExit();
}
bool LayerPassword::onTouchBegan(Touch *touch, Event *event){
	if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce([=](float){CloseWithAction(this); }, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
	return true;
}




void LayerPassword::onUpdatePasswordCallBack(Ref *pSender)
{   


	//SetNickName回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		
		LayerLoading::Close();
		if (msg->code == 0)
		{
			//this->removeFromParent();
			Api::User::GetUserInfo();		//申请一次新的数据，scene会自动更新数据
			//是否重置默认登录
		
			_oldPassEditBox->setText(""); oldPassCheck->setVisible(false);
			_newPassEditBox->setText(""); newPassCheck->setVisible(false);
			_confirmPassEditBox->setText(""); confirmPassCheck->setVisible(false);

			btnConfirm->setEnabled(false);
			btnConfirm->setBright(false);
			
		    
			UserDefault::getInstance()->setIntegerForKey("LoginType", ACCOUNTTYPE::NONE);
			SocketControl::GetInstance()->close();
			auto scene = SceneLogin::create();
			Director::getInstance()->replaceScene(scene);
			
			LayerLoading::CloseWithTip(Language::getStringByKey("PasswordResetSuccess"));
		}
		else
		{
			 //login fail
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneLogin::onLoginCallBack", "login error.", msg->code, msg->data.c_str());

			LayerLoading::CloseWithTip(Language::getStringByKey("PasswordIncorrect"));
		}
		msg->release();
	});
}


bool LayerPassword::CheckOldPassword()
{
	std::string password = _oldPassEditBox->getText();
	if (password.empty())
	{
		//lbPassword->setString(Language::getStringByKey("PasswordCanNotEmpty"));
		oldPassCheck->setTexture("login/img/RegisterWrong.png");
		oldPassCheck->setVisible(true);
		return false;
	}
	if (password.length()<6)
	{
		//lbPassword->setString(Language::getStringByKey("PasswordLenError"));
		oldPassCheck->setTexture("login/img/RegisterWrong.png");
		oldPassCheck->setVisible(true);
		return false;
	}
	//lbPassword->setString("");
	oldPassCheck->setTexture("login/img/RegisterRight.png");
	oldPassCheck->setVisible(true);
	return true;
}

bool LayerPassword::CheckNewPassword()
{
	std::string password = _newPassEditBox->getText();
	if (password.empty())
	{
		//lbPassword->setString(Language::getStringByKey("PasswordCanNotEmpty"));
		newPassCheck->setTexture("login/img/RegisterWrong.png");
		newPassCheck->setVisible(true);
		return false;
	}
	if (password.length()<6)
	{
		//lbPassword->setString(Language::getStringByKey("PasswordLenError"));
		newPassCheck->setTexture("login/img/RegisterWrong.png");
		newPassCheck->setVisible(true);
		return false;
	}
	//lbPassword->setString("");
	newPassCheck->setTexture("login/img/RegisterRight.png");
	newPassCheck->setVisible(true);
	return true;
}

bool LayerPassword::CheckConfirmPassword()
{
	if (strcmp(_newPassEditBox->getText(), _confirmPassEditBox->getText()) != 0)
	{
		//lbPassword1->setString(Language::getStringByKey("Password1Error"));
		confirmPassCheck->setTexture("login/img/RegisterWrong.png");
		confirmPassCheck->setVisible(true);
		return false;
	}
	//lbPassword1->setString("");
	confirmPassCheck->setTexture("login/img/RegisterRight.png");
	confirmPassCheck->setVisible(true);
	return true;
}


void LayerPassword::editBoxReturn(EditBox* editBox)
{
	//if (_editBoxNickName == editBox)
	//	CheckNickName();		//检查 昵称

	if (_oldPassEditBox == editBox)
		CheckOldPassword();
	else if (_newPassEditBox == editBox)
		CheckNewPassword();
	else if (_confirmPassEditBox == editBox)
	{
		////密码框输入确认后，直接登录
		
		if (CheckConfirmPassword()&&CheckOldPassword() && CheckNewPassword()) //检验通过
		{
			btnConfirm->setBright(true);
			btnConfirm->setEnabled(true);
		}
	}
}