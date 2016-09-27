#include "IActor.h"
#include "GoodsServer.h"
#include "IPacketPart.h"

#include "IConfigServer.h"
#include "IGoods.h"
#include "IBasicService.h"
#include "RandomService.h"
#include "IPacketPart.h"
#include "IMailPart.h"
#include "IGameWorld.h"
#include "XDateTime.h"
#include "IEquipPart.h"
#include "IEquipment.h"
#include "StringUtil.h"
#include "IGodSword.h"
#include "ITalisman.h"
#include "ICDTime.h"
#include "DMsgSubAction.h"
#include <vector>
#include "ICDTimerPart.h"

IGameServer * g_pGameServer=0;

IGoodsServer * CreateGoodsServer(IGameServer *pServerGlobal)
{
	g_pGameServer = pServerGlobal;
	GoodsServer * pGoodsServer = new GoodsServer();

	if(pGoodsServer->Create()==false)
	{
		delete pGoodsServer;
		pGoodsServer = 0;
	}

	return pGoodsServer;
}

GoodsServer::GoodsServer()
{
}
GoodsServer::~GoodsServer()
{
}

bool GoodsServer::Create()
{
	if(m_ShopMall.Create() == false){
		return false;
	}

	if(m_GoodsUseHandler.Create()==false)
	{
		return false;
	}

	if( m_CDTimeMgr.Create() == false)
	{
		return false;
	}

	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_AddGoods,this)==false)
	{
	}

	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Money,this)==false)
	{
	}

	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Stone,this)==false)
	{
	}

	if(g_pGameServer->GetRelationServer()->RegisterGMCmdHanler(enGMCmd_Ticket,this)==false)
	{
	}

	//获得需要记录日志的物品
	SDB_Get_NeedLogGoodsID_Req DBReq;
	
	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(3 , enDBCmd_Get_NeedLogGoods, ob.TakeOsb(), this, 0);

	return true;
}
//释放
void GoodsServer::Release(void)
{
	delete this;
}

void GoodsServer::Close()
{
	m_ShopMall.Close();

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_AddGoods,this);

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_Money,this);

	g_pGameServer->GetRelationServer()->UnRegisterGMCmdHanler(enGMCmd_Stone,this);
}

