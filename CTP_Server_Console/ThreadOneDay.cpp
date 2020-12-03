#include "ThreadOneDay.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <deque>
#include "CCsvData.h"
#include <iostream>

ThreadOneDay::ThreadOneDay()
	:m_pTimerOneDay(nullptr)
{
	//连接redis数据库
	bool bRt = false;
#ifdef _DEBUG
	bRt = m_redis.ConnectRedis();
#else
	bRt = m_redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com");
#endif 
	if (bRt)
		std::cout << "1天线程redis连接成功" << std::endl;
	else
		std::cout << "1天线程redis连接失败" << std::endl;
}

ThreadOneDay::~ThreadOneDay()
{
	if (m_pTimerOneDay)
	{
		delete m_pTimerOneDay;
		m_pTimerOneDay = nullptr;
	}
}

void ThreadOneDay::CloseThread()
{
	//关闭定时器
	if (m_pTimerOneDay->isActive())//判断定时器是否运行
		m_pTimerOneDay->stop();	//停止定时器
}

void ThreadOneDay::run()
{
	std::cout << "1天线程定时器开启" << std::endl;

	//初始化定时器
	m_pTimerOneDay = new QTimer;
	//设置定时器是否为单次触发。默认为 false 多次触发
	m_pTimerOneDay->setSingleShot(false);
	//定时器触发信号槽
	connect(m_pTimerOneDay, SIGNAL(timeout()), this, SLOT(slotTimerOneDay()), Qt::DirectConnection);
	//开启定时器
	m_pTimerOneDay->start(1000);//启动或重启定时器, 并设置定时器时间：毫秒

	//开启事件循环
	exec();
}

void ThreadOneDay::slotTimerOneDay()
{
	//得到当前时间
	QDateTime curTime = QDateTime::currentDateTime();
	QString strCurTime = curTime.toString("hh:mm:ss");
	if (strCurTime != "00:00:00")
		return;

	QString strCurDateTime = curTime.toString("yyyy.MM.dd hh:mm:ss");	//当前日期字符串
	std::cout << strCurDateTime.toStdString() << "  1天线程定时器触发，开始加工csv文件数据.............................." << std::endl;

	QString strCurDay = curTime.toString("yyyy.MM.dd");	//当天字符串
	QString strLastDay = curTime.addSecs(-60).toString("yyyy.MM.dd");//昨天字符串

	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//创建csv目录
	dir.cd("csv");//进入csv目录
	dir.mkdir(strLastDay);//创建日期目录
	dir.cd(strLastDay);//进入日期目录
	QFileInfoList fileInfoList = dir.entryInfoList();//得到当天的csv文件路径集合

	//遍历每一个csv文件
	for (int i = 0; i < fileInfoList.size(); i++)
	{
		QFileInfo info = fileInfoList.at(i);
		if (info.suffix() != "csv")
			continue;
		QString strFileName = info.baseName();	//合约代码名
		QString strFilePath = info.filePath();	//csv文件路径

		std::cout << strCurDateTime.toStdString() << "  1天线程：开始加工 " << strFileName.toStdString() << " csv数据" << std::endl;
		QTime time;
		time.start();

		//加工csv文件中1天的数据
		QString strProcessData;
		ProcessCsvDataOneDay(strFilePath, strLastDay, strCurDay, strProcessData);

		if (strProcessData == "")
		{
			std::cout << strCurDateTime.toStdString() << "  1天线程：" << strFileName.toStdString() << " 没有加工出数据，请查看csv文件中是否有数据" << std::endl;
			continue;
		}
		std::cout << strCurDateTime.toStdString() << "  1天线程：" << strFileName.toStdString() << " 加工出数据" <<  std::endl;

		//1天数据添加到redis数据库
		bool bRt = AppendToRedisOneDay(strFileName, strProcessData);
		if (bRt)
			std::cout << strCurDateTime.toStdString() << "  1天线程：" << strFileName.toStdString() << " 数据全部添加到redis中" << std::endl;
		else
			std::cout << strCurDateTime.toStdString() << "  1天线程：" << strFileName.toStdString() << " 数据没有添加到redis中" << std::endl;
	}
}

void ThreadOneDay::ProcessCsvDataOneDay(const QString& strFilePath, const QString& strLastDay, const QString& strCurDay, QString& strProcessData) const
{
	//读取csv文件数据
	QFile file(strFilePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream in(&file);
	QStringList listRowData = in.readAll().split("\n");//每行以\n区分
	file.close();
	int nCountRow = listRowData.size();
	if (nCountRow < 2)
		return;

	//
	QString strHydm;//合约代码
	std::vector<double> m_vecPrice;
	std::vector<double> m_vecVolume;
	for (int i = 0; i < nCountRow -1; i++)
	{
		QStringList listCellData = listRowData[i].split(",");
		if (i == 0)
			strHydm = listCellData[0];
		double dPrice = listCellData[2].toDouble();		//最新价
		double dVolume = listCellData[3].toDouble();	//成交量
		m_vecPrice.push_back(dPrice);
		m_vecVolume.push_back(dVolume);
	}

	QString strOpenPrice = QString("%1").arg(m_vecPrice.front());	//开盘价
	QString strHighestPrice = QString("%1").arg(*std::max_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//最高价
	QString strLowestPrice = QString("%1").arg(*std::min_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//最低价
	QString strClosePrice = QString("%1").arg(m_vecPrice.back());//收盘价
	// 成交量的真实的算法是当前区间最后一个成交量减去上去一个区间最后一个成交量
	QString strVolume = QString("%1").arg(m_vecVolume.back() - m_vecVolume.front());//成交量

	strProcessData = strHydm + "," + strLastDay + "," + strCurDay + "," +
		strOpenPrice + "," + strHighestPrice + "," + strLowestPrice + "," + strClosePrice + "," + strVolume + "\n";

}

bool ThreadOneDay::AppendToRedisOneDay(const QString& strHydm, const QString& strProcessData)
{
	//切换到db6数据库
	if (!m_redis.SwitchDb(6))
		return false;

	//添加数据
	return m_redis.SetKeyValue(strHydm.toStdString(), strProcessData.toStdString());
}

