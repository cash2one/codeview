
#ifndef __BCL_UNIQUEID_GENERATOR_H__
#define __BCL_UNIQUEID_GENERATOR_H__

#include "BclHeader.h"
#include "stdio.h"


#define OBJ_SN_MAX 262143

struct UID { //tolua_export
	union
	{
		struct 
		{	
			UINT64 m_worldID:9;			//[0,511]		游戏服ID,一个游戏服里面会有多个服务器serverID,	
			UINT64 m_serverID:4;			//[0,15]		场景服ID
			UINT64 m_thingClass : 4;        //[0,15]    thingclass
			UINT64 m_objectSN:18;			//[0,262143]	在一个游戏场景中，每个实体的(m_objectSN+m_passTime)是唯一的
			UINT64 m_passTime:29;			//[0,536870911]	从2012-1-1 00:00 到现在经过的秒数
		};

		UINT64 m_uid;
	};

//tolua_begin
	UID()
	{
		m_uid  = UINT64(0);
	}

	//推荐使用这个函数来判断uid是否有效，不要使用 == INVALD_UID，这样容易扩展
	bool	IsValid()const
	{
		return m_uid != UINT64(-1) && m_uid !=0;
	}

	bool operator == (const UID & uid) const
	{
		return uid.m_uid == m_uid;
	}
//tolua_end
	UINT64  ToUint64() const {return m_uid;} //显式转换

	explicit UID(UINT64 uid) { m_uid = uid; } //显式构造

	bool operator != (const UID & uid) const
	{
		return !(uid.m_uid == m_uid);
	}
//tolua_begin
	bool operator < (const UID & uid) const
	{
		return m_uid < uid.m_uid;
	}

	//不要轻易改变这个函数的实现,涉及到uid的hash函数
	char*	ToString()const
	{
		static char buf[128];
		sprintf_s(buf, sizeof(buf),"%lld",m_uid);
		return buf;
	}

	void	FromString(const char * str)
	{
		if(str)
		{
			return;
		}
		m_uid = sscanf_s(str,"%lld",&m_uid);
	}

};
//tolua_end



//UID生成器类
class BCL_API UniqueIDGenerator
{
public:
	UniqueIDGenerator();

	//产生一个UID
	UID GenerateUID(UINT8 thingClass);

	//初始化,设置开始时间,服务端ID,游戏世界ID
	void Init(UINT8 serverID, UINT16 worldID);

private:
	void recalcPass();		//重新计算过去多少秒

private:	
	time_t   m_time;			//begin计算出来的秒数
	time_t   m_pass;			//从m_time到现在经过的秒数
	UINT32	 m_sid;			//
	UINT8	 m_serverID;		//游戏服ID
	UINT16   m_worldID;		//游戏世界ID
};	




//因为跨服的需求，所有的ID都必须是全国唯一的
//UID生成服务
class BCL_API UniqueIDGeneratorService
{
public:
	//设置开始时间,服务端ID,游戏世界ID
	static void Init(UINT8 serverID, UINT16 worldID);

	//产生一个UID
	static UID GenerateUID(UINT8 thingClass);

	
private:
	static UniqueIDGenerator m_uidGenerator; //对象ID生成

};



#endif

