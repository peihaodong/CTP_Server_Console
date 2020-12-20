#include "QtChart.h"
#include "qcustomplot.h"
#include "TickToKlineHelper.h"
#include "QtCustomMdSpi.h"
#include <QtNetwork>
#include "../../PhdRedis.h"
#include "QStringList"
#include "ui_QtChart.h"
#include "qpainter.h"
#include "qevent.h"
#include "KeyEvent.h"
#include "QWidget"

//引用其他（外部）文件定义的全局变量
extern std::unordered_map<std::string, TickToKlineHelper> g_KlineHash;
extern std::map<std::string, TimeSharingHelper> g_mapTimeSharing;

QtChart::QtChart(QWidget *parent)
	: QWidget(parent), m_bShow(false)
	, m_pCandlesticksOneMinute(nullptr), m_pCandlesticksFiveMinute(nullptr), m_pCandlesticksFifteenMinute(nullptr)
	, m_pCandlesticksHalfHour(nullptr), m_pCandlesticksOneHour(nullptr), m_pCandlesticksFourHour(nullptr)
	, m_pCandlesticksOneDay(nullptr)
{
	ui.setupUi(this);
	
	ConnectAll();//所有的信号、按钮连接函数
	PushButtonSet();//按钮设置
	ChartInit();//图形初始化，包括分时图、K线图
	ui.customPlot_KLineOneMinute->setMouseTracking(true);//鼠标追踪
}

QtChart::~QtChart()
{
	
}

void QtChart::showEvent(QShowEvent *event)
{
	//每次显示界面重绘分时图
	m_vecXData.clear();
	m_vecYData.clear();
	ui.customPlot->graph(0)->setData(m_vecXData, m_vecYData);//显示数据
	ui.customPlot->replot();//重绘图像

	DrawKLine();//绘制各个时间段的K线图

	QWidget::showEvent(event);

	m_bShow = true;
}

void QtChart::hideEvent(QHideEvent *event)
{
	m_bShow = false;

	QWidget::hideEvent(event);
}

//分时图信息
void QtChart::slotMarketTimeSharingChart(QString dataTimeSharing)
{
	//当界面没有显示的时候不绘制
	if (!m_bShow)
		return;

	QStringList strList = dataTimeSharing.split(",");
	if (strList[0] != m_strHydm)//是否是要显示的合约代码
		return;

	QDateTime dateTime = QDateTime::currentDateTime();
	double  now = dateTime.toTime_t();//当前时间转化为秒

	double dY = strList[1].toDouble();//最新价，y轴坐标

	//距离上一次是否超过1秒
	double dSecond = 1;//秒
	int nSize = m_vecXData.size();
	if (nSize == 0)
	{
		m_dYMin = dY;
		m_dYMax = dY;
		ui.customPlot->xAxis->setRange(now, now + 300);//设置x轴范围，从当前时间起往后推5分钟
	}
	else
	{
		double dPreTime = m_vecXData[nSize - 1];
		if (now < (dPreTime + dSecond))
			return;
	}

	m_vecXData.push_back(now);
	m_vecYData.push_back(dY);

	if (dY > m_dYMax)
		m_dYMax = dY;
	if (dY < m_dYMin)
		m_dYMin = dY;

	ui.customPlot->yAxis->setRange(m_dYMin, m_dYMax);//设置y轴范围

	ui.customPlot->graph(0)->setData(m_vecXData, m_vecYData);//显示数据
	ui.customPlot->replot();//重绘图像
}

//1分钟k线图信息
void QtChart::slotMarketKLineOneMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 0, strValueKLineAll);                                 //从redis数据库中获取（db0）1分钟数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有1分钟数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 0,	CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);	
	ProcessingTextTickX(0, textTicker);                                             //x轴数据格式设置加工

	//1分钟K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartOneMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//1分钟K线图信息——MA均线绘制
	MaketKLineMAOneMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//5分钟K线图信息
void QtChart::slotMarketKLineFiveMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 1, strValueKLineAll);                                 //从redis数据库中获取（db1）5分钟数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有5分钟数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 1, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(1, textTicker);                                             //x轴数据格式设置加工

	//5分钟K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartFiveMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
    MA10Datas = calculateMA(CandleDatas, 10);
 	MA20Datas = calculateMA(CandleDatas, 20);
 	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//5分钟K线图信息——MA均线绘制
	MaketKLineMAFiveMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//15分钟K线图信息
void QtChart::slotMarketKLineFifteenMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 2, strValueKLineAll);                                 //从redis数据库中获取（db2）15分钟数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有15分钟数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 2, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(2, textTicker);                                             //x轴数据格式设置加工

	//15分钟K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartFifteenMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//15分钟K线图信息——MA均线绘制
	MaketKLineMAFifteenMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//30分钟K线图信息
void QtChart::slotMarketKLineHalfHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 3, strValueKLineAll);                                 //从redis数据库中获取（db3）30分钟数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有30分钟数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 3, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(3, textTicker);                                             //x轴数据格式设置加工

	//30分钟K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartHalfHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//30分钟K线图信息——MA均线绘制
	MaketKLineMAHalfHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//1小时K线图信息
void QtChart::slotMarketKLineOneHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 4, strValueKLineAll);                                 //从redis数据库中获取（db4）1小时数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有30分钟数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

