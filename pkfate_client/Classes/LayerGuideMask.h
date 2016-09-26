#pragma once

#include "cocos2d.h"
#include "GuideControl.h"
#include "ui/UIText.h"
#include "ui/UIImageView.h"
#include "ui/UILayout.h"
USING_NS_CC;
using namespace cocos2d::ui;


class GuideHandler{
public:
	virtual void addGuide() =0 ;
};

enum DIRECTION
{
	LEFT = 1,
	LEFT_TOP = 2,
	TOP = 3,
	TOP_RIGHT = 4,
	RIGHT = 5,
	RIGHT_BOTTON = 6,
	BOTTOM = 7,
	BOTTOM_LEFT = 8
};

class GuideMessage :public Node
{
private:
	const unsigned short x_space, y_space;
	ImageView *tipBg;
	Text* tipMsg;
	Layout* tipLayoutBg;
public:
	void setTip(std::string tip);
	void setTipColor(const Color4B color);
	Layout* getTipLayoutBg();
	void setTipContentSize(const cocos2d::Size &contentSize);
	GuideMessage();
	CREATE_FUNC(GuideMessage);
};

class LayerGuideMask : public Layer
{
private :
	float video_x;
	float video_y;
	ClippingNode* clip;
	Layer* _maskLayer;
	EventListenerTouchOneByOne* listener;
	int idSound;
	Rect _waitRect;
protected:
	GuideMessage* _textSay;
	TechData* _clickData;
	std::function<void()>  clickHandler;

	void checkClick(Vec2 p);
	
	void wait();

	void handlerWaitTouch();
    
    void playKavayiVideo(Vec2 pos);
public:
	void clickAction(TechData* data);
	void sayAction(TechData* data);
	void soundAction(TechData* data);
	void videoAction(TechData* data);

	void playTechAction(int step, Node* content =nullptr);

	void techAction(std::vector<TechData*> list);

	void setSwallowTouches(bool is);

	void cleanTech();

	void cleanMask();

	void cleanVideo();

	void close();

	void addTo(Node* node);

	Sprite* addArrow(DIRECTION direction, Vec2 p);

	void removeFromMask(string name);

	bool validateStep(int step);

	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();

	void setClickHandler(std::function<void()>  clickHandler);

	static LayerGuideMask* GetInstance();

	GuideMessage *getTextSay();

	Layer* getMaskLayer();

	void textArrow(const std::string& text, const Vec2 &position, bool isLeft, DIRECTION dir);
	
	// implement the "static create()" method manually
	//touch事件监听 屏蔽向下触摸
	//bool onTouchBegan(Touch *touch, Event *event);
    LayerGuideMask();
	CREATE_FUNC(LayerGuideMask);

};