//使用
bool GoodsServer::UseGoods(IActor * pActor,UID uidGoods,UID uidTarget,INT32 nNum, UINT8 Index)
{
	SC_PacketUseGoods_Rsp Rsp;
	Rsp.m_Result = enPacketRetCode_OK;

	std::string strUseDesc;

	const SGoodsUseCnfg * pGoodsUseCnfg = 0; 

	GOODSUSE_FUNC  pHandle_func = 0;

	IPacketPart * pPacketPart = pActor->GetPacketPart();

	if(pPacketPart==0)
	{
		return false;
	}

	IActor * pTarget = g_pGameServer->GetGameWorld()->FindActor(uidTarget);
	if( 0 == pTarget){
		return false;
	}

	ICDTimerPart * pCDTimerPart = pTarget->GetCDTimerPart();
	if( 0 == pCDTimerPart){
		return false;
	}

	IGoods * pGoods = pPacketPart->GetGoods(uidGoods);
	if(pGoods==0)
	{
		Rsp.m_Result = enPacketRetCode_NoGoods;
	}
	else if(0==(pGoodsUseCnfg=g_pGameServer->GetConfigServer()->GetGoodsUseCnfg(pGoods->GetGoodsID())))
	{
		Rsp.m_Result = enPacketRetCode_NoGoodsUse;
	}
	else if( pGoodsUseCnfg->m_MaxUseNum>0 && nNum>pGoods->GetNumber())
	{
		Rsp.m_Result = enPacketRetCode_NoNum;
	}
	else if( pGoods->GetGoodsCnfg()->m_UsedLevel > pTarget->GetCrtProp(enCrtProp_Level))
	{
		Rsp.m_Result = enPacketRetCode_ErrLevel;
	}
	else if( pGoodsUseCnfg->m_CDTimeRange == 0 ? (!pCDTimerPart->IsCDTimeOK(pGoodsUseCnfg->m_CDTimerID)) : (!pCDTimerPart->IsCDTimeOK(pGoodsUseCnfg->m_CDTimerID, true)))
	{
		Rsp.m_Result = enPacketRetCode_ErrCDTime;
	}
	else if(0==(pHandle_func=m_GoodsUseHandler.GetHandleFunc((enGoodsUseHandlerType)(pGoodsUseCnfg->m_GoodsUseType))))
	{
		Rsp.m_Result = enPacketRetCode_NoGoodsUse;
		TRACE("<error> %s : %d line 找不到物品使用处理函数,GoodsID = %d", __FUNCTION__,__LINE__,pGoods->GetGoodsID());
	}
	else if( 0 == *pHandle_func)
	{
		TRACE("<error> %s : %d line 物品的使用类型没有对应函数，使用类型 = %d", __FUNCTION__,__LINE__,pGoodsUseCnfg->m_GoodsUseType);
		return false;
	}
	else
	{
		Rsp.m_Result = (enPacketRetCode)((*pHandle_func)(pActor,pGoods,pGoodsUseCnfg,uidTarget,nNum,strUseDesc,Index));

		if(Rsp.m_Result==enPacketRetCode_OK && pGoodsUseCnfg->m_MaxUseNum>0)
		{
			//发布事件
			const SGoodsCnfg * pGoodsCnfg = pGoods->GetGoodsCnfg();
			if( 0 != pGoodsCnfg){
				SS_UseGoods UseGoods;
				UseGoods.m_GoodsID = pGoodsUseCnfg->m_GoodsID;
				UseGoods.m_Category = pGoods->GetGoodsClass();
				UseGoods.m_SubClass = pGoods->GetGoodsCnfg()->m_SubClass;
				UseGoods.m_ThreeClass = pGoods->GetGoodsCnfg()->m_ThirdClass;

				UINT32 msgID = MAKE_MSGID(CIRCULTYPE_SS,enMsgID_UseGoods);
				pActor->OnEvent(msgID,&UseGoods,sizeof(UseGoods));
			}

			INT32 number = pGoods->GetNumber();
			if(number<=nNum)
			{				//全部扣除
				if(pPacketPart->DestroyGoods(uidGoods)==false)
				{
					TRACE("<error> %s : %d line 扣除玩家[%s]使用的物品失败,GoodsID = %d", __FUNCTION__,__LINE__,pActor->GetName(),pGoods->GetGoodsID());
				}
			}
			else if(pPacketPart->DestroyGoods(pGoods->GetUID(),nNum)==false)
			{
				TRACE("<error> %s : %d line 扣除玩家[%s]使用的物品失败,GoodsID = %d nNum=%d", __FUNCTION__,__LINE__,pActor->GetName(),pGoods->GetGoodsID(),nNum);
			}
		}
	}

	OBuffer1k ob;

	if( enPacketRetCode_OK == Rsp.m_Result){

		if( INVALID_CDTIMER_ID != pGoodsUseCnfg->m_CDTimerID)
		{
			if( pGoodsUseCnfg->m_CDTimeRange == 0){
				//针对单个角色的CD时间
				pCDTimerPart->RegistCDTime(pGoodsUseCnfg->m_CDTimerID);
			}else{
				//针对所有角色的CD时间
				pCDTimerPart->RegistCDTime(pGoodsUseCnfg->m_CDTimerID, true);
			}
		}

		g_pGameServer->GetGoodsServer()->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Use,enGameGoodsChanel_Use,pGoodsUseCnfg->m_GoodsID,uidGoods,nNum,"使用物品");

		if( Index == 0){
			ob << Packet_Header(enPacketCmd_UseGoods,sizeof(Rsp) + strUseDesc.length() + 1) << Rsp;
		}else{
			ob << Packet_Header(enPacketCmd_ConfirmationUseGoods,sizeof(Rsp) + strUseDesc.length() + 1) << Rsp;
		}

		ob.Push(strUseDesc.c_str(), strUseDesc.length() + 1);
	}else{
		if( Index == 0){
			ob << Packet_Header(enPacketCmd_UseGoods,sizeof(Rsp)) << Rsp;
		}else{
			ob << Packet_Header(enPacketCmd_ConfirmationUseGoods,sizeof(Rsp)) << Rsp;
		}
	}

	pActor->SendData(ob.TakeOsb());

	return true;
}

