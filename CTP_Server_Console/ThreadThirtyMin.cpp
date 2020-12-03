#include "ThreadThirtyMin.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <deque>
#include "CCsvData.h"
#include <iostream>

ThreadThirtyMin::ThreadThirtyMin()
	:m_pTimerThirtyMin(nullptr)
{
	//连接redis数据库
	bool bRt = false;
#ifdef _DEBUG
	bRt = m_redis.ConnectRedis();
#else
	bRt = m_redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com");
#endif 
	if (bRt)
		std::cout << "30分钟线程redis连接成功" << std::endl;
	else
		std::cout << "30分钟线程redis连接失败" << std::endl;
}

ThreadThirtyMin::~ThreadThirtyMin()
{
	if (m_pTimerThirtyMin)
	{
		delete m_pTimerThirtyMin;
		m_pTimerThirtyMin = nullptr;
	}
}

void ThreadThirtyMin::CloseThread()
{
	//关闭定时器
	if (m_pTimerThirtyMin->isActive())//判断定时器是否运行
		m_pTimerThirtyMin->stop();	//停止定时器
}

void ThreadThirtyMin::run()
{
	std::cout << "30分钟线程定时器开启" << std::endl;

	//初始化定时器
	m_pTimerThirtyMin = new QTimer;
	//设置定时器是否为单次触发。默认为 false 多次触发
	m_pTimerThirtyMin->setSingleShot(false);
	//定时器触发信号槽
	connect(m_pTimerThirtyMin, SIGNAL(timeout()), this, SLOT(slotTimerThirtyMin()), Qt::DirectConnection);
	//开启定时器
	m_pTimerThirtyMin->start(60 * 1000 * 30);//启动或重启定时器, 并设置定时器时间：毫秒
	//m_pTimerThirtyMin->start(1000 * 30);

	//开启事件循环
	exec();
}

void ThreadThirtyMin::slotTimerThirtyMin()
{
	QDateTime curTime = QDateTime::currentDateTime();
	QString strCurDateTime = curTime.toString("yyyy.MM.dd hh:mm:ss");	//当前日期字符串
	std::cout << strCurDateTime.toStdString() << "  30分钟线程定时器触发，开始加工csv文件数据.............................." << std::endl;

	//return;

	//存当天的数据
	QString strCurDay = curTime.toString("yyyy.MM.dd");	//当天字符串
	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//创建csv目录
	dir.cd("csv");//进入csv目录
	dir.mkdir(strCurDay);//创建日期目录
	dir.cd(strCurDay);//进入日期目录
	QFileInfoList fileInfoList = dir.entryInfoList();//得到当天的csv文件路径集合

	//遍历每一个csv文件
	for (int i = 0; i < fileInfoList.size(); i++)
	{
		QFileInfo info = fileInfoList.at(i);
		if (info.suffix() != "csv")
			continue;
		QString strFileName = info.baseName();	//合约代码名
		QString strFilePath = info.filePath();	//csv文件路径

		std::cout << strCurDateTime.toStdString() << "  30分钟线程：开始加工 " << strFileName.toStdString() << " csv数据" << std::endl;
		QTime time;
		time.start();

		//读取redis数据库中该合约代码最后一条数据的时间
		QString strRedisLastDateTime = GetRedisLastDateTimeThirtyMin(strFileName);
		//加工csv文件中30分钟的数据
		std::vector<QString> vecProcessData;
		ProcessCsvDataThirtyMin(strFilePath, strRedisLastDateTime, vecProcessData);

		std::cout << "加工时长：" << time.elapsed() / 1000.0 << "s" << std::endl;
		if (vecProcessData.size() == 0)
		{
			std::cout << strCurDateTime.toStdString() << "  30分钟线程：" << strFileName.toStdString() << " 没有加工出数据，请查看csv文件中是否有新数据" << std::endl;
			continue;
		}
		std::cout << strCurDateTime.toStdString() << "  30分钟线程：" << strFileName.toStdString() << " 加工出数据" << vecProcessData.size() << "条" << std::endl;

		//30分钟数据添加到redis数据库
		bool bRt = AppendToRedisThirtyMin(strFileName, vecProcessData);
		if (bRt)
			std::cout << strCurDateTime.toStdString() << "  30分钟线程：" << strFileName.toStdString() << " 数据全部添加到redis中" << std::endl;
		else
			std::cout << strCurDateTime.toStdString() << "  30分钟线程：" << strFileName.toStdString() << " 部分或全部数据没有添加到redis中" << std::endl;
	}
}

