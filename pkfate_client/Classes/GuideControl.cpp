#include "GuideControl.h"
#include "UserControl.h"
#include "VirtualProxy.h"
#include "SoundControl.h"
#include "ApiGame.h"
#include "SimpleAudioEngine.h"
#include "PKNotificationCenter.h"
#include "ApiGame.h"
#include "cmd.h"
using namespace CocosDenshion;

#define GUIDE_JSON_FILE "static/guide.json" 
#define GUIDE_JSON_BJL_TECH_FILE "static/bjl_tech.json"
#define GUIDE_JSON_DZ_TECH_FILE "static/dz_tech.json"

#define BJL_WAIT_VIDEO "guide/bjl_video/w1.mp4"
#define DZ_WAIT_VIDEO "guide/dz_video/w1.mp4"


static GuideControl *m_pInstance = nullptr;


bool GuideControl::read(string fileName)
{
	std::string jsonData;
	String* content = String::createWithContentsOfFile(fileName);
	if (!content)return false;
	jsonData = content->getCString();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;
	cJSON *pageIndex, *anchor, *position, *tip;

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Array){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else
		{
			if (!(pageIndex = cJSON_GetArrayItem(item, 0)) || pageIndex->type != cJSON_Number
				|| !(anchor = cJSON_GetArrayItem(item, 1)) || anchor->type != cJSON_String
				|| !(position = cJSON_GetArrayItem(item, 2)) || position->type != cJSON_String
				|| !(tip = cJSON_GetArrayItem(item, 3)) || tip->type != cJSON_String
				){
				cJSON_Delete(jsonMsg);
				return false;
			}
			
			
			Array* anchorArray = String::create(anchor->valuestring)->componentsSeparatedByString(",");
			Array* positionArray = String::create(position->valuestring)->componentsSeparatedByString(",");
			if (anchorArray->count() != 2 || anchorArray->count() != 2)
			{
				cJSON_Delete(jsonMsg);
				return false;
			}

			GuideData* guideData = new GuideData();
			float position_x = ((String*)positionArray->getObjectAtIndex(0))->floatValue();
			float position_y = ((String*)positionArray->getObjectAtIndex(1))->floatValue();
			guideData->position = ccp(position_x, position_y);
			float anchor_x = ((String*)anchorArray->getObjectAtIndex(0))->floatValue();
			float anchor_y = ((String*)anchorArray->getObjectAtIndex(1))->floatValue();
			guideData->anchor = ccp(anchor_x, anchor_y);
			guideData->pageIndex = pageIndex->valueint;
			guideData->tip = tip->valuestring;
			
			vector<GuideData*> *vec = new  vector<GuideData*>();

			if (guideMap.find(guideData->pageIndex) == guideMap.end())
			{
				guideMap[guideData->pageIndex] = vec;
			}
			(*guideMap[guideData->pageIndex]).push_back(guideData);
			
		}
	}

	cJSON_Delete(jsonMsg);//delete
	return true;
}


map<int, std::vector<TechData*>*> &GuideControl::getTechMap()
{
	return techMap;
}

bool GuideControl::readTech(string fileName )
{
	std::string jsonData;
	String* content = String::createWithContentsOfFile(fileName);
	if (!content)return false;
	jsonData = content->getCString();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsg);
	cJSON *item;

	map<int, std::vector<TechData*>*>& techMap = getTechMap();
	

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Array){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else
		{
			int stepSize = cJSON_GetArraySize(item);
			cJSON *stepItem, *stepType,*stepDelay,*stepKey,*stepPos,*stepWidth,*stepHeight,*wait;
			Array* positionArray;
			float position_x, position_y;

			std::vector<TechData*>* techs = new std::vector<TechData*>();
		    
			techMap[i + 1] = techs;

			for (int i = 0; i < stepSize; i++){
				if (!(stepItem = cJSON_GetArrayItem(item, i)) || stepItem->type != cJSON_Object){
					cJSON_Delete(jsonMsg);
					return false;
				}
				else{

					if (!(stepType = cJSON_GetObjectItem(stepItem, "type")) || stepType->type != cJSON_Number){
						cJSON_Delete(jsonMsg);
						return false;
					}

					if (!(stepDelay = cJSON_GetObjectItem(stepItem, "delay")) || stepDelay->type != cJSON_Number){
						cJSON_Delete(jsonMsg);
						return false;
					}

					TechData *techData = new TechData;
					techData->type = stepType->valueint;
					techData->delay = stepDelay->valuedouble;
					techData->wait = false;

					
					if ((stepHeight = cJSON_GetObjectItem(stepItem, "height")) && stepHeight->type == cJSON_Number){
						techData->height = stepHeight->valueint;
					}

					if ((stepWidth = cJSON_GetObjectItem(stepItem, "width")) && stepWidth->type == cJSON_Number){
						techData->width = stepWidth->valueint;
					}

					if ((stepKey = cJSON_GetObjectItem(stepItem, "res")) && stepKey->type == cJSON_String){
						techData->res = stepKey->valuestring;
					}

					if ((stepPos = cJSON_GetObjectItem(stepItem, "pos")) && stepPos->type == cJSON_String){
						positionArray = String::create(stepPos->valuestring)->componentsSeparatedByString(",");
						position_x = ((String*)positionArray->getObjectAtIndex(0))->floatValue();
						position_y = ((String*)positionArray->getObjectAtIndex(1))->floatValue();
						techData->pos = ccp(position_x, position_y);
					}
					if (techData->type == TECHTYPE::SOUND||techData->type == TECHTYPE::VIDEO){
						if (FileUtils::getInstance()->isFileExist(techData->res)){
							techs->push_back(techData);
						}
					}
					else{

						if (techData->type == TECHTYPE::HAND){
							if ((wait = cJSON_GetObjectItem(stepItem, "wait")) && wait->type == cJSON_Number){
								techData->wait = true;
							}
						}
						techs->push_back(techData);
					}
					
				}
			}
		}
	}

	cJSON_Delete(jsonMsg);//delete
	return true;
}

