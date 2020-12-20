#pragma once
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "ThostFtdcMdApi.h"
#include <QString>

// k�����ݽṹ
struct KLineDataType
{
	double open_price;   // ��
	double high_price;   // ��
	double low_price;    // ��
	double close_price;  // ��
	int volume;          // ��
};

//	����k�ߵ���
class TickToKlineHelper
{

public:
	// �ӱ������ݹ���k�ߣ����洢������(�ٶ���������û�ж���)
	void KLineFromLocalData(const std::string &sFilePath, const std::string &dFilePath);
	// ��ʵʱ���ݹ���k��
	void KLineFromRealtimeData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	std::vector<double> m_priceVec; // �洢1���ӵļ۸�
	std::vector<int> m_volumeVec; // �洢1���ӵĳɽ���
	std::vector<KLineDataType> m_KLineDataArray;
};

//��ʱͼ����
struct TimeSharingDataType
{
	QString m_strXTime;        //x��ʱ��
	double m_dYBuyPrice;       //���
	double m_dYSellPrice;      //����
	double m_dPreClosePrice;   //������Сֵ���������̼ۣ�
};

//�����ʱͼ���ݵ���
class TimeSharingHelper
{
public:
	TimeSharingHelper();
	~TimeSharingHelper();
	// �ӱ������ݹ�����ʱͼ���ߣ����洢������(�ٶ���������û�ж���)
	/*void TimeSharingFromLocalData(const std::string &sFilePath, const std::string &dFilePath);*/
	// ��ʵʱ���ݹ�����ʱͼ
	void AddTimeSharingFromRealtimeDataData(CThostFtdcDepthMarketDataField *pDepthMarketData);

	std::vector< TimeSharingDataType> m_vecTimeSharingData;//��ŷ�ʱͼ���ݼ���
private:

};

