#include "IrregularTest.h"
IrregularTest::IrregularTest()
{
}
IrregularTest::~IrregularTest()
{
	for (auto irregularData : _irregularDatas)
	{
		if (irregularData.second)
		{
			if (irregularData.second->boolDatas)
				delete[] irregularData.second->boolDatas;
			delete irregularData.second;
		}
	}
}
//添加一个新的不规则sprite
bool IrregularTest::AddSprite(Sprite *sp, string filename)
{
	if (!sp || filename.empty())
		return false;
	
	Image* normalImage = new Image();
	if (!normalImage->initWithImageFile(filename))
	{
		delete normalImage;
		return false;
	}
	

	auto dataLen = normalImage->getDataLen();
	auto irregularData = _irregularDatas[sp];
	if (irregularData == nullptr)
	{
		irregularData = new IrregularData();
		_irregularDatas[sp] = irregularData;
	}
	else if (irregularData->boolDatas)
	{
		delete[] irregularData->boolDatas;
	}
	irregularData->width = normalImage->getWidth();
	irregularData->height = normalImage->getHeight();
	irregularData->boolDatas = new bool[dataLen / (sizeof(unsigned char) * 4)];
	auto normalPixels = normalImage->getData();
	for (auto i = 0; i < irregularData->height; i++) {
		for (auto j = 0; j < irregularData->width; j++) {
			irregularData->boolDatas[i * irregularData->width + j] = (normalPixels[(i * irregularData->width + j) * 4] == 0);
		}
	}
	delete normalImage;
	return true;
}
bool IrregularTest::getIsTransparentAtPoint(Vec2 point, IrregularData* irregularData)
{
	if (!irregularData)
		return true;
	point.y = irregularData->height - point.y;
	int x = (int)point.x - 1;
	if (x < 0) {
		x = 0;
	}
	else if (x >= irregularData->width) {
		x = irregularData->width - 1;
	}
	int y = (int)point.y - 1;
	if (y < 0) {
		y = 0;
	}
	else if (y >= irregularData->height) {
		y = irregularData->height - 1;
	}
	return irregularData->boolDatas[irregularData->width * y + x];
}
bool IrregularTest::HitTest(Sprite *sp, const Vec2 &pt)
{
	Vec2 localLocation = sp->convertToNodeSpace(pt);
	Rect validTouchedRect;
	validTouchedRect.size = sp->getContentSize();
	if (validTouchedRect.containsPoint(localLocation) && getIsTransparentAtPoint(localLocation, _irregularDatas[sp]) == false)
	{
		return true;
	}
	return false;
}