void GuideControl::startWithType(int type)
{
	dispose();//释放
	this->current_tech_game_type = type;

	if (!checkInGuide()) return;

	switch (current_tech_game_type)
	{
	case GAMETYPE::BJL:
	{
		readTech(GUIDE_JSON_BJL_TECH_FILE);
	}
		break;
	case GAMETYPE::DZPK:
	{
		readTech(GUIDE_JSON_DZ_TECH_FILE);
	}
		break;

	default:
		break;
	}

	VirtualProxy::GetInstance()->startWithType(type);

	SoundControl::SetMusicVolume(0.1);
}

string GuideControl::getGuideWaitVideo()
{
	if (current_tech_game_type == GAMETYPE::BJL){
		return BJL_WAIT_VIDEO;
	}
	else if (current_tech_game_type == GAMETYPE::DZPK){
		return DZ_WAIT_VIDEO;
	}
	return "";
}


void GuideControl::injectHander(int step, string key, std::function<void()>  handler )
{

	if (!checkInGuide(step))return;
	std::vector<TechData*>* list = GuideControl::GetInstance()->getTechData(step);
	if (list->empty()) return;

	for (TechData* techData : *list){
		if (techData->type == TECHTYPE::HAND||techData->type == TECHTYPE::CLICK){
			if (key == techData->res){
				techData->handler = handler;
			}
		}
	}
}


bool GuideControl::isGuideFinished(int type)
{
	std::string guide_key = StringUtils::format("%d_guide_%d", UserControl::GetInstance()->GetUserData()->user_id, current_tech_game_type);
	int step = UserDefault::getInstance()->getIntegerForKey(guide_key.c_str());

	return MAX_GUIDE_STEP <= step;
}


bool GuideControl::checkInGuide(int step)
{
	if (current_tech_game_type == 0) return false;
	//step = -1;
	return getFinishGuideStep()< step;
}

int GuideControl::getFinishGuideStep()
{
	std::string guide_key = StringUtils::format("%d_guide_%d", UserControl::GetInstance()->GetUserData()->user_id,current_tech_game_type);
	int step = UserDefault::getInstance()->getIntegerForKey(guide_key.c_str());
	return UserDefault::getInstance()->getIntegerForKey(guide_key.c_str());
}

void GuideControl::finishGuideStep(int step)
{
	std::string guide_key = StringUtils::format("%d_guide_%d", UserControl::GetInstance()->GetUserData()->user_id, current_tech_game_type);
	UserDefault::getInstance()->setIntegerForKey(guide_key.c_str(), step);

	if (step == MAX_GUIDE_STEP){
		SimpleAudioEngine::getInstance()->stopBackgroundMusic(true);
		SoundControl::SetMusicVolume(0.5);//恢复声音大小
	}
}

void GuideControl::finishGuideStepWithType(int type, int step)
{
	this->current_tech_game_type = type;
	this->finishGuideStep(step);
}


void GuideControl::dispose()
{
	map<int, std::vector<TechData*>*> techMap;

	for (auto item : techMap){
		std::vector<TechData*>* list = item.second;
		for (auto techData : *list){
			delete techData;
		}
		list->clear();
		delete list;
	}
	techMap.clear();
}



std::vector<TechData*>* GuideControl::getTechData(int step)
{
	return techMap[step];
}


vector<GuideData*>* GuideControl::getGuideListByPageIndex(unsigned int pageIndex)
{
	if (guideMap.size() <= 0)//未读取
	{
		read(GUIDE_JSON_FILE);
	}
	return guideMap[pageIndex];
}


bool GuideControl::sendVirtual(unsigned short type, unsigned short typesub, std::string data)
{
	if (!checkInGuide()) return false;
	bool ret = VirtualProxy::GetInstance()->SendMsg(type, typesub, data);
	return true;
}

bool GuideControl::sendVitualTrigger(string tiggerType)
{
	if (!checkInGuide()) return false;
	bool ret = VirtualProxy::GetInstance()->SendTrigger(tiggerType);
	return true;
}

GuideControl::GuideControl():current_tech_game_type(0)
{
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(GuideControl::onGetGameStep), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_GAME_STEP), NULL);
}

GuideControl::~GuideControl()
{
	dispose();

	if (guideMap.size()>0)
	{
		for (auto data : guideMap)
		{
			vector<GuideData*> vec = *data.second;

			for (GuideData* var : vec)
			{
				delete var;
			}
			vec.clear();
			delete data.second;
		}
		guideMap.clear();
	}


	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

GuideControl* GuideControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用 单线程无需加锁
	{
		m_pInstance = new GuideControl();
	}

	return m_pInstance;
}

bool GuideControl::uploadGameStep(int step)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	return Api::Game::update_game_step(user_id, current_tech_game_type, step);
}

bool GuideControl::getGameStep(int game_type)
{
	int user_id = UserControl::GetInstance()->GetUserData()->user_id;
	return Api::Game::get_game_step(user_id, game_type);
}

void GuideControl::onGetGameStep(Ref*pSender)
{

}
