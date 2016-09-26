#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIEditBox/UIEditBox.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
#include "ui/UIListView.h"
#include "ui/UILoadingBar.h"
#include "ui/UITextBMFont.h"
#include "ui/UIScrollView.h"
#include "ui/UITextField.h"
#include "comm.h"
USING_NS_CC;
using namespace cocos2d::ui;




//, public EditBoxDelegate
class LayerMailWriter : public Layer, public EditBoxDelegate
{
protected:
	//bool _enableExit;		//是否允许退出，首次设置时，不允许退出
	Node *RootNode;
	TextBMFont *txtCoins, *txtLevel;
	Layout *touch_layer;
	Button *btnSend;
	TextField* text_to, *text_content;
	
	EditBox *_textToEditBox, *_textContentEditBox;
	
	void funSendMessage();
	//

	bool checkEmailTo();
	bool checkContent();

	int send_user_id;
	
public:
	/*Text *lbNickName;*/
	int _avatarId;
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter();
	virtual void onExit();

	LayerMailWriter();

	void onTriggleSetMailTo(Ref *pSender);

	void setSendTo(string sendTo,int send_user_id);

	virtual void editBoxReturn(EditBox* editBox);
	
	
	CREATE_FUNC(LayerMailWriter);
};