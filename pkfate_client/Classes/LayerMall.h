#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UIScrollView.h"
#include "ui/UILayout.h"
#include "ui/UITextBMFont.h"
#include "ui/UILoadingBar.h"
#include "comm.h"
#include "cJSON.h"
USING_NS_CC;
using namespace cocos2d::ui;
class LayerMall :public Layer
{
	// ----------------自定义	
protected:	
	Button *_btExit;

	Node *_selectItem;

	Sprite *_light;//发光


	//Text *_textResultWin, *_textResultLose, *_textResultTotal;
public:
	LayerMall();
	~LayerMall();
	CREATE_FUNC(LayerMall);
	void updateBlance();
	void touchHandler(Ref* sender, ui::Widget::TouchEventType type);
	void UpdateBalance(Ref *ref);//更新balance
	void onButtonExitClicked(Ref *ref);
	
	void onTagButtonClicked(Ref *ref);
	// ----------------自定义结束
	virtual bool init();

	void onHideBtnLotteryNotification(Ref *pSender);

	virtual void onEnterTransitionDidFinish();

	bool refreshProductConversion();
private:
	Button *_pBtnMall, *_pBtnConversion, *_pBtnVip, *_pBtnLottery;
	Sprite *_pTagPitch;
	Layout *_pLayoutMall, *_pLayoutConversion, *_pLayoutVIP;
	Text *_pTxtNumChip, *_pTxtNumGoldCard, *_pTxtNumSilverCard, 
		*_pTxtNumDiamondCard, *_pTxtVipLevel, *_pTxtVipLevel2,
		*_pTxtVipLevelUpPrompt, *_pTextNumVipPercent;
	Button *_pBtnConversionProduct1, *_pBtnConversionProduct2;
	LoadingBar *_pLoadingBarVip;
	ImageView *_pIconDiamondCard;

};