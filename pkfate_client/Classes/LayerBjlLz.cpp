#include "LayerBjlLz.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
#include "bjlcomm.h"

#define BJLLZROWSIZE 6

bool BjlLzSm::init(Node* root)
{
	if (!root)
		return false;
	auto lbPlayer = root->getChildByName<Text*>("lbPlayer");
	if (!lbPlayer)
		return false;
	lbPlayer->setString(Language::getStringByKey("PlayerP"));
	auto lbBanker = root->getChildByName<Text*>("lbBanker");
	if (!lbBanker)
		return false;
	lbBanker->setString(Language::getStringByKey("BankerB"));
	auto lbTie = root->getChildByName<Text*>("lbTie");
	if (!lbTie)
		return false;
	lbTie->setString(Language::getStringByKey("TieT"));
	_textBanker = (Text*)root->getChildByName("Text_Banker");
	if (!_textBanker)
		return false;
	_textPlayer = (Text*)root->getChildByName("Text_Player");
	if (!_textPlayer)
		return false;
	_textTie = (Text*)root->getChildByName("Text_Tie");
	if (!_textTie)
		return false;
	_scrollViewLzSm = (ScrollView*)root->getChildByName("ScrollView_Lzsm");
	if (!_scrollViewLzSm)
		return false;
	Rect rect = _scrollViewLzSm->getBoundingBox();
	//_cellSize.width = rect.size.width / 21;		//计算单个元素的宽度
	_cellSize.height = rect.size.height / BJLLZROWSIZE;		//计算单个元素的高度	
	_cellSize.width = _cellSize.height;
	_countCol = rect.size.width / _cellSize.width;
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("bjl/room/PlistLoad.plist");		//资源缓存
	//auto sprite1 = Sprite::create("bglistlz.png");
	//auto sprite2 = Sprite::createWithSpriteFrameName("db.png");
	//纹理贴图测试
	Sprite *spbg = Sprite::createWithSpriteFrameName("bglistlz.png");
	if (!spbg)
		return false;
	_designSize = spbg->getContentSize();
	float scale = _cellSize.height / _designSize.height;
	_nodeContent = Node::create();
	_nodeContent->setScale(scale);			//统一缩放,后面就不需要再缩放了.
	_scrollViewLzSm->addChild(_nodeContent);
	clearLz();
	return true;
}
void BjlLzSm::clearLz()
{
	//init lz grid
	_nodeContent->removeAllChildren();
	_indexRow = _indexCol = _indexColUsed = _curType = _curFx = 0;
	Vec2 pt0 = Vec2(0, 0);
	for (int i = 0; i < _countCol; i++)
	{
		addNewCol();
	}
	_countB = _countP = _countT = 0;
	_textBanker->setText("");
	_textPlayer->setText("");
	_textTie->setText("");
}
void BjlLzSm::addLz(int type)
{
	//检查类型,必须是庄闲和三种其中一种
	int temptype = type&(BJLBETTYPE::BJLBETP | BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETT);
	if (!(temptype == BJLBETTYPE::BJLBETP || temptype == BJLBETTYPE::BJLBETB || temptype == BJLBETTYPE::BJLBETT))
		return;
	Node* node;
	if (type&BJLBETTYPE::BJLBETT)
	{
		//如果是和,则使用当前位置显示,不用做其他判断
		node = _nodeContent->getChildByTag(_indexCol*BJLLZROWSIZE + _indexRow);
	}
	else
	{
		if (_indexRow == 0 && _indexCol == 0 && _curType == 0)
		{
			//第一局庄或闲
			_curType = type&(BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETP);
		}
		else
		{
			if (_curType&type)
			{

				//与上局类型一样或者第一局,向下
				if (_curFx)
					_indexCol++;
				else
					_indexRow++;
			}
			else
			{
				//与上局类型不同,开新列
				_indexCol = ++_indexColUsed;
				_indexRow = 0;
				_curFx = 0;
				_curType = type&(BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETP);
			}
		}
		int row = _indexRow, col = _indexCol;
		int tag;
		if (_indexRow > 5)
		{
			//向右,不用检查是否有数据,只检查是否超出总数组
			_curFx = 1;
			col = _indexCol + 1;
			row = _indexRow - 1;
			tag = col*BJLLZROWSIZE + row;
			if ((tag + 1) >= _nodeContent->getChildrenCount())
				addNewCol();		//超出最大值,增加新列
			node = _nodeContent->getChildByTag(tag);
		}
		else
		{
			tag = col*BJLLZROWSIZE + row;
			if ((tag + 1) > _nodeContent->getChildrenCount())
				addNewCol();		//超出最大值,增加新列
			node = _nodeContent->getChildByTag(tag);
			if (!node)
				return;
			if (node->getChildrenCount() > 0 && (_indexRow != 0 && _indexCol != 0))
			{
				//已经有数据了,原位向右
				//不用检查是否有数据,只检查是否超出总数组
				_curFx = 1;
				col = _indexCol + 1;
				row = _indexRow - 1;
				tag = col*BJLLZROWSIZE + row;
				if ((tag + 1) >= _nodeContent->getChildrenCount())
					addNewCol();		//超出最大值,增加新列
				node = _nodeContent->getChildByTag(tag);
			}
		}
		_indexCol = col;
		_indexRow = row;
	}
	Vec2 pt0 = Vec2(0, 0);
	if (type&BJLBETTYPE::BJLBETP)
	{
		//闲
		Sprite* sp = Sprite::createWithSpriteFrameName("dp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp, 0, "p");
		_textPlayer->setText(toString(++_countP));
	}
	else if (type&BJLBETTYPE::BJLBETB)
	{
		//庄
		Sprite* sp = Sprite::createWithSpriteFrameName("db.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp, 0, "b");
		_textBanker->setText(toString(++_countB));
	}
	else if (type&BJLBETTYPE::BJLBETT)
	{
		//和
		Sprite* sp = Sprite::createWithSpriteFrameName("dt.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
		_textTie->setText(toString(++_countT));
	}
	if (type&BJLBETTYPE::BJLBETPP)
	{
		//闲对
		Sprite* sp = Sprite::createWithSpriteFrameName("dpp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	if (type&BJLBETTYPE::BJLBETBP)
	{
		//庄对
		Sprite* sp = Sprite::createWithSpriteFrameName("dbp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
}
void BjlLzSm::addNewCol()
{
	Sprite* sp;
	Vec2 pt0 = Vec2(0, 0);
	int size = _nodeContent->getChildrenCount();
	int i = size / BJLLZROWSIZE;
	float x = i*_designSize.width;
	for (int j = (BJLLZROWSIZE - 1); j >= 0; j--)
	{

		sp = Sprite::createWithSpriteFrameName("bglistlz.png");
		sp->setAnchorPoint(pt0);
		sp->setPosition(x, j * _designSize.height);
		_nodeContent->addChild(sp, 0, size++);
	}
	_scrollViewLzSm->setInnerContainerSize(Size((i + 1)*_cellSize.width, BJLLZROWSIZE*_cellSize.height));
	_scrollViewLzSm->scrollToRight(0.5f, false);
}

void BjlLzSm::setScrollEnable(bool enable)
{
	_scrollViewLzSm->setEnabled(enable);
}

bool LayerBjlLz::init()
{
	_root = CSLoader::createNode("bjl/LayerLz.csb");
	if (!_root)
		return false;
	Node* panelInfo = _root->getChildByName("Panel_Info");
	if (!panelInfo)
		return false;
	_textB = (Text*)panelInfo->getChildByName("Text_B");
	if (!_textB)
		return false;
	_textP = (Text*)panelInfo->getChildByName("Text_P");
	if (!_textP)
		return false;
	_textT = (Text*)panelInfo->getChildByName("Text_T");
	if (!_textT)
		return false;
	_textBP = (Text*)panelInfo->getChildByName("Text_BP");
	if (!_textBP)
		return false;
	_textPP = (Text*)panelInfo->getChildByName("Text_PP");
	if (!_textPP)
		return false;
	_textCount = (Text*)panelInfo->getChildByName("Text_Count");
	if (!_textCount)
		return false;
	_bjlLzD = new BjlLzD();
	if (!_bjlLzD)
		return false;
	if (!_bjlLzD->init(_root->getChildByName("ScrollView_D")))
		return false;
	_bjlLzXDYZ = new BjlLzX();
	if (!_bjlLzXDYZ)
		return false;
	if (!_bjlLzXDYZ->init(_root->getChildByName("ScrollView_DYZ"), BJLLZXTYPE::BJLLZX_DYZ))
		return false;
	_bjlLzXXL = new BjlLzX();
	if (!_bjlLzXXL)
		return false;
	if (!_bjlLzXXL->init(_root->getChildByName("ScrollView_XL"), BJLLZXTYPE::BJLLZX_XL))
		return false;
	_bjlLzXGZ = new BjlLzX();
	if (!_bjlLzXGZ)
		return false;
	if (!_bjlLzXGZ->init(_root->getChildByName("ScrollView_GZ"), BJLLZXTYPE::BJLLZX_GZ))
		return false;
	_bjlLzZP = new BjlLzZP();
	if (!_bjlLzZP)
		return false;
	if (!_bjlLzZP->init(_root->getChildByName("ScrollView_ZP")))
		return false;
	Node *node = _root->getChildByName("Panel_NextCon");
	if (!node)
		return false;
	node = node->getChildByName("Panel_Next");
	if (!node)
		return false;
	Rect rect = node->getBoundingBox();			//计算下局投注格子的宽度和高度
	_cellSize.width = rect.size.width / 2;		//计算单个元素的宽度
	_cellSize.height = rect.size.height / 3;		//计算单个元素的高度	
	Sprite *spbg = Sprite::createWithSpriteFrameName("bglistlz.png");
	if (!spbg)
		return false;
	_designSize = spbg->getContentSize();
	float scale = _cellSize.height / _designSize.height;
	node->setScale(scale);
	//init next grid
	Vec2 pt0 = Vec2(0, 0);
	for (int i = 0; i < 6; i++)
	{
		Sprite* sp = Sprite::createWithSpriteFrameName("bglistlz.png");
		sp->setAnchorPoint(pt0);
		sp->setPosition((i % 2) * _designSize.width, (2 - i / 2) * _designSize.height);
		//sp->setScale(scale);
		node->addChild(sp);
		_spNext.pushBack(sp);
	}
	Button *bt = (Button*)_root->getChildByName("Button_Close");
	if (!bt)
		return false;
	bt->addClickEventListener([=](Ref* ref){
		this->setVisible(false);
	});
	bt->setPressedActionEnabled(true);
	clearLz();
	addChild(_root);
	return true;
}
int LayerBjlLz::GetLzXColor(const std::vector<int> lzLengths, BJLLZXTYPE type)
{
	int referCol;
	int lzSize = lzLengths.size();
	switch (type)
	{
	case BJLLZX_DYZ:
		if (lzSize > 2 || (lzSize == 2 && lzLengths.back() > 1))
			referCol = 1;
		else
			return -1;
		break;
	case BJLLZX_XL:
		if (lzSize > 3 || (lzSize == 3 && lzLengths.back() > 1))
			referCol = 2;
		else
			return -1;
		break;
	case BJLLZX_GZ:
		if (lzSize > 4 || (lzSize == 4 && lzLengths.back() > 1))
			referCol = 3;
		else
			return -1;
		break;
	default:
		return -1;
	}
	int lastIndex = lzSize - 1;
	if (lzLengths[lastIndex] == 1)
		return lzLengths[lastIndex - 1] == lzLengths[lastIndex - 1 - referCol] ? 1 : 0;
	else
	{
		if (lzLengths[lastIndex - referCol] >= lzLengths[lastIndex])
			return 1;
		else if (lzLengths[lastIndex] - lzLengths[lastIndex - referCol] == 1)
			return 0;
		else
			return 1;
	}
}
int LayerBjlLz::GetNextLzXColor(const std::vector<int> lzLengths, BJLLZXTYPE type)
{
	int referCol;
	int lzSize = lzLengths.size();
	switch (type)
	{
	case BJLLZX_DYZ:
		if (lzSize >= 2)
			referCol = 1;
		else
			return -1;
		break;
	case BJLLZX_XL:
		if (lzSize >= 3)
			referCol = 2;
		else
			return -1;
		break;
	case BJLLZX_GZ:
		if (lzSize >= 4)
			referCol = 3;
		else
			return -1;
		break;
	default:
		return -1;
	}
	int lastIndex = lzSize - 1;
	return lzLengths[lastIndex] == lzLengths[lastIndex - referCol] ? 0 : 1;
}
void LayerBjlLz::SetNextLzXColor(int color, bool banker, BJLLZXTYPE type)
{
	if (color < 0)
		return;
	Sprite *spB, *spP, *spRed, *spBlue;
	switch (type)
	{
	case BJLLZX_DYZ:
		spB = _spNext.at(0);
		spP = _spNext.at(1);
		spRed = Sprite::createWithSpriteFrameName("db.png");
		spBlue = Sprite::createWithSpriteFrameName("dp.png");
		break;
	case BJLLZX_XL:
		spB = _spNext.at(2);
		spP = _spNext.at(3);
		spRed = Sprite::createWithSpriteFrameName("xb.png");
		spBlue = Sprite::createWithSpriteFrameName("xp.png");
		break;
	case BJLLZX_GZ:
		spB = _spNext.at(4);
		spP = _spNext.at(5);
		spRed = Sprite::createWithSpriteFrameName("gzb.png");
		spBlue = Sprite::createWithSpriteFrameName("gzp.png");
		break;
	default:
		return;
	}
	if (!spB || !spP || !spRed || !spBlue)
		return;
	static Vec2 pt0 = Vec2(0, 0);
	spB->removeAllChildren();
	spP->removeAllChildren();
	spRed->setAnchorPoint(pt0);
	spBlue->setAnchorPoint(pt0);
	if (banker)
	{
		//banker
		if (color)
		{
			spB->addChild(spRed);
			spP->addChild(spBlue);
		}
		else
		{
			spB->addChild(spBlue);
			spP->addChild(spRed);
		}
	}
	else
	{
		//player
		if (color)
		{
			spB->addChild(spBlue);
			spP->addChild(spRed);
		}
		else
		{
			spB->addChild(spRed);
			spP->addChild(spBlue);
		}
	}
}
void LayerBjlLz::addLz(int type)
{
	//检查类型,必须是庄闲和三种其中一种
	int temptype = type&(BJLBETTYPE::BJLBETP | BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETT);
	if (!(temptype == BJLBETTYPE::BJLBETP || temptype == BJLBETTYPE::BJLBETB || temptype == BJLBETTYPE::BJLBETT))
		return;
	//设置大路
	if (!_bjlLzD->addLz(type))
		return;
	//设置珠盘路
	if (!_bjlLzZP->addLz(type))
		return;
	//设置下路
	if (!(type&BJLBETTYPE::BJLBETT))
	{
		Vec2 pt0 = Vec2(0, 0);
		//不为和时,计算下一局下路信息,和本局下路
		int color = GetNextLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_DYZ);
		if (color >= 0)
			SetNextLzXColor(color, type&BJLBETTYPE::BJLBETB, BJLLZXTYPE::BJLLZX_DYZ);
		color = GetLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_DYZ);
		if (color >= 0)
			_bjlLzXDYZ->addLz(color);
		//XL
		color = GetNextLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_XL);
		if (color >= 0)
			SetNextLzXColor(color, type&BJLBETTYPE::BJLBETB, BJLLZXTYPE::BJLLZX_XL);
		color = GetLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_XL);
		if (color >= 0)
			_bjlLzXXL->addLz(color);
		//GZ
		color = GetNextLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_GZ);
		if (color >= 0)
			SetNextLzXColor(color, type&BJLBETTYPE::BJLBETB, BJLLZXTYPE::BJLLZX_GZ);
		color = GetLzXColor(_bjlLzD->_lzLengths, BJLLZXTYPE::BJLLZX_GZ);
		if (color >= 0)
			_bjlLzXGZ->addLz(color);
	}
	//设置统计信息
	if (type&BJLBETTYPE::BJLBETP)
	{
		//闲
		_textP->setText(toString(++_countP));
	}
	else if (type&BJLBETTYPE::BJLBETB)
	{
		//庄
		_textB->setText(toString(++_countB));
	}
	else if (type&BJLBETTYPE::BJLBETT)
	{
		//和
		_textT->setText(toString(++_countT));
	}
	if (type&BJLBETTYPE::BJLBETPP)
	{
		//闲对
		_textPP->setText(toString(++_countPP));
	}
	if (type&BJLBETTYPE::BJLBETBP)
	{
		//庄对
		_textBP->setText(toString(++_countBP));
	}
	_textCount->setText(toString(++_count));
}
void LayerBjlLz::clearLz()
{
	//清空数据显示区
	_countB = _countP = _countT = _countBP = _countPP = _count = 0;
	_textB->setText("");
	_textP->setText("");
	_textT->setText("");
	_textBP->setText("");
	_textPP->setText("");
	_textCount->setText("");
	//清空下局路单提示区
	for (auto sp : _spNext)
	{
		sp->removeAllChildren();
	}
	//清空所有路子区
	_bjlLzD->clearLz();
	_bjlLzXXL->clearLz();
	_bjlLzXDYZ->clearLz();
	_bjlLzXGZ->clearLz();
	_bjlLzZP->clearLz();
}
// ***************************************** bjllzx class
bool BjlLzX::init(Node* root, BJLLZXTYPE type)
{
	if (!root)
		return false;
	_scrollViewLzSm = (ScrollView*)root;
	Rect rect = _scrollViewLzSm->getBoundingBox();
	_cellSize.height = rect.size.height / BJLLZROWSIZE;		//计算单个元素的高度	
	_cellSize.width = _cellSize.height;


	Sprite *spbg = Sprite::createWithSpriteFrameName("bglistlz.png");
	if (!spbg)
		return false;
	_designSize = spbg->getContentSize();
	float scale = _cellSize.height / _designSize.height;
	_nodeContent = Node::create();
	_nodeContent->setScale(scale);			//统一缩放,后面就不需要再缩放了.
	_scrollViewLzSm->addChild(_nodeContent);
	switch (type)
	{
	case BJLLZX_DYZ:
		_colorFileName0 = "dp.png";
		_colorFileName1 = "db.png";
		break;
	case BJLLZX_XL:
		_colorFileName0 = "xp.png";
		_colorFileName1 = "xb.png";
		break;
	case BJLLZX_GZ:
		_colorFileName0 = "gzp.png";
		_colorFileName1 = "gzb.png";
		break;
	default:
		return false;
	}
	//计算初始元素个数
	_initColCount = rect.size.width / _cellSize.width;
	clearLz();
	return true;
}
void BjlLzX::clearLz()
{
	_indexRow = _indexCol = _indexColUsed = 0;
	_curColor = -1;
	_nodeContent->removeAllChildren();
	for (int i = 0; i < _initColCount; i++)
	{
		addNewCol();
	}
}
void BjlLzX::addLz(int color)
{
	Node* node;
	if (_indexRow == 0 && _indexCol == 0 && _curColor == -1)
	{
		//第一局庄或闲
		_curColor = color;
	}
	else
	{
		if (_curColor == color)
		{
			//与上局类型一样或者第一局,向下
			_indexRow++;
		}
		else
		{
			//与上局类型不同,开新列
			_indexCol = ++_indexColUsed;
			_indexRow = 0;
			_curColor = color;
		}
	}
	int row = _indexRow, col = _indexCol;
	int tag;
	if (_indexRow > 5)
	{
		//向右,不用检查是否有数据,只检查是否超出总数组
		col = _indexCol + 1;
		row = _indexRow - 1;
		tag = col*BJLLZROWSIZE + row;
		if ((tag + 1) >= _nodeContent->getChildrenCount())
			addNewCol();		//超出最大值,增加新列
		node = _nodeContent->getChildByTag(tag);
	}
	else
	{
		tag = col*BJLLZROWSIZE + row;
		if ((tag + 1) > _nodeContent->getChildrenCount())
			addNewCol();		//超出最大值,增加新列
		node = _nodeContent->getChildByTag(tag);
		if (!node)
			return;
		if (node->getChildrenCount() > 0)
		{
			//已经有数据了,原位向右
			//不用检查是否有数据,只检查是否超出总数组
			col = _indexCol + 1;
			row = _indexRow - 1;
			tag = col*BJLLZROWSIZE + row;
			if ((tag + 1) >= _nodeContent->getChildrenCount())
				addNewCol();		//超出最大值,增加新列
			node = _nodeContent->getChildByTag(tag);
		}
	}
	_indexCol = col;
	_indexRow = row;
	Sprite* sp = Sprite::createWithSpriteFrameName(color ? _colorFileName1 : _colorFileName0);
	sp->setAnchorPoint(Vec2(0, 0));
	node->addChild(sp);
}
void BjlLzX::addNewCol()
{
	Sprite* sp;
	Vec2 pt0 = Vec2(0, 0);
	int size = _nodeContent->getChildrenCount();
	int i = size / BJLLZROWSIZE;
	float x = i*_designSize.width;
	for (int j = (BJLLZROWSIZE - 1); j >= 0; j--)
	{

		sp = Sprite::createWithSpriteFrameName("bglistlz.png");
		sp->setAnchorPoint(pt0);
		sp->setPosition(x, j * _designSize.height);
		_nodeContent->addChild(sp, 0, size++);
	}
	_scrollViewLzSm->setInnerContainerSize(Size((i + 1)*_cellSize.width, BJLLZROWSIZE*_cellSize.height));
	_scrollViewLzSm->scrollToRight(0.5f, false);
}

// **************************** bjllzd class
bool BjlLzD::init(Node* root)
{
	if (!root)
		return false;
	_scrollViewLzSm = (ScrollView*)root;
	Rect rect = _scrollViewLzSm->getBoundingBox();
	_cellSize.height = rect.size.height / BJLLZROWSIZE;		//计算单个元素的高度	
	_cellSize.width = _cellSize.height;
	
	Size innerSize=_scrollViewLzSm->getInnerContainerSize();
	Size size=_scrollViewLzSm->getContentSize();


	Sprite *spbg = Sprite::createWithSpriteFrameName("bglistlz.png");
	if (!spbg)
		return false;
	_designSize = spbg->getContentSize();
	float scale = _cellSize.height / _designSize.height;
	_nodeContent = Node::create();
	_nodeContent->setScale(scale);			//统一缩放,后面就不需要再缩放了.
	_scrollViewLzSm->addChild(_nodeContent);
	//计算初始元素个数
	_initColCount = rect.size.width / _cellSize.width;
	clearLz();
	return true;
}
void BjlLzD::clearLz()
{
	//init lz grid
	_indexRow = _indexCol = _indexColUsed = _curType = 0;
	_nodeContent->removeAllChildren();
	for (int i = 0; i < _initColCount;i++)
	{
		addNewCol();
	}
}
bool BjlLzD::addLz(int type)
{
	//检查类型,必须是庄闲和三种其中一种
	int temptype = type&(BJLBETTYPE::BJLBETP | BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETT);
	if (!(temptype == BJLBETTYPE::BJLBETP || temptype == BJLBETTYPE::BJLBETB || temptype == BJLBETTYPE::BJLBETT))
		return false;
	Node* node;
	if (type&BJLBETTYPE::BJLBETT)
	{
		//如果是和,则使用当前位置显示,不用做其他判断
		node = _nodeContent->getChildByTag(_indexCol*BJLLZROWSIZE + _indexRow);
	}
	else
	{
		if (_indexRow == 0 && _indexCol == 0 && _curType == 0)
		{
			//第一局庄或闲
			_curType = type&(BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETP);
			_lzLengths.push_back(1);
		}
		else
		{
			if (_curType&type)
			{
				//与上局类型一样或者第一局,向下
				_lzLengths.back()++;
				_indexRow++;
			}
			else
			{
				//与上局类型不同,开新列
				_indexCol = ++_indexColUsed;
				_lzLengths.push_back(1);
				_indexRow = 0;
				_curType = type&(BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETP);
			}
		}
		int row = _indexRow, col = _indexCol;
		int tag;
		if (_indexRow > 5)
		{
			//向右,不用检查是否有数据,只检查是否超出总数组
			col = _indexCol + 1;
			row = _indexRow - 1;
			tag = col*BJLLZROWSIZE + row;
			if ((tag + 1) >= _nodeContent->getChildrenCount())
				addNewCol();		//超出最大值,增加新列
			node = _nodeContent->getChildByTag(tag);
		}
		else
		{
			tag = col*BJLLZROWSIZE + row;
			if ((tag + 1) > _nodeContent->getChildrenCount())
				addNewCol();		//超出最大值,增加新列
			node = _nodeContent->getChildByTag(tag);
			if (!node)
				return false;
			if (node->getChildrenCount() > 0 && (_indexRow != 0 && _indexCol != 0))
			{
				//已经有数据了,原位向右
				//不用检查是否有数据,只检查是否超出总数组
				col = _indexCol + 1;
				row = _indexRow - 1;
				tag = col*BJLLZROWSIZE + row;
				if ((tag + 1) >= _nodeContent->getChildrenCount())
					addNewCol();		//超出最大值,增加新列
				node = _nodeContent->getChildByTag(tag);
			}
		}
		_indexCol = col;
		_indexRow = row;
	}
	Vec2 pt0 = Vec2(0, 0);
	if (type&BJLBETTYPE::BJLBETP)
	{
		//闲
		Sprite* sp = Sprite::createWithSpriteFrameName("dp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp, 0, "p");
	}
	else if (type&BJLBETTYPE::BJLBETB)
	{
		//庄
		Sprite* sp = Sprite::createWithSpriteFrameName("db.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp, 0, "b");
	}
	else if (type&BJLBETTYPE::BJLBETT)
	{
		//和
		Sprite* sp = Sprite::createWithSpriteFrameName("dt.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	if (type&BJLBETTYPE::BJLBETPP)
	{
		//闲对
		Sprite* sp = Sprite::createWithSpriteFrameName("dpp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	if (type&BJLBETTYPE::BJLBETBP)
	{
		//庄对
		Sprite* sp = Sprite::createWithSpriteFrameName("dbp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	return true;
}
void BjlLzD::addNewCol()
{
	Sprite* sp;
	Vec2 pt0 = Vec2(0, 0);
	int size = _nodeContent->getChildrenCount();
	int i = size / BJLLZROWSIZE;
	float x = i*_designSize.width;
	for (int j = (BJLLZROWSIZE - 1); j >= 0; j--)
	{

		sp = Sprite::createWithSpriteFrameName("bglistlz.png");
		sp->setAnchorPoint(pt0);
		sp->setPosition(x, j * _designSize.height);
		_nodeContent->addChild(sp, 0, size++);
	}
	_scrollViewLzSm->setInnerContainerSize(Size((i + 1)*_cellSize.width, BJLLZROWSIZE*_cellSize.height));
	_scrollViewLzSm->scrollToRight(0.5f, false);
}

// **************************** bjllzZP class
bool BjlLzZP::init(Node* root)
{
	if (!root)
		return false;
	_scrollViewLzSm = (ScrollView*)root;
	Rect rect = _scrollViewLzSm->getBoundingBox();
	_cellSize.height = rect.size.height / BJLLZROWSIZE;		//计算单个元素的高度	
	_cellSize.width = _cellSize.height;


	Sprite *spbg = Sprite::createWithSpriteFrameName("bglistlz.png");
	if (!spbg)
		return false;
	_designSize = spbg->getContentSize();
	float scale = _cellSize.height / _designSize.height;
	_nodeContent = Node::create();
	_nodeContent->setScale(scale);			//统一缩放,后面就不需要再缩放了.
	_scrollViewLzSm->addChild(_nodeContent);
	
	//计算初始元素个数
	_initColCount = rect.size.width / _cellSize.width;
	clearLz();
	return true;
}
void BjlLzZP::clearLz()
{
	_indexRow = _indexCol = 0;
	_nodeContent->removeAllChildren();
	for (int i = 0; i < _initColCount;i++)
	{
		addNewCol();
	}
}
bool BjlLzZP::addLz(int type)
{
	//检查类型,必须是庄闲和三种其中一种
	int temptype = type&(BJLBETTYPE::BJLBETP | BJLBETTYPE::BJLBETB | BJLBETTYPE::BJLBETT);
	if (!(temptype == BJLBETTYPE::BJLBETP || temptype == BJLBETTYPE::BJLBETB || temptype == BJLBETTYPE::BJLBETT))
		return false;
	Node* node;
	if (_indexRow > 5)
	{
		_indexCol++;
		_indexRow = 0;
		if ((_indexCol*BJLLZROWSIZE + _indexRow + 1) >= _nodeContent->getChildrenCount())
			addNewCol();		//超出最大值,增加新列
	}
	node = _nodeContent->getChildByTag(_indexCol*BJLLZROWSIZE + _indexRow++);
	Vec2 pt0 = Vec2(0, 0);
	if (type&BJLBETTYPE::BJLBETP)
	{
		//闲
		Sprite* sp = Sprite::createWithSpriteFrameName("zpp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	else if (type&BJLBETTYPE::BJLBETB)
	{
		//庄
		Sprite* sp = Sprite::createWithSpriteFrameName("zpb.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	else if (type&BJLBETTYPE::BJLBETT)
	{
		//和
		Sprite* sp = Sprite::createWithSpriteFrameName("zpt.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	if (type&BJLBETTYPE::BJLBETPP)
	{
		//闲对
		Sprite* sp = Sprite::createWithSpriteFrameName("dpp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	if (type&BJLBETTYPE::BJLBETBP)
	{
		//庄对
		Sprite* sp = Sprite::createWithSpriteFrameName("dbp.png");
		sp->setAnchorPoint(pt0);
		node->addChild(sp);
	}
	return true;
}
void BjlLzZP::addNewCol()
{
	Sprite* sp;
	Vec2 pt0 = Vec2(0, 0);
	int size = _nodeContent->getChildrenCount();
	int i = size / BJLLZROWSIZE;
	float x = i*_designSize.width;
	for (int j = (BJLLZROWSIZE - 1); j >= 0; j--)
	{

		sp = Sprite::createWithSpriteFrameName("bglistlz.png");
		sp->setAnchorPoint(pt0);
		sp->setPosition(x, j * _designSize.height);
		_nodeContent->addChild(sp, 0, size++);
	}
	_scrollViewLzSm->setInnerContainerSize(Size((i + 1)*_cellSize.width, BJLLZROWSIZE*_cellSize.height));
	_scrollViewLzSm->scrollToRight(0.5f, false);
}