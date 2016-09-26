/*******************************
//  Created by Vinci on 2016.6.19
*********************************/

#include "LayerDuang.h"
#include "SceneGames.h"
#include "LayerDzGames.h"
#include "LayerBjlGames.h"

void LayerDuang::uiBorderAppear(Layout *pLayout)
{
	Vec2 tempVec2 = pLayout->getPosition();
	Size tempSize = pLayout->getSize();
	
	Size phoneScreenSize = Director::getInstance()->getVisibleSize();
	
	//The pLayout in the above?
	if (tempVec2.x < phoneScreenSize.width/2 && tempVec2.y > phoneScreenSize.height/2)
	{
		//Move from down to up
		pLayout->setPosition(Vec2(tempVec2.x, tempVec2.y + tempSize.height));
		pLayout->runAction(MoveBy::create(0.3, Vec2(0, -tempSize.height)));
	}
	//The pLayout in the left?
	else if (tempVec2.x < phoneScreenSize.width / 2 && tempVec2.y < phoneScreenSize.height / 2)
	{
		//Move from left to right
		pLayout->setPosition(Vec2(tempVec2.x - tempSize.width, tempVec2.y));
		pLayout->runAction(MoveBy::create(0.3, Vec2(tempSize.width, 0)));
	}
};

void LayerDuang::panelStatus()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			Layout* layoutTemp = (Layout*)pSceneGames->getRootNode()->getChildByName("panelStatus");
			if (layoutTemp)
				uiBorderAppear(layoutTemp);
		}
	}
};

void LayerDuang::uiGameMenu()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			Vector<Widget*> vecTemp = pSceneGames->getListGames()->getItems();
			uiLargenAppear(vecTemp);
		}
	}
};

void LayerDuang::uiLargenAppear(Vector<Widget*> vecTemp)
{
	for (vector<Widget*>::iterator it = vecTemp.begin(); it != vecTemp.end(); ++it)
	{
		uiLargenAppear(*it);
	}
};

void LayerDuang::uiLargenAppear(Widget* pWidget)
{
	pWidget->setAnchorPoint(Vec2(0.5f, 0.5f));
	pWidget->setScale(0.8f);
	auto actionBy = ScaleTo::create(0.3f, 1.0f);
	pWidget->runAction(actionBy);
};

void LayerDuang::uiBorderAppear(const std::string& name)
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		Layout *pLayout = dynamic_cast<Layout*>(runningScene->getChildByName(name));
		if (pLayout != NULL)
		{
			uiBorderAppear(pLayout);
		}
	}
}

void LayerDuang::dzGamesRoom()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			LayerDzGames *pLayerDzGames = dynamic_cast<LayerDzGames*>(pSceneGames->getCurrentPanel());
			if (pLayerDzGames)
			{
				uiLargenAppear((Widget*)pLayerDzGames->getMaster());
				uiLargenAppear((Widget*)pLayerDzGames->getElite());
				uiLargenAppear((Widget*)pLayerDzGames->getRoyal());
				uiLargenAppear((Widget*)pLayerDzGames->getNormal());
			}
		}
	}
}

void  LayerDuang::intoDzGamesRoom()
{
	panelStatus();
	dzGamesRoom();
	dzPanelLobby();
}

void  LayerDuang::bjlGamesRoom()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			LayerBjlGames *pLayerBjlGames = dynamic_cast<LayerBjlGames*>(pSceneGames->getCurrentPanel());
			if (pLayerBjlGames)
			{
				Vector<Widget*> vecTemp = pLayerBjlGames->getListRooms()->getItems();
				uiLargenAppear(vecTemp);
			}
		}
	}
}

void LayerDuang::dzPanelLobby()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			LayerDzGames *pLayerDzGames = dynamic_cast<LayerDzGames*>(pSceneGames->getCurrentPanel());
			if (pLayerDzGames)
			{
				uiBorderAppear((Layout*)pLayerDzGames->getPanelLobby());
			}
		}
	}
}

void LayerDuang::bjlPanelLobby()
{
	auto runningScene = Director::getInstance()->getRunningScene();
	if (runningScene)
	{
		SceneGames *pSceneGames = dynamic_cast<SceneGames*>(runningScene);
		if (pSceneGames != NULL)
		{
			LayerBjlGames *pLayerBjlGames = dynamic_cast<LayerBjlGames*>(pSceneGames->getCurrentPanel());
			if (pLayerBjlGames)
			{
				uiBorderAppear((Layout*)pLayerBjlGames->getPanelLobby());
			}
		}
	}
}