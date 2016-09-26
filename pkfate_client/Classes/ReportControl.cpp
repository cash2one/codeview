#include "ReportControl.h"
#include "ui/UIText.h"
#include "ui/UIImageView.h"
#include "ui/UIButton.h"
#include "ApiBill.h"
#include "ApiGame.h"
#include "UserControl.h"
#include "AvatarControl.h"
#include "LayerLoading.h"
#include "PKNotificationCenter.h"
#include "cmd.h"
#include "cocostudio/CocoStudio.h"
#include "GameControlBjl.h"
#include "LayerGameRecordBjl.h"
#include "LayerGameRecordDz.h"
using namespace Report;
//*************** Report
ReportControl::ReportControl() :
_className(""),
_totalCount(0),
_rootNode(nullptr),
_listViewReport(nullptr){
}
ReportControl::~ReportControl(){
	clear();
}
bool ReportControl::init(){
	if (!_listViewReport)
		return false;
	_listViewReport->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(ReportControl::onSelectedItemEvent, this));
	_listViewReport->addEventListener((ui::ListView::ccScrollViewCallback)CC_CALLBACK_2(ReportControl::onSelectedItemEventScrollView, this));
	return true;
}
string ReportControl::getSelectedItemEventName(){
	return _className + "SelectedItem";
}
string ReportControl::getNextPageEventName(){
	return _className + "NextPage";
}
void ReportControl::onSelectedItemEvent(Ref *pSender, ListView::EventType type){
	if (type == ListView::EventType::ON_SELECTED_ITEM_END){
		_rootNode->getEventDispatcher()->dispatchCustomEvent(getSelectedItemEventName(), _listViewReport->getItem(_listViewReport->getCurSelectedIndex())->getUserData());
	}
}
void ReportControl::onSelectedItemEventScrollView(Ref *pSender, ScrollView::EventType type){
	if (type == ScrollView::EventType::SCROLL_TO_BOTTOM){
		if (_records.size() < _totalCount){
			int pageIndex = _records.size() / REPORT_PAGE_SIZE + 1;
			_rootNode->getEventDispatcher()->dispatchCustomEvent(getNextPageEventName(), &pageIndex);
		}
	}
}
void ReportControl::clear(){
	for (auto record : _records)
		delete record;
	_records.clear();
	if (_listViewReport){
		_listViewReport->removeAllItems();
		_listViewReport->scrollToTop(0.1f, false);
	}
	setTotal(0, nullptr);		//重置统计栏
}
void ReportControl::show(){
	_rootNode->setVisible(true);
}
void ReportControl::hide(){
	_rootNode->setVisible(false);
}
Node *ReportControl::getRootNode(){
	return _rootNode;
}
void ReportControl::setTotal(int totalCount, void *data){
	_totalCount = totalCount;
}
void ReportControl::setData(void* datas){

}
//*************** ListUserBar
ListUserBar* ListUserBar::create(ListView* listView)
{
	ListUserBar* pRet = new(std::nothrow) ListUserBar();
	if (pRet&&pRet->init(listView)){
		return pRet;
	}
	else{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool ListUserBar::init(ListView* listView)
{
	if (!listView)
		return false;
	auto item = listView->getItem(0);
	if (!item || !item->getChildByName("lbUserName"))
		return false;
	listView->setItemModel(item);
	listView->removeAllChildren();
	listView->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(ListUserBar::onSelectedItemEvent, this));
	_listView = listView;
	return true;
}
bool ListUserBar::AddUserToBar(int userid, string nickname){
	_listView->pushBackDefaultItem();
	auto item = _listView->getItems().back();
	auto lbUserName = item->getChildByName<Text*>("lbUserName");
	Size size = item->getSize();
	lbUserName->setString(nickname);
	size.width = lbUserName->getSize().width + 60;		//item宽度为name+60，高度不变
	item->setSize(size);
	item->setTag(userid);			//将tag设置为userid,以便读取
	return true;
}
void ListUserBar::onSelectedItemEvent(Ref *pSender, ListView::EventType type)
{
	if (type == ListView::EventType::ON_SELECTED_ITEM_END){
		int pos = _listView->getCurSelectedIndex();
		int size = _listView->getItems().size();
		//删除点击项之后的导航项
		while ((size-- - 1) > pos)
			_listView->removeLastItem();
		//通知用户已经改变
		if (onUserChanged)
			onUserChanged(_listView->getItems().back()->getTag());
	}
}
//*************** ReportUser
ReportUserControl::ReportUserControl(void) :
_txtTotal(nullptr),
_txtTotalCoin(nullptr),
_txtTotalPlayerNum(nullptr)
{
}
bool ReportUserControl::init()
{
	_className = "ReportUser";
	if (!(_rootNode = CSLoader::createNode("report/LayerUserReport.csb")))
		return false;
	auto panelTitle = _rootNode->getChildByName("panelTitle");
	if (!panelTitle)
		return false;
    
    auto lbNickname=panelTitle->getChildByName<Text*>("lbNickname");
    if(!lbNickname)return false;
    lbNickname->setString(Language::getStringByKey("NickName"));
    
    auto lbCoin=panelTitle->getChildByName<Text*>("lbCoin");
    if(!lbCoin)return false;
    lbCoin->setString(Language::getStringByKey("Balance"));
    
    auto lbChildCount=panelTitle->getChildByName<Text*>("lbChildCount");
    if(!lbChildCount)return false;
    lbChildCount->setString(Language::getStringByKey("Subordinate"));
    
    auto lbLastLogin=panelTitle->getChildByName<Text*>("lbLastLogin");
    if(!lbLastLogin)return false;
    lbLastLogin->setString(Language::getStringByKey("LastLoginTime"));
    
    auto lbStatus=panelTitle->getChildByName<Text*>("lbStatus");
    if(!lbStatus)return false;
    lbStatus->setString(Language::getStringByKey("Status"));
    
	auto panelTotal = _rootNode->getChildByName("panelTotal");
	if (!panelTotal)
		return false;
	if (!(_txtTotal = panelTotal->getChildByName<Text*>("txtTotal")))
		return false;
	if (!(_txtTotalPlayerNum = panelTotal->getChildByName<Text*>("txtTotalPlayerNum")))
		return false;
	if (!(_txtTotalCoin = panelTotal->getChildByName<Text*>("txtTotalCoin")))
		return false;
	if (!(_listViewReport = _rootNode->getChildByName<ListView*>("listViewReport")))
		return false;
	//只检查一次模板项
	auto layout = _listViewReport->getItem(0);
	if (!layout)
		return false;
	Text *txtIdx = layout->getChildByName<Text*>("txtIdx");
	ImageView *ImgAvatar = layout->getChildByName<ImageView*>("ImgAvatar");
	Text *txtName = layout->getChildByName<Text*>("txtName");
	Text *txtCoin = layout->getChildByName<Text*>("txtCoin");
	Text *txtPlayerNum = layout->getChildByName<Text*>("txtPlayerNum");
	Text *txtLastOnline = layout->getChildByName<Text*>("txtLastOnline");
	ImageView *ImgStatus = layout->getChildByName<ImageView*>("ImgStatus");
	if (!txtIdx || !ImgAvatar || !txtName || !txtCoin || !txtPlayerNum || !txtLastOnline || !ImgStatus)
		return false;
	_listViewReport->setItemModel(layout);
	_listViewReport->removeAllChildren();
	return ReportControl::init();		//最后调用基类，完成事件注册
}
void ReportUserControl::setTotal(int totalCount, void *data){
	ReportControl::setTotal(totalCount, data);
	if (totalCount <= 0){
		_txtTotal->setString("");
		_txtTotalPlayerNum->setString("");
		_txtTotalCoin->setString("");
	}
	else{
		UserFormItemRecord *record = (UserFormItemRecord *)data;
        _txtTotal->setString(StringUtils::format(Language::getStringByKey("TotalFormat"), totalCount));
		char strBuf[255];
		//sprintf(strBuf, "%lld", record->balance);
		//_txtTotalCoin->setString(strBuf);
		_txtTotalCoin->setString(Comm::GetShortStringFromInt64(record->balance));
		sprintf(strBuf, "%d", record->children_num + 1);
		_txtTotalPlayerNum->setString(strBuf);
	}
}
void ReportUserControl::setData(void* datas){
	if (!datas)
		return;
	vector<UserFormItemRecord*> *records = (vector<UserFormItemRecord*> *)datas;
	//显示报表数据
	for (auto record : *records) {
		_listViewReport->pushBackDefaultItem();
		auto layout = _listViewReport->getItems().back();
		Text *txtIdx = layout->getChildByName<Text*>("txtIdx");
		ImageView *ImgAvatar = layout->getChildByName<ImageView*>("ImgAvatar");
		Text *txtName = layout->getChildByName<Text*>("txtName");
		Text *txtCoin = layout->getChildByName<Text*>("txtCoin");
		Text *txtPlayerNum = layout->getChildByName<Text*>("txtPlayerNum");
		Text *txtLastOnline = layout->getChildByName<Text*>("txtLastOnline");
		ImageView *ImgStatus = layout->getChildByName<ImageView*>("ImgStatus");
		layout->setUserData(record);
		_records.push_back(record);
		char strBuf[255];
		sprintf(strBuf, "%d", _records.size());
		txtIdx->setString(strBuf);
		if (record->avatar > 0)
			ImgAvatar->loadTexture(AvatarControl::GetInstance()->GetAvatarSpriteName(record->avatar, 0), Widget::TextureResType::PLIST);
		else
			ImgAvatar->setVisible(false);
		txtName->setTextColor(Color4B(0x1E, 0xB1, 0xFF, 0xFF));
		txtName->setString(record->nickname);
		//sprintf(strBuf, "%lld", record->balance);
		//txtCoin->setString(strBuf);
		txtCoin->setTextColor(Color4B(0xFF, 0xA5, 0x00, 0xFF));
		txtCoin->setString(Comm::GetShortStringFromInt64(record->balance));
		sprintf(strBuf, "%d", _records.size() == 1 ? 1 : record->children_num + 1);		//第一行，下级人数始终为1
		txtPlayerNum->setString(strBuf);
		txtLastOnline->setString(Comm::GetLocalTimeStrngFromUTCString(record->last_login_time));
		Director::getInstance()->getTextureCache()->addImage("img/reports/status_02.png");
		ImgStatus->loadTexture("img/reports/status_02.png");
	}
}
//*************** ReportProfit
ReportProfitControl::ReportProfitControl(void) :
_txtTotal(nullptr),
_txtBetAmount(nullptr),
_txtBuyTotal(nullptr),
//_txtGiveTotal(nullptr),
_txtCommisionTotal(nullptr),
_txtProfitTotal(nullptr),
_txtFinalProfit(nullptr)
{
}
bool ReportProfitControl::init()
{
	_className = "ReportProfit";
	if (!(_rootNode = CSLoader::createNode("report/LayerProfitReport.csb")))
		return false;
	auto panelTitle = _rootNode->getChildByName("panelTitle");
	if (!panelTitle)
		return false;
    
    auto lbNickname=panelTitle->getChildByName<Text*>("lbNickname");
    if(!lbNickname)return false;
    lbNickname->setString(Language::getStringByKey("NickName"));
    
    auto lbBetAmount=panelTitle->getChildByName<Text*>("lbBetAmount");
    if(!lbBetAmount)return false;
    lbBetAmount->setString(Language::getStringByKey("BetAmount"));
    
    auto lbBuy=panelTitle->getChildByName<Text*>("lbBuy");
    if(!lbBuy)return false;
    lbBuy->setString(Language::getStringByKey("Buy"));
    
  /*  auto lbGive=panelTitle->getChildByName<Text*>("lbGive");
    if(!lbGive)return false;
    lbGive->setString(Language::getStringByKey("Give"));*/
    
    auto lbCommision=panelTitle->getChildByName<Text*>("lbCommision");
    if(!lbCommision)return false;
    lbCommision->setString(Language::getStringByKey("Commission"));
    
    auto lbProfit=panelTitle->getChildByName<Text*>("lbProfit");
    if(!lbProfit)return false;
    lbProfit->setString(Language::getStringByKey("Profit"));
    
    auto lbFinalProfit=panelTitle->getChildByName<Text*>("lbFinalProfit");
    if(!lbFinalProfit)return false;
    lbFinalProfit->setString(Language::getStringByKey("FinalProfit"));
    
	auto panelTotal = _rootNode->getChildByName("panelTotal");
	if (!panelTotal)
		return false;
	if (!(_txtTotal = panelTotal->getChildByName<Text*>("txtTotal")))
		return false;
	if (!(_txtBetAmount = panelTotal->getChildByName<Text*>("txtBetAmount")))
		return false;
	if (!(_txtBuyTotal = panelTotal->getChildByName<Text*>("txtBuyTotal")))
		return false;
	/*if (!(_txtGiveTotal = panelTotal->getChildByName<Text*>("txtGiveTotal")))
		return false;*/
	if (!(_txtCommisionTotal = panelTotal->getChildByName<Text*>("txtCommisionTotal")))
		return false;
	if (!(_txtProfitTotal = panelTotal->getChildByName<Text*>("txtProfitTotal")))
		return false;
	if (!(_txtFinalProfit = panelTotal->getChildByName<Text*>("txtFinalProfit")))
		return false;
	if (!(_listViewReport = _rootNode->getChildByName<ListView*>("listViewReport")))
		return false;
	//只检查一次模板项
	auto layout = _listViewReport->getItem(0);
	if (!layout)
		return false;
	Text *txtUserName = layout->getChildByName<Text*>("txtUserName");
	Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
	Text *txtBuy = layout->getChildByName<Text*>("txtBuy");
	//Text *txtGive = layout->getChildByName<Text*>("txtGive");
	Text *txtCommission = layout->getChildByName<Text*>("txtCommission");
	Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
	Text *txtFinalProfit = layout->getChildByName<Text*>("txtFinalProfit");
	if (!txtUserName || !txtBetAmount || !txtBuy || !txtCommission || !txtProfit || !txtFinalProfit)
		return false;
	_listViewReport->setItemModel(layout);
	_listViewReport->removeAllChildren();
	return ReportControl::init();		//最后调用基类，完成事件注册
}
void ReportProfitControl::setTotal(int totalCount, void *data){
	ReportControl::setTotal(totalCount, data);
	if (totalCount <= 0){
		_totalCount = 0;
		_txtTotal->setString("");
		_txtBetAmount->setString("");
		_txtBuyTotal->setString("");
		//_txtGiveTotal->setString("");
		_txtCommisionTotal->setString("");
		_txtProfitTotal->setString("");
		_txtFinalProfit->setString("");
	}
	else if(data){
		ProfitRecord *record = (ProfitRecord *)data;
        _txtTotal->setString(StringUtils::format(Language::getStringByKey("TotalFormat"), totalCount));
		_txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		_txtBuyTotal->setString(Comm::GetShortStringFromInt64(record->deposit));
		//_txtGiveTotal->setString(Comm::GetShortStringFromInt64(record->transfer));
		_txtCommisionTotal->setString(Comm::GetShortStringFromInt64(record->commission));
		_txtProfitTotal->setString(Comm::GetShortStringFromInt64(record->profit));
		if (record->profit >= 0) {
			_txtProfitTotal->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			_txtProfitTotal->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
		int64_t totalFinalProfit = record->commission + record->profit;
		_txtFinalProfit->setString(Comm::GetShortStringFromInt64(totalFinalProfit));
		if (totalFinalProfit >= 0) {
			_txtFinalProfit->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			_txtFinalProfit->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
void ReportProfitControl::setData(void* datas){
	if (!datas)
		return;
	vector<ProfitRecord*> *records = (vector<ProfitRecord*> *)datas;
	//显示报表数据
	for (auto record : *records) {
		_listViewReport->pushBackDefaultItem();
		auto layout = _listViewReport->getItems().back();
		Text *txtUserName = layout->getChildByName<Text*>("txtUserName");
		Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
		Text *txtBuy = layout->getChildByName<Text*>("txtBuy");
		//Text *txtGive = layout->getChildByName<Text*>("txtGive");
		Text *txtCommission = layout->getChildByName<Text*>("txtCommission");
		Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
		Text *txtFinalProfit = layout->getChildByName<Text*>("txtFinalProfit");
		layout->setUserData(record);
		_records.push_back(record);

		//必须再次setTextColor，因为3.6版本有BUG，listView->pushBackDefaultItem的item文本颜色会丢失
		txtUserName->setTextColor(Color4B(0x1E, 0xB1, 0xFF, 0xFF));
		txtUserName->setString(record->nickname);
		txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		txtBuy->setString(Comm::GetShortStringFromInt64(record->deposit));
		//txtGive->setString(Comm::GetShortStringFromInt64(record->transfer));
		txtCommission->setString(Comm::GetShortStringFromInt64(record->commission));
		txtProfit->setString(Comm::GetShortStringFromInt64(record->profit));
		txtBuy->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		//txtGive->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		if (record->profit >= 0) {
			txtProfit->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			txtProfit->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
		int64_t finalProfit = record->commission + record->profit;
		txtFinalProfit->setString(Comm::GetShortStringFromInt64(finalProfit));
		if (finalProfit >= 0) {
			txtFinalProfit->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			txtFinalProfit->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
//*************** ReportGame
ReportGameControl::ReportGameControl(void) :
_txtTotal(nullptr),
_txtBetAmount(nullptr),
_txtProfitTotal(nullptr)
{
}
bool ReportGameControl::init()
{
	_className = "ReportGame";
	if (!(_rootNode = CSLoader::createNode("report/LayerGameReport.csb")))
		return false;
	auto panelTitle = _rootNode->getChildByName("panelTitle");
	if (!panelTitle)
		return false;
    
    auto lbNickname=panelTitle->getChildByName<Text*>("lbNickname");
    if(!lbNickname)return false;
    lbNickname->setString(Language::getStringByKey("NickName"));
    
    auto lbBetAmount=panelTitle->getChildByName<Text*>("lbBetAmount");
    if(!lbBetAmount)return false;
    lbBetAmount->setString(Language::getStringByKey("BetAmount"));
    
    auto lbProfit=panelTitle->getChildByName<Text*>("lbProfit");
    if(!lbProfit)return false;
    lbProfit->setString(Language::getStringByKey("Profit"));
    
	auto panelTotal = _rootNode->getChildByName("panelTotal");
	if (!panelTotal)
		return false;
	if (!(_txtTotal = panelTotal->getChildByName<Text*>("txtTotal")))
		return false;
	if (!(_txtBetAmount = panelTotal->getChildByName<Text*>("txtBetAmount")))
		return false;
	if (!(_txtProfitTotal = panelTotal->getChildByName<Text*>("txtProfitTotal")))
		return false;
	if (!(_listViewReport = _rootNode->getChildByName<ListView*>("listViewReport")))
		return false;
	//只检查一次模板项
	auto layout = _listViewReport->getItem(0);
	if (!layout)
		return false;
	Text *txtIdx = layout->getChildByName<Text*>("txtIdx");
	ImageView *ImgAvatar = layout->getChildByName<ImageView*>("ImgAvatar");
	Text *txtName = layout->getChildByName<Text*>("txtName");
	Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
	Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
	if (!txtIdx || !ImgAvatar || !txtName || !txtBetAmount || !txtProfit)
		return false;
	_listViewReport->setItemModel(layout);
	_listViewReport->removeAllChildren();
	return ReportControl::init();		//最后调用基类，完成事件注册
}
void ReportGameControl::setTotal(int totalCount, void *data){
	ReportControl::setTotal(totalCount, data);
	if (totalCount <= 0){
		_totalCount = 0;
		_txtTotal->setString("");
		_txtBetAmount->setString("");
		_txtProfitTotal->setString("");
	}
	else if (data){
		ProfitRecord *record = (ProfitRecord *)data;
        _txtTotal->setString(StringUtils::format(Language::getStringByKey("TotalFormat"), totalCount));
		_txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		_txtProfitTotal->setString(Comm::GetShortStringFromInt64(record->profit));
		if (record->profit >= 0) {
			_txtProfitTotal->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			_txtProfitTotal->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
void ReportGameControl::setData(void* datas){
	if (!datas)
		return;
	vector<ProfitRecord*> *records = (vector<ProfitRecord*> *)datas;
	char strBuf[255];
	//显示报表数据
	for (auto record : *records) {
		_listViewReport->pushBackDefaultItem();
		auto layout = _listViewReport->getItems().back();
		Text *txtIdx = layout->getChildByName<Text*>("txtIdx");
		ImageView *ImgAvatar = layout->getChildByName<ImageView*>("ImgAvatar");
		Text *txtName = layout->getChildByName<Text*>("txtName");
		Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
		Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
		layout->setUserData(record);
		_records.push_back(record);

		//必须再次setTextColor，因为3.6版本有BUG，listView->pushBackDefaultItem的item文本颜色会丢失
		sprintf(strBuf, "%d", _records.size());
		txtIdx->setString(strBuf);
		if (record->avatar > 0)
			ImgAvatar->loadTexture(AvatarControl::GetInstance()->GetAvatarSpriteName(record->avatar, 0), Widget::TextureResType::PLIST);
		else
			ImgAvatar->setVisible(false);
		txtName->setTextColor(Color4B(0x1E, 0xB1, 0xFF, 0xFF));
		txtName->setString(record->nickname);
		txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		txtProfit->setString(Comm::GetShortStringFromInt64(record->profit));
		if (record->profit >= 0) {
			txtProfit->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			txtProfit->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
//*************** ReportGameDetail
ReportGameDetailControl::ReportGameDetailControl(void) :
_txtTotal(nullptr),
_txtBetAmount(nullptr),
_txtProfitTotal(nullptr)
{
}
bool ReportGameDetailControl::init()
{
	_className = "ReportGameDetail";
	if (!(_rootNode = CSLoader::createNode("report/LayerGameDetailReport.csb")))
		return false;
	auto panelTitle = _rootNode->getChildByName("panelTitle");
	if (!panelTitle)
		return false;
    
    auto lbRecord=panelTitle->getChildByName<Text*>("lbRecord");
    if(!lbRecord)return false;
    lbRecord->setString(Language::getStringByKey("GameRecord"));
    
    auto lbBalanceTime=panelTitle->getChildByName<Text*>("lbBalanceTime");
    if(!lbBalanceTime)return false;
    lbBalanceTime->setString(Language::getStringByKey("SettleTime"));
    
    auto lbGameType=panelTitle->getChildByName<Text*>("lbGameType");
    if(!lbGameType)return false;
    lbGameType->setString(Language::getStringByKey("GameType"));
    
    auto lbBetAmount=panelTitle->getChildByName<Text*>("lbBetAmount");
    if(!lbBetAmount)return false;
    lbBetAmount->setString(Language::getStringByKey("BetAmount"));
    
    auto lbProfit=panelTitle->getChildByName<Text*>("lbProfit");
    if(!lbProfit)return false;
    lbProfit->setString(Language::getStringByKey("Profit"));
    
	auto panelTotal = _rootNode->getChildByName("panelTotal");
	if (!panelTotal)
		return false;
	if (!(_txtTotal = panelTotal->getChildByName<Text*>("txtTotal")))
		return false;
	if (!(_txtBetAmount = panelTotal->getChildByName<Text*>("txtBetAmount")))
		return false;
	if (!(_txtProfitTotal = panelTotal->getChildByName<Text*>("txtProfitTotal")))
		return false;
	if (!(_listViewReport = _rootNode->getChildByName<ListView*>("listViewReport")))
		return false;
	//只检查一次模板项
	auto layout = _listViewReport->getItem(0);
	if (!layout)
		return false;
	Text *txtRecord = layout->getChildByName<Text*>("txtRecord");
	Text *txtBalanceTime = layout->getChildByName<Text*>("txtBalanceTime");
	Text *txtGameType = layout->getChildByName<Text*>("txtGameType");
	Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
	Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
	if (!txtRecord || !txtBalanceTime || !txtGameType || !txtBetAmount || !txtProfit)
		return false;
	_listViewReport->setItemModel(layout);
	_listViewReport->removeAllChildren();
	return ReportControl::init();		//最后调用基类，完成事件注册
}
void ReportGameDetailControl::setTotal(int totalCount, void *data){
	ReportControl::setTotal(totalCount, data);
	if (totalCount <= 0){
		_totalCount = 0;
		_txtTotal->setString("");
		_txtBetAmount->setString("");
		_txtProfitTotal->setString("");
	}
	else if (data){
		GameDetailRecord *record = (GameDetailRecord *)data;
        _txtTotal->setString(StringUtils::format(Language::getStringByKey("TotalFormat"), totalCount));
		_txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		_txtProfitTotal->setString(Comm::GetShortStringFromInt64(record->profit));
		if (record->profit >= 0) {
			_txtProfitTotal->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			_txtProfitTotal->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
void ReportGameDetailControl::setData(void* datas){
	if (!datas)
		return;
	vector<GameDetailRecord*> *records = (vector<GameDetailRecord*> *)datas;
	char strBuf[255];
	//显示报表数据
	for (auto record : *records) {
		_listViewReport->pushBackDefaultItem();
		auto layout = _listViewReport->getItems().back();
		Text *txtRecord = layout->getChildByName<Text*>("txtRecord");
		Text *txtBalanceTime = layout->getChildByName<Text*>("txtBalanceTime");
		Text *txtGameType = layout->getChildByName<Text*>("txtGameType");
		Text *txtBetAmount = layout->getChildByName<Text*>("txtBetAmount");
		Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
		layout->setUserData(record);
		_records.push_back(record);

		//必须再次setTextColor，因为3.6版本有BUG，listView->pushBackDefaultItem的item文本颜色会丢失
		sprintf(strBuf, "%d", record->round_id);
		txtRecord->setString(strBuf);
		txtBalanceTime->setString(Comm::GetLocalTimeStrngFromUTCString(record->create_time));
		switch (record->game_type) {
		case BJL:
			txtGameType->setString("Baccarat");
			break;
		case DZPK:
		{
			txtGameType->setString("Pocker");
		}
			break;
		case DZPK1:
		{
			txtGameType->setString("Royal Pocker");
		}
			break;
		case SGJ:
			txtGameType->setString("Fruit");
			break;
		default:
			txtGameType->setString("Unknown");
			break;
		}
		txtBetAmount->setString(Comm::GetShortStringFromInt64(record->bet_amount));
		txtProfit->setString(Comm::GetShortStringFromInt64(record->profit));
		if (record->profit >= 0) {
			txtProfit->setTextColor(Color4B(0x40, 0xcc, 0x40, 0xff));
		}
		else {
			txtProfit->setTextColor(Color4B(0xff, 0x00, 0x80, 0xff));
		}
	}
}
//************** ReportsControl
ReportsControl::~ReportsControl(){
	if (_listUser)
		delete _listUser;
	if (_reportUser)
		delete _reportUser;
	if (_reportGame)
		delete _reportGame;
	if (_reportGameDetail)
		delete _reportGameDetail;
	if (_reportProfit)
		delete _reportProfit;
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
void ReportsControl::onGetUserReportCallBack(Ref *pSender){
	if (pSender == nullptr || _curType != REPORTTYPE::USERREPORT)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto layerLoading = _rootNode->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();
		if (msg->code == 0)
		{
			vector<UserFormItemRecord*> records;
			UserFormItemRecord totalRecord={0};
			int totalCount;
			if (InitUserReportData(msg->data, &totalCount, &totalRecord, &records)){
				_reportUser->setTotal(totalCount, &totalRecord);
				_reportUser->setData(&records);
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetUserReportCallBack", "init user report data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetUserReportCallBack", "Get user report data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
bool ReportsControl::InitUserReportData(string jsonData, int *totalCount, UserFormItemRecord* totalRecord, vector<UserFormItemRecord*> *records) {

	if (jsonData.empty() || !totalCount || !totalRecord || !records)
		return false;
	*totalCount = 0;
	records->clear();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array || cJSON_GetArraySize(jsonMsg) != 3)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	//先检查记录数
	cJSON *jsonMsgInner1;
	if (!(jsonMsgInner1 = cJSON_GetArrayItem(jsonMsg, 1)) || jsonMsgInner1->type != cJSON_Number) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		*totalCount = jsonMsgInner1->valueint;
	}
	if (*totalCount <= 0){
		cJSON_Delete(jsonMsg);
		return true;
	}
	//解析总和记录
	cJSON *jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2);
	if (!(jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2)) || jsonMsgInner2->type != cJSON_Object) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		cJSON *balance, *children_num;
		if (!(balance = cJSON_GetObjectItem(jsonMsgInner2, "balance")) || balance->type != cJSON_Number
			|| !(children_num = cJSON_GetObjectItem(jsonMsgInner2, "children_num")) || children_num->type != cJSON_Number){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else {
			totalRecord->balance = balance->valuedouble;
			totalRecord->children_num = children_num->valueint;
		}
	}
	//解析用户报表记录,最后解析
	cJSON *jsonMsgInner = cJSON_GetArrayItem(jsonMsg, 0);
	if (jsonMsgInner->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsgInner);
	cJSON *item;
	cJSON *username, *nickname, *avatar, *balance, *user_id, *children_num, *last_login_time;

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsgInner, i)) || item->type != cJSON_Object
			|| !(username = cJSON_GetObjectItem(item, "username")) || username->type != cJSON_String
			|| !(nickname = cJSON_GetObjectItem(item, "nickname"))
			|| !(avatar = cJSON_GetObjectItem(item, "avatar"))
			|| !(balance = cJSON_GetObjectItem(item, "balance")) || balance->type != cJSON_Number
			|| !(user_id = cJSON_GetObjectItem(item, "user_id")) || user_id->type != cJSON_Number
			|| !(children_num = cJSON_GetObjectItem(item, "children_num")) || children_num->type != cJSON_Number
			|| !(last_login_time = cJSON_GetObjectItem(item, "last_login_time")))
		{
			for (auto record : *records)
				delete record;
			records->clear();
			cJSON_Delete(jsonMsg);
			return false;
		}
		else {
			UserFormItemRecord *record = new UserFormItemRecord();
			record->user_id = user_id->valueint;
			record->username = username->valuestring;
			record->nickname = nickname->type == cJSON_String ? nickname->valuestring : "";
			record->avatar = avatar->type == cJSON_Number ? avatar->valueint : -1;
			record->balance = balance->valuedouble;
			record->children_num = children_num->valueint;
			record->last_login_time = last_login_time->type == cJSON_String ? last_login_time->valuestring : "";
			records->push_back(record);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}
void ReportsControl::onGetGameDetailReportCallBack(Ref *pSender){
	if (pSender == nullptr || _curType != REPORTTYPE::GAMEDETAILREPORT)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto layerLoading = _rootNode->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();
		if (msg->code == 0)
		{
			vector<GameDetailRecord*> records;
			GameDetailRecord totalRecord={0};
			int totalCount;
			if (InitGameDetailReportData(msg->data, &totalCount, &totalRecord, &records)){
				_reportGameDetail->setTotal(totalCount, &totalRecord);
				_reportGameDetail->setData(&records);
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetGameDetailReportCallBack", "init game detail report data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetGameDetailReportCallBack", "Get game detail report data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
bool ReportsControl::InitGameDetailReportData(string jsonData, int *totalCount, GameDetailRecord* totalRecord, vector<GameDetailRecord*> *records) {

	if (jsonData.empty() || !totalCount || !totalRecord || !records)
		return false;
	*totalCount = 0;
	records->clear();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array || cJSON_GetArraySize(jsonMsg) != 3)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	//先检查记录数
	cJSON *jsonMsgInner1;
	if (!(jsonMsgInner1 = cJSON_GetArrayItem(jsonMsg, 1)) || jsonMsgInner1->type != cJSON_Number) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		*totalCount = jsonMsgInner1->valueint;
	}
	if (*totalCount <= 0){
		cJSON_Delete(jsonMsg);
		return true;
	}
	//解析总和记录
	cJSON *jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2);
	if (!(jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2)) || jsonMsgInner2->type != cJSON_Object) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		cJSON *profit, *bet_amount;
		if (!(profit = cJSON_GetObjectItem(jsonMsgInner2, "profit")) || profit->type != cJSON_Number
			|| !(bet_amount = cJSON_GetObjectItem(jsonMsgInner2, "bet_amount")) || bet_amount->type != cJSON_Number){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else {
			totalRecord->profit = profit->valuedouble;
			totalRecord->bet_amount = bet_amount->valuedouble;
		}
	}
	//解析游戏详细报表记录,最后解析
	cJSON *jsonMsgInner = cJSON_GetArrayItem(jsonMsg, 0);
	if (jsonMsgInner->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsgInner);
	cJSON *item;
	cJSON *username, *user_id, *uuid, *bet_amount, *currency_type, *round_id, *profit, *game_type, *_id, *commission, *create_time, *play_type, *valid_amount;
	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsgInner, i)) || item->type != cJSON_Object
			|| !(username = cJSON_GetObjectItem(item, "username")) || username->type != cJSON_String
			|| !(user_id = cJSON_GetObjectItem(item, "user_id")) || user_id->type != cJSON_Number
			|| !(uuid = cJSON_GetObjectItem(item, "uuid")) || uuid->type != cJSON_String
			|| !(bet_amount = cJSON_GetObjectItem(item, "bet_amount")) || bet_amount->type != cJSON_Number
			|| !(currency_type = cJSON_GetObjectItem(item, "currency_type")) || currency_type->type != cJSON_Number
			|| !(round_id = cJSON_GetObjectItem(item, "round_id")) || round_id->type != cJSON_Number
			|| !(profit = cJSON_GetObjectItem(item, "profit")) || profit->type != cJSON_Number
			|| !(game_type = cJSON_GetObjectItem(item, "game_type")) || game_type->type != cJSON_Number
			|| !(_id = cJSON_GetObjectItem(item, "id")) || _id->type != cJSON_Number
			|| !(commission = cJSON_GetObjectItem(item, "commission")) || commission->type != cJSON_Number
			|| !(create_time = cJSON_GetObjectItem(item, "create_time")) || create_time->type != cJSON_String
			//|| !(commission_time = cJSON_GetObjectItem(item, "commission_time")) || commission_time->type != cJSON_Number
			|| !(play_type = cJSON_GetObjectItem(item, "play_type")) || play_type->type != cJSON_Number
			|| !(valid_amount = cJSON_GetObjectItem(item, "valid_amount")) || valid_amount->type != cJSON_Number){
			for (auto record : *records)
				delete record;
			records->clear();
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			GameDetailRecord *record = new GameDetailRecord();
			record->bet_amount = bet_amount->valueint;
			record->currency_type = currency_type->valueint;
			record->round_id = round_id->valueint;
			record->profit = profit->valueint;
			record->game_type = game_type->valueint;
			record->_id = _id->valueint;
			record->commission = commission->valueint;
			record->create_time = create_time->valuestring;
			//record->commission_time = commission_time->valueint;
			record->play_type = play_type->valueint;
			record->valid_amount = valid_amount->valueint;
			records->push_back(record);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}
void ReportsControl::onGetProfitReportCallBack(Ref *pSender){
	if (pSender == nullptr || ((_curType != REPORTTYPE::GAMEREPORT) && (_curType != REPORTTYPE::PROFITREPORT)))
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto layerLoading = _rootNode->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();
		if (msg->code == 0)
		{
			vector<ProfitRecord*> records;
			ProfitRecord totalRecord={0};
			int totalCount;
			if (InitProfitReportData(msg->data, &totalCount, &totalRecord, &records)){
				if (_curType == REPORTTYPE::GAMEREPORT){
					_reportGame->setTotal(totalCount, &totalRecord);
					_reportGame->setData(&records);
				}
				else if (_curType == REPORTTYPE::PROFITREPORT){
					_reportProfit->setTotal(totalCount, &totalRecord);
					_reportProfit->setData(&records);
				}
				
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetProfitReportCallBack", "init profit report data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetProfitReportCallBack", "Get profit report data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
bool ReportsControl::InitProfitReportData(string jsonData, int *totalCount, ProfitRecord* totalRecord, vector<ProfitRecord*> *records) {

	if (jsonData.empty() || !totalCount || !totalRecord || !records)
		return false;
	*totalCount = 0;
	records->clear();
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array || cJSON_GetArraySize(jsonMsg) != 3)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	//先检查记录数
	cJSON *jsonMsgInner1;
	if (!(jsonMsgInner1 = cJSON_GetArrayItem(jsonMsg, 1)) || jsonMsgInner1->type != cJSON_Number) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		*totalCount = jsonMsgInner1->valueint;
	}
	if (*totalCount <= 0){
		cJSON_Delete(jsonMsg);
		return true;
	}
	//解析总和记录
	cJSON *jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2);
	if (!(jsonMsgInner2 = cJSON_GetArrayItem(jsonMsg, 2)) || jsonMsgInner2->type != cJSON_Object) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		cJSON *profit, *commission, *deposit, *amount, *transfer;
		if (!(profit = cJSON_GetObjectItem(jsonMsgInner2, "profit")) || profit->type != cJSON_Number
			|| !(commission = cJSON_GetObjectItem(jsonMsgInner2, "commission")) || commission->type != cJSON_Number
			|| !(deposit = cJSON_GetObjectItem(jsonMsgInner2, "deposit")) || deposit->type != cJSON_Number
			|| !(amount = cJSON_GetObjectItem(jsonMsgInner2, "bet_amount")) || amount->type != cJSON_Number
			|| !(transfer = cJSON_GetObjectItem(jsonMsgInner2, "transfer")) || transfer->type != cJSON_Number){
			cJSON_Delete(jsonMsg);
			return false;
		}
		else {
			totalRecord->profit = profit->valuedouble;
			totalRecord->commission = commission->valuedouble;
			totalRecord->deposit = deposit->valuedouble;
			totalRecord->bet_amount = amount->valuedouble;
			totalRecord->transfer = transfer->valuedouble;
		}
	}
	//解析盈亏记录,最后解析
	cJSON *jsonMsgInner = cJSON_GetArrayItem(jsonMsg, 0);
	if (jsonMsgInner->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsgInner);
	cJSON *item;
	cJSON *nickname, *user_id, *avatar, *bet_amount, *profit, *transfer, *commission, *deposit;
	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsgInner, i)) || item->type != cJSON_Object
			|| !(avatar = cJSON_GetObjectItem(item, "avatar"))
			|| !(user_id = cJSON_GetObjectItem(item, "user_id")) || user_id->type != cJSON_Number
			|| !(nickname = cJSON_GetObjectItem(item, "nickname"))
			|| !(bet_amount = cJSON_GetObjectItem(item, "bet_amount")) || bet_amount->type != cJSON_Number
			|| !(profit = cJSON_GetObjectItem(item, "profit")) || profit->type != cJSON_Number
			|| !(transfer = cJSON_GetObjectItem(item, "transfer")) || transfer->type != cJSON_Number
			|| !(commission = cJSON_GetObjectItem(item, "commission")) || commission->type != cJSON_Number
			|| !(deposit = cJSON_GetObjectItem(item, "deposit")) || deposit->type != cJSON_Number) {
			for (auto record : *records)
				delete record;
			records->clear();
			cJSON_Delete(jsonMsg);
			return false;
		}
		else{

			ProfitRecord *record = new ProfitRecord();
			record->nickname = nickname->type == cJSON_String ? nickname->valuestring : "";
			record->user_id = user_id->valueint;
			record->avatar = avatar->type == cJSON_Number ? avatar->valueint : -1;
			record->bet_amount = bet_amount->valuedouble;
			record->profit = profit->valuedouble;
			record->transfer = transfer->valuedouble;
			record->commission = commission->valuedouble;
			record->deposit = deposit->valuedouble;
			records->push_back(record);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}
ReportsControl* ReportsControl::create(Node* node)
{
	ReportsControl* pRet = new(std::nothrow) ReportsControl();
	if (pRet&&pRet->init(node)){
		return pRet;
	}
	else{
		delete pRet;
		pRet = nullptr;
		return nullptr;
	}
}
bool ReportsControl::init(Node* node)
{
	if (!node)
		return false;
	_rootNode = node;
	auto panelControl = _rootNode->getChildByName("panelControl");
	if (!panelControl)
		return false;
	//用户导航类
	if (!(_listUser = ListUserBar::create(panelControl->getChildByName<ListView*>("listViewUser"))))
		return false;
	_listUser->onUserChanged = CC_CALLBACK_1(ReportsControl::onUserChanged, this);
	//报表控件类
	if (!(_panelReport = _rootNode->getChildByName("panelReport")))
		return false;
	//用户报表初始化
	if (!(_reportUser = ReportUserControl::create()))
		return false;
	_reportUser->hide();
	_panelReport->addChild(_reportUser->getRootNode());
	//监听用户报表数据接受
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(ReportsControl::onGetUserReportCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::PAGE_AGENT_BALANCE), nullptr);
	//监听用户列表翻页事件
	EventListenerCustom *listenerUserReportNextPage = EventListenerCustom::create(_reportUser->getNextPageEventName(), [=](EventCustom *eventCustom){
		CCLOG("User report event: Next page");
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		auto layerLoading = LayerLoading::create();
		layerLoading->setName("LayerLoading");
		_rootNode->addChild(layerLoading);
		int *pageIndex = (int*)eventCustom->getUserData();
		Api::Bill::sg_baccarat_bill_page_agent_balance(*pageIndex, REPORT_PAGE_SIZE, _userid);
	});
	_reportUser->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerUserReportNextPage, _reportUser->getRootNode());
	//监听用户列表点击事件
	EventListenerCustom *listenerUserReportSelectedItem = EventListenerCustom::create(_reportUser->getSelectedItemEventName(), [=](EventCustom *eventCustom){
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		UserFormItemRecord *record = (UserFormItemRecord *)eventCustom->getUserData();
		if (_userid == record->user_id)
			return;		//点击本人时，不做处理
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
			if (_listUser->AddUserToBar(record->user_id, record->nickname)){
				_userid = record->user_id;
				showReport(Report::USERREPORT, _strFrom, _strTo);
			}
		});
	});
	_reportUser->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerUserReportSelectedItem, _reportUser->getRootNode());
	//游戏报表初始化
	if (!(_reportGame = ReportGameControl::create()))
		return false;
	_reportGame->hide();
	_panelReport->addChild(_reportGame->getRootNode());
	//监听游戏列表翻页事件
	EventListenerCustom *listenerGameReportNextPage = EventListenerCustom::create(_reportGame->getNextPageEventName(), [=](EventCustom *eventCustom){
		CCLOG("Game report event: Next page");
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		auto layerLoading = LayerLoading::create();
		layerLoading->setName("LayerLoading");
		_rootNode->addChild(layerLoading);
		int *pageIndex = (int*)eventCustom->getUserData();
		Api::Bill::sg_baccarat_bill_page_date_summary_tree(_strFrom, _strTo, *pageIndex, REPORT_PAGE_SIZE, _userid);
	});
	_reportGame->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerGameReportNextPage, _reportGame->getRootNode());
	//监听游戏列表点击事件
	EventListenerCustom *listenerGameReportSelectedItem = EventListenerCustom::create(_reportGame->getSelectedItemEventName(), [=](EventCustom *eventCustom){
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		ProfitRecord *record = (ProfitRecord *)eventCustom->getUserData();
		if (_userid == record->user_id){
			//点击本人时，显示游戏明细报表
			showReport(REPORTTYPE::GAMEDETAILREPORT, _strFrom, _strTo);
		}
		else{
			//点击其他行，显示下级游戏统计报表
			Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
				if (_listUser->AddUserToBar(record->user_id, record->nickname)){
					_userid = record->user_id;
					showReport(Report::GAMEREPORT, _strFrom, _strTo);
				}
			});
		}
	});
	_reportGame->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerGameReportSelectedItem, _reportGame->getRootNode());
	//游戏明细报表初始化
	if (!(_reportGameDetail = ReportGameDetailControl::create()))
		return false;
	_reportGameDetail->hide();
	_rootNode->addChild(_reportGameDetail->getRootNode());
	//监听游戏明细报表数据接受
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(ReportsControl::onGetGameDetailReportCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::PAGE_GAME_INDEXES), nullptr);
	//监听单局游戏数据接受
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(ReportsControl::onGetGameRoundDataCallBack), GetMsgTypeString(MSGTYPE_GAME, MSGCMD_GAME::GET_USER_GAME_ROUND), nullptr);
	//监听游戏明细列表翻页事件
	EventListenerCustom *listenerGameDetailReportNextPage = EventListenerCustom::create(_reportGameDetail->getNextPageEventName(), [=](EventCustom *eventCustom){
		CCLOG("GameDetail report event: Next page");
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		auto layerLoading = LayerLoading::create();
		layerLoading->setName("LayerLoading");
		_rootNode->addChild(layerLoading);
		int *pageIndex = (int*)eventCustom->getUserData();
		Api::Game::sg_baccarat_get_game_page_game_indexes(_strFrom, _strTo, *pageIndex, REPORT_PAGE_SIZE, _userid);
	});
	_reportGameDetail->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerGameDetailReportNextPage, _reportGameDetail->getRootNode());
	//监听游戏明细列表点击事件
	EventListenerCustom *listenerGameDetailReportSelectedItem = EventListenerCustom::create(_reportGameDetail->getSelectedItemEventName(), [=](EventCustom *eventCustom){
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		GameDetailRecord *record = (GameDetailRecord *)eventCustom->getUserData();
		//显示单项游戏记录详细信息
		showGameRecord(record);
	});
	_reportGameDetail->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerGameDetailReportSelectedItem, _reportGameDetail->getRootNode());
	//盈亏报表初始化
	if (!(_reportProfit = ReportProfitControl::create()))
		return false;
	_reportProfit->hide();
	_panelReport->addChild(_reportProfit->getRootNode());
	//监听盈亏报表数据接受
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(ReportsControl::onGetProfitReportCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::PAGE_DATE_SUMMARY_TREE), nullptr);
	//监听盈亏列表翻页事件
	EventListenerCustom *listenerProfitReportNextPage = EventListenerCustom::create(_reportProfit->getNextPageEventName(), [=](EventCustom *eventCustom){
		CCLOG("Profit report event: Next page");
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		auto layerLoading = LayerLoading::create();
		layerLoading->setName("LayerLoading");
		_rootNode->addChild(layerLoading);
		int *pageIndex = (int*)eventCustom->getUserData();
		Api::Bill::sg_baccarat_bill_page_date_summary_tree(_strFrom, _strTo, *pageIndex, REPORT_PAGE_SIZE, _userid);
	});
	_reportProfit->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerProfitReportNextPage, _reportProfit->getRootNode());
	//监听盈亏列表点击事件
	EventListenerCustom *listenerProfitReportSelectedItem = EventListenerCustom::create(_reportProfit->getSelectedItemEventName(), [=](EventCustom *eventCustom){
		if (_rootNode->getChildByName("LayerLoading"))
			return;		//loading时不响应其他事件
		ProfitRecord *record = (ProfitRecord *)eventCustom->getUserData();
		if (_userid == record->user_id){
			//点击当前用户自身，显示盈亏明细列表
			LayerBillFlowList *layer = LayerBillFlowList::create();
			if (layer){
				_rootNode->addChild(layer);
				layer->Show(_strFrom, _strTo, _userid);
			}
		}
		else{
			Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
				if (_listUser->AddUserToBar(record->user_id, record->nickname)){
					_userid = record->user_id;
					showReport(Report::PROFITREPORT, _strFrom, _strTo);
				}
			});
		}
	});
	_reportProfit->getRootNode()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listenerProfitReportSelectedItem, _reportProfit->getRootNode());
	//默认显示用户为自己
	UserData *userData = UserControl::GetInstance()->GetUserData();
	_userid = userData->user_id;
	_listUser->AddUserToBar(userData->user_id, userData->nickname);
	return true;
}
void ReportsControl::onUserChanged(int userid){
	if (_userid == userid){
		//如果当前显示的是游戏详细列表，则改为显示游戏列表
		if (_curType == REPORTTYPE::GAMEDETAILREPORT){
			_curType = REPORTTYPE::GAMEREPORT;
			showReport(_curType, _strFrom, _strTo);
		}
	}
	else{
		_userid = userid;
		if (_curType == REPORTTYPE::GAMEDETAILREPORT)
			_curType = REPORTTYPE::GAMEREPORT;		//如果当前显示的是游戏详细列表，则改为显示游戏列表
		showReport(_curType, _strFrom, _strTo);
	}
}
void ReportsControl::showReport(REPORTTYPE type, string strFrom, string strTo){
	_curType = type;
	_strFrom = strFrom;
	_strTo = strTo;
	auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	_rootNode->addChild(layerLoading);
	switch (type)
	{
	case Report::USERREPORT:
		_reportUser->clear();
		_reportUser->show();
		_reportGame->hide();
		_reportGameDetail->hide();
		_reportProfit->hide();
		//请求用户表单数据
		Api::Bill::sg_baccarat_bill_page_agent_balance(1, REPORT_PAGE_SIZE, _userid);
		break;
	case Report::GAMEREPORT:
		_reportUser->hide();
		_reportGame->clear();
		_reportGame->show();
		_reportGameDetail->hide();
		_reportProfit->hide();
		//请求盈亏表单数据,游戏列表与盈亏列表使用相同的数据
		Api::Bill::sg_baccarat_bill_page_date_summary_tree(_strFrom, _strTo, 1, REPORT_PAGE_SIZE, _userid);
		break;
	case Report::GAMEDETAILREPORT:
		_reportUser->hide();
		_reportGame->hide();
		_reportGameDetail->clear();
		_reportGameDetail->show();
		_reportProfit->hide();
		//请求游戏明细表单数据
		Api::Game::sg_baccarat_get_game_page_game_indexes(_strFrom, _strTo, 1, REPORT_PAGE_SIZE, _userid);
		break;
	case Report::PROFITREPORT:
		_reportUser->hide();
		_reportGame->hide();
		_reportGameDetail->hide();
		_reportProfit->clear();
		_reportProfit->show();
		//请求盈亏表单数据
		Api::Bill::sg_baccarat_bill_page_date_summary_tree(_strFrom, _strTo, 1, REPORT_PAGE_SIZE, _userid);
		break;
	default:
		return;
	}
}
void ReportsControl::onDateChanged(string strFrom, string strTo){
	showReport(_curType, strFrom, strTo);
}
void ReportsControl::showGameRecord(GameDetailRecord *record){
	if (!record)
		return;
	_curGameDetailRecord = record;
	auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	_rootNode->addChild(layerLoading);
	switch (_curGameDetailRecord->game_type)
	{
	case GAMETYPE::BJL:
		Api::Game::sg_baccarat_game_get_user_game_round(_curGameDetailRecord->round_id, _userid);
		break;
	default:
		Api::Game::sg_baccarat_game_get_user_game_round(_curGameDetailRecord->round_id, _userid);
		//layerLoading->SetString("Unknown game type");
		break;
	}
}
void ReportsControl::onGetGameRoundDataCallBack(Ref *pSender){
	if (!_curGameDetailRecord || !pSender)
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto layerLoading = _rootNode->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();
		if (msg->code == 0)
		{
			GameRoundDataBjl data;
			if (GameControlBjl::GetInstance()->InitGameRoundData(msg->data, &data)){

				switch (_curGameDetailRecord->game_type)
				{
				case GAMETYPE::BJL:
					{
						//显示单项游戏数据
						if (_curGameDetailRecord->round_id == data.round_id){
							LayerGameRecordBjl *layerGameRecordBjl = LayerGameRecordBjl::create();
							if (layerGameRecordBjl) {
								layerGameRecordBjl->ShowGameRound(_curGameDetailRecord, &data);
								_rootNode->addChild(layerGameRecordBjl);
							}
						}
					}
					break;
				case GAMETYPE::DZPK:
				case GAMETYPE::DZPK1:
					{
						if (_curGameDetailRecord->round_id == data.round_id){
							LayerGameRecordDz *layerGameRecordDz = LayerGameRecordDz::create();
							if (layerGameRecordDz) {
								layerGameRecordDz->ShowGameRound(_curGameDetailRecord, &data);
								_rootNode->addChild(layerGameRecordDz);
							}
						}
					}
					break;
				default:
					{

					}
					break;
				}
				_curGameDetailRecord = nullptr;		//清除当前记录

			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetGameRoundDataCallBack", "init game round data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "ReportsControl::onGetGameRoundDataCallBack", "Get game round data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}

//************** LayerBillFlowList`
LayerBillFlowList::LayerBillFlowList(void) :
_userid(0),
_totalCount(0),
_strFrom(""),
_strTo(""),
_curRecord(nullptr)
{
}
LayerBillFlowList::~LayerBillFlowList(void)
{
	clear();
	PKNotificationCenter::getInstance()->removeAllObservers(this);
}
bool LayerBillFlowList::init(){
	if (!Layer::init()) {
		return false;
	}

	/*	载入cocos studio 资源
	*/
	_rootNode = CSLoader::createNode("report/LayerBillFlowList.csb");

	if (!_rootNode)
		return false;
	if (!(_txtTotal = _rootNode->getChildByName<Text*>("txtTotal")))
		return false;
	auto spBg = _rootNode->getChildByName("spBg");
	if (!spBg)
		return false;
    
   /* auto lbTitle=_rootNode->getChildByName<Text*>("lbTitle");
    if(!lbTitle)return false;
    lbTitle->setString(Language::getStringByKey("ProfitList"));*/
    
    auto txtTitleId=_rootNode->getChildByName<Text*>("txtTitleId");
    if(!txtTitleId)return false;
    txtTitleId->setString(Language::getStringByKey("ProfitID"));
    
    auto txtTitleTime=_rootNode->getChildByName<Text*>("txtTitleTime");
    if(!txtTitleTime)return false;
    txtTitleTime->setString(Language::getStringByKey("SettleTime"));
    
    auto txtTitleProfitType=_rootNode->getChildByName<Text*>("txtTitleProfitType");
    if(!txtTitleProfitType)return false;
    txtTitleProfitType->setString(Language::getStringByKey("ProfitType"));
    
    auto txtTitleProfit=_rootNode->getChildByName<Text*>("txtTitleProfit");
    if(!txtTitleProfit)return false;
    txtTitleProfit->setString(Language::getStringByKey("GameProfit"));
    
	//屏蔽事件
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = [=](Touch *touch, Event *event){
		if (!spBg->getBoundingBox().containsPoint(touch->getLocation()))
			this->scheduleOnce([=](float){this->removeFromParent(); }, 0.1f, "DelayExit");
		return true;
	};
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	if (!(_listViewReport = _rootNode->getChildByName<ListView*>("listViewReport")))
		return false;
	auto layout = _listViewReport->getItem(0);
	if (!layout)
		return false;
	Text *txtProfitID = layout->getChildByName<Text*>("txtProfitID");
	Text *txtSettlementTime = layout->getChildByName<Text*>("txtSettlementTime");
	Text *txtProfitType = layout->getChildByName<Text*>("txtProfitType");
	Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
	if (!txtProfitID || !txtSettlementTime || !txtProfitType || !txtProfit)
		return false;
	_listViewReport->setItemModel(layout);
	_listViewReport->removeAllChildren();
	_listViewReport->addEventListener((ui::ListView::ccListViewCallback)CC_CALLBACK_2(LayerBillFlowList::onSelectedItemEvent, this));
	_listViewReport->addEventListener((ui::ListView::ccScrollViewCallback)CC_CALLBACK_2(LayerBillFlowList::onSelectedItemEventScrollView, this));

	this->addChild(_rootNode);

	//监听盈亏详细信息表单接受
	PKNotificationCenter::getInstance()->addObserver(this, CC_CALLFUNCO_SELECTOR(LayerBillFlowList::onGetListDataCallBack), GetMsgTypeString(MSGTYPE_BILL, MSGCMD_BILL::PAGE_FLOW), nullptr);
	clear();
	return true;
}
void LayerBillFlowList::Show(string strFrom, string strTo, int child){
	auto layerLoading = LayerLoading::create();
	layerLoading->setName("LayerLoading");
	_rootNode->addChild(layerLoading);
	_strFrom = strFrom;
	_strTo = strTo;
	_userid = child;
	//请求盈亏详细信息
	Api::Bill::sg_baccarat_bill_page_flow(_strFrom, _strTo, 1, REPORT_PAGE_SIZE, _userid);
}
void LayerBillFlowList::clear(){
	_totalCount = 0;
	_curRecord = nullptr;
	for (auto record : _records)
		delete record;
	_records.clear();
	_listViewReport->removeAllChildren();
}
void LayerBillFlowList::onSelectedItemEvent(Ref *pSender, ListView::EventType type){
	if (type == ListView::EventType::ON_SELECTED_ITEM_END){
		auto item = _listViewReport->getItem(_listViewReport->getCurSelectedIndex());
		_curRecord = (BillFlow *)item->getUserData();
		//暂不处理，以后可能增加单条盈亏记录详细显示页面
	}
}
void LayerBillFlowList::onSelectedItemEventScrollView(Ref *pSender, ScrollView::EventType type){
	if (type == ScrollView::EventType::SCROLL_TO_BOTTOM){
		if (_records.size() < _totalCount){
			if (_rootNode->getChildByName("LayerLoading"))
				return;		//loading时不响应其他事件
			auto layerLoading = LayerLoading::create();
			layerLoading->setName("LayerLoading");
			_rootNode->addChild(layerLoading);
			//请求下一页列表数据
			int pageIndex = _records.size() / REPORT_PAGE_SIZE + 1;
			Api::Bill::sg_baccarat_bill_page_flow(_strFrom, _strTo, pageIndex, REPORT_PAGE_SIZE, _userid);
		}
	}
}
void LayerBillFlowList::onGetListDataCallBack(Ref *pSender){
	if (pSender == nullptr || !_rootNode->isVisible())
		return;
	RECVMSG* msg = (RECVMSG*)pSender;
	msg->retain();
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
		auto layerLoading = _rootNode->getChildByName<LayerLoading*>("LayerLoading");
		if (layerLoading)
			layerLoading->removeFromParent();
		if (msg->code == 0)
		{
			vector<BillFlow*> records;
			int totalCount;
			if (InitListData(msg->data, &totalCount, &records)){
				_totalCount = totalCount;
                _txtTotal->setString(StringUtils::format(Language::getStringByKey("TotalFormat"), totalCount));
				//显示报表数据
				for (auto record : records) {
					_listViewReport->pushBackDefaultItem();
					auto layout = _listViewReport->getItems().back();
					Text *txtProfitID = layout->getChildByName<Text*>("txtProfitID");
					Text *txtSettlementTime = layout->getChildByName<Text*>("txtSettlementTime");
					Text *txtProfitType = layout->getChildByName<Text*>("txtProfitType");
					Text *txtProfit = layout->getChildByName<Text*>("txtProfit");
					layout->setUserData(record);
					_records.push_back(record);
					char strBuf[255];
					sprintf(strBuf, "%d", record->flow_id);
					txtProfitID->setString(strBuf);
					txtProfitID->setColor(Color3B(249, 9, 243));
					txtSettlementTime->setString(Comm::GetLocalTimeStrngFromUTCString(record->create_time));
					switch (record->business_type) {
					case TYPE_DEPOSIT:
						txtProfitType->setString(Language::getStringByKey("Deposit"));
						break;
					case TYPE_WITHDRAW:
						txtProfitType->setString(Language::getStringByKey("TakeOut"));
						break;
					case TYPE_EXCHANGE:
						txtProfitType->setString(Language::getStringByKey("Buy"));
						break;
					case TYPE_ADMIN_FREEZE:
						txtProfitType->setString(Language::getStringByKey("Freeze"));
						break;
					case TYPE_CASINO:
						//游戏记录
					{
						switch (record->operate_type)
						{
						case 6:
							txtProfitType->setString(Language::getStringByKey("Bet"));
							break;
						case 7:
							txtProfitType->setString(Language::getStringByKey("Profit"));
							break;
						case 8:
							txtProfitType->setString(Language::getStringByKey("Commission"));
							break;
						default:
							txtProfitType->setString(Language::getStringByKey("Game"));
							break;
						}
					}
						break;
					case TYPE_TASK:
					{
						txtProfitType->setString(Language::getStringByKey("Bonus"));
					}
						break;
					default:
						break;
					}
					txtProfit->setString(Comm::GetShortStringFromInt64(record->amount));
					if (record->amount >= 0) {
						txtProfit->setColor(Color3B(18, 219, 75));
					}
					else {
						txtProfit->setColor(Color3B(255, 0, 0));
					}
				}
			}
			else
				CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBillFlowList::onGetListDataCallBack", "init billflow data failed.", msg->code, msg->data.c_str());
		}
		else
		{
			//失败
			CCLOG("[%s]:%s\t(code:%d) - %s", "LayerBillFlowList::onGetListDataCallBack", "get billflow data error.", msg->code, msg->data.c_str());
		}
		msg->release();
	});
}
bool LayerBillFlowList::InitListData(string jsonData, int *totalCount, vector<BillFlow*> *records){
	if (jsonData.empty() || !totalCount || !records)
		return false;
	cJSON *jsonMsg;
	if (!(jsonMsg = cJSON_Parse(jsonData.c_str())))
		return false;
	if (jsonMsg->type != cJSON_Array || cJSON_GetArraySize(jsonMsg) != 2)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	//设置初始值
	*totalCount = 0;
	records->clear();
	//先检查记录数
	cJSON *jsonMsgInner1;
	if (!(jsonMsgInner1 = cJSON_GetArrayItem(jsonMsg, 1)) || jsonMsgInner1->type != cJSON_Number) {
		cJSON_Delete(jsonMsg);
		return false;
	}
	else {
		*totalCount = jsonMsgInner1->valueint;
	}
	if (*totalCount <= 0){
		cJSON_Delete(jsonMsg);
		return true;
	}
	//解析报表项
	cJSON *jsonMsgInner = cJSON_GetArrayItem(jsonMsg, 0);
	if (jsonMsgInner->type != cJSON_Array)
	{
		cJSON_Delete(jsonMsg);
		return false;
	}
	int size = cJSON_GetArraySize(jsonMsgInner);
	cJSON *item;
	cJSON *flow_id, *create_time, *business_type, *operate_type, *amount;

	for (int i = 0; i < size; i++) {
		if (!(item = cJSON_GetArrayItem(jsonMsgInner, i)) || item->type != cJSON_Object
			|| !(flow_id = cJSON_GetObjectItem(item, "flow_id")) || flow_id->type != cJSON_Number
			|| !(create_time = cJSON_GetObjectItem(item, "create_time")) || create_time->type != cJSON_String
			|| !(business_type = cJSON_GetObjectItem(item, "business_type")) || business_type->type != cJSON_Number
			|| !(operate_type = cJSON_GetObjectItem(item, "operate_type")) || operate_type->type != cJSON_Number
			|| !(amount = cJSON_GetObjectItem(item, "amount")) || amount->type != cJSON_Number) {
			for (auto record : *records)
				delete record;
			records->clear();
			cJSON_Delete(jsonMsg);
			return false;
		}
		else {
			BillFlow *record = new BillFlow();
			record->flow_id = flow_id->valueint;
			record->create_time = create_time->valuestring;
			record->business_type = business_type->valueint;
			record->operate_type = operate_type->valueint;
			record->amount = amount->valuedouble;
			records->push_back(record);
		}
	}
	cJSON_Delete(jsonMsg);
	return true;
}
