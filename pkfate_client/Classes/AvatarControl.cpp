#include "AvatarControl.h"

#include "comm.h"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32) && (CC_TARGET_PLATFORM != CC_PLATFORM_WP8) && (CC_TARGET_PLATFORM != CC_PLATFORM_WINRT)
#include <dirent.h>
#include <unistd.h>
#else
#include <io.h>
#endif

#define AVATARPATH "img/avatars/"
static AvatarControl *m_pInstance = nullptr;
AvatarControl::AvatarControl()
{
}
AvatarControl::~AvatarControl()
{
	if (_avatarsData.size()>0)
	{
		for (auto avatar : _avatarsData)
		{
			delete avatar.second;
		}
	}
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}
AvatarControl* AvatarControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new AvatarControl();
	return m_pInstance;
}
//初始化头像资源，只与目录下的文件相关
bool AvatarControl::InitAvatars()
{
	/*
	//string path = FileUtils::getInstance()->getWritablePath() + AVATARPATH;
	if (!FileUtils::getInstance()->isDirectoryExist(AVATARPATH))
		return false;
	string path = FileUtils::getInstance()->fullPathForFilename(AVATARPATH);
	std::vector<std::string> path_vec;
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32) && (CC_TARGET_PLATFORM != CC_PLATFORM_WP8) && (CC_TARGET_PLATFORM != CC_PLATFORM_WINRT)
	
	DIR *dp;
	dp = opendir(path.c_str());
	if (dp)
	{
		struct dirent *entry;
		while ((entry = readdir(dp)) != NULL)
		{
			if(entry->d_type&DT_DIR)
				continue;
			path_vec.push_back(StringUtils::format("%s", entry->d_name));
		}
		closedir(dp);
	}
#else
	string desc = path + "*.plist";
	long handle;												//用于查找的句柄
	struct _finddata_t fileinfo;						  //文件信息的结构体
	handle = _findfirst(desc.c_str(), &fileinfo);		 //第一次查找
	if (handle >= 0)
	{
		do
		{
			if (fileinfo.attrib & _A_SUBDIR)
				continue;
			path_vec.push_back(StringUtils::format("%s", fileinfo.name));
		} while (!_findnext(handle, &fileinfo));			   //循环查找其他符合的文件，知道找不到其他的为止
		_findclose(handle);
	}
#endif
	if (path_vec.size() <= 0)
		return false;*/
	std::vector<std::string> path_vec;
	char strfilename[16];
	for (int i = 1; i <= 26; i++)
	{
		sprintf(strfilename, "avatar%d.plist", i);
		path_vec.push_back(strfilename);
	}
	for (auto filename : path_vec)
	{
		//CCLOG("filename:%s", filename.c_str());
		//前6个字母为 avatar,中间是id,后6个字母为.plist
		if (filename.size() <= 12 
			|| filename.substr(0, 6).compare("avatar") != 0 
			|| filename.substr(filename.size() - 6, 6).compare(".plist") != 0)
			continue;
		AvatarData *data = new AvatarData();
		data->id = atoi(filename.substr(6, filename.size() - 12).c_str());
		//data->name[0] = StringUtils::format("atlas%d_%d.png", data->id, 0);
		data->path = AVATARPATH + filename;
		_avatarsData[data->id] = data;
		SpriteFrameCache::getInstance()->addSpriteFramesWithFile(data->path);
		//CCLOG("id:%d\tpath:%s\tname0:%s\tname1:%s\tname2:%s", data->id, data->path.c_str(), StringUtils::format("atlas%d_%d.png", data->id, 0).c_str(), StringUtils::format("atlas%d_%d.png", data->id, 1).c_str(), StringUtils::format("atlas%d_%d.png", data->id, 2).c_str());
	}
	CCLOG("avatars:%d",_avatarsData.size());
	return true;
}
map<int, AvatarData*> AvatarControl::GetAvatarsData(){
	return _avatarsData;
}
Sprite * AvatarControl::GetAvatarSprite(int id)
{
	return GetAvatarSprite(id, 0);
}
Sprite * AvatarControl::GetAvatarSprite(int id, int index)
{
	auto avatarIt = _avatarsData.find(id);
	if (avatarIt == _avatarsData.end())
		return nullptr;
	AvatarData *data = avatarIt->second;
	CCLOG(StringUtils::format("atlas%d_%d.png", data->id, index).c_str());
	return Sprite::createWithSpriteFrameName(StringUtils::format("atlas%d_%d.png", data->id, index));
}
string AvatarControl::GetAvatarSpriteName(int id, int index)
{
	if (_avatarsData.find(id) == _avatarsData.end())
		id = 1;		//找不到头像时，全部用1号头像代替
	return StringUtils::format("atlas%d_%d.png", id, index);
}