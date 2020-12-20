#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "ThostFtdcMdApi.h"
#include <QString>

// k线数据结构
struct KLineDataType
{
	double open_price;   // 开
	double high_price;   // 高
	double low_price;    // 低
	double close_price;  // 收
	int volume;          // 量
};

//	计算k线的类
class TickToKlineHelper
{

public:
	// 从本地数据构建k线，并存储到本地(假定本地数据没有丢包)
	void KLineFromLocalData(const std::string &sFilePath, const std::string &dFilePath);
	// 从实时数据构建k线
	void KLineFromRealtimeData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	std::vector<double> m_priceVec; // 存储1分钟的价格
	std::vector<int> m_volumeVec; // 存储1分钟的成交量
	std::vector<KLineDataType> m_KLineDataArray;
};

//分时图数据
struct TimeSharingDataType
{
	QString m_strXTime;        //x轴时间
	double m_dYBuyPrice;       //买价
	double m_dYSellPrice;      //卖价
	double m_dPreClosePrice;   //纵轴最小值（昨日收盘价）
};

//计算分时图数据的类
class TimeSharingHelper
{
public:
	TimeSharingHelper();
	~TimeSharingHelper();
	// 从本地数据构建分时图曲线，并存储到本地(假定本地数据没有丢包)
	/*void TimeSharingFromLocalData(const std::string &sFilePath, const std::string &dFilePath);*/
	// 从实时数据构建分时图
	void AddTimeSharingFromRealtimeDataData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	std::vector< TimeSharingDataType> m_vecTimeSharingData;//存放分时图数据集合
private:

};

