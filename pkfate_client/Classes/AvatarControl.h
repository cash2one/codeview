#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;
struct AvatarData
{
	int id;
	//string name[3];
	string path;
};
class AvatarControl :public Ref
{
public:
	static AvatarControl* GetInstance();
	bool InitAvatars();
	map<int, AvatarData*> GetAvatarsData();
	Sprite *GetAvatarSprite(int id);
	Sprite *GetAvatarSprite(int id,int index);
	string GetAvatarSpriteName(int id,int index);
private:
	AvatarControl();
	~AvatarControl();
	map<int, AvatarData*> _avatarsData;
};