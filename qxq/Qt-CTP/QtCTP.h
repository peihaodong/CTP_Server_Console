#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtCTP.h"
#include "ThostFtdcMdApi.h"
#include "QtCustomMdSpi.h"
#include "qcustomplot.h"
#include <QtNetwork>
#include "QtChart.h"

class QtCTP : public QMainWindow
{
    Q_OBJECT

public:
    QtCTP(QWidget *parent = Q_NULLPTR);
	~QtCTP();
	//所有的信号与槽连接函数
	void ConnectAll();

	//1.行情表初始化
	void QtMarketTableWidgetStockInit();//行情——股指期货表格初始化
	bool SetFirstColumnStock(const QString& strFilePath);//设置股指期货表格第一列数据——合约代码
	void Selected();//行情信息被选中
	void QtMarketTableWidgetPreciousMetalInit();//行情——贵金属表格初始化
	void SetFirstColumnMetal();//设置贵金属表格第一列数据——合约代码

	//2.委托表初始化
	void QtEntrustTableWidget();//委托表初始化

    //3.成交表(Bargain)初始化
	void QtBargainTableWidget();//成交表(Bargain)初始化

	//4.1.持仓表初始化
	void QtPositionTableWidget();//持仓表初始化
	void QtFundTableWidget();//资金表初始化


public:
    Ui::QtCTPClass ui;
private:
	QtCustomMdSpi* m_pMd;//行情事件实例
	QtChart* m_pUi_Chart;//数据图像界面


	QVector<double> m_vecXData;
	QVector<double> m_vecYData;

	double m_dYMax = 0;//y轴的最大值
	double m_dYMin = 0;//y轴最小值

	bool m_bShow;		//界面是否显示的标记
	QString m_strHydm;	//合约代码

	//k线实例
	QCPFinancial* m_pCandlesticks;

	std::vector<QString> vecTypeStock;//股指期货合约代码集合

	bool ExitStockFutures(QString str);//判断是否有股指期货信息
	bool ExitPreciousMetal(QString str);//判断是否有贵金属信息



	//信号
signals:
	void signalSendMdToChart(QString);//主界面发送行情信息信号至数据图像界面
	
	//槽函数
private slots:
	void slotButtonLogin();	                     //点击登录按钮
    //接收行情信息
	void slotReceiveMarket(QString MarketTick);//显示行情信息
	void slotReceiveMarketStockFutures(QStringList strlist);//股指期货信息
	void slotReceiveMarketPreciousMetal(QStringList strlist);//贵金属信息
	
	void slotDoubleClicked(int row, int column);//双击行情股指期货表格槽函数
	void slotDoubleClickedPreciousMetal(int row,int column);//双击行情贵金属表格槽函数
	void slotReturnMain();//数据图界面跳转返回至主界面，在主界面文件中定义
	
	void slotShowPageChartTimeSharing();//分时图图像界面显示槽函数
	void slotShowPageMain();//报价界面显示槽函数
	void slotShowPageChartK();//K线图图像界面显示函数

	void slotCellClickedStock(int row, int column);

	void slotReceiveTimeSharing(QString dataTimeSharing);
	void slotReceiveKLineChart();//k线图信息	
};


