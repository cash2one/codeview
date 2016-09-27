
#ifndef __THINGSERVER_BUILDING_H__
#define __THINGSERVER_BUILDING_H__

#include "IBuilding.h"
#include "ThingBase.h"

#include "DBProtocol.h"
#include "ITimeAxis.h"
struct IActor;
struct IResOutputPart;


//建筑数据
struct SBuildingData
{
	SBuildingData()
	{
		MEM_ZERO(this);
	}

	UINT8          m_BuildingType;       //建筑类型
	UID            m_uidOwner;           //建筑主人
	UINT32         m_LastTakeResTime;    //上次领取资源的时间	
	INT32          m_OwnerNoTakeResNum;  //主人未领取的资源数

	UINT32		m_LastCollectTime;		//上次代收时间
	UINT32		m_TotalCollectResOfDay;	//当天总共代收了的资源数
	bool		m_bAutoTakeRes;		//是否开启自动收取灵石
};

//资源建筑记录数据
struct ResBuildRecordInfo
{
	ResBuildRecordInfo()
	{
		MEM_ZERO(this);
	}

	bool	m_bInsert;
	UINT8	m_BuildingType;
	UINT32	m_HappenTime;
	char	m_szTargetName[THING_NAME_LEN];
	char	m_szHandleName[THING_NAME_LEN];
	char	m_szContext[DESCRIPT_LEN_50];
};


class Building : public  TThingBase<IBuilding>
{
public:
	Building();
	virtual ~Building();
public:
    virtual enBuildingType GetBuildingType();

		virtual TMsgSourceType GetEventSrcType(){ return enEventSrcType_Building;}

	virtual enThing_Class GetThingClass(void){ return enThing_Class_Building; }

			//初始化，
	virtual bool Create(void);

	virtual void Release();

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen);
	virtual bool OnSetPrivateContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	virtual void SetSceneID(TSceneID SceneID) ;

	 TSceneID GetSceneID() ;

private:
    enBuildingType  m_BuildingType;
	TSceneID        m_SceneID;

};


class ResOutputBuilding : public TThingBase<IResOutputBuilding>,
	         public ITimerSink
{
	enum enTIMER_ID
	{
		enTIMER_ID_Guard = 0,
		enTIMER_ID_AutoTakeRes,	//自动收取
	};
public:
   ResOutputBuilding();

   virtual ~ResOutputBuilding();

   virtual void OnTimer(UINT32 timerID);

    void SetSceneID(TSceneID SceneID) ;

	 TSceneID GetSceneID() ;


public:

		virtual TMsgSourceType GetEventSrcType(){ return enEventSrcType_Building;}

	virtual enThing_Class GetThingClass(void){ return enThing_Class_Building; }

			//初始化，
	virtual bool Create(void);

	virtual void Release();

	virtual enBuildingType GetBuildingType();

			//获得位置
	virtual XPoint GetLoc();


	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的公开现场
	// 输  入：保存公开现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：公开现场为可以让所有GameWorld中的其它对象查看的本实体对象信息，如生物名字等
	//         服务器取得对象现场，以dwThingClassID + uid + wDataLen + data[wDataLen] 的形式
	//         发给客户端，客户端根据dwThingClassID创建对象，调用OnSetContext()设置对象现场
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPublicContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：获得本实体对象的私有现场
	// 输  入：保存私有现场数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示成功，nLen为buf中数据的大小
	// 备  注：私有现场为实体对象的详细信息，
	//         供控制（或者拥有）此实体的客户端使用，客户端根据此信息创建相应的对象    
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetPrivateContext(void * buf, int &nLen);
	virtual bool OnSetPrivateContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将数据库保存的数据传给本实体
	// 输  入：数据库保存数据的缓冲区buf，nLen为buf中数据的大小
	// 返回值：返回TRUE表示设置数据成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnSetDBContext(const void * buf, int nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：将本实体的数据保存到数据库
	// 输  入：保存数据的缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：发送消息给本实体
	// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
	// 返回值：
	// 备  注：在此函数内，可能会发FireVote和FireAction
	//////////////////////////////////////////////////////////////////////////
	virtual void OnMessage( UINT32 dwMsg, const void * buf, int nLen);

	//设置自动收取灵石
	virtual void SetAutoTakeRes(bool bAutoTakeRes);

	virtual bool GetAutoTakeRes();

	//上线时，如果有自动收取灵石，则自动收取
	virtual void Online_AutoTake();
public:
  
   //上次领取时间
   UINT32 GetLastTakeResTime();

    //资源产出余下时间
   INT32 GetOutputRemainingTime();

   //领取资源
   INT32 TakeRes();

  
   //被pActor代收资源
   INT32 Collectioned(IActor* pActor,INT32 maxNum);

    //当天已代收资源数
  virtual   INT32 GetTotalCollectResOfDay();

  //增加当天代收资源数
  virtual  bool AddCollectRes(INT32 Num);


	//手动设置上次领取时间，用于资源快熟
	void SetLastTakeResTime(UINT32 nTime);

	 //记录别人对自己的操作
	void RecordHandleOtherUser(const char * pszTargetUserName, const char * pszHandleName, const char * pszContext);

	//保存记录
	void SaveBuildingRecordToDB(UID uidUser);

	//加进记录
	void Push(const BuildingRecordData & RecordData);
	
	//查看指定资源建筑记录
	void ViewBuildingRecord();

	//得到主人UID
	UID  GetMasterUID();

   private:
	   IResOutputBuilding * GetBuildingFromGameWorld(UID uidBuilding);

	   //设置自动收取灵石定时器
	   void	SetAutoTakeResTimer(bool bNeedResetTimer);

	   //收取灵石
	   void __TakeResource(INT32 SetTakeResNum = 0);

	   INT32 GetTakeResNum();


private:
	SBuildingData   m_BuildingData;
	TSceneID        m_SceneID;
	std::vector<ResBuildRecordInfo> m_vectBuildingRecordData;

	bool			m_bNeedResetTimer;	//是否需要再重新设定时器
	
};




#endif
