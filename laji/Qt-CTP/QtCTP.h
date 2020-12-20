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
	//���е��ź�������Ӻ���
	void ConnectAll();

	//1.������ʼ��
	void QtMarketTableWidgetStockInit();//���顪����ָ�ڻ�����ʼ��
	bool SetFirstColumnStock(const QString& strFilePath);//���ù�ָ�ڻ�����һ�����ݡ�����Լ����
	void Selected();//������Ϣ��ѡ��
	void QtMarketTableWidgetPreciousMetalInit();//���顪�����������ʼ��
	void SetFirstColumnMetal();//���ù��������һ�����ݡ�����Լ����

	//2.ί�б��ʼ��
	void QtEntrustTableWidget();//ί�б��ʼ��

    //3.�ɽ���(Bargain)��ʼ��
	void QtBargainTableWidget();//�ɽ���(Bargain)��ʼ��

	//4.1.�ֱֲ��ʼ��
	void QtPositionTableWidget();//�ֱֲ��ʼ��
	void QtFundTableWidget();//�ʽ���ʼ��


public:
    Ui::QtCTPClass ui;
private:
	QtCustomMdSpi* m_pMd;//�����¼�ʵ��
	QtChart* m_pUi_Chart;//����ͼ�����


	QVector<double> m_vecXData;
	QVector<double> m_vecYData;

	double m_dYMax = 0;//y������ֵ
	double m_dYMin = 0;//y����Сֵ

	bool m_bShow;		//�����Ƿ���ʾ�ı��
	QString m_strHydm;	//��Լ����

	//k��ʵ��
	QCPFinancial* m_pCandlesticks;

	std::vector<QString> vecTypeStock;//��ָ�ڻ���Լ���뼯��

	bool ExitStockFutures(QString str);//�ж��Ƿ��й�ָ�ڻ���Ϣ
	bool ExitPreciousMetal(QString str);//�ж��Ƿ��й������Ϣ



	//�ź�
signals:
	void signalSendMdToChart(QString);//�����淢��������Ϣ�ź�������ͼ�����
	
	//�ۺ���
private slots:
	void slotButtonLogin();	                     //�����¼��ť
    //����������Ϣ
	void slotReceiveMarket(QString MarketTick);//��ʾ������Ϣ
	void slotReceiveMarketStockFutures(QStringList strlist);//��ָ�ڻ���Ϣ
	void slotReceiveMarketPreciousMetal(QStringList strlist);//�������Ϣ
	
	void slotDoubleClicked(int row, int column);//˫�������ָ�ڻ����ۺ���
	void slotDoubleClickedPreciousMetal(int row,int column);//˫�������������ۺ���
	void slotReturnMain();//����ͼ������ת�����������棬���������ļ��ж���
	
	void slotShowPageChartTimeSharing();//��ʱͼͼ�������ʾ�ۺ���
	void slotShowPageMain();//���۽�����ʾ�ۺ���
	void slotShowPageChartK();//K��ͼͼ�������ʾ����

	void slotCellClickedStock(int row, int column);

	void slotReceiveTimeSharing(QString dataTimeSharing);
	void slotReceiveKLineChart();//k��ͼ��Ϣ	
};


