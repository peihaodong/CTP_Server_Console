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

	//���ú�Լ������
	inline void SetHydm(const QString& strHydm) {
		m_strHydm = strHydm;
	}

	//�ź�
signals:
	void signalReturn();

private:
	Ui::QtChart ui;

	QVector<double> m_vecXData;
	QVector<double> m_vecYData;

	double m_dYMax = 0;//y������ֵ
	double m_dYMin = 0;//y����Сֵ

	bool m_bShow;		//�����Ƿ���ʾ�ı��
	QString m_strHydm;	//��Լ����

	//k��ʵ��
	QCPFinancial* m_pCandlesticksOneMinute;     //1��������ͼʵ��
	QCPFinancial* m_pCandlesticksFiveMinute;    //5��������ͼʵ��
	QCPFinancial* m_pCandlesticksFifteenMinute; //15��������ͼʵ��
	QCPFinancial* m_pCandlesticksHalfHour;      //30��������ͼʵ��
	QCPFinancial* m_pCandlesticksOneHour;       //1Сʱ����ͼʵ��
	QCPFinancial* m_pCandlesticksFourHour;      //4Сʱ����ͼʵ��
	QCPFinancial* m_pCandlesticksOneDay;        //1������ͼʵ��

	const QVector<QColor> ColorOptions = { "white", "yellow", "purple", "green","blue" }; //MA����ɫ,�ס��ơ��ϡ��̡���

	XxwCustomPlot* plotKLineOneMinute;           //���ʵ��

protected:

	virtual void showEvent(QShowEvent *event) override;	      //��д��ʾ�����¼�
	virtual void hideEvent(QHideEvent *event) override;	      //��д���ؽ����¼�

private slots:
	void slotMarketTimeSharingChart(QString dataTimeSharing);  //��ʱͼ��Ϣ

	void slotMarketKLineOneMinuteChart(QString strHydm);    //1����K��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineFiveMinuteChart(QString strHydm);   //5����K��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineFifteenMinuteChart(QString strHydm);//15����K��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineHalfHourChart(QString strHydm);     //30����K��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineOneHourChart(QString strHydm);      //1СʱK��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineFourHourChart(QString strHydm);     //4СʱK��ͼ��Ϣ��������ͼ��MA����
	void slotMarketKLineOneDayChart(QString strHydm);       //��K��ͼ��Ϣ��������ͼ��MA����

	void slotButtonReturn();	//�����ť�ۺ���,���������桪������

	void slotKLineOneMinuteShow();        //1����K��ͼ��ʾ
	void slotKLintFiveMinuteShow();       //5����K��ͼ��ʾ
	void slotKLineFifteenMinuteShow();    //15����K��ͼ��ʾ
	void slotKLineHalfHourShow();         //30����K��ͼ��ʾ
	void slotKLineOneHourShow();          //1СʱK��ͼ��ʾ
	void slotKLineFourHourShow();         //4СʱK��ͼ��ʾ
	void slotKLineDayShow();              //��K��ͼ��ʾ

private:	
	void ConnectAll();                    //���е��źš���ť���Ӻ���
	void PushButtonSet();                 //��ť����
	void ChartInit();                     //ͼ�γ�ʼ����������ʱͼ��K��ͼ
	void DrawKLine();                     //���Ƹ���ʱ��ε�K��ͼ

	void InitMarketTimeSharingChart();    //��ʼ����ʱͼ��

	void InitKLineOneMinuteChart();       //��ʼ��1����K��ͼ��
	void InitKLineFiveMinuteChart();      //��ʼ��5����K��ͼ��
	void InitKLineFifteenMinuteChart();   //��ʼ��15����K��ͼ��
	void InitKLineHalfHourChart();        //��ʼ��30����K��ͼ��
	void InitKLineOneHourChart();         //��ʼ��1СʱK��ͼ��
	void InitKLineFourHourChart();        //��ʼ��4СʱK��ͼ��
	void InitKLineOneDayChart();          //��ʼ����K��ͼ��
	
	void GetRedisValueAll(const QString &strHydm, const int &db, std::string &strValueKLineAll);//dbʱ���K��ͼ��Ϣ������redis���ݿ��л�ȡdb�����ݵļ���
	

	//***����������������������������K��ͼ��Ϣ�������ݴ���������������������������***//
	//�����redis���ݿ��õ����ݣ��ӹ�������ͼ��MA��Ҫ������
	void ProcessingData(const std::string &strValueKLineAll, const int &db, 
		QVector<QVector<double>> &CandleDatas, QVector<QString> &m_qvecHydm,QSharedPointer<QCPAxisTickerText> &textTicker,
		QVector<double> &MA5Datas, QVector<double> &MA10Datas, QVector<double> &MA20Datas,QVector<double> &MA40Datas, QVector<double> &MA60Datas);

	//x��  ʱ����  ���ݴ���
	void ProcessingXTime(const QStringList &qstrListRedisKLineRow, const int &db, QString &xTime);                  

	//x�����ݸ�ʽ���üӹ�
	void ProcessingTextTickX(const int &db, QSharedPointer<QCPAxisTickerText> &textTicker);


	//***����������������������������K��ͼ��Ϣ��������ͼ���ơ�������������������������***//
	//1����K��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartOneMinute(const QVector<QVector<double>> &CandleDatas,const QVector<QString> &m_qvecHydm, 
		const QSharedPointer<QCPAxisTickerText> textTicker,	QVector<double> &timeDatas);

	//5����K��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartFiveMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//15����K��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartFifteenMinute(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//30����K��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartHalfHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//1СʱK��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartOneHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//4СʱK��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartFourHour(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//��K��ͼ��Ϣ��������ͼ����
	void MaketKLineCandlestickChartOneDay(const QVector<QVector<double>> &CandleDatas, const QVector<QString> &m_qvecHydm,
		const QSharedPointer<QCPAxisTickerText> textTicker, QVector<double> &timeDatas);

	//***����������������������������K��ͼ��Ϣ����MA���߻��ơ�������������������������***//
	//1����K��ͼ��Ϣ����MA���߻���
	void MaketKLineMAOneMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas,const QVector<double> &MA10Datas,
		const QVector<double> &MA20Datas, const QVector<double> &MA40Datas,const QVector<double> &MA60Datas);       

	//5����K��ͼ��Ϣ����MA���߻���
	void MaketKLineMAFiveMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//15����K��ͼ��Ϣ����MA���߻���
	void MaketKLineMAFifteenMinute(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//30����K��ͼ��Ϣ����MA���߻���
	void MaketKLineMAHalfHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//1СʱK��ͼ��Ϣ����MA���߻���
	void MaketKLineMAOneHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//4СʱK��ͼ��Ϣ����MA���߻���
	void MaketKLineMAFourHour(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);

	//��K��ͼ��Ϣ����MA���߻���
	void MaketKLineMAOneDay(const QVector<double> &timeDatas, const QVector<double> &MA5Datas, const QVector<double> &MA10Datas,const QVector<double> &MA20Datas, const QVector<double> &MA40Datas, const QVector<double> &MA60Datas);


	QVector<double> calculateMA(const QVector<QVector<double> > &v, int dayCount);//����MA�ƶ�ƽ����
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

