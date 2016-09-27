

#ifndef __THINGSERVER_GAMEWORLD_H__
#define __THINGSERVER_GAMEWORLD_H__

#include "IGameWorld.h"
#include <hash_map>
#include "StringUtil.h"
#include "ActorBuilder.h"
#include "ThingContainer.h"
#include "GameScene.h"
#include "MonsterBuilder.h"
#include "IDBProxyClient.h"
#include "CreateEmployee.h"
#include "IMessageDispatch.h"
#include <vector>
#include "ITimeAxis.h"

struct ISession;
struct IBuilding;


class GameWorld : public IGameWorld, public IDBProxyClientSink , public IMsgRootDispatchSink, public ITimerSink
{
   	//游戏世界的所有场景
	typedef	       std::hash_map<TSceneID, GameScene*> GameSceneMap;
	GameSceneMap	m_scenes;

	typedef std::hash_map<const char*,IThing*,std::hash<const char*>,StringUtil::eqstr> MAP_NAME; //通过名称查询

	typedef	std::hash_map<TMapID, UINT16>	SCENE_ID_MAP;
	SCENE_ID_MAP	m_sidMap;

	enum enGameWorldTimer
	{
		enGameWorldTimer_DestroyScene = 0,		//定时检测回收场景
	};

public:
   GameWorld();
  virtual ~GameWorld();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib); 

private:
	void GameWorld::OnEnterPortal(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//解雇招募角色
	void GameWorld::UnLoadEmployee(IActor *pActor,UINT8 nCmd, IBuffer & ib);

public:

	virtual void Release();

	virtual bool Create(); 

	  	//IDBProxyClientSink接口
	virtual	void OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
		OStreamBuffer & ReqOsb,UINT64 userdata=0);

    ///根据UID，查询IThing
    virtual	IThing	*	GetThing(const UID & uid);

    //在游戏世界里创建一个Thing,
    virtual	IThing*				CreateThing(enThing_Class enThingClass,TSceneID SceneID, const char * buf, int len, UINT32 flag);
    //从游戏世界里删除Thing
    virtual	void				DestroyThing(const UID & uid);

		//创建怪物
	virtual IMonster *  CreateMonster(SCreateMonsterContext & MonsterCnt);

		//创建物品
	virtual IGoods * CreateGoods(const SCreateGoodsContext & GoodsCnt );

	virtual std::vector<IGoods *> CreateGoods(TGoodsID GoodsID, UINT32 nNum, bool bBinded);

	//通过UID取得角色
	virtual IActor*				FindActor(const UID& uid);

	virtual IMonster *          FindMonster(const UID& uid);

	GameScene*	GetGameScene(const TSceneID & sid);

	virtual IGameScene*      	CreateGameSceneByMapID(TMapID mapid,INT32 CreateNpcIndex=0, bool bDelNoneUser = false);

	virtual IGameScene*      	CreateGameSceneBySceneID(TSceneID sceneid,INT32 CreateNpcIndex=0, bool bDelNoneUser = false);


		//获得主城场景ID
	virtual TSceneID GetMainSceneID();

		//创建建筑
	virtual IBuilding * CreateBuilding(SCreateBuildingContext & BuildingCnt);

	//创建传送门
	virtual IPortal * CreatePortal(const SCreatePortalContext & PortalCnt);

	virtual IGoods *    GetGoods(UID uidGoods) ;

		virtual IEquipment *    GetEquipment(UID uidGoods);

	virtual IGodSword * GetGodSword(UID uidGoods);

	virtual ITalisman * GetTalisman(UID uidGoods); 

	//创建招募角色
	virtual IActor *    CreateEmploy(const SBuild_Employee & Build_Employee);

	//创建招募角色
	virtual IActor *	CreateEmploy(TEmployeeID EmployeeID, IActor * pActor);

	//写系统邮件
	virtual void		WriteSystemMail(const SWriteSystemData & SystemMail);

	//写系统邮件
	virtual void		WriteSystemMail(const SWriteSystemData & SystemMail, std::vector<IGoods *> & vecGoods);

	//访问世界指定类事物
	virtual void		VisitWorldThing(enThing_Class cls, IVisitWorldThing & VisitThing);

	//全服公告
	virtual void		WorldNotice(IActor * pActor, const char * pszNoticeContext);

	//世界频道的系统消息 (pActor为0对世界所有玩家发送，不为0则表示在世界频道对个人发送系统消息)
	virtual void		WorldSystemMsg(const char * pszMsgContext, IActor * pActor = 0, enTalkMsgType enMsgType = enTalkMsgType_System);

	//删除场景
	virtual bool		DeleteGameScene(IGameScene* pGameScene);

	//删除场景
	virtual bool		DeleteGameScene(TSceneID SceneID);

	//弹出框提示
	virtual void		TipPopUpBox(IActor * pActor, enGameWorldRetCode GameWorldRetCode);

	//通过UserID获取人物
	virtual IActor *	GetUserByUserID(TUserID UserID);

	//给所有玩家写系统邮件
	virtual void		WriteAllUserSysMail(const SWriteSystemData & SystemMail);

	//记录仙石使用日志
	virtual void		Save_GodStoneLog(TUserID UserID,INT32 Num,INT32 Aftergodstone,char * pszDesc,enGodStoneChanel chanel = enGodStoneChanel_Game,enGodStoneType type = enGodStoneType_Use);

	//获取语言字串
	virtual const char * GetLanguageStr(INT32 LanguageID);	//fly add 20121106

	//收入无效场景中，等待回收
	virtual void		Push_InvalidScene(TSceneID SceneID);

	virtual void		OnTimer(UINT32 timerID);

private:
	IActor * CreateActor(TSceneID SceneID, const char * buf, int len, UINT32 flag);

	IMonster * CreateMonster(TSceneID SceneID, const char * buf, int len, UINT32 flag);

	//创建建筑
	IBuilding * CreateBuilding(TSceneID SceneID, const char * buf, int len,UINT32 flag=0);

	//创建物品
	IGoods * CreateGoods(TSceneID SceneID, const char * buf, int len,UINT32 flag=0);

	//传送门
	IPortal * CreatePortal(TSceneID SceneID, const char * buf, int len,UINT32 flag=0);

	IGoods * __CreateGoods(TGoodsID GoodsID);

	IBuilding * CreateBuilding(enBuildingType BuildingType);

	//写系统邮件的回调函数
	void	HandleWriteSystemMail(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
		OBuffer4k & ReqOb,UINT64 userdata);

	friend class GameScene;//让GameScene可以调,是否有更好的方法
	void AddGameScene(GameScene* pGameScene);	 
	void RemoveGameScene(IGameScene* pGameScene); 

	GameWorld::GameSceneMap::iterator	FindScene(TSceneID sid);

	TSceneID GetNextSceneID(const TMapID mapid);

private:
   ThingContainer m_ThingContainer;

   ActorBuilder   m_ActorBuilder;  //创建角色
   MonsterBuilder m_MonsterBuilder;  //创建怪物

   std::vector<TSceneID>	m_vecInvalidScene;	//无效的场景ID,等待垃圾回收
};


#endif