// 	//遍历获取数据
// 	QString qstrValueKLine = QString::fromStdString(strValueKLineAll);
// 	QStringList strListRedisKLine = qstrValueKLine.split("\n");                     //得到每1小时的数据
// 	for (int i = 0; i < strListRedisKLine.size() - 1; i++)
// 	{
// 		QString qstrRedisKLineRow = strListRedisKLine.at(i);
// 		QStringList qstrListRedisKLineRow = qstrRedisKLineRow.split(",");
// 
// 		//qsting.replace 将从索引位置开始的n个字符替换为后面的字符串
// 		QString xTimeData = qstrListRedisKLineRow.at(1);//日期，年月日
// 		QString xTimeYear = xTimeData.replace(4, 6, "");
// 		xTimeData = qstrListRedisKLineRow.at(1);//日期，年月日
// 		QString xTimeDataMonth = xTimeData.right(5);//日期，月日
// 		xTimeDataMonth = xTimeDataMonth.left(2);//日期，月
// 		xTimeData = qstrListRedisKLineRow.at(1);//重新初始化
// 		QString xTimeDataDay = xTimeData.right(2);//日期，日
// 		QString xTimePoint = qstrListRedisKLineRow.at(2);//具体时间点，时分秒
// 		xTimePoint = xTimePoint.replace(5, 3, "");//具体时间点，时分
// 		QString xTime;
// 		if (xTimeDataMonth == "01")
// 		{
// 			//当月份为1时，时间轴显示为日期，年月
// 			xTime = xTimeYear + "." + xTimeDataMonth;
// 		}
// 		else
// 		{
// 			xTime = "";//初始化
// 			//正常情况下，时间轴显示为日期，月日
// 			xTime = xTimeDataMonth + "." + xTimeDataDay;
// 		}
// 		textTicker->addTick(i, xTime);//设置x轴时间
// 	}

    //处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 4, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(4, textTicker);                                             //x轴数据格式设置加工

	//1小时K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartOneHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//1小时K线图信息——MA均线绘制
	MaketKLineMAOneHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//4小时K线图信息
void QtChart::slotMarketKLineFourHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 5, strValueKLineAll);                         //从redis数据库中获取（db5）4小时数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有4小时数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 5, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(5, textTicker);                                             //x轴数据格式设置加工

	//4小时K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartFourHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//4小时K线图信息——MA均线绘制
	MaketKLineMAFourHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//日K线图信息
void QtChart::slotMarketKLineOneDayChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 6, strValueKLineAll);                                 //从redis数据库中获取（db6）日数据的集合

	QVector<QVector<double>> CandleDatas;                                           //所有  日 数据的集合
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//日期下标 以及 MA均线
	QVector<QString> m_qvecHydm;                                                    //合约代码
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //定义x轴文字轴

	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	ProcessingData(strValueKLineAll, 6, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(6, textTicker);                                             //x轴数据格式设置加工

	//日K线图信息——蜡烛图绘制
	MaketKLineCandlestickChartOneDay(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//日K线图信息——MA均线绘制
	MaketKLineMAOneDay(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//点击按钮槽函数,返回主界面——报价
void QtChart::slotButtonReturn()
{
	emit signalReturn();
}

//1分钟K线图显示
void QtChart::slotKLineOneMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(0);
}

//5分钟K线图显示
void QtChart::slotKLintFiveMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(2);
}

//15分钟K线图显示
void QtChart::slotKLineFifteenMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(4);
}

//30分钟K线图显示
void QtChart::slotKLineHalfHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(5);
}

//1小时K线图显示
void QtChart::slotKLineOneHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(6);
}

//4小时K线图显示
void QtChart::slotKLineFourHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(1);
}

//日K线图显示
void QtChart::slotKLineDayShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(7);
}

//所有的信号、按钮连接函数
void QtChart::ConnectAll()
{
	//绑定数据图像界面和主界面
	connect(ui.pushButton_return, SIGNAL(clicked()), this, SLOT(slotButtonReturn()));

	/*connect(ui.pushButtonK, SIGNAL(clicked()), this, SLOT(slotMarketKLineChart(QString)));*/
	connect(ui.pushButton_KLineOneMinute, SIGNAL(clicked()), this, SLOT(slotKLineOneMinuteShow()));//绑定1分钟K线图按钮显示
	connect(ui.pushButton_KLineFiveMinute, SIGNAL(clicked()), this, SLOT(slotKLintFiveMinuteShow()));//绑定5分钟K线图按钮显示
	connect(ui.pushButton_KLineFifteenMinute, SIGNAL(clicked()), this, SLOT(slotKLineFifteenMinuteShow()));//绑定15分钟K线图按钮显示
	connect(ui.pushButton_KLineHalfHour, SIGNAL(clicked()), this, SLOT(slotKLineHalfHourShow()));//绑定30分钟K线图按钮显示
	connect(ui.pushButton_KLineOneHour, SIGNAL(clicked()), this, SLOT(slotKLineOneHourShow()));//绑定1小时K线图按钮显示
	connect(ui.pushButton_KLineFourHour, SIGNAL(clicked()), this, SLOT(slotKLineFourHourShow()));//绑定4小时K线图按钮显示
	connect(ui.pushButton_KLineDay, SIGNAL(clicked()), this, SLOT(slotKLineDayShow()));//绑定日K线图按钮显示
}

//按钮设置
void QtChart::PushButtonSet()
{
	QString styleButton = "QPushButton{background:white;color:black;\
		border: 13px solid white;border-radius:18px;border-style: outset;}"//设定边框宽度及颜色
		"QPushButton:hover{background:violet; color: white;border: 13px solid violet;border-radius:18px;}"  //鼠标停靠时的颜色
		"QPushButton:pressed{background-color:rgb(85, 170, 255);border: 13px solid rgb(85, 170, 255);border-radius:18px;}";//鼠标按下时的颜色

	ui.pushButton_return->setStyleSheet(styleButton);
	ui.pushButton_KLineOneMinute->setStyleSheet(styleButton);
	ui.pushButton_KLineFiveMinute->setStyleSheet(styleButton);
	ui.pushButton_KLineFifteenMinute->setStyleSheet(styleButton);
	ui.pushButton_KLineHalfHour->setStyleSheet(styleButton);
	ui.pushButton_KLineOneHour->setStyleSheet(styleButton);
	ui.pushButton_KLineFourHour->setStyleSheet(styleButton);
	ui.pushButton_KLineDay->setStyleSheet(styleButton);
	// 	ui.pushButton_KLineWeek->setStyleSheet(styleButton);
	// 	ui.pushButton_KLineMonth->setStyleSheet(styleButton);
}

