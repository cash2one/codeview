#include "LayerMessageTip.h"
#include "Settings.h"
#include "cocostudio/CocoStudio.h"
#include "SimpleAudioEngine.h"
#include "comm.h"
#include "SoundControl.h"
using namespace CocosDenshion;

#define text_x_space  30
#define text_y_space  60

#define tp_x_space  40
#define tp_y_space  30

#define btn_space  50

#define top_space 20

// on "init" you need to initialize your instance
bool LayerMessageTip::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}


	/*	载入cocos studio 资源
	*/
	Node* _nodeRoot = CSLoader::createNode("LayerMessageTip.csb");
	if (!_nodeRoot)
		return false;

	this->addChild(_nodeRoot);

	//tipLayer = Layer::create();
	//_nodeRoot->addChild(tipLayer);

	tipLayer = _nodeRoot->getChildByName<Layout*>("tip_layer");
	
	tipBg = tipLayer->getChildByName<ImageView*>("tip_bg");

	btnCancel = tipLayer->getChildByName<Button*>("btn_cancel");
	btnCancel->setTitleText(Language::getStringByKey("Cancel"));

	btnConfirm = tipLayer->getChildByName<Button*>("btn_confirm");
	btnConfirm->setTitleText(Language::getStringByKey("Confirm"));
	/*tipBg = ImageView::create("common/page_bg.png");
	tipBg->setAnchorPoint(ccp(0, 0));
	tipBg->setScale9Enabled(true);
	unsigned int tp_w = 20;
	unsigned int tp_h = 3;
	tipBg->setCapInsets(CCRectMake(tp_w, tp_h, 1489 - tp_w * 2, 760 - tp_h * 2));
	tipLayer->addChild(tipBg);*/


	textBg = ImageView::create("common/bar_1489_760.png");
	textBg->setAnchorPoint(ccp(0.5, 0.5));
	textBg->setScale9Enabled(true);
	textBg->setVisible(false);
	unsigned int tt_w = 0;
	unsigned int tt_h = 0;
	textBg->setCapInsets(CCRectMake(tt_w, tt_h, 1164 - tt_w * 2, 704 - tt_h * 2));
	tipLayer->addChild(textBg);
	//textBg->setPosition(ccp(tp_x_space, tp_x_space+btn_space));

	tipMsg = Text::create();
	tipMsg->setFontName("font/msyhbd.ttf");
	tipMsg->setAnchorPoint(ccp(0.5, 0.5));
	tipMsg->setTextHorizontalAlignment(cocos2d::TextHAlignment::CENTER);
	tipMsg->setFontSize(40);
	//tipMsg->setPosition(ccp(tp_x_space + text_x_space, tp_x_space+text_y_space+btn_space));
	tipLayer->addChild(tipMsg);


	btnCancel->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CLOSE);
		if (_cancelCallback){
			_cancelCallback();
		}
		this->removeFromParent();
	});

	btnConfirm->addClickEventListener([=](Ref *ref){
		SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_CONFIRM);
		if (_confirmCallback){
			_confirmCallback();
		}
		this->removeFromParent();
	});

	return true;
}


void LayerMessageTip::showTip(string tip, std::function<void()> confirmCallback, std::function<void()> cancelCallback,bool isOnlyOk)

{
	_confirmCallback = confirmCallback;
	_cancelCallback = cancelCallback;


	tipMsg->setText(tip);
	Size testSize = tipMsg->getContentSize();
	textBg->setContentSize(CCSizeMake(testSize.width + 2 * text_x_space, testSize.height + 2 * text_y_space));
	this->setContentSize(textBg->getContentSize());

	Size textBgSize = textBg->getContentSize();

	tipBg->setContentSize(CCSizeMake(textBgSize.width + 2 * tp_x_space, textBgSize.height + 2 * tp_y_space + btn_space + top_space));
	
	//this->setContentSize(tipBg->getContentSize());

	Size tipBgSize = tipBg->getContentSize();

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	tipLayer->setPosition(ccp((visibleSize.width - tipBgSize.width) / 2,(visibleSize.height - tipBgSize.height) / 2));

	textBg->setPosition(Vec2(tipBgSize.width / 2, (tipBgSize.height + 80) / 2));
	tipMsg->setPosition(Vec2(tipBgSize.width / 2, (tipBgSize.height+80) / 2));

	btnCancel->setVisible(!isOnlyOk);
	if (isOnlyOk){
		btnConfirm->setPositionX(tipBgSize.width / 2.0);
	}
	else{
		btnCancel->setPositionX(tipBgSize.width / 4.0);
		btnConfirm->setPositionX(tipBgSize.width * 3 / 4.0);
	}

}


MessageTip::MessageTip() :x_space(70), y_space(18)
{
	tipBg = ImageView::create("img/message_tip.png");
	tipBg->setAnchorPoint(ccp(0, 0));
	tipBg->setScale9Enabled(true);
	unsigned int w = 130;
	unsigned int h = 23;
	tipBg->setCapInsets(CCRectMake(w, h, 396 - w * 2, 70 - h * 2));
	this->addChild(tipBg);
	tipMsg = Text::create();
	tipMsg->setFontName("font/msyhbd.ttf");
	tipMsg->setAnchorPoint(ccp(0, 0));
	tipMsg->setTextColor(ccc4(0, 250, 28, 255));
	tipMsg->setFontSize(36);
	tipMsg->setPosition(ccp(x_space, y_space));
	this->addChild(tipMsg);
}

void MessageTip::setTip(std::string tip)
{
	tipMsg->setText(tip);
	Size size = tipMsg->getContentSize();
	tipBg->setContentSize(CCSizeMake(size.width + 2 * x_space, size.height + 2 * y_space));
	this->setContentSize(tipBg->getContentSize());
}