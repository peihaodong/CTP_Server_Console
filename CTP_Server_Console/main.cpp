#include <QtCore/QCoreApplication>
#include "QtCustomMdSpi.h"
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <memory>
#include "ThreadOneMin.h"
#include "ThreadFiveMin.h"
#include "ThreadFifteenMin.h"
#include "ThreadThirtyMin.h"
#include "ThreadOneHour.h"
#include "ThreadFourHour.h"
#include "ThreadOneDay.h"

//加载CTP库文件
#pragma comment (lib, "thostmduserapi_se.lib")

//	得到合约代码字符串集合
bool GetHydmVector(std::vector<std::string>& vecHydm);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	//获取要订阅的合约代码
	std::vector<std::string> vecHydm;
	if (!GetHydmVector(vecHydm))
	{
		std::cout << "获取合约代码失败" << std::endl;
		return -1;
	}

	//创建行情事件实例
	QtCustomMdSpi* pMd = new QtCustomMdSpi;
	std::strcpy(pMd->m_hq.gMdFrontAddr, "tcp://180.168.146.187:10111");
	std::strcpy(pMd->m_hq.gBrokerID, "9999");
	std::strcpy(pMd->m_hq.gInvesterID, "175068");
	std::strcpy(pMd->m_hq.gInvesterPassword, "Donny123");
	pMd->Init(vecHydm);//行情连接
	
	//创建并开启线程
	ThreadOneMin* pThreadOneMin = new ThreadOneMin;				//1分钟线程实例
	pThreadOneMin->start();
	
	ThreadFiveMin* pThreadFiveMin = new ThreadFiveMin;			//5分钟线程实例
	pThreadFiveMin->start();

	ThreadFifteenMin* pThreadFifteenMin = new ThreadFifteenMin;	//15分钟线程实例
	pThreadFifteenMin->start();

	ThreadThirtyMin* pThreadThirtyMin = new ThreadThirtyMin;	//30分钟线程实例
	pThreadThirtyMin->start();

	ThreadOneHour* pThreadOneHour = new ThreadOneHour;			//1小时线程实例
	pThreadOneHour->start();

	ThreadFourHour* pThreadFourHour = new ThreadFourHour;		//4小时线程实例
	pThreadFourHour->start();
	
 	ThreadOneDay* pThreadOneDay = new ThreadOneDay;				//1天线程实例
 	pThreadOneDay->start();

    return a.exec();
}

bool GetHydmVector(std::vector<std::string>& vecHydm)
{
	//得到文件路径
	QString strAppDir = QCoreApplication::applicationDirPath();
	QString strTxtPath = strAppDir + "\\HydmConfig.txt";
	//读取txt文件数据
	QFile file(strTxtPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QStringList list;
	QTextStream in(&file);
	while (!in.atEnd())
	{
		QString fileLine = in.readLine();
		if (fileLine == "")
			continue;
		vecHydm.push_back(fileLine.toStdString());
	}
	file.close();

	if (vecHydm.size() > 0)
		return true;
	else
		return false;
}