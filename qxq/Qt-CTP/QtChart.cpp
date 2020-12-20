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

//�����������ⲿ���ļ������ȫ�ֱ���
extern std::unordered_map<std::string, TickToKlineHelper> g_KlineHash;
extern std::map<std::string, TimeSharingHelper> g_mapTimeSharing;

QtChart::QtChart(QWidget *parent)
	: QWidget(parent), m_bShow(false)
	, m_pCandlesticksOneMinute(nullptr), m_pCandlesticksFiveMinute(nullptr), m_pCandlesticksFifteenMinute(nullptr)
	, m_pCandlesticksHalfHour(nullptr), m_pCandlesticksOneHour(nullptr), m_pCandlesticksFourHour(nullptr)
	, m_pCandlesticksOneDay(nullptr)
{
	ui.setupUi(this);
	
	ConnectAll();//���е��źš���ť���Ӻ���
	PushButtonSet();//��ť����
	ChartInit();//ͼ�γ�ʼ����������ʱͼ��K��ͼ
	ui.customPlot_KLineOneMinute->setMouseTracking(true);//���׷��
}

QtChart::~QtChart()
{
	
}

void QtChart::showEvent(QShowEvent *event)
{
	//ÿ����ʾ�����ػ��ʱͼ
	m_vecXData.clear();
	m_vecYData.clear();
	ui.customPlot->graph(0)->setData(m_vecXData, m_vecYData);//��ʾ����
	ui.customPlot->replot();//�ػ�ͼ��

	DrawKLine();//���Ƹ���ʱ��ε�K��ͼ

	QWidget::showEvent(event);

	m_bShow = true;
}

void QtChart::hideEvent(QHideEvent *event)
{
	m_bShow = false;

	QWidget::hideEvent(event);
}

//��ʱͼ��Ϣ
void QtChart::slotMarketTimeSharingChart(QString dataTimeSharing)
{
	//������û����ʾ��ʱ�򲻻���
	if (!m_bShow)
		return;

	QStringList strList = dataTimeSharing.split(",");
	if (strList[0] != m_strHydm)//�Ƿ���Ҫ��ʾ�ĺ�Լ����
		return;

	QDateTime dateTime = QDateTime::currentDateTime();
	double  now = dateTime.toTime_t();//��ǰʱ��ת��Ϊ��

	double dY = strList[1].toDouble();//���¼ۣ�y������

	//������һ���Ƿ񳬹�1��
	double dSecond = 1;//��
	int nSize = m_vecXData.size();
	if (nSize == 0)
	{
		m_dYMin = dY;
		m_dYMax = dY;
		ui.customPlot->xAxis->setRange(now, now + 300);//����x�᷶Χ���ӵ�ǰʱ����������5����
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

	ui.customPlot->yAxis->setRange(m_dYMin, m_dYMax);//����y�᷶Χ

	ui.customPlot->graph(0)->setData(m_vecXData, m_vecYData);//��ʾ����
	ui.customPlot->replot();//�ػ�ͼ��
}

//1����k��ͼ��Ϣ
void QtChart::slotMarketKLineOneMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 0, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db0��1�������ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����1�������ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 0,	CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);	
	ProcessingTextTickX(0, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//1����K��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartOneMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//1����K��ͼ��Ϣ����MA���߻���
	MaketKLineMAOneMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//5����K��ͼ��Ϣ
void QtChart::slotMarketKLineFiveMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 1, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db1��5�������ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����5�������ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 1, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(1, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//5����K��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartFiveMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
    MA10Datas = calculateMA(CandleDatas, 10);
 	MA20Datas = calculateMA(CandleDatas, 20);
 	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//5����K��ͼ��Ϣ����MA���߻���
	MaketKLineMAFiveMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//15����K��ͼ��Ϣ
void QtChart::slotMarketKLineFifteenMinuteChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 2, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db2��15�������ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����15�������ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 2, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(2, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//15����K��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartFifteenMinute(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//15����K��ͼ��Ϣ����MA���߻���
	MaketKLineMAFifteenMinute(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//30����K��ͼ��Ϣ
void QtChart::slotMarketKLineHalfHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 3, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db3��30�������ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����30�������ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 3, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(3, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//30����K��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartHalfHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//30����K��ͼ��Ϣ����MA���߻���
	MaketKLineMAHalfHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//1СʱK��ͼ��Ϣ
void QtChart::slotMarketKLineOneHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 4, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db4��1Сʱ���ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����30�������ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

// 	//������ȡ����
// 	QString qstrValueKLine = QString::fromStdString(strValueKLineAll);
// 	QStringList strListRedisKLine = qstrValueKLine.split("\n");                     //�õ�ÿ1Сʱ������
// 	for (int i = 0; i < strListRedisKLine.size() - 1; i++)
// 	{
// 		QString qstrRedisKLineRow = strListRedisKLine.at(i);
// 		QStringList qstrListRedisKLineRow = qstrRedisKLineRow.split(",");
// 
// 		//qsting.replace ��������λ�ÿ�ʼ��n���ַ��滻Ϊ������ַ���
// 		QString xTimeData = qstrListRedisKLineRow.at(1);//���ڣ�������
// 		QString xTimeYear = xTimeData.replace(4, 6, "");
// 		xTimeData = qstrListRedisKLineRow.at(1);//���ڣ�������
// 		QString xTimeDataMonth = xTimeData.right(5);//���ڣ�����
// 		xTimeDataMonth = xTimeDataMonth.left(2);//���ڣ���
// 		xTimeData = qstrListRedisKLineRow.at(1);//���³�ʼ��
// 		QString xTimeDataDay = xTimeData.right(2);//���ڣ���
// 		QString xTimePoint = qstrListRedisKLineRow.at(2);//����ʱ��㣬ʱ����
// 		xTimePoint = xTimePoint.replace(5, 3, "");//����ʱ��㣬ʱ��
// 		QString xTime;
// 		if (xTimeDataMonth == "01")
// 		{
// 			//���·�Ϊ1ʱ��ʱ������ʾΪ���ڣ�����
// 			xTime = xTimeYear + "." + xTimeDataMonth;
// 		}
// 		else
// 		{
// 			xTime = "";//��ʼ��
// 			//��������£�ʱ������ʾΪ���ڣ�����
// 			xTime = xTimeDataMonth + "." + xTimeDataDay;
// 		}
// 		textTicker->addTick(i, xTime);//����x��ʱ��
// 	}

    //�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 4, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(4, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//1СʱK��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartOneHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);

	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//1СʱK��ͼ��Ϣ����MA���߻���
	MaketKLineMAOneHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//4СʱK��ͼ��Ϣ
void QtChart::slotMarketKLineFourHourChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 5, strValueKLineAll);                         //��redis���ݿ��л�ȡ��db5��4Сʱ���ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����4Сʱ���ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 5, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(5, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//4СʱK��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartFourHour(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//4СʱK��ͼ��Ϣ����MA���߻���
	MaketKLineMAFourHour(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//��K��ͼ��Ϣ
void QtChart::slotMarketKLineOneDayChart(QString strHydm)
{
	std::string strValueKLineAll;
	GetRedisValueAll(strHydm, 6, strValueKLineAll);                                 //��redis���ݿ��л�ȡ��db6�������ݵļ���

	QVector<QVector<double>> CandleDatas;                                           //����  �� ���ݵļ���
	QVector<double> timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas;//�����±� �Լ� MA����
	QVector<QString> m_qvecHydm;                                                    //��Լ����
	QSharedPointer<QCPAxisTickerText> textTicker(new MyAxisTickerText);             //����x��������

	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	ProcessingData(strValueKLineAll, 6, CandleDatas, m_qvecHydm, textTicker, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
	ProcessingTextTickX(6, textTicker);                                             //x�����ݸ�ʽ���üӹ�

	//��K��ͼ��Ϣ��������ͼ����
	MaketKLineCandlestickChartOneDay(CandleDatas, m_qvecHydm, textTicker, timeDatas);
	MA5Datas = calculateMA(CandleDatas, 5);
	MA10Datas = calculateMA(CandleDatas, 10);
	MA20Datas = calculateMA(CandleDatas, 20);
	MA40Datas = calculateMA(CandleDatas, 40);
	MA60Datas = calculateMA(CandleDatas, 60);

	//��K��ͼ��Ϣ����MA���߻���
	MaketKLineMAOneDay(timeDatas, MA5Datas, MA10Datas, MA20Datas, MA40Datas, MA60Datas);
}

//�����ť�ۺ���,���������桪������
void QtChart::slotButtonReturn()
{
	emit signalReturn();
}

//1����K��ͼ��ʾ
void QtChart::slotKLineOneMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(0);
}

//5����K��ͼ��ʾ
void QtChart::slotKLintFiveMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(2);
}

//15����K��ͼ��ʾ
void QtChart::slotKLineFifteenMinuteShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(4);
}

//30����K��ͼ��ʾ
void QtChart::slotKLineHalfHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(5);
}

//1СʱK��ͼ��ʾ
void QtChart::slotKLineOneHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(6);
}

//4СʱK��ͼ��ʾ
void QtChart::slotKLineFourHourShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(1);
}

