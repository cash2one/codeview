
#ifndef __THINGSERVER_CHENGJIU_PART_H__
#define __THINGSERVER_CHENGJIU_PART_H__

#include "IChengJiuPart.h"
#include <hash_map>
#include "IConfigServer.h"

//单个成就的数据
struct SChengJiuData
{
	SChengJiuData() : m_ChengJiuID(INVALID_CHENGJIU_ID), m_Progress(0),m_FinishTime(0), m_bUpdate(false)
	{
	}

	TChengJiuID  m_ChengJiuID;  //成就ID
	UINT16       m_Progress;    //进度
	UINT32       m_FinishTime;  //达成时间，为零表示未达成

	bool		 m_bUpdate;		//是否有更新过,有更新过的才保存进数据库
};

struct IActor;

class ChengJiuPart : public IChengJiuPart
{
	
public:
	ChengJiuPart();
	virtual ~ChengJiuPart();


public:
	//推进成就进度
	virtual void AdvanceProgress(TChengJiuID  ChengJiuID );

	//复位成就进度
	virtual void ResetProgress(TChengJiuID  ChengJiuID );

	//是否已获得某成就
	virtual bool IsAttainChengJiu(TChengJiuID  ChengJiuID); 

		//打开成就栏
	virtual void OpenChengJiuPanel();

	//更换称号
	virtual void ChangeTitle(TTitleID  TitleID); 


	
	//////////////////////////////////////////////////////////////////////////
	// 描  述：创建部件
	// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
	// 返回值：返回TRUE表示创建成功
	//////////////////////////////////////////////////////////////////////////
	virtual bool Create(IThing *pMaster, void *pContext, int nLen);

	//释放
	virtual void Release(void);

	//取得部件ID
	virtual enThingPart GetPartID(void);

	//取得本身生物
	virtual IThing*		GetMaster(void);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：取得部件的数据库现场
	// 输  入：数据缓冲区buf，nLen为buf的大小
	// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
	// 备  注：用于将部件中的数据保存到数据库
	//////////////////////////////////////////////////////////////////////////
	virtual bool OnGetDBContext(void * buf, int &nLen);

	//////////////////////////////////////////////////////////////////////////
	// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
	// 输  入：
	// 备  注：生物创建后，会逐个部件调用此函数，
	//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
	//////////////////////////////////////////////////////////////////////////
	virtual void InitPrivateClient();
	

	//玩家下线了，需要关闭该ThingPart
	virtual void Close();

	//保存数据
	virtual void SaveData();

	//获得当前称号
	virtual std::string GetTitle();

	//得到当前称号ID
	virtual TTitleID	GetTitleID();

	//获取成就完成时间
	virtual UINT32		GetChengJiuFinishTime(TChengJiuID ChengJiuID);

	//发送自己的成就数据给别的玩家（成就对比用）
	virtual void		SendMeChengJiuToOtherUser(IActor * pRecvActor);

private:
	//增加成就
	bool	AddChengJiu(const SChengJiuCnfg * pChengJiuCnfg);

	//获得成就数据
	SChengJiuData * GetChengJiuData(TChengJiuID ChengJiuID);

	//通知更新成就
	void	NoticeUpdateChengJiu(TChengJiuID ChengJiuID);

	//自动获得的成就
	void	AutoGetChengJiu(const std::vector<TChengJiuID> & vectChengJiuID);

	//奖励
	void	Award(const SChengJiuCnfg * pChengJiuCnfg);


private:
	IActor *   m_pActor;
	INT32      m_ChengJiuPoint;  //获得的成就点
	INT32      m_GetChengJiuNum; //已获得成就数量
	TTitleID   m_ActiveTitleID;  //当前启用的称号ID

	bool	 m_bClientOpen;  //客户端是否打开过成就栏

	typedef std::hash_map<TChengJiuID,SChengJiuData> MAP_CHENGJIU;

	MAP_CHENGJIU  m_mapChengJiu;  //所有已完成或已有进度的成就
};





#endif