//图形初始化，包括分时图、K线图
void QtChart::ChartInit()
{
	//初始化分时图
	InitMarketTimeSharingChart();

	//初始化1分钟k线
	InitKLineOneMinuteChart();	
	//初始化5分钟k线
	InitKLineFiveMinuteChart();
	//初始化15分钟k线
	InitKLineFifteenMinuteChart();
	//初始化30分钟k线
	InitKLineHalfHourChart();
	//初始化1小时k线
	InitKLineOneHourChart();
	//初始化4小时K线
	InitKLineFourHourChart();
	//初始化日k线
	InitKLineOneDayChart();
}

//绘制各个时间段的K线图
void QtChart::DrawKLine()
{
	//绘制1分钟k线图
	emit slotMarketKLineOneMinuteChart(m_strHydm);
	ui.customPlot_KLineOneMinute->replot();//重绘

	//绘制5分钟k线图
	emit slotMarketKLineFiveMinuteChart(m_strHydm);
	ui.customPlot_KLineFiveMinute->replot();//重绘

	//绘制15分钟k线图
	emit slotMarketKLineFifteenMinuteChart(m_strHydm);
	ui.customPlot_KLineFifteenMinute->replot();//重绘

	//绘制30分钟k线图
	emit slotMarketKLineHalfHourChart(m_strHydm);
	ui.customPlot_KLineHalfHour->replot();//重绘

	//绘制1小时k线图
	emit slotMarketKLineOneHourChart(m_strHydm);
	ui.customPlot_KLineOneHour->replot();//重绘

	//绘制4小时k线图
	emit slotMarketKLineFourHourChart(m_strHydm);
	ui.customPlot_KLineFourHour->replot();//重绘

	//绘制日k线图
	emit slotMarketKLineOneDayChart(m_strHydm);
	ui.customPlot_KLineOneDay->replot();//重绘
}

//初始化分时图表
void QtChart::InitMarketTimeSharingChart()
{
	//初始化分时图，coutomPlot控件初始化
	ui.customPlot->setInteraction(QCP::iRangeDrag, true);//可以拖动
	ui.customPlot->setInteraction(QCP::iRangeZoom, true);//整体显示绘制的图
	//生成时间刻度对象
	QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
	ui.customPlot->xAxis->setTicker(dateTimeTicker);//时间刻度对象绑定customPlot控件
	dateTimeTicker->setTickCount(12);
	dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
	ui.customPlot->xAxis->setSubTicks(false);
	//ui.customPlot->xAxis->setRange(now, now + 3600 * 0.5);//x轴范围，从当前时间起往后推半小时
	//ui.customPlot->yAxis->setRange(4900, 5200);
	dateTimeTicker->setDateTimeFormat("hh:mm");//设置x轴刻度显示格式
	ui.customPlot->addGraph();//添加一条线
}

//初始化1分钟K线图表
void QtChart::InitKLineOneMinuteChart()
{
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneMinute = new QCPFinancial(ui.customPlot_KLineOneMinute->xAxis, ui.customPlot_KLineOneMinute->yAxis);
	m_pCandlesticksOneMinute->setName(QString::fromLocal8Bit("一分钟K"));
	m_pCandlesticksOneMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksOneMinute->setPenPositive(PenPositive);
	m_pCandlesticksOneMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksOneMinute->setPenNegative(PenNegative);

	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneMinute->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineOneMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneMinute->yAxis->setTickLabelColor(Qt::white);


// 	//设置坐标名称文本颜色
// 	ui.customPlot_KLineOneMinute->xAxis->setLabelColor(Qt::white);
// 	ui.customPlot_KLineOneMinute->yAxis->setLabelColor(Qt::white);

	//设置坐标轴颜色
	ui.customPlot_KLineOneMinute->xAxis->setBasePen(QPen(Qt::red));
// 	ui.customPlot_KLineOneMinute->xAxis->setTickPen(QPen(Qt::red));
// 	ui.customPlot_KLineOneMinute->xAxis->setSubTickPen(QPen(Qt::red));
	ui.customPlot_KLineOneMinute->yAxis->setBasePen(QPen(Qt::red));

	ui.customPlot_KLineOneMinute->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineOneMinute->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineOneMinute->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);//只允许水平拖动,不允许上下拖动
}

//初始化5分钟K线图表——蜡烛图、MA线
void QtChart::InitKLineFiveMinuteChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFiveMinute->setBackground(plotGradient);
// 	//设置QCPAxisRect背景颜色,不包括横纵坐标
// 	QLinearGradient axisRectGradient;
// 	axisRectGradient.setStart(0, 0);
// 	axisRectGradient.setFinalStop(0, 350);
// 	axisRectGradient.setColorAt(0, QColor(80, 80, 80));
// 	axisRectGradient.setColorAt(1, QColor(30, 30, 30));
// 	ui.customPlot_KLineFiveMinute->axisRect()->setBackground(axisRectGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineFiveMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFiveMinute->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineFiveMinute->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFiveMinute->yAxis->setBasePen(QPen(Qt::red));
    //设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFiveMinute = new QCPFinancial(ui.customPlot_KLineFiveMinute->xAxis, ui.customPlot_KLineFiveMinute->yAxis);
	m_pCandlesticksFiveMinute->setName(QString::fromLocal8Bit(" 5分钟K"));
	m_pCandlesticksFiveMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksFiveMinute->setPenPositive(PenPositive);
	m_pCandlesticksFiveMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksFiveMinute->setPenNegative(PenNegative);

	ui.customPlot_KLineFiveMinute->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineFiveMinute->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineFiveMinute->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动

	//图中插入文本框
	QCPItemText* textLabel;
	textLabel = new QCPItemText(ui.customPlot_KLineFiveMinute);            //在QCustomplot中新建文字框
	textLabel->setPositionAlignment(Qt::AlignAbsolute | Qt::AlignLeft);         //文字布局：顶、左对齐
	textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);        //位置类型（当前轴范围的比例为单位/实际坐标为单位）
	textLabel->position->setCoords(0, 0);                                  //把文字框放在X轴的最左边，Y轴的最顶部
	textLabel->setText("MA5:");
	textLabel->setColor(QColor("white"));