//��K��ͼ��ʾ
void QtChart::slotKLineDayShow()
{
	ui.tabWidget_Chart->setCurrentIndex(1);
	ui.stackedWidget_KLine->setCurrentIndex(7);
}

//���е��źš���ť���Ӻ���
void QtChart::ConnectAll()
{
	//������ͼ������������
	connect(ui.pushButton_return, SIGNAL(clicked()), this, SLOT(slotButtonReturn()));

	/*connect(ui.pushButtonK, SIGNAL(clicked()), this, SLOT(slotMarketKLineChart(QString)));*/
	connect(ui.pushButton_KLineOneMinute, SIGNAL(clicked()), this, SLOT(slotKLineOneMinuteShow()));//��1����K��ͼ��ť��ʾ
	connect(ui.pushButton_KLineFiveMinute, SIGNAL(clicked()), this, SLOT(slotKLintFiveMinuteShow()));//��5����K��ͼ��ť��ʾ
	connect(ui.pushButton_KLineFifteenMinute, SIGNAL(clicked()), this, SLOT(slotKLineFifteenMinuteShow()));//��15����K��ͼ��ť��ʾ
	connect(ui.pushButton_KLineHalfHour, SIGNAL(clicked()), this, SLOT(slotKLineHalfHourShow()));//��30����K��ͼ��ť��ʾ
	connect(ui.pushButton_KLineOneHour, SIGNAL(clicked()), this, SLOT(slotKLineOneHourShow()));//��1СʱK��ͼ��ť��ʾ
	connect(ui.pushButton_KLineFourHour, SIGNAL(clicked()), this, SLOT(slotKLineFourHourShow()));//��4СʱK��ͼ��ť��ʾ
	connect(ui.pushButton_KLineDay, SIGNAL(clicked()), this, SLOT(slotKLineDayShow()));//����K��ͼ��ť��ʾ
}

