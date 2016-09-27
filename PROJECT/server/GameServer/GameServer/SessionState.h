
#ifndef __GAMESERVER_SESSION_STATE_H__
#define __GAMESERVER_SESSION_STATE_H__


#include "ISession.h"
#include "IGameWorld.h"
#include "GameSrvProtocol.h"
#include "DBProtocol.h"

class Session;

///会话状态接口
struct	ISessionState 
{
	virtual	void	Enter(void *pContext)=0;

	virtual	void	Leave(void * pContext)=0;

	//数据到达,
	virtual void OnNetDataRecv(OStreamBuffer & osb)=0;

	virtual	void	OnCloseConnect()=0;

		// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0)=0;

	virtual	enPlayerSessionState	GetStateID()=0;	

	//装载玩家入内存
	virtual void  LoadActor(const UID & UID_Actor) = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//进入服务器态
class EnterServerState : public ISessionState
{
public:
	EnterServerState(Session * pSession);

	virtual ~EnterServerState();

public:
    virtual	void	Enter(void *pContext);

	virtual	void	Leave(void * pContext);

	//数据到达,
	virtual void OnNetDataRecv(OStreamBuffer & osb);

	virtual	void	OnCloseConnect();

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual	enPlayerSessionState	GetStateID();

	//装载玩家入内存
	virtual void  LoadActor(const UID & UID_Actor);

private:
	void HandleClientEnter(int nCmd,IBuffer & ib, OBuffer4k & ob);

	void HandleClientCreateActor(int nCmd,IBuffer & ib, OBuffer4k & ob);

	void HandleClientCheckCnfg(int nCmd,IBuffer & ib, OBuffer4k & ob);

	void  CheckCnfgFile(const std::string strFileName,const char* szMd5);

	//void HandleDBRetGetActorInfo(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	//	OBuffer4k & ReqOb,UINT64 userdata);

	//进入游戏服应答
	void HandleEnterGameRsp(enEnterGameRetCode RetCode,OBuffer4k & RspOb);

	void Check_Build_Actor(OBuffer4k & ob);

	//创建新角色时，向数据库请求得到所有part数据
	void GetThingPartDataCreateNewUser(UINT64 uidUser, TUserID UserID);
	//已经存在的玩家进入角色时，向数据库请求得到所有part数据
	void GetThingPartDataEnterGame(UINT64 uidUser, TUserID UserID);
	//招募角色的PART数据获取
	void GetThingPartDataEmployEnter(UINT64 uidUser, TUserID UserID);

	//资源产出型建筑PART
	void HandleGetBuildinData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//伏魔洞PART
	void HandleGetFuMoDongData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//练功堂PART
	void HandleGetTrainingHallData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//聚仙楼
	void HandleGetGatherGodHouseData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//修炼PART
	void HandleGetXiuLianData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//副本PART
	void HandleGetFuBenData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//剑冢PART
	void HandleGetGodSwordShopData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//背包PART
	void HandleGetPacketData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//装备PART
	void HandleGetEquipPanelData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//法术栏PART
	void HandleGetMagicPanelData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//帮派技能
	void HandleGetSynMagicData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//战斗数据
	void HandleGetCombatData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//状态数据
	void HandleGetStatusData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//获得玩家基本数据，并调用获取玩家的所有PART数据
	void HandleGetActorBasicDataAndPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);
	//斗法数据
	void HandleGetDouFaData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//任务数据
	void HandleGetTaskData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//成就数据
	void HandleGetChengJiuData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//活动
	void HandleGetActivityData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//冷却时间
	void HandleGetCDTimerData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//帮派PART
	void HandleGetSynPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//法宝PART
	void HandleGetTalismanPart(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//获取玩家招募角色集的数据
	void HandleGetEmployeeData(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	//检测是否可以创建招募角色，可以的话创建招募角色
	void CheckBuildEployee(UINT64 Uid_Employee);

	//主角登录
	bool ActorLogin(IActor * pActor);

	//招募角色登录
	bool EmployeeLogin(IActor * pMaster);

	//通知客户端初始化
    void NoticClientInit(TSceneID SecenID,UID uidActor);

   //帐号重复登陆，踢掉旧玩家
   void KickUser(IActor* pActor);

private:
	Session * m_pSession;

	SBuild_Actor m_BuildActor;

	bool	  m_bSuccessBuildUser;	//玩家角色是否创建成功

	bool      m_bNeedUpdateCnfg;  //是否需要更新配置文件

	bool      m_bNeedUpdateRes;  //是否需要更新资源文件

	bool	  m_bSelfEnter;		//是否是玩家自己登录
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//玩游戏态
class GamePlayState : public ISessionState
{
public:
	GamePlayState(Session * pSession);

	virtual ~GamePlayState();

public:
    virtual	void	Enter(void *pContext);

	virtual	void	Leave(void * pContext);

	//数据到达,
	virtual void OnNetDataRecv(OStreamBuffer & osb);

	virtual	void	OnCloseConnect();

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual	enPlayerSessionState	GetStateID();

	//装载玩家入内存
	virtual void  LoadActor(const UID & UID_Actor);


private:
	Session * m_pSession;
};



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//登出游戏态
class LogoutState : public ISessionState
{
public:
	LogoutState(Session * pSession);

	virtual ~LogoutState();

public:
    virtual	void	Enter(void *pContext);

	virtual	void	Leave(void * pContext);

	//数据到达,
	virtual void OnNetDataRecv(OStreamBuffer & osb);

	virtual	void	OnCloseConnect();

	// nRetCode: 取值于 enDBRetCode
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

	virtual	enPlayerSessionState	GetStateID();

	//装载玩家入内存
	virtual void  LoadActor(const UID & UID_Actor);


private:
	Session * m_pSession;
};




#endif