/*	textLabel->setFont(QFont(font().family(), 16)); //字体大小*/
/*	textLabel->setPen(QPen(Qt::white)); //字体颜色*/
/*	textLabel->setPadding(QMargins(2, 2, 2, 2));//文字距离边框几个像素*/
	
	//图中插入直线箭头
	QCPItemLine* arrow = new QCPItemLine(ui.customPlot_KLineFiveMinute);
	arrow->start->setParentAnchor(textLabel->bottom);                       //设置该直线的起点为文字框的下锚点  
	arrow->end->setCoords(4, 1.6);                                          //设置直线终点为坐标系中的点
	arrow->setHead(QCPLineEnding::esSpikeArrow);                            //设置箭头类型（三角形、菱形、方形等）
	arrow->setVisible(true);
	arrow->setPen(QColor("white"));
	arrow->start->setParentAnchor(textLabel->bottom);
		
// 	QCPItemTracer* tracer;
// 	tracer->setPen(QPen(Qt::DashLine));//游标线型：虚线
// 	tracer->setStyle(QCPItemTracer::tsPlus);//游标样式：十字星、圆圈、方框等

// 	plotKLineOneMinute = new XxwCustomPlot(ui.customPlot_KLineOneMinute);
// 	plotKLineOneMinute->showTracer(true);
}

//初始化15分钟K线图表
void QtChart::InitKLineFifteenMinuteChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFifteenMinute->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineFifteenMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFifteenMinute->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineFifteenMinute->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFifteenMinute->yAxis->setBasePen(QPen(Qt::red));
	//设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFifteenMinute = new QCPFinancial(ui.customPlot_KLineFifteenMinute->xAxis, ui.customPlot_KLineFifteenMinute->yAxis);
	m_pCandlesticksFifteenMinute->setName(QString::fromLocal8Bit(" 15分钟K"));
	m_pCandlesticksFifteenMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksFifteenMinute->setPenPositive(PenPositive);
	m_pCandlesticksFifteenMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksFifteenMinute->setPenNegative(PenNegative);
	
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineFifteenMinute->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineFifteenMinute->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动
}

//初始化30分钟K线图表
void QtChart::InitKLineHalfHourChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineHalfHour->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineHalfHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineHalfHour->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineHalfHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineHalfHour->yAxis->setBasePen(QPen(Qt::red));
	//设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksHalfHour = new QCPFinancial(ui.customPlot_KLineHalfHour->xAxis, ui.customPlot_KLineHalfHour->yAxis);
	m_pCandlesticksHalfHour->setName(QString::fromLocal8Bit(" 30分钟K"));
	m_pCandlesticksHalfHour->setBrushPositive(BrushPositive);
	m_pCandlesticksHalfHour->setPenPositive(PenPositive);
	m_pCandlesticksHalfHour->setBrushNegative(BrushNegative);
	m_pCandlesticksHalfHour->setPenNegative(PenNegative);

	ui.customPlot_KLineHalfHour->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineHalfHour->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineHalfHour->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动
}

//初始化1小时K线图表
void QtChart::InitKLineOneHourChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneHour->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineOneHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneHour->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineOneHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineOneHour->yAxis->setBasePen(QPen(Qt::red));
	//设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneHour = new QCPFinancial(ui.customPlot_KLineOneHour->xAxis, ui.customPlot_KLineOneHour->yAxis);
	m_pCandlesticksOneHour->setName(QString::fromLocal8Bit(" 1小时K"));
	m_pCandlesticksOneHour->setBrushPositive(BrushPositive);
	m_pCandlesticksOneHour->setPenPositive(PenPositive);
	m_pCandlesticksOneHour->setBrushNegative(BrushNegative);
	m_pCandlesticksOneHour->setPenNegative(PenNegative);

	ui.customPlot_KLineOneHour->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineOneHour->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineOneHour->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动
}

//初始化4小时K线图表
void QtChart::InitKLineFourHourChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFourHour->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineFourHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFourHour->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineFourHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFourHour->yAxis->setBasePen(QPen(Qt::red));
	//设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFourHour = new QCPFinancial(ui.customPlot_KLineFourHour->xAxis, ui.customPlot_KLineFourHour->yAxis);
	m_pCandlesticksFourHour->setName(QString::fromLocal8Bit(" 5分钟K"));
	m_pCandlesticksFourHour->setBrushPositive(BrushPositive);
	m_pCandlesticksFourHour->setPenPositive(PenPositive);
	m_pCandlesticksFourHour->setBrushNegative(BrushNegative);
	m_pCandlesticksFourHour->setPenNegative(PenNegative);

	ui.customPlot_KLineFourHour->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineFourHour->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineFourHour->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动
}