//��ť����
void QtChart::PushButtonSet()
{
	QString styleButton = "QPushButton{background:white;color:black;\
		border: 13px solid white;border-radius:18px;border-style: outset;}"//�趨�߿��ȼ���ɫ
		"QPushButton:hover{background:violet; color: white;border: 13px solid violet;border-radius:18px;}"  //���ͣ��ʱ����ɫ
		"QPushButton:pressed{background-color:rgb(85, 170, 255);border: 13px solid rgb(85, 170, 255);border-radius:18px;}";//��갴��ʱ����ɫ

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

//ͼ�γ�ʼ����������ʱͼ��K��ͼ
void QtChart::ChartInit()
{
	//��ʼ����ʱͼ
	InitMarketTimeSharingChart();

	//��ʼ��1����k��
	InitKLineOneMinuteChart();	
	//��ʼ��5����k��
	InitKLineFiveMinuteChart();
	//��ʼ��15����k��
	InitKLineFifteenMinuteChart();
	//��ʼ��30����k��
	InitKLineHalfHourChart();
	//��ʼ��1Сʱk��
	InitKLineOneHourChart();
	//��ʼ��4СʱK��
	InitKLineFourHourChart();
	//��ʼ����k��
	InitKLineOneDayChart();
}

//���Ƹ���ʱ��ε�K��ͼ
void QtChart::DrawKLine()
{
	//����1����k��ͼ
	emit slotMarketKLineOneMinuteChart(m_strHydm);
	ui.customPlot_KLineOneMinute->replot();//�ػ�

	//����5����k��ͼ
	emit slotMarketKLineFiveMinuteChart(m_strHydm);
	ui.customPlot_KLineFiveMinute->replot();//�ػ�

	//����15����k��ͼ
	emit slotMarketKLineFifteenMinuteChart(m_strHydm);
	ui.customPlot_KLineFifteenMinute->replot();//�ػ�

	//����30����k��ͼ
	emit slotMarketKLineHalfHourChart(m_strHydm);
	ui.customPlot_KLineHalfHour->replot();//�ػ�

	//����1Сʱk��ͼ
	emit slotMarketKLineOneHourChart(m_strHydm);
	ui.customPlot_KLineOneHour->replot();//�ػ�

	//����4Сʱk��ͼ
	emit slotMarketKLineFourHourChart(m_strHydm);
	ui.customPlot_KLineFourHour->replot();//�ػ�

	//������k��ͼ
	emit slotMarketKLineOneDayChart(m_strHydm);
	ui.customPlot_KLineOneDay->replot();//�ػ�
}

//��ʼ����ʱͼ��
void QtChart::InitMarketTimeSharingChart()
{
	//��ʼ����ʱͼ��coutomPlot�ؼ���ʼ��
	ui.customPlot->setInteraction(QCP::iRangeDrag, true);//�����϶�
	ui.customPlot->setInteraction(QCP::iRangeZoom, true);//������ʾ���Ƶ�ͼ
	//����ʱ��̶ȶ���
	QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
	ui.customPlot->xAxis->setTicker(dateTimeTicker);//ʱ��̶ȶ����customPlot�ؼ�
	dateTimeTicker->setTickCount(12);
	dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
	ui.customPlot->xAxis->setSubTicks(false);
	//ui.customPlot->xAxis->setRange(now, now + 3600 * 0.5);//x�᷶Χ���ӵ�ǰʱ���������ư�Сʱ
	//ui.customPlot->yAxis->setRange(4900, 5200);
	dateTimeTicker->setDateTimeFormat("hh:mm");//����x��̶���ʾ��ʽ
	ui.customPlot->addGraph();//���һ����
}

//��ʼ��1����K��ͼ��
void QtChart::InitKLineOneMinuteChart()
{
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneMinute = new QCPFinancial(ui.customPlot_KLineOneMinute->xAxis, ui.customPlot_KLineOneMinute->yAxis);
	m_pCandlesticksOneMinute->setName(QString::fromLocal8Bit("һ����K"));
	m_pCandlesticksOneMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksOneMinute->setPenPositive(PenPositive);
	m_pCandlesticksOneMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksOneMinute->setPenNegative(PenNegative);

	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneMinute->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineOneMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneMinute->yAxis->setTickLabelColor(Qt::white);


// 	//�������������ı���ɫ
// 	ui.customPlot_KLineOneMinute->xAxis->setLabelColor(Qt::white);
// 	ui.customPlot_KLineOneMinute->yAxis->setLabelColor(Qt::white);

	//������������ɫ
	ui.customPlot_KLineOneMinute->xAxis->setBasePen(QPen(Qt::red));
// 	ui.customPlot_KLineOneMinute->xAxis->setTickPen(QPen(Qt::red));
// 	ui.customPlot_KLineOneMinute->xAxis->setSubTickPen(QPen(Qt::red));
	ui.customPlot_KLineOneMinute->yAxis->setBasePen(QPen(Qt::red));

	ui.customPlot_KLineOneMinute->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineOneMinute->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineOneMinute->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);//ֻ����ˮƽ�϶�,�����������϶�
}

