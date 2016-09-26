#pragma once
#include "cocos2d.h"
#include "ui/UIText.h"
#include "ui/UIListView.h"
#include "comm.h"
USING_NS_CC;
using namespace std;
using namespace cocos2d::ui;
namespace Report
{
#define DATE_MODIFY_IN_GAME_FORM		 "data_modify_in_game_form"
#define DATE_MODIFY_IN_PROFIT_FORM	   "data_modify_in_profit_form"
#define REPORT_PAGE_SIZE	10
	//用户导航类
	class ListUserBar
	{
	private:
		ListView *_listView;
		void onSelectedItemEvent(Ref *pSender, ListView::EventType type);
		bool init(ListView* listView);
	public:
		bool AddUserToBar(int userid, string nickname);
		static ListUserBar* create(ListView* listView);
		std::function<void(int userid)> onUserChanged;
	};
	//报表基类
	class ReportControl
	{
	protected:
		ReportControl();
		Node *_rootNode;
		ListView *_listViewReport;
		int _totalCount;
		vector<void*> _records;
		CREATE_C_FUNC(ReportControl);
		void onSelectedItemEvent(Ref *pSender, ListView::EventType type);
		void onSelectedItemEventScrollView(Ref *pSender, ScrollView::EventType type);
		string _className;
	public:
		~ReportControl();
		void clear();
		void show();
		void hide();
		virtual bool init();
		virtual void setData(void* datas);
		virtual void setTotal(int totalCount, void *data);
		Node *getRootNode();
		string getSelectedItemEventName();
		string getNextPageEventName();
	};
	
	//盈亏明细报表类
	struct BillFlow {
		int flow_id;
		string create_time;
		int business_type;
		int operate_type;
		int64_t amount;
	};
	class LayerBillFlowList :public Layer
	{
	public:
		~LayerBillFlowList(void);
		void Show(string strFrom, string strTo, int child);
		virtual bool init();
		CREATE_FUNC(LayerBillFlowList);
	private:
		LayerBillFlowList(void);
		void onSelectedItemEvent(Ref *pSender, ListView::EventType type);
		void onSelectedItemEventScrollView(Ref *pSender, ScrollView::EventType type);
		void onGetListDataCallBack(Ref *pSender);
		void clear();
		bool InitListData(string jsonData, int *totalCount, vector<BillFlow*> *records);

		int _userid;					//当前显示用户ID
		Node* _rootNode;
		Sprite *_spBg;
		ListView *_listViewReport;		//报表列表控件
		int _totalCount;				//总记录数
		vector<BillFlow*> _records;		//用户列表数据
		BillFlow * _curRecord;			//当前选中列表项
		Text *_txtTotal;
		string _strFrom, _strTo;
	};
	//用户报表类
	struct UserFormItemRecord {
		int user_id;
		string nickname;
		int avatar;
		string username;
		int64_t balance;
		int children_num;
		string last_login_time;
	};
	class ReportUserControl:public ReportControl
	{
	public:
		ReportUserControl(void);
		CREATE_C_FUNC(ReportUserControl);
		virtual void setData(void* datas);
		virtual void setTotal(int totalCount, void *data);
	private:
		Text *_txtTotal, *_txtTotalCoin, *_txtTotalPlayerNum;
		virtual bool init();
	};
	//盈亏报表类
	struct ProfitRecord {
		int user_id;
		string nickname;
		int avatar;
		int64_t bet_amount;
		int64_t profit;
		int64_t transfer;
		int64_t commission;
		int64_t deposit;
	};
	class ReportProfitControl :public ReportControl
	{
	public:
		ReportProfitControl(void);
		CREATE_C_FUNC(ReportProfitControl);
		virtual void setData(void* datas);
		virtual void setTotal(int totalCount, void *data);
	private:
		virtual bool init();
		int _totalCount;
		Text *_txtTotal, *_txtBetAmount, *_txtBuyTotal,  *_txtCommisionTotal, *_txtProfitTotal, *_txtFinalProfit;
	};

	//游戏详细记录类
	struct GameDetailRecord
	{
		int64_t bet_amount;
		int currency_type;
		int round_id;
		int64_t profit;
		int game_type;
		int _id;
		int64_t commission;
		string create_time;
		long int commission_time;
		int play_type;
		int64_t valid_amount;
	};
	class ReportGameDetailControl :public ReportControl
	{
	public:
		ReportGameDetailControl(void);
		CREATE_C_FUNC(ReportGameDetailControl);
		virtual void setData(void* datas);
		virtual void setTotal(int totalCount, void *data);
	private:
		virtual bool init();
		int _totalCount;
		Text *_txtTotal, *_txtBetAmount, *_txtProfitTotal;
	};
	class ReportGameControl :public ReportControl
	{
	public:
		ReportGameControl(void);
		CREATE_C_FUNC(ReportGameControl);
		virtual void setData(void* datas);
		virtual void setTotal(int totalCount, void *data);
	private:
		virtual bool init();
		int _totalCount;
		Text *_txtTotal, *_txtBetAmount, *_txtProfitTotal;
	};
	enum REPORTTYPE{ USERREPORT, GAMEREPORT, PROFITREPORT, GAMEDETAILREPORT };
	class ReportsControl :public Ref
	{
		REPORTTYPE _curType;
		ReportControl *_curReport;
		ListUserBar *_listUser;			//用户导航条
		ReportUserControl *_reportUser;
		ReportGameControl *_reportGame;
		ReportGameDetailControl *_reportGameDetail;
		ReportProfitControl *_reportProfit;
		Node *_rootNode, *_panelReport;
		void onUserChanged(int userid);
		void onGetUserReportCallBack(Ref *pSender);
		bool InitUserReportData(string jsonData, int *totalCount, UserFormItemRecord* totalRecord, vector<UserFormItemRecord*> *records);
		void onGetGameDetailReportCallBack(Ref *pSender);
		bool InitGameDetailReportData(string jsonData, int *totalCount, GameDetailRecord* totalRecord, vector<GameDetailRecord*> *records);
		void onGetProfitReportCallBack(Ref *pSender);
		bool InitProfitReportData(string jsonData, int *totalCount, ProfitRecord *totalRecord, vector<ProfitRecord*> *records);
		void onGetGameRoundDataCallBack(Ref *pSender);
		bool init(Node* node);
		int _userid;
		string _strFrom, _strTo;
		void showGameRecord(GameDetailRecord *record);		//显示单项游戏记录
		GameDetailRecord* _curGameDetailRecord;			//当前显示的游戏明细记录项,此方法逻辑性不强，可能会出错。
	public:
		~ReportsControl();
		void showReport(REPORTTYPE type, string strFrom, string strTo);
		static ReportsControl* create(Node* node);
		std::function<void(int,string)> onSelectedUser;
		void onDateChanged(string strFrom, string strTo);
	};
}