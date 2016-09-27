
#ifndef __THINGSERVER_PACKET_MGR_H__
#define __THINGSERVER_PACKET_MGR_H__

#include "TBuffer.h"
#include "IMessageDispatch.h"

struct IActor;

class PacketMgr : public IMsgRootDispatchSink
{
public:
   PacketMgr();

   virtual ~PacketMgr();

   bool Create();

   void Close();

public:
	//收到MSG_ROOT消息
	virtual void OnRecv(IActor *pActor, UINT8 nCmd,IBuffer & ib); 

private:
	//增加物品
	void AddGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);


	//移除移品
	void RemoveGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//扩充容量
	void ExtendCapacity(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//使用物品
	void UseGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//丢弃物品
	void DiscardGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//装备
	void Equip(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//合成
	void Compose(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//角色更名卡确认使用
	void UseActorChangeName(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//使用传音符
	void UseChangYinFu(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//物品出售
	void SellGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//确认使用物品
	void ConfirmationUseGoods(IActor *pActor,UINT8 nCmd, IBuffer & ib);

	//点击人物面板的资质按键
	void ClickAptitude(IActor *pActor,UINT8 nCmd, IBuffer & ib);
};






#endif