//��ʼ��5����K��ͼ��������ͼ��MA��
void QtChart::InitKLineFiveMinuteChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFiveMinute->setBackground(plotGradient);
// 	//����QCPAxisRect������ɫ,��������������
// 	QLinearGradient axisRectGradient;
// 	axisRectGradient.setStart(0, 0);
// 	axisRectGradient.setFinalStop(0, 350);
// 	axisRectGradient.setColorAt(0, QColor(80, 80, 80));
// 	axisRectGradient.setColorAt(1, QColor(30, 30, 30));
// 	ui.customPlot_KLineFiveMinute->axisRect()->setBackground(axisRectGradient);
	//���������ı���ɫ
	ui.customPlot_KLineFiveMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFiveMinute->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineFiveMinute->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFiveMinute->yAxis->setBasePen(QPen(Qt::red));
    //��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFiveMinute = new QCPFinancial(ui.customPlot_KLineFiveMinute->xAxis, ui.customPlot_KLineFiveMinute->yAxis);
	m_pCandlesticksFiveMinute->setName(QString::fromLocal8Bit(" 5����K"));
	m_pCandlesticksFiveMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksFiveMinute->setPenPositive(PenPositive);
	m_pCandlesticksFiveMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksFiveMinute->setPenNegative(PenNegative);

	ui.customPlot_KLineFiveMinute->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineFiveMinute->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineFiveMinute->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�

	//ͼ�в����ı���
	QCPItemText* textLabel;
	textLabel = new QCPItemText(ui.customPlot_KLineFiveMinute);            //��QCustomplot���½����ֿ�
	textLabel->setPositionAlignment(Qt::AlignAbsolute | Qt::AlignLeft);         //���ֲ��֣����������
	textLabel->position->setType(QCPItemPosition::ptAxisRectRatio);        //λ�����ͣ���ǰ�᷶Χ�ı���Ϊ��λ/ʵ������Ϊ��λ��
	textLabel->position->setCoords(0, 0);                                  //�����ֿ����X�������ߣ�Y������
	textLabel->setText("MA5:");
	textLabel->setColor(QColor("white"));
/*	textLabel->setFont(QFont(font().family(), 16)); //�����С*/
/*	textLabel->setPen(QPen(Qt::white)); //������ɫ*/
/*	textLabel->setPadding(QMargins(2, 2, 2, 2));//���־���߿򼸸�����*/
	
	//ͼ�в���ֱ�߼�ͷ
	QCPItemLine* arrow = new QCPItemLine(ui.customPlot_KLineFiveMinute);
	arrow->start->setParentAnchor(textLabel->bottom);                       //���ø�ֱ�ߵ����Ϊ���ֿ����ê��  
	arrow->end->setCoords(4, 1.6);                                          //����ֱ���յ�Ϊ����ϵ�еĵ�
	arrow->setHead(QCPLineEnding::esSpikeArrow);                            //���ü�ͷ���ͣ������Ρ����Ρ����εȣ�
	arrow->setVisible(true);
	arrow->setPen(QColor("white"));
	arrow->start->setParentAnchor(textLabel->bottom);
		
// 	QCPItemTracer* tracer;
// 	tracer->setPen(QPen(Qt::DashLine));//�α����ͣ�����
// 	tracer->setStyle(QCPItemTracer::tsPlus);//�α���ʽ��ʮ���ǡ�ԲȦ�������

// 	plotKLineOneMinute = new XxwCustomPlot(ui.customPlot_KLineOneMinute);
// 	plotKLineOneMinute->showTracer(true);
}

//��ʼ��15����K��ͼ��
void QtChart::InitKLineFifteenMinuteChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFifteenMinute->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineFifteenMinute->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFifteenMinute->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineFifteenMinute->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFifteenMinute->yAxis->setBasePen(QPen(Qt::red));
	//��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFifteenMinute = new QCPFinancial(ui.customPlot_KLineFifteenMinute->xAxis, ui.customPlot_KLineFifteenMinute->yAxis);
	m_pCandlesticksFifteenMinute->setName(QString::fromLocal8Bit(" 15����K"));
	m_pCandlesticksFifteenMinute->setBrushPositive(BrushPositive);
	m_pCandlesticksFifteenMinute->setPenPositive(PenPositive);
	m_pCandlesticksFifteenMinute->setBrushNegative(BrushNegative);
	m_pCandlesticksFifteenMinute->setPenNegative(PenNegative);
	
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineFifteenMinute->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineFifteenMinute->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�
}

//��ʼ��30����K��ͼ��
void QtChart::InitKLineHalfHourChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineHalfHour->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineHalfHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineHalfHour->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineHalfHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineHalfHour->yAxis->setBasePen(QPen(Qt::red));
	//��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksHalfHour = new QCPFinancial(ui.customPlot_KLineHalfHour->xAxis, ui.customPlot_KLineHalfHour->yAxis);
	m_pCandlesticksHalfHour->setName(QString::fromLocal8Bit(" 30����K"));
	m_pCandlesticksHalfHour->setBrushPositive(BrushPositive);
	m_pCandlesticksHalfHour->setPenPositive(PenPositive);
	m_pCandlesticksHalfHour->setBrushNegative(BrushNegative);
	m_pCandlesticksHalfHour->setPenNegative(PenNegative);

	ui.customPlot_KLineHalfHour->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineHalfHour->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineHalfHour->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�
}

//��ʼ��1СʱK��ͼ��
void QtChart::InitKLineOneHourChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneHour->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineOneHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneHour->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineOneHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineOneHour->yAxis->setBasePen(QPen(Qt::red));
	//��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneHour = new QCPFinancial(ui.customPlot_KLineOneHour->xAxis, ui.customPlot_KLineOneHour->yAxis);
	m_pCandlesticksOneHour->setName(QString::fromLocal8Bit(" 1СʱK"));
	m_pCandlesticksOneHour->setBrushPositive(BrushPositive);
	m_pCandlesticksOneHour->setPenPositive(PenPositive);
	m_pCandlesticksOneHour->setBrushNegative(BrushNegative);
	m_pCandlesticksOneHour->setPenNegative(PenNegative);

	ui.customPlot_KLineOneHour->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineOneHour->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineOneHour->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�
}