//根据掉落ID，给玩家掉落物品
bool GoodsServer::GiveUserDropGoods(IActor * pActor, UINT16 DropID,std::vector<TGoodsID> & vectGoodsID)
{
	TGoodsID	DropGoodsID = 0;

	//获得掉落的物品数量和物品ID集合
	this->GetDropGoodsID(DropID, vectGoodsID);

	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		TRACE("<error> %s : %d line 玩家背包获取失败!!,ActorID = %d", __FUNCTION__,__LINE__,pActor->GetCrtProp(enCrtProp_ActorUserID));
		return false;
	}

	std::vector<IGoods *> vecGoods;

	for( int nCount = 0; nCount < vectGoodsID.size(); ++nCount)
	{
		DropGoodsID = vectGoodsID[nCount];
		if(DropGoodsID==INVALID_GOODS_ID)
		{
			continue;
		}

		if( pPacketPart->AddGoods(DropGoodsID) == false){
			//如果加入背包失败，则使用邮件发送给玩家
			SCreateGoodsContext CreateGoods;
			CreateGoods.m_Binded  = false;
			CreateGoods.m_GoodsID = DropGoodsID;
			CreateGoods.m_Number  = 1;

			IGoods * pGoods = g_pGameServer->GetGameWorld()->CreateGoods(CreateGoods);
			if( 0 == pGoods){
				TRACE("<error> %s : %d line 创建物品失败!!,GoodsID = %d", __FUNCTION__,__LINE__,DropGoodsID);
				return false;
			}

			vecGoods.push_back(pGoods);
			continue;
		}

		this->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_Drop,DropGoodsID,UID(),1,"物品掉落");
	}

	if ( vecGoods.size() > 0 ){

		SWriteSystemData MailData;

		MailData.m_DestUID		    = pActor->GetUID();

		// fly add	20121106

		strncpy(MailData.m_szThemeText, g_pGameServer->GetGameWorld()->GetLanguageStr(10014), sizeof(MailData.m_szThemeText));
		//strncpy(MailData.m_szThemeText, "掉落获得物品", sizeof(MailData.m_szThemeText));

		//发送系统邮件
		g_pGameServer->GetGameWorld()->WriteSystemMail(MailData, vecGoods);

		for(int i=0; i< vectGoodsID.size(); i++)
		{
			pActor->SendGoodsCnfg(vectGoodsID[i]);
		}
	}

	return true;
}

//根据掉落ID，得到掉落的物品ID
void	GoodsServer::GetDropGoodsID(UINT16 DropID, std::vector<TGoodsID> & vectGoods)
{
	const std::vector<SDropGoods> * pVectDropGoods = g_pGameServer->GetConfigServer()->GetDropGoodsCnfg(DropID);
	if( 0 == pVectDropGoods){
		TRACE("<error> %s : %d line 获取掉落的配置文件失败!!,DropID = %d", __FUNCTION__,__LINE__,DropID);
		return;
	}

	for(int i=0; i<pVectDropGoods->size();i++)
	{
		const SDropGoods * pDropGoods = &(*pVectDropGoods)[i];

		if(pDropGoods->m_DropNum < 1 || pDropGoods->m_vectDropGoods.size() < pDropGoods->m_DropNum || pDropGoods->m_nTotalDropGoodsRand < 1 ){
			continue;
		}

		INT32 nTotalDropGoodsRand = pDropGoods->m_nTotalDropGoodsRand;

		//复制一份
		std::vector<TGoodsID> vectDropGoods = pDropGoods->m_vectDropGoods;

		for(int nCount =0; nCount < pDropGoods->m_DropNum; nCount++)
		{
			INT32 nTmpRand = RandomService::GetRandom()%nTotalDropGoodsRand;

			//几率
			INT16 prob = 0;

			for( int k = 0; k < vectDropGoods.size()/3; k++)
			{
				//物品
				TGoodsID GoodsID = vectDropGoods[k*3];
				//数量
				INT16    nNum    = vectDropGoods[k*3+1];
				//几率
				prob += vectDropGoods[k*3 + 2];

				if( nTmpRand < prob)
				{					
					//相同GoodsID需要去掉，避免重复选到
					for(int n=0; n<vectDropGoods.size()/3;n++)
					{
						if(vectDropGoods[n*3] == GoodsID)
						{
							nTotalDropGoodsRand -= vectDropGoods[n*3+2];
							vectDropGoods.erase(vectDropGoods.begin()+n*3,vectDropGoods.begin()+(n+1)*3);
							n--;
						}
					}		

					if(GoodsID != INVALID_GOODS_ID)
					{
						for(int n=0; n< nNum;n++)  //数量
					   {
						  vectGoods.push_back(GoodsID);
					   }
					}

					break;
				}
			}
		}
	}
}


