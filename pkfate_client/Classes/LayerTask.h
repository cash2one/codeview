#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UILayout.h"
#include "ui/UITextBMFont.h"
#include "ui/UIListView.h"
#include "comm.h"
#include "cJSON.h"
#include "TaskControl.h"
USING_NS_CC;
using namespace cocos2d::ui;
using namespace std;


class TaskItem :public Widget
{
	// ----------------自定义	
protected:
	

	Sprite  *_taskProgress;

	Text *_taskAward, *_taskName;

	Button* _btnStatus;

	TaskData* taskData;

	

public:

	TaskItem();
	~TaskItem();
	CREATE_FUNC(TaskItem);
	void setProperty(TaskData *taskData);
	TaskData* getTaskData();
	Button* getStatusBtn();
	
};


class LayerTask :public Layer
{
	// ----------------自定义	
protected:	
	Button *_btExit;


	ListView * taskList;
	bool daily;


public:
	void selectList(bool daily = false);
	LayerTask();
	~LayerTask();
	CREATE_FUNC(LayerTask);
	void UpdateTaskList();//更新taskList
	void onButtonExitClicked(Ref *ref);


	// ----------------自定义结束
	virtual bool init();
};
