#include "QtCustomMdPreciousMetalSpi.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include "TickToKlineHelper.h"

#pragma warning(disable : 4996)

// ---- ȫ�ֱ��� ---- //
char *g_pInstrumentPreciousMetalID[] = { "�����","����ָ��","����2011","����2012","����2101","����2102","����2103",
"����2104","����2105","����2106","����2107","����2108","����2109","����2110","����2112","����2202",
"����2204","����2206","����2208","����2210","����ָ��","����2011","����2012","����2101","����2102",
"����2103","����2104","����2105","����2106","����2107","����2108","����2109","����2110" }; // �����Լ�����б��С��ϡ���֣��������ѡһ�֣����鶩���б�
int instrumentPreciousMetalNum = 10;                                             // �����Լ�������������鶩��������
/*std::unordered_map<std::string, TickToKlineHelper> g_KlineHashPreciousMetal;              // ��ͬ��Լ��k�ߴ洢��*/
std::unordered_map<std::string, TickToKlineHelper> g_KlineHashPreciousMetal;              // ��ͬ��Լ��k�ߴ洢��

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
	g_pMdUserPreciousMetalApi = CThostFtdcMdApi::CreateFtdcMdApi();   // ��������ʵ��
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdPreciousMetalSpi(this);       // ��������ص�ʵ��
	g_pMdUserPreciousMetalApi->RegisterSpi(this);               // ע���¼���
	g_pMdUserPreciousMetalApi->RegisterFront(this->m_hq.gMdFrontAddr);           // ��������ǰ�õ�ַ
	g_pMdUserPreciousMetalApi->Init();                                // ��������
}

// ---- ctp_api�ص����� ---- //
// ���ӳɹ�Ӧ��
void QtCustomMdPreciousMetalSpi::OnFrontConnected()
{
	std::cout << "=====�����������ӳɹ�=====" << std::endl;
	// ��ʼ��¼
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, this->m_hq.gBrokerID);
	strcpy(loginReq.UserID, this->m_hq.gInvesterID);
	strcpy(loginReq.Password, this->m_hq.gInvesterPassword);
	static int requestID = 0; // ������
	int rt = g_pMdUserPreciousMetalApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>���͵�¼����ɹ�" << std::endl;
	else
		std::cerr << "--->>>���͵�¼����ʧ��" << std::endl;
}

// �Ͽ�����֪ͨ
void QtCustomMdPreciousMetalSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====�������ӶϿ�=====" << std::endl;
	std::cerr << "�����룺 " << nReason << std::endl;
}

// ������ʱ����
void QtCustomMdPreciousMetalSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====����������ʱ=====" << std::endl;
	std::cerr << "���ϴ�����ʱ�䣺 " << nTimeLapse << std::endl;
}

//	��¼Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====�˻���¼�ɹ�=====" << std::endl;
		std::cout << "�����գ� " << pRspUserLogin->TradingDay << std::endl;
		std::cout << "��¼ʱ�䣺 " << pRspUserLogin->LoginTime << std::endl;
		std::cout << "�����̣� " << pRspUserLogin->BrokerID << std::endl;
		std::cout << "�ʻ����� " << pRspUserLogin->UserID << std::endl;
		// ��ʼ��������
		int rt = g_pMdUserPreciousMetalApi->SubscribeMarketData(g_pInstrumentPreciousMetalID, instrumentPreciousMetalNum);
		if (!rt)
			std::cout << ">>>>>>���Ͷ�����������ɹ�" << std::endl;
		else
			std::cerr << "--->>>���Ͷ�����������ʧ��" << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// �ǳ�Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====�˻��ǳ��ɹ�=====" << std::endl;
		std::cout << "�����̣� " << pUserLogout->BrokerID << std::endl;
		std::cout << "�ʻ����� " << pUserLogout->UserID << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ����֪ͨ
void QtCustomMdPreciousMetalSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ��������Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====��������ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pSpecificInstrument->InstrumentID << std::endl;
		// �����Ҫ�����ļ��������ݿ⣬�����ﴴ����ͷ,��ͬ�ĺ�Լ�����洢
		char filePath[100] = { '\0' };
		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
		std::ofstream outFile;
		outFile.open(filePath, std::ios::out); // �¿��ļ�
		outFile << "��Լ����" << ","
			<< "����ʱ��" << ","
			<< "���¼�" << ","
			<< "�ɽ���" << ","
			<< "���һ" << ","
			<< "����һ" << ","
			<< "����һ" << ","
			<< "����һ" << ","
			<< "�ֲ���" << ","
			<< "������"
			<< std::endl;
		outFile.close();
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ȡ����������Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspUnSubMarketData(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====ȡ����������ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ����ѯ��Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspSubForQuoteRsp(
	CThostFtdcSpecificInstrumentField *pSpecificInstrument,
	CThostFtdcRspInfoField *pRspInfo,
	int nRequestID,
	bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====����ѯ�۳ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ȡ������ѯ��Ӧ��
void QtCustomMdPreciousMetalSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (!bResult)
	{
		std::cout << "=====ȡ������ѯ�۳ɹ�=====" << std::endl;
		std::cout << "��Լ���룺 " << pSpecificInstrument->InstrumentID << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ��������֪ͨ
void QtCustomMdPreciousMetalSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{

	QString ContractCode = pDepthMarketData->InstrumentID;                 //��Լ����
	QString UpdataTime = pDepthMarketData->UpdateTime;                    //����ʱ��(����޸�ʱ�䣩
	QString LastPrice = QString::number(pDepthMarketData->LastPrice);     //���¼�
	QString BidPrice1 = QString::number(pDepthMarketData->BidPrice1);     //���һ�������һ��
	QString BidVolume1 = QString::number(pDepthMarketData->BidVolume1);   //��һ��(������һ)
	QString AskPrice1 = QString::number(pDepthMarketData->AskPrice1);     //��һ��(������һ)
	QString AskVolume1 = QString::number(pDepthMarketData->AskVolume1);   //��һ��(������һ)
	QString Increace = QString::number(pDepthMarketData->LastPrice - pDepthMarketData->PreClosePrice * 100
		/ pDepthMarketData->PreClosePrice, 'f', 2);//�Ƿ�()
	QString Volume = QString::number(pDepthMarketData->Volume);           //�ɽ���(����)
	QString UpperLimitPrice = QString::number(pDepthMarketData->UpperLimitPrice);//��ͣ��(��ͣ���)
	QString LowerLimitPrice = QString::number(pDepthMarketData->LowerLimitPrice);//��ͣ��(��ͣ���)
	QString MarketPreciousMetalTick = ContractCode + "," + UpdataTime + "," + LastPrice + "," + BidPrice1 + "," + BidVolume1 + ","
		+ AskPrice1 + "," + AskVolume1 + "," + Increace + "," + Volume + "," + UpperLimitPrice + "," + LowerLimitPrice;
	emit signalSendDataPreciousMetal(MarketPreciousMetalTick);//�����ź�
}

// ѯ������֪ͨ
void QtCustomMdPreciousMetalSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	// ����ѯ�۽��
	std::cout << "=====���ѯ�۽��=====" << std::endl;
	std::cout << "�����գ� " << pForQuoteRsp->TradingDay << std::endl;
	std::cout << "���������룺 " << pForQuoteRsp->ExchangeID << std::endl;
	std::cout << "��Լ���룺 " << pForQuoteRsp->InstrumentID << std::endl;
	std::cout << "ѯ�۱�ţ� " << pForQuoteRsp->ForQuoteSysID << std::endl;
}