void ThreadThirtyMin::ProcessCsvDataThirtyMin(const QString& strFilePath, const QString& strRedisLastDateTime,
	std::vector<QString>& vecProcessData) const
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

	//反向遍历，得到大于strRedisLastDateTime时间的数据
	std::deque<QString> deqCsvData;
	QDateTime curDateTime = QDateTime::currentDateTime();
	QString strCurDate = curDateTime.toString("yyyy.MM.dd");	//当天日期

	for (int i = nCountRow - 2; i >= 0; i--)
	{
		QStringList listCellData = listRowData[i].split(",");
		QString strCsvTime = listCellData[1];
		
		//比较strRedisDateTime时间是否小于strCsvDateTime时间，小的话，说明该行数据是需要加工的
		//如果strRedisDateTime为空，直接添加数据
		QString strCsvDateTime = strCurDate + " " + strCsvTime;
		if (strRedisLastDateTime != "")
		{
			if (CompDateTime(strCsvDateTime, strRedisLastDateTime) < 0)
				break;
		}
		deqCsvData.push_front(listRowData[i]);//将该行数据添加到deqCsvData对象中
	}
	if (deqCsvData.size() == 0)
		return;

	//将数据按30分钟分类
	QString strDateTimeStart, strDateTimeEnd;
	CCsvData csvdata;
	for (int i = 0; i < deqCsvData.size(); i++)
	{
		//判断csv时间是否在strTimeStart和strTimeEnd时间内
		QStringList listCellData = deqCsvData[i].split(",");
		QString strCsvDateTime = strCurDate + " " + listCellData[1];
		if (!IsItInTime(strCsvDateTime, strDateTimeStart, strDateTimeEnd))
		{//重新赋值时间字符串
			if (strDateTimeEnd == "")
			{
				strDateTimeStart = strCsvDateTime;
				strDateTimeStart = strDateTimeStart.left(strDateTimeStart.size() - 2);
				strDateTimeStart += "00";
				QDateTime datetimeTemp = QDateTime::fromString(strDateTimeStart, "yyyy.MM.dd hh:mm:ss");
				strDateTimeEnd = datetimeTemp.addSecs(60 * 30).toString("yyyy.MM.dd hh:mm:ss");
			}
			else
			{
				strDateTimeStart = strDateTimeEnd;
				QDateTime datetimeTemp = QDateTime::fromString(strDateTimeStart, "yyyy.MM.dd hh:mm:ss");
				strDateTimeEnd = datetimeTemp.addSecs(60 * 30).toString("yyyy.MM.dd hh:mm:ss");

				i--;
				continue;
			}
		}
		//添加数据
		csvdata.AppendData(strDateTimeStart, strDateTimeEnd, deqCsvData[i]);
	}

	//加工数据
	std::map<QString, std::vector<QString> > mapCsvData = csvdata.GetCsvData();
	if (mapCsvData.size() == 0)
		return;
	for (auto iter = mapCsvData.begin(); iter != std::end(mapCsvData); iter++)
	{
		QString strProcess;
		QString strKey = iter->first;
		std::vector<QString> vecData = iter->second;

		QString strStartTime, strEndTime;//开始时间，结束时间
		int nFindIndex = strKey.indexOf("-");
		if (nFindIndex == -1)
			continue;
		strStartTime = strKey.left(nFindIndex);
		strEndTime = strKey.right(strKey.size() - nFindIndex - 1);
		nFindIndex = strStartTime.indexOf(" ");
		if (nFindIndex == -1)
			continue;
		strStartTime = strStartTime.right(strStartTime.size() - nFindIndex - 1);
		nFindIndex = strEndTime.indexOf(" ");
		if (nFindIndex == -1)
			continue;
		strEndTime = strEndTime.right(strEndTime.size() - nFindIndex - 1);

		QString strHydm;//合约代码
		std::vector<double> m_vecPrice;
		std::vector<double> m_vecVolume;
		for (int i = 0; i < vecData.size(); i++)
		{
			QStringList dataRow = vecData[i].split(",");
			if (i == 0)
				strHydm = dataRow[0];
			double dPrice = dataRow[2].toDouble();	//最新价
			double dVolume = dataRow[3].toDouble();	//成交量
			m_vecPrice.push_back(dPrice);
			m_vecVolume.push_back(dVolume);
		}

		QString strOpenPrice = QString("%1").arg(m_vecPrice.front());	//开盘价
		QString strHighestPrice = QString("%1").arg(*std::max_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//最高价
		QString strLowestPrice = QString("%1").arg(*std::min_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//最低价
		QString strClosePrice = QString("%1").arg(m_vecPrice.back());//收盘价
		// 成交量的真实的算法是当前区间最后一个成交量减去上去一个区间最后一个成交量
		QString strVolume = QString("%1").arg(m_vecVolume.back() - m_vecVolume.front());//成交量

		strProcess = strHydm + "," + strCurDate + "," + strStartTime + "," + strEndTime + "," +
			strOpenPrice + "," + strHighestPrice + "," + strLowestPrice + "," + strClosePrice + "," + strVolume + "\n";
		vecProcessData.push_back(strProcess);
	}

	//去掉最后一个时间段的数据
	if (vecProcessData.size() > 0)
		vecProcessData.pop_back();
}

bool ThreadThirtyMin::AppendToRedisThirtyMin(const QString& strHydm, const std::vector<QString>& vecProcessData)
{
	//切换到db3数据库
	if (!m_redis.SwitchDb(3))
		return false;

	//添加数据
	bool bRt = true;
	for (int i = 0; i < vecProcessData.size(); i++)
	{
		if (!m_redis.SetKeyValue(strHydm.toStdString(), vecProcessData[i].toStdString()))
			bRt = false;
	}

	return bRt;
}

QString ThreadThirtyMin::GetRedisLastDateTimeThirtyMin(const QString& strHydm)
{
	QString strRedisLastDateTime;

	//切换到db3数据库
	m_redis.SwitchDb(3);

	//读取该合约代码的value
	std::string strValue;
	m_redis.GetKeyValue(strHydm.toStdString(), strValue);

	//判断value中是否有数据
	QString strQValue = QString::fromStdString(strValue);
	QStringList listRowData = strQValue.split("\n");//每行以\n区分
	int nCountRow = listRowData.size();
	if (nCountRow > 1)
	{//redis中有数据
		//得到最后一行数据的时间
		QStringList listCellData = listRowData[nCountRow - 2].split(",");
		QString strDate = listCellData[1];//日期
		QString strEndTime = listCellData[3];//结束时间
		strRedisLastDateTime = strDate + " " + strEndTime;
		return strRedisLastDateTime;
	}
	else
	{//redis中没数据
		return strRedisLastDateTime;
	}
}

int ThreadThirtyMin::CompDateTime(const QString& strDateTime1, const QString& strDateTime2) const
{
	QDateTime datetime1 = QDateTime::fromString(strDateTime1, "yyyy.MM.dd hh:mm:ss");
	uint dDateTime1 = datetime1.toTime_t();

	QDateTime datetime2 = QDateTime::fromString(strDateTime2, "yyyy.MM.dd hh:mm:ss");
	uint dDateTime2 = datetime2.toTime_t();

	if (dDateTime1 > dDateTime2)
		return 1;
	else if (dDateTime1 < dDateTime2)
		return -1;
	else
		return 0;
}

bool ThreadThirtyMin::IsItInTime(const QString& strDateTime, const QString& strDateTimeStart, const QString& strDateTimeEnd) const
{
	if (strDateTimeStart == "" || strDateTimeEnd == "")
		return false;

	QDateTime datetime = QDateTime::fromString(strDateTime, "yyyy.MM.dd hh:mm:ss");
	uint dDateTime = datetime.toTime_t();

	QDateTime datetimeStart = QDateTime::fromString(strDateTimeStart, "yyyy.MM.dd hh:mm:ss");
	uint dDateTimeStart = datetimeStart.toTime_t();

	QDateTime datetimeEnd = QDateTime::fromString(strDateTimeEnd, "yyyy.MM.dd hh:mm:ss");
	uint dDateTimeEnd = datetimeEnd.toTime_t();

	if (dDateTime >= dDateTimeStart && dDateTime < dDateTimeEnd)
		return true;
	else
		return false;
}
