#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIScrollView.h"

USING_NS_CC;
using namespace cocos2d::ui;
enum BJLLZXTYPE{ BJLLZX_DYZ, BJLLZX_XL, BJLLZX_GZ };
class BjlLzSm
{
	// ----------------自定义	
protected:
	Node *_nodeContent;		//根节点,容器节点,用于纹理背景
	Text *_textBanker, *_textPlayer, *_textTie;
	int _countB, _countP, _countT;
	Size _cellSize, _designSize;
	Vector<Sprite*> _cells;
	int _indexRow, _indexCol, _indexColUsed, _curFx;			//当前行索引，当前列索引,已使用列,当前方向
	int _curType;					//当前类型
	void addNewCol();		//添加新列
	int _countCol;
public:
	ScrollView *_scrollViewLzSm;
	bool init(Node* root);
	void addLz(int type);
	void clearLz();		//清空路子
	void setScrollEnable(bool enable);
};
class BjlLzD
{
	// ----------------自定义	
protected:
	Node *_nodeContent;		//根节点,容器节点
	Size _cellSize, _designSize;
	int _initColCount;
	Vector<Sprite*> _cells;
	int _indexRow, _indexCol, _indexColUsed;			//当前行索引，当前列索引,已使用列
	int _curType;					//当前类型	
	void addNewCol();		//添加新列
public:
	std::vector<int>	_lzLengths;		//保存每列长度
	ScrollView *_scrollViewLzSm;
	bool init(Node* root);
	bool addLz(int type);
	void clearLz();
};

//下三路,通用类
class BjlLzX
{
protected:
	Node *_nodeContent;		//根节点,容器节点,用于统一缩放
	ScrollView *_scrollViewLzSm;
	Size _cellSize, _designSize;
	int _initColCount;
	Vector<Sprite*> _cells;
	int _indexRow, _indexCol, _indexColUsed;			//当前行索引，当前列索引,已使用列
	int _curColor;					//当前类型
	char *_colorFileName0, *_colorFileName1;		//color文件名
	void addNewCol();			//添加新列
public:
	void addLz(int color);		//添加新列,指定颜色,0蓝,1红
	bool init(Node* root, BJLLZXTYPE type);		//必须指明下路类型
	void clearLz();
};
//珠盘路
class BjlLzZP
{
	// ----------------自定义	
protected:
	Node *_nodeContent;		//根节点,容器节点
	Size _cellSize, _designSize;
	Vector<Sprite*> _cells;
	int _indexRow, _indexCol;			//当前行索引，当前列索引
	int _initColCount;
	void addNewCol();		//添加新列
public:
	ScrollView *_scrollViewLzSm;
	bool init(Node* root);
	bool addLz(int type);
	void clearLz();
};
class LayerBjlLz :public Layer
{
	// ----------------自定义	
protected:
	Text *_textB, *_textP, *_textT, *_textBP, *_textPP, *_textCount;
	Size _cellSize, _designSize;
	BjlLzD *_bjlLzD;
	BjlLzX *_bjlLzXDYZ, *_bjlLzXXL, *_bjlLzXGZ;
	BjlLzZP *_bjlLzZP;
	ScrollView *_scrollViewLzDYZ, *_scrollViewLzX, *_scrollViewLzGZ, *_scrollViewLzZP;
	Vector<Sprite*> _spNext;
	Node *_root;		//根节点,容器节点
	int GetLzXColor(const std::vector<int> lzLengths, BJLLZXTYPE type);		//根据长度数组及指定类型,获取下路颜色
	int GetNextLzXColor(const std::vector<int> lzLengths, BJLLZXTYPE type);		//根据长度数组及指定类型,获取下一局下路颜色
	void SetNextLzXColor(int color, bool banker, BJLLZXTYPE type);		//根据颜色,庄闲,及类型,设置下一局下路颜色
public:
	int _countB, _countP, _countT, _countBP, _countPP, _count;
	CREATE_FUNC(LayerBjlLz);
	bool init();
	void addLz(int type);
	void clearLz();	//清空路子
};
