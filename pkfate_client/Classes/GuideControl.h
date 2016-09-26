#pragma once
#include"cocos2d.h"
#include "cJSON.h"
USING_NS_CC;
using namespace std;

#define MAX_GUIDE_STEP 25

enum TECHTYPE{
	VIDEO = 1,
	SOUND = 2,
	SAY = 3,
	CLICK =4,
	HAND=5
};
struct GuideData{
	unsigned int pageIndex;
	Point anchor;
    Point position;
	string tip;
};


struct TechData{
	bool wait;
	int type;
	float delay;
	int width;
	int height;
	Point pos;
	string res;
	string url;
	std::function<void()>  handler;
};




enum GUIDESTEP{
	N1=1,
	N2=2,
	N3=3,
	N4=4,
	N5=5,
	N6=6,
	N7=7,
	N8=8,
	N9=9,
	N10=10,
	N11=11,
	N12=12,
	N13=13,
	N14=14,
	N15=15
};

class GuideControl :public Ref
{
public:
	static GuideControl* GetInstance();
	std::vector<GuideData*>* getGuideListByPageIndex(unsigned int PageIndex);
	bool read(string fileName);
	bool readTech(string fileName);
	std::vector<TechData*>* getTechData(int step);

	

	bool checkInGuide(int step = MAX_GUIDE_STEP);

	int getFinishGuideStep();

	void finishGuideStep(int step = MAX_GUIDE_STEP);

	void finishGuideStepWithType(int type, int step = MAX_GUIDE_STEP);

	void injectHander(int step, string key, std::function<void()>  handler);

	map<int, std::vector<TechData*>*> &getTechMap();

	bool sendVirtual(unsigned short type, unsigned short typesub, std::string data);//发送guide 代理消息

	bool sendVitualTrigger(string tiggerType);

	void startWithType(int type);

	void dispose();
	string getGuideWaitVideo();//获取等待视频


	bool isGuideFinished(int type);//判断 某个类型的游戏是否完成引导

	bool uploadGameStep(int step);
	
	bool getGameStep(int game_type);
	
	//bool page_game_step(); todo

	void onGetGameStep(Ref*pSender);

	int current_tech_game_type;//当前的游戏类型

private:

	
	map<int, std::vector<GuideData*>*> guideMap;//帮助的引导

	map<int, std::vector<TechData*>*> techMap;//教学引导


	GuideControl();
	~GuideControl();
};