#ifndef __THINGSERVER_CREATEEMPLOYEE_H__
#define __THINGSERVER_CREATEEMPLOYEE_H__

#include "DSystem.h"
#include "IGameWorld.h"

struct IActor;

class CreateEmployee
{
public:
	CreateEmployee(TEmployeeID EmployeeID, IActor * pMaster);
	~CreateEmployee();

	////////////////////////////////////////////////////////////////////////////////////////
	//招募角色的第一次创建,不用操作数据库的创建

	//创建招募角色,第一次创建不通过数据库
	IActor * CreateEmploy();

	//创建装备
	void	CreateEquipPanelData();
	//创建法术栏
	void	CreateMagicPanelData();
	//创建帮派技能栏
//	void	CreateSynMagicPanelData();
	//创建战斗数据
	void	CreateCombatData();
	//创建状态数据
	void	CreateStatusData();
	//创建冷却时间数据
	void	CreateCDTimerData();

	////////////////////////////////////////////////////////////////////////////////////////////
private:
	//检测是否可以创建招募角色，可以的话创建招募角色
	IActor * CheckBuildEployee();

private:
	SBuild_Employee m_BuildEmployee;

	TEmployeeID	 m_EmployeeID;

	//所属主人
	IActor *	 m_pMaster;
};

#endif