//初始化日K线图表
void QtChart::InitKLineOneDayChart()
{
	//设置背景颜色，包括横纵坐标,渐变效果
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneDay->setBackground(plotGradient);
	//设置坐标文本颜色
	ui.customPlot_KLineOneDay->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneDay->yAxis->setTickLabelColor(Qt::white);
	//设置坐标轴颜色
	ui.customPlot_KLineOneDay->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineOneDay->yAxis->setBasePen(QPen(Qt::red));
	//设置蜡烛图
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneDay = new QCPFinancial(ui.customPlot_KLineOneDay->xAxis, ui.customPlot_KLineOneDay->yAxis);
	m_pCandlesticksOneDay->setName(QString::fromLocal8Bit(" 日K"));
	m_pCandlesticksOneDay->setBrushPositive(BrushPositive);
	m_pCandlesticksOneDay->setPenPositive(PenPositive);
	m_pCandlesticksOneDay->setBrushNegative(BrushNegative);
	m_pCandlesticksOneDay->setPenNegative(PenNegative);

	ui.customPlot_KLineOneDay->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLineOneDay->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLineOneDay->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动
}

//db时间段K线图信息——从redis数据库中获取db库数据的集合
void QtChart::GetRedisValueAll(const QString &strHydm, const int &db,std::string &strValueKLineAll)
{
	PhdRedis redis;	                                                 
	if (!redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com"))//连接数据库
		return;	
	if (!redis.SwitchDb(db))                                         //切换数据库db
		return;	
	if (!redis.GetKeyValue(strHydm.toStdString(), strValueKLineAll)) //从redis数据库中获取1分钟数据的集合
		return;
}

//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
void QtChart::ProcessingData(const std::string &strValueKLineAll, const int &db, 
	QVector<QVector<double>> &CandleDatas,QVector<QString> &m_qvecHydm, QSharedPointer<QCPAxisTickerText> &textTicker,
	QVector<double> &MA5Datas, QVector<double> &MA10Datas, QVector<double> &MA20Datas, 
	QVector<double> &MA40Datas, QVector<double> &MA60Datas)
{
	//遍历获取数据、数据封装
	QString qstrValueKLine = QString::fromStdString(strValueKLineAll);
	QStringList strListRedisKLine = qstrValueKLine.split("\n");                //得到每1分钟的数据的集合
	QString xTime;                                                             //x轴   时间轴

	for (int i = 0; i < strListRedisKLine.size() - 1; i++)
	{
		QVector<double> rawDatas;                                              //每一个 时间段  数据的集合
		QString qstrRedisKLineRow = strListRedisKLine.at(i);
		QStringList qstrListRedisKLineRow = qstrRedisKLineRow.split(",");      //每一个 时间段  数据的具体数据
		for (int j = 0; j < qstrListRedisKLineRow.size(); j++)
		{
			ProcessingXTime(qstrListRedisKLineRow,db,xTime);                   //x轴  时间轴  数据处理,得到xTime
			textTicker->addTick(i, xTime);                                     //设置x轴时间，x轴文字轴赋值
			if (db <= 5)                                                       //绘制日以内时间的K线时，数据划分
			{
				if (j == 0)
				{
					m_qvecHydm.append(qstrListRedisKLineRow.at(j));            //合约代码集合赋值
				}
				else if (j >= 4)
				{
					rawDatas.append(qstrListRedisKLineRow.at(j).toDouble());   //价格集合——开盘、最高、最低、收盘
				}
			}
			else if(db == 6)                                                   //绘制日K时，数据划分
			{
				if (j == 0)
				{
					m_qvecHydm.append(qstrListRedisKLineRow.at(j));
				}
				else if (j >= 3)
				{
					rawDatas.append(qstrListRedisKLineRow.at(j).toDouble());    //价格集合——开盘、最高、最低、收盘
				}
			}	
		}
		CandleDatas.append(rawDatas);                                           //蜡烛图数据集合——价格集合——开盘、最高、最低、收盘
	}	
	
}

//x轴  时间轴  数据处理
void QtChart::ProcessingXTime(const QStringList &qstrListRedisKLineRow, const int &db, QString &xTime)
{
	if (db <= 5 && db >=0)
	{
		QString strYearMonthDay = qstrListRedisKLineRow.at(1);
		strYearMonthDay = strYearMonthDay.right(5);
		strYearMonthDay = strYearMonthDay.replace(QString("."), QString("/"));
		QString strHourMinute = qstrListRedisKLineRow.at(2);
		strHourMinute = strHourMinute.left(5);
		xTime = strYearMonthDay + " " + strHourMinute;
	}
	else if (db == 6)
	{
		xTime = qstrListRedisKLineRow.at(1);
		//qsting.replace 将从索引位置开始的n个字符替换为后面的字符串
		QString xTimeData = qstrListRedisKLineRow.at(1);                          //日期，年月日
		QString xTimeYear = xTimeData.replace(4, 6, "");                          //日期，年

		xTimeData = qstrListRedisKLineRow.at(1);                                  //重新初始化
		QString xTimeDataMonth = xTimeData.right(5);                              //日期，月日
		xTimeDataMonth = xTimeDataMonth.left(2);                                  //日期，月

		xTimeData = qstrListRedisKLineRow.at(1);                                  //重新初始化
		QString xTimeDataDay = xTimeData.right(2);                                //日期，日
		QString xTime = qstrListRedisKLineRow.at(1);

		xTime = xTimeDataDay;                                                 //正常情况下，时间轴显示为日期，月日
		if (xTimeDataMonth == "01" && xTimeDataDay == "01")
		{			
			xTime = xTimeYear + "." + xTimeDataMonth + "." + xTimeDataDay;        //当月份为1且日为1时，时间轴显示为日期，年月
		}
		else if (xTimeDataDay == "01" && xTimeDataMonth != "01")
		{
			xTime = xTimeDataMonth + "." + xTimeDataDay;
		}
		else
		{			
			xTime = xTimeDataDay;                                                 //正常情况下，时间轴显示为日期，月日
		}
	}
}

//x轴数据格式设置加工
void QtChart::ProcessingTextTickX(const int &db, QSharedPointer<QCPAxisTickerText> &textTicker)
{
	textTicker->setTickCount(5);                                                //x轴显示刻度数量
	if (db == 0)
	{
		ui.customPlot_KLineOneMinute->xAxis->setTicker(textTicker);                 //x轴数据填充
	}
	else if (db == 1)
	{
		ui.customPlot_KLineFiveMinute->xAxis->setTicker(textTicker);
	}
	else if (db == 2)
	{
		ui.customPlot_KLineFifteenMinute->xAxis->setTicker(textTicker);
	}
	else if (db == 3)
	{
		ui.customPlot_KLineHalfHour->xAxis->setTicker(textTicker);
	}
	else if (db == 4)
	{
		ui.customPlot_KLineOneHour->xAxis->setTicker(textTicker);
	}
	else if (db == 5)
	{
		ui.customPlot_KLineFourHour->xAxis->setTicker(textTicker);
	}
	else if (db == 6)
	{
		ui.customPlot_KLineOneDay->xAxis->setTicker(textTicker);
	}
	else
	{
		return;
	}
}

//1分钟K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartOneMinute(const QVector<QVector<double>> &CandleDatas, 
	const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker,
	QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;                                    //k线的数据集合
	QVector<double> m_qvecPrice;
	//设置蜡烛图数据
	//所有1分钟数据的集合,KLineOneMinute
	for (int i = 0; i < CandleDatas.size(); ++i)
	{
		timeDatas.append(i);                                                    //时间下标，x轴MA
		QCPFinancialData data;
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
/*		QCPFinancialData abc = datas.findBegin(20,512);*/
// 		double dYMax = CandleDatas.at(i).at(1);//最新价，y轴坐标
// 		double dYMin = CandleDatas.at(i).at(2);
// 		ui.customPlot_KLineOneMinute->yAxis->setRange(m_dYMin, m_dYMax);//设置y轴范围
		QString strHydmId = m_qvecHydm.at(i);                                     //合约代码
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit("  1分钟K");
		m_pCandlesticksOneMinute->setName(strTiltleName);
	}
	m_pCandlesticksOneMinute->data()->set(datas);                                 //填充蜡烛图数据
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineOneMinute->xAxis->range().center());
	ui.customPlot_KLineOneMinute->legend->setVisible(true);
	ui.customPlot_KLineOneMinute->legend->setBrush(QColor(255, 255, 255, 0));      //设置图例背景颜色,透明
	ui.customPlot_KLineOneMinute->legend->setTextColor(QColor("white"));           //设置图例文字颜色
	ui.customPlot_KLineOneMinute->rescaleAxes();                                   //重新调整坐标轴
	ui.customPlot_KLineOneMinute->replot();                                        //重绘
	//设置游标是否可见
	ui.customPlot_KLineOneMinute->showTracer(true);

	QVector<double> x, y;
	for (int i = 0; i < 5; i++)
	{
		x << i;
		y << i;
	}
	ui.customPlot_KLineOneMinute->xAxis->setLabelColor(Qt::white);
	ui.customPlot_KLineOneMinute->yAxis->setLabelColor(Qt::white);
	ui.customPlot_KLineOneMinute->addGraph();

	ui.customPlot_KLineOneMinute->graph(0)->setData(x, y);
