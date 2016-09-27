
#ifndef __BCL_IEVENT_SERVER_H__
#define __BCL_IEVENT_SERVER_H__

#include "BclHeader.h"


typedef UINT8   TMsgSourceType;  //消息源类型取值为enEventSrcType
typedef INT32   TMsgID;          //消息ID
typedef void *  TMsgSource;      //消息源

#define ANY_SOURCE (TMsgSource)0


//事件
struct XEventData
{
    TMsgSource        m_MsgSource;		// 事件源
    void    	      *m_pContext;		// 现场指针
    UINT32	          m_len;				// 现场大小
	TMsgID	          m_MsgID;				// 事件ID
    TMsgSourceType	  m_EventSrcType;		// 事件源类型ID
};




struct IEventListener;
struct IVoteListener;
struct IActionListener;
struct IEventSource;


// 事件服务器接口
struct IEventServer
{
public:
	// 释放事件服务器
	virtual void Release() = 0;

	//创建，SourceType[]指定事件源的继承关系
	virtual bool Create(TMsgSourceType SourceType[],int size)=0;

	// 触发事件

	//类型为srctype的消息源发出事件msgid，附带现场数据pContext,现场长度nLen
	virtual bool FireVote(TMsgSourceType srctype, TMsgSource MsgSource,TMsgID msgid, void *pContext=0, int nLen=0 ) = 0;
	virtual bool FireVote(IEventSource * pEventSource,TMsgID msgid, void *pContext=0, int nLen=0	 ) = 0;

	// 触发行为
	virtual void FireAction(TMsgSourceType srctype,TMsgSource MsgSource,TMsgID msgid, void *pContext=0, int nLen=0  ) = 0;	
	virtual void FireAction(IEventSource * pEventSource,TMsgID msgid, void *pContext=0, int nLen=0  ) = 0;

	// 触发响应
	virtual void FireResponse(TMsgSourceType srctype,TMsgSource MsgSource,TMsgID msgid, void *pContext=0, int nLen=0  ) = 0;
	virtual void FireResponse(IEventSource * pEventSource,TMsgID msgid, void *pContext=0, int nLen=0) = 0;


	//订阅事件

	// 描  述：增加Listener,订阅否决事件
	// 参  数：[输出] IVoteListener *pListener  - 事件否决者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 	
	// 参  数：[输出] const char *pDescription  - 订阅者描述信息,可以就填订阅者的类名	
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE
	virtual bool AddListener(IVoteListener *pListener,TMsgID msgid, TMsgSourceType srctype, TMsgSource MsgSource, const char *pDescription) = 0;
	virtual bool AddListener(IVoteListener *pListener,TMsgID msgid, IEventSource * pEventSource, const char *pDescription) = 0;

	// 描  述：增加Listener,订阅行为事件
	// 参  数：[输出] IActionListener *pListener  - 事件行为执行者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 	
	// 参  数：[输出] const char *pDescription  - 订阅者描述信息,可以就填订阅者的类名	
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE
	virtual bool AddListener(IActionListener *pListener,TMsgID msgid, TMsgSourceType srctype, TMsgSource MsgSource, const char *pDescription) = 0;
	virtual bool AddListener(IActionListener *pListener,TMsgID msgid, IEventSource * pEventSource, const char *pDescription) = 0;


	// 描  述：增加Listener,订阅响应事件
	// 参  数：[输出] IEventListener *pListener  - 事件响应者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 	
	// 参  数：[输出] const char *pDescription  - 订阅者描述信息,可以就填订阅者的类名	
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE
	virtual bool AddListener(IEventListener *pListener,TMsgID msgid, TMsgSourceType srctype,TMsgSource MsgSource, const char *pDescription) = 0;
	virtual bool AddListener(IEventListener *pListener,TMsgID msgid, IEventSource * pEventSource, const char *pDescription) = 0;



	 //移除事件

	// 描  述：删除Listener,取消订阅否决事件,不会delete pListener指针
	// 参  数：[输出] IVoteListener *pListener  - 事件否决者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 		
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE
	virtual bool RemoveListener(IVoteListener *pListener,TMsgID msgid, TMsgSourceType srctype, TMsgSource MsgSource) = 0;
	virtual bool RemoveListener(IVoteListener *pListener,TMsgID msgid, IEventSource * pEventSource) = 0;


	// 描  述：删除Listener,取消订阅行为事件,不会delete pListener指针
	// 参  数：[输出] IActionListener *pListener  - 事件行为执行者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 		
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE
	virtual bool RemoveListener(IActionListener *pListener,TMsgID msgid, TMsgSourceType srctype, TMsgSource MsgSource) = 0;
	virtual bool RemoveListener(IActionListener *pListener,TMsgID msgid, IEventSource * pEventSource) = 0;

	// 描  述：删除Listener,取消订阅响应事件,不会delete pListener指针
	// 参  数：[输出] IEventListener *pListener  - 事件响应者	
	// 参  数：[输出] MsgID msgid  - 事件ID
	// 参  数：[输出] MsgSourceID sourceid  - 事件源指针 		
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE
	virtual bool RemoveListener(IEventListener *pListener,TMsgID msgid, TMsgSourceType srctype,  TMsgSource MsgSource) = 0;
	virtual bool RemoveListener(IEventListener *pListener,TMsgID msgid, IEventSource * pEventSource) = 0;

	//删除所有以这个对象为处理器的注册器，方便在处理器销毁的时候，一次性取消所有消息的回调
	virtual	void	RemoveAllListener(IActionListener * pActionListener)=0;

