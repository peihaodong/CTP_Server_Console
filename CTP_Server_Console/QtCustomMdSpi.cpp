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

	g_pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();   // ��������ʵ��
	//CThostFtdcMdSpi *pMdUserSpi = new QtCustomMdSpi(this);       // ��������ص�ʵ��
	g_pMdUserApi->RegisterSpi(this);               // ע���¼���
	g_pMdUserApi->RegisterFront(this->m_hq.gMdFrontAddr);           // ��������ǰ�õ�ַ
	g_pMdUserApi->Init();                                // ��������
}

void QtCustomMdSpi::Close()
{
	//g_pMdUserApi->Join();
	g_pMdUserApi->Release();
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
		//�Ѻ�Լ���뼯��תΪ��ά����
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
		// ��ʼ��������
		int rt = g_pMdUserApi->SubscribeMarketData(ppHydm, nCount);
		//�ͷ�new�ĵ�ַ
		for (int i = 0; i < nCount; i++)
			delete[]ppHydm[i];
		delete[]ppHydm;
		//
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

// 		// ��������Ϣд��csv�ļ�
// 		QString strAppDir = QCoreApplication::applicationDirPath();
// 		QDir dir(strAppDir);
// 		dir.mkdir("csv");//����csvĿ¼
// 		dir.cd("csv");//����csvĿ¼
// 		//��ȡ��ǰ����
// 		QDateTime current_date_time = QDateTime::currentDateTime();
// 		QString current_date = current_date_time.toString("yyyy.MM.dd");
// 		dir.mkdir(current_date);//������ǰ����Ŀ¼
// 		dir.cd(current_date);//���뵱ǰ����Ŀ¼
// 		//�õ��ú�Լ����csv�ļ�·��
// 		QString strHydmName = QString("\\%1.csv").arg(pSpecificInstrument->InstrumentID);
// 		QString strCsvPath = dir.path();
// 		strCsvPath += strHydmName;
// 		//д��csvͷ����
// 		std::ofstream outFile;
// 		outFile.open(strCsvPath.toStdString(), std::ios::out); // �¿��ļ�
// 		outFile << "��Լ����" << ","
// 			<< "����ʱ��" << ","
// 			<< "���¼�" << ","
// 			<< "�ɽ���" << ","
// 			<< "���һ" << ","
// 			<< "����һ" << ","
// 			<< "����һ" << ","
// 			<< "����һ" << ","
// 			<< "�ֲ���" << ","
// 			<< "������"
// 			<< std::endl;
// 		outFile.close();
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
	// ��������Ϣд��csv�ļ�
	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//����csvĿ¼
	dir.cd("csv");//����csvĿ¼
	//��ȡҵ������
	QString strCurDate = pDepthMarketData->ActionDay;//ҵ������
	strCurDate.insert(4, ".");
	strCurDate.insert(7, ".");
	dir.mkdir(strCurDate);//����ҵ������Ŀ¼
	dir.cd(strCurDate);//����ҵ������Ŀ¼
	//�õ��ú�Լ����csv�ļ�·��
	QString strHydmName = QString("\\%1.csv").arg(pDepthMarketData->InstrumentID);
	QString strCsvPath = dir.path();
	strCsvPath += strHydmName;

	std::ofstream outFile;
	outFile.open(strCsvPath.toStdString(), std::ios::app); // �ļ�׷��д�� 

	outFile << pDepthMarketData->InstrumentID << ","				//��Լ���루0��
		<< pDepthMarketData->UpdateTime << ","						//����޸�ʱ�䣨1��
		<< pDepthMarketData->LastPrice << ","						//���¼ۣ�2��
		<< pDepthMarketData->Volume << std::endl;					//������3��

// 	outFile << pDepthMarketData->InstrumentID << ","				//��Լ���루0��
// 		<< pDepthMarketData->UpdateTime << ","						//����޸�ʱ�䣨1��
// 		<< GetString(pDepthMarketData->UpdateMillisec) << ","		//����޸ĺ��루2��
// 		<< GetString(pDepthMarketData->LastPrice) << ","			//���¼ۣ�3��
// 		<< GetString(pDepthMarketData->Volume) << ","				//������4��
// 		<< GetString(pDepthMarketData->OpenPrice) << ","			//���̼ۣ�5��
// 		<< GetString(pDepthMarketData->HighestPrice) << ","			//��߼ۣ�6��
// 		<< GetString(pDepthMarketData->LowestPrice) << ","			//��ͼۣ�7��
// 		<< GetString(pDepthMarketData->ClosePrice) << ","			//�����̼ۣ�8��
//
// 		<< GetString(pDepthMarketData->BidPrice1) << ","			//�����һ��9��
// 		<< GetString(pDepthMarketData->BidVolume1) << ","			//������һ��10��
// 		<< GetString(pDepthMarketData->AskPrice1) << ","			//������һ��11��
// 		<< GetString(pDepthMarketData->AskVolume1) << ","			//������һ��12��
// 		<< GetString(pDepthMarketData->BidPrice2) << ","			//����۶���13��
// 		<< GetString(pDepthMarketData->BidVolume2) << ","			//����������14��
// 		<< GetString(pDepthMarketData->AskPrice2) << ","			//�����۶���15��
// 		<< GetString(pDepthMarketData->AskVolume2) << ","			//����������16��
// 		<< GetString(pDepthMarketData->BidPrice3) << ","			//���������17��
// 		<< GetString(pDepthMarketData->BidVolume3) << ","			//����������18��
// 		<< GetString(pDepthMarketData->AskPrice3) << ","			//����������19��
// 		<< GetString(pDepthMarketData->AskVolume3) << ","			//����������20��
// 		<< GetString(pDepthMarketData->BidPrice4) << ","			//������ģ�21��
// 		<< GetString(pDepthMarketData->BidVolume4) << ","			//�������ģ�22��
// 		<< GetString(pDepthMarketData->AskPrice4) << ","			//�������ģ�23��
// 		<< GetString(pDepthMarketData->AskVolume4) << ","			//�������ģ�24��
// 		<< GetString(pDepthMarketData->BidPrice5) << ","			//������壨25��
// 		<< GetString(pDepthMarketData->BidVolume5) << ","			//�������壨26��
// 		<< GetString(pDepthMarketData->AskPrice5) << ","			//�������壨27��
// 		<< GetString(pDepthMarketData->AskVolume5) << ","			//�������壨28��
// 
// 		<< GetString(pDepthMarketData->OpenInterest) << ","			//�ֲ�����29��
// 		<< GetString(pDepthMarketData->SettlementPrice) << ","		//���ν���ۣ�30��
// 		<< GetString(pDepthMarketData->UpperLimitPrice) << ","		//��ͣ��ۣ�31��
// 		<< GetString(pDepthMarketData->LowestPrice) << ","			//��ͣ��ۣ�32��
// 		<< GetString(pDepthMarketData->PreDelta) << ","				//����ʵ�ȣ�33��
// 		<< GetString(pDepthMarketData->CurrDelta) << ","			//����ʵ�ȣ�34��
// 		<< pDepthMarketData->TradingDay << ","						//�����գ�35��
// 		<< pDepthMarketData->InstrumentID << ","					//��������Լ���루36��
// 		<< pDepthMarketData->ExchangeInstID << ","					//��Լ�ڽ������Ĵ��루37��
// 		<< GetString(pDepthMarketData->PreSettlementPrice) << ","	//�ϴν���ۣ�38��
// 		<< GetString(pDepthMarketData->PreClosePrice) << ","		//�����̼ۣ�39��
// 		<< GetString(pDepthMarketData->PreOpenInterest) << ","		//��ֲ�����40��
// 		<< GetString(pDepthMarketData->AveragePrice) << ","			//���վ��ۣ�41��
// 		<< pDepthMarketData->ActionDay << ","						//ҵ�����ڣ�42��
//		<< GetString(pDepthMarketData->Turnover) << std::endl;		//�ɽ���43��

	outFile.close();

	// ȡ����������
	//int rt = g_pMdUserApi->UnSubscribeMarketData(g_pInstrumentID, instrumentNum);
	//if (!rt)
	//	std::cout << ">>>>>>����ȡ��������������ɹ�" << std::endl;
	//else
	//	std::cerr << "--->>>����ȡ��������������ʧ��" << std::endl;

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

