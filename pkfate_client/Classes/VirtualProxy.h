#pragma once
//#include <map>
#include "comm.h"
#include "SocketClient.h"
#include "cJSON.h"
#define MAX_GUIDE_STEP 25
using namespace std;

struct Trigger
{
	int msg_id;
	float delay;
};

struct TriggerBean
{
	TriggerBean() :curIndex(0){};
	string trigger_type;
	vector<Trigger*> triggers;
	int curIndex;
};

struct VirtualMsg
{
	int type;
	int typesub;
	string data;
};


class VirtualProxy:Ref
{
public:	
	~VirtualProxy();
	
	static VirtualProxy *GetInstance();

	void startWithType(int type);

	void dispose();

	void startNextTriggerBean();

	bool SendMsg(unsigned short type, unsigned short typesub, std::string data = ""); // turnback表示原样推送 传输的数据

	bool SendTrigger(string triggerType);//

private:

	void replaceValue(string& content, string replace);//获取当前用户替换的KEY的实际值

	bool CreateSocketData(VirtualMsg* msg, SOCKETDATA &data);

	void OnMessage(SOCKETDATA *data);

	bool isAutoTrigger(string trigger_type);//是否自动触发

	void sendTriggerBean(TriggerBean* triggerBean);
	
	void triggerMsg(Trigger* trigger);
	

	bool readVirtualMsg(string fileName);
	bool readVirtualTrigger(string fileName);

	string& replaceAll(string&   str, const   string&   old_value, const   string&   new_value);

	TriggerBean* currentTriggerBean;

	std::queue<TriggerBean*>  triggerBeanList;

	map<int, VirtualMsg*> msgMap;

	int current_tech_game_type;

	VirtualProxy();
	bool init();
};
