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

//行情历史信息全局变量
QNetworkAccessManager * Manager;

QtCTP::QtCTP(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	//初始化副界面
	m_pUi_Chart = new QtChart;

	m_pMd = new QtCustomMdSpi(this);

	//历史行情信息实例化
	Manager = new QNetworkAccessManager(this);
	//所有的信号连接函数
	ConnectAll();

	//1.行情表初始化
	QtMarketTableWidgetStockInit();//1.1行情——股指期货表格初始化
	QtMarketTableWidgetPreciousMetalInit();//1.2行情——贵金属表格初始化
	
	//2.委托表初始化
	QtEntrustTableWidget();	

	//3.成交表(Bargain)初始化
	QtBargainTableWidget();	

	//4.1.持仓表初始化
	QtPositionTableWidget();
	//4.2.资金表初始化
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

//所有的信号与槽连接函数
void QtCTP::ConnectAll()
{
	//绑定登录按钮的响应函数
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotButtonLogin()));
	//绑定行情表信息显示的响应函数（CTP发送的行情数据）
	connect(m_pMd, SIGNAL(signalSendData(QString)), this, SLOT(slotReceiveMarket(QString)));
	//绑定登陆按钮和分时图，主界面发送行情信息信号至数据图像界面
	//把CTP发送的行情数据发送给数据图像界面的分时图槽函数
	connect(m_pMd, SIGNAL(signalSendMdToFst(QString)), m_pUi_Chart, SLOT(slotMarketTimeSharingChart(QString)));
	//绑定登陆按钮和K线图--把CTP发送的行情数据发送给数据图像界面的K线图槽函数
	//connect(m_pMd, SIGNAL(signalSendMdToK(QString)), m_pUi_Chart, SLOT(slotMarketKLineChart(QString)));

	//绑定双击行情股指期货信息与分时图（k线图）
	connect(ui.tableWidget_Market_StockFutures, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotDoubleClicked(int, int)));
	//绑定双击行情贵金属信息与分时图（k线图）
	connect(ui.tableWidget_Market_PreciousMetal, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(slotDoubleClickedPreciousMetal(int, int)));
	//副界面返回主界面
	connect(m_pUi_Chart, SIGNAL(signalReturn()), this, SLOT(slotReturnMain()));
	//按钮控件信号与槽
	connect(ui.actionMain, SIGNAL(triggered()), this, SLOT(slotShowPageMain()));
	connect(ui.actionTimeSharing, SIGNAL(triggered()), this, SLOT(slotShowPageChartTimeSharing()));