//判断玩家是否拥有该物品,包括装备栏，背包栏
IGoods* GoodsServer::GetGoodsFromPacketOrEquipPanel(IActor * pActor,UID uidGoods)
{
	IPacketPart * pPacketPart  = pActor->GetPacketPart();

	IEquipPart * pEquipPart = pActor->GetEquipPart();

	IGoods * pGoods = pPacketPart->GetGoods(uidGoods);

	if(pGoods==0)
	{
		pGoods = pEquipPart->GetEquipment(uidGoods);

		if( 0 != pGoods){
			return pGoods;
		}
	}else{
		return pGoods;
	}

	//从招募角色装备栏上获取
	for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
	{
		IActor * pEmployee = pActor->GetEmployee(i);
		if( 0 == pEmployee){
			continue;
		}

		IEquipPart * pTmpEquipPart = pEmployee->GetEquipPart();
		if( 0 == pTmpEquipPart){
			continue;
		}

		pGoods = pTmpEquipPart->GetEquipment(uidGoods);
		if( 0 != pGoods){
			return pGoods;
		}
	}

	return pGoods;
}

//判断一件装备是装备哪个角色身上
IActor * GoodsServer::GetActorOnEquip(IActor * pActor, IEquipment * pEquipment)
{
	IEquipPart * pEquipPart = pActor->GetEquipPart();
	if( 0 == pEquipPart){
		return 0;
	}

	//判断在哪个角色身上
	if( pEquipment == pEquipPart->GetEquipment(pEquipment->GetUID())){
		return pActor;
	}else{
		//招募角色身上获取
		for( int i = 0; i < MAX_EMPLOY_NUM; ++i)
		{
			IActor * pEmployee = pActor->GetEmployee(i);
			if( 0 == pEmployee){
				continue;
			}

			IEquipPart * pTmpEquipPart = pEmployee->GetEquipPart();
			if( 0 == pTmpEquipPart){
				continue;
			}

			if( pEquipment == pTmpEquipPart->GetEquipment(pEquipment->GetUID())){
				return pEmployee;
			}
		}
	}

	return 0;
}

//获得冷却接口
ICDTimeMgr * GoodsServer::GetCDTimeMgr()
{
	return &m_CDTimeMgr;
}


void GoodsServer::OnHandleGMCmd(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam) 
{
	switch(Cmd)
	{
	case enGMCmd_AddGoods:
		{
			OnHandleGMCmdAddGoods(pActor,Cmd,vectParam);
		}
		break;
	case enGMCmd_Money:
		{
			OnHandleGMCmdSetMoney(pActor,Cmd,vectParam);
		}
		break;
	case enGMCmd_Stone:
		{
			OnHandleGMCmdSetStone(pActor,Cmd,vectParam);
		}
		break;
	case enGMCmd_Ticket:
		{
			OnHandleGMCmdSetTicket(pActor,Cmd,vectParam);
		}
		break;
	default:
		break;
	}
}


