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

//����CTP���ļ�
#pragma comment (lib, "thostmduserapi_se.lib")

//	�õ���Լ�����ַ�������
bool GetHydmVector(std::vector<std::string>& vecHydm);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	//��ȡҪ���ĵĺ�Լ����
	std::vector<std::string> vecHydm;
	if (!GetHydmVector(vecHydm))
	{
		std::cout << "��ȡ��Լ����ʧ��" << std::endl;
		return -1;
	}

	//���������¼�ʵ��
	QtCustomMdSpi* pMd = new QtCustomMdSpi;
	std::strcpy(pMd->m_hq.gMdFrontAddr, "tcp://180.168.146.187:10111");
	std::strcpy(pMd->m_hq.gBrokerID, "9999");
	std::strcpy(pMd->m_hq.gInvesterID, "175068");
	std::strcpy(pMd->m_hq.gInvesterPassword, "Donny123");
	pMd->Init(vecHydm);//��������
	
	//�����������߳�
	ThreadOneMin* pThreadOneMin = new ThreadOneMin;				//1�����߳�ʵ��
	pThreadOneMin->start();
	
	ThreadFiveMin* pThreadFiveMin = new ThreadFiveMin;			//5�����߳�ʵ��
	pThreadFiveMin->start();

	ThreadFifteenMin* pThreadFifteenMin = new ThreadFifteenMin;	//15�����߳�ʵ��
	pThreadFifteenMin->start();

	ThreadThirtyMin* pThreadThirtyMin = new ThreadThirtyMin;	//30�����߳�ʵ��
	pThreadThirtyMin->start();

	ThreadOneHour* pThreadOneHour = new ThreadOneHour;			//1Сʱ�߳�ʵ��
	pThreadOneHour->start();

	ThreadFourHour* pThreadFourHour = new ThreadFourHour;		//4Сʱ�߳�ʵ��
	pThreadFourHour->start();
	
 	ThreadOneDay* pThreadOneDay = new ThreadOneDay;				//1���߳�ʵ��
 	pThreadOneDay->start();

    return a.exec();
}

bool GetHydmVector(std::vector<std::string>& vecHydm)
{
	//�õ��ļ�·��
	QString strAppDir = QCoreApplication::applicationDirPath();
	QString strTxtPath = strAppDir + "\\HydmConfig.txt";
	//��ȡtxt�ļ�����
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