/*	connect(ui.tableWidget_Market_StockFutures, SIGNAL(cellActivated(int, int)), this, SLOT(slotCellClickedStock(int,int)));*/
// 	//把CTP发送的行情数据发送给数据图像界面的分时图槽函数
 	connect(m_pMd, SIGNAL(signalSendMdToFst(QString)), this, SLOT(slotReceiveTimeSharing(QString)));

	connect(ui.actionK, SIGNAL(triggered()), this, SLOT(slotShowPageChartK()));

	connect(Manager, SIGNAL(GetHistoralMarketData(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
}

//--------------1.行情表初始化--------------///
//1.1行情——股指期货表初始化
void QtCTP::QtMarketTableWidgetStockInit()
{
	//1.1行情——股指期货表
	ui.tableWidget_Market_StockFutures->setColumnCount(11);//设置列数为11
 	ui.tableWidget_Market_StockFutures->setRowCount(39);//设置行数为39

	QStringList headerMarket;
	headerMarket.append(QString::fromLocal8Bit("合约代码"));
	headerMarket.append(QString::fromLocal8Bit("更新时间"));
	headerMarket.append(QString::fromLocal8Bit("最新价"));
	headerMarket.append(QString::fromLocal8Bit("买一价"));
	headerMarket.append(QString::fromLocal8Bit("买一量"));
	headerMarket.append(QString::fromLocal8Bit("卖一价"));
	headerMarket.append(QString::fromLocal8Bit("卖一量"));
	headerMarket.append(QString::fromLocal8Bit("涨  幅"));
	headerMarket.append(QString::fromLocal8Bit("成交量"));
	headerMarket.append(QString::fromLocal8Bit("涨停价"));
	headerMarket.append(QString::fromLocal8Bit("跌停价"));

	ui.tableWidget_Market_StockFutures->setHorizontalHeaderLabels(headerMarket);//设置列名（使用标签设置水平标题标签）
	QString strFilePath = QCoreApplication::applicationDirPath();
	if (!SetFirstColumnStock(strFilePath))//设置股指期货表格第一列数据——合约代码
		return;
	for (int i = 0;i <= 39;i++)
	{
		for (int j = 1;j <= 10;j++)
		{
			ui.tableWidget_Market_StockFutures->setItem(i, j, new QTableWidgetItem("----"));
		}
	}	
	ui.tableWidget_Market_StockFutures->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_Market_StockFutures->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_Market_StockFutures->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}

//设置股指期货表格第一列数据——合约代码
bool QtCTP::SetFirstColumnStock(const QString& strFilePath)
{
// 	//读取txt文件数据
// 	QFile file(strFilePath);
// 	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
// 		return false;
	ui.tableWidget_Market_StockFutures->setItem(0, 0, new QTableWidgetItem(QString::fromLocal8Bit("IF加权")));
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
	ui.tableWidget_Market_StockFutures->setItem(13, 0, new QTableWidgetItem(QString::fromLocal8Bit("IH加权")));
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
	ui.tableWidget_Market_StockFutures->setItem(26, 0, new QTableWidgetItem(QString::fromLocal8Bit("IC加权")));
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
	//返回当前所选项目的列表。退回的物品没有特别的顺序。
	QList<QTableWidgetItem*> items = ui.tableWidget_Market_StockFutures->selectedItems();
	int count = items.count();
	for (int i = 0; i < count; i++)
	{
		int row = ui.tableWidget_Market_StockFutures->row(items.at(i));
		QTableWidgetItem *item = items.at(i);
		QString text = item->text(); //获取内容
	}
}

//1.2行情——贵金属表格初始化
void QtCTP::QtMarketTableWidgetPreciousMetalInit()//1.2行情——贵金属表格初始化
{
	ui.tableWidget_Market_PreciousMetal->setColumnCount(11);//设置列数为11
	ui.tableWidget_Market_PreciousMetal->setRowCount(33);//设置行数为33

	QStringList headerMarketPreciousMetal;
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("合约代码"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("更新时间"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("最新价"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("买一价"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("买一量"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("卖一价"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("卖一量"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("涨  幅"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("成交量"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("涨停价"));
	headerMarketPreciousMetal.append(QString::fromLocal8Bit("跌停价"));

	ui.tableWidget_Market_PreciousMetal->setHorizontalHeaderLabels(headerMarketPreciousMetal);//设置列名	
	SetFirstColumnMetal();////设置贵金属表格第一列数据——合约代码
	for (int i = 0; i <= 39; i++)
	{
		for (int j = 1; j <= 10; j++)
		{
			ui.tableWidget_Market_PreciousMetal->setItem(i, j, new QTableWidgetItem("----"));
		}
	}
	ui.tableWidget_Market_PreciousMetal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_Market_PreciousMetal->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_Market_PreciousMetal->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}

//设置贵金属表格第一列数据——合约代码
void QtCTP::SetFirstColumnMetal()
{
	ui.tableWidget_Market_PreciousMetal->setItem(0, 0, new QTableWidgetItem(QString::fromLocal8Bit("贵金属")));
	ui.tableWidget_Market_PreciousMetal->setItem(1, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金指数")));
	ui.tableWidget_Market_PreciousMetal->setItem(2, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2012")));
	ui.tableWidget_Market_PreciousMetal->setItem(3, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2101")));
	ui.tableWidget_Market_PreciousMetal->setItem(4, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2102")));
	ui.tableWidget_Market_PreciousMetal->setItem(5, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2103")));
	ui.tableWidget_Market_PreciousMetal->setItem(6, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2104")));
	ui.tableWidget_Market_PreciousMetal->setItem(7, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2105")));
	ui.tableWidget_Market_PreciousMetal->setItem(8, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2106")));
	ui.tableWidget_Market_PreciousMetal->setItem(9, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2107")));
	ui.tableWidget_Market_PreciousMetal->setItem(10, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2108")));
	ui.tableWidget_Market_PreciousMetal->setItem(11, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2109")));
	ui.tableWidget_Market_PreciousMetal->setItem(12, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2110")));
	ui.tableWidget_Market_PreciousMetal->setItem(13, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2111")));
	ui.tableWidget_Market_PreciousMetal->setItem(14, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2112")));
	ui.tableWidget_Market_PreciousMetal->setItem(15, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2202")));
	ui.tableWidget_Market_PreciousMetal->setItem(16, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2204")));
	ui.tableWidget_Market_PreciousMetal->setItem(17, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2206")));
	ui.tableWidget_Market_PreciousMetal->setItem(18, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2208")));
	ui.tableWidget_Market_PreciousMetal->setItem(19, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪金2210")));
	ui.tableWidget_Market_PreciousMetal->setItem(20, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银指数")));
	ui.tableWidget_Market_PreciousMetal->setItem(21, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2012")));
	ui.tableWidget_Market_PreciousMetal->setItem(22, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2101")));
	ui.tableWidget_Market_PreciousMetal->setItem(23, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2102")));
	ui.tableWidget_Market_PreciousMetal->setItem(24, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2103")));
	ui.tableWidget_Market_PreciousMetal->setItem(25, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2104")));
	ui.tableWidget_Market_PreciousMetal->setItem(26, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2105")));
	ui.tableWidget_Market_PreciousMetal->setItem(27, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2106")));
	ui.tableWidget_Market_PreciousMetal->setItem(28, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2107")));
	ui.tableWidget_Market_PreciousMetal->setItem(29, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2108")));
	ui.tableWidget_Market_PreciousMetal->setItem(30, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2109")));
	ui.tableWidget_Market_PreciousMetal->setItem(31, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2110")));
	ui.tableWidget_Market_PreciousMetal->setItem(32, 0, new QTableWidgetItem(QString::fromLocal8Bit("沪银2111")));
}

//---------------2.委托表初始化----------------///
void QtCTP::QtEntrustTableWidget()
{
	ui.tableWidget_Entrust->setColumnCount(9);//设置列数为9
	ui.tableWidget_Entrust->setRowCount(10);//设置行数为10

	QStringList headerEntrust;
	headerEntrust.append(QString::fromLocal8Bit("成交时间"));
	headerEntrust.append(QString::fromLocal8Bit("合约代码"));
	headerEntrust.append(QString::fromLocal8Bit("买卖"));
	headerEntrust.append(QString::fromLocal8Bit("开平"));
	headerEntrust.append(QString::fromLocal8Bit("数量"));
	headerEntrust.append(QString::fromLocal8Bit("价格"));
	headerEntrust.append(QString::fromLocal8Bit("状态"));
	headerEntrust.append(QString::fromLocal8Bit("委托号"));
	headerEntrust.append(QString::fromLocal8Bit("交易所"));

	ui.tableWidget_Entrust->setHorizontalHeaderLabels(headerEntrust);//设置列名

	QStringList VerticalEntrust;
	VerticalEntrust.append(QString::fromLocal8Bit("IF加权"));
	ui.tableWidget_Entrust->setVerticalHeaderLabels(VerticalEntrust);
	ui.tableWidget_Entrust->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_Entrust->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_Entrust->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}

//---------------3.成交表初始化----------------///
void QtCTP::QtBargainTableWidget()
{
	ui.tableWidget_Bargain->setColumnCount(8);
	ui.tableWidget_Bargain->setRowCount(10);

	QStringList headerBargain;
	headerBargain.append(QString::fromLocal8Bit("成交时间"));
	headerBargain.append(QString::fromLocal8Bit("合约代码"));
	headerBargain.append(QString::fromLocal8Bit("买卖"));
	headerBargain.append(QString::fromLocal8Bit("开平"));
	headerBargain.append(QString::fromLocal8Bit("数量"));
	headerBargain.append(QString::fromLocal8Bit("价格"));
	headerBargain.append(QString::fromLocal8Bit("委托号"));
	headerBargain.append(QString::fromLocal8Bit("交易所"));

	ui.tableWidget_Bargain->setHorizontalHeaderLabels(headerBargain);//设置列名
	ui.tableWidget_Bargain->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_Bargain->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_Bargain->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}

//---------------4.1持仓表初始化----------------///
void QtCTP::QtPositionTableWidget()
{
	ui.tableWidget_Position->setColumnCount(4);//设置列数为4
	ui.tableWidget_Position->setRowCount(6);//设置行数为6

	QStringList headerPosition;
	headerPosition.append(QString::fromLocal8Bit("合约代码"));
	headerPosition.append(QString::fromLocal8Bit("持仓类型"));
	headerPosition.append(QString::fromLocal8Bit("持仓数量"));
	headerPosition.append(QString::fromLocal8Bit("持仓成本"));

	ui.tableWidget_Position->setHorizontalHeaderLabels(headerPosition);//设置列名
	ui.tableWidget_Position->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_Position->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_Position->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}

//4.2资金表初始化
void QtCTP::QtFundTableWidget()
{
	ui.tableWidget_fund->setColumnCount(5);//设置列数为5
	ui.tableWidget_fund->setRowCount(6);//设置行数为6

	QStringList headerFund;
	headerFund.append(QString::fromLocal8Bit("账号"));
	headerFund.append(QString::fromLocal8Bit("总权益"));
	headerFund.append(QString::fromLocal8Bit("占用保证金"));
	headerFund.append(QString::fromLocal8Bit("可用资金"));
	headerFund.append(QString::fromLocal8Bit("风险度"));

	ui.tableWidget_fund->setHorizontalHeaderLabels(headerFund);//设置列名
	ui.tableWidget_fund->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//均匀拉直表头
	ui.tableWidget_fund->setSelectionBehavior(QAbstractItemView::SelectRows);//整行选中的方式
	ui.tableWidget_fund->setEditTriggers(QTableWidget::NoEditTriggers);//将表格变为禁止编辑
}


//判断是否有股指期货信息
bool QtCTP::ExitStockFutures(QString str)
{
/*	std::vector<QString> vecType;*/

	vecTypeStock.push_back(QString::fromLocal8Bit("IF加权"));
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
	vecTypeStock.push_back(QString::fromLocal8Bit("IH加权"));
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
	vecTypeStock.push_back(QString::fromLocal8Bit("IC加权"));
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

//判断是否有贵金属信息
bool QtCTP::ExitPreciousMetal(QString str)
{
	std::vector<QString> vecType;

	vecType.push_back(QString::fromLocal8Bit("贵金属"));
	vecType.push_back(QString::fromLocal8Bit("沪金指数"));
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
	vecType.push_back(QString::fromLocal8Bit("沪银指数"));
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

//点击登录按钮
void QtCTP::slotButtonLogin()
{
// 	PhdOperExcel excel;
// 	QString str = "D:\\CTP项目资料\\CTP\CTP1分钟Tick数据\\主连合约\\上期能源\\20号胶主连1分钟.csv";
// 	excel.OpenExcel(str);

	//QString转char
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

//显示行情信息
void QtCTP::slotReceiveMarket(QString MarketTick)
{
	QStringList strlist = MarketTick.split(",");
	slotReceiveMarketStockFutures(strlist);
	slotReceiveMarketPreciousMetal(strlist);
}

//股指期货信息
void QtCTP::slotReceiveMarketStockFutures(QStringList strlist)//股指期货
{
	QString str = strlist.at(0);
	//判断是否有股指期货信息
	if (!ExitStockFutures(str))
		return;

	for (int i = 0; i < ui.tableWidget_Market_StockFutures->rowCount(); i++)
	{		
		//判断第i行第1列的内容是否与第一个拆分的字符串相等
		//判断控件表格中获取的内容与接口中获取的合约代码是否相同，相同则更新控件表格中数据
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
	//合约代码不同则重新插入一行
	int row = ui.tableWidget_Market_StockFutures->rowCount();                               //获取行情表的行数
	ui.tableWidget_Market_StockFutures->insertRow(row);                                     //插入获取的行数
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

//贵金属信息
void QtCTP::slotReceiveMarketPreciousMetal(QStringList strlist)//贵金属信息
{
	QString str = strlist.at(0);
	//判断是否有贵金属信息
	if (!ExitPreciousMetal(str))
		return;

	for (int i = 0; i < ui.tableWidget_Market_PreciousMetal->rowCount(); i++)
	{
		QString InstrumentId = strlist.at(0);
		InstrumentId.replace("au", QString::fromLocal8Bit("沪金"));
		InstrumentId.replace("ag", QString::fromLocal8Bit("沪银"));
		//判断第i行第1列的内容是否与第一个拆分的字符串相等
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

//数据图界面跳转返回至主界面
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
		qDebug() << "选中了某行";
		int row = ui.tableWidget_Market_StockFutures->currentRow();
		if (row < 0)
		{
			row = ui.tableWidget_Market_PreciousMetal->currentRow();
			//得到用户选择的合约代码
			QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
			QString strHydm = pItem->text();
		}
		else
		{
			//得到用户选择的合约代码
			QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
			QString strHydm = pItem->text();
		}
		ui.stackedWidget_Main->setCurrentIndex(1);
		ui.tabWidget_Chart->setCurrentIndex(0);
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先选择一行再进行操作"));
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
		qDebug() << "选中了某行";
		int row = ui.tableWidget_Market_StockFutures->currentRow();
		if (row < 0)
		{
			row = ui.tableWidget_Market_PreciousMetal->currentRow();
			//得到用户选择的合约代码
			QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
			QString strHydm = pItem->text();
		}
		else
		{
			//得到用户选择的合约代码
			QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
			QString strHydm = pItem->text();
		}
		slotReceiveKLineChart();//k线图信息
		ui.stackedWidget_Main->setCurrentIndex(1);
		ui.tabWidget_Chart->setCurrentIndex(1);
	}
	else
	{
		QMessageBox::warning(this, QString::fromLocal8Bit("警告"), QString::fromLocal8Bit("请先选择一行再进行操作"));
	}
}

void QtCTP::slotCellClickedStock(int row, int column)
{
	//得到用户选择的合约代码
	QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
	QString strHydm = pItem->text();

	m_pUi_Chart->SetHydm(strHydm);//设置要绘制的合约代码
	m_pUi_Chart->show();
	this->close();
}

void QtCTP::slotReceiveTimeSharing(QString dataTimeSharing)
{
/*	ui.setupUi(this);*/
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
	dateTimeTicker->setDateTimeFormat("hh:mm:ss");//设置x轴刻度显示格式
	ui.customPlot->addGraph();//添加一条线

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

void QtCTP::slotReceiveKLineChart()
{
	//初始化k线
	m_pCandlesticks = new QCPFinancial(ui.customPlot_KLine->xAxis, ui.customPlot_KLine->yAxis);
	//如果设置为true，向QCustomPlot添加一个标绘表(例如一个图形)也会自动添加图例可标绘(QCustomPlot::图例)。
	ui.customPlot_KLine->setAutoAddPlottableToLegend(m_pCandlesticks);
	m_pCandlesticks->setChartStyle(QCPFinancial::csCandlestick);

	m_pCandlesticks->setTwoColored(true);//设置颜色
	m_pCandlesticks->setBrushPositive(QColor(255, 0, 0));//红色
	m_pCandlesticks->setBrushNegative(QColor(0, 100, 0));//蓝色    下面的颜色
	m_pCandlesticks->setPenPositive(QColor(0, 0, 0));
	m_pCandlesticks->setPenNegative(QColor(0, 0, 0));
	ui.customPlot_KLine->xAxis->scaleRange(1, Qt::AlignRight);//可移动
	ui.customPlot_KLine->setInteraction(QCP::iRangeDrag, true);//可拖动
	ui.customPlot_KLine->axisRect()->setRangeDrag(Qt::Horizontal);//只允许水平拖动,不允许上下拖动

	//2.添加数据
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

	m_pCandlesticks->setWidth(5 * 0.005);//设置宽度
	ui.customPlot_KLine->rescaleAxes();//重新调整坐标轴
	ui.customPlot_KLine->replot();//重绘
}

//双击行情股指期货表格槽函数
void QtCTP::slotDoubleClicked(int row, int column)
{
	//得到用户选择的合约代码
	QTableWidgetItem* pItem = ui.tableWidget_Market_StockFutures->item(row, 0);
	QString strHydm = pItem->text();

	m_pUi_Chart->SetHydm(strHydm);//设置要绘制的合约代码
	m_pUi_Chart->show();
	this->close();
}

void QtCTP::slotDoubleClickedPreciousMetal(int row, int column)
{
	//得到用户选择的合约代码
	QTableWidgetItem* pItem = ui.tableWidget_Market_PreciousMetal->item(row, 0);
	QString strHydm = pItem->text();
	strHydm.replace(QString::fromLocal8Bit("沪金"),"au");
	strHydm.replace(QString::fromLocal8Bit("沪银"),"ag");

	m_pUi_Chart->SetHydm(strHydm);//设置要绘制的合约代码
	m_pUi_Chart->show();
	this->close();
}

