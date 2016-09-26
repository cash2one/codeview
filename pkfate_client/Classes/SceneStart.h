#pragma once

#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIButton.h"
#include "ui/UILayout.h"
USING_NS_CC;
using namespace cocos2d::ui;

class SceneStart : public Scene
{
protected:
	Node *RootNode;
	void ImageAsyncCallback(Texture2D *texture);	//回调函数，每加载一张纹理就调用一次
	int currentNum;//当前加载到第几张
	int totalNum;//加载总数
	
	Text *txtInfoTitle,*txtLoading, *txtPct;
	Sprite *imgCheckVer, *imgInformationBluePanel, *imgInformationRedPanel;
	Button *btUpdate, *btUpdateLater;
	void onGetServerInfoCallBack(Ref *pSender);
	void onCheckResourcesCallBack(Ref *ref);
	int getServerInfoCount;
public:
	char _strPct[5];		//更新百分比
	std::string _strFileName;		//更新文件名
	Layout *panelInfo;
	void LoadResources();
	void CheckResources();
	// Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void onEnter() override;
	virtual void onExit() override;
	// implement the "static create()" method manually
	CREATE_FUNC(SceneStart);
};