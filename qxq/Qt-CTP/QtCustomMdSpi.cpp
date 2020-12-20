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

// ---- ȫ�ֱ��� ---- //
char *g_pInstrumentID[] = { "IF��Ȩ", "IF2012", "IF2101","IF2102", "IF2103",  "IF2104", "IF2105", "IF2106", "IF2107", "IF2108", "IF2109", "IF2110","IF2111",
	"IH��Ȩ","IH2012", "IH2101","IH2102", "IH2103","IH2104", "IH2105", "IH2106", "IH2107", "IH2108", "IH2109", "IH2110","IH2111",
	"IC��Ȩ","IC2012", "IC2101","IC2102", "IC2103","IC2104", "IC2105", "IC2106", "IC2107", "IC2108", "IC2109", "IC2110","IC2111",
	"�����","auָ��","au2012","au2101","au2102","au2103","au2104","au2105","au2106","au2107","au2108","au2109","au2110","au2111","au2112",
	"au2202","au2204","au2206","au2208","au2210","agָ��","ag2012","ag2101",
	"ag2102","ag2103","ag2104","ag2105","ag2106","ag2107","ag2108","ag2109","ag2110","ag2111"}; // �����Լ�����б��С��ϡ���֣��������ѡһ�֣����鶩���б�
int instrumentNum = 72;

std::unordered_map<std::string, TickToKlineHelper> g_KlineHash;              // ��ͬ��Լ��k�ߴ洢��

std::map<std::string, TimeSharingHelper> g_mapTimeSharing;                   //��ʱͼ����

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
	g_pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();   // ��������ʵ��
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdSpi(this);       // ��������ص�ʵ��
	g_pMdUserApi->RegisterSpi(this);               // ע���¼���
	g_pMdUserApi->RegisterFront(this->m_hq.gMdFrontAddr);           // ��������ǰ�õ�ַ
	g_pMdUserApi->Init();                                // ��������
}

// ---- ctp_api�ص����� ---- //
// ���ӳɹ�Ӧ��
void QtCustomMdSpi::OnFrontConnected()
{
	std::cout << "=====�����������ӳɹ�=====" << std::endl;
	// ��ʼ��¼
	CThostFtdcReqUserLoginField loginReq;
	memset(&loginReq, 0, sizeof(loginReq));
	strcpy(loginReq.BrokerID, this->m_hq.gBrokerID);
	strcpy(loginReq.UserID, this->m_hq.gInvesterID);
	strcpy(loginReq.Password,this->m_hq.gInvesterPassword);
	static int requestID = 0; // ������
	int rt = g_pMdUserApi->ReqUserLogin(&loginReq, requestID);
	if (!rt)
		std::cout << ">>>>>>���͵�¼����ɹ�" << std::endl;
	else
		std::cerr << "--->>>���͵�¼����ʧ��" << std::endl;
}

// �Ͽ�����֪ͨ
void QtCustomMdSpi::OnFrontDisconnected(int nReason)
{
	std::cerr << "=====�������ӶϿ�=====" << std::endl;
	std::cerr << "�����룺 " << nReason << std::endl;
}

// ������ʱ����
void QtCustomMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	std::cerr << "=====����������ʱ=====" << std::endl;
	std::cerr << "���ϴ�����ʱ�䣺 " << nTimeLapse << std::endl;
}

//	��¼Ӧ��
void QtCustomMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
		int rt = g_pMdUserApi->SubscribeMarketData(g_pInstrumentID, instrumentNum);
		if (!rt)
			std::cout << ">>>>>>���Ͷ�����������ɹ�" << std::endl;
		else
			std::cerr << "--->>>���Ͷ�����������ʧ��" << std::endl;
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// �ǳ�Ӧ��
void QtCustomMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void QtCustomMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
	if (bResult)
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ��������Ӧ��
void QtCustomMdSpi::OnRspSubMarketData(
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
// 		// �����Ҫ�����ļ��������ݿ⣬�����ﴴ����ͷ,��ͬ�ĺ�Լ�����洢
// 		char filePath[100] = { '\0' };
// 		//�Ժ�Լ����ID����_market_data.csv��Ϊ�ļ���
// 		sprintf(filePath, "%s_market_data.csv", pSpecificInstrument->InstrumentID);
// 		//std::ofstream��stl������ļ��࣬����д���ļ���������
// 		std::ofstream outFile;
// 		outFile.open(filePath, std::ios::out); // �¿��ļ�
// 		outFile << "��Լ����" << ","
// 			<< "����ʱ��" << ","
// 			<< "���¼�" << ","			
// 			<< "���һ" << ","
// 			<< "����һ" << ","
// 			<< "����һ" << ","
// 			<< "����һ" << ","
// 			<< "�ɽ���" << ","
// 			<< "��ͣ��" << ","
// 			<< "��ͣ��"
// 			<< std::endl;
/*		outFile.close();*/
	}
	else
		std::cerr << "���ش���--->>> ErrorID=" << pRspInfo->ErrorID << ", ErrorMsg=" << pRspInfo->ErrorMsg << std::endl;
}