	//删除所有以这个对象为处理器的注册器，方便在处理器销毁的时候，一次性取消所有消息的回调
	virtual	void	RemoveAllListener(IEventListener * pResponseListener)=0;

	//删除所有以这个对象为处理器的注册器，方便在处理器销毁的时候，一次性取消所有消息的回调
	virtual	void	RemoveAllListener(IVoteListener* pVoteListener)=0;

	//移除事件源下的所有处理器
	virtual void  RemoveSourceAllListenter(TMsgSource MsgSource) = 0;
	
};



BCL_API IEventServer* GetEventServer();


//投票监听者
struct  IVoteListener
{
	virtual ~IVoteListener(){GetEventServer()->RemoveAllListener(this);}
	virtual bool OnVote(XEventData & EventData)=0 ;

};

// 行为执行者监听者接口
struct  IActionListener
{	
	virtual ~IActionListener(){GetEventServer()->RemoveAllListener(this);}

	virtual void OnAction(XEventData & EventData)=0;
};

//事件监听者
struct  IEventListener
{
	virtual~IEventListener(){GetEventServer()->RemoveAllListener(this);}
	virtual void OnEvent(XEventData & EventData)=0; //需要实现该接口
};


//事件源
struct IEventSource
{
	virtual TMsgSourceType GetEventSrcType()=0;
	// 描  述: 订阅本对象的否决事件
	// 参  数: [输入] UINT32 dwMsg - 事件ID
	// 参  数: [输入] IVoteListener * pListener - 事件否决者
	// 参  数: [输入] const char szDescription[32]  - 订阅者描述信息,可以就填订阅者的类名, 已便调试
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE
	virtual bool SubscribeVote(UINT32 dwMsg, IVoteListener * pListener, const char* szDescription)
	{
		return GetEventServer()->AddListener(pListener,dwMsg,this,szDescription);
	}

	// 描  述：取消订阅本对象的否决事件,不会delete pListener指针
	// 参  数：[输入] UINT32 dwMsg  - 事件ID
	// 参  数：[输入] IVoteListener * pListener  - 事件否决者	
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE退订消息
	virtual bool UnsubscribeVote(UINT32 dwMsg, IVoteListener * pListener)
	{
		return GetEventServer()->RemoveListener(pListener,dwMsg,this);
	}

	// 描  述: 订阅本对象的行为事件
	// 参  数: [输入] UINT32 dwMsg - 事件ID
	// 参  数: [输入] IActionListener * pListener - 事件行为执行者
	// 参  数: [输入] const char szDescription[32]  - 订阅者描述信息,可以就填订阅者的类名, 已便调试
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE	
	virtual bool SubscribeAction(UINT32 dwMsg, IActionListener * pListener, const char* szDescription)
	{
		return GetEventServer()->AddListener(pListener,dwMsg,this,szDescription);
	}
	
	// 描  述：取消订阅本对象的行为事件,不会delete pListener指针
	// 参  数：[输入] UINT32 dwMsg  - 事件ID
	// 参  数：[输入] IActionListener * pListener  - 事件行为执行者
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE退订消息	
	virtual bool UnsubscribeAction(UINT32 dwMsg, IActionListener * pListener)
	{
		return GetEventServer()->RemoveListener(pListener,dwMsg,this);
	}

	// 描  述: 订阅本对象的响应事件
	// 参  数: [输入] UINT32 dwMsg - 事件ID
	// 参  数: [输入] IEventListener * pListener - 事件响应者
	// 参  数: [输入] const char szDescription[32]  - 订阅者描述信息,可以就填订阅者的类名, 已便调试
	// 返回值：BOOL - 订阅成功，则返回TRUE,否则返回FALSE	
	virtual bool SubscribeEvent(UINT32 dwMsg, IEventListener * pListener, const char * szDescription)
	{
		return GetEventServer()->AddListener(pListener,dwMsg,this,szDescription);
	}
	
	// 描  述：取消订阅本对象的行为事件,不会delete pListener指针
	// 参  数：[输入] UINT32 dwMsg  - 事件ID
	// 参  数：[输入] IEventListener * pListener  - 事件响应者
	// 返回值：BOOL - 取消订阅成功，则返回TRUE,否则返回FALSE退订消息	
	virtual bool UnsubscribeEvent(UINT32 dwMsg, IEventListener * pListener)
	{
		return GetEventServer()->RemoveListener(pListener,dwMsg,this);
	}

	//取消所有注册

	//////////////////////////////////////////////////////////////////////////
	// 描  述：发送响应给本实体
	// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
	// 备  注：在此函数内，可能会发FireResponse
	//////////////////////////////////////////////////////////////////////////
	virtual void OnEvent(UINT32 dwMsg,  void * pContext, int nLen)
	{
		GetEventServer()->FireResponse(this,dwMsg,pContext,nLen);
	}

	//触发投票
	virtual bool FireVote(UINT32 dwMsg,  void * pContext, UINT32 nLen)
	{
		return GetEventServer()->FireVote(this,dwMsg,pContext,nLen);
	}

	//触发执行
	virtual void OnAction(UINT32 dwMsg,  void * pContext, UINT32 nLen)
	{
		GetEventServer()->FireAction(this,dwMsg,pContext,nLen);
	}

	virtual ~IEventSource()
	{
		IEventServer * pEventServer = GetEventServer();
		if(pEventServer)
		{
			pEventServer->RemoveSourceAllListenter(this);
		}
	}

};


#endif

