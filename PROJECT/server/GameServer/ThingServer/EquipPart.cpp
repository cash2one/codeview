
#include "IActor.h"

#include "EquipPart.h"
#include "IEquipment.h"
#include "DBProtocol.h"
#include "IGameServer.h"
#include "ThingServer.h"
#include "IBasicService.h"
#include "IGameWorld.h"

#include "IConfigServer.h"
#include "IGodSword.h"
#include "ITalisman.h"
#include "IPacketPart.h"
#include "StringUtil.h"

EquipPart::EquipPart()
{
	for(int i=0; i<ARRAY_SIZE(m_uidEquips);++i)
	{
		m_uidEquips[i] = UID();
	}

	m_pActor = 0;

	m_GoodsNum = 0;
}

EquipPart::~EquipPart()
{
	for( int i = 0; i < enEquipPos_Max; ++i)
	{
		if( !m_uidEquips[i].IsValid()){
			continue;
		}

		g_pGameServer->GetGameWorld()->DestroyThing(m_uidEquips[i]);
	}
}



//////////////////////////////////////////////////////////////////////////
// 描  述：创建部件
// 输  入：实体pMaster，数据缓冲区pContext，nLen为buf中数据的大小
// 返回值：返回TRUE表示创建成功
//////////////////////////////////////////////////////////////////////////
bool EquipPart::Create(IThing *pMaster, void *pContext, int nLen)
{
	if(pMaster==0 || pContext==0 || nLen<sizeof(SDBEquipPanel))
	{
		return false;
	}
	m_pActor = (IActor*)pMaster;

	SDBEquipPanel * pDBEquipPanel = (SDBEquipPanel*)pContext;

	if(pDBEquipPanel->m_szFacade[0]==0)
	{
		m_vectFacade.push_back(m_pActor->GetCrtProp(enCrtProp_ActorFacade));
	}
	else
	{
		StringUtil::StrToNumber(std::string(pDBEquipPanel->m_szFacade),m_vectFacade);		
	}

	bool bSave = false;

	for(int i=0; i<ARRAY_SIZE(pDBEquipPanel->m_GoodsData); ++i)
	{
		if(pDBEquipPanel->m_GoodsData[i].m_Location >=ARRAY_SIZE(pDBEquipPanel->m_GoodsData) )
		{
			continue;
			TRACE("<error> %s %d line 错误的装备位置 %d",__FUNCTION__,__LINE__,pDBEquipPanel->m_GoodsData[i].m_Location );
		}

		if(pDBEquipPanel->m_GoodsData[i].m_uidGoods.IsValid()==false)
		{
			continue;
		}

		m_setEquipCompare.insert(UID(pDBEquipPanel->m_GoodsData[i].m_uidGoods));

		IGoods * pGoods = (IGoods*)g_pGameServer->GetGameWorld()->CreateThing(enThing_Class_Goods,INVALID_SCENE_ID,(char*)&pDBEquipPanel->m_GoodsData[i],sizeof(pDBEquipPanel->m_GoodsData[i]),THING_CREATE_FLAG_DB_DATA);

		if(0 == pGoods)
		{
			TRACE("<error> %s : %d line 创建物品失败, goodsid = %d", __FUNCTION__,__LINE__,pDBEquipPanel->m_GoodsData[i].m_GoodsID);
			continue;
		}

		//设置成不需要保存
		pGoods->SetUpdate(false);



		if(__AddEquip(pGoods->GetUID(),pDBEquipPanel->m_GoodsData[i].m_Location, true)==false)
		{
			//由于配置数据修改，有可能装不上去，把物品放回背包
			IPacketPart * pPacketPart = m_pActor->GetPacketPart();
			if(pPacketPart==0)
			{
				TRACE("<error> %s : %d line 玩家[%s] pPacketPart==0, goodsid = %d", __FUNCTION__,__LINE__,m_pActor->GetName(),pDBEquipPanel->m_GoodsData[i].m_GoodsID);
			}
			else
			{
				if(pPacketPart->AddGoods(pGoods->GetUID())==false)
				{
					bSave = true;

					//发邮件
					SWriteSystemData SysMailData;

					//主角
					if(m_pActor->GetMaster())
					{
						SysMailData.m_DestUID = m_pActor->GetMaster()->GetUID();
					}
					else
					{
						SysMailData.m_DestUID = m_pActor->GetUID();
					}

					sprintf(SysMailData.m_szThemeText,g_pGameServer->GetGameWorld()->GetLanguageStr(10079));

					sprintf(SysMailData.m_szContentText,g_pGameServer->GetGameWorld()->GetLanguageStr(10080));

					std::vector<IGoods *> vecGoods;

					vecGoods.push_back(pGoods);

					g_pGameServer->GetGameWorld()->WriteSystemMail(SysMailData, vecGoods);


				}
			}

		}


	}

	const std::vector<INT16> & vectAptitude = g_pGameServer->GetConfigServer()->GetGameConfigParam().m_ServerConfigParam.m_vectAptitudeFacade;

	//默认外观
	UINT16 DefaultFacadeID = GetDefaultFacade();

	//资质
	INT32 AptitudeValue = m_pActor->GetCrtProp(enCrtProp_ActorAptitude);

	for(int i=0;i+1<vectAptitude.size();i += 2)
	{
		if(AptitudeValue >= vectAptitude[i])
		{

			INT16 SuitID = vectAptitude[i+1];
			UINT16 FacadeID = g_pGameServer->GetConfigServer()->GetUpgradeFacadeID(SuitID,DefaultFacadeID);
			if(FacadeID != 0)
			{
				AddFacade(FacadeID);

			}
		}
		else
		{
			break;
		}
	}


	UINT16 FacadeID = m_pActor->GetCrtProp(enCrtProp_ActorFacade);

	std::vector<UINT16>::iterator it = std::find(m_vectFacade.begin(),m_vectFacade.end(),FacadeID);

	if(it == m_vectFacade.end())
	{
		this->SetShowFacade(m_vectFacade[0]);
	}

	if ( bSave )
	{
		this->SaveData();
	}


	return true;
}

