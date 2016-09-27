
#ifndef __THINGSERVER_IPORTAL_H__
#define __THINGSERVER_IPORTAL_H__


#include "IThing.h"

struct IActor;


struct IPortal : public IThing
{
	//被点击
	virtual void OnClicked(IActor * pActor,TSceneID SceneID=INVALID_SCENE_ID) = 0;
};
































#endif
