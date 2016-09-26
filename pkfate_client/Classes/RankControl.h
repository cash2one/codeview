#pragma once
#include "cocos2d.h"

USING_NS_CC;
using namespace std;
namespace Rank{
	struct RankData{
		
		string nickname;
		string username;
		int user_id;
		int avatar;
		int64_t balance;
		unsigned int vip_level = 0;
		double profit = DBL_MIN;//没值判断
		int winRate = -1;

	};



	class RankControl :public Ref
	{
	public:
		static RankControl* GetInstance();
		void setRankView(Node* node);
		bool Init();		//从服务端返回的json数据，生成balance data
	private:
		RankControl();
		~RankControl();
		Node *_rankView;
		map<std::string, std::string > _rankConfig;
		void onRankSwitch(Ref *pSender);
		map<string, vector<RankData*>*> _rankDic;
		void onGetRankCallBack(Ref *pSender);

		void clearCacheAtPoint(float dt);

		string getTypeString(unsigned short type, unsigned short subType);

		bool InitRankData(std::string jsonData, string rankType);
	};

}
