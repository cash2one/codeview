#include "LayerBjlResult.h"
#include "cocostudio/CocoStudio.h"
#include "comm.h"
bool LayerBjlResult::init()
{
	if (!Layer::init())
		return false;
	Node *root = CSLoader::createNode("bjl/LayerResult.csb");
	if (!root)
		return false;
	_textResultWin = (Text*)root->getChildByName("Text_Win");
	_textResultLose = (Text*)root->getChildByName("Text_Lose");
	_textResultTotal = (Text*)root->getChildByName("Text_Total");
	if (!_textResultWin || !_textResultLose || !_textResultTotal)
		return false;
	resetAmount();
	addChild(root);
	return true;
}
void LayerBjlResult::addAmount(float amount)
{
	if (amount > 0)
	{
		_amountWin += amount;
		_textResultWin->setText(toString(_amountWin));
		_textResultTotal->setText(toString(_amountWin + _amountLose));
	}
	else if (amount < 0)
	{
		_amountLose += amount;
		_textResultLose->setText(toString(_amountLose));
		_textResultTotal->setText(toString(_amountWin + _amountLose));
	}
}
void LayerBjlResult::resetAmount()
{
	_amountWin = _amountLose = 0.0f;
	_textResultWin->setText(toString(_amountWin));
	_textResultLose->setText(toString(_amountLose));
	_textResultTotal->setText(toString(_amountWin + _amountLose));
}
float LayerBjlResult::getAmount()
{
	return _amountWin + _amountLose;
}