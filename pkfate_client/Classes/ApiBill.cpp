#include "ApiBill.h"
#include "cmd.h"
using namespace Api;
Bill::Bill()
{
}
Bill::~Bill()
{
}
bool Bill::GET_BALANCE()
{
	char args[BUFSIZE];
	sprintf(args, "{\"currency_type\":0}");
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::GET_BALANCE, args);
}
bool Bill::Get_Balance(int userId, int type)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"currency_type\":%d}", userId, type);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::GET_BALANCE, args);
}
bool Bill::List_Balance(int userId)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d}", userId);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::LIST_BALANCE, args);
}
bool Bill::GET_LEVEL_CONFIG()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::GET_LEVEL_CONFIG, "");
}

bool Bill::sg_bill_get_rank(unsigned short rankType){

	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, rankType, "");
}


bool Bill::sg_baccarat_bill_page_date_summary_tree(string strFrom, string strTo, int page_index, int page_size, int child){
	if (strFrom.empty() || strTo.empty() || page_index <= 0 || page_size <= 0) {
		return false;
	}
	char args[BUFSIZE];
	if (child > 0)
		sprintf(args, "{\"create_date\":[\"%s\",\"%s\"],\"page_index\":%d,\"page_size\":%d,\"child\":%d}", strFrom.c_str(), strTo.c_str(), page_index, page_size, child);
	else
		sprintf(args, "{\"create_date\":[\"%s\",\"%s\"],\"page_index\":%d,\"page_size\":%d}", strFrom.c_str(), strTo.c_str(), page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::PAGE_DATE_SUMMARY_TREE, args);
}

//{"create_date":["2015-08-01","2015-09-20"],"page_index":1,"user_id":1,"page_size":100,"child":166}
bool Bill::bill_page_date_summary_tree(string create_data, int page_index, int user_id, int page_size, int child_id) {
	if (create_data == "" || page_index <= 0 || user_id < 0 || page_size <= 0 || child_id < 0) {
		return false;
	}
	char args[BUFSIZE];
	sprintf(args,
		"{\"create_date\":%s,\"page_index\":%d,\"user_id\":%d,\"page_size\":%d,\"child_id\":%d}", create_data.c_str(), page_index, user_id, page_size, child_id);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::PAGE_DATE_SUMMARY_TREE, args);
}

//{"page_index":1,"page_size":100,"child":1}
bool Bill::sg_baccarat_bill_page_agent_balance(int page_index, int page_size, int childId) {
	if (page_index <= 0 || page_size <= 0)
		return false;
	char args[BUFSIZE];
	if (childId > 0)
		sprintf(args, "{\"page_index\":%d,\"page_size\":%d,\"child\":%d}", page_index, page_size, childId);
	else
		sprintf(args, "{\"page_index\":%d,\"page_size\":%d}", page_index, page_size);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::PAGE_AGENT_BALANCE, args);
}

bool Bill::sg_baccarat_bill_page_flow(string strFrom, string strTo, int page_index, int page_size, int child) {
	if (strFrom.empty() || strTo.empty() || page_index <= 0 || page_size <= 0) {
		return false;
	}
	strFrom += " 00:00:00";
	strTo += " 23:59:59";
	char args[BUFSIZE];
	sprintf(args, "{\"create_time\":[\"%s\",\"%s\"],\"page_index\":%d,\"page_size\":%d,\"child\":%d}", strFrom.c_str(), strTo.c_str(), page_index, page_size, child);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::PAGE_FLOW, args);
}

bool Bill::BuyGoods(int userId, int goodsId, int amount)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"id\":%d,\"amount\":%d}", userId, goodsId, amount);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::BUY_GOODS, args);
}

bool Bill::GetAllGoods()
{
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::GET_ALL_GOODS, "");
}

bool Bill::GET_BALANCE_STATE(int userId, int currencyType, int businessType)
{
	char args[BUFSIZE];
	sprintf(args, "{\"user_id\":%d,\"currency_type\":%d,\"business_type\":%d}", userId, currencyType, businessType);
	return SocketControl::GetInstance()->SendMsg(MSGTYPE_BILL, MSGCMD_BILL::GET_BALANCE_STAT, args);
}