void GoodsServer::OnHandleGMCmdAddGoods(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam) 
{
	//物品ID
	TGoodsID goodsID = INVALID_GOODS_ID;

	//数量
	INT32 nNum = 1;

	//星级
	INT32 nStarLevel = 0;

	//参数数量
	INT32 ParamNum = vectParam.size();

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";


	if(ParamNum > 0)
	{
		StringUtil::StrToNumber(vectParam[0],goodsID);
	}


	if(ParamNum > 1)
	{
		StringUtil::StrToNumber(vectParam[1],nNum);
	}

	if(ParamNum > 2)
	{
		StringUtil::StrToNumber(vectParam[2],nStarLevel);
	}

	if(goodsID == INVALID_GOODS_ID || nNum<1)
	{
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
		return;
	}

	//加入背包
	IPacketPart * pPacketPart = pActor->GetPacketPart();
	if( 0 == pPacketPart){
		return;
	}

	//创建物品并增加星级
	SCreateGoodsContext CreateGoodsContext;
	CreateGoodsContext.m_Binded  = false;
	CreateGoodsContext.m_GoodsID = goodsID;
	CreateGoodsContext.m_Number	 = nNum;

	std::vector<IGoods *> vectGoods = g_pGameServer->GetGameWorld()->CreateGoods(goodsID, nNum, false);

	if( vectGoods.size() == 0){
		sprintf_s(szTip, sizeof(szTip), "创建物品失败！！物品ID:%d 数量:%d", goodsID, nNum);
		g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
		return;	
	}


	//设置星级并加入背包
	IGoods * pGoods = 0;
	for( int i = 0; i < vectGoods.size(); ++i)
	{
		pGoods = vectGoods[i];

		sprintf_s(szTip, sizeof(szTip), "成功获得%s", pGoods->GetName()); //后面pGoods可能会销毁，需要先获得物品名

		if( pGoods->GetGoodsClass() == enGoodsCategory_GodSword && nStarLevel > 0){
			IGodSword * pGodSword = (IGodSword *)pGoods;
			pGodSword->SetPropNum(pActor, enGoodsProp_StarLevel, nStarLevel);
		}else if( pGoods->GetGoodsClass() == enGoodsCategory_Talisman){
			ITalisman * pTalisman = (ITalisman *)pGoods;
			pTalisman->SetPropNum(pActor, enGoodsProp_StarLevel, nStarLevel);
		}

		if( false == pPacketPart->AddGoods(pGoods->GetUID())){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("物品加入背包失败，检查背包是否不够！！", pActor);
			g_pGameServer->GetGameWorld()->DestroyThing(pGoods->GetUID());
			return;
		}

		this->Save_GoodsLog(pActor->GetCrtProp(enCrtProp_ActorUserID),enGameGoodsType_Add,enGameGoodsChanel_GMCmd,pGoods->GetGoodsID(),pGoods->GetUID(),pGoods->GetNumber(),"GM命令获得物品");
	}


	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}                              

//通过GM命令增加仙石
void GoodsServer::OnHandleGMCmdSetMoney(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam)
{
	INT32 Money = 0;

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],Money)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}

	if( Money < 0){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！仙石数量不可以为负数", pActor);
		return;
	}

	pActor->SetCrtProp(enCrtProp_ActorMoney, Money);

	sprintf_s(szTip, sizeof(szTip), "成功获得%d仙石", Money);
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

//通过GM命令增加灵石
void GoodsServer::OnHandleGMCmdSetStone(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam)
{
	INT32 Stone = 0;

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],Stone)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}

	if( Stone < 0){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！灵石数量不可以为负数", pActor);
		return;
	}

	pActor->SetCrtProp(enCrtProp_ActorStone, Stone);

	sprintf_s(szTip, sizeof(szTip), "成功获得%d灵石", Stone);
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);
}

//通过GM命令增加礼券
void GoodsServer::OnHandleGMCmdSetTicket(IActor * pActor,enGMCmd Cmd,std::vector<std::string> & vectParam)
{
	INT32 Ticket = 0;

	//提示
	char  szTip[DESCRIPT_LEN_100] = "\0";

	if( vectParam.size() > 0){
		if( false == StringUtil::StrToNumber(vectParam[0],Ticket)){
			g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！", pActor);
			return;
		}
	}

	if( Ticket < 0){
		g_pGameServer->GetGameWorld()->WorldSystemMsg("GM命令参数有错误！！礼券数量不可以为负数", pActor);
		return;
	}

	pActor->SetCrtProp(enCrtProp_ActorTicket, Ticket);

	sprintf_s(szTip, sizeof(szTip), "成功获得%d礼券", Ticket);
	g_pGameServer->GetGameWorld()->WorldSystemMsg(szTip, pActor);	
}

