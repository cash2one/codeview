#include "cocos2d.h"
#include "VirtualProxy.h"
#include "PKNotificationCenter.h"
#include "UserControl.h"
#include "TimeControl.h"
#include "BillControl.h"
#include "GuideControl.h"

#define VIRTUAL_DZ_TRIGGER_FILE "static/dz_virtual_trigger.json"
#define VIRTUAL_DZ_MSG_FILE "static/dz_virtual_msg.json"

#define VIRTUAL_BJL_TRIGGER_FILE "static/bjl_virtual_trigger.json"
#define VIRTUAL_BJL_MSG_FILE "static/bjl_virtual_msg.json"

static VirtualProxy *m_pInstance = nullptr;
VirtualProxy::VirtualProxy()
{
}

VirtualProxy::~VirtualProxy()
{
	dispose();//释放
}

VirtualProxy* VirtualProxy::GetInstance()
{
	if (m_pInstance == nullptr)
	{
		m_pInstance = new VirtualProxy();
		if (m_pInstance)
		{
			if (!m_pInstance->init())
			{
				delete m_pInstance;
				m_pInstance = nullptr;
			}
		}
	}
	return m_pInstance;
}

bool VirtualProxy::init()
{
	return true;
}

void VirtualProxy::dispose()
{
	this->currentTriggerBean = nullptr;

	std::queue<TriggerBean*>  triggerBeanList;

	while (triggerBeanList.size() > 0){
		TriggerBean* bean = triggerBeanList.front();
		triggerBeanList.pop();
		delete bean;
	}

	for (auto item : msgMap)
	{
		delete item.second;
	}

	msgMap.clear();
}

void VirtualProxy::startWithType(int type)
{
	dispose();//释放

	if (!GuideControl::GetInstance()->checkInGuide()) return;

	current_tech_game_type = type;

	switch (current_tech_game_type)
	{
	case GAMETYPE::BJL:
	{
		readVirtualMsg(VIRTUAL_BJL_MSG_FILE);
		readVirtualTrigger(VIRTUAL_BJL_TRIGGER_FILE);
	}
		break;
	case GAMETYPE::DZPK:
	{
		readVirtualMsg(VIRTUAL_DZ_MSG_FILE);
		readVirtualTrigger(VIRTUAL_DZ_TRIGGER_FILE);
	}
		break;

	default:
		break;
	}
	startNextTriggerBean();//开启一个触发bean
}

bool VirtualProxy::SendMsg(unsigned short type, unsigned short typesub, std::string data)
{
	CCLOG("vitual send:\n{\n\ttype:%d\n\ttypesub:%d\n\tcode:%d\n\tlen:%d\n\tdata:%s\n}", type, typesub, 0, 0, data.c_str());

	string triggerType = StringUtils::format("%d_%d", type, typesub);

	SendTrigger(triggerType);
	
	return true;
}

bool VirtualProxy::SendTrigger(string triggerType)
{
	CCLOG("trigger :%s",triggerType.c_str());
	if (!currentTriggerBean || currentTriggerBean->trigger_type != triggerType) return false;

	sendTriggerBean(currentTriggerBean);

	return true;
}


bool VirtualProxy::CreateSocketData(VirtualMsg* msg, SOCKETDATA &data)
{
	data.type = msg->type;
	data.typesub = msg->typesub;
	data.code = 0;
	data.len = 0;
	data.content = msg->data;

	CCLOG("vitual recv:\n{\n\ttype:%d\n\ttypesub:%d\n\tcode:%d\n\tlen:%d\n\tdata:%s\n}", data.type, data.typesub, data.code, data.len, data.content.c_str());
	return true;
}

void VirtualProxy::OnMessage(SOCKETDATA *data)
{
	RECVMSG *msg = RECVMSG::create(data);
	if (msg) {
		PKNotificationCenter::getInstance()->postNotification(GetMsgTypeString(data->type, data->typesub), (Ref*)msg);
		msg->release();
	}
}


bool VirtualProxy::isAutoTrigger(string trigger_type)
{
	return trigger_type == "0";
}



void VirtualProxy::startNextTriggerBean()
{
	if (triggerBeanList.size() > 0){
		currentTriggerBean = triggerBeanList.front();
		triggerBeanList.pop();
		if (currentTriggerBean){
			CCLOG("start triggerBean:%s,size:%d", currentTriggerBean->trigger_type.c_str(), currentTriggerBean->triggers.size());
		}
		else{
			CCLOG("start triggerBean error");
		}
		
		if (currentTriggerBean&&isAutoTrigger(currentTriggerBean->trigger_type)){
			CCLOG("auto trigger");
			sendTriggerBean(currentTriggerBean);
		}
		else{
			CCLOG("wait trigger");
		}
	}
	else{
		CCLOG(" triggerList finish");
	}
}

void VirtualProxy::sendTriggerBean(TriggerBean* triggerBean)
{

	if (triggerBean->curIndex >= triggerBean->triggers.size()){
		startNextTriggerBean();//开启下一个
		return;//结束递归发送
	}
	Trigger* trigger = triggerBean->triggers[triggerBean->curIndex++];
	float delay = trigger->delay;
	Director::getInstance()->getScheduler()->schedule([=](float ft){//下一帧
		triggerMsg(trigger);

		sendTriggerBean(triggerBean);

	}, this, 0, 0, delay!=0?delay:1/60, false,toString(rand()));

}