//��ʼ��4СʱK��ͼ��
void QtChart::InitKLineFourHourChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineFourHour->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineFourHour->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineFourHour->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineFourHour->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineFourHour->yAxis->setBasePen(QPen(Qt::red));
	//��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksFourHour = new QCPFinancial(ui.customPlot_KLineFourHour->xAxis, ui.customPlot_KLineFourHour->yAxis);
	m_pCandlesticksFourHour->setName(QString::fromLocal8Bit(" 5����K"));
	m_pCandlesticksFourHour->setBrushPositive(BrushPositive);
	m_pCandlesticksFourHour->setPenPositive(PenPositive);
	m_pCandlesticksFourHour->setBrushNegative(BrushNegative);
	m_pCandlesticksFourHour->setPenNegative(PenNegative);

	ui.customPlot_KLineFourHour->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineFourHour->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineFourHour->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�
}

//��ʼ����K��ͼ��
void QtChart::InitKLineOneDayChart()
{
	//���ñ�����ɫ��������������,����Ч��
	QLinearGradient plotGradient;
	plotGradient.setStart(0, 0);
	plotGradient.setFinalStop(0, 350);
	plotGradient.setColorAt(0, QColor(40, 40, 40));
	plotGradient.setColorAt(1, QColor(30, 30, 30));
	ui.customPlot_KLineOneDay->setBackground(plotGradient);
	//���������ı���ɫ
	ui.customPlot_KLineOneDay->xAxis->setTickLabelColor(Qt::white);
	ui.customPlot_KLineOneDay->yAxis->setTickLabelColor(Qt::white);
	//������������ɫ
	ui.customPlot_KLineOneDay->xAxis->setBasePen(QPen(Qt::red));
	ui.customPlot_KLineOneDay->yAxis->setBasePen(QPen(Qt::red));
	//��������ͼ
	const QColor BrushPositive("#ec0000");
	const QColor PenPositive("#8a0000");
	const QColor BrushNegative("#00da3c");
	const QColor PenNegative("#008f28");
	m_pCandlesticksOneDay = new QCPFinancial(ui.customPlot_KLineOneDay->xAxis, ui.customPlot_KLineOneDay->yAxis);
	m_pCandlesticksOneDay->setName(QString::fromLocal8Bit(" ��K"));
	m_pCandlesticksOneDay->setBrushPositive(BrushPositive);
	m_pCandlesticksOneDay->setPenPositive(PenPositive);
	m_pCandlesticksOneDay->setBrushNegative(BrushNegative);
	m_pCandlesticksOneDay->setPenNegative(PenNegative);

	ui.customPlot_KLineOneDay->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLineOneDay->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLineOneDay->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�
}

//dbʱ���K��ͼ��Ϣ������redis���ݿ��л�ȡdb�����ݵļ���
void QtChart::GetRedisValueAll(const QString &strHydm, const int &db,std::string &strValueKLineAll)
{
	PhdRedis redis;	                                                 
	if (!redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com"))//�������ݿ�
		return;	
	if (!redis.SwitchDb(db))                                         //�л����ݿ�db
		return;	
	if (!redis.GetKeyValue(strHydm.toStdString(), strValueKLineAll)) //��redis���ݿ��л�ȡ1�������ݵļ���
		return;
}

//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
void QtChart::ProcessingData(const std::string &strValueKLineAll, const int &db, 
	QVector<QVector<double>> &CandleDatas,QVector<QString> &m_qvecHydm, QSharedPointer<QCPAxisTickerText> &textTicker,
	QVector<double> &MA5Datas, QVector<double> &MA10Datas, QVector<double> &MA20Datas, 
	QVector<double> &MA40Datas, QVector<double> &MA60Datas)
{
	//������ȡ���ݡ����ݷ�װ
	QString qstrValueKLine = QString::fromStdString(strValueKLineAll);
	QStringList strListRedisKLine = qstrValueKLine.split("\n");                //�õ�ÿ1���ӵ����ݵļ���
	QString xTime;                                                             //x��   ʱ����

	for (int i = 0; i < strListRedisKLine.size() - 1; i++)
	{
		QVector<double> rawDatas;                                              //ÿһ�� ʱ���  ���ݵļ���
		QString qstrRedisKLineRow = strListRedisKLine.at(i);
		QStringList qstrListRedisKLineRow = qstrRedisKLineRow.split(",");      //ÿһ�� ʱ���  ���ݵľ�������
		for (int j = 0; j < qstrListRedisKLineRow.size(); j++)
		{
			ProcessingXTime(qstrListRedisKLineRow,db,xTime);                   //x��  ʱ����  ���ݴ���,�õ�xTime
			textTicker->addTick(i, xTime);                                     //����x��ʱ�䣬x�������ḳֵ
			if (db <= 5)                                                       //����������ʱ���K��ʱ�����ݻ���
			{
				if (j == 0)
				{
					m_qvecHydm.append(qstrListRedisKLineRow.at(j));            //��Լ���뼯�ϸ�ֵ
				}
				else if (j >= 4)
				{
					rawDatas.append(qstrListRedisKLineRow.at(j).toDouble());   //�۸񼯺ϡ������̡���ߡ���͡�����
				}
			}
			else if(db == 6)                                                   //������Kʱ�����ݻ���
			{
				if (j == 0)
				{
					m_qvecHydm.append(qstrListRedisKLineRow.at(j));
				}
				else if (j >= 3)
				{
					rawDatas.append(qstrListRedisKLineRow.at(j).toDouble());    //�۸񼯺ϡ������̡���ߡ���͡�����
				}
			}	
		}
		CandleDatas.append(rawDatas);                                           //����ͼ���ݼ��ϡ����۸񼯺ϡ������̡���ߡ���͡�����
	}	
	
}

//x��  ʱ����  ���ݴ���
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
		//qsting.replace ��������λ�ÿ�ʼ��n���ַ��滻Ϊ������ַ���
		QString xTimeData = qstrListRedisKLineRow.at(1);                          //���ڣ�������
		QString xTimeYear = xTimeData.replace(4, 6, "");                          //���ڣ���

		xTimeData = qstrListRedisKLineRow.at(1);                                  //���³�ʼ��
		QString xTimeDataMonth = xTimeData.right(5);                              //���ڣ�����
		xTimeDataMonth = xTimeDataMonth.left(2);                                  //���ڣ���

		xTimeData = qstrListRedisKLineRow.at(1);                                  //���³�ʼ��
		QString xTimeDataDay = xTimeData.right(2);                                //���ڣ���
		QString xTime = qstrListRedisKLineRow.at(1);

		xTime = xTimeDataDay;                                                 //��������£�ʱ������ʾΪ���ڣ�����
		if (xTimeDataMonth == "01" && xTimeDataDay == "01")
		{			
			xTime = xTimeYear + "." + xTimeDataMonth + "." + xTimeDataDay;        //���·�Ϊ1����Ϊ1ʱ��ʱ������ʾΪ���ڣ�����
		}
		else if (xTimeDataDay == "01" && xTimeDataMonth != "01")
		{
			xTime = xTimeDataMonth + "." + xTimeDataDay;
		}
		else
		{			
			xTime = xTimeDataDay;                                                 //��������£�ʱ������ʾΪ���ڣ�����
		}
	}
}