//得到物品品质对应颜色
enColor GoodsServer::GetGoodsQualityColor(IGoods * pGoods)
{
	const SGoodsCnfg * pGoodsCnfg = pGoods->GetGoodsCnfg();
	if( 0 == pGoodsCnfg){
		return enColor_Max;
	}

	switch(pGoodsCnfg->m_Quality)
	{
	case 0:
		return enColor_White;
	case 1:
		return enColor_Green;
	case 2:
		return enColor_Blue;
	case 3:
		return enColor_Violet;
	case 4:
		return enColor_Orange;
	default:
		TRACE("<error> %s : %d 获取物品品质颜色错误，物品ID=%d，品质=%d", __FUNCTION__, __LINE__, pGoodsCnfg->m_GoodsID, pGoodsCnfg->m_Quality);
		break;
	}

	return enColor_Max;
}

void GoodsServer::OnDBRet(unsigned int userID,enDBCmd ReqCmd, int nRetCode, OStreamBuffer & RspOsb,
	OStreamBuffer & ReqOsb,UINT64 userdata)
{
	if(nRetCode != enDBRetCode_OK)
	{
		TRACE("<error> %s : %d 行 DB应答错误 cmd=%d userID = %u nRetCode = %d",__FUNCTION__,__LINE__,ReqCmd,userID,nRetCode);
		return;
	}

	OBuffer4k RspOb(RspOsb);
	OBuffer4k ReqOb(ReqOsb);

	switch(ReqCmd)
	{
	   case enDBCmd_Get_NeedLogGoods:
			{
				HandleGetNeedLogGoods(userID,ReqCmd, nRetCode, RspOb, ReqOb, userdata);
			}
			break;
	}
}

void GoodsServer::HandleGetNeedLogGoods(unsigned int DBUserID,enDBCmd ReqCmd, int nRetCode, OBuffer4k & RspOb,
	OBuffer4k & ReqOb,UINT64 userdata)
{
	IBuffer RspIb(RspOb.Buffer(),RspOb.Size());

	DBRspPacketHeader RspHeader;
	DB_OutParam OutParam;
	RspIb >> RspHeader >> OutParam;

	if(RspIb.Error()){

		TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
		return;
	}

	if(OutParam.retCode != enDBRetCode_OK){
		return;
	}

	INT32 Num = RspIb.Remain() / sizeof(SDB_NeedLogGoodsID);

	for( int i = 0; i < Num; ++i)
	{
		SDB_NeedLogGoodsID GoodsID;
		RspIb >> GoodsID;

		if(RspIb.Error()){

			TRACE("<error> %s : %d 行 DB前端应答长度有误 cmd=%d userID = %u len=%d",__FUNCTION__, __LINE__,ReqCmd,DBUserID,RspIb.Capacity());
			return;
		}

		m_mapNeedLogGoodsID.insert(GoodsID.m_GoodsID);
	}
}

//记录物品日志
void GoodsServer::Save_GoodsLog(TUserID UserID,enGameGoodsType type,enGameGoodsChanel chanel,TGoodsID GoodsID,UID uidGoods,UINT16 GoodsNum,const char * pszDesc)
{
	std::set<TGoodsID>::iterator iter = m_mapNeedLogGoodsID.find(GoodsID);

	if( iter == m_mapNeedLogGoodsID.end()){
	
		return;
	}

	SDB_Save_GameGoodsLog_Req DBReq;

	DBReq.m_UserID = UserID;
	DBReq.m_Type = type;
	DBReq.m_Chanel = chanel;
	DBReq.m_GoodsID = GoodsID;
	DBReq.m_GoodsUID = uidGoods.ToUint64();
	DBReq.m_GoodsNum = GoodsNum;
	strncpy(DBReq.m_Description, pszDesc, sizeof(DBReq.m_Description));

	OBuffer1k ob;
	ob << DBReq;
	g_pGameServer->GetDBProxyClient()->Request(UserID , enDBCmd_Save_GameGoodsLog, ob.TakeOsb(), 0, 0);
}

//放置商城配置信息
void GoodsServer::Push_ShopMallCnfg(const SDB_Get_ShopMallCnfg & DBShopCnfg)
{
	m_ShopMall.Push_ShopMallCnfg(DBShopCnfg);
}


