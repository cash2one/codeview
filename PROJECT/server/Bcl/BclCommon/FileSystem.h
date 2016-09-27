
#ifndef __BCL_FILESYSTEM_H__
#define __BCL_FILESYSTEM_H__


#include "BclHeader.h"
#include <vector>
#include <string>

class BCL_API FileSystem
{
public:
	//获得指定目录下的所有文件名
	static std::vector<std::string>  GetAllFileName(const char* szPath);

};












#endif
