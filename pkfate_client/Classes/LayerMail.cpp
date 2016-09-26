#include "LayerMail.h"
#include "cocostudio/CocoStudio.h"

#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "LayerMailWriter.h"
#include "LayerMailReader.h"
#include "LayerDuang.h"

MailItem::MailItem()
{
	Node *_nodeRoot = CSLoader::createNode("mail/MailItem.csb");
	addChild(_nodeRoot);
	_title = (Text*)_nodeRoot->getChildByName("title");
	_date = (Text*)_nodeRoot->getChildByName("date");

	_from = (Text*)_nodeRoot->getChildByName("from");

	read_icon = _nodeRoot->getChildByName<Sprite*>("read_icon");
	unread_icon = _nodeRoot->getChildByName<Sprite*>("unread_icon");

	_btnItem = (Button*)_nodeRoot->getChildByName("btn_item");
	_btnItem->setSwallowTouches(false);
	_btnItem->addClickEventListener([&](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerMailReader * mailReader = this->getChildByName<LayerMailReader*>("LayerMailReader");
		if (!mailReader){
			mailReader = LayerMailReader::create(); //临时创建释放 avoid texture memory overflow
			mailReader->setName("LayerMailReader");
			Director::getInstance()->getRunningScene()->addChild(mailReader, 2);
			mailReader->setMessageData(this->messageData);
		}
	});

	this->setContentSize(Size(1880.00,160));

}

void MailItem::setProperty(MessageData* messageData)
{
	this->messageData = messageData;
	_from->setString(this->messageData->sender_nickname);
	_title->setString(messageData->title);
	_date->setString(messageData->create_time);

	if (this->messageData->status == NOTICE_MSG_STATUS::READED){
		read_icon->setVisible(true);
		unread_icon->setVisible(false);
		_title->setTextColor(ccc4(133, 133, 133,255));
	}
	else{
		read_icon->setVisible(false);
		unread_icon->setVisible(true);
		_title->setTextColor(ccc4(255, 255, 255, 255));
	}

}

MailItem::~MailItem()
{

}



LayerMail::LayerMail(){

}

bool LayerMail::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("mail/LayerMail.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;

	mail_tip = _nodeRoot->getChildByName<Text*>("mail_tip");
	if (!mail_tip) return false;

	mail_tip->setString(Language::getStringByKey("NoMail"));
	Button *btnPerson = _nodeRoot->getChildByName<Button*>("btn_person");
	Button *btnSystem = _nodeRoot->getChildByName<Button*>("btn_system");

	btnPerson->addClickEventListener([=](Ref *ref){
		btnPerson->setEnabled(false);
		btnPerson->setBright(false);
		btnSystem->setEnabled(true);
		btnSystem->setBright(true);
		this->currentType = NOTICE_MSG_TYPE::USER_MSG;
		PKNotificationCenter::getInstance()->postNotification("MailSwitch", String::create(toString(getMailType()))); //
	});

	btnSystem->addClickEventListener([=](Ref *ref){
		btnSystem->setEnabled(false);
		btnSystem->setBright(false);
		btnPerson->setEnabled(true);
		btnPerson->setBright(true);
		this->currentType = NOTICE_MSG_TYPE::SYS_MSG;
		PKNotificationCenter::getInstance()->postNotification("MailSwitch", String::create(toString(getMailType()))); //
	});

	mailList = (ListView*)_nodeRoot->getChildByName("mail_list");
	 //mailList->setBackGroundColorOpacity(0);
	mailList->setItemsMargin(30);

	Button *btBack = _nodeRoot->getChildByName<Button*>("btBack");


	btBack->addClickEventListener([&](Ref *ref){
		this->removeFromParent();

		LayerDuang::panelStatus();
		LayerDuang::uiGameMenu();
		LayerDuang::dzGamesRoom();
		LayerDuang::bjlGamesRoom();
		LayerDuang::bjlPanelLobby();
		LayerDuang::dzPanelLobby();
	});

	Button * btnMail = _nodeRoot->getChildByName<Button*>("btn_mail");
	btnMail->addClickEventListener([&](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);

		LayerMailWriter * mailWriter = this->getChildByName<LayerMailWriter*>("LayerMailWriter");
		if (!mailWriter){
			mailWriter = LayerMailWriter::create(); //临时创建释放 avoid texture memory overflow
			mailWriter->setName("LayerMailWriter");
			this->addChild(mailWriter, 2);
		}
	});



	MailControl::GetInstance()->setMailView(this); //添加视图
	

	this->currentType = NOTICE_MSG_TYPE::USER_MSG;
	btnPerson->setEnabled(false);
	btnPerson->setBright(false);

	PKNotificationCenter::getInstance()->postNotification("MailSwitch", String::create(toString(getMailType()))); //第一次 默认切换

	return true;
}


int LayerMail::getMailType()
{
	return this->currentType;
}

void LayerMail::updateMailList()
{
	vector<MessageData*> list = MailControl::GetInstance()->getMailList(getMailType());


	for (int i = 0, length = list.size(); i < length; i++){  //数据已顺序排列
		MailItem *item = (MailItem *)mailList->getItem(i);// list item 重复利用
		if (item == nullptr)
		{
			item = new MailItem();// list item 重复利用
			mailList->pushBackCustomItem(item);//少了加
		}
			
		MessageData *messageData = list[i];
		item->setProperty(messageData);
	}
	int mi = mailList->getChildrenCount() - list.size();// 多出item 数目
	while (mi-- > 0)
	{
		mailList->removeLastItem();
	
	}
	
	mailList->refreshView();//记得刷新，更新innerContainer size
	mailList->jumpToTop();

	mail_tip->setVisible(list.size() <= 0);
}




LayerMail::~LayerMail()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
	 MailControl::GetInstance()->setMailView(nullptr);  //释放视图
}


void LayerMail::onButtonExitClicked(Ref *ref){
	this->removeFromParent();
}