void VirtualProxy::triggerMsg(Trigger* trigger)
{
	if (msgMap.find(trigger->msg_id) == msgMap.end()) return;
	VirtualMsg* virtualMsg = msgMap[trigger->msg_id];
	CCLOG("trigger msg_id:%d", trigger->msg_id);
	SOCKETDATA socketData;
	if (CreateSocketData(virtualMsg, socketData))
	{
		OnMessage(&socketData);
	}
}


bool VirtualProxy::readVirtualTrigger(string fileName)
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
	cJSON * triggerList, *trigger, *msg_id, *delay, *trigger_type;
	int t_size = 0;

	for (int i = 0; i < size; i++) {
		if (!(triggerList = cJSON_GetArrayItem(jsonMsg, i)) || triggerList->type != cJSON_Array){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else
		{
			TriggerBean* triggerBean = new TriggerBean;
			triggerBeanList.push(triggerBean);

			std::vector<Trigger*> *triggers = new std::vector<Trigger*>();

			t_size = cJSON_GetArraySize(triggerList);

			for (int i = 0; i < t_size; i++) {
				if (!(trigger = cJSON_GetArrayItem(triggerList, i)) || trigger->type != cJSON_Object
					|| !(trigger_type = cJSON_GetObjectItem(trigger, "trigger_type")) || trigger_type->type != cJSON_String
					|| !(msg_id = cJSON_GetObjectItem(trigger, "msg_id")) || msg_id->type != cJSON_Number
					|| !(delay = cJSON_GetObjectItem(trigger, "delay")) || delay->type != cJSON_Number
					){
					cJSON_Delete(jsonMsg);
					return false;
				}
				else
				{
					triggerBean->trigger_type = trigger_type->valuestring;
					Trigger * trigger = new Trigger;
					trigger->msg_id = msg_id->valueint;
					trigger->delay = delay->valuedouble;
					triggerBean->triggers.push_back(trigger);
				}
			}

		}
	}
	cJSON_Delete(jsonMsg);
	return true;

}

string&   VirtualProxy::replaceAll(string&   str, const   string&   old_value, const   string&   new_value)
{
	while (true)   {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}



bool VirtualProxy::readVirtualMsg(string fileName)
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
	cJSON *item, *type, *typesub, *data, *msg_id, *replace;
	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsg, i)) || item->type != cJSON_Object){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else
		{
			if (!(type = cJSON_GetObjectItem(item, "type")) || type->type != cJSON_Number
				|| !(typesub = cJSON_GetObjectItem(item, "typesub")) || typesub->type != cJSON_Number
				|| !(msg_id = cJSON_GetObjectItem(item, "msg_id")) || msg_id->type != cJSON_Number
				|| !(data = cJSON_GetObjectItem(item, "data"))
				){
				cJSON_Delete(jsonMsg);
				return false;
			}

			VirtualMsg * msg = new VirtualMsg;
			msg->type = type->valueint;
			msg->typesub = typesub->valueint;
			msg->data = cJSON_Print(data);
			msgMap[msg_id->valueint] = msg;
			//check replace 需要替换的
			if ((replace = cJSON_GetObjectItem(item, "replace")) && replace->type == cJSON_Array){

				int r_size = cJSON_GetArraySize(replace);
				cJSON *replace_key;
				for (int i = 0; i < r_size; i++){
					if ((replace_key = cJSON_GetArrayItem(replace, i)) && replace_key->type == cJSON_String){
						replaceValue(msg->data, replace_key->valuestring);
					}
				}
			}
		}
	}

	cJSON_Delete(jsonMsg);//delete
	return true;
}



void VirtualProxy::replaceValue(string& content,string replace){
	if (replace == "USER_ID"){
		int user_id = UserControl::GetInstance()->GetUserData()->user_id;
		content = replaceAll(content, "\"USER_ID\"", toString(user_id)); //数值型的替换
	}
	else if(replace == "USER_NAME"){
		string nickname = UserControl::GetInstance()->GetUserData()->nickname;
		content = replaceAll(content, "USER_NAME", nickname); //字符型的替换
	}
	else if (replace == "USER_AVATAR"){
		int avatar = UserControl::GetInstance()->GetUserData()->avatar;
		content = replaceAll(content, "\"USER_AVATAR\"", toString(avatar)); //数值型的替换
	}
	else if (replace == "BALANCE"){
		double balance = BillControl::GetInstance()->GetBalanceData(0)->balance;
		content = replaceAll(content, "\"BALANCE\"", toString(balance)); //数值型的替换
	}
	else if (replace == "NEXT_EVENT_TIME"){
		int time = TimeControl::GetInstance()->GetServerTime() ;
		content = replaceAll(content, "\"NEXT_EVENT_TIME\"", toString(time)); //数值型的替换
	}
	else if (replace == "BALANCE_MINUS"){
		double balance = BillControl::GetInstance()->GetBalanceData(0)->balance;
		content = replaceAll(content, "\"BALANCE_MINUS\"", toString(balance-100)); //数值型的替换
	}
	else if (replace == "BALANCE_ADD"){
		double balance = BillControl::GetInstance()->GetBalanceData(0)->balance;
		content = replaceAll(content, "\"BALANCE_ADD\"", toString(balance + 100)); //数值型的替换
	}
}


