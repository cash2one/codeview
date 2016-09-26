#include "SceneReports.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
#include "UserControl.h"
#include "LayerDate2Date.h"
#include "bjlcomm.h"
#include "PKNotificationCenter.h"
#include "SoundControl.h"
SceneReports::SceneReports():
_curType((REPORTTYPE)-1),currentForm(nullptr){
}
SceneReports::~SceneReports() {
	PKNotificationCenter::getInstance()->removeAllObservers(this);
	if (_reportsControl)
		delete _reportsControl;
}

bool SceneReports::init() {
	if (!Layer::init()) {
		return false;
	}

	/*auto listener = EventListenerKeyboard::create();
	listener->onKeyReleased = CC_CALLBACK_2(SceneReports::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);*/

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("report/SceneReports.csb");
	if (!RootNode)
		return false;
	Layout *PanelControl = nullptr;
	if (!(PanelControl = RootNode->getChildByName<Layout*>("panelControl")))
		return false;
	if (!(cbUserForm = PanelControl->getChildByName<CheckBox*>("CBUserForm")))
		return false;
	cbUserForm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		cbUserFormClick();
	});
	if (!(cbGameForm = PanelControl->getChildByName<CheckBox*>("CBGameForm")))
		return false;
	cbGameForm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		cbGameFormClick();
	});
	if (!(cbProfitForm = PanelControl->getChildByName<CheckBox*>("CBProfitForm")))
		return false;
	cbProfitForm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		cbProfitFormClick();
	});
	if (!(btnBack = PanelControl->getChildByName<Button*>("BtBack")))
		return false;
	btnBack->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_RETURN);
		//Director::getInstance()->popScene();
		this->removeFromParent();
	});
	//日期控件
	if (!(PanelDate = PanelControl->getChildByName<Layout*>("PanelDate")))
		return false;
	if (!(txtLocalTime = PanelDate->getChildByName<Text*>("txtLocalTime")))
		return false;
	if (!(txtFrom = PanelDate->getChildByName<Text*>("txtFrom")))
		return false;
	if (!(txtTo = PanelDate->getChildByName<Text*>("txtTo")))
		return false;
	PanelDate->addTouchEventListener(CC_CALLBACK_2(SceneReports::onDateClicked, this));
	PanelDate->setTouchEnabled(true);
	PanelDate->setVisible(false);
	const time_t t = time(NULL);
	struct tm* current_time = localtime(&t);
	string strDate = StringUtils::format("%d-%02d-%02d %02d:%02d:%02d", current_time->tm_year + 1900, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	strDate = Comm::GetUTCStringFromLocalTimeStrng(strDate).substr(0, 10);
	txtFrom->setString(strDate);
	txtTo->setString(strDate);
	updateLocalTimeString();

	//报表控件类
	if (!(_reportsControl = ReportsControl::create(RootNode)))
		return false;

	this->addChild(RootNode);
	//监听报表日期修改
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(SceneReports::onDateChangedCallBack), "DateChanged", nullptr);
	
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("img/PlistSmallCard.plist");	//加载small牌面


	//验证完成任务 首次查看游戏报表
	
	PKNotificationCenter::getInstance()->postNotification("TriggerDoneRookieTask", String::create(toString(7)));

	return  true;
}

void SceneReports::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	log("Key with keycode %d released", keyCode);
	if (keyCode == EventKeyboard::KeyCode::KEY_ESCAPE)
	{
		//ShowTip(Language::getStringByKey("WantLeaveRoom"), [=](){
			//Director::getInstance()->popScene();
		//});
		this->removeFromParent();
	}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

void SceneReports::onEnter() {
	Layer::onEnter();
	//首次进入时，显示自己的用户表
	cbUserFormClick();
}
void SceneReports::onDateChangedCallBack(Ref *pSender) {
	if (pSender == nullptr)
		return;
	DateFromTo* msg = (DateFromTo*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (txtFrom->getString() != msg->from || txtTo->getString() != msg->to) {
			//报表日期修改
			txtFrom->setString(msg->from);
			txtTo->setString(msg->to);
			updateLocalTimeString();
			_reportsControl->onDateChanged(msg->from, msg->to);
		}
		msg->release();
	});
}
void SceneReports::cbUserFormClick() {
	showReport(REPORTTYPE::USERREPORT);
}
void SceneReports::cbGameFormClick() {
	showReport(REPORTTYPE::GAMEREPORT);
}
void SceneReports::cbProfitFormClick() {
	showReport(REPORTTYPE::PROFITREPORT);
}
void SceneReports::onDateClicked(cocos2d::Ref *ref, Widget::TouchEventType touchType) {
	if (touchType == Widget::TouchEventType::ENDED) {
		LayerDate2Date *pLayerDate2Date = LayerDate2Date::create(txtFrom->getString().c_str(), txtTo->getString().c_str(), "DateChanged");
		this->addChild(pLayerDate2Date);
	}
}

void SceneReports::updateLocalTimeString()
{
	string strFrom = txtFrom->getString() + " 00:00:00";
	string strTo = txtTo->getString() + " 23:59:59";
	string localTime = StringUtils::format("%s: %s - %s", UserDefault::getInstance()->getStringForKey("LangType") == "zh" ? "本地时间" : "Local Time", Comm::GetLocalTimeStrngFromUTCString(strFrom).c_str(), Comm::GetLocalTimeStrngFromUTCString(strTo).c_str());
	txtLocalTime->setString(localTime);
}
void SceneReports::showReport(REPORTTYPE type){

	if (currentForm != nullptr){
		currentForm->getChildByName("label_normal")->setVisible(true);
		currentForm->getChildByName("label_select")->setVisible(false);
	}
	switch (type)
	{
	case Report::USERREPORT:
		cbUserForm->setSelected(true);
		cbGameForm->setSelected(false);
		cbProfitForm->setSelected(false);
		cbUserForm->setEnabled(false);
		cbGameForm->setEnabled(true);
		cbProfitForm->setEnabled(true);
		cbUserForm->setHighlighted(true);
		cbGameForm->setHighlighted(false);
		cbProfitForm->setHighlighted(false);
		PanelDate->setVisible(false);
		currentForm = cbUserForm;
		break;
	case Report::GAMEREPORT:
		cbUserForm->setSelected(false);
		cbGameForm->setSelected(true);
		cbProfitForm->setSelected(false);
		cbUserForm->setEnabled(true);
		cbGameForm->setEnabled(false);
		cbProfitForm->setEnabled(true);
		cbUserForm->setHighlighted(false);
		cbGameForm->setHighlighted(true);
		cbProfitForm->setHighlighted(false);
		PanelDate->setVisible(true);
		currentForm = cbGameForm;
		break;
	case Report::PROFITREPORT:
		cbUserForm->setSelected(false);
		cbGameForm->setSelected(false);
		cbProfitForm->setSelected(true);
		cbUserForm->setEnabled(true);
		cbGameForm->setEnabled(true);
		cbProfitForm->setEnabled(false);
		cbUserForm->setHighlighted(false);
		cbGameForm->setHighlighted(false);
		cbProfitForm->setHighlighted(true);
		PanelDate->setVisible(true);
		currentForm = cbProfitForm;
		break;
	default:
		return;
	}
	_curType = type;
	if (currentForm != nullptr){
		currentForm->getChildByName("label_normal")->setVisible(false);
		currentForm->getChildByName("label_select")->setVisible(true);
	}
	_reportsControl->showReport(_curType, txtFrom->getString(), txtTo->getString());
}