/*	ui.customPlot_KLineOneMinute->graph(0)->setPen(QPen(Qt::yellow, 1));*/

	QCPScatterStyle sty;
	sty.setBrush(QBrush(Qt::green));
	sty.setShape(QCPScatterStyle::ssDisc);
	sty.setSize(10);
	sty.setPen(QPen(Qt::green));

	ui.customPlot_KLineOneMinute->graph(0)->setScatterStyle(sty);

	sty.setBrush(QBrush(Qt::green));
	sty.setShape(QCPScatterStyle::ssDisc);
	sty.setSize(10);
	sty.setPen(QPen(Qt::green));

	ui.customPlot_KLineOneMinute->graph(0)->setScatterStyle(sty);
	ui.customPlot_KLineOneMinute->xAxis->grid()->setPen(QColor(255, 255, 255, 0));          //横线网格线透明色
	ui.customPlot_KLineOneMinute->yAxis->grid()->subGridPen();

	QLinearGradient plotG;
	plotG.setStart(0, 0);
	plotG.setFinalStop(0, 350);
	plotG.setColorAt(0, QColor(80, 80, 80));
	plotG.setColorAt(1, QColor(30, 30, 30));	
	ui.customPlot_KLineOneMinute->setBackground(plotG);

	//允许用户用鼠标拖动轴范围，用鼠标滚轮缩放和点击选择图形:
	ui.customPlot_KLineOneMinute->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
		QCP::iSelectLegend | QCP::iSelectPlottables);

	QKeyEvent *event;
	ui.customPlot_KLineOneMinute->keyPressEventUp(event);
 	ui.customPlot_KLineOneMinute->keyReleaseEventUp(event);
/*	QKeyEvent *event;*/
	if (event->key() == Qt::Key_Up)
	{
		ui.customPlot_KLineOneMinute->setInteractions(QCP::iRangeDrag);
	}
	QCustomPlot *obj;
	QEvent *e;
	/*QKeyEvent *ek;*/
	eventFilter(ui.customPlot_KLineOneMinute,e);

	//使左轴和底部轴总是将它们的范围转移到右轴和顶部轴:
	connect(ui.customPlot_KLineOneMinute->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_KLineOneMinute->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.customPlot_KLineOneMinute->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_KLineOneMinute->yAxis2, SLOT(setRange(QCPRange)));

	ui.customPlot_KLineOneMinute->legend->setSelectableParts(QCPLegend::spItems);

	//在坐标轴右侧和上方画线，和X/Y轴一起形成一个矩形
	ui.customPlot_KLineOneMinute->axisRect()->setupFullAxesBox();
}

