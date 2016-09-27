
#ifndef __THINGSERVER_WEB_INTERFACE_H__
#define __THINGSERVER_WEB_INTERFACE_H__

#include "IDBProxyClient.h"
#include "IEventServer.h"
#include <vector>

struct IActor;

struct SGiveGoodsInfo
{
	SGiveGoodsInfo(){
		m_GiveGoodsID = 0;
		m_GoodsID = INVALID_GOODS_ID;
		m_GoodsNum = 0;
	}

	UINT32		m_GiveGoodsID;
	TGoodsID	m_GoodsID;
	UINT16		m_GoodsNum;
};

enum enCalType
{
	enCalType_One = 1,	//单次充值方式
	enCalType_Total,	//累计充值
};

enum enForwardType
{
	enForwardType_Fix = 1,		//固定额度
	enForwardType_Scale,		//比例
};

struct SRechargeBack
{
	INT32					m_MoneyNum;							//金额数
	UINT8					m_CalType;							//计算的充值方式(1:单次充值，2:累计充值)
	UINT8					m_ForwardType;						//奖励方式(1:固定额度,2:一定比例)
	INT32					m_GodStone;							//奖励的仙石数或者比例
	INT32					m_Ticket;							//奖励的礼卷数或者比例
	INT32					m_Stone;							//奖励的灵石数或者比例
	INT32					m_PolyNimbus;						//奖励的聚灵气或者比例
	std::vector<UINT32>		m_vecGoods;							//物品(ID,数量,...)
};

struct SRechargeBackCnfg
{
	SRechargeBackCnfg()
	{
		MEM_ZERO(this);
	}
	UINT32					m_BeginTime;						//充值返利开始时间
	UINT32					m_EndTime;							//充值返利结束时间
	char					m_szMailTheme[DESCRIPT_LEN_50];		//邮件主题
	char					m_szMailContent[DESCRIPT_LEN_600];	//邮件正文
};


//从某时间开始起首次充值奖励
struct SFirstRechargeCnfg
{
	SFirstRechargeCnfg()
	{
		MEM_ZERO(this);
	}
	UINT8					m_RateTicket;						//获得的礼卷奖励
	UINT8					m_RateGodStone;						//获得的仙石比例
	INT32					m_MinRecharge;						//奖励的最小充值金额
	char					m_szMailTheme[DESCRIPT_LEN_50];;	//邮件主题
	char					m_szMailContent[DESCRIPT_LEN_600];	//邮件内容
};


class WebInterface : public IDBProxyClientSink, public IEventListener
{
public:
    WebInterface();

	~WebInterface();

	
	bool Create();

	void Close();

public:
	//收到后台消息
	virtual void OnRecv(UINT8 nCmd, IBuffer & ib);

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata = 0);

	virtual void	OnEvent(XEventData & EventData);

private:
	//禁言
	void DontTalk(UINT8 nCmd, IBuffer & ib);

	//封号
	void SealNo(UINT8 nCmd, IBuffer & ib);

	//发放物品
	void GiveGoods(UINT8 nCmd, IBuffer & ib);

	//增加资源
	void AddResource(UINT8 nCmd, IBuffer & ib);

	//后台系统邮件
	void BackSysMail(UINT8 nCmd, IBuffer & ib);

	//版本更新
	void VersionUpdate(UINT8 nCmd, IBuffer & ib);

	//客服信息
	void ServiceInfo(UINT8 nCmd, IBuffer & ib);

	//更改关键字
	void ChangeKeyword(UINT8 nCmd, IBuffer & ib);

	//系统消息
	void SysMsg(UINT8 nCmd, IBuffer & ib);

	//支付
	void Pay(UINT8 nCmd, IBuffer & ib);

	//商城
	void ShopMall(UINT8 nCmd, IBuffer & ib);

	//充值返利修改
	void RechargeForward(UINT8 nCmd, IBuffer & ib);

	//重新加载数据库中的怪物配置
	void ReloadMonsterCnfg(UINT8 nCmd, IBuffer & ib);

	//修改活动配置
	void ChangeActivity(UINT8 nCmd, IBuffer & ib);

	//多倍经验变化
	void ChangeMultipExp(UINT8 nCmd, IBuffer & ib);

	//重新加载某时间起首次充值奖励
	void ReloadFirstRechargeCnfg(UINT8 nCmd, IBuffer & ib);

private:
	void HandleDontTalk(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleSealNo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleGiveGoods(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//给玩家发物品
	void GiveGoodsToUser(TUserID UserID, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText);

	//给所有玩家发物品
	void GiveGoodsToAllUser(std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText);

	//在线玩家扣除玩家物品处理
	void OnLine_RemoveGoods(IActor * pActor, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText);

	//离线玩家扣除玩家物品处理
	void OffLine_RemoveGoods(TUserID UserID, std::vector<UINT16> & vectGoods, const char * ThemeText, const char * ContentText);

	//字符串中得到数值参数
	void GetVectValue(char * pszValue, std::vector<UINT32> & vectValue);

	void HandleAddResource(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//在线扣除资源
	void OnLine_DesResource(IActor * pActor, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText);

	//离线扣除资源
	void OffLine_DescResource(TUserID UserID, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText);

	//增加资源
	void AddResource(TUserID UserID, INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText);

	//给所有玩家增加资源
	void AddAllUserResource(INT32 nMoney, INT32 nStone, INT32 nTicket, INT32 nPolyNimbus, const char * ThemeText, const char * ContentText);

	void HandleBackSysMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//给所有玩家写文本系统邮件
	void WriteAllUserSysMail(const char * pszThemeText, const char * pszContentText);

	//用UserID给玩家写文本系统邮件
	void WriteUserSysMail(TUserID UserID, const char * pszThemeText, const char * pszContentText, bool bAddMailNum = false);

	//得到所有禁言
	void LoadDontTalk();

	void HandleLoadDontTalk(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到所有封号
	void LoadSealNo();

	void HandleLoadSealNo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleVersionUpdate(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleServiceInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleSysMsg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到所有系统消息
	void LoadSysMsg();

	void HandleAllSysMsg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleGetPayData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleGetAllPayData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//支付仙石给玩家
	bool PayMoneyToUser(IActor * pActor, const SDB_Get_PayData & PayData);

	void HandleGetGoodsShopCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//得到充值返利信息
	void LoadRechargeForward();

	void HandleGetRechargeForward(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	const SRechargeBack * GetOneRecharge(INT32 OneRechargeNum);

	const SRechargeBack * GetTotalRecharge(INT32 TotalRechargeNum);

	void GetRechargeForword(IActor * pActor, const SRechargeBack * pBack, INT32 RechargeNum = 0, INT32 TotalRechargeNum = 0);

	void HandleGetRechargeForwardCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//是否在充值返利期间
	bool IsInRechargeBackTime();

	void HandleGetTotalRecharge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleGetMultipExpInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleGetFirstRechargeCnfg(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	void HandleIsGetFirstRecharge(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//给某时间起首次充值的玩家奖励
	void GetFirstRechargeForward(UID uidUser, UINT32 RechargeGodStoneNum);

	//得到某时间首次充值奖励信息
	void LoadFirstRechargeForward();

private:
	SRechargeBackCnfg			m_RechargeBackCnfg;		//充值返利配置

	std::vector<SRechargeBack>	m_vecOneRechare;		//单次充值返利奖励

	std::vector<SRechargeBack>	m_vecTotalRechare;		//累计充值反复奖励

	SFirstRechargeCnfg			m_FirstRechargeCnfg;	//给某时间开始起首次充值奖励
};















#endif
