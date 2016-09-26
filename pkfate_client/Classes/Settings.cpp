#include "Settings.h"
#include "CurlControl.h"
#include "cJSON.h"
#include "ResourceControl.h"
#include "PKNotificationCenter.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#define SETTINGURL	"http://pkfate.com/publish/serverinfo.json"
#else		 
#define SETTINGURL	"http://pkfate.com/publish/serverinfo.json"
#endif


static Settings *m_pInstance = nullptr;
Settings::Settings()
{
	
}
Settings::~Settings()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

Settings* Settings::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
		m_pInstance = new Settings();
	return m_pInstance;
}

void Settings::GetServerInfo()
{
	// request url
	CurlControl *curl = new CurlControl();
	curl->onDone = CC_CALLBACK_1(Settings::onGetServerInfoCallBack, this);
    CCLOG(SETTINGURL);
	curl->GetUrlAsync(SETTINGURL);
}
void Settings::onGetServerInfoCallBack(Ref *ref)
{
	if (ref)
	{
		// dump data
		CurlMsg *msg = (CurlMsg*)ref;
		cJSON *jsonData = cJSON_Parse(msg->data);
		if (jsonData)
		{
			ServerInfo.clear();
			if (ResourceControl::GetInstance()->RemoteResources.size())
			{
				for (auto pRes : ResourceControl::GetInstance()->RemoteResources)
				{
					delete pRes;
				}
				ResourceControl::GetInstance()->RemoteResources.clear();
			}
			//解析serverinfo json数据
			if (jsonData->type == cJSON_Object)
			{
				cJSON *data = jsonData->child;
				while (data)
				{
					if (data->type == cJSON_String)
						ServerInfo[data->string] = data->valuestring;
					else
					{
						if (strcmp(data->string, "resources") == 0 && data->type == cJSON_Array)
						{
							//解析资源文件
							int count = cJSON_GetArraySize(data);
							if (count > 0)
							{
								for (int i = 0; i < count; i++)
								{
									cJSON *resdatas = cJSON_GetArrayItem(data, i);
									if (resdatas == NULL)
										break;
									cJSON *respath = cJSON_GetObjectItem(resdatas, "path");
									if (!respath || respath->type != cJSON_String)
										break;
									cJSON *ressha = cJSON_GetObjectItem(resdatas, "sha");
									if (!ressha || ressha->type != cJSON_String)
										break;
									RemoteResource *res = new RemoteResource();
									res->path = respath->valuestring;
									res->sha = ressha->valuestring;
									res->url = ServerInfo["resroot"]+respath->valuestring+".zip";
									ResourceControl::GetInstance()->RemoteResources.push_back(res);
								}
							}
						}
					}
					data = data->next;
				}
			}
			cJSON_Delete(jsonData);
		}
	}
	PKNotificationCenter::getInstance()->postNotification("getserverinfo",nullptr);		//通知SceneStart已经返回
}