#include "QtCustomMdSpi.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "TickToKlineHelper.h"
#include "../../PhdRedis.h"

#pragma warning(disable : 4996)

// ---- 全局变量 ---- //
char *g_pInstrumentID[] = { "IF加权", "IF2012", "IF2101","IF2102", "IF2103",  "IF2104", "IF2105", "IF2106", "IF2107", "IF2108", "IF2109", "IF2110","IF2111",
	"IH加权","IH2012", "IH2101","IH2102", "IH2103","IH2104", "IH2105", "IH2106", "IH2107", "IH2108", "IH2109", "IH2110","IH2111",
	"IC加权","IC2012", "IC2101","IC2102", "IC2103","IC2104", "IC2105", "IC2106", "IC2107", "IC2108", "IC2109", "IC2110","IC2111",
	"贵金属","au指数","au2012","au2101","au2102","au2103","au2104","au2105","au2106","au2107","au2108","au2109","au2110","au2111","au2112",
	"au2202","au2204","au2206","au2208","au2210","ag指数","ag2012","ag2101",
	"ag2102","ag2103","ag2104","ag2105","ag2106","ag2107","ag2108","ag2109","ag2110","ag2111"}; // 行情合约代码列表，中、上、大、郑交易所各选一种（行情订阅列表）
int instrumentNum = 72;

std::unordered_map<std::string, TickToKlineHelper> g_KlineHash;              // 不同合约的k线存储表

std::map<std::string, TimeSharingHelper> g_mapTimeSharing;                   //分时图数据

QtCustomMdSpi::QtCustomMdSpi(QObject *parent)
	: QObject(parent)
	, g_pMdUserApi(nullptr)
{
}

QtCustomMdSpi::~QtCustomMdSpi()
{
}

void QtCustomMdSpi::Init()
{
	g_pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();   // 创建行情实例
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdSpi(this);       // 创建行情回调实例
	g_pMdUserApi->RegisterSpi(this);               // 注册事件类
	g_pMdUserApi->RegisterFront(this->m_hq.gMdFrontAddr);           // 设置行情前置地址
	g_pMdUserApi->Init();                                // 连接运行
}

// ---- ctp_api回调函数 ---- //
// 连接成功应答
void QtCustomMdSpi::OnFrontConnected()
{
	std::cout << "=====建立网络连接成功=====" << std::endl;
	// 开始登录
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, this->m_hq.gBrokerID);
	strcpy(loginReq.UserID, this->m_hq.gInvesterID);
	strcpy(loginReq.Password,this->m_hq.gInvesterPassword);
	static int requestID = 0; // 请求编号
	int rt = g_pMdUserApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>发送登录请求成功" << std::endl;
	else
		std::cerr << "--->>>发送登录请求失败" << std::endl;
}

// 断开连接通知
void QtCustomMdSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====网络连接断开=====" << std::endl;
	std::cerr << "错误码： " << nReason << std::endl;
}

// 心跳超时警告
void QtCustomMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====网络心跳超时=====" << std::endl;
	std::cerr << "距上次连接时间： " << nTimeLapse << std::endl;
}

//	登录应答
void QtCustomMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
		int rt = g_pMdUserApi->SubscribeMarketData(g_pInstrumentID, instrumentNum);
		if (!rt)
			std::cout << ">>>>>>发送订阅行情请求成功" << std::endl;
		else
			std::cerr << "--->>>发送订阅行情请求失败" << std::endl;
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 登出应答
void QtCustomMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void QtCustomMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 订阅行情应答
void QtCustomMdSpi::OnRspSubMarketData(
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
// 		// 如果需要存入文件或者数据库，在这里创建表头,不同的合约单独存储
// 		char filePath[100] = { '\0' };
// 		//以合约代码ID加上_market_data.csv作为文件名
// 		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
// 		//std::ofstream是stl的输出文件类，用来写入文件的数据流
// 		std::ofstream outFile;
// 		outFile.open(filePath, std::ios::out); // 新开文件
// 		outFile << "合约代码" << ","
// 			<< "更新时间" << ","
// 			<< "最新价" << ","			
// 			<< "买价一" << ","
// 			<< "买量一" << ","
// 			<< "卖价一" << ","
// 			<< "卖量一" << ","
// 			<< "成交量" << ","
// 			<< "涨停价" << ","
// 			<< "跌停价"
// 			<< std::endl;
/*		outFile.close();*/
	}
	else
		std::cerr << "返回错误--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// 取消订阅行情应答
