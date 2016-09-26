/*******************************
// Created by vinci on 2016.6.19
*********************************/
#pragma once

#include "cocos2d.h"
#include "ui/UILayout.h"
USING_NS_CC;
using namespace cocos2d::ui;

class LayerDuang
{
public:
	
	//UI from edge popup animation effects
	static void uiBorderAppear(Layout *pLayout);
	static void uiBorderAppear(const std::string& name);

	//Game menu larger animation
	static void uiGameMenu();

	//Widget larger animation
	static void uiLargenAppear(Vector<Widget*> vecTemp);
	static void uiLargenAppear(Widget* pWidget);

	//The game hall at the top of the animation
	static void panelStatus();

	//Texas lobby animation 
	//To call after LayerBjlGames.create
	static void dzPanelLobby();

	//Baccarat lobby animation
	static void bjlPanelLobby();

	//Game room larger animation
	static void dzGamesRoom();

	//Texas holdem room button animation
	static void  intoDzGamesRoom();

	//Baccarat Game room lobby animation
	static void bjlGamesRoom();
};