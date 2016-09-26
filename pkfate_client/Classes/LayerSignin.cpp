#include "LayerSignin.h"
#include "cocostudio/CocoStudio.h"
#include "BillControl.h"
#include "PKNotificationCenter.h"
#include "AvatarControl.h"
#include "UserControl.h"
#include "network/HttpClient.h"
#include "comm.h"
#include "SoundControl.h"
#include "MailControl.h"
const string gNameRewardsType[] = { "Chip", "GoldCard" };

LayerSignin::LayerSignin() :_nodeSigninRoot(NULL), _nodeObtainmentRoot(NULL)
{

}

bool LayerSignin::init()
{
	if (!Layer::init())
		return false;

	_nodeSigninRoot = CSLoader::createNode("LayerSignin.csb");
	if (!_nodeSigninRoot)
		return false;
	addChild(_nodeSigninRoot);

	Node *pNodeTask = _nodeSigninRoot->getChildByName<Button*>("btnGet");
	if (!pNodeTask)
		return false;
	Button *pBtnTask = pNodeTask->getChildByName<Button*>("button");
	if (!pBtnTask)
		return false;
	pBtnTask->addClickEventListener(CC_CALLBACK_1(LayerSignin::onTakeButtonClickEvent, this));

	_nodeObtainmentRoot = CSLoader::createNode("LayerSigninObtainment.csb");
	if (!_nodeObtainmentRoot)
		return false;
	addChild(_nodeObtainmentRoot);
	_nodeObtainmentRoot->setVisible(false);

	Layout *pLayoutMask = _nodeObtainmentRoot->getChildByName<Layout*>("bgMask");
	pLayoutMask->addClickEventListener([=](Ref *ref){
		this->removeFromParentAndCleanup(true);
	});

	return true;
}

LayerSignin::~LayerSignin()
{

}

void LayerSignin::onExit()
{
	Node::onExit();

	PKNotificationCenter::getInstance()->postNotification("TriggerRequestNotice");
	PKNotificationCenter::getInstance()->postNotification("ClearLayerSigninNotification");
}

void LayerSignin::setSigninDayNumber(int day)
{
	if (!_nodeSigninRoot)
		return;

	Text* pText = _nodeSigninRoot->getChildByName<Text*>("txtNumberSignin");
	//todo
	//string temp = Language::getStringByKey("NumberSignin");

	string temp;
	std::string langType = UserDefault::getInstance()->getStringForKey("LangType");
	if (langType == "zh")
	{
		temp = "已连续签到%d天";
	}
	else
	{
		temp = "Consecutive sign in %d days";
	}

	string txt = StringUtils::format(temp.c_str(), day);
	pText->setString(txt);
}

