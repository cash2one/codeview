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



class LayerNotice :public Layer
{
	// ----------------自定义	
protected:	
	Text* notice_title,*notice_content;
	ListView* title_list;
	ScrollView *content_scrollView;

	Button* currentBtn;

	void selectBtn(Button* btn);

	


public:
	bool fromSet;
	void updateNoticeList();
	LayerNotice();
	~LayerNotice();
	CREATE_FUNC(LayerNotice);

	// ----------------自定义结束
	virtual bool init();
};