void QtCustomMdSpi::OnRspUnSubMarketData(
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
void QtCustomMdSpi::OnRspSubForQuoteRsp(
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
void QtCustomMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void QtCustomMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
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
	QString MarketTick = ContractCode + "," + UpdataTime + "," + LastPrice + "," + BidPrice1 + "," + BidVolume1 + ","
		+ AskPrice1 + "," + AskVolume1 + "," + Increace + "," + Volume + "," +  UpperLimitPrice + "," + LowerLimitPrice;
	emit signalSendData(MarketTick);//发送行情信息 信号

// 	// 打印行情，字段较多，截取部分
// 	std::cout << "=====获得深度行情=====" << std::endl;
// 	std::cout << "交易日： " << pDepthMarketData->TradingDay << std::endl;
// 	std::cout << "交易所代码： " << pDepthMarketData->ExchangeID << std::endl;
// 	std::cout << "合约代码： " << pDepthMarketData->InstrumentID << std::endl;
// 	std::cout << "合约在交易所的代码： " << pDepthMarketData->ExchangeInstID << std::endl;
// 	std::cout << "最新价： " << pDepthMarketData->LastPrice << std::endl;
// 	std::cout << "数量： " << pDepthMarketData->Volume << std::endl;
// 	// 如果只获取某一个合约行情，可以逐tick地存入文件或数据库
// 	char filePath[100] = { '\0' };
// 	sprintf(filePath, "%s_market_data.csv", pDepthMarketData->InstrumentID);
// 	std::ofstream outFile;
// 	outFile.open(filePath, std::ios::app); // 文件追加写入 
// 	outFile << pDepthMarketData->InstrumentID << ","
// 		<< pDepthMarketData->UpdateTime << "." << pDepthMarketData->LastPrice << ","		
// 		<< pDepthMarketData->BidPrice1 << ","
// 		<< pDepthMarketData->BidVolume1 << ","
// 		<< pDepthMarketData->AskPrice1 << ","
// 		<< pDepthMarketData->AskVolume1 << ","
// 		<< pDepthMarketData->LastPrice - pDepthMarketData->PreClosePrice * 100
// 		/ pDepthMarketData->PreClosePrice << ","
// 		<< pDepthMarketData->Volume << ","
// 		<< pDepthMarketData->UpperLimitPrice << ","
// 		<< pDepthMarketData->LowerLimitPrice << std::endl;
// 	outFile.close();

	// 计算实时k线
	std::string instrumentKey = std::string(pDepthMarketData->InstrumentID);
	if (g_KlineHash.find(instrumentKey) == g_KlineHash.end())
		g_KlineHash[instrumentKey] = TickToKlineHelper();
	g_KlineHash[instrumentKey].KLineFromRealtimeData(pDepthMarketData);

	QString strId = pDepthMarketData->InstrumentID;	//合约代码
	QString strOpen = QString("1%").arg(pDepthMarketData->OpenPrice);//开盘价
	QString strHigh = QString("1%").arg(pDepthMarketData->HighestPrice);//最高价
	QString strLow = QString("1%").arg(pDepthMarketData->LowestPrice);//最低价
	QString strClose = QString("1%").arg(pDepthMarketData->ClosePrice);//收盘价
	QString strVol = QString("1%").arg(pDepthMarketData->Volume);//成交量
	QString strMdtoK = strId + "," + strOpen + "," + strHigh + "," + strLow + "," + strClose + "," + strVol;

	emit signalSendMdToK(strMdtoK);//发送K线图的行情信息 信号

	//填充分时图数据
	if (g_mapTimeSharing.find(instrumentKey) == g_mapTimeSharing.end())
	{//如果没有该合约代码的数据，就创建一个
		g_mapTimeSharing[instrumentKey] = TimeSharingHelper();
	}
	g_mapTimeSharing[instrumentKey].AddTimeSharingFromRealtimeDataData(pDepthMarketData);//添加数据

/*	QString strId = pDepthMarketData->InstrumentID;	//合约代码*/
	QString strLastPrice = QString("%1").arg(pDepthMarketData->LastPrice);//最新价
	QString strBuy = QString("%1").arg(pDepthMarketData->BidPrice1);//买价
	QString strSell = QString("%1").arg(pDepthMarketData->AskPrice1);//卖价
	QString strMaxPrice = QString("%1").arg(pDepthMarketData->HighestPrice);//最高价
	QString strMinPrice = QString("%1").arg(pDepthMarketData->LowestPrice);//最低价

	QString strUpdateTime = pDepthMarketData->UpdateTime;//更新时间
	QString strPreClosePrice = QString("%1").arg(pDepthMarketData->PreClosePrice);//上次收盘价

	QString strMd = strId + "," + strLastPrice + "," + strBuy + "," + strSell + ","
		+ strMaxPrice + "," + strMinPrice + ","
		+ strUpdateTime + "," + strPreClosePrice;

	emit signalSendMdToFst(strMd);//发送分时图的行情信息 信号

// 	// 取消订阅行情
// 	int rt = g_pMdUserApi->UnSubscribeMarketData(g_pInstrumentID, instrumentNum);
// 	if (!rt)
// 		std::cout << ">>>>>>发送取消订阅行情请求成功" << std::endl;
// 	else
// 		std::cerr << "--->>>发送取消订阅行情请求失败" << std::endl;
}

// 询价详情通知
void QtCustomMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	// 部分询价结果
	std::cout << "=====获得询价结果=====" << std::endl;
	std::cout << "交易日： " << pForQuoteRsp->TradingDay << std::endl;
	std::cout << "交易所代码： " << pForQuoteRsp->ExchangeID << std::endl;
	std::cout << "合约代码： " << pForQuoteRsp->InstrumentID << std::endl;
	std::cout << "询价编号： " << pForQuoteRsp->ForQuoteSysID << std::endl;
}

