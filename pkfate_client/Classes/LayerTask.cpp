#include "LayerTask.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "LayerDuang.h"
TaskItem::TaskItem()
{
	Node *_nodeRoot = CSLoader::createNode("task/TaskItem.csb");
	addChild(_nodeRoot);
	_taskName = (Text*)_nodeRoot->getChildByName("task_name");
	_taskProgress = (Sprite*)_nodeRoot->getChildByName("task_progress");
	_taskAward = (Text*)_nodeRoot->getChildByName("task_award");
	_btnStatus = (Button*)_nodeRoot->getChildByName("btn_status");

}


void TaskItem::setProperty(TaskData *taskData)
{
	this->taskData = taskData;
	if (taskData->staticData){
		_taskName->setString(Language::getStringByKey(taskData->staticData->task_name.c_str()));
		if (taskData->daily){
			_taskAward->setString(StringUtils::format("200-%d",taskData->staticData->coin));
		}
		else{
			_taskAward->setString(toString(taskData->staticData->coin));
		}
		
	}

    switch (taskData->task_status)
	{  
	case 0: //未完成
	{  
		_taskProgress->setTexture(Director::getInstance()->getTextureCache()->addImage("task/task_not_finish.png"));
		_btnStatus->setEnabled(false);
		_btnStatus->setBright(false);
		_btnStatus->setTitleText(Language::getStringByKey("NotFinish"));
		break; 
	}
	case 1:
	{   
		_taskProgress->setTexture(Director::getInstance()->getTextureCache()->addImage("task/task_finish.png"));
		_btnStatus->setEnabled(true);
		_btnStatus->setBright(true);
		_btnStatus->setTitleText(Language::getStringByKey("Get"));
		_btnStatus->addClickEventListener([=](Ref* ref){
			SoundControl::PlayGameEffect(EFFECTSOUND::EFFECT_BUTTON);
		   //发送通知领取奖励
			_btnStatus->setEnabled(false);
			_btnStatus->setBright(false);
			PKNotificationCenter::getInstance()->postNotification("GetRookieAwardClick", this);
		});
		break;
	}
	case 2:
	{  
		_btnStatus->setEnabled(false);
		_btnStatus->setBright(false);
		_btnStatus->setVisible(false);
		break;
	}
		default:
			break;
	}
}

TaskItem::~TaskItem()
{

}


TaskData* TaskItem::getTaskData()
{
	return taskData;
}

Button* TaskItem::getStatusBtn()
{
	return _btnStatus;
}

LayerTask::LayerTask(){

}

bool LayerTask::init()
{
	if (!Layer::init())
		return false;
	
	Node *_nodeRoot = CSLoader::createNode("task/LayerTask.csb");
	addChild(_nodeRoot);
	if (!_nodeRoot)
		return false;
	Layout *touch_layer;
	if (!(touch_layer = _nodeRoot->getChildByName<Layout*>("touch_layer")))
		return false;

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!touch_layer->getBoundingBox().containsPoint(touch->getLocation()))
			this->scheduleOnce([=](float dt){
			
			CloseWithAction(this);

			LayerDuang::panelStatus();
			LayerDuang::uiGameMenu();
			LayerDuang::dzGamesRoom();
			LayerDuang::bjlGamesRoom();
			LayerDuang::bjlPanelLobby();
			LayerDuang::dzPanelLobby();
		
		}, 0.1f, "DelayExit");	//延迟调用退出，以免touch事件传送到下层
		return true;
	};
	
	auto dispatcher = Director::getInstance()->getEventDispatcher();
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);


	Button *btn_day = _nodeRoot->getChildByName<Button*>("btn_day");
	Button *btn_achieve = _nodeRoot->getChildByName<Button*>("btn_achieve");

	btn_day->addClickEventListener([=](Ref *ref){
		btn_day->setEnabled(false);
		btn_day->setBright(false);
		btn_achieve->setEnabled(true);
		btn_achieve->setBright(true);
		selectList(true);
	});

	btn_achieve->addClickEventListener([=](Ref *ref){
		btn_achieve->setEnabled(false);
		btn_achieve->setBright(false);
		btn_day->setEnabled(true);
		btn_day->setBright(true);
		selectList(false);
	});


	 taskList = (ListView*)_nodeRoot->getChildByName("task_list");
	 taskList->setBackGroundColorOpacity(0);
	 taskList->setItemsMargin(152);
	
	
	 TaskControl::GetInstance()->setTaskView(this); //添加视图
	 
	 selectList(true);


	//test PKNotificationCenter::getInstance()->postNotification("TriggerDoneInervalTask", String::create(toString(1008)));

	 return true;
}


void LayerTask::selectList(bool daily )
{
	this->daily = daily;
	UpdateTaskList();
}


void LayerTask::UpdateTaskList()
{
	vector<TaskData*> taskDataList;
    TaskControl::GetInstance()->getTaskList(taskDataList,daily);
	taskList->removeAllItems();
	for (int i = 0, length = taskDataList.size(); i < length; i++){  //数据已顺序排列
		
		TaskItem *item = new TaskItem();// list item 重复利用
		taskList->pushBackCustomItem(item);//少了加
		
		TaskData *taskData = taskDataList[i];
		item->setProperty(taskData);
	}
	

	taskList->refreshView();//记得刷新，更新innerContainer size
	taskList->jumpToTop();
 }



LayerTask::~LayerTask()
{
	// remove callback func
	 PKNotificationCenter::getInstance()->removeAllObservers(this);
	 TaskControl::GetInstance()->setTaskView(nullptr); //释放视图
}


void LayerTask::onButtonExitClicked(Ref *ref){
	CloseWithAction(this);

}