//释放
void EquipPart::Release(void)
{
	delete this;
}

//取得部件ID
enThingPart EquipPart::GetPartID(void)
{
	return enThingPart_Actor_Equip;
}

//取得本身生物
IThing*		EquipPart::GetMaster(void)
{
	return m_pActor;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：取得部件的数据库现场
// 输  入：数据缓冲区buf，nLen为buf的大小
// 返回值：返回TRUE表示获得数据成功，nLen为buf中数据的大小
// 备  注：用于将部件中的数据保存到数据库
//////////////////////////////////////////////////////////////////////////
bool EquipPart::OnGetDBContext(void * buf, int &nLen)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// 描  述：初始化客户端，将私有现场发送给客户端，适合在玩家自己的客户端上使用
// 输  入：
// 备  注：生物创建后，会逐个部件调用此函数，
//         需要给客户端发送初始化数据的部件，在这个函数中实现发送数据的方法
//////////////////////////////////////////////////////////////////////////
void EquipPart::InitPrivateClient()
{
	SC_EquipPanel_Sync Rsp;
	Rsp.m_EquipNum = 0;
	Rsp.m_uidActor = m_pActor->GetUID();



	OBuffer1k ob2;
	for(int i=0; i<ARRAY_SIZE(m_uidEquips);++i)
	{
		if(m_uidEquips[i].IsValid())
		{
			//			m_pActor->NoticClientCreatePublicThing(m_uidEquips[i]);
			m_pActor->NoticClientCreatePrivateThing(m_uidEquips[i]);

			EquipInfo Info;
			Info.m_uidEquip = m_uidEquips[i];
			Info.m_pos = i;
			ob2 << Info;

			Rsp.m_EquipNum++;
		}
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_SyncEquip,sizeof(Rsp)+ob2.Size()) << Rsp << ob2;
	m_pActor->SendData(ob.TakeOsb());
}


//玩家下线了，需要关闭该ThingPart
void EquipPart::Close()
{

}

//保存数据
void EquipPart::SaveData()
{
	//保存外观数据
	SDB_Update_EquipPanelData_Req Req;
	Req.Uid_User = m_pActor->GetUID().ToUint64();
	std::string strFacade ;
	StringUtil::NumberToStr(this->m_vectFacade,strFacade);
	strncpy(Req.m_szFacade,strFacade.c_str(),sizeof(Req.m_szFacade));

	OBuffer1k ob;
	ob << Req;

	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_UpdateEquipInfo,ob.TakeOsb(),0,0);		



	for( int i = 0; i < enEquipPos_Max; ++i)
	{
		if( !m_uidEquips[i].IsValid()){
			continue;
		}

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(m_uidEquips[i]);
		if( 0 == pGoods){
			continue;
		}

		SET_EQUIPCOMPARE::iterator iter = m_setEquipCompare.find(m_uidEquips[i]);
		if( iter == m_setEquipCompare.end()){
			//新增加的装备,要入库
			this->SaveGoodsToDB(pGoods, i);
			continue;
		}

		if( pGoods->GetNeedUpdate()){
			//属性有改动,要入库
			this->SaveGoodsToDB(pGoods, i);
		}

		m_setEquipCompare.erase(iter);
	}

	SET_EQUIPCOMPARE::iterator it = m_setEquipCompare.begin();
	for( ; it != m_setEquipCompare.end(); ++it)
	{
		//剩下的删除
		SDB_Delete_GoodsReq DeleteEquip;
		DeleteEquip.m_uidUser = m_pActor->GetUID().ToUint64();
		DeleteEquip.m_uidGoods = (*it).ToUint64();
		DeleteEquip.m_Pos	  = 1;

		OBuffer1k ob;
		ob << DeleteEquip;
		g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteGoods,ob.TakeOsb(),0,0);		
	}

	m_setEquipCompare.clear();

	for( int i = 0; i < enEquipPos_Max; ++i)
	{
		if( !m_uidEquips[i].IsValid()){
			continue;
		}

		m_setEquipCompare.insert(m_uidEquips[i]);
	}
}

