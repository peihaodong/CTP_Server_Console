#pragma once

#include <QObject>
#include <ThostFtdcMdApi.h>

class QtCustomMdSpi : public QObject , public CThostFtdcMdSpi
{
	Q_OBJECT

public:
	QtCustomMdSpi(QObject *parent = nullptr);
	~QtCustomMdSpi();

public:
	struct MDStruct
	{
		char gMdFrontAddr[100];						// ģ������ǰ�õ�ַ
		TThostFtdcBrokerIDType gBrokerID;           // ģ�⾭���̴���
		TThostFtdcInvestorIDType gInvesterID;       // Ͷ�����˻���
		TThostFtdcPasswordType gInvesterPassword;   // Ͷ��������

		MDStruct()
			:gMdFrontAddr("tcp://180.168.146.187:10111")
			, gBrokerID("9999")
			, gInvesterID("")
			, gInvesterPassword("")
		{}
	};

	MDStruct m_hq;	//����

	void Init(const std::vector<std::string>& vecHydm);//��ʼ��
	void Close();//�ر�

private:
	inline std::string GetString(int nValue) const {
		return QString("%1").arg(nValue).toStdString();
	}
	inline std::string GetString(double dValue) const {
		return QString("%1").arg(dValue).toStdString();
	}

private:
	std::vector<std::string> m_vecHydm;//��Լ���뼯��

public:
	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected() override;

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        0x1001 �����ʧ��
	///        0x1002 ����дʧ��
	///        0x2001 ����������ʱ
	///        0x2002 ��������ʧ��
	///        0x2003 �յ�������
	virtual void OnFrontDisconnected(int nReason) override;

	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse) override;


	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///�����ѯ�鲥��Լ��Ӧ
	//virtual void OnRspQryMulticastInstrument(CThostFtdcMulticastInstrumentField *pMulticastInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///��������Ӧ��
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///ȡ����������Ӧ��
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///����ѯ��Ӧ��
	virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///ȡ������ѯ��Ӧ��
	virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///�������֪ͨ
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

	///ѯ��֪ͨ
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override;

private:

	CThostFtdcMdApi *g_pMdUserApi; // ����ָ��
};