//x�����ݸ�ʽ���üӹ�
void QtChart::ProcessingTextTickX(const int &db, QSharedPointer<QCPAxisTickerText> &textTicker)
{
	textTicker->setTickCount(5);                                                //x����ʾ�̶�����
	if (db == 0)
	{
		ui.customPlot_KLineOneMinute->xAxis->setTicker(textTicker);                 //x���������
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

//1����K��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartOneMinute(const QVector<QVector<double>> &CandleDatas, 
	const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker,
	QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;                                    //k�ߵ����ݼ���
	QVector<double> m_qvecPrice;
	//��������ͼ����
	//����1�������ݵļ���,KLineOneMinute
	for (int i = 0; i < CandleDatas.size(); ++i)
	{
		timeDatas.append(i);                                                    //ʱ���±꣬x��MA
		QCPFinancialData data;
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
/*		QCPFinancialData abc = datas.findBegin(20,512);*/
// 		double dYMax = CandleDatas.at(i).at(1);//���¼ۣ�y������
// 		double dYMin = CandleDatas.at(i).at(2);
// 		ui.customPlot_KLineOneMinute->yAxis->setRange(m_dYMin, m_dYMax);//����y�᷶Χ
		QString strHydmId = m_qvecHydm.at(i);                                     //��Լ����
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit("  1����K");
		m_pCandlesticksOneMinute->setName(strTiltleName);
	}
	m_pCandlesticksOneMinute->data()->set(datas);                                 //�������ͼ����
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineOneMinute->xAxis->range().center());
	ui.customPlot_KLineOneMinute->legend->setVisible(true);
	ui.customPlot_KLineOneMinute->legend->setBrush(QColor(255, 255, 255, 0));      //����ͼ��������ɫ,͸��
	ui.customPlot_KLineOneMinute->legend->setTextColor(QColor("white"));           //����ͼ��������ɫ
	ui.customPlot_KLineOneMinute->rescaleAxes();                                   //���µ���������
	ui.customPlot_KLineOneMinute->replot();                                        //�ػ�
	//�����α��Ƿ�ɼ�
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
	ui.customPlot_KLineOneMinute->xAxis->grid()->setPen(QColor(255, 255, 255, 0));          //����������͸��ɫ
	ui.customPlot_KLineOneMinute->yAxis->grid()->subGridPen();

	QLinearGradient plotG;
	plotG.setStart(0, 0);
	plotG.setFinalStop(0, 350);
	plotG.setColorAt(0, QColor(80, 80, 80));
	plotG.setColorAt(1, QColor(30, 30, 30));	
	ui.customPlot_KLineOneMinute->setBackground(plotG);

	//�����û�������϶��᷶Χ�������������ź͵��ѡ��ͼ��:
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

	//ʹ����͵ײ������ǽ����ǵķ�Χת�Ƶ�����Ͷ�����:
	connect(ui.customPlot_KLineOneMinute->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_KLineOneMinute->xAxis2, SLOT(setRange(QCPRange)));
	connect(ui.customPlot_KLineOneMinute->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_KLineOneMinute->yAxis2, SLOT(setRange(QCPRange)));

	ui.customPlot_KLineOneMinute->legend->setSelectableParts(QCPLegend::spItems);

	//���������Ҳ���Ϸ����ߣ���X/Y��һ���γ�һ������
	ui.customPlot_KLineOneMinute->axisRect()->setupFullAxesBox();
}

//5����K��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartFiveMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;                                    //k�ߵ����ݼ���
	//��������ͼ����
	for (int i = 0; i < CandleDatas.size(); ++i)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//��Լ����
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit("  5����K");
		m_pCandlesticksFiveMinute->setName(strTiltleName);
	}
	m_pCandlesticksFiveMinute->data()->set(datas);//�������ͼ����
	ui.customPlot_KLineFiveMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineFiveMinute->xAxis->range().center());
	ui.customPlot_KLineFiveMinute->yAxis->scaleRange(1.05, ui.customPlot_KLineFiveMinute->yAxis->range().center());
	// show legend with slightly transparent background brush:
	//����΢͸���ı���������ʾͼ��:
	ui.customPlot_KLineFiveMinute->legend->setVisible(true);
	ui.customPlot_KLineFiveMinute->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineFiveMinute->legend->setTextColor(QColor("white"));//����ͼ��������ɫ

