
#ifndef __BCL_IKEYWORD_FILTER_H__
#define __BCL_IKEYWORD_FILTER_H__

#include "BclHeader.h"
#include <string>

struct IKeywordFilter
{

	//设置关键词库
	virtual void AddKeyword(const char * szKey) = 0;

		//删除关键词szKey
	virtual void DeleteKeyword(const char * szKey) = 0;

		//删除所有关键词szKey
	virtual void ClearKeyword() = 0;

		//过滤输入的字符串，非法字符串会使用*代替，返回true表示有字符被替换
	virtual bool    Filter(char * szStr)=0;
	//判断是否不包含非法关键字
    virtual bool    IsLegalString(const std::string & str) = 0;

	
	  //判断是否是合法的名字,必须用unicode判断
    virtual bool    IsValidName(const std::string & name) = 0;
};




#endif