bool LayerSignin::refreshDayRewards(SigninControl::SigninRewardsMap map)
{
	if (map.size() != 7 || !_nodeSigninRoot)
		return false;

	for (int i = 0; i < map.size(); i++)
	{
		string nameFileBigOrSmall;
		if (i != map.size()-1)
			nameFileBigOrSmall = "Small";
		else
			nameFileBigOrSmall = "Big";

		string filePathEverydayRewards = StringUtils::format("signin/nodeEverydayRewards%s%d.csb", nameFileBigOrSmall.c_str(), map[i].size());

		Node *pNodeEverydayRewards = CSLoader::createNode(filePathEverydayRewards);
		if (!pNodeEverydayRewards)
			return false;
		
		for (int j = 0; j < map[i].size(); j++)
		{
			string nameFileRewards = StringUtils::format("signin/nodeRewards%s%s.csb", 
				gNameRewardsType[map[i][j].byRewardsType].c_str(), nameFileBigOrSmall.c_str());
			Node *pNodeRewards = CSLoader::createNode(nameFileRewards);
			if (!pNodeRewards)
				return false;
			Text* pText = pNodeRewards->getChildByName<Text*>("number");
			if (!pText)
				return false;
			char strNum[20];
			itoa(map[i][j].numRewards, strNum, 10);
			pText->setString(strNum);

			string nameRewardsSite = StringUtils::format("nodeRewards%d", map[i][j].byRewardsType);
			Node *pNodeRewardsSite = pNodeEverydayRewards->getChildByName(nameRewardsSite);
			if (!pNodeRewardsSite)
				return false;
			pNodeRewardsSite->addChild(pNodeRewards);
			
			string nameRewardsMask = StringUtils::format("signin/nodeEveryDayRewardsOK%s.csb", nameFileBigOrSmall.c_str());
			if (map[i][0].bTake && pNodeEverydayRewards->getChildByName(nameRewardsMask) == NULL)
			{
				Node *pNodeRewardsMask = CSLoader::createNode(nameRewardsMask);
				if (!pNodeRewardsMask)
					return false;
				pNodeRewardsMask->setName(nameRewardsMask);
				pNodeEverydayRewards->addChild(pNodeRewardsMask);
			}
		}
		string nameSigninDay = StringUtils::format("nodeSigninDay%d", i + 1);
		Node *pNodeSigninDaySite = _nodeSigninRoot->getChildByName(nameSigninDay);
		if (!pNodeSigninDaySite)
			return false;
		pNodeSigninDaySite->addChild(pNodeEverydayRewards);
	}

	return true;
}

bool LayerSignin::refreshObtainmentRoot(std::vector<SRewards> vec)
{
	if (vec.size() == 0 || !_nodeObtainmentRoot)
		return false;

	const string nameIconRewardsType[] = { "iconChip", "iconGoldCard" };

	string nameLayoutRewards = StringUtils::format("layoutRewards%d", vec.size());
	Layout* pLayoutRewards = _nodeObtainmentRoot->getChildByName<Layout*>(nameLayoutRewards);
	
	for (int i = 0; i < vec.size(); i++)
	{
		string nameIconSite, nameRewardsSite, nameRewards;
		Node *pNodeIconSite, *pNodeIconRewards, *pNodeRewardsSite, *pNodeRewards;

		nameIconSite = StringUtils::format("iconSite%d", i+1);
		nameRewardsSite = StringUtils::format("rewardsSite%d", i+1);
		pNodeIconSite = pLayoutRewards->getChildByName<Node*>(nameIconSite);
		pNodeRewardsSite = pLayoutRewards->getChildByName<Node*>(nameRewardsSite);
		nameRewards = StringUtils::format("signin/nodeRewards%s%s.csb", gNameRewardsType[vec[i].byRewardsType].c_str(), "Big");
		pNodeRewards = CSLoader::createNode(nameRewards);
		pNodeIconRewards = _nodeObtainmentRoot->getChildByName<Node*>(nameIconRewardsType[vec[i].byRewardsType]);

		if (!pNodeIconSite || !pNodeRewardsSite || !pNodeRewards || !pNodeIconRewards)
			return false;

		pNodeIconRewards->setVisible(true);
		pNodeIconRewards->setPosition(pNodeIconSite->getPosition());
		pNodeRewardsSite->addChild(pNodeRewards);

		Text* pText = pNodeRewards->getChildByName<Text*>("number");
		if (!pText)
			return false;
		char strNum[20];
		itoa(vec[i].numRewards, strNum, 10);
		pText->setString(strNum);
	}

	pLayoutRewards->setVisible(true);
	return true;
}

bool LayerSignin::showLayoutObtainment()
{
	if (!_nodeObtainmentRoot || !_nodeSigninRoot)
		return false;

	_nodeSigninRoot->setVisible(false);
	_nodeObtainmentRoot->setVisible(true);
	return true;
}

void LayerSignin::onTakeButtonClickEvent(Ref *ref)
{
	PKNotificationCenter::getInstance()->postNotification("RewardSigninTaskNotification");
}