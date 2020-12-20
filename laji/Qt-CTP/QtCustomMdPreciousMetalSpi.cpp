#include "QtCustomMdPreciousMetalSpi.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "TickToKlineHelper.h"

#pragma warning(disable : 4996)

// ---- 全局变量 ---- //
char *g_pInstrumentPreciousMetalID[] = { "贵金属","沪金指数","沪金2011","沪金2012","沪金2101","沪金2102","沪金2103",
"沪金2104","沪金2105","沪金2106","沪金2107","沪金2108","沪金2109","沪金2110","沪金2112","沪金2202",
"沪金2204","沪金2206","沪金2208","沪金2210","沪银指数","沪银2011","沪银2012","沪银2101","沪银2102",
"沪银2103","沪银2104","沪银2105","沪银2106","沪银2107","沪银2108","沪银2109","沪银2110" }; // 行情合约代码列表，中、上、大、郑交易所各选一种（行情订阅列表）
int instrumentPreciousMetalNum = 10;                                             // 行情合约订阅数量（行情订阅数量）
/*std::unordered_map<std::string, TickToKlineHelper> g_KlineHashPreciousMetal;              // 不同合约的k线存储表*/
std::unordered_map<std::string, TickToKlineHelper> g_KlineHashPreciousMetal;              // 不同合约的k线存储表

QtCustomMdPreciousMetalSpi::QtCustomMdPreciousMetalSpi(QObject *parent)
	: QObject(parent)
	, g_pMdUserPreciousMetalApi(nullptr)
{
}

QtCustomMdPreciousMetalSpi::~QtCustomMdPreciousMetalSpi()
{
}

void QtCustomMdPreciousMetalSpi::InitPreciousMetal()
{
	g_pMdUserPreciousMetalApi = CThostFtdcMdApi::CreateFtdcMdApi();   // 创建行情实例
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdPreciousMetalSpi(this);       // 创建行情回调实例
	g_pMdUserPreciousMetalApi->RegisterSpi(this);               // 注册事件类
	g_pMdUserPreciousMetalApi->RegisterFront(this->m_hq.gMdFrontAddr);           // 设置行情前置地址
	g_pMdUserPreciousMetalApi->Init();                                // 连接运行
}

// ---- ctp_api回调函数 ---- //
// 连接成功应答
void QtCustomMdPreciousMetalSpi::OnFrontConnected()
{
	std::cout << "=====建立网络连接成功=====" << std::endl;
	// 开始登录
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, this->m_hq.gBrokerID);
	strcpy(loginReq.UserID, this->m_hq.gInvesterID);
	strcpy(loginReq.Password, this->m_hq.gInvesterPassword);
	static int requestID = 0; // 请求编号
	int rt = g_pMdUserPreciousMetalApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>发送登录请求成功" << std::endl;
	else
		std::cerr << "--->>>发送登录请求失败" << std::endl;
}

// 断开连接通知
void QtCustomMdPreciousMetalSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====网络连接断开=====" << std::endl;
	std::cerr << "错误码： " << nReason << std::endl;
}

// 心跳超时警告
void QtCustomMdPreciousMetalSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====网络心跳超时=====" << std::endl;
	std::cerr << "距上次连接时间： " << nTimeLapse << std::endl;
}

//	登录应答
void QtCustomMdPreciousMetalSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====账户登录成功=====" << std::endl;
		std::cout << "交易日： " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "登录时间： " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "经纪商： " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "帐户名： " << pRspUserLogin->UserID << std::endl;
		// 开始订阅行情
		int rt = g_pMdUserPreciousMetalApi->SubscribeMarketData(g_pInstrumentPreciousMetalID, instrumentPreciousMetalNum);
		if (!rt)
			std::cout << ">>>>>>发送订阅行情请求成功" << std::endl;
		else
			std::cerr << "--->>>发送订阅行情请求失败" << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 登出应答
void QtCustomMdPreciousMetalSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====账户登出成功=====" << std::endl;
		std::cout << "经纪商： " << pUserLogout->BrokerID << std::endl;
		std::cout << "帐户名： " << pUserLogout->UserID << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 错误通知
void QtCustomMdPreciousMetalSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 订阅行情应答
void QtCustomMdPreciousMetalSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====订阅行情成功=====" << std::endl;
		std::cout << "合约代码： " << pSpecificInstrument->InstrumentID << std::endl;
		// 如果需要存入文件或者数据库，在这里创建表头,不同的合约单独存储
		char filePath[100] = { '\0' };
		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
		std::ofstream outFile;
		outFile.open(filePath, std::ios::out); // 新开文件
		outFile << "合约代码" << ","
			<< "更新时间" << ","
			<< "最新价" << ","
			<< "成交量" << ","
			<< "买价一" << ","
			<< "买量一" << ","
			<< "卖价一" << ","
			<< "卖量一" << ","
			<< "持仓量" << ","
			<< "换手率"
			<< std::endl;
		outFile.close();
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 取消订阅行情应答
void QtCustomMdPreciousMetalSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====取消订阅行情成功=====" << std::endl;
		std::cout << "合约代码： " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 订阅询价应答
void QtCustomMdPreciousMetalSpi::OnRspSubForQuoteRsp(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====订阅询价成功=====" << std::endl;
		std::cout << "合约代码： " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 取消订阅询价应答
void QtCustomMdPreciousMetalSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====取消订阅询价成功=====" << std::endl;
		std::cout << "合约代码： " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 行情详情通知
void QtCustomMdPreciousMetalSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{

	QString ContractCode = pDepthMarketData->InstrumentID;                 //合约代码
	QString UpdataTime = pDepthMarketData->UpdateTime;                    //更新时间(最后修改时间）
	QString LastPrice = QString::number(pDepthMarketData->LastPrice);     //最新价
	QString BidPrice1 = QString::number(pDepthMarketData->BidPrice1);     //买价一（申买价一）
	QString BidVolume1 = QString::number(pDepthMarketData->BidVolume1);   //买一量(申买量一)
	QString AskPrice1 = QString::number(pDepthMarketData->AskPrice1);     //卖一价(申卖价一)
	QString AskVolume1 = QString::number(pDepthMarketData->AskVolume1);   //卖一量(申卖量一)
	QString Increace = QString::number(pDepthMarketData->LastPrice - pDepthMarketData->PreClosePrice * 100
		/ pDepthMarketData->PreClosePrice, 'f', 2);//涨幅()
	QString Volume = QString::number(pDepthMarketData->Volume);           //成交量(数量)
	QString UpperLimitPrice = QString::number(pDepthMarketData->UpperLimitPrice);//涨停价(涨停板价)
	QString LowerLimitPrice = QString::number(pDepthMarketData->LowerLimitPrice);//跌停价(跌停板价)
	QString MarketPreciousMetalTick = ContractCode + "," + UpdataTime + "," + LastPrice + "," + BidPrice1 + "," + BidVolume1 + ","
		+ AskPrice1 + "," + AskVolume1 + "," + Increace + "," + Volume + "," + UpperLimitPrice + "," + LowerLimitPrice;
	emit signalSendDataPreciousMetal(MarketPreciousMetalTick);//发送信号
}

// 询价详情通知
void QtCustomMdPreciousMetalSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	// 部分询价结果
	std::cout << "=====获得询价结果=====" << std::endl;
	std::cout << "交易日： " << pForQuoteRsp->TradingDay << std::endl;
	std::cout << "交易所代码： " << pForQuoteRsp->ExchangeID << std::endl;
	std::cout << "合约代码： " << pForQuoteRsp->InstrumentID << std::endl;
	std::cout << "询价编号： " << pForQuoteRsp->ForQuoteSysID << std::endl;
}


