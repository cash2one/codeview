#include "LayerDate.h"
#include "ui/UIImageView.h"
#define RANGEYEAR 2		//显示年份的范围，上下2年
bool LayerDate::init(Node* root)
{
	if (!root)
		return false;
	if (!initScroll(root->getChildByName<ScrollView*>("scrollYear"), &dateYear)
		|| !initScroll(root->getChildByName<ScrollView*>("scrollMonth"), &dateMonth)
		|| !initScroll(root->getChildByName<ScrollView*>("scrollDay"), &dateDay))
		return false;
	_nodeContent = root;
	_currentDateData = nullptr;
	//设置初始值
	//time_t是long类型，精确到秒，是当前时间和1970年1月1日零点时间的差
	const time_t t = time(NULL);
	/*本地时间：日期，时间 年月日，星期，时分秒*/
	struct tm* current_time = localtime(&t);
	/*
	SYSTEMTIME st = { 0 };
	GetLocalTime(&st);
	dateYear.value = st.wYear;
	dateYear.valueStart = dateYear.value - RANGEYEAR;
	dateYear.valueCount = 2 * RANGEYEAR + 1;
	dateMonth.value = st.wMonth;
	dateMonth.valueStart = 1;
	dateMonth.valueCount = 12;
	dateDay.value = st.wDay;
	dateDay.valueStart = 1;*/
	dateYear.value = mInitYear;
	dateYear.valueStart = dateYear.value - RANGEYEAR;
	dateYear.valueCount = 2 * RANGEYEAR + 1;
	dateMonth.value = mInitMonth;
	dateMonth.valueStart = 1;
	dateMonth.valueCount = 12;
	dateDay.value = mInitDay;
	dateDay.valueStart = 1;
	setDayCount();		//设置天数
	//设置控件
	setScrolContent(&dateYear);
	setScrolContent(&dateMonth);
	setScrolContent(&dateDay);

	CCLOG("claudis ############## Y = %d M = %d D = %d", mInitYear, mInitMonth, mInitDay);

	//add layer touch event
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(false);
	listener->onTouchBegan = CC_CALLBACK_2(LayerDate::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(LayerDate::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(LayerDate::onTouchEnded, this);
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, root);
	Rect rect = _nodeContent->getBoundingBox();
	CCLOG("content rect:%.0f;%.0f;%.0f;%.0f;", rect.getMinX(), rect.getMinY(), rect.getMaxX(), rect.getMaxY());
	return true;
}
bool LayerDate::onTouchBegan(Touch *touch, Event *event)
{
	/*
	CCLOG("name:%s", _nodeContent->getName().c_str());
	if (event->getCurrentTarget() != _nodeContent)
	return false;
	Rect rect = dateYear.scroll->getBoundingBox();
	CCLOG("dateYear rect:%.0f;%.0f;%.0f;%.0f;", rect.getMinX(), rect.getMinY(), rect.getMaxX(), rect.getMaxY());
	Vec2 pt = _nodeContent->convertTouchToNodeSpace(touch);
	CCLOG("pt:%.0f;%.0f", pt.x, pt.y);
	return false;*/
	Vec2 pt = _nodeContent->convertTouchToNodeSpace(touch);
	if (dateYear.scroll->getBoundingBox().containsPoint(pt))
	{
		CCLOG("%s:touch year", _nodeContent->getName().c_str());
		_currentDateData = &dateYear;
		return true;
	}
	if (dateMonth.scroll->getBoundingBox().containsPoint(pt))
	{
		CCLOG("%s:touch month", _nodeContent->getName().c_str());
		_currentDateData = &dateMonth;
		return true;
	}
	if (dateDay.scroll->getBoundingBox().containsPoint(pt))
	{
		CCLOG("%s:touch day", _nodeContent->getName().c_str());
		_currentDateData = &dateDay;
		return true;
	}
	return false;
}
void LayerDate::onTouchMoved(Touch* touch, Event* event)
{

}
void LayerDate::onTouchEnded(Touch* touch, Event* event)
{
	CCLOG("%s:touch end", _nodeContent->getName().c_str());
	_nodeContent->scheduleOnce([=](float dt){checkValue(); }, 0.5f, "checkvalue");
}
//初始化年月日控件模型，每个scroll包含上中下三个文本控件
bool LayerDate::initScroll(ScrollView *scroll, DateData *data)
{
	if (!data || !scroll || scroll->getChildrenCount() != 3)
		return false;
	data->scroll = scroll;
	data->scroll->setSwallowTouches(false);		//允许事件传递到下层，以便知道何时结束触摸
	//获取文本模型
	auto lb = (Text*)scroll->getChildren().at(0);
	auto lb1 = (Text*)scroll->getChildren().at(1);
	//计算间距
	Rect rect = lb->getBoundingBox();
	Rect rect1 = lb1->getBoundingBox();
	data->span = rect.getMinY() - rect1.getMaxY();
	//获取scroll大小，及计算滚动初始位置
	data->contentSize = scroll->getContentSize();
	Size sizeLb = lb->getContentSize();
	data->lbHeight = sizeLb.height;
	data->centerX = data->contentSize.width / 2;
	data->blank = (data->contentSize.height - sizeLb.height) / 2;		//计算头尾留空大小，即滚动到第一条时，上方的空余位置,尾部相同
	return true;
}
//根据月和年，计算日期天数
void LayerDate::setDayCount()
{
	if (dateMonth.value == 2)
		dateDay.valueCount = dateYear.value % 4 ? 28 : 29;		//计算是否闰年,简单用4求余，并非100%准确
	else if (dateMonth.value == 4 || dateMonth.value == 6 || dateMonth.value == 9 || dateMonth.value == 11)
		dateDay.valueCount = 30;
	else
		dateDay.valueCount = 31;
}
Text *LayerDate::cloneText(Text* lb)
{
	if (!lb)
		return nullptr;
	auto newlb = (Text*)lb->clone();
	newlb->setFontName(lb->getFontName());
	newlb->setFontSize(lb->getFontSize());
	newlb->setTextColor(lb->getTextColor());
	return newlb;
}
void LayerDate::setScrolContent(DateData* data)
{
	if (!data || !data->scroll || data->scroll->getChildrenCount() < 1)
		return;
	//获取文本模型
	auto lbModel = cloneText((Text*)data->scroll->getChildren().at(0));
	//复制Text属性，clone时，不会复制这些属性

	data->scroll->removeAllChildren();
	data->contentSize.height = data->blank * 2 + data->lbHeight*data->valueCount + (data->valueCount - 1)*data->span;		//滚动区总高度
	float posY = data->contentSize.height - data->blank - data->lbHeight / 2;		//起始位置
	for (int i = data->valueStart; i < (data->valueStart + data->valueCount); i++)
	{
		auto lb = cloneText(lbModel);
		lb->setPosition(Vec2(data->centerX, posY));
		lb->setString(StringUtils::format("%d", i));
		data->scroll->addChild(lb);
		posY -= (data->lbHeight + data->span);
	}
	//设置滚动区大小，并滚动到当前值
	data->scroll->setInnerContainerSize(data->contentSize);
	posY = (data->value - data->valueStart)*(data->lbHeight + data->span);
	//data->scroll->scrollToPercentVertical(posY / data->contentSize.height * 100, 0.5f, false);
	//data->scroll->scrollToPercentVertical((float)(data->value - data->valueStart) / data->valueCount * 100, 0.0f, false);
	data->scroll->scrollToPercentVertical((float)(data->value - data->valueStart) / (data->valueCount - 1) * 100, 0.5f, false);
}
void LayerDate::checkValue()
{
	//校准数据
	if (!_currentDateData)
		return;
	auto layout = _currentDateData->scroll->getInnerContainer();
	Size size = layout->getContentSize();
	Vec2 pt = layout->getPosition();
	int miny = 0, maxy = (int)(size.height - _currentDateData->blank - _currentDateData->lbHeight);
	int step = maxy / (_currentDateData->valueCount - 1);
	int mod = (int)(-pt.y) % step;
	int index = (maxy - (int)(-pt.y)) / step;
	int value = _currentDateData->valueStart + index;
	_currentDateData->value = value;
	if (mod > 0)
	{
		//需要校准位置
		_currentDateData->scroll->scrollToPercentVertical((float)(_currentDateData->value - _currentDateData->valueStart) / (_currentDateData->valueCount - 1) * 100, 0.5f, false);
	}
	if (_currentDateData == &dateYear || _currentDateData == &dateMonth)
	{
		//如果年和月被修改，需要重新设置日
		int oldCount = dateDay.valueCount;
		setDayCount();
		if (oldCount != dateDay.valueCount)
		{
			if (dateDay.value >= (dateDay.valueStart + dateDay.valueCount))
			{
				//新的月份比原来的月份天数要少，必须重新设置
				dateDay.value = dateDay.valueStart + dateDay.valueCount - 1;
			}
			setScrolContent(&dateDay);
		}
	}
	_currentDateData = nullptr;
}

void LayerDate::setInitYMD(int year, int month, int day) {
	mInitYear = year;
	mInitMonth = month;
	mInitDay = day;
}