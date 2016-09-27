#ifndef __THINGSERVER_DUOBAOWAR_H__
#define __THINGSERVER_DUOBAOWAR_H__

#include <vector>
#include "ITimeAxis.h"
#include <set>
#include "UniqueIDGenerator.h"
#include <map>
#include "DSystem.h"

class DuoBaoGroup;
class PairGroup;

struct IActor;


class DuoBaoWar : public ITimerSink
{
	enum enDuoBaoWarTimer
	{
		enDuoBaoWarTimer_Begin = 0,		//开启夺宝定时器
		enDuoBaoWarTimer_Over,			//结束夺宝定时器
		enDuoBaoWarTimer_Pair,			//配对定时器
	};
public:
	DuoBaoWar();

	~DuoBaoWar();

	bool Create();

	virtual void OnTimer(UINT32 timerID);

	//得到玩家所属的等级组下标，-1表示没有
	int	 GetGroupUserLvIndex(UINT8 userLevel);

	//加入夺宝
	void JoinDuoBao(IActor * pActor);

	//点击战斗准备框按钮
	void ReadySelect(IActor *pActor, bool bEnterCombat);

	//退出或掉线或顶号或离队
	void	QuitDuoBao(IActor * pQuiter);

	//创建一个配对成功组
	PairGroup * GetPairGroup();

	//配对组的使命结束，放入空间配对组池,供下次使用
	void	InValid_PairGroup(TGroupID GroupID);

	//是否开启
	bool	IsOpen();

private:
	//设置结束定时器
	void SetOverTimer();

	//开启夺宝
	void StartDuoBao();

	//结束夺宝
	void OverDuoBao();

private:

	//是否开启
	bool								m_bOpen;

	//单人夺宝等级组
	std::vector<DuoBaoGroup *>			m_vecSingleGroup;

	//组队夺宝等级组
	std::vector<DuoBaoGroup *>			m_vecTeamGroup;

	//配对成功组
	std::map<TGroupID, PairGroup*>		m_mapPairGroup;

	//空闲配对组池，存放空闲的配对组
	std::vector<PairGroup*>				m_vecInValidPairGroup;
};


#endif
