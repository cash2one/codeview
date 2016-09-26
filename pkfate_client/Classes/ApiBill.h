#pragma once
#include "SocketControl.h"
using namespace std;
namespace Api
{
	class Bill
	{
	public:
		Bill();
		~Bill();
		static bool GET_BALANCE();
		static bool Get_Balance(int userId, int type);
		static bool GET_LEVEL_CONFIG();
		static bool List_Balance(int userId);
		
		static bool GetAllGoods();
		
		static bool BuyGoods(int userId, int goodsId, int amount);
		//表单相关
		static bool sg_baccarat_bill_page_date_summary_tree(string strFrom, string strTo, int page_index, int page_size, int child = 0);
		static bool bill_page_date_summary_tree(string create_data, int page_index, int user_id, int page_size, int child_id);
		static bool sg_baccarat_bill_page_agent_balance(int page_index, int page_size, int childId = 0);
		static bool sg_baccarat_bill_page_flow(string strFrom, string strTo, int page_index, int page_size, int child);

		//排行
		static bool sg_bill_get_rank(unsigned short rankType);
		
		static bool GET_BALANCE_STATE(int userId, int currencyType, int businessType);
	private:
	};
}