#pragma once
#include "cocos2d.h"
USING_NS_CC;
using namespace std;

class IrregularTest
{
public:
	IrregularTest();
	~IrregularTest();
	bool AddSprite(Sprite *sp, string filename);
	bool HitTest(Sprite *sp, const Vec2 &pt);
protected:
	struct IrregularData
	{
		int width, height;
		bool *boolDatas;
	};
	map<Sprite*, IrregularData*> _irregularDatas;
	bool getIsTransparentAtPoint(cocos2d::Vec2 point, IrregularData* irregularData);
};