//5分钟K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartFiveMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;                                    //k线的数据集合
	//设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); ++i)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//合约代码
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit("  5分钟K");
		m_pCandlesticksFiveMinute->setName(strTiltleName);
	}
	m_pCandlesticksFiveMinute->data()->set(datas);//填充蜡烛图数据
	ui.customPlot_KLineFiveMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineFiveMinute->xAxis->range().center());
	ui.customPlot_KLineFiveMinute->yAxis->scaleRange(1.05, ui.customPlot_KLineFiveMinute->yAxis->range().center());
	// show legend with slightly transparent background brush:
	//用稍微透明的背景画笔显示图例:
	ui.customPlot_KLineFiveMinute->legend->setVisible(true);
	ui.customPlot_KLineFiveMinute->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineFiveMinute->legend->setTextColor(QColor("white"));//设置图例文字颜色

// 	// configure bottom axis to show date instead of number:
// 	//配置底部轴显示日期而不是数字:
// 	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
// 	dateTicker->setDateTimeFormat("d. MMMM\nyyyy");
// 	ui.customPlot_KLineFiveMinute->xAxis->setTicker(dateTicker);
// 	// configure left axis text labels:
// 	//配置左轴文本标签:
// 	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
// 	textTicker->addTick(10, "a bit\nlow");
// 	textTicker->addTick(50, "quite\nhigh");
// 	ui.customPlot_KLineFiveMinute->yAxis->setTicker(textTicker);
// 	// set a more compact font size for bottom and left axis tick labels:
// 	//为底部和左轴标记设置更紧凑的字体大小:
// 	ui.customPlot_KLineFiveMinute->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
// 	ui.customPlot_KLineFiveMinute->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
// 	// set axis labels:
// 	//设置轴标签:
// 	ui.customPlot_KLineFiveMinute->xAxis->setLabel("Date");
// 	ui.customPlot_KLineFiveMinute->yAxis->setLabel("Random wobbly lines value");
// 	// make top and right axes visible but without ticks and labels:
// 	//使顶部和右侧轴可见，但不带刻度和标签:
// 	ui.customPlot_KLineFiveMinute->xAxis2->setVisible(true);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setVisible(true);
// 	ui.customPlot_KLineFiveMinute->xAxis2->setTicks(false);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setTicks(false);
// 	ui.customPlot_KLineFiveMinute->xAxis2->setTickLabels(false);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setTickLabels(false);
// 	// set axis ranges to show all data:
// 	//设置轴范围显示所有数据:
// 	ui.customPlot_KLineFiveMinute->xAxis->setRange(now, now + 24 * 3600 * 249);
// 	ui.customPlot_KLineFiveMinute->yAxis->setRange(0, 60);
	ui.customPlot_KLineFiveMinute->legend->setVisible(true);
	ui.customPlot_KLineFiveMinute->legend->setBrush(QColor(255, 255, 255, 0));

	ui.customPlot_KLineFiveMinute->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineFiveMinute->replot();//重绘
}

//15分钟K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartFifteenMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k线的数据集合
	//设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;//一条数据
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//合约代码
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 15分钟K");
		m_pCandlesticksFifteenMinute->setName(strTiltleName);
	}
	m_pCandlesticksFifteenMinute->data()->set(datas);
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineFifteenMinute->xAxis->range().center());
	ui.customPlot_KLineFifteenMinute->yAxis->scaleRange(1.05, ui.customPlot_KLineFifteenMinute->yAxis->range().center());
	ui.customPlot_KLineFifteenMinute->legend->setVisible(true);
	ui.customPlot_KLineFifteenMinute->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineFifteenMinute->legend->setTextColor(QColor("white"));//设置图例文字颜色
	ui.customPlot_KLineFifteenMinute->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineFifteenMinute->replot();//重绘
}

//30分钟K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartHalfHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k线的数据集合
    //设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;//一条数据
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//合约代码
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 30分钟K");
		m_pCandlesticksHalfHour->setName(strTiltleName);
	}
	m_pCandlesticksHalfHour->data()->set(datas);
	ui.customPlot_KLineHalfHour->xAxis->scaleRange(1.05, ui.customPlot_KLineHalfHour->xAxis->range().center());
	ui.customPlot_KLineHalfHour->yAxis->scaleRange(1.05, ui.customPlot_KLineHalfHour->yAxis->range().center());
	ui.customPlot_KLineHalfHour->legend->setVisible(true);
	ui.customPlot_KLineHalfHour->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineHalfHour->legend->setTextColor(QColor("white"));//设置图例文字颜色
	ui.customPlot_KLineHalfHour->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineHalfHour->replot();//重绘
}

//1小时K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartOneHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k线的数据集合
	//设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;//一条数据
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//合约代码
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 1小时K");
		m_pCandlesticksOneHour->setName(strTiltleName);
	}
	m_pCandlesticksOneHour->data()->set(datas);
	ui.customPlot_KLineOneHour->xAxis->setTicker(textTicker);
	ui.customPlot_KLineOneHour->xAxis->scaleRange(1.05, ui.customPlot_KLineOneHour->xAxis->range().center());
	ui.customPlot_KLineOneHour->yAxis->scaleRange(1.05, ui.customPlot_KLineOneHour->yAxis->range().center());
	ui.customPlot_KLineOneHour->legend->setVisible(true);
	ui.customPlot_KLineOneHour->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineOneHour->legend->setTextColor(QColor("white"));//设置图例文字颜色
	ui.customPlot_KLineOneHour->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineOneHour->replot();//重绘
}

