#include "QtCustomMdSpi.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>

#pragma warning(disable : 4996)


QtCustomMdSpi::QtCustomMdSpi(QObject *parent)
	: QObject(parent)
	, g_pMdUserApi(nullptr)
{
}

QtCustomMdSpi::~QtCustomMdSpi()
{
}

void QtCustomMdSpi::Init(const std::vector<std::string>& vecHydm)
{
	m_vecHydm = vecHydm;

	g_pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();   // 创建行情实例
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdSpi(this);       // 创建行情回调实例
	g_pMdUserApi->RegisterSpi(this);               // 注册事件类
	g_pMdUserApi->RegisterFront(this->m_hq.gMdFrontAddr);           // 设置行情前置地址
	g_pMdUserApi->Init();                                // 连接运行
}

void QtCustomMdSpi::Close()
{
	//g_pMdUserApi->Join();
	g_pMdUserApi->Release();
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
		//把合约代码集合转为二维数据
		int nCount = m_vecHydm.size();
		char** ppHydm = new char*[nCount];
		std::memset(ppHydm,0,sizeof(ppHydm));
		for (int i = 0; i < m_vecHydm.size(); i++)
		{
			int nLen = m_vecHydm[i].size() + 1;
			char* pChar = new char[nLen];
			std::memset(pChar,0,nLen);
			std::strcpy(pChar,m_vecHydm[i].c_str());
			ppHydm[i] = pChar;
		}
		// 开始订阅行情
		int rt = g_pMdUserApi->SubscribeMarketData(ppHydm, nCount);
		//释放new的地址
		for (int i = 0; i < nCount; i++)
			delete[]ppHydm[i];
		delete[]ppHydm;
		//
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

// 		// 将行情信息写入csv文件
// 		QString strAppDir = QCoreApplication::applicationDirPath();
// 		QDir dir(strAppDir);
// 		dir.mkdir("csv");//创建csv目录
// 		dir.cd("csv");//进入csv目录
// 		//获取当前日期
// 		QDateTime current_date_time = QDateTime::currentDateTime();
// 		QString current_date = current_date_time.toString("yyyy.MM.dd");
// 		dir.mkdir(current_date);//创建当前日期目录
// 		dir.cd(current_date);//进入当前日期目录
// 		//得到该合约代码csv文件路径
// 		QString strHydmName = QString("\\%1.csv").arg(pSpecificInstrument->InstrumentID);
// 		QString strCsvPath = dir.path();
// 		strCsvPath += strHydmName;
// 		//写入csv头数据
// 		std::ofstream outFile;
// 		outFile.open(strCsvPath.toStdString(), std::ios::out); // 新开文件
// 		outFile << "合约代码" << ","
// 			<< "更新时间" << ","
// 			<< "最新价" << ","
// 			<< "成交量" << ","
// 			<< "买价一" << ","
// 			<< "买量一" << ","
// 			<< "卖价一" << ","
// 			<< "卖量一" << ","
// 			<< "持仓量" << ","
// 			<< "换手率"
// 			<< std::endl;
// 		outFile.close();
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
	// 将行情信息写入csv文件
	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//创建csv目录
	dir.cd("csv");//进入csv目录
	//获取业务日期
	QString strCurDate = pDepthMarketData->ActionDay;//业务日期
	strCurDate.insert(4, ".");
	strCurDate.insert(7, ".");
	dir.mkdir(strCurDate);//创建业务日期目录
	dir.cd(strCurDate);//进入业务日期目录
	//得到该合约代码csv文件路径
	QString strHydmName = QString("\\%1.csv").arg(pDepthMarketData->InstrumentID);
	QString strCsvPath = dir.path();
	strCsvPath += strHydmName;

	std::ofstream outFile;
	outFile.open(strCsvPath.toStdString(), std::ios::app); // 文件追加写入 

	outFile << pDepthMarketData->InstrumentID << ","				//合约代码（0）
		<< pDepthMarketData->UpdateTime << ","						//最后修改时间（1）
		<< pDepthMarketData->LastPrice << ","						//最新价（2）
		<< pDepthMarketData->Volume << std::endl;					//数量（3）

// 	outFile << pDepthMarketData->InstrumentID << ","				//合约代码（0）
// 		<< pDepthMarketData->UpdateTime << ","						//最后修改时间（1）
// 		<< GetString(pDepthMarketData->UpdateMillisec) << ","		//最后修改毫秒（2）
// 		<< GetString(pDepthMarketData->LastPrice) << ","			//最新价（3）
// 		<< GetString(pDepthMarketData->Volume) << ","				//数量（4）
// 		<< GetString(pDepthMarketData->OpenPrice) << ","			//今开盘价（5）
// 		<< GetString(pDepthMarketData->HighestPrice) << ","			//最高价（6）
// 		<< GetString(pDepthMarketData->LowestPrice) << ","			//最低价（7）
// 		<< GetString(pDepthMarketData->ClosePrice) << ","			//今收盘价（8）
//
// 		<< GetString(pDepthMarketData->BidPrice1) << ","			//申买价一（9）
// 		<< GetString(pDepthMarketData->BidVolume1) << ","			//申买量一（10）
// 		<< GetString(pDepthMarketData->AskPrice1) << ","			//申卖价一（11）
// 		<< GetString(pDepthMarketData->AskVolume1) << ","			//申卖量一（12）
// 		<< GetString(pDepthMarketData->BidPrice2) << ","			//申买价二（13）
// 		<< GetString(pDepthMarketData->BidVolume2) << ","			//申买量二（14）
// 		<< GetString(pDepthMarketData->AskPrice2) << ","			//申卖价二（15）
// 		<< GetString(pDepthMarketData->AskVolume2) << ","			//申卖量二（16）
// 		<< GetString(pDepthMarketData->BidPrice3) << ","			//申买价三（17）
// 		<< GetString(pDepthMarketData->BidVolume3) << ","			//申买量三（18）
// 		<< GetString(pDepthMarketData->AskPrice3) << ","			//申卖价三（19）
// 		<< GetString(pDepthMarketData->AskVolume3) << ","			//申卖量三（20）
// 		<< GetString(pDepthMarketData->BidPrice4) << ","			//申买价四（21）
// 		<< GetString(pDepthMarketData->BidVolume4) << ","			//申买量四（22）
// 		<< GetString(pDepthMarketData->AskPrice4) << ","			//申卖价四（23）
// 		<< GetString(pDepthMarketData->AskVolume4) << ","			//申卖量四（24）
// 		<< GetString(pDepthMarketData->BidPrice5) << ","			//申买价五（25）
// 		<< GetString(pDepthMarketData->BidVolume5) << ","			//申买量五（26）
// 		<< GetString(pDepthMarketData->AskPrice5) << ","			//申卖价五（27）
// 		<< GetString(pDepthMarketData->AskVolume5) << ","			//申卖量五（28）
// 
// 		<< GetString(pDepthMarketData->OpenInterest) << ","			//持仓量（29）
// 		<< GetString(pDepthMarketData->SettlementPrice) << ","		//本次结算价（30）
// 		<< GetString(pDepthMarketData->UpperLimitPrice) << ","		//涨停板价（31）
// 		<< GetString(pDepthMarketData->LowestPrice) << ","			//跌停板价（32）
// 		<< GetString(pDepthMarketData->PreDelta) << ","				//昨虚实度（33）
// 		<< GetString(pDepthMarketData->CurrDelta) << ","			//今虚实度（34）
// 		<< pDepthMarketData->TradingDay << ","						//交易日（35）
// 		<< pDepthMarketData->InstrumentID << ","					//交易所合约代码（36）
// 		<< pDepthMarketData->ExchangeInstID << ","					//合约在交易所的代码（37）
// 		<< GetString(pDepthMarketData->PreSettlementPrice) << ","	//上次结算价（38）
// 		<< GetString(pDepthMarketData->PreClosePrice) << ","		//昨收盘价（39）
// 		<< GetString(pDepthMarketData->PreOpenInterest) << ","		//昨持仓量（40）
// 		<< GetString(pDepthMarketData->AveragePrice) << ","			//当日均价（41）
// 		<< pDepthMarketData->ActionDay << ","						//业务日期（42）
//		<< GetString(pDepthMarketData->Turnover) << std::endl;		//成交金额（43）

	outFile.close();

	// 取消订阅行情
	//int rt = g_pMdUserApi->UnSubscribeMarketData(g_pInstrumentID, instrumentNum);
	//if (!rt)
	//	std::cout << ">>>>>>发送取消订阅行情请求成功" << std::endl;
	//else
	//	std::cerr << "--->>>发送取消订阅行情请求失败" << std::endl;

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

