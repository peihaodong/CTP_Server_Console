#include "QtCTP.h"
#include "ThostFtdcMdApi.h"
#include "QtCustomMdSpi.h"
#include <Vector>
#include <QTableWidgetItem>
#include "qcustomplot.h"
#include <QtNetwork>
#include "PhdOperExcel.h"
#include <QAction>
#include <QMessageBox>
#include "../../PhdRedis.h"

//������ʷ��Ϣȫ�ֱ���
QNetworkAccessManager * Manager;

QtCTP::QtCTP(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	//��ʼ��������
	m_pUi_Chart = new QtChart;

	m_pMd = new QtCustomMdSpi(this);

	//��ʷ������Ϣʵ����
	Manager = new QNetworkAccessManager(this);
	//���е��ź����Ӻ���
	ConnectAll();

	//1.������ʼ��
	QtMarketTableWidgetStockInit();//1.1���顪����ָ�ڻ�����ʼ��
	QtMarketTableWidgetPreciousMetalInit();//1.2���顪�����������ʼ��
	
	//2.ί�б��ʼ��
	QtEntrustTableWidget();	

	//3.�ɽ���(Bargain)��ʼ��
	QtBargainTableWidget();	

	//4.1.�ֱֲ��ʼ��
	QtPositionTableWidget();
	//4.2.�ʽ���ʼ��
	QtFundTableWidget();
	

	//////////////////************************************************************************
	ui.lineEdit_MarketAddress->setText("tcp://180.168.146.187:10010");
	ui.lineEdit_TradeAddress->setText("tcp://180.168.146.187:10000");
	ui.lineEdit_BrokerID->setText("9999");
	ui.lineEdit_Acount->setText("175068");
	ui.lineEdit_Password->setText("Donny123");

	ui.radioButton_MarketPrice->setCheckable(true);
	ui.radioButton_FlatWarehouseToday->setCheckable(true);
}

QtCTP::~QtCTP()
{
}

//���е��ź�������Ӻ���
void QtCTP::ConnectAll()
{
	//�󶨵�¼��ť����Ӧ����
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotButtonLogin()));
	//���������Ϣ��ʾ����Ӧ������CTP���͵��������ݣ�
	connect(m_pMd, SIGNAL(signalSendData(QString)), this, SLOT(slotReceiveMarket(QString)));
	//�󶨵�½��ť�ͷ�ʱͼ�������淢��������Ϣ�ź�������ͼ�����
	//��CTP���͵��������ݷ��͸�����ͼ�����ķ�ʱͼ�ۺ���
	connect(m_pMd, SIGNAL(signalSendMdToFst(QString)), m_pUi_Chart, SLOT(slotMarketTimeSharingChart(QString)));
	//�󶨵�½��ť��K��ͼ--��CTP���͵��������ݷ��͸�����ͼ������K��ͼ�ۺ���
	//connect(m_pMd, SIGNAL(signalSendMdToK(QString)), m_pUi_Chart, SLOT(slotMarketKLineChart(QString)));

	//��˫�������ָ�ڻ���Ϣ���ʱͼ��k��ͼ��
	connect(ui.tableWidget_Market_StockFutures, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotDoubleClicked(int, int)));
	//��˫������������Ϣ���ʱͼ��k��ͼ��
	connect(ui.tableWidget_Market_PreciousMetal, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotDoubleClickedPreciousMetal(int, int)));
	//�����淵��������
	connect(m_pUi_Chart, SIGNAL(signalReturn()), this, SLOT(slotReturnMain()));
	//��ť�ؼ��ź����
	connect(ui.actionMain, SIGNAL(triggered()), this, SLOT(slotShowPageMain()));
	connect(ui.actionTimeSharing, SIGNAL(triggered()), this, SLOT(slotShowPageChartTimeSharing()));


