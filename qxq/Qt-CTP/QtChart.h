#pragma once

#include <QWidget>
#include "ui_QtChart.h"
#include "qcustomplot.h"
#include "QtCustomMdSpi.h"
#include <QtNetwork>
#include "XxwCustomPlot.h"
/*#include <math.h>*/

class QtChart : public QWidget
{
	Q_OBJECT

public:
	QtChart(QWidget *parent = Q_NULLPTR);
	~QtChart();

	//设置合约代码名
	inline void SetHydm(const QString& strHydm) {
		m_strHydm = strHydm;
	}

	//信号
signals:
	void signalReturn();

private:
	Ui::QtChart ui;

	QVector<double> m_vecXData;
	QVector<double> m_vecYData;

	double m_dYMax = 0;//y轴的最大值
	double m_dYMin = 0;//y轴最小值

	bool m_bShow;		//界面是否显示的标记
	QString m_strHydm;	//合约代码

	//k线实例
	QCPFinancial* m_pCandlesticksOneMinute;     //1分钟蜡烛图实例
	QCPFinancial* m_pCandlesticksFiveMinute;    //5分钟蜡烛图实例
	QCPFinancial* m_pCandlesticksFifteenMinute; //15分钟蜡烛图实例
	QCPFinancial* m_pCandlesticksHalfHour;      //30分钟蜡烛图实例
	QCPFinancial* m_pCandlesticksOneHour;       //1小时蜡烛图实例
	QCPFinancial* m_pCandlesticksFourHour;      //4小时蜡烛图实例
	QCPFinancial* m_pCandlesticksOneDay;        //1日蜡烛图实例

	const QVector<QColor> ColorOptions = { "white", "yellow", "purple", "green","blue" }; //MA线颜色,白、黄、紫、绿、蓝

	XxwCustomPlot* plotKLineOneMinute;           //光标实例

protected:

	virtual void showEvent(QShowEvent *event) override;	      //重写显示界面事件
	virtual void hideEvent(QHideEvent *event) override;	      //重写隐藏界面事件

private slots:
	void slotMarketTimeSharingChart(QString dataTimeSharing);  //分时图信息

	void slotMarketKLineOneMinuteChart(QString strHydm);    //1分钟K线图信息——蜡烛图、MA均线
	void slotMarketKLineFiveMinuteChart(QString strHydm);   //5分钟K线图信息——蜡烛图、MA均线
	void slotMarketKLineFifteenMinuteChart(QString strHydm);//15分钟K线图信息——蜡烛图、MA均线
	void slotMarketKLineHalfHourChart(QString strHydm);     //30分钟K线图信息——蜡烛图、MA均线
	void slotMarketKLineOneHourChart(QString strHydm);      //1小时K线图信息——蜡烛图、MA均线
	void slotMarketKLineFourHourChart(QString strHydm);     //4小时K线图信息——蜡烛图、MA均线
	void slotMarketKLineOneDayChart(QString strHydm);       //日K线图信息——蜡烛图、MA均线

	void slotButtonReturn();	//点击按钮槽函数,返回主界面——报价

	void slotKLineOneMinuteShow();        //1分钟K线图显示
	void slotKLintFiveMinuteShow();       //5分钟K线图显示
	void slotKLineFifteenMinuteShow();    //15分钟K线图显示
	void slotKLineHalfHourShow();         //30分钟K线图显示
	void slotKLineOneHourShow();          //1小时K线图显示
	void slotKLineFourHourShow();         //4小时K线图显示
	void slotKLineDayShow();              //日K线图显示

private:	
	void ConnectAll();                    //所有的信号、按钮连接函数
	void PushButtonSet();                 //按钮设置
	void ChartInit();                     //图形初始化，包括分时图、K线图
	void DrawKLine();                     //绘制各个时间段的K线图

	void InitMarketTimeSharingChart();    //初始化分时图表

	void InitKLineOneMinuteChart();       //初始化1分钟K线图表
	void InitKLineFiveMinuteChart();      //初始化5分钟K线图表
	void InitKLineFifteenMinuteChart();   //初始化15分钟K线图表
	void InitKLineHalfHourChart();        //初始化30分钟K线图表
	void InitKLineOneHourChart();         //初始化1小时K线图表
	void InitKLineFourHourChart();        //初始化4小时K线图表
	void InitKLineOneDayChart();          //初始化日K线图表
	
	void GetRedisValueAll(const QString &strHydm, const int &db, std::string &strValueKLineAll);//db时间段K线图信息——从redis数据库中获取db库数据的集合
	

	//***··············K线图信息——数据处理·············***//
	//处理从redis数据库获得的数据，加工成蜡烛图和MA需要的数据
	void ProcessingData(const std::string &strValueKLineAll, const int &db, 
		QVector<QVector<double>> &CandleDatas, QVector<QString> &m_qvecHydm,QSharedPointer<QCPAxisTickerText> &textTicker,
		QVector<double> &MA5Datas, QVector<double> &MA10Datas, QVector<double> &MA20Datas,QVector<double> &MA40Datas, QVector<double> &MA60Datas);

	//x轴  时间轴  数据处理
	void ProcessingXTime(const QStringList &qstrListRedisKLineRow, const int &db, QString &xTime);                  

	//x轴数据格式设置加工
	void ProcessingTextTickX(const int &db, QSharedPointer<QCPAxisTickerText> &textTicker);


	//***··············K线图信息——蜡烛图绘制·············***//
	//1分钟K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartOneMinute(const QVector<QVector<double>> &CandleDatas,const QVector<QString> &m_qvecHydm, 
		const QSharedPointer<QCPAxisTickerText> textTicker,	QVector<double> &timeDatas);

	//5分钟K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartFiveMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//15分钟K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartFifteenMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//30分钟K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartHalfHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//1小时K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartOneHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//4小时K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartFourHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//日K线图信息——蜡烛图绘制
	void MaketKLineCandlestickChartOneDay(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//***··············K线图信息——MA均线绘制·············***//
	//1分钟K线图信息——MA均线绘制
	void MaketKLineMAOneMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas,const QVector<double> &MA10Datas,
		const QVector<double> &MA20Datas, const QVector<double> &MA40Datas,const QVector<double> &MA60Datas);       

	//5分钟K线图信息——MA均线绘制
	void MaketKLineMAFiveMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//15分钟K线图信息——MA均线绘制
	void MaketKLineMAFifteenMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//30分钟K线图信息——MA均线绘制
	void MaketKLineMAHalfHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//1小时K线图信息——MA均线绘制
	void MaketKLineMAOneHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//4小时K线图信息——MA均线绘制
	void MaketKLineMAFourHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//日K线图信息——MA均线绘制
	void MaketKLineMAOneDay(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);


	QVector<double> calculateMA(const QVector<QVector<double> > &v, int dayCount);//计算MA移动平均线
};

class MyAxisTickerText : public QCPAxisTickerText
{
protected:
	virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE
	{
		Q_UNUSED(tickStep)
			QVector<double> result;
		if (mTicks.isEmpty())
			return result;

		auto start = mTicks.lowerBound(range.lower);
		auto end = mTicks.upperBound(range.upper);
		if (start != mTicks.constBegin()) --start;
		if (end != mTicks.constEnd()) ++end;

		int count = cleanMantissa(std::distance(start, end) / double(mTickCount + 1e-10));

		auto it = start;
		while (it != end) {
			result.append(it.key());
			int step = count;
			if (step == 0) ++it;
			while (--step >= 0 && it != end)
				++it;
		}

		return result;
	}
};