//4小时K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartFourHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k线的数据集合
	//设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;//一条数据
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
// 		//qsting.replace 将从索引位置开始的n个字符替换为后面的字符串
// 		QString xTimeData = qstrListRedisKLineRow.at(1);//日期，年月日
// 		QString xTimeYear = xTimeData.replace(0, 5, "");
// 		QString xTimeDataMonth = xTimeData.replace(0, 5, "");//日期，月日
// 		xTimeDataMonth = xTimeDataMonth.replace(2, 3, "");//日期，月
// 		QString xTimeDataDay = xTimeData.replace(0, 8, "");//日期，日
// 		QString xTimePoint = qstrListRedisKLineRow.at(2);//具体时间点，时分秒
// 		xTimePoint = xTimePoint.replace(5, 3, "");//具体时间点，时分
// 		QString xTime;
// 		if (xTimePoint == "00:00")
// 		{
// 			//当时间为零点时，时间轴显示为日期，月日
// 			xTime = xTimeDataDay;
// 		}
// 		else if (xTimeDataMonth == "01")
// 		{
// 			//当月份为1时，时间轴显示为日期，年
// 			xTime = xTimeYear;
// 		}
// 		else
// 		{
// 			//正常情况下，时间轴显示为时间点，时分
// 			xTime = xTimePoint;
// 		}
// 		textTicker->addTick(i, xTime);//设置x轴时间
		QString strHydmId = m_qvecHydm.at(i);
		QString strTiltleName = strHydmId + QString::fromLocal8Bit("  4小时K");
		m_pCandlesticksFourHour->setName(strTiltleName);
	}
	m_pCandlesticksFourHour->data()->set(datas);
	ui.customPlot_KLineFourHour->xAxis->scaleRange(1.05, ui.customPlot_KLineFourHour->xAxis->range().center());
	ui.customPlot_KLineFourHour->yAxis->scaleRange(1.05, ui.customPlot_KLineFourHour->yAxis->range().center());
	ui.customPlot_KLineFourHour->legend->setVisible(true);
	ui.customPlot_KLineFourHour->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineFourHour->legend->setTextColor(QColor("white"));//设置图例文字颜色
	ui.customPlot_KLineFourHour->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineFourHour->replot();//重绘
}

//日K线图信息——蜡烛图绘制
void QtChart::MaketKLineCandlestickChartOneDay(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k线的数据集合
	//设置蜡烛图数据
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//时间下标，x轴
		QCPFinancialData data;//一条数据
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 日K");
		m_pCandlesticksOneDay->setName(strTiltleName);
	}
	m_pCandlesticksOneDay->data()->set(datas);
	ui.customPlot_KLineOneDay->xAxis->scaleRange(1.05, ui.customPlot_KLineOneDay->xAxis->range().center());
	ui.customPlot_KLineOneDay->yAxis->scaleRange(1.05, ui.customPlot_KLineOneDay->yAxis->range().center());
	ui.customPlot_KLineOneDay->legend->setVisible(true);
	ui.customPlot_KLineOneDay->legend->setBrush(QColor(255, 255, 255, 0));//设置图例背景颜色,透明
	ui.customPlot_KLineOneDay->legend->setTextColor(QColor("white"));//设置图例文字颜色
	ui.customPlot_KLineOneDay->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLineOneDay->replot();//重绘
}

//1分钟K线图信息——MA均线绘制
void QtChart::MaketKLineMAOneMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineOneMinute->clearGraphs();

	//添加5条MD均线
	QCPGraph *m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA5");
	m_pGraphOneMinute->setData(timeDatas, MA5Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(0));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);//开启平滑曲线

	m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA10");
	m_pGraphOneMinute->setData(timeDatas, MA10Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(1));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);

	m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA20");
	m_pGraphOneMinute->setData(timeDatas, MA20Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(2));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);

	m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA40");
	m_pGraphOneMinute->setData(timeDatas, MA40Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(3));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);

	m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA60");
	m_pGraphOneMinute->setData(timeDatas, MA60Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(4));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);
}

//5分钟K线图信息——MA均线绘制
void QtChart::MaketKLineMAFiveMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineFiveMinute->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//15分钟K线图信息——MA均线绘制
void QtChart::MaketKLineMAFifteenMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineFifteenMinute->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//30分钟K线图信息——MA均线绘制
void QtChart::MaketKLineMAHalfHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineHalfHour->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//1小时K线图信息——MA均线绘制
void QtChart::MaketKLineMAOneHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineOneHour->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//4小时K线图信息——MA均线绘制
void QtChart::MaketKLineMAFourHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineFourHour->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//日K线图信息——MA均线绘制
void QtChart::MaketKLineMAOneDay(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//每次显示曲线之前，先清除上一次绘制的曲线
	ui.customPlot_KLineOneDay->clearGraphs();

	// 	添加5条MA均线
	QCPGraph *graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//开启平滑曲线

	graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA10");
	graph->setData(timeDatas, MA10Datas);
	graph->setPen(ColorOptions.at(1));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(1), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA20");
	graph->setData(timeDatas, MA20Datas);
	graph->setPen(ColorOptions.at(2));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(2), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA40");
	graph->setData(timeDatas, MA40Datas);
	graph->setPen(ColorOptions.at(3));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(3), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);

	graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA60");
	graph->setData(timeDatas, MA60Datas);
	graph->setPen(ColorOptions.at(4));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(4), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);
}

//计算MA移动平均线
QVector<double> QtChart::calculateMA(const QVector<QVector<double> > &v, int dayCount)
{
	auto func = [](double result, const QVector<double> &v2)
	{
		return result + v2[3];
	};

	QVector<double> result;
	for (int i = 0; i < v.size(); ++i) 
	{
		if (i < dayCount) 
		{
/*			result.append(qQNaN());*/
			auto iter = v.begin();
			iter += i;
			double dValue = (*iter)[3];
			result.append(dValue);
		}
		else 
		{
			double sum = std::accumulate(v.begin() + i + 1 - dayCount, v.begin() + i + 1, 0.0, func);
			result.append(sum / dayCount);
		}
	}
	return result;
}