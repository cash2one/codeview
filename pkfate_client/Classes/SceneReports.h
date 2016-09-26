#pragma once

#include "cocos2d.h"
#include "ui/UILayout.h"
#include "ui/UIButton.h"
#include "ui/UICheckBox.h"
#include "ui/UIText.h"
#include "ReportControl.h"

USING_NS_CC;
using namespace cocos2d::ui;
using namespace Report;

class SceneReports : public Layer
{
public:
	SceneReports();
	~SceneReports();
	bool init();
	virtual void onEnter();

	void onDateChangedCallBack(Ref *pSender);
	
	void cbUserFormClick();
	void cbGameFormClick();
	void cbProfitFormClick();
	void onDateClicked(cocos2d::Ref *ref, Widget::TouchEventType touchType);

	CREATE_FUNC(SceneReports);

protected:
	Node *RootNode;
	Layout *PanelDate;
	Button *btnBack, *btnSearch;
	CheckBox *cbUserForm, *cbGameForm, *cbProfitForm,*currentForm;
	ReportsControl *_reportsControl;
	REPORTTYPE _curType;
	void showReport(REPORTTYPE type);			//显示报表
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
private:
	//日期
	Text *txtLocalTime, *txtFrom, *txtTo;
	void updateLocalTimeString();
	int _userid;
};