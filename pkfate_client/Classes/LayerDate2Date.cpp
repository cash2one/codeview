//
//  LayerDate2Date.cpp
//  PKFate
//
//  Created by claudis on 15-9-10.
//
//

#include "LayerDate2Date.h"
#include "bjlcomm.h"
#include "PKNotificationCenter.h"
#include "comm.h"
bool LayerDate2Date::init(const char *from, const char *to, std::string flag) {
	if (!from || !to || !Layer::init()) {
		return false;
	}

	mflag = flag;

	/*	载入cocos studio 资源
	*/
	RootNode = CSLoader::createNode("LayerDate2Date.csb");
	if (!RootNode)
		return false;
	Layer *layerDateFrom, *layerDateTo;
	if (!(panelDate = RootNode->getChildByName<Layout*>("panelDate")))
		return false;
	if (!(layerDateFrom = panelDate->getChildByName<Layer*>("layerDateFrom")))
		return false;
	auto *lbTitle = RootNode->getChildByName<Text*>("lbTitle");
	if (!lbTitle) return false;
	lbTitle->setString(Language::getStringByKey("DateSet"));
	DateFrom = new LayerDate();
	int y, m, d;
	if (sscanf(from, "%d-%d-%d", &y, &m, &d) != 3)
		return false;
	DateFrom->setInitYMD(y, m, d);
	/*std::vector<std::string> vecStringFrom = NBUtil::split(from->getString().c_str(), "-");
	DateFrom->setInitYMD(atoi(vecStringFrom[0].c_str()), atoi(vecStringFrom[1].c_str()), atoi(vecStringFrom[2].c_str()));*/
	DateFrom->init(layerDateFrom);

	if (!(layerDateTo = panelDate->getChildByName<Layer*>("layerDateTo")))
		return false;
	DateTo = new LayerDate();
	if (sscanf(to, "%d-%d-%d", &y, &m, &d) != 3)
		return false;
	DateTo->setInitYMD(y, m, d);
	/*std::vector<std::string> vecStringTo = NBUtil::split(to->getString().c_str(), "-");
	DateTo->setInitYMD(atoi(vecStringTo[0].c_str()), atoi(vecStringTo[1].c_str()), atoi(vecStringTo[2].c_str()));*/
	DateTo->init(layerDateTo);


	Text* txtDataFromTitle = panelDate->getChildByName<Text*>("txtDataFromTitle");
	txtDataFromTitle->setText(Language::getStringByKey("DateStart"));
	Text* txtDataEndTitle = panelDate->getChildByName<Text*>("txtDataEndTitle");
	txtDataEndTitle->setText(Language::getStringByKey("DateEnd"));
	//设定
	if (!(btOk = panelDate->getChildByName<Button*>("btOk")))
		return false;
	btOk->setTitleText(Language::getStringByKey("Confirm"));
	btOk->addClickEventListener([=](Ref *ref){
		okCallBack();
	});

	//取消
	if (!(btCancel = panelDate->getChildByName<Button*>("btCancel")))
		return false;
	btCancel->setTitleText(Language::getStringByKey("Cancel"));
	btCancel->addClickEventListener([=](Ref *ref){
		cancleCallBack();
	});

	this->addChild(RootNode);

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(LayerDate2Date::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerDate2Date::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerDate2Date::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	return true;
}

bool LayerDate2Date::onTouchBegan(Touch *touch, Event *event) {
	if (!panelDate->getBoundingBox().containsPoint(touch->getLocation()))
		this->scheduleOnce(schedule_selector(LayerDate2Date::DelayExit), 0.1f);
	return true;
}

void LayerDate2Date::onTouchMoved(Touch *touch, Event *event) {

}

void LayerDate2Date::onTouchEnded(Touch* touch, Event* event) {

}

void LayerDate2Date::DelayExit(float dt)
{
	this->removeFromParent();
}



void LayerDate2Date::okCallBack() {

	int Yfrom = DateFrom->dateYear.value;
	int Mfrom = DateFrom->dateMonth.value;
	int Dfrom = DateFrom->dateDay.value;

	int Yto = DateTo->dateYear.value;
	int Mto = DateTo->dateMonth.value;
	int Dto = DateTo->dateDay.value;


	long fromsec = (Yfrom - 1) * 365 * 24 * 3600 + (Mfrom - 1) * 30 * 24 * 3600 + Dfrom * 24 * 3600;
	long tosec = (Yto - 1) * 365 * 24 * 3600 + (Mto - 1) * 30 * 24 * 3600 + Dto * 24 * 3600;
	if (tosec >= fromsec) {
		DateFromTo *fromto = new DateFromTo();
		char strBuf[255];
		sprintf(strBuf, "%d-%02d-%02d", Yfrom, Mfrom, Dfrom);
		fromto->from = std::string(strBuf);
		sprintf(strBuf, "%d-%02d-%02d", Yto, Mto, Dto);
		fromto->to = std::string(strBuf);
		PKNotificationCenter::getInstance()->postNotification(mflag, fromto);
		fromto->release();
		this->removeFromParent();
	}
	else {
		CCLOG("BJL ################ 发生错误:起始日期大于终止日期");
		this->removeFromParent();
	}
}

void LayerDate2Date::cancleCallBack() {
	this->removeFromParent();
}



LayerDate2Date* LayerDate2Date::create(const char *from, const char *to, std::string flag)
{
	LayerDate2Date *pRet = new(std::nothrow) LayerDate2Date();
	if (pRet && pRet->init(from, to, flag))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}