//激活套装
void  EquipPart::ActiveSuit(IEquipment * pAddEquip, UINT16 SuitID,UINT8 EquipNum)
{
	if( EquipNum < 2)
	{
		return;
	}

	if( EquipNum == 2){
		pAddEquip->ActiveSuitProp(m_pActor,enGoodsProp_ActiveSuit1);
	}else if( EquipNum == 4){
		pAddEquip->ActiveSuitProp(m_pActor,enGoodsProp_ActiveSuit2);
	}

	for(int i=0; i < MAX_LOAD_EQUIPMENT_NUM; i++)
	{
		IEquipment * pEquipmentTemp = GetEquipByPos(i);
		if(pEquipmentTemp==0)
		{
			continue;
		}

		if(pEquipmentTemp->GetGoodsCnfg()->m_SuitIDOrSwordSecretID == SuitID)
		{
			int bAdd = true;

			if( EquipNum < 4){
				//2到3件只激活套装属性1
				if( !pEquipmentTemp->GetPropNum(enGoodsProp_ActiveSuit1, bAdd)){
					return;
				}

				if( !bAdd){
					pEquipmentTemp->SetPropNum(m_pActor,enGoodsProp_ActiveSuit1,true);
				}
			}else if( EquipNum <= MAX_LOAD_EQUIPMENT_NUM){
				//激活套装属性1和套装属性2
				if( !pEquipmentTemp->GetPropNum(enGoodsProp_ActiveSuit1, bAdd)){
					return;
				}

				if( !bAdd){
					pEquipmentTemp->SetPropNum(m_pActor,enGoodsProp_ActiveSuit1,true);
				}

				if( !pEquipmentTemp->GetPropNum(enGoodsProp_ActiveSuit2, bAdd)){
					return;
				}

				if( !bAdd){
					pEquipmentTemp->SetPropNum(m_pActor,enGoodsProp_ActiveSuit2,true);					
				}
			}
		}
	}	
}

