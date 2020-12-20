#include "TickToKlineHelper.h"


const int kDataLineNum = 2 * 60; // 1����k����������(ĩβ����һ���ӵ���ȥ��)

void TickToKlineHelper::KLineFromLocalData(const std::string &sFilePath, const std::string &dFilePath)
{
	// �������������
	m_priceVec.clear();
	m_volumeVec.clear();
	m_KLineDataArray.clear();

	std::cout << "��ʼת��tick��k��..." << std::endl;
	// Ĭ�϶�ȡ��tick���ݱ���4���ֶΣ���Լ���롢����ʱ�䡢���¼ۡ��ɽ���
	std::ifstream srcInFile;
	std::ofstream dstOutFile;
	srcInFile.open(sFilePath, std::ios::in);
	dstOutFile.open(dFilePath, std::ios::out);
	dstOutFile << "���̼�" << ','
		<< "��߼�" << ','
		<< "��ͼ�" << ','
		<< "���̼�" << ','
		<< "�ɽ���" << std::endl;

	// һ������ļ�һ�߼���k�����ݣ�1����k��ÿ�ζ�ȡ60 * 2 = 120������
	std::string lineStr;
	bool isFirstLine = true;
	while (std::getline(srcInFile, lineStr))
	{
		if (isFirstLine)
		{
			// ������һ�б�ͷ
			isFirstLine = false;
			continue;
		}
		std::istringstream ss(lineStr);
		std::string fieldStr;
		int count = 4;
		while (std::getline(ss, fieldStr, ','))
		{
			count--;
			if (count == 1)
				m_priceVec.push_back(std::atof(fieldStr.c_str()));
			else if (count == 0)
			{
				m_volumeVec.push_back(std::atoi(fieldStr.c_str()));
				break;
			}
		}

		// ����k��

		if (m_priceVec.size() == kDataLineNum)
		{
			KLineDataType k_line_data;
			k_line_data.open_price = m_priceVec.front();
			k_line_data.high_price = *std::max_element(m_priceVec.cbegin(), m_priceVec.cend());
			k_line_data.low_price = *std::min_element(m_priceVec.cbegin(), m_priceVec.cend());
			k_line_data.close_price = m_priceVec.back();
			// �ɽ�������ʵ���㷨�ǵ�ǰ�������һ���ɽ�����ȥ��ȥһ���������һ���ɽ���
			k_line_data.volume = m_volumeVec.back() - m_volumeVec.front();
			//m_KLineDataArray.push_back(k_line_data); // �˴����Դ浽�ڴ�

			dstOutFile << k_line_data.open_price << ','
				<< k_line_data.high_price << ','
				<< k_line_data.low_price << ','
				<< k_line_data.close_price << ','
				<< k_line_data.volume << std::endl;

			m_priceVec.clear();
			m_volumeVec.clear();
		}
	}

	srcInFile.close();
	dstOutFile.close();

	std::cout << "k�����ɳɹ�" << std::endl;

}

void TickToKlineHelper::KLineFromRealtimeData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	m_priceVec.push_back(pDepthMarketData->LastPrice);
	m_volumeVec.push_back(pDepthMarketData->Volume);
	if (m_priceVec.size() == kDataLineNum) //1����k����������(ĩβ����һ���ӵ���ȥ��)  kDataLineNum
	{
		KLineDataType k_line_data;
		k_line_data.open_price = m_priceVec.front();
		k_line_data.high_price = *std::max_element(m_priceVec.cbegin(), m_priceVec.cend());
		k_line_data.low_price = *std::min_element(m_priceVec.cbegin(), m_priceVec.cend());
		k_line_data.close_price = m_priceVec.back();
		// �ɽ�������ʵ���㷨�ǵ�ǰ�������һ���ɽ�����ȥ��ȥһ���������һ���ɽ���
		k_line_data.volume = m_volumeVec.back() - m_volumeVec.front();
		m_KLineDataArray.push_back(k_line_data); // �˴����Դ浽�ڴ�

		m_priceVec.clear();
		m_volumeVec.clear();
	}
}