// ȡ����������Ӧ��
void QtCustomMdSpi::OnRspUnSubMarketData(
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
void QtCustomMdSpi::OnRspSubForQuoteRsp(
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
void QtCustomMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
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
void QtCustomMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
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
	QString MarketTick = ContractCode + "," + UpdataTime + "," + LastPrice + "," + BidPrice1 + "," + BidVolume1 + ","
		+ AskPrice1 + "," + AskVolume1 + "," + Increace + "," + Volume + "," +  UpperLimitPrice + "," + LowerLimitPrice;
	emit signalSendData(MarketTick);//����������Ϣ �ź�

// 	// ��ӡ���飬�ֶν϶࣬��ȡ����
// 	std::cout << "=====����������=====" << std::endl;
// 	std::cout << "�����գ� " << pDepthMarketData->TradingDay << std::endl;
// 	std::cout << "���������룺 " << pDepthMarketData->ExchangeID << std::endl;
// 	std::cout << "��Լ���룺 " << pDepthMarketData->InstrumentID << std::endl;
// 	std::cout << "��Լ�ڽ������Ĵ��룺 " << pDepthMarketData->ExchangeInstID << std::endl;
// 	std::cout << "���¼ۣ� " << pDepthMarketData->LastPrice << std::endl;
// 	std::cout << "������ " << pDepthMarketData->Volume << std::endl;
// 	// ���ֻ��ȡĳһ����Լ���飬������tick�ش����ļ������ݿ�
// 	char filePath[100] = { '\0' };
// 	sprintf(filePath, "%s_market_data.csv", pDepthMarketData->InstrumentID);
// 	std::ofstream outFile;
// 	outFile.open(filePath, std::ios::app); // �ļ�׷��д�� 
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

	// ����ʵʱk��
	std::string instrumentKey = std::string(pDepthMarketData->InstrumentID);
	if (g_KlineHash.find(instrumentKey) == g_KlineHash.end())
		g_KlineHash[instrumentKey] = TickToKlineHelper();
	g_KlineHash[instrumentKey].KLineFromRealtimeData(pDepthMarketData);

	QString strId = pDepthMarketData->InstrumentID;	//��Լ����
	QString strOpen = QString("1%").arg(pDepthMarketData->OpenPrice);//���̼�
	QString strHigh = QString("1%").arg(pDepthMarketData->HighestPrice);//��߼�
	QString strLow = QString("1%").arg(pDepthMarketData->LowestPrice);//��ͼ�
	QString strClose = QString("1%").arg(pDepthMarketData->ClosePrice);//���̼�
	QString strVol = QString("1%").arg(pDepthMarketData->Volume);//�ɽ���
	QString strMdtoK = strId + "," + strOpen + "," + strHigh + "," + strLow + "," + strClose + "," + strVol;

	emit signalSendMdToK(strMdtoK);//����K��ͼ��������Ϣ �ź�

	//����ʱͼ����
	if (g_mapTimeSharing.find(instrumentKey) == g_mapTimeSharing.end())
	{//���û�иú�Լ��������ݣ��ʹ���һ��
		g_mapTimeSharing[instrumentKey] = TimeSharingHelper();
	}
	g_mapTimeSharing[instrumentKey].AddTimeSharingFromRealtimeDataData(pDepthMarketData);//�������

/*	QString strId = pDepthMarketData->InstrumentID;	//��Լ����*/
	QString strLastPrice = QString("%1").arg(pDepthMarketData->LastPrice);//���¼�
	QString strBuy = QString("%1").arg(pDepthMarketData->BidPrice1);//���
	QString strSell = QString("%1").arg(pDepthMarketData->AskPrice1);//����
	QString strMaxPrice = QString("%1").arg(pDepthMarketData->HighestPrice);//��߼�
	QString strMinPrice = QString("%1").arg(pDepthMarketData->LowestPrice);//��ͼ�

	QString strUpdateTime = pDepthMarketData->UpdateTime;//����ʱ��
	QString strPreClosePrice = QString("%1").arg(pDepthMarketData->PreClosePrice);//�ϴ����̼�

	QString strMd = strId + "," + strLastPrice + "," + strBuy + "," + strSell + ","
		+ strMaxPrice + "," + strMinPrice + ","
		+ strUpdateTime + "," + strPreClosePrice;

	emit signalSendMdToFst(strMd);//���ͷ�ʱͼ��������Ϣ �ź�

// 	// ȡ����������
// 	int rt = g_pMdUserApi->UnSubscribeMarketData(g_pInstrumentID, instrumentNum);
// 	if (!rt)
// 		std::cout << ">>>>>>����ȡ��������������ɹ�" << std::endl;
// 	else
// 		std::cerr << "--->>>����ȡ��������������ʧ��" << std::endl;
}

// ѯ������֪ͨ
void QtCustomMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
	// ����ѯ�۽��
	std::cout << "=====���ѯ�۽��=====" << std::endl;
	std::cout << "�����գ� " << pForQuoteRsp->TradingDay << std::endl;
	std::cout << "���������룺 " << pForQuoteRsp->ExchangeID << std::endl;
	std::cout << "��Լ���룺 " << pForQuoteRsp->InstrumentID << std::endl;
	std::cout << "ѯ�۱�ţ� " << pForQuoteRsp->ForQuoteSysID << std::endl;
}