/*	connect(ui.tableWidget_Market_StockFutures, SIGNAL(cellActivated(int, int)), this, SLOT(slotCellClickedStock(int,int)));*/
// 	//��CTP���͵��������ݷ��͸�����ͼ�����ķ�ʱͼ�ۺ���
 	connect(m_pMd, SIGNAL(signalSendMdToFst(QString)), this, SLOT(slotReceiveTimeSharing(QString)));

	connect(ui.actionK, SIGNAL(triggered()), this, SLOT(slotShowPageChartK()));

	connect(Manager, SIGNAL(GetHistoralMarketData(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
}

//--------------1.������ʼ��--------------///
//1.1���顪����ָ�ڻ����ʼ��
void QtCTP::QtMarketTableWidgetStockInit()
{
	//1.1���顪����ָ�ڻ���
	ui.tableWidget_Market_StockFutures->setColumnCount(11);//��������Ϊ11
 	ui.tableWidget_Market_StockFutures->setRowCount(39);//��������Ϊ39

	QStringList headerMarket;
	headerMarket.append(QString::fromLocal8Bit("��Լ����"));
	headerMarket.append(QString::fromLocal8Bit("����ʱ��"));
	headerMarket.append(QString::fromLocal8Bit("���¼�"));
	headerMarket.append(QString::fromLocal8Bit("��һ��"));
	headerMarket.append(QString::fromLocal8Bit("��һ��"));
	headerMarket.append(QString::fromLocal8Bit("��һ��"));
	headerMarket.append(QString::fromLocal8Bit("��һ��"));
	headerMarket.append(QString::fromLocal8Bit("��  ��"));
	headerMarket.append(QString::fromLocal8Bit("�ɽ���"));
	headerMarket.append(QString::fromLocal8Bit("��ͣ��"));
	headerMarket.append(QString::fromLocal8Bit("��ͣ��"));

	ui.tableWidget_Market_StockFutures->setHorizontalHeaderLabels(headerMarket);//����������ʹ�ñ�ǩ����ˮƽ�����ǩ��
	QString strFilePath = QCoreApplication::applicationDirPath();
	if (!SetFirstColumnStock(strFilePath))//���ù�ָ�ڻ�����һ�����ݡ�����Լ����
		return;
	for (int i = 0;i <= 39;i++)
	{
		for (int j = 1;j <= 10;j++)
		{
			ui.tableWidget_Market_StockFutures->setItem(i, j, new QTableWidgetItem("----"));
		}
	}	
	ui.tableWidget_Market_StockFutures->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_Market_StockFutures->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_Market_StockFutures->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}

//���ù�ָ�ڻ�����һ�����ݡ�����Լ����
bool QtCTP::SetFirstColumnStock(const QString& strFilePath)
{
// 	//��ȡtxt�ļ�����
// 	QFile file(strFilePath);
// 	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
// 		return false;
	ui.tableWidget_Market_StockFutures->setItem(0, 0, new QTableWidgetItem(QString::fromLocal8Bit("IF��Ȩ")));
	ui.tableWidget_Market_StockFutures->setItem(1, 0, new QTableWidgetItem("IF2012"));
	ui.tableWidget_Market_StockFutures->setItem(2, 0, new QTableWidgetItem("IF2101"));
	ui.tableWidget_Market_StockFutures->setItem(3, 0, new QTableWidgetItem("IF2102"));
	ui.tableWidget_Market_StockFutures->setItem(4, 0, new QTableWidgetItem("IF2103"));
	ui.tableWidget_Market_StockFutures->setItem(5, 0, new QTableWidgetItem("IF2104"));
	ui.tableWidget_Market_StockFutures->setItem(6, 0, new QTableWidgetItem("IF2105"));
	ui.tableWidget_Market_StockFutures->setItem(7, 0, new QTableWidgetItem("IF2106"));
	ui.tableWidget_Market_StockFutures->setItem(8, 0, new QTableWidgetItem("IF2107"));
	ui.tableWidget_Market_StockFutures->setItem(9, 0, new QTableWidgetItem("IF2108"));
	ui.tableWidget_Market_StockFutures->setItem(10, 0, new QTableWidgetItem("IF2109"));
	ui.tableWidget_Market_StockFutures->setItem(11, 0, new QTableWidgetItem("IF2110"));
	ui.tableWidget_Market_StockFutures->setItem(12, 0, new QTableWidgetItem("IF2111"));
	ui.tableWidget_Market_StockFutures->setItem(13, 0, new QTableWidgetItem(QString::fromLocal8Bit("IH��Ȩ")));
	ui.tableWidget_Market_StockFutures->setItem(14, 0, new QTableWidgetItem("IH2012"));
	ui.tableWidget_Market_StockFutures->setItem(15, 0, new QTableWidgetItem("IH2101"));
	ui.tableWidget_Market_StockFutures->setItem(16, 0, new QTableWidgetItem("IH2102"));
	ui.tableWidget_Market_StockFutures->setItem(17, 0, new QTableWidgetItem("IH2103"));
	ui.tableWidget_Market_StockFutures->setItem(18, 0, new QTableWidgetItem("IH2104"));
	ui.tableWidget_Market_StockFutures->setItem(19, 0, new QTableWidgetItem("IH2105"));
	ui.tableWidget_Market_StockFutures->setItem(20, 0, new QTableWidgetItem("IH2106"));
	ui.tableWidget_Market_StockFutures->setItem(21, 0, new QTableWidgetItem("IH2107"));
	ui.tableWidget_Market_StockFutures->setItem(22, 0, new QTableWidgetItem("IH2108"));
	ui.tableWidget_Market_StockFutures->setItem(23, 0, new QTableWidgetItem("IH2109"));
	ui.tableWidget_Market_StockFutures->setItem(24, 0, new QTableWidgetItem("IH2110"));
	ui.tableWidget_Market_StockFutures->setItem(25, 0, new QTableWidgetItem("IH2111"));
	ui.tableWidget_Market_StockFutures->setItem(26, 0, new QTableWidgetItem(QString::fromLocal8Bit("IC��Ȩ")));
	ui.tableWidget_Market_StockFutures->setItem(27, 0, new QTableWidgetItem("IC2012"));
	ui.tableWidget_Market_StockFutures->setItem(28, 0, new QTableWidgetItem("IC2101"));
	ui.tableWidget_Market_StockFutures->setItem(29, 0, new QTableWidgetItem("IC2102"));
	ui.tableWidget_Market_StockFutures->setItem(30, 0, new QTableWidgetItem("IC2103"));
	ui.tableWidget_Market_StockFutures->setItem(31, 0, new QTableWidgetItem("IC2104"));
	ui.tableWidget_Market_StockFutures->setItem(32, 0, new QTableWidgetItem("IC2105"));
	ui.tableWidget_Market_StockFutures->setItem(33, 0, new QTableWidgetItem("IC2106"));
	ui.tableWidget_Market_StockFutures->setItem(34, 0, new QTableWidgetItem("IC2107"));
	ui.tableWidget_Market_StockFutures->setItem(35, 0, new QTableWidgetItem("IC2108"));
	ui.tableWidget_Market_StockFutures->setItem(36, 0, new QTableWidgetItem("IC2109"));
	ui.tableWidget_Market_StockFutures->setItem(37, 0, new QTableWidgetItem("IC2110"));
	ui.tableWidget_Market_StockFutures->setItem(38, 0, new QTableWidgetItem("IC2111"));
	return true;
}

void QtCTP::Selected()
{
	//���ص�ǰ��ѡ��Ŀ���б��˻ص���Ʒû���ر��˳��
	QList<QTableWidgetItem*> items = ui.tableWidget_Market_StockFutures->selectedItems();
	int count = items.count();
	for (int i = 0; i < count; i++)
	{
		int row = ui.tableWidget_Market_StockFutures->row(items.at(i));
		QTableWidgetItem *item = items.at(i);
		QString text = item->text(); //��ȡ����
	}
}

//1.2���顪�����������ʼ��
void QtCTP::QtMarketTableWidgetPreciousMetalInit()//1.2���顪�����������ʼ��
{
	ui.tableWidget_Market_PreciousMetal->setColumnCount(11);//��������Ϊ11
	ui.tableWidget_Market_PreciousMetal->setRowCount(33);//��������Ϊ33

	QStringList headerMarketPreciousMetal;
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��Լ����"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("����ʱ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("���¼�"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��һ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��һ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��һ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��һ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��  ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("�ɽ���"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��ͣ��"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("��ͣ��"));

	ui.tableWidget_Market_PreciousMetal->setHorizontalHeaderLabels(headerMarketPreciousMetal);//��������	
	SetFirstColumnMetal();////���ù��������һ�����ݡ�����Լ����
	for (int i = 0; i <= 39; i++)
	{
		for (int j = 1; j <= 10; j++)
		{
			ui.tableWidget_Market_PreciousMetal->setItem(i, j, new QTableWidgetItem("----"));
		}
	}
	ui.tableWidget_Market_PreciousMetal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_Market_PreciousMetal->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_Market_PreciousMetal->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}

//���ù��������һ�����ݡ�����Լ����
void QtCTP::SetFirstColumnMetal()
{
	ui.tableWidget_Market_PreciousMetal->setItem(0, 0, new QTableWidgetItem(QString::fromLocal8Bit("�����")));
	ui.tableWidget_Market_PreciousMetal->setItem(1, 0, new QTableWidgetItem(QString::fromLocal8Bit("����ָ��")));
	ui.tableWidget_Market_PreciousMetal->setItem(2, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2012")));
	ui.tableWidget_Market_PreciousMetal->setItem(3, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2101")));
	ui.tableWidget_Market_PreciousMetal->setItem(4, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2102")));
	ui.tableWidget_Market_PreciousMetal->setItem(5, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2103")));
	ui.tableWidget_Market_PreciousMetal->setItem(6, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2104")));
	ui.tableWidget_Market_PreciousMetal->setItem(7, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2105")));
	ui.tableWidget_Market_PreciousMetal->setItem(8, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2106")));
	ui.tableWidget_Market_PreciousMetal->setItem(9, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2107")));
	ui.tableWidget_Market_PreciousMetal->setItem(10, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2108")));
	ui.tableWidget_Market_PreciousMetal->setItem(11, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2109")));
	ui.tableWidget_Market_PreciousMetal->setItem(12, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2110")));
	ui.tableWidget_Market_PreciousMetal->setItem(13, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2111")));
	ui.tableWidget_Market_PreciousMetal->setItem(14, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2112")));
	ui.tableWidget_Market_PreciousMetal->setItem(15, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2202")));
	ui.tableWidget_Market_PreciousMetal->setItem(16, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2204")));
	ui.tableWidget_Market_PreciousMetal->setItem(17, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2206")));
	ui.tableWidget_Market_PreciousMetal->setItem(18, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2208")));
	ui.tableWidget_Market_PreciousMetal->setItem(19, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2210")));
	ui.tableWidget_Market_PreciousMetal->setItem(20, 0, new QTableWidgetItem(QString::fromLocal8Bit("����ָ��")));
	ui.tableWidget_Market_PreciousMetal->setItem(21, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2012")));
	ui.tableWidget_Market_PreciousMetal->setItem(22, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2101")));
	ui.tableWidget_Market_PreciousMetal->setItem(23, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2102")));
	ui.tableWidget_Market_PreciousMetal->setItem(24, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2103")));
	ui.tableWidget_Market_PreciousMetal->setItem(25, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2104")));
	ui.tableWidget_Market_PreciousMetal->setItem(26, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2105")));
	ui.tableWidget_Market_PreciousMetal->setItem(27, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2106")));
	ui.tableWidget_Market_PreciousMetal->setItem(28, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2107")));
	ui.tableWidget_Market_PreciousMetal->setItem(29, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2108")));
	ui.tableWidget_Market_PreciousMetal->setItem(30, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2109")));
	ui.tableWidget_Market_PreciousMetal->setItem(31, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2110")));
	ui.tableWidget_Market_PreciousMetal->setItem(32, 0, new QTableWidgetItem(QString::fromLocal8Bit("����2111")));
}

//---------------2.ί�б��ʼ��----------------///
void QtCTP::QtEntrustTableWidget()
{
	ui.tableWidget_Entrust->setColumnCount(9);//��������Ϊ9
	ui.tableWidget_Entrust->setRowCount(10);//��������Ϊ10

	QStringList headerEntrust;
	headerEntrust.append(QString::fromLocal8Bit("�ɽ�ʱ��"));
	headerEntrust.append(QString::fromLocal8Bit("��Լ����"));
	headerEntrust.append(QString::fromLocal8Bit("����"));
	headerEntrust.append(QString::fromLocal8Bit("��ƽ"));
	headerEntrust.append(QString::fromLocal8Bit("����"));
	headerEntrust.append(QString::fromLocal8Bit("�۸�"));
	headerEntrust.append(QString::fromLocal8Bit("״̬"));
	headerEntrust.append(QString::fromLocal8Bit("ί�к�"));
	headerEntrust.append(QString::fromLocal8Bit("������"));

	ui.tableWidget_Entrust->setHorizontalHeaderLabels(headerEntrust);//��������

	QStringList VerticalEntrust;
	VerticalEntrust.append(QString::fromLocal8Bit("IF��Ȩ"));
	ui.tableWidget_Entrust->setVerticalHeaderLabels(VerticalEntrust);
	ui.tableWidget_Entrust->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_Entrust->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_Entrust->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}

//---------------3.�ɽ����ʼ��----------------///
void QtCTP::QtBargainTableWidget()
{
	ui.tableWidget_Bargain->setColumnCount(8);
	ui.tableWidget_Bargain->setRowCount(10);

	QStringList headerBargain;
	headerBargain.append(QString::fromLocal8Bit("�ɽ�ʱ��"));
	headerBargain.append(QString::fromLocal8Bit("��Լ����"));
	headerBargain.append(QString::fromLocal8Bit("����"));
	headerBargain.append(QString::fromLocal8Bit("��ƽ"));
	headerBargain.append(QString::fromLocal8Bit("����"));
	headerBargain.append(QString::fromLocal8Bit("�۸�"));
	headerBargain.append(QString::fromLocal8Bit("ί�к�"));
	headerBargain.append(QString::fromLocal8Bit("������"));

	ui.tableWidget_Bargain->setHorizontalHeaderLabels(headerBargain);//��������
	ui.tableWidget_Bargain->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_Bargain->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_Bargain->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}

//---------------4.1�ֱֲ��ʼ��----------------///
void QtCTP::QtPositionTableWidget()
{
	ui.tableWidget_Position->setColumnCount(4);//��������Ϊ4
	ui.tableWidget_Position->setRowCount(6);//��������Ϊ6

	QStringList headerPosition;
	headerPosition.append(QString::fromLocal8Bit("��Լ����"));
	headerPosition.append(QString::fromLocal8Bit("�ֲ�����"));
	headerPosition.append(QString::fromLocal8Bit("�ֲ�����"));
	headerPosition.append(QString::fromLocal8Bit("�ֲֳɱ�"));

	ui.tableWidget_Position->setHorizontalHeaderLabels(headerPosition);//��������
	ui.tableWidget_Position->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_Position->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_Position->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}

//4.2�ʽ���ʼ��
void QtCTP::QtFundTableWidget()
{
	ui.tableWidget_fund->setColumnCount(5);//��������Ϊ5
	ui.tableWidget_fund->setRowCount(6);//��������Ϊ6

	QStringList headerFund;
	headerFund.append(QString::fromLocal8Bit("�˺�"));
	headerFund.append(QString::fromLocal8Bit("��Ȩ��"));
	headerFund.append(QString::fromLocal8Bit("ռ�ñ�֤��"));
	headerFund.append(QString::fromLocal8Bit("�����ʽ�"));
	headerFund.append(QString::fromLocal8Bit("���ն�"));

	ui.tableWidget_fund->setHorizontalHeaderLabels(headerFund);//��������
	ui.tableWidget_fund->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//������ֱ��ͷ
	ui.tableWidget_fund->setSelectionBehavior(QAbstractItemView::SelectRows);//����ѡ�еķ�ʽ
	ui.tableWidget_fund->setEditTriggers(QTableWidget::NoEditTriggers);//������Ϊ��ֹ�༭
}


//�ж��Ƿ��й�ָ�ڻ���Ϣ
bool QtCTP::ExitStockFutures(QString str)
{
/*	std::vector<QString> vecType;*/

	vecTypeStock.push_back(QString::fromLocal8Bit("IF��Ȩ"));
	vecTypeStock.push_back("IF2011");
	vecTypeStock.push_back("IF2012");
	vecTypeStock.push_back("IF2101");
	vecTypeStock.push_back("IF2102");
	vecTypeStock.push_back("IF2103");
	vecTypeStock.push_back("IF2104");
	vecTypeStock.push_back("IF2105");
	vecTypeStock.push_back("IF2106");
	vecTypeStock.push_back("IF2107");
	vecTypeStock.push_back("IF2108");
	vecTypeStock.push_back("IF2109");
	vecTypeStock.push_back("IF2110");
	vecTypeStock.push_back(QString::fromLocal8Bit("IH��Ȩ"));
	vecTypeStock.push_back("IH2011");
	vecTypeStock.push_back("IH2012");
	vecTypeStock.push_back("IH2101");
	vecTypeStock.push_back("IH2102");
	vecTypeStock.push_back("IH2103");
	vecTypeStock.push_back("IH2104");
	vecTypeStock.push_back("IH2105");
	vecTypeStock.push_back("IH2106");
	vecTypeStock.push_back("IH2107");
	vecTypeStock.push_back("IH2108");
	vecTypeStock.push_back("IH2109");
	vecTypeStock.push_back("IH2110");
	vecTypeStock.push_back(QString::fromLocal8Bit("IC��Ȩ"));
	vecTypeStock.push_back("IC2011");
	vecTypeStock.push_back("IC2012");
	vecTypeStock.push_back("IC2101");
	vecTypeStock.push_back("IC2102");
	vecTypeStock.push_back("IC2103");
	vecTypeStock.push_back("IC2104");
	vecTypeStock.push_back("IC2105");
	vecTypeStock.push_back("IC2106");
	vecTypeStock.push_back("IC2107");
	vecTypeStock.push_back("IC2108");
	vecTypeStock.push_back("IC2109");
	vecTypeStock.push_back("IC2110");

	auto iter = std::find(vecTypeStock.begin(), vecTypeStock.end(), str);
	if (iter != vecTypeStock.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//�ж��Ƿ��й������Ϣ
bool QtCTP::ExitPreciousMetal(QString str)
{
	std::vector<QString> vecType;

	vecType.push_back(QString::fromLocal8Bit("�����"));
	vecType.push_back(QString::fromLocal8Bit("����ָ��"));
	vecType.push_back("au2011");
	vecType.push_back("au2012");
	vecType.push_back("au2101");
	vecType.push_back("au2102");
	vecType.push_back("au2103");
	vecType.push_back("au2104");
	vecType.push_back("au2105");
	vecType.push_back("au2106");
	vecType.push_back("au2107");
	vecType.push_back("au2108");
	vecType.push_back("au2109");
	vecType.push_back("au2110");
	vecType.push_back("au2112");
	vecType.push_back("au2202");
	vecType.push_back("au2204");
	vecType.push_back("au2206");
	vecType.push_back("au2208");
	vecType.push_back("au2210");
	vecType.push_back(QString::fromLocal8Bit("����ָ��"));
	vecType.push_back("ag2011");
	vecType.push_back("ag2012");
	vecType.push_back("ag2101");
	vecType.push_back("ag2102");
	vecType.push_back("ag2103");
	vecType.push_back("ag2104");
	vecType.push_back("ag2105");
	vecType.push_back("ag2106");
	vecType.push_back("ag2107");
	vecType.push_back("ag2108");
	vecType.push_back("ag2109");
	vecType.push_back("ag2110");

	auto iter = std::find(vecType.begin(), vecType.end(), str);
	if (iter != vecType.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//�����¼��ť
void QtCTP::slotButtonLogin()
{
// 	PhdOperExcel excel;
// 	QString str = "D:\\CTP��Ŀ����\\CTP\CTP1����Tick����\\������Լ\\������Դ\\20�Ž�����1����.csv";
// 	excel.OpenExcel(str);

	//QStringתchar
	//char* ch1;
	//QByteArray ba1 = ui.lineEdit->text().toLatin1();
	//ch1 = bal.data();
	ui.stackedWidget_Main->setCurrentIndex(0);
// 	ui.page_Chart->hide();
// 	ui.page_Main->show();

	std::strcpy(m_pMd->m_hq.gMdFrontAddr, "tcp://180.168.146.187:10111");
	std::strcpy(m_pMd->m_hq.gBrokerID, "9999");
	std::strcpy(m_pMd->m_hq.gInvesterID, "175068");
	std::strcpy(m_pMd->m_hq.gInvesterPassword, "Donny123");

	m_pMd->Init();
}

//��ʾ������Ϣ
void QtCTP::slotReceiveMarket(QString MarketTick)
{
	QStringList strlist = MarketTick.split(",");
	slotReceiveMarketStockFutures(strlist);
	slotReceiveMarketPreciousMetal(strlist);
}

//��ָ�ڻ���Ϣ
void QtCTP::slotReceiveMarketStockFutures(QStringList strlist)//��ָ�ڻ�
{
	QString str = strlist.at(0);
	//�ж��Ƿ��й�ָ�ڻ���Ϣ
	if (!ExitStockFutures(str))
		return;

	for (int i = 0; i < ui.tableWidget_Market_StockFutures->rowCount(); i++)
	{		
		//�жϵ�i�е�1�е������Ƿ����һ����ֵ��ַ������
		//�жϿؼ�����л�ȡ��������ӿ��л�ȡ�ĺ�Լ�����Ƿ���ͬ����ͬ����¿ؼ����������
		QString a = ui.tableWidget_Market_StockFutures->item(i, 0)->text();
		QString b = strlist.at(0);
		if (ui.tableWidget_Market_StockFutures->item(i, 0)->text() == strlist.at(0))
		{
			ui.tableWidget_Market_StockFutures->setItem(i, 0, new QTableWidgetItem(strlist.at(0)));
			ui.tableWidget_Market_StockFutures->setItem(i, 1, new QTableWidgetItem(strlist.at(1)));
			ui.tableWidget_Market_StockFutures->setItem(i, 2, new QTableWidgetItem(strlist.at(2)));
			ui.tableWidget_Market_StockFutures->setItem(i, 3, new QTableWidgetItem(strlist.at(3)));
			ui.tableWidget_Market_StockFutures->setItem(i, 4, new QTableWidgetItem(strlist.at(4)));
			ui.tableWidget_Market_StockFutures->setItem(i, 5, new QTableWidgetItem(strlist.at(5)));
			ui.tableWidget_Market_StockFutures->setItem(i, 6, new QTableWidgetItem(strlist.at(6)));
			ui.tableWidget_Market_StockFutures->setItem(i, 7, new QTableWidgetItem(strlist.at(7)));
			ui.tableWidget_Market_StockFutures->setItem(i, 8, new QTableWidgetItem(strlist.at(8)));
			ui.tableWidget_Market_StockFutures->setItem(i, 9, new QTableWidgetItem(strlist.at(9)));
			ui.tableWidget_Market_StockFutures->setItem(i, 10, new QTableWidgetItem(strlist.at(10)));
			return;
		}
	}
	//��Լ���벻ͬ�����²���һ��
	int row = ui.tableWidget_Market_StockFutures->rowCount();                               //��ȡ����������
	ui.tableWidget_Market_StockFutures->insertRow(row);                                     //�����ȡ������
	ui.tableWidget_Market_StockFutures->setItem(row, 0, new QTableWidgetItem(strlist.at(0)));
	ui.tableWidget_Market_StockFutures->setItem(row, 1, new QTableWidgetItem(strlist.at(1)));
	ui.tableWidget_Market_StockFutures->setItem(row, 2, new QTableWidgetItem(strlist.at(2)));
	ui.tableWidget_Market_StockFutures->setItem(row, 3, new QTableWidgetItem(strlist.at(3)));
	ui.tableWidget_Market_StockFutures->setItem(row, 4, new QTableWidgetItem(strlist.at(4)));
	ui.tableWidget_Market_StockFutures->setItem(row, 5, new QTableWidgetItem(strlist.at(5)));
	ui.tableWidget_Market_StockFutures->setItem(row, 6, new QTableWidgetItem(strlist.at(6)));
	ui.tableWidget_Market_StockFutures->setItem(row, 7, new QTableWidgetItem(strlist.at(7)));
	ui.tableWidget_Market_StockFutures->setItem(row, 8, new QTableWidgetItem(strlist.at(8)));
	ui.tableWidget_Market_StockFutures->setItem(row, 9, new QTableWidgetItem(strlist.at(9)));
	ui.tableWidget_Market_StockFutures->setItem(row, 10, new QTableWidgetItem(strlist.at(10)));
}

//�������Ϣ
void QtCTP::slotReceiveMarketPreciousMetal(QStringList strlist)//�������Ϣ
{
	QString str = strlist.at(0);
	//�ж��Ƿ��й������Ϣ
	if (!ExitPreciousMetal(str))
		return;

	for (int i = 0; i < ui.tableWidget_Market_PreciousMetal->rowCount(); i++)
	{
		QString InstrumentId = strlist.at(0);
		InstrumentId.replace("au", QString::fromLocal8Bit("����"));
		InstrumentId.replace("ag", QString::fromLocal8Bit("����"));
		//�жϵ�i�е�1�е������Ƿ����һ����ֵ��ַ������
		if (ui.tableWidget_Market_PreciousMetal->item(i, 0)->text() == InstrumentId)
		{
			ui.tableWidget_Market_PreciousMetal->setItem(i, 0, new QTableWidgetItem(InstrumentId));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 1, new QTableWidgetItem(strlist.at(1)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 2, new QTableWidgetItem(strlist.at(2)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 3, new QTableWidgetItem(strlist.at(3)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 4, new QTableWidgetItem(strlist.at(4)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 5, new QTableWidgetItem(strlist.at(5)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 6, new QTableWidgetItem(strlist.at(6)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 7, new QTableWidgetItem(strlist.at(7)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 8, new QTableWidgetItem(strlist.at(8)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 9, new QTableWidgetItem(strlist.at(9)));
			ui.tableWidget_Market_PreciousMetal->setItem(i, 10, new QTableWidgetItem(strlist.at(10)));
			return;	
		}
	}

}

//����ͼ������ת������������
void QtCTP::slotReturnMain()
{
	this->show();
	m_pUi_Chart->close();
}

void QtCTP::slotShowPageChartTimeSharing()
{
	QList<QTableWidgetItem*> itemsMarketStock = ui.tableWidget_Market_StockFutures->selectedItems();
	QList<QTableWidgetItem*> itemsMarketPreciouMetal = ui.tableWidget_Market_PreciousMetal->selectedItems();
	if (!itemsMarketStock.empty() || !itemsMarketPreciouMetal.empty())
	{
		qDebug() << "ѡ����ĳ��";
		int row = ui.tableWidget_Market_StockFutures->currentRow();
		if (row < 0)
		{
			row = ui.tableWidget_Market_PreciousMetal->currentRow();
			//�õ��û�ѡ��ĺ�Լ����
			QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
			QString strHydm = pItem->text();
		}
		else
		{
			//�õ��û�ѡ��ĺ�Լ����
			QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
			QString strHydm = pItem->text();
		}
		ui.stackedWidget_Main->setCurrentIndex(1);
		ui.tabWidget_Chart->setCurrentIndex(0);
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ѡ��һ���ٽ��в���"));
	}
}

void QtCTP::slotShowPageMain()
{
	ui.stackedWidget_Main->setCurrentIndex(0);
}

void QtCTP::slotShowPageChartK()
{
	QList<QTableWidgetItem*> itemsMarketStock = ui.tableWidget_Market_StockFutures->selectedItems();
	QList<QTableWidgetItem*> itemsMarketPreciouMetal = ui.tableWidget_Market_PreciousMetal->selectedItems();
	if (!itemsMarketStock.empty() || !itemsMarketPreciouMetal.empty())
	{
		qDebug() << "ѡ����ĳ��";
		int row = ui.tableWidget_Market_StockFutures->currentRow();
		if (row < 0)
		{
			row = ui.tableWidget_Market_PreciousMetal->currentRow();
			//�õ��û�ѡ��ĺ�Լ����
			QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
			QString strHydm = pItem->text();
		}
		else
		{
			//�õ��û�ѡ��ĺ�Լ����
			QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
			QString strHydm = pItem->text();
		}
		slotReceiveKLineChart();//k��ͼ��Ϣ
		ui.stackedWidget_Main->setCurrentIndex(1);
		ui.tabWidget_Chart->setCurrentIndex(1);
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("����"), QString::fromLocal8Bit("����ѡ��һ���ٽ��в���"));
	}
}

void QtCTP::slotCellClickedStock(int row, int column)
{
	//�õ��û�ѡ��ĺ�Լ����
	QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
	QString strHydm = pItem->text();

	m_pUi_Chart->SetHydm(strHydm);//����Ҫ���Ƶĺ�Լ����
	m_pUi_Chart->show();
	this->close();
}

void QtCTP::slotReceiveTimeSharing(QString dataTimeSharing)
{
/*	ui.setupUi(this);*/
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
	dateTimeTicker->setDateTimeFormat("hh:mm:ss");//����x��̶���ʾ��ʽ
	ui.customPlot->addGraph();//���һ����

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

void QtCTP::slotReceiveKLineChart()
{
	//��ʼ��k��
	m_pCandlesticks = new QCPFinancial(ui.customPlot_KLine->xAxis, ui.customPlot_KLine->yAxis);
	//�������Ϊtrue����QCustomPlot���һ������(����һ��ͼ��)Ҳ���Զ����ͼ���ɱ��(QCustomPlot::ͼ��)��
	ui.customPlot_KLine->setAutoAddPlottableToLegend(m_pCandlesticks);
	m_pCandlesticks->setChartStyle(QCPFinancial::csCandlestick);

	m_pCandlesticks->setTwoColored(true);//������ɫ
	m_pCandlesticks->setBrushPositive(QColor(255, 0, 0));//��ɫ
	m_pCandlesticks->setBrushNegative(QColor(0, 100, 0));//��ɫ    �������ɫ
	m_pCandlesticks->setPenPositive(QColor(0, 0, 0));
	m_pCandlesticks->setPenNegative(QColor(0, 0, 0));
	ui.customPlot_KLine->xAxis->scaleRange(1, Qt::AlignRight);//���ƶ�
	ui.customPlot_KLine->setInteraction(QCP::iRangeDrag, true);//���϶�
	ui.customPlot_KLine->axisRect()->setRangeDrag(Qt::Horizontal);//ֻ����ˮƽ�϶�,�����������϶�

	//2.�������
	m_pCandlesticks->addData(1, 4800, 5000,
		4500, 4900);
	m_pCandlesticks->addData(2, 4800, 5200,
		4800, 5100);
	m_pCandlesticks->addData(3, 5000, 5500,
		4900, 5400);
	m_pCandlesticks->addData(4, 4900, 5300,
		4800, 5000);
	m_pCandlesticks->addData(5, 5200, 5300,
		5000, 5300);

	m_pCandlesticks->setWidth(5 * 0.005);//���ÿ��
	ui.customPlot_KLine->rescaleAxes();//���µ���������
	ui.customPlot_KLine->replot();//�ػ�
}

//˫�������ָ�ڻ����ۺ���
void QtCTP::slotDoubleClicked(int row, int column)
{
	//�õ��û�ѡ��ĺ�Լ����
	QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
	QString strHydm = pItem->text();

	m_pUi_Chart->SetHydm(strHydm);//����Ҫ���Ƶĺ�Լ����
	m_pUi_Chart->show();
	this->close();
}

void QtCTP::slotDoubleClickedPreciousMetal(int row, int column)
{
	//�õ��û�ѡ��ĺ�Լ����
	QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
	QString strHydm = pItem->text();
	strHydm.replace(QString::fromLocal8Bit("����"),"au");
	strHydm.replace(QString::fromLocal8Bit("����"),"ag");

	m_pUi_Chart->SetHydm(strHydm);//����Ҫ���Ƶĺ�Լ����
	m_pUi_Chart->show();
	this->close();
}