TimeSharingHelper::TimeSharingHelper()
{

}

TimeSharingHelper::~TimeSharingHelper()
{

}

// void TimeSharingHelper::TimeSharingFromLocalData(const std::string &sFilePath, const std::string &dFilePath)
// {
// 	// �������������
// 	m_vecBuyPrice.clear();
// 	m_vecSellPrice.clear();
// 	m_vecTimeSharingData.clear();
// 
// 	std::cout << "��ʼת��tick����ʱͼ..." << std::endl;
// 	// Ĭ�϶�ȡ��tick���ݱ���4���ֶΣ�����ʱ�䡢��ۡ����ۡ��������̼�
// 	std::ifstream srcInFile;
// 	std::ofstream dstOutFile;
// 	srcInFile.open(sFilePath, std::ios::in);
// 	dstOutFile.open(dFilePath, std::ios::out);
// 	dstOutFile << "����ʱ��" << ','
// 		<< "���" << ','
// 		<< "����" << ','
// 		<< "�������̼�" << std::endl;
// 
// 	// һ������ļ�һ�߼���k�����ݣ�1����k��ÿ�ζ�ȡ60 * 2 = 120������
// 	std::string TimeSharingStr;
// 	bool isFirstTimeShaing = true;
// 	while (std::getline(srcInFile, TimeSharingStr))
// 	{
// 		if (isFirstTimeShaing)
// 		{
// 			// ������һ�б�ͷ
// 			isFirstTimeShaing = false;
// 			continue;
// 		}
// 		std::istringstream ss(TimeSharingStr);
// 		std::string fieldStr;
// 		int count = 4;
// 		while (std::getline(ss, fieldStr, ','))
// 		{
// 			count--;
// 			if (count == 1)
// 				m_vecBuyPrice.push_back(std::atof(fieldStr.c_str()));
// 			else if (count == 0)
// 			{
// 				m_vecSellPrice.push_back(std::atoi(fieldStr.c_str()));
// 				break;
// 			}
// 		}
// 
// 		// �����ʱͼ
// 
// 		if (m_vecBuyPrice.size() == kDataLineNum)
// 		{
// 			KLineDataType k_line_data;
// 			k_line_data.open_price = m_vecBuyPrice.front();
// 			k_line_data.high_price = *std::max_element(m_vecBuyPrice.cbegin(), m_vecBuyPrice.cend());
// 			k_line_data.low_price = *std::min_element(m_vecBuyPrice.cbegin(), m_vecSellPrice.cend());
// 			k_line_data.close_price = m_vecBuyPrice.back();
// 			// �ɽ�������ʵ���㷨�ǵ�ǰ�������һ���ɽ�����ȥ��ȥһ���������һ���ɽ���
// 			k_line_data.volume = m_vecSellPrice.back() - m_vecSellPrice.front();
// 			//m_KLineDataArray.push_back(k_line_data); // �˴����Դ浽�ڴ�
// 
// 			dstOutFile << k_line_data.open_price << ','
// 				<< k_line_data.high_price << ','
// 				<< k_line_data.low_price << ','
// 				<< k_line_data.close_price << ','
// 				<< k_line_data.volume << std::endl;
// 
// 			m_vecBuyPrice.clear();
// 			m_vecSellPrice.clear();
// 		}
// 	}
// 
// 	srcInFile.close();
// 	dstOutFile.close();
// 
// 	std::cout << "��ʱͼ���ɳɹ�" << std::endl;
// 
// }

void TimeSharingHelper::AddTimeSharingFromRealtimeDataData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	TimeSharingDataType data;
	data.m_strXTime = pDepthMarketData->UpdateTime;
	data.m_dPreClosePrice = pDepthMarketData->PreClosePrice;	//�������̼�
	data.m_dYBuyPrice = pDepthMarketData->BidPrice1;
	data.m_dYSellPrice = pDepthMarketData->AskPrice1;

	m_vecTimeSharingData.push_back(data);
}
