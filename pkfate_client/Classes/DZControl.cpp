#include "DZControl.h"
#include "cJSON.h"
#include "comm.h"
#include "cmd.h"
#include "UserControl.h"
#include "BillControl.h"
#include "LayerMall.h"
#include "LayerLoading.h"
#include "ApiBill.h"
#include "PKNotificationCenter.h"
#include "dzpkcomm.h"
#include "ApiGame.h"
#include "SceneDzRoom.h"
#include "LayerDzGames.h"
#include "LayerSNGTip.h"
static DZControl *m_pInstance = nullptr;
DZControl::DZControl() :roomView(nullptr), hallView(nullptr), my_pos(0), roomTempInfo(nullptr), roomKind(DZROOMKIND::DZ_GAME_ROOM),
gameToken(""), gameRoomType(0), isSNG(false), sngRoundInfo(nullptr)
{

}

bool DZControl::Init()
{
	return true;
}

void DZControl::addNoticeListeners()
{
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onConnectSNGCallBack), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::MSGTYPE_DzpkGameMo_Connect), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushSngRoomType), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::TP_SNG_PUSH_ROOM_TYPES), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushSngGameStatus), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::TP_SNG_PUSH_ROOM_STATUS), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onSngBeginApply), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::TP_SNG_BEGIN_APPLY), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushSNGRoundInfo), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::TP_SNG_PUSH_ROUND_INFO), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onEnterSNGRoom), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::MSGTYPE_DzpkGameMo_EnterRoom), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBalanceUpdateEx), GetMsgTypeString(MSGTYPE_SNG_DZPK, MSGCMD_DZPKGAMEMO::PUSH_BALANCE_UPDATE_EX), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onSendEnterSNGRoom), "EnterSNGRoom", NULL);//监听来自视图的事件


	
	//----------------------------------------------------------------------------------------------------------------------------------------------------------//
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushSNGSessionResult), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_SNG_PUSH_SESSION_RESULT), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBuyChip), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_SNG_BUY_CHIP), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBuyChipTimes), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_SNG_PUSH_BUY_CHIP_TIMES), NULL);
	
	
	
	//login之后 推送types信息，join之后返回type
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushRoomType), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_ROOM_TYPES), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushGameStatus), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_GAME_STATUS), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onLoginCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Login), nullptr);
	//PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushChipFresh), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::TP_PUSH_CHIP_FRESH), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onRoomPlayersCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_RoomPlayers), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onRecvSitCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_RecvSit), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onRecvNewPlayerCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_NewRoomPlayer), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onRecvDelPlayerCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_DelRoomPlayer), nullptr);
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBalanceInfo), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::PUSH_BLANCE_UPDATE), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBetInfo), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_BET_INFO), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushFlopInfo), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_PRE_FLOP_INFO), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushRoundInfo), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_ROUND_INFO), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushGameResult), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_GAME_RESULT), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushNextTime), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_NEXT_TIME), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushBeginBet), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_BEGIN_BET), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushGameStart), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_BEGIN_START), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onTpBetCallback), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_BET), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushChipFreshCallback), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_PUSH_CHIP_FRESH), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onConnectCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_Connect), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onPushEnterRoomCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_EnterRoom), nullptr);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onChatCallBack), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGTYPE_DzpkGameMo_RecvChat), nullptr);
	//TP_PUSH_CHIP_FRESH


	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onCreateRoom), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_CREATE_ROOM), NULL);
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onJoinRoom), GetMsgTypeString(MSGTYPE_DZPKGAMEMO, MSGCMD_DZPKGAMEMO::TP_JOIN_ROOM), NULL);

	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onSendConnectDezpk), "ConnectDzpk", NULL);//监听来自视图的事件
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onSendJoinGame), "JoinGame", NULL);//监听来自视图的事件
	
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onTriggerJoinCreateRoom), "TriggerJoinCreateRoom", NULL);//监听来自视图的事件
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onTriggerCreateRoom), "TriggerCreateGame", NULL);//监听来自视图的事件


	//PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(DZControl::onSendBet), "SendBet", NULL);//监听来自视图的事件
}

void DZControl::setRoomInfo(int gameRoomType , DZROOMKIND roomKind ,string gameToken )//进入房间设置
{
	this->gameRoomType = gameRoomType; //房间类型
	this->roomKind = roomKind; //房间来源
	this->gameToken = gameToken;
}

DzpkRoomType* DZControl::getCurrentRoomType()
{
	if (gameRoomType != 0){
		if (RoomTypeInfos.find(gameRoomType) != RoomTypeInfos.end()){
			return RoomTypeInfos[gameRoomType];
		}
	}
	return nullptr;
}



void DZControl::disposeNoticeListeners()
{
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}


void DZControl::onSendConnectDezpk(Ref *pSender)
{

	LayerLoading::Wait("",8.0f,true);

	if (!Api::Game::mo_dzpk_connect()){
		LayerLoading::CloseWithTip(Language::getStringByKey("RequestFail"));
		CCLOG("DZControl::onSendConnectDezpk mo_dzpk_connect fail");
	};//开始连接


	if (!Api::Game::mo_sng_dzpk_connect()){
		LayerLoading::CloseWithTip(Language::getStringByKey("RequestFail"));
	}
}



