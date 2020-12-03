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
		char gMdFrontAddr[100];						// 模拟行情前置地址
		TThostFtdcBrokerIDType gBrokerID;           // 模拟经纪商代码
		TThostFtdcInvestorIDType gInvesterID;       // 投资者账户名
		TThostFtdcPasswordType gInvesterPassword;   // 投资者密码

		MDStruct()
			:gMdFrontAddr("tcp://180.168.146.187:10111")
			, gBrokerID("9999")
			, gInvesterID("")
			, gInvesterPassword("")
		{}
	};

	MDStruct m_hq;	//行情

	void Init(const std::vector<std::string>& vecHydm);//初始化
	void Close();//关闭

private:
	inline std::string GetString(int nValue) const {
		return QString("%1").arg(nValue).toStdString();
	}
	inline std::string GetString(double dValue) const {
		return QString("%1").arg(dValue).toStdString();
	}

private:
	std::vector<std::string> m_vecHydm;//合约代码集合

public:
	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected() override;

	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	virtual void OnFrontDisconnected(int nReason) override;

	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse) override;


	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///请求查询组播合约响应
	//virtual void OnRspQryMulticastInstrument(CThostFtdcMulticastInstrumentField *pMulticastInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///订阅询价应答
	virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///取消订阅询价应答
	virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) override;

	///询价通知
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp) override;

private:

	CThostFtdcMdApi *g_pMdUserApi; // 行情指针
};
