#ifndef __RELATIONSERVER_IVISITSYNMEMBER_H__
#define __RELATIONSERVER_IVISITSYNMEMBER_H__

struct IThing;

struct IVisitSynMember
{
	//遍历帮派成员，对所有帮派成员调用此函数
	virtual void VisitMember(IThing * pThing) = 0;
};

#endif