void DZControl::onConnectCallBack(Ref *pSender)
{
	//连接游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功连接,开始登录
			Api::Game::mo_dzpk_login_game();
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onConnectCallBack", "connect error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


void DZControl::onConnectSNGCallBack(Ref *pSender)
{
	//连接游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功连接,开始登录
			Api::Game::mo_sng_dzpk_login_game();
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onConnectCallBack", "connect error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


void DZControl::onSendJoinGame(Ref *pSender)
{
	String *data = (String*)pSender;
	if (data == nullptr) return;
	
	LayerLoading::Wait();
	if (!Api::Game::mo_dzpk_join_game(data->intValue())){
		LayerLoading::CloseWithTip(Language::getStringByKey("RequestFail"));
		CCLOG("DZControl::onSendConnectDezpk mo_dzpk_connect fail");
	}
	else{
		gameRoomType = data->intValue();
	};//开始连接
}

void DZControl::onSendEnterSNGRoom(Ref *pSender)
{
	String *data = (String*)pSender;
	if (data == nullptr) return;

	LayerLoading::Wait();
	if (!Api::Game::mo_dzpk_enter_sng_game(data->intValue())){
		LayerLoading::CloseWithTip(Language::getStringByKey("RequestFail"));
		CCLOG("DZControl::onSendConnectDezpk mo_dzpk_connect fail");
	}
	else{
		gameRoomType = data->intValue();
	};//开始连接
}

void DZControl::onEnterSNGRoom(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		//清除旧的玩家数据
		if (msg->code == 0)
		{
			isSNG = true;

			//成功
			this->my_pos = 0;

			PKNotificationCenter::getInstance()->postNotification("EnterDzpkRoom");
		}
		else
		{
			if (msg->code == 5514){//不够
				LayerSNGTip * sngTip = LayerSNGTip::create();
				SNGRoomType *roomType = DZControl::GetInstance()->SngRoomTypeInfos[current_sng_type_id];

				string desc = DZControl::GetInstance()->getCurrencyTypeStr(roomType->apply_currency_type);

				string condition_text  = StringUtils::format("%d张%s", roomType->apply_cost, desc.c_str());

				string tip = StringUtils::format(Language::getStringByKey("SngSignUpFailTip2"), condition_text.c_str(), desc.c_str());

				sngTip->set(tip, "label_get_mall");
				sngTip->handler = [=](){
					((LayerDzGames*)hallView)->openMall();
				};
				sngTip->show();
			}
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "onPushEnterRoomCallBack::onEnterSNGRoom", "onEnterSNGRoom.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealEnterSNGRoom(string jsonData)
{

	return true;
}


void DZControl::onPushEnterRoomCallBack(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		
		//清除旧的玩家数据
		if (msg->code == 0)
		{
			//成功
			this->my_pos = 0;

			PKNotificationCenter::getInstance()->postNotification("EnterDzpkRoom");
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "onPushEnterRoomCallBack::onPushEnterRoomCallBack", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void DZControl::onRoomPlayersCallBack(Ref *pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
			//清除旧的玩家数据
		if (msg->code == 0  )
		{
			dealRoomPlayers(msg->data);
			//成功
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onRoomPlayersCallBack", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealRoomPlayers(string jsonData)
{

	for (auto player : RoomPlayers)
		delete player.second;
	RoomPlayers.clear();

	if (jsonData.empty())
		return false;
	bool result = false;
	cJSON *jsonMsg;

	if ((jsonMsg = cJSON_Parse(jsonData.c_str())))
	{
		int size;
		if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg)) > 0)
		{
			result = true;
			int playersize;
			cJSON  *player, *user_id, *nickname, *avatar;
			for (int i = 0; i < size; i++)
			{

				if ((player = cJSON_GetArrayItem(jsonMsg, i)) && player->type == cJSON_Array && (playersize = cJSON_GetArraySize(player)) == 3
					&& (user_id = cJSON_GetArrayItem(player, 0)) && user_id->type == cJSON_Number
					&& (nickname = cJSON_GetArrayItem(player, 1)) && nickname->type == cJSON_String
					&& (avatar = cJSON_GetArrayItem(player, 2)) && avatar->type == cJSON_Number)
				{
					RoomPlayer *roomPlayer = new RoomPlayer();
					roomPlayer->user_id = user_id->valueint;
					roomPlayer->nickname = nickname->valuestring;
					roomPlayer->avatar = avatar->valueint;
					roomPlayer->pos = 0;
					roomPlayer->status = TPParticipantStatus::LEAVE;
					RoomPlayers[roomPlayer->user_id] = roomPlayer;
				}
			}
		}

		cJSON_Delete(jsonMsg);
	}
	return result;
}


int DZControl::getCurrentPlayersNum()
{
	int index = 0;
	for (auto item : RoomPlayers){
		RoomPlayer* player = item.second;
		if (player->pos != 0 && player->status != TPParticipantStatus::LEAVE&&player->status != TPParticipantStatus::LEAVE_ROOM){
			index = index + 1;
		}
	}
	return index;
}

void DZControl::onRecvSitCallBack(Ref* pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		
		if (msg->code == 0 )
		{
			LayerLoading::Close();

			dealSitPlayers(msg->data);
		}
		else
		{
			LayerLoading::CloseWithTip("onRecvSitCallBack fail");
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onRecvSitCallBack", "get sit data fail.", msg->code, msg->data.c_str());
		}
		
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

RoomPlayer * DZControl::getPlayerByPos(int pos){
	for (auto item : RoomPlayers){

		if (item.second&&item.second->pos == pos) return item.second;
	}
	return nullptr;
}

bool DZControl::dealSitPlayers(string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg=cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	int size;
	
	
	if (jsonMsg->type == cJSON_Array && (size = cJSON_GetArraySize(jsonMsg)) > 0)
	{
		int playersize;
		int my_user_id = UserControl::GetInstance()->GetUserData()->user_id;
		cJSON  *player, *pos, *userid;
		for (int i = 0; i < size; i++)
		{

			if ((player = cJSON_GetArrayItem(jsonMsg, i)) && player->type == cJSON_Array && (playersize = cJSON_GetArraySize(player)) > 2
				&& (pos = cJSON_GetArrayItem(player, 0)) && pos->type == cJSON_Number
				&& (userid = cJSON_GetArrayItem(player, 1)) && userid->type == cJSON_Number
				)
			{

				int p_pos = pos->valueint;
				int p_user_id = userid->valueint;


				if (p_user_id == 0){ //说明已经存在，离开
					if (sitInfos.find(p_pos) == sitInfos.end()){
						CCLOG("the seat %d not sit player", p_pos);
						cJSON_Delete(jsonMsg);
						return false;
					}
					RoomSitInfo * oldSitInfo = sitInfos[p_pos];
					
					if (RoomPlayers.find(oldSitInfo->user_id) == RoomPlayers.end())
					{
						CCLOG("cant find user_id ( %d )  RoomPlayers", oldSitInfo->user_id);
						cJSON_Delete(jsonMsg);
						return false;
					}
					
					auto player = RoomPlayers[oldSitInfo->user_id]; // 下一轮删除所以还存在

					if (player != nullptr){
						if (player->status != TPParticipantStatus::LEAVE_ROOM){
							player->status = TPParticipantStatus::LEAVE;
						}
					}
				}
				else{
					RoomSitInfo * sitInfo = nullptr;
					if (sitInfos.find(p_pos) == sitInfos.end()){
						sitInfo = new RoomSitInfo;
						sitInfo->pos = p_pos;
						sitInfos[p_pos] = sitInfo;
					}
					else{
						sitInfo = sitInfos[p_pos];
					}

					RoomSitInfo *replaceSitInfo = getSitInfo(p_user_id); //获取 之前所在的位置
					if (replaceSitInfo != nullptr){
						replaceSitInfo->user_id = 0;//清除
					}

					sitInfo->user_id = p_user_id;

					if (RoomPlayers.find(sitInfo->user_id) != RoomPlayers.end())
					{
						auto player = RoomPlayers[sitInfo->user_id];
						player->status = TPParticipantStatus::GAMING;
						player->pos = sitInfo->pos;
					}

					//处理my_pos

					if (my_pos != 0){// 玩家自己已经在，新推的用户
						if (sitInfo->pos == my_pos && sitInfo->user_id != my_user_id){//有人占据了我的位子
							my_pos = 0; //我的位子重置为0
						}
					}
					
					if (my_user_id == sitInfo->user_id)//玩家自己刚进来坐下
					{
						my_pos = sitInfo->pos;
					}

				}
			}
		}

		if (roomView){
			((SceneDzRoom*)roomView)->updateSitInfos();
		}
		checkAndUpdateRoomTempInfo();
	}

	

	cJSON_Delete(jsonMsg);
	return true;
}


RoomSitInfo* DZControl::getSitInfo(int user_id)
{
	for (auto item : sitInfos){
		if (item.second->user_id == user_id){
			return item.second;
		}
	}

	return nullptr;
}

int  DZControl::getOneNotSitPos(){
	if (my_pos == 0){//被占据
		//随意取
		int pos = 0;
		for (int i = 1; i <= 7; i++){
			pos = i;
			for (auto item : sitInfos){
				auto sitInfo = item.second;
				if (sitInfo->pos == i){
					pos = 0;//重置
					break;
				}
			}
			if (pos != 0) return pos;
		}
		return pos;

	}
	else{
		return my_pos;
	}

}


void DZControl::onRecvNewPlayerCallBack(Ref* pSender)
{
	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0  )
		{
			dealNewPlayer(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onRecvNewPlayerCallBack", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealNewPlayer(string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON *id,*nickname,*avatar;
	if (!(id = cJSON_GetArrayItem(jsonMsg, 0)) || id->type != cJSON_Number
		|| !(nickname = cJSON_GetArrayItem(jsonMsg, 1)) || nickname->type != cJSON_String
		|| !(avatar = cJSON_GetArrayItem(jsonMsg, 2)) || avatar->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}

	RoomPlayer *roomPlayer = new RoomPlayer();
	roomPlayer->user_id = id->valueint;
	roomPlayer->nickname = nickname->valuestring;
	roomPlayer->avatar = avatar->valueint;
	roomPlayer->pos = 0;
	roomPlayer->status = TPParticipantStatus::LEAVE;
	RoomPlayers[roomPlayer->user_id] = roomPlayer;

	cJSON_Delete(jsonMsg);
	return true;
}

void DZControl::onRecvDelPlayerCallBack(Ref *pSender)
{
	//玩家离开房间回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功
			int id = atoi(msg->data.c_str());
			/*delete RoomPlayers[id];*/

			map<int, RoomPlayer*>::iterator itr = RoomPlayers.find(id);

			if (itr != RoomPlayers.end()) {
				//RoomPlayers.erase(itr);
				itr->second->status = TPParticipantStatus::LEAVE_ROOM;
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "SceneBjlRoom::onNewRoomPlay", "get new room player data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void DZControl::checkAndDelRoomPlayers()
{
	for (map<int, RoomPlayer*>::iterator i = RoomPlayers.begin(); i != RoomPlayers.end();)
	{
		if (i->second->status == TPParticipantStatus::LEAVE_ROOM)
		{
			
			delete i->second;

			RoomPlayers.erase(i++);
		}
		else
		{
			i++;
		}
	}
}

int DZControl::getLogicPos(int ui_pos)
{
	if (my_pos == 0)
	{
		return ui_pos;
	}
	return (ui_pos - 1 + my_pos)%7;
}

int DZControl::getUIPos(int logic_pos)
{
	if (my_pos == 0)
	{
		return logic_pos;
	}
	if (logic_pos>=my_pos)
	{
		return 1 + logic_pos - my_pos;
	}
	else
	{
		return 7 - my_pos + logic_pos + 1;
	}

}




void DZControl::onLoginCallBack(Ref *pSender)
{
	//登录游戏大厅回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			// 等待房间信息
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBjlGames::onConnectCallBack", "connect error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void DZControl::onPushRoomType(Ref *pSender){

	// 获取所有房间类型信息
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{

			//成功获取所有房间类型信息数据
			if (!InitRoomTypesInfoData(msg->data)){
				CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onMsgRoomTypesInfo", "init roomtypes info error.", msg->code, msg->data.c_str());
				ShowTip(Language::getStringByKey("RoomInfoEmpty"), [=](){
					PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::NO)));
				},nullptr,true);
				
			}
			else{
				if (hallView){
					((LayerDzGames*)hallView)->updateRoomsTypes();
				}
			}
		}
		else
		{

			ShowTip(Language::getStringByKey("RoomInfoEmpty"), [=](){
				PKNotificationCenter::getInstance()->postNotification("triggerSwitchGameType", String::create(toString(GAMETYPE::NO)));
			},nullptr,true);
			LayerLoading::CloseWithTip("onMsgRoomTypesInfo error",true);
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onMsgRoomTypesInfo", "code error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});

}



//从服务端返回的json数据，生成room types
bool DZControl::InitRoomTypesInfoData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	if (jsonMsg->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	for (auto info : RoomTypeInfos)
		delete info.second;
	RoomTypeInfos.clear();
	int size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < size; i++)
	{
		cJSON *typeinfo, *type_id, *ante, *bet_seconds, *card_count, *init_chip, *next_game_seconds, *name, *fee_rate;
		if (!(typeinfo = cJSON_GetArrayItem(jsonMsg, i)) || typeinfo->type != cJSON_Object
			|| !(type_id = cJSON_GetObjectItem(typeinfo, "type")) || type_id->type != cJSON_Number
			|| !(ante = cJSON_GetObjectItem(typeinfo, "ante")) || ante->type != cJSON_Number
			|| !(bet_seconds = cJSON_GetObjectItem(typeinfo, "bet_seconds")) || bet_seconds->type != cJSON_Number
			|| !(card_count = cJSON_GetObjectItem(typeinfo, "card_count")) || card_count->type != cJSON_Number
			|| !(init_chip = cJSON_GetObjectItem(typeinfo, "init_chip")) || init_chip->type != cJSON_Number
			|| !(name = cJSON_GetObjectItem(typeinfo, "name")) || name->type != cJSON_String
			|| !(next_game_seconds = cJSON_GetObjectItem(typeinfo, "next_game_seconds")) || next_game_seconds->type != cJSON_Number
			|| !(fee_rate = cJSON_GetObjectItem(typeinfo, "fee_rate")) || fee_rate->type != cJSON_Number
			)
			continue;
		DzpkRoomType *type = new DzpkRoomType();
		type->type_id = type_id->valueint;
		type->ante = ante->valueint; 
		type->name = name->valuestring;
		type->next_game_seconds = next_game_seconds->valueint;
		//type->reward_seconds = reward_seconds->valueint;
		type->bet_seconds = bet_seconds->valueint;
		type->card_count = card_count->valueint;
		type->next_game_seconds = next_game_seconds->valueint;
		type->init_chip = init_chip->valueint;
		type->fee_rate = fee_rate->valuedouble;
		RoomTypeInfos[type->type_id] = type;
	}


	cJSON_Delete(jsonMsg);
	return true;
}


void DZControl::onPushGameStatus(Ref *pSender){

	// 获取所有房间类型信息
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			//成功获取所有房间类型信息数据
			if (!dealGameStatusData(msg->data)){
				CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushGameStatus", "onPushGameStatus error.", msg->code, msg->data.c_str());
			}
			else{
				//成功连接,开始登录
				//Api::Game::mo_dzpk_join_game(1); //for test
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushGameStatus", "onPushGameStatus error.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});



}



bool DZControl::dealGameStatusData(std::string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	if (jsonMsg->type != cJSON_Array){
		cJSON_Delete(jsonMsg);
		return false;
	}
	

	cJSON *participants_info, *round_info, *bet_player,*token, *public_cards, *big_blind, *small_blind, *round_num, *user_id, *end_time, *action, *round_amount, *nickname,*pos,*avatar;

	if (!(participants_info = cJSON_GetArrayItem(jsonMsg,0)) || participants_info->type != cJSON_Array
		|| !(round_info = cJSON_GetArrayItem(jsonMsg, 1)) || round_info->type != cJSON_Array
		|| !(bet_player = cJSON_GetArrayItem(jsonMsg, 2)) || bet_player->type != cJSON_Array
		|| !(token = cJSON_GetArrayItem(jsonMsg, 3)) || token->type != cJSON_String
		){
		cJSON_Delete(jsonMsg);
		return false;
	}

	this->gameToken = token->valuestring;

	if (!this->gameToken.empty()){
		
	}

	if (!(round_num = cJSON_GetArrayItem(round_info, 0)) || round_num->type != cJSON_Number
		|| !(public_cards = cJSON_GetArrayItem(round_info, 1)) || public_cards->type != cJSON_String
		|| !(big_blind = cJSON_GetArrayItem(round_info, 2)) || big_blind->type != cJSON_Number
		|| !(small_blind = cJSON_GetArrayItem(round_info, 3)) || small_blind->type != cJSON_Number
		){
		cJSON_Delete(jsonMsg);
		return false;
	}
	RoundInfo *roundInfo = new RoundInfo();
	roundInfo->round_num = round_num->valueint;
	roundInfo->public_cards = public_cards->valuestring;
	roundInfo->big_blind = big_blind->valueint;
	roundInfo->small_blind = small_blind->valueint;


	if (!(user_id = cJSON_GetArrayItem(bet_player, 0)) || user_id->type != cJSON_Number
		|| !(round_num = cJSON_GetArrayItem(bet_player, 1)) || round_num->type != cJSON_Number
		|| !(end_time = cJSON_GetArrayItem(bet_player, 2)) || end_time->type != cJSON_Number

		){
		cJSON_Delete(jsonMsg);
		return false;
	}

	BetBegin *betBegin = new BetBegin();
	betBegin->round_num = round_num->valueint;
	betBegin->user_id = user_id->valueint;
	betBegin->end_time = end_time->valueint;

	vector<BetInfo*> vec;

	int my_user_id = UserControl::GetInstance()->GetUserData()->user_id;
	
	int size = cJSON_GetArraySize(participants_info);
	for (int i = 0; i < size; i++)
	{
		cJSON *player_info, *user_id, *bet_amount, *chip, *status;
		if (!(player_info = cJSON_GetArrayItem(participants_info, i)) || player_info->type != cJSON_Array
			|| !(user_id = cJSON_GetArrayItem(player_info, 0)) || user_id->type != cJSON_Number
			|| !(bet_amount = cJSON_GetArrayItem(player_info, 1)) || bet_amount->type != cJSON_Number
			|| !(chip = cJSON_GetArrayItem(player_info, 2)) || chip->type != cJSON_Number
			|| !(status = cJSON_GetArrayItem(player_info, 3)) || status->type != cJSON_Number
			|| !(round_amount = cJSON_GetArrayItem(player_info, 4)) || round_amount->type != cJSON_Number
			|| !(nickname = cJSON_GetArrayItem(player_info, 5)) || nickname->type != cJSON_String
			|| !(avatar = cJSON_GetArrayItem(player_info, 6)) || avatar->type != cJSON_Number
			|| !(pos = cJSON_GetArrayItem(player_info, 7)) || pos->type != cJSON_Number
			|| !(action = cJSON_GetArrayItem(player_info, 8)) || action->type != cJSON_Number
			)
			continue;

		if (RoomPlayers.find(user_id->valueint) != RoomPlayers.end()){
			RoomPlayer *roomPlayer = RoomPlayers[user_id->valueint];
			roomPlayer->status = (TPParticipantStatus)status->valueint;
			roomPlayer->pos = pos->valueint;
		}
		else{
			CCLOG("can`t  find user_id = %d RoomPlayer !", user_id->valueint);
		}
		
		
		



		//RoomSitInfo* sitInfo = new RoomSitInfo;
		//sitInfo->pos = pos->valueint;
		//sitInfo->user_id = user_id->valueint;
		//sitInfos[sitInfo->pos] = sitInfo;

		//if (my_user_id == sitInfo->user_id)//玩家自己刚进来坐下
		//{
		//	my_pos = sitInfo->pos;
		//}


		BetInfo * betInfo = new BetInfo;
		betInfo->bet_amount = bet_amount->valueint;
		betInfo->chip = chip->valueint;
		betInfo->status = status->valueint;
		betInfo->action = action->valueint;
		
		betInfo->user_id = user_id->valueint;
		betInfo->round_amount = round_amount->valueint;


		vec.push_back(betInfo);
	}
	cJSON_Delete(jsonMsg);

	clearRoomTempInfo();

	

	if (roomTempInfo == nullptr){
		roomTempInfo = new RoomTempInfo();
	}

	roomTempInfo->betBegin = betBegin;
	roomTempInfo->betInfos = vec;
	roomTempInfo->roundInfo = roundInfo;

	//if (roomView)
	//{
	//	((SceneDzRoom*)roomView)->updateSitInfos();//先更新一轮座次
	//	((SceneDzRoom*)roomView)->updateRoomInfo(roundInfo, vec, betBegin);
	//}

	//delete roundInfo;
	//delete betBegin;

	//for (BetInfo* betInfo : vec){ //对于 每个坐着的
	//	delete betInfo;
	//}

	return true;
}

void DZControl::checkAndUpdateRoomTempInfo()
{
	if (roomTempInfo == nullptr) return;
	
	if (roomView)
	{
		RoundInfo * roundInfo = roomTempInfo->roundInfo;

		BetBegin* betBegin = roomTempInfo->betBegin;

		vector<BetInfo*> betInfos = roomTempInfo->betInfos;

		((SceneDzRoom*)roomView)->updateRoomInfo(roundInfo, betInfos, betBegin);
	}

	clearRoomTempInfo();
}

void DZControl::clearRoomTempInfo(){

	if (roomTempInfo == nullptr) return;

	RoundInfo * roundInfo = roomTempInfo->roundInfo;
	if (roundInfo != nullptr){
		delete roundInfo;
	}

	BetBegin* betBegin = roomTempInfo->betBegin;

	if (betBegin != nullptr){
		delete betBegin;
	}

	vector<BetInfo*> betInfos = roomTempInfo->betInfos;

	if (betInfos.size() >0){
		for (BetInfo* betInfo : betInfos){ //对于 每个坐着的
			delete betInfo;
		}
		betInfos.clear();
	}

	delete roomTempInfo;

	roomTempInfo = nullptr;
}


void DZControl::onPushChipFresh(Ref *pSender){
	//暂时不需要
}
void DZControl::onPushBetInfo(Ref *pSender){


	//房间玩家数据回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0 && dealBetInfo(msg->data))
		{

		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBetInfo", "get sit data fail.", msg->code, msg->data.c_str());
		}
	
		msg->release();		//清除msg数据，以免内存泄漏
	});

}


bool DZControl::dealBetInfo(string jsonData){
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	cJSON  *bet_amount, *user_id, *chip, *status, *action, *round_amount;

	if (!(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(bet_amount = cJSON_GetArrayItem(jsonMsg, 1)) || bet_amount->type != cJSON_Number
		|| !(chip = cJSON_GetArrayItem(jsonMsg, 2)) || chip->type != cJSON_Number
		|| !(status = cJSON_GetArrayItem(jsonMsg, 3)) || status->type != cJSON_Number
		|| !(action = cJSON_GetArrayItem(jsonMsg, 4)) || action->type != cJSON_Number
		|| !(round_amount = cJSON_GetArrayItem(jsonMsg, 5)) || round_amount->type != cJSON_Number
		){
		cJSON_Delete(jsonMsg);
		return false;
	}

    BetInfo * betInfo = new BetInfo;
	betInfo->bet_amount = bet_amount->valueint;
	betInfo->chip = chip->valueint;
	betInfo->status = status->valueint;
	betInfo->user_id = user_id->valueint;
	betInfo->action = action->valueint;
	betInfo->round_amount = round_amount->valueint;

	CCLOG("bet_amount:%d chip:%d status:%d user_id:%d action:%d round_amount:%d ", betInfo->bet_amount, betInfo->chip, betInfo->status, betInfo->user_id, betInfo->action, betInfo->round_amount);

	if (roomView){
		((SceneDzRoom*)roomView)->updateBetInfo(betInfo);
	}
	delete betInfo;

	cJSON_Delete(jsonMsg);
	return true;
}

void DZControl::onPushFlopInfo(Ref *pSender){

	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0 )
		{
			dealFlopInfo(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushFlopInfo", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealFlopInfo(string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	
	cJSON  *hand_cards;

	if (!(hand_cards = cJSON_GetArrayItem(jsonMsg, 0)) || hand_cards->type != cJSON_String){
		cJSON_Delete(jsonMsg);
		return false;
	}
	if (roomView){
		((SceneDzRoom*)roomView)->updateFlopInfo(hand_cards->valuestring);
	}
	cJSON_Delete(jsonMsg);
	return true;
}

void DZControl::onPushRoundInfo(Ref *pSender){

	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0 )
		{
			dealRoundInfo(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushRoundInfo", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealRoundInfo(string jsonData)
{
	if (jsonData.empty())
		return false;

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
    
	cJSON  *round_num, *public_cards, *big_blind, *small_blind;

	if (!(round_num = cJSON_GetArrayItem(jsonMsg, 0)) || round_num->type != cJSON_Number
		|| !(public_cards = cJSON_GetArrayItem(jsonMsg, 1)) || public_cards->type != cJSON_String
		|| !(big_blind = cJSON_GetArrayItem(jsonMsg, 2)) || big_blind->type != cJSON_Number
		|| !(small_blind = cJSON_GetArrayItem(jsonMsg, 3)) || small_blind->type != cJSON_Number
		){
		cJSON_Delete(jsonMsg);
		return false;
	}
	RoundInfo *roundInfo = new RoundInfo();
	roundInfo->round_num = round_num->valueint;
	roundInfo->public_cards = public_cards->valuestring;
	roundInfo->big_blind = big_blind->valueint;
	roundInfo->small_blind = small_blind->valueint;

	if (roomView)
	{
		((SceneDzRoom*)roomView)->updateRoundInfo(roundInfo);
	}

	delete roundInfo;

	cJSON_Delete(jsonMsg);
	return true;
}


void DZControl::onPushGameResult(Ref *pSender){
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0  )
		{
			dealGameResult(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::dealGameResult", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealGameResult(string jsonData){
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	if (jsonMsg->type != cJSON_Array){
		cJSON_Delete(jsonMsg);
		return false;
	}
	
	vector<PlayerResult*> vec;

	int size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < size; i++)
	{
		cJSON *player_result, *user_id, *hand_cards, *poker_type, *bonus, *chip, *bet_amount,*score,*isWiner;
		if (!(player_result = cJSON_GetArrayItem(jsonMsg, i)) || player_result->type != cJSON_Array
			|| !(user_id = cJSON_GetArrayItem(player_result, 0)) || user_id->type != cJSON_Number
			|| !(hand_cards = cJSON_GetArrayItem(player_result, 1)) || hand_cards->type != cJSON_String
			|| !(poker_type = cJSON_GetArrayItem(player_result, 2)) || poker_type->type != cJSON_Number
			|| !(bonus = cJSON_GetArrayItem(player_result, 3)) || bonus->type != cJSON_Number
			|| !(chip = cJSON_GetArrayItem(player_result, 4)) || chip->type != cJSON_Number
			|| !(bet_amount = cJSON_GetArrayItem(player_result, 5)) || bet_amount->type != cJSON_Number
			|| !(score = cJSON_GetArrayItem(player_result, 6)) || score->type != cJSON_Number
			|| !(isWiner = cJSON_GetArrayItem(player_result, 7)) || (isWiner->type != cJSON_False && isWiner->type != cJSON_True)
			)
			continue;
		PlayerResult *playerResult = new PlayerResult();
		playerResult->user_id = user_id->valueint;
		playerResult->hand_cards = hand_cards->valuestring;
		playerResult->poker_type = poker_type->valueint;
		playerResult->bonus = bonus->valueint;
		playerResult->chip = chip->valueint;
		playerResult->bet_amount = bet_amount->valueint;
		playerResult->score = score->valueint;
		playerResult->isWiner = isWiner->valueint>=1;
		vec.push_back(playerResult);
	}
	cJSON_Delete(jsonMsg);
	

	if (roomView)
	{
		((SceneDzRoom*)roomView)->updateGameResults(vec);
		
	}
	
	//释放
	for (PlayerResult* playerResult : vec){
		delete playerResult;
	}
	return true;
}

void DZControl::onPushNextTime(Ref *pSender){
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			dealNextTime(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::dealGameResult", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealNextTime(string jsonData){
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	

	if (roomView)
	{
		((SceneDzRoom*)roomView)->finishGame();
	}

	cJSON_Delete(jsonMsg);
	return true;
}


void DZControl::onPushBeginBet(Ref *pSender){

	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			dealBeginBet(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBeginBet", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealBeginBet(string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	cJSON  *user_id, *round_num, *end_time;

	if (!(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(round_num = cJSON_GetArrayItem(jsonMsg, 1)) || round_num->type != cJSON_Number
		|| !(end_time = cJSON_GetArrayItem(jsonMsg, 2)) || end_time->type != cJSON_Number

		){
		cJSON_Delete(jsonMsg);
		return false;
	}

	BetBegin *betBegin = new BetBegin();
	betBegin->round_num = round_num->valueint;
	betBegin->user_id = user_id->valueint;
	betBegin->end_time = end_time->valueint;
	
	if (roomView)
	{
		((SceneDzRoom*)roomView)->updateBetBegin(betBegin);
	}

	delete betBegin;

	cJSON_Delete(jsonMsg);
	return true;
}

void DZControl::onPushGameStart(Ref *pSender){
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			if (roomView)
			{
				
				((SceneDzRoom*)roomView)->setGameStart();

				 checkAndDelRoomPlayers();
				
			}
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBeginBet", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});

	//暂时不处理
}


void DZControl::onTpBetCallback(Ref *pSender){


	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		
		if (msg->code == 0)
		{

		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBeginBet", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}

void DZControl::onPushChipFreshCallback(Ref *pSender){


	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{

		if (msg->code == 0)
		{
			dealChipFresh(msg->data);
		}
		else
		{
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBeginBet", "get sit data fail.", msg->code, msg->data.c_str());
		}

		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealChipFresh(string jsonData)
{
	if (jsonData.empty())
		return false;
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	cJSON  *user_id, *chip;

	if (!(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(chip = cJSON_GetArrayItem(jsonMsg, 1)) || chip->type != cJSON_Number
		){
		cJSON_Delete(jsonMsg);
		return false;
	}

	ChipInfo *chipInfo = new ChipInfo();
	chipInfo->chip = chip->valueint;
	chipInfo->user_id = user_id->valueint;

	if (roomView)
	{
		((SceneDzRoom*)roomView)->updatePlayerChip(chipInfo);
	}
	delete chipInfo;

	cJSON_Delete(jsonMsg);
	return true;
}


void DZControl::onChatCallBack(Ref *pSender)
{
	//聊天回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0 )
		{
			dealChatData(msg->data);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onChatCallBack", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealChatData(string jsonData )
{
	if (jsonData.empty()) {
		return false;
	}
	
	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON  *user_id, *msg;
	int size;
	if (jsonMsg->type != cJSON_Array || !((size =cJSON_GetArraySize(jsonMsg)) == 2)
		|| !(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(msg = cJSON_GetArrayItem(jsonMsg, 1)) || msg->type != cJSON_String)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	DzChatData *chatData = new DzChatData;
	chatData->user_id = user_id->valueint;
	chatData->msg = msg->valuestring;

	if (roomView)
	{
		((SceneDzRoom*)roomView)->updateChatData(chatData);
	}

	delete chatData;
	cJSON_Delete(jsonMsg);
	
	return true;
}


void DZControl::onPushBalanceInfo(Ref *pSender)
{
	//聊天回调函数
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		if (msg->code == 0)
		{
			dealBalanceInfo(msg->data);
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBlanceInfo", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealBalanceInfo(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON  *user_id, *blance;
	int size;
	if (jsonMsg->type != cJSON_Array || !((size = cJSON_GetArraySize(jsonMsg)) == 2)
		|| !(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(blance = cJSON_GetArrayItem(jsonMsg, 1)) || blance->type != cJSON_Number)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	//更新bill
	BillControl::GetInstance()->GetBalanceData(0)->balance = blance->valueint;

	if (roomView)
	{
		((SceneDzRoom*)roomView)->UpdateBalance(nullptr);
	}

	cJSON_Delete(jsonMsg);

	return true;
}


void DZControl::onTriggerCreateRoom(Ref* pSender)
{

	String* type_key = (String*)pSender;
	if (!type_key) return; 

	gameRoomType = type_key->intValue();

	LayerLoading::Wait();
	

	if (!Api::Game::mo_dzpk_create_room(type_key->intValue()))
	{
		LayerLoading::CloseWithTip("mo_dzpk_create_room net error");
	}
}

void DZControl::onTriggerJoinCreateRoom(Ref* pSender)
{

	LayerLoading::Wait();

	if (!Api::Game::mo_dzpk_join_room(gameToken.c_str()))
	{
		LayerLoading::CloseWithTip("mo_dzpk_join_room net error");
	}
}



void DZControl::onCreateRoom(Ref* pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealCreateRoom(msg->data)){
				PKNotificationCenter::getInstance()->postNotification("CreateRoomSucc", nullptr); //通知创建成功
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBlanceInfo", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealCreateRoom(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	return true;
}

void DZControl::onJoinRoom(Ref* pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealJoinRoom(msg->data)){

			}
		}
		else
		{
			setRoomInfo();//重置房间信息
			LayerLoading::CloseWithTip("room not exist");
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBlanceInfo", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealJoinRoom(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	return true;
}



void DZControl::onPushSngRoomType(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushSngRoomType(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSngRoomType", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}



bool DZControl::dealPushSngRoomType(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	cJSON *room_info,*apply_cost, *apply_currency_type, *bet_seconds, *card_count, *next_game_seconds, *open_players, *open_time, *reward_amount, *reward_currency_type,
		*total_sessions, *type_id,*name;

	int size = cJSON_GetArraySize(jsonMsg);
	for (int i = 0; i < size; i++)
	{

		if (!(room_info = cJSON_GetArrayItem(jsonMsg, i)) || room_info->type != cJSON_Object
			|| !(apply_cost = cJSON_GetObjectItem(room_info, "apply_cost")) || apply_cost->type != cJSON_Number
			|| !(apply_currency_type = cJSON_GetObjectItem(room_info, "apply_currency_type")) || apply_currency_type->type != cJSON_Number
			|| !(card_count = cJSON_GetObjectItem(room_info, "card_count")) || card_count->type != cJSON_Number
			|| !(bet_seconds = cJSON_GetObjectItem(room_info, "bet_seconds")) || bet_seconds->type != cJSON_Number
			|| !(next_game_seconds = cJSON_GetObjectItem(room_info, "next_game_seconds")) || next_game_seconds->type != cJSON_Number
			|| !(open_players = cJSON_GetObjectItem(room_info, "open_players")) || open_players->type != cJSON_Number
			|| !(reward_amount = cJSON_GetObjectItem(room_info, "reward_amount")) || reward_amount->type != cJSON_Number
			|| !(reward_currency_type = cJSON_GetObjectItem(room_info, "reward_currency_type")) || reward_currency_type->type != cJSON_Number
			|| !(total_sessions = cJSON_GetObjectItem(room_info, "total_sessions")) || total_sessions->type != cJSON_Number
			|| !(type_id = cJSON_GetObjectItem(room_info, "type_id")) || type_id->type != cJSON_Number
			|| !(name = cJSON_GetObjectItem(room_info, "name")) || name->type != cJSON_String
			|| !(open_time = cJSON_GetObjectItem(room_info, "open_time")) || open_time->type != cJSON_String
			){
			cJSON_Delete(jsonMsg);
			return false;

		}else{
			SNGRoomType *sNGRoomType = new SNGRoomType();
			sNGRoomType->apply_cost = apply_cost->valueint;
			sNGRoomType->apply_currency_type = apply_currency_type->valueint;
			sNGRoomType->bet_seconds = bet_seconds->valueint;
			sNGRoomType->next_game_seconds = next_game_seconds->valueint;
			sNGRoomType->open_players = open_players->valueint;
			sNGRoomType->reward_amount = reward_amount->valueint;
			sNGRoomType->reward_currency_type = reward_currency_type->valueint;
			sNGRoomType->total_sessions = total_sessions->valueint;
			sNGRoomType->name = name->valuestring;
			sNGRoomType->open_time = open_time->valuestring;
			sNGRoomType->type_id = type_id->valueint;
			SngRoomTypeInfos[sNGRoomType->type_id] = sNGRoomType;
		}
		
	}
	if (hallView){
		((LayerDzGames*)hallView)->enableRoom(true);
	}

	cJSON_Delete(jsonMsg);

	return true;
}

void DZControl::onSngBeginApply(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealSngBeginApply(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onSngBeginApply", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealSngBeginApply(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	return true;
}


void DZControl::onPushSngGameStatus(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushSngGameStatus(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSngGameStatus", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealPushSngGameStatus(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;

	cJSON *roomStatus, *room_id, *type_id, *players, *is_running, *session_num;
	int size = cJSON_GetArraySize(jsonMsg);

	if (size <= 0){
		cJSON_Delete(jsonMsg);
		return false;
	}
	SNGRoomStatus* sNGRoomStatus = nullptr;
	for (int i = 0; i < size; i++)
	{
		if (!(roomStatus = cJSON_GetArrayItem(jsonMsg, i)) || roomStatus->type != cJSON_Array
			|| !(room_id = cJSON_GetArrayItem(roomStatus, 0)) || room_id->type != cJSON_Number
			|| !(type_id = cJSON_GetArrayItem(roomStatus, 1)) || type_id->type != cJSON_Number
			|| !(players = cJSON_GetArrayItem(roomStatus, 2)) || players->type != cJSON_Number
			|| !(is_running = cJSON_GetArrayItem(roomStatus, 3)) || is_running->type != cJSON_Number
			|| !(session_num = cJSON_GetArrayItem(roomStatus, 4)) || session_num->type != cJSON_Number
			){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{
			sNGRoomStatus = new SNGRoomStatus;
			sNGRoomStatus->room_id = room_id->valueint;
			sNGRoomStatus->type_id = type_id->valueint;
			sNGRoomStatus->players = players->valueint;
			sNGRoomStatus->session_num = session_num->valueint;
			sNGRoomStatus->is_running = is_running->valueint == 0 ? false:true;


			if (sngRoomStatusMap.find(sNGRoomStatus->room_id) != sngRoomStatusMap.end()){
				delete sngRoomStatusMap[sNGRoomStatus->room_id];
			}
			sngRoomStatusMap[sNGRoomStatus->room_id] = sNGRoomStatus;

		}
		
	}

	if (size == 1){
		((LayerDzGames*)hallView)->updateSngRoom(sNGRoomStatus);
	}
	else{
		((LayerDzGames*)hallView)->updateSngRooms(sngRoomStatusMap);
	}
	

	cJSON_Delete(jsonMsg);

	return true;
}


void DZControl::onPushSNGRoundInfo(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushSNGRoundInfo(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSngGameStatus", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool DZControl::dealPushSNGRoundInfo(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON  *session_num, *round_num,*ante;
	int size;
	if (jsonMsg->type != cJSON_Array || !((size = cJSON_GetArraySize(jsonMsg)) == 3)
		|| !(session_num = cJSON_GetArrayItem(jsonMsg, 0)) || session_num->type != cJSON_Number
		|| !(round_num = cJSON_GetArrayItem(jsonMsg, 1)) || round_num->type != cJSON_Number
		|| !(ante = cJSON_GetArrayItem(jsonMsg, 2)) || ante->type != cJSON_Number
		)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{
		if (sngRoundInfo == nullptr){
			sngRoundInfo = new SNGRoundInfo;
		}
		sngRoundInfo->session_num = session_num->valueint;
		sngRoundInfo->ante = ante->valueint;
		sngRoundInfo->round_num = round_num->valueint;
	}
	
	cJSON_Delete(jsonMsg);

	return true;
}

void DZControl::onPushBalanceUpdateEx(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushBalanceUpdateEx(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushBalanceUpdateEx", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}


bool DZControl::dealPushBalanceUpdateEx(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON  *user_id, *currency_type, *balance;
	int size;
	if (jsonMsg->type != cJSON_Array || !((size = cJSON_GetArraySize(jsonMsg)) == 3)
		|| !(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(currency_type = cJSON_GetArrayItem(jsonMsg, 1)) || currency_type->type != cJSON_Number
		|| !(balance = cJSON_GetArrayItem(jsonMsg, 2)) || balance->type != cJSON_Number
		)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{
		//  处理  更新 卡..
		BalanceData *data = BillControl::GetInstance()->GetBalanceData(currency_type->valueint);
		data->balance = balance->valueint;
	}

	cJSON_Delete(jsonMsg);

	return true;
}



void DZControl::onPushSNGSessionResult(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushSNGSessionResult(msg->data)){

			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSNGSessionResult", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}
bool DZControl::dealPushSNGSessionResult(string jsonData)
{
	if (jsonData.empty()) {
		return false;
	}

	cJSON *jsonMsg = cJSON_Parse(jsonData.c_str());
	if (jsonMsg == nullptr)
		return false;
	cJSON  *user_id, *avatar, *nickname,*currency_type,*reward_amount;
	int size;
	if (jsonMsg->type != cJSON_Array || !((size = cJSON_GetArraySize(jsonMsg)) == 5)
		|| !(user_id = cJSON_GetArrayItem(jsonMsg, 0)) || user_id->type != cJSON_Number
		|| !(avatar = cJSON_GetArrayItem(jsonMsg, 1)) || avatar->type != cJSON_Number
		|| !(nickname = cJSON_GetArrayItem(jsonMsg, 2)) || nickname->type != cJSON_String
		|| !(currency_type = cJSON_GetArrayItem(jsonMsg, 3)) || currency_type->type != cJSON_Number
		|| !(reward_amount = cJSON_GetArrayItem(jsonMsg, 4)) || reward_amount->type != cJSON_Number
		)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	else{
		//  处理  更新 卡..

		if (UserControl::GetInstance()->GetUserData()->user_id == user_id->valueint)
		{
			SNGRoomType *roomType = DZControl::GetInstance()->SngRoomTypeInfos[current_sng_type_id];
			LayerSNGTip * sngTip = LayerSNGTip::create();
			string des = StringUtils::format("%d%s", reward_amount->valueint, getCurrencyTypeStr(currency_type->valueint).c_str());

			
			string tip = StringUtils::format(Language::getStringByKey("SNSAwardTip"), getRoomTypeStr(roomType->apply_currency_type).c_str(), des.c_str());
			sngTip->set(tip, "label_confirm");
			sngTip->handler = [=](){
				((SceneDzRoom*)roomView)->closeRoom();
			};
			sngTip->show();
		}
	}

	cJSON_Delete(jsonMsg);

	return true;
}


void DZControl::onPushBuyChip(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushBuyChip(msg->data)){
				buyChipTimes = atoi(msg->data.c_str());
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSNGSessionResult", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool DZControl::dealPushBuyChip(string jsonData)
{
	return true;
}

void  DZControl::onPushBuyChipTimes(Ref *pSender)
{
	if (pSender == nullptr)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();		//因为要使用主线程更新UI，所以msg需要retain，以免在子线程中被清除
	//使用主线程调用
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		LayerLoading::Close();
		if (msg->code == 0)
		{
			if (dealPushBuyChipTimes(msg->data)){
				buyChipTimes = atoi(msg->data.c_str());
				if (roomView){
					((SceneDzRoom*)roomView)->enableExchange(buyChipTimes > 0);
				}
			}
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "DZControl::onPushSNGSessionResult", "init chat data fail.", msg->code, msg->data.c_str());
		}
		msg->release();		//清除msg数据，以免内存泄漏
	});
}

bool  DZControl::dealPushBuyChipTimes(string jsonData)
{
	return true;
}

string DZControl::getRoomTypeStr(int roomType)
{
	if (roomType == 1){
		return Language::getStringByKey("Master");
	}
	else if (roomType == 2){
		return Language::getStringByKey("Elite");
	}
	return "";
}

string DZControl::getCurrencyTypeStr(int type)
{
	if (type == 0){
		return Language::getStringByKey("Coin");
	}
	else if (type == 1){
		return Language::getStringByKey("GoldCard");
	}
	else if (type == 2){
		return Language::getStringByKey("SilverCard");
	}
	else if (type == 3){
		return Language::getStringByKey("DiamondCard");
	}
	return "";
}


int DZControl::getAnte()
{
	if (isSNG){
		if (sngRoundInfo){
			return sngRoundInfo->ante;
		}
		else{
			return 20;
		}
		
	}
	else{
		return RoomTypeInfos[gameRoomType]->ante;
	}
}
int DZControl::getBetSeconds()
{
	if (isSNG){
		return SngRoomTypeInfos[current_sng_type_id]->bet_seconds;
	}
	else{
		return RoomTypeInfos[gameRoomType]->bet_seconds;
	}
}
int DZControl::getInitChip()
{
	if (isSNG){
		return 1000;
	}
	else{
		return RoomTypeInfos[gameRoomType]->init_chip;
	}
}

float DZControl::getFeeRate()
{
	if (isSNG){
		return 0;
	}
	else{
		return RoomTypeInfos[gameRoomType]->fee_rate;
	}
}


void DZControl::closeRoom()
{
	if (this->roomView){
		((SceneDzRoom*)this->roomView)->closeRoom();
	}
}

void DZControl::setDzpkRoomView(Node * roomView){
	this->roomView = roomView;

	if (roomView == nullptr){ //退出房间时,清掉数据
		disposeRoom();
	}
}


void DZControl::disposeRoom()
{
	for (auto item2 : RoomPlayers){
		delete item2.second;
	}
	RoomPlayers.clear();
	for (auto item3 : sitInfos){
		delete item3.second;
	}
	sitInfos.clear();

	isSNG = false;
	//重置
	my_pos = 0;
	this->gameToken = "";
	this->roomKind = DZROOMKIND::DZ_GAME_ROOM;
	this->gameRoomType = 0;
}


void DZControl::disposeHall()
{
	disposeRoom();
	disposeNoticeListeners();
	for (auto item : RoomTypeInfos){
		delete item.second;
	}
	RoomTypeInfos.clear();

	
	for (auto item : SngRoomTypeInfos){
		delete item.second;
	}
	SngRoomTypeInfos.clear();


	for (auto item : sngRoomStatusMap){
		delete item.second;
	}
	sngRoomStatusMap.clear();
	
}

void DZControl::setDzpkHallView(Node * busView){
	this->hallView = busView;

	if (this->hallView == nullptr){//置空时
		disposeHall();
	}
	else{
		addNoticeListeners();
	}
}

bool DZControl::isHallView()
{
	return hallView != nullptr;
}

DZControl::~DZControl()
{
	// remove callback func
	disposeHall();
	disposeRoom();
}


DZControl* DZControl::GetInstance()
{
	if (m_pInstance == nullptr)  //判断是否第一次调用
	{
		m_pInstance = new DZControl();
		m_pInstance->Init();
	}
	return m_pInstance;
}
