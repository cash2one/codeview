#include "LayerInviteDzWait.h"
#include "cocostudio/CocoStudio.h"
#include "FriendControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "LayerFriendInvite.h"
#include "SceneDzRoom.h"

LayerInviteDzWait::LayerInviteDzWait()
{

}

bool LayerInviteDzWait::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("friend/LayerInviteDzWait.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;

	Text* label_tip = _nodeRoot->getChildByName<Text*>("label_tip");
	if (!label_tip) return false;
	label_tip->setString(Language::getStringByKey("WaitJoinDzTip"));
	

	btn_confirm = _nodeRoot->getChildByName<Button*>("btn_confirm");
	if (!btn_confirm)return false;
	btn_confirm->setTitleText(Language::getStringByKey("Confirm"));

	btn_wait = _nodeRoot->getChildByName<Button*>("btn_wait");
	if (!btn_wait)return false;
	btn_wait->setTitleText(Language::getStringByKey("Wait"));

	btn_confirm->addClickEventListener([&](Ref *ref){
		inviteFriend();//打开列表邀请好友
	});

	btn_wait->addClickEventListener([&](Ref *ref){
		wait();//进入房间等待
	});

	return true;
}

void LayerInviteDzWait::inviteFriend()
{
	LayerFriendInvite * layerFriendInvite = LayerFriendInvite::create(); //临时创建释放 avoid texture memory overflow
	layerFriendInvite->setFrom(FROMPAGE::WAIT_INVITE_PAGE);
	Director::getInstance()->getRunningScene()->addChild(layerFriendInvite, 2);

	this->removeFromParent();
}

void LayerInviteDzWait::wait()
{
	this->removeFromParent();
	
}

LayerInviteDzWait::~LayerInviteDzWait()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void LayerInviteDzWait::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