// 	// configure bottom axis to show date instead of number:
// 	//���õײ�����ʾ���ڶ���������:
// 	QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
// 	dateTicker->setDateTimeFormat("d. MMMM\nyyyy");
// 	ui.customPlot_KLineFiveMinute->xAxis->setTicker(dateTicker);
// 	// configure left axis text labels:
// 	//���������ı���ǩ:
// 	QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
// 	textTicker->addTick(10, "a bit\nlow");
// 	textTicker->addTick(50, "quite\nhigh");
// 	ui.customPlot_KLineFiveMinute->yAxis->setTicker(textTicker);
// 	// set a more compact font size for bottom and left axis tick labels:
// 	//Ϊ�ײ������������ø����յ������С:
// 	ui.customPlot_KLineFiveMinute->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
// 	ui.customPlot_KLineFiveMinute->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
// 	// set axis labels:
// 	//�������ǩ:
// 	ui.customPlot_KLineFiveMinute->xAxis->setLabel("Date");
// 	ui.customPlot_KLineFiveMinute->yAxis->setLabel("Random wobbly lines value");
// 	// make top and right axes visible but without ticks and labels:
// 	//ʹ�������Ҳ���ɼ����������̶Ⱥͱ�ǩ:
// 	ui.customPlot_KLineFiveMinute->xAxis2->setVisible(true);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setVisible(true);
// 	ui.customPlot_KLineFiveMinute->xAxis2->setTicks(false);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setTicks(false);
// 	ui.customPlot_KLineFiveMinute->xAxis2->setTickLabels(false);
// 	ui.customPlot_KLineFiveMinute->yAxis2->setTickLabels(false);
// 	// set axis ranges to show all data:
// 	//�����᷶Χ��ʾ��������:
// 	ui.customPlot_KLineFiveMinute->xAxis->setRange(now, now + 24 * 3600 * 249);
// 	ui.customPlot_KLineFiveMinute->yAxis->setRange(0, 60);
	ui.customPlot_KLineFiveMinute->legend->setVisible(true);
	ui.customPlot_KLineFiveMinute->legend->setBrush(QColor(255, 255, 255, 0));

	ui.customPlot_KLineFiveMinute->rescaleAxes();//���µ���������
	ui.customPlot_KLineFiveMinute->replot();//�ػ�
}

//15����K��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartFifteenMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k�ߵ����ݼ���
	//��������ͼ����
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;//һ������
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//��Լ����
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 15����K");
		m_pCandlesticksFifteenMinute->setName(strTiltleName);
	}
	m_pCandlesticksFifteenMinute->data()->set(datas);
	ui.customPlot_KLineFifteenMinute->xAxis->scaleRange(1.05, ui.customPlot_KLineFifteenMinute->xAxis->range().center());
	ui.customPlot_KLineFifteenMinute->yAxis->scaleRange(1.05, ui.customPlot_KLineFifteenMinute->yAxis->range().center());
	ui.customPlot_KLineFifteenMinute->legend->setVisible(true);
	ui.customPlot_KLineFifteenMinute->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineFifteenMinute->legend->setTextColor(QColor("white"));//����ͼ��������ɫ
	ui.customPlot_KLineFifteenMinute->rescaleAxes();//���µ���������
	ui.customPlot_KLineFifteenMinute->replot();//�ػ�
}

//30����K��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartHalfHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k�ߵ����ݼ���
    //��������ͼ����
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;//һ������
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//��Լ����
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 30����K");
		m_pCandlesticksHalfHour->setName(strTiltleName);
	}
	m_pCandlesticksHalfHour->data()->set(datas);
	ui.customPlot_KLineHalfHour->xAxis->scaleRange(1.05, ui.customPlot_KLineHalfHour->xAxis->range().center());
	ui.customPlot_KLineHalfHour->yAxis->scaleRange(1.05, ui.customPlot_KLineHalfHour->yAxis->range().center());
	ui.customPlot_KLineHalfHour->legend->setVisible(true);
	ui.customPlot_KLineHalfHour->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineHalfHour->legend->setTextColor(QColor("white"));//����ͼ��������ɫ
	ui.customPlot_KLineHalfHour->rescaleAxes();//���µ���������
	ui.customPlot_KLineHalfHour->replot();//�ػ�
}

//1СʱK��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartOneHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k�ߵ����ݼ���
	//��������ͼ����
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;//һ������
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);//��Լ����
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" 1СʱK");
		m_pCandlesticksOneHour->setName(strTiltleName);
	}
	m_pCandlesticksOneHour->data()->set(datas);
	ui.customPlot_KLineOneHour->xAxis->setTicker(textTicker);
	ui.customPlot_KLineOneHour->xAxis->scaleRange(1.05, ui.customPlot_KLineOneHour->xAxis->range().center());
	ui.customPlot_KLineOneHour->yAxis->scaleRange(1.05, ui.customPlot_KLineOneHour->yAxis->range().center());
	ui.customPlot_KLineOneHour->legend->setVisible(true);
	ui.customPlot_KLineOneHour->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineOneHour->legend->setTextColor(QColor("white"));//����ͼ��������ɫ
	ui.customPlot_KLineOneHour->rescaleAxes();//���µ���������
	ui.customPlot_KLineOneHour->replot();//�ػ�
}