//不激活套装
void  EquipPart::InActiveSuit(IEquipment * pRemoveEquip, UINT16 SuitID,UINT8 EquipNum)
{
	if( EquipNum == 1){
		//从2件到1件时，移除套装属性1
		pRemoveEquip->InActiveSuitProp(m_pActor,enGoodsProp_ActiveSuit1);
	}else if( EquipNum == 3){
		//降到3件时，移除套装属性2
		pRemoveEquip->InActiveSuitProp(m_pActor,enGoodsProp_ActiveSuit2);
	}

	if( EquipNum < 4){
		pRemoveEquip->SetPropNum(m_pActor, enGoodsProp_ActiveSuit2, false);

	}

	if( EquipNum < 2){
		pRemoveEquip->SetPropNum(m_pActor, enGoodsProp_ActiveSuit1, false);	
	}

	if( EquipNum % 2 == 0)
	{
		return;
	}

	enGoodsProp PropID = enGoodsProp_ActiveSuit2;
	if(EquipNum < 2)
	{
		PropID = enGoodsProp_ActiveSuit1;
	}

	for(int i=0; i < MAX_LOAD_EQUIPMENT_NUM; i++)
	{
		IEquipment * pEquipmentTemp = GetEquipByPos(i);
		if(pEquipmentTemp==0)
		{
			continue;
		}
		if(pEquipmentTemp->GetGoodsCnfg()->m_SuitIDOrSwordSecretID == SuitID)
		{
			pEquipmentTemp->SetPropNum(m_pActor,PropID,false);
		}
	}	
}

//增加装备
bool EquipPart::__AddEquip(UID uidEquip,UINT8 pos, bool bCreateActor)
{
	if(pos>=enEquipPos_Max)
	{
		return false;
	}


	if(m_uidEquips[pos].IsValid())
	{
		return false;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(uidEquip);

	if(pGoods==0)
	{
		return false;
	}

	std::string strErr;

	UINT8 Category = pGoods->GetGoodsClass();

	if(Category==enGoodsCategory_Equip
		|| Category == enGoodsCategory_GodSword
		|| Category == enGoodsCategory_Talisman)
	{


		//增加人物属性
		IEquipment * pEquipment = (IEquipment *)pGoods;

		if(pEquipment->CanEquip(m_pActor,pos,strErr)==false)
		{
			return false;
		}

		m_uidEquips[pos] = uidEquip;  //需要先装上去，不然OnEquip计算属性时找不到该仙剑

		m_GoodsNum++;

		if(pEquipment->OnEquip(m_pActor)==false)
		{
			TRACE("<error> %s : %d line 穿载装备失败! GoodsID = %d",__FUNCTION__,__LINE__,pEquipment->GetGoodsID());
			return false;
		}

		//是否装备即邦定
		if(pEquipment->GetGoodsCnfg()->m_BindType == enBindType_Equip)
		{
			if(pEquipment->IsBinded()==false)
			{
				pEquipment->SetPropNum(m_pActor,enGoodsProp_Bind,true);
			}
		}

		//判断套装属性
		if(Category==enGoodsCategory_Equip)
		{
			UINT16 SuitID = pEquipment->GetGoodsCnfg()->m_SuitIDOrSwordSecretID;

			if(SuitID != 0)
			{				
				UINT8 & EquipNum = m_mapSuit[SuitID];

				EquipNum++;

				ActiveSuit(pEquipment, SuitID,EquipNum);

				const SSuitCnfg* pSuitCnfg = g_pGameServer->GetConfigServer()->GetSuitCnfg(SuitID);

				if(pSuitCnfg == 0)
				{
					TRACE("<error>  %s : %d line 找不到套装配置信息 SuitID = %d ", __FUNCTION__,__LINE__,SuitID);

				}
				else
				{
					if(EquipNum == pSuitCnfg->m_SuitTotalNum)
					{
						//获得外观
						UINT16 FacadeID = g_pGameServer->GetConfigServer()->GetUpgradeFacadeID(SuitID,this->GetDefaultFacade());
						if(FacadeID != 0)
						{
							this->AddFacade(FacadeID);
						}
					}
				}

			}
		}

		//设置成要更新
		pGoods->SetUpdate(true);

		if (!bCreateActor){

			this->SaveGoodsToDB(pGoods, pos);
		}
	}	
	else
	{
		return false;
	}

	return true;
}

//增加装备
bool EquipPart::AddEquip(UID uidEquip,UINT8 pos)
{
	if(__AddEquip(uidEquip,pos) == false)
	{
		return false;
	}


	//通知客户端

	SC_AddEquip_Rsp Rsp;
	Rsp.m_uidActor = m_pActor->GetUID(); //角色
	Rsp.m_uidEquip = uidEquip;
	Rsp.m_pos = pos;

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_AddEquip,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

	return true;
}

//移除装备 
bool EquipPart::RemoveEquip(UID uidEquip, bool bDestroyGoods)
{
	//需要通知客户端
	SC_RemoveEquip_Rsp Rsp;
	Rsp.m_Result = enEquipRetCode_OK;

	for(int i=0;i<ARRAY_SIZE(m_uidEquips);i++)
	{
		if(m_uidEquips[i] == uidEquip)
		{

			IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(uidEquip);

			if(pGoods)
			{
				//放回背包,但不用通知客户端重新创建了
				if( !bDestroyGoods && m_pActor->GetPacketPart()->AddGoods(pGoods->GetUID(),false)==false)
				{
					Rsp.m_Result = enEquipRetCode_PacketFull;
					break;
				}
				else
				{
					m_uidEquips[i] = UID();

					m_GoodsNum--;

					UINT8 Category = pGoods->GetGoodsClass();

					if(Category==enGoodsCategory_Equip
						|| Category == enGoodsCategory_GodSword
						|| Category == enGoodsCategory_Talisman)
					{

						IEquipment * pEquipment = (IEquipment*)pGoods;
						pEquipment->UnEquip(m_pActor);

						//判断套装属性
						if(Category==enGoodsCategory_Equip)
						{
							UINT16 SuitID = pEquipment->GetGoodsCnfg()->m_SuitIDOrSwordSecretID;
							if(SuitID != 0)
							{
								UINT8 & EquipNum = m_mapSuit[SuitID];

								EquipNum--;

								InActiveSuit(pEquipment, SuitID,EquipNum);

								const SSuitCnfg* pSuitCnfg = g_pGameServer->GetConfigServer()->GetSuitCnfg(SuitID);

								if(pSuitCnfg == 0)
								{
									TRACE("<error>  %s : %d line 找不到套装配置信息 SuitID = %d ", __FUNCTION__,__LINE__,SuitID);

								}
								else
								{
									if(EquipNum != pSuitCnfg->m_SuitTotalNum)
									{
										//获得外观
										UINT16 FacadeID = g_pGameServer->GetConfigServer()->GetUpgradeFacadeID(SuitID,this->GetDefaultFacade());
										if(FacadeID != 0)
										{
											this->RemoveFacade(FacadeID);
										}
									}
								}
							}
						}
					}

					this->DBDeleteGoods(uidEquip);

					//要删除装备
					if ( bDestroyGoods)
					{
						g_pGameServer->GetGameWorld()->DestroyThing(uidEquip);

						pGoods = 0;
					}
				}
			}


			Rsp.m_uidEquip = uidEquip;
			Rsp.m_uidActor = m_pActor->GetUID();
		}
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_RemoveEquip,sizeof(Rsp)) << Rsp;
	m_pActor->SendData(ob.TakeOsb());

	if ( enEquipRetCode_OK == Rsp.m_Result )
		return true;

	return false;
}

//获得装备数量
INT32 EquipPart::GetEquipNum()
{
	return m_GoodsNum;
}

IEquipment * EquipPart::GetEquipment(UID uidEquip)
{

	for(int i=0; i<ARRAY_SIZE(m_uidEquips); i++)
	{
		if(m_uidEquips[i] == uidEquip)
		{
			return GetEquipByPos(i);
		}
	}

	return 0;
}

//获得指定部位的装备
IEquipment * EquipPart::GetEquipByPos(UINT8 pos)
{
	if(pos >= enEquipPos_Max)
	{
		return 0;
	}

	if(!m_uidEquips[pos].IsValid())
	{
		return 0;
	}

	IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(m_uidEquips[pos]);

	if(0 == pGoods)
	{
		TRACE("<error> %s : %d Line 获取装备失败！！装备栏位置:%d, 装备UID:%s, 所在角色UID:%s, 角色名字:%s", __FUNCTION__, __LINE__, pos, m_uidEquips[pos].ToString(), m_pActor->GetUID().ToString(),m_pActor->GetName());
		this->DBDeleteGoods(m_uidEquips[pos]);
		m_uidEquips[pos] = UID();
		return 0;
	}

	UINT8 GoodsClass = pGoods->GetGoodsClass();

	if(GoodsClass != enGoodsCategory_Equip
		&& GoodsClass !=enGoodsCategory_GodSword
		&& GoodsClass !=enGoodsCategory_Talisman)
	{
		return 0;
	}


	return (IEquipment *)pGoods;
}

//重置装备栏中的仙剑副本
void EquipPart::ResetGodSwordFuBen()
{
	int nKillNpcNum = 0;
	int nFuBenFloor = 0;

	for( int i = enEquipPos_GodSwordOne; i <= enEquipPos_GodSwordThree; ++i)
	{
		IEquipment * pEquipment = this->GetEquipByPos(enEquipPos_GodSwordOne);
		if( 0 == pEquipment){
			continue;
		}

		pEquipment->GetPropNum(enGoodsProp_KillNpcNum, nKillNpcNum);
		if( nKillNpcNum > 0){
			//设为0
			pEquipment->SetPropNum(m_pActor, enGoodsProp_KillNpcNum, 0);
			nKillNpcNum = 0;
		}

		pEquipment->GetPropNum(enGoodsProp_FuBenLevel, nFuBenFloor);
		if( nFuBenFloor > 0){
			//设为0
			pEquipment->SetPropNum(m_pActor, enGoodsProp_FuBenLevel, 0);
			nFuBenFloor = 0;
		}
	}
}

//把装备栏的装备全部放回背包，！！！注意：不管背包是否已满
void	EquipPart::RemoveAllEquipToPacket()
{
	IPacketPart * pPacketPart = m_pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	for( int i = 0; i < enEquipPos_Max; ++i)
	{
		if( !m_uidEquips[i].IsValid()){
			continue;
		}

		IGoods * pGoods = g_pGameServer->GetGameWorld()->GetGoods(m_uidEquips[i]);
		if( 0 == pGoods){
			continue;
		}

		if( enGoodsCategory_Equip == pGoods->GetGoodsClass() || enGoodsCategory_GodSword == pGoods->GetGoodsClass()
			|| enGoodsCategory_Talisman == pGoods->GetGoodsClass())
		{
			((IEquipment*)pGoods)->UnEquip(m_pActor);
		}

		pPacketPart->AddGoods(m_uidEquips[i], false);

		this->DBDeleteGoods(m_uidEquips[i]);

		m_uidEquips[i] = UID();
	}
}

//发送装备栏数据给别的玩家查看
void	EquipPart::SendEquipPanelData(IActor * pActor)
{
	SC_EquipPanel_Sync Rsp;
	Rsp.m_EquipNum = 0;
	Rsp.m_uidActor = m_pActor->GetUID();

	OBuffer1k ob2;
	for(int i=0; i<ARRAY_SIZE(m_uidEquips);++i)
	{
		if(m_uidEquips[i].IsValid())
		{
			pActor->NoticClientCreatePrivateThing(m_uidEquips[i]);

			EquipInfo Info;
			Info.m_uidEquip = m_uidEquips[i];
			Info.m_pos = i;
			ob2 << Info;

			Rsp.m_EquipNum++;
		}
	}

	OBuffer1k ob;
	ob << Equip_Header(enEquipCmd_SyncEquip,sizeof(Rsp)+ob2.Size()) << Rsp << ob2;
	pActor->SendData(ob.TakeOsb());
}

//物品保存到数据库
void	 EquipPart::SaveGoodsToDB(IGoods * pGoods, UINT8 nLocation)
{
	SDB_Save_GoodsReq DBGoodsData;

	int size = sizeof(SDB_Save_GoodsReq);

	if(false == pGoods->OnGetDBContext(&DBGoodsData,size)){
		return;
	}

	DBGoodsData.m_uidUser = m_pActor->GetUID().ToUint64();
	DBGoodsData.m_Pos = 1;
	DBGoodsData.m_Location = nLocation;

	OBuffer1k ob;
	ob << DBGoodsData;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_SaveGoods,ob.TakeOsb(),0,0);

	//重新设置成不需要保存
	pGoods->SetUpdate(false);	
}

bool	 EquipPart::SaveGoodsToDB(IGoods * pGoods)
{
	if (0 == pGoods){
		return false;
	}

	UID uidGoods = pGoods->GetUID();

	for (int i = 0; i < enEquipPos_Max; ++i)
	{
		if (uidGoods == m_uidEquips[i]){

			this->SaveGoodsToDB(pGoods, i);
			return true;
		}
	}

	return false;
}

//数据库删除装备
void	 EquipPart::DBDeleteGoods(UID uidGoods)
{
	//剩下的删除
	SDB_Delete_GoodsReq DeleteEquip;

	DeleteEquip.m_uidUser = m_pActor->GetUID().ToUint64();
	DeleteEquip.m_uidGoods = uidGoods.ToUint64();
	DeleteEquip.m_Pos	  = 1;

	OBuffer1k ob;
	ob << DeleteEquip;
	g_pGameServer->GetDBProxyClient()->Request(m_pActor->GetCrtProp(enCrtProp_ActorUserID),enDBCmd_DeleteGoods,ob.TakeOsb(),0,0);
}


//打开外观栏
void EquipPart::OpenFacadePanel()
{
	SC_OpenFacade_Rsp Rsp;
	Rsp.m_uidActor = m_pActor->GetUID();
	Rsp.m_FacadeNum = m_vectFacade.size();

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_OpenFacade,sizeof(Rsp)+Rsp.m_FacadeNum*sizeof(UINT16)) << Rsp;
	if(Rsp.m_FacadeNum>0)
	{
		ob.Push(&m_vectFacade[0],Rsp.m_FacadeNum*sizeof(UINT16));
	}

	m_pActor->SendData(ob.TakeOsb());

	/*	IActor * pEmployee = 0;

	for(int i=0; 0!=(pEmployee=m_pActor->GetEmployee(i));i++)
	{
	IEquipPart * pEquipPart = pEmployee->GetEquipPart();
	if(pEquipPart !=0)
	{
	pEquipPart->OpenFacadePanel();
	}
	}*/
}

//设置显示外观
void EquipPart::SetShowFacade(UINT16 FacadeID)
{
	SC_SetFacade_Rsp Rsp;
	Rsp.m_uidActor = m_pActor->GetUID();
	Rsp.m_FacadeID = FacadeID;
	Rsp.m_Result = enEquipRetCode_OK;

	std::vector<UINT16>::iterator it = std::find(m_vectFacade.begin(),m_vectFacade.end(),FacadeID);

	if(it == m_vectFacade.end())
	{
		Rsp.m_Result = enEquipRetCode_NoFacade;
	}
	else
	{
		m_pActor->SetCrtProp(enCrtProp_ActorFacade,FacadeID);
	}

	OBuffer1k ob;

	ob << Equip_Header(enEquipCmd_SetFacade,sizeof(Rsp)) << Rsp;

	m_pActor->SendData(ob.TakeOsb());

}

//增加外观
void EquipPart::AddFacade(UINT16 FacadeID)
{
	std::vector<UINT16>::iterator it = std::find(m_vectFacade.begin(),m_vectFacade.end(),FacadeID);

	if(it == m_vectFacade.end())
	{
		m_vectFacade.push_back(FacadeID);
	}
}

//移除外观
void EquipPart::RemoveFacade(UINT16 FacadeID)
{
	std::vector<UINT16>::iterator it = std::find(m_vectFacade.begin(),m_vectFacade.end(),FacadeID);

	if(it != m_vectFacade.end())
	{
		if(FacadeID == m_vectFacade[0])
		{
			TRACE("<error> %s : %d 默认外观不可以移除！",__FUNCTION__,__LINE__);
			return;
		}
		m_vectFacade.erase(it);
	}

	if(m_pActor->GetCrtProp(enCrtProp_ActorFacade)==FacadeID)
	{
		if(!m_vectFacade.empty())
		{
			SetShowFacade(m_vectFacade[0]);
		}
	}
}

//获得默认外观
UINT16 EquipPart::GetDefaultFacade()
{
	if(m_vectFacade.empty())
	{
		m_vectFacade.push_back(m_pActor->GetCrtProp(enCrtProp_ActorFacade));
	}

	return m_vectFacade[0];
}
