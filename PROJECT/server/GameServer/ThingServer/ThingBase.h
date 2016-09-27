
#ifndef __THINGSERVER_THINGBASE_H__
#define __THINGSERVER_THINGBASE_H__

#include "DSystem.h"
#include "IThing.h"
#include "DMsgSubAction.h"

template <typename IBase = IThing>
class TThingBase : public IBase
{
public:
	TThingBase()
	{
		m_szName[0] = 0;
	}

	virtual ~TThingBase(){}

	//设置本实体对象的UID
	virtual void SetUID(UID uid)
	{
		m_Uid = uid;
	}

	//取得本实体对象的UID
	virtual UID GetUID(void)
	{
		return m_Uid;
	}

		//取得本实体对象的名字
	virtual const char * GetName(void)
	{
		return m_szName;
	}

		//////////////////////////////////////////////////////////////////////////
	// 描  述：发送消息给本实体
	// 输  入：消息码dwMsg，数据缓冲区buf，数据大小nLen
	// 返回值：
	// 备  注：在此函数内，可能会发FireVote和FireAction
	//////////////////////////////////////////////////////////////////////////
	virtual void OnMessage( UINT32 dwMsg, const void * buf, int nLen)
	{
	}

    virtual XPoint GetLoc()
	{
		return XPoint();
	}

	virtual TMsgSourceType GetEventSrcType()
	{
		return enEventSrcType_Thing;
	}

		//设置是否需要存盘
	void SetSaveFlag(bool bSave) {}

	virtual bool GetSaveFlag()
	{
		return false;
	}

protected:
	void SetName(const char * szName)
	{
		if(szName != 0)
		{
			strncpy(m_szName,szName,THING_NAME_LEN);
		}
	}

protected:
	UID   m_Uid;
    char  m_szName[THING_NAME_LEN];

};


#endif
