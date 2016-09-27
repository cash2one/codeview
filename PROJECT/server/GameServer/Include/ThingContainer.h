

#ifndef __THINGSERVER_THING_CONTAINER_H__
#define __THINGSERVER_THING_CONTAINER_H__

#include <vector>
#include "IThing.h"
#include "IGameWorld.h"
#include "IVisitWorldThing.h"
#include "IActor.h"


//用来放置所有的游戏对象
class	ThingContainer
{
	//所有游戏对象的Map
	typedef	std::hash_map<UID, IThing*,std::hash<UID>, std::equal_to<UID> >	THING_MAP;
	
	typedef	std::vector<THING_MAP>				THING_TYPE_MAP;

	THING_TYPE_MAP	m_emap;

public:
	ThingContainer()
	{
		for(int i=0; i<enThing_Class_Max; i++)
		{
			m_emap.push_back(THING_MAP());
		}
		//m_emap.resize(enThing_Class_Max);
	}

	bool		AddThing(IThing * pthing)
	{
		if(pthing==0)
		{
			return false;
		}
		enThing_Class thingclass = pthing->GetThingClass();
		UID uid = pthing->GetUID();
		m_emap[thingclass][uid]		= pthing;	 
		return true;
	}


	void		RemoveThing(const UID & uid)
	{			
		if(!uid.IsValid())
		{
			return ;
		}
		
		m_emap[uid.m_thingClass].erase(uid);
	}

	void		RemoveThing(IThing * pThing)
	{			
		if(pThing == 0)
		{
			return;
		}
		RemoveThing(pThing->GetUID());
	}

	//通过UID查询对象
	IThing*	GetThing(const UID & uid) const
	{
		if(!uid.IsValid() || uid.m_thingClass >= enThing_Class_Max)
		{
			return 0;
		}

		const THING_MAP & ThingMap = m_emap[uid.m_thingClass];
			
		THING_MAP::const_iterator it =	ThingMap.find(uid);

		if (it == m_emap[uid.m_thingClass].end())
		{
			return 0;
		}

		return it->second;
	}

	//通过UserID获取人物
	IActor * GetUserByUserID(TUserID userID) const
	{
		const THING_MAP & ActorMap = m_emap[enThing_Class_Actor];

		THING_MAP::const_iterator iter = ActorMap.begin();

		for( ; iter != ActorMap.end(); ++iter)
		{
			IActor * pActor = (IActor *)(iter->second);
			if( 0 == pActor){
				continue;
			}

			if( userID == pActor->GetCrtProp(enCrtProp_ActorUserID)){
				return pActor;
			}
		}

		return 0;
	}


	//返回指定类型的实体数量
	int	GetThingCount(enThing_Class cls) { return m_emap[cls].size();}

	//访问世界指定类事物
	void Visit(enThing_Class cls, IVisitWorldThing & VisitThing)
	{
		THING_MAP & emap = m_emap[cls];

		for(THING_MAP::iterator it = emap.begin(); it != emap.end(); )
		{
			IThing* pThing = it->second;
			++it;//提前++， 防止遍历器失效
			VisitThing.Visit(pThing);
		}
	}

	//移除指定类事物
	void RemoveThingClass(enThing_Class cls)
	{
		THING_MAP & emap = m_emap[cls];

		emap.clear();
	}


	///通过这个访问者，提供遍历等扩展操作
	template <class Visitor> void Visit(Visitor & v);
	template <class Visitor> void Visit(enThing_Class cls, Visitor & v);
};

template	<class Visitor>
void	ThingContainer::Visit(enThing_Class cls,Visitor & v)
{
	THING_MAP & emap = m_emap[cls];

	for(THING_MAP::iterator it = emap.begin(); it != emap.end(); )
	{
		IThing* pThing = it->second;
		++it;//提前++， 防止遍历器失效
		v(pThing);
	}
}

template	<class Visitor>
void	ThingContainer::Visit(Visitor & v)
{
	for(int k =0; k< enThing_Class_Max; ++k)
	{
		Visit((enThing_Class)k, v);
	}
}







#endif