//4СʱK��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartFourHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k�ߵ����ݼ���
	//��������ͼ����
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;//һ������
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
// 		//qsting.replace ��������λ�ÿ�ʼ��n���ַ��滻Ϊ������ַ���
// 		QString xTimeData = qstrListRedisKLineRow.at(1);//���ڣ�������
// 		QString xTimeYear = xTimeData.replace(0, 5, "");
// 		QString xTimeDataMonth = xTimeData.replace(0, 5, "");//���ڣ�����
// 		xTimeDataMonth = xTimeDataMonth.replace(2, 3, "");//���ڣ���
// 		QString xTimeDataDay = xTimeData.replace(0, 8, "");//���ڣ���
// 		QString xTimePoint = qstrListRedisKLineRow.at(2);//����ʱ��㣬ʱ����
// 		xTimePoint = xTimePoint.replace(5, 3, "");//����ʱ��㣬ʱ��
// 		QString xTime;
// 		if (xTimePoint == "00:00")
// 		{
// 			//��ʱ��Ϊ���ʱ��ʱ������ʾΪ���ڣ�����
// 			xTime = xTimeDataDay;
// 		}
// 		else if (xTimeDataMonth == "01")
// 		{
// 			//���·�Ϊ1ʱ��ʱ������ʾΪ���ڣ���
// 			xTime = xTimeYear;
// 		}
// 		else
// 		{
// 			//��������£�ʱ������ʾΪʱ��㣬ʱ��
// 			xTime = xTimePoint;
// 		}
// 		textTicker->addTick(i, xTime);//����x��ʱ��
		QString strHydmId = m_qvecHydm.at(i);
		QString strTiltleName = strHydmId + QString::fromLocal8Bit("  4СʱK");
		m_pCandlesticksFourHour->setName(strTiltleName);
	}
	m_pCandlesticksFourHour->data()->set(datas);
	ui.customPlot_KLineFourHour->xAxis->scaleRange(1.05, ui.customPlot_KLineFourHour->xAxis->range().center());
	ui.customPlot_KLineFourHour->yAxis->scaleRange(1.05, ui.customPlot_KLineFourHour->yAxis->range().center());
	ui.customPlot_KLineFourHour->legend->setVisible(true);
	ui.customPlot_KLineFourHour->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineFourHour->legend->setTextColor(QColor("white"));//����ͼ��������ɫ
	ui.customPlot_KLineFourHour->rescaleAxes();//���µ���������
	ui.customPlot_KLineFourHour->replot();//�ػ�
}

//��K��ͼ��Ϣ��������ͼ����
void QtChart::MaketKLineCandlestickChartOneDay(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm, const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas)
{
	QCPDataContainer<QCPFinancialData> datas;//k�ߵ����ݼ���
	//��������ͼ����
	for (int i = 0; i < CandleDatas.size(); i++)
	{
		timeDatas.append(i);//ʱ���±꣬x��
		QCPFinancialData data;//һ������
		data.key = i;
		data.open = CandleDatas.at(i).at(0);
		data.high = CandleDatas.at(i).at(1);
		data.low = CandleDatas.at(i).at(2);
		data.close = CandleDatas.at(i).at(3);
		datas.add(data);
		QString strHydmId = m_qvecHydm.at(i);
		QString strTiltleName = strHydmId + "  " + QString::fromLocal8Bit(" ��K");
		m_pCandlesticksOneDay->setName(strTiltleName);
	}
	m_pCandlesticksOneDay->data()->set(datas);
	ui.customPlot_KLineOneDay->xAxis->scaleRange(1.05, ui.customPlot_KLineOneDay->xAxis->range().center());
	ui.customPlot_KLineOneDay->yAxis->scaleRange(1.05, ui.customPlot_KLineOneDay->yAxis->range().center());
	ui.customPlot_KLineOneDay->legend->setVisible(true);
	ui.customPlot_KLineOneDay->legend->setBrush(QColor(255, 255, 255, 0));//����ͼ��������ɫ,͸��
	ui.customPlot_KLineOneDay->legend->setTextColor(QColor("white"));//����ͼ��������ɫ
	ui.customPlot_KLineOneDay->rescaleAxes();//���µ���������
	ui.customPlot_KLineOneDay->replot();//�ػ�
}

//1����K��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAOneMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineOneMinute->clearGraphs();

	//���5��MD����
	QCPGraph *m_pGraphOneMinute = ui.customPlot_KLineOneMinute->addGraph();
	m_pGraphOneMinute->setName("MA5");
	m_pGraphOneMinute->setData(timeDatas, MA5Datas);
	m_pGraphOneMinute->setPen(ColorOptions.at(0));
	m_pGraphOneMinute->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	m_pGraphOneMinute->setSmooth(true);//����ƽ������

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

//5����K��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAFiveMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineFiveMinute->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineFiveMinute->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//15����K��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAFifteenMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineFifteenMinute->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineFifteenMinute->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//30����K��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAHalfHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineHalfHour->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineHalfHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//1СʱK��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAOneHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineOneHour->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineOneHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//4СʱK��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAFourHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineFourHour->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineFourHour->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//��K��ͼ��Ϣ����MA���߻���
void QtChart::MaketKLineMAOneDay(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas)
{
	//ÿ����ʾ����֮ǰ���������һ�λ��Ƶ�����
	ui.customPlot_KLineOneDay->clearGraphs();

	// 	���5��MA����
	QCPGraph *graph = ui.customPlot_KLineOneDay->addGraph();
	graph->setName("MA5");
	graph->setData(timeDatas, MA5Datas);
	graph->setPen(ColorOptions.at(0));
	graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(ColorOptions.at(0), 2), QBrush(Qt::white), 8));
	graph->setSmooth(true);//����ƽ������

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

//����MA�ƶ�ƽ����
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