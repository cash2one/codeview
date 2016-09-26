#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIScrollView.h"
USING_NS_CC;
using namespace cocos2d::ui;
struct DateData
{
	ScrollView *scroll;
	float lbHeight;		//控件高度
	float span;			//控件间距,可以为负数
	float blank;		//头尾留空大小
	float centerX;
	int value;				//当前值，即年月日
	int valueStart;			//起始日期
	int valueCount;				//数量
	Size contentSize;		//滚动区域大小
};
class LayerDate
{
public:
	Node *_nodeContent;		//根节点
protected:
	Text *lbYearModel, *lbMonthModel, *lbDayModel;		//年，月，日控件模型
	bool initScroll(ScrollView *scroll, DateData *data);				//初始化年月日控件
	void setScrolContent(DateData* data);
	void setDayCount();
	Text *cloneText(Text* lb);
	void checkValue();
public:
	DateData dateYear, dateMonth, dateDay;
	DateData *_currentDateData;
	bool init(Node* root);
	//touch事件监听 屏蔽向下触摸
	bool onTouchBegan(Touch *touch, Event *event);
	void onTouchMoved(Touch* touch, Event* event);
	void onTouchEnded(Touch* touch, Event* event);
	void setInitYMD(int year, int month, int day);

private:
	int mInitYear;
	int mInitMonth;
	int mInitDay;
};