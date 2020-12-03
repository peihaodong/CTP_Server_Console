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
	//����redis���ݿ�
	bool bRt = false;
#ifdef _DEBUG
	bRt = m_redis.ConnectRedis();
#else
	bRt = m_redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com");
#endif 
	if (bRt)
		std::cout << "1���߳�redis���ӳɹ�" << std::endl;
	else
		std::cout << "1���߳�redis����ʧ��" << std::endl;
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
	//�رն�ʱ��
	if (m_pTimerOneDay->isActive())//�ж϶�ʱ���Ƿ�����
		m_pTimerOneDay->stop();	//ֹͣ��ʱ��
}

void ThreadOneDay::run()
{
	std::cout << "1���̶߳�ʱ������" << std::endl;

	//��ʼ����ʱ��
	m_pTimerOneDay = new QTimer;
	//���ö�ʱ���Ƿ�Ϊ���δ�����Ĭ��Ϊ false ��δ���
	m_pTimerOneDay->setSingleShot(false);
	//��ʱ�������źŲ�
	connect(m_pTimerOneDay, SIGNAL(timeout()), this, SLOT(slotTimerOneDay()), Qt::DirectConnection);
	//������ʱ��
	m_pTimerOneDay->start(1000);//������������ʱ��, �����ö�ʱ��ʱ�䣺����

	//�����¼�ѭ��
	exec();
}

void ThreadOneDay::slotTimerOneDay()
{
	//�õ���ǰʱ��
	QDateTime curTime = QDateTime::currentDateTime();
	QString strCurTime = curTime.toString("hh:mm:ss");
	if (strCurTime != "00:00:00")
		return;

	QString strCurDateTime = curTime.toString("yyyy.MM.dd hh:mm:ss");	//��ǰ�����ַ���
	std::cout << strCurDateTime.toStdString() << "  1���̶߳�ʱ����������ʼ�ӹ�csv�ļ�����.............................." << std::endl;

	QString strCurDay = curTime.toString("yyyy.MM.dd");	//�����ַ���
	QString strLastDay = curTime.addSecs(-60).toString("yyyy.MM.dd");//�����ַ���

	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//����csvĿ¼
	dir.cd("csv");//����csvĿ¼
	dir.mkdir(strLastDay);//��������Ŀ¼
	dir.cd(strLastDay);//��������Ŀ¼
	QFileInfoList fileInfoList = dir.entryInfoList();//�õ������csv�ļ�·������

	//����ÿһ��csv�ļ�
	for (int i = 0; i < fileInfoList.size(); i++)
	{
		QFileInfo info = fileInfoList.at(i);
		if (info.suffix() != "csv")
			continue;
		QString strFileName = info.baseName();	//��Լ������
		QString strFilePath = info.filePath();	//csv�ļ�·��

		std::cout << strCurDateTime.toStdString() << "  1���̣߳���ʼ�ӹ� " << strFileName.toStdString() << " csv����" << std::endl;
		QTime time;
		time.start();

		//�ӹ�csv�ļ���1�������
		QString strProcessData;
		ProcessCsvDataOneDay(strFilePath, strLastDay, strCurDay, strProcessData);

		if (strProcessData == "")
		{
			std::cout << strCurDateTime.toStdString() << "  1���̣߳�" << strFileName.toStdString() << " û�мӹ������ݣ���鿴csv�ļ����Ƿ�������" << std::endl;
			continue;
		}
		std::cout << strCurDateTime.toStdString() << "  1���̣߳�" << strFileName.toStdString() << " �ӹ�������" <<  std::endl;

		//1��������ӵ�redis���ݿ�
		bool bRt = AppendToRedisOneDay(strFileName, strProcessData);
		if (bRt)
			std::cout << strCurDateTime.toStdString() << "  1���̣߳�" << strFileName.toStdString() << " ����ȫ����ӵ�redis��" << std::endl;
		else
			std::cout << strCurDateTime.toStdString() << "  1���̣߳�" << strFileName.toStdString() << " ����û����ӵ�redis��" << std::endl;
	}
}

void ThreadOneDay::ProcessCsvDataOneDay(const QString& strFilePath, const QString& strLastDay, const QString& strCurDay, QString& strProcessData) const
{
	//��ȡcsv�ļ�����
	QFile file(strFilePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	QTextStream in(&file);
	QStringList listRowData = in.readAll().split("\n");//ÿ����\n����
	file.close();
	int nCountRow = listRowData.size();
	if (nCountRow < 2)
		return;

	//
	QString strHydm;//��Լ����
	std::vector<double> m_vecPrice;
	std::vector<double> m_vecVolume;
	for (int i = 0; i < nCountRow -1; i++)
	{
		QStringList listCellData = listRowData[i].split(",");
		if (i == 0)
			strHydm = listCellData[0];
		double dPrice = listCellData[2].toDouble();		//���¼�
		double dVolume = listCellData[3].toDouble();	//�ɽ���
		m_vecPrice.push_back(dPrice);
		m_vecVolume.push_back(dVolume);
	}

	QString strOpenPrice = QString("%1").arg(m_vecPrice.front());	//���̼�
	QString strHighestPrice = QString("%1").arg(*std::max_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//��߼�
	QString strLowestPrice = QString("%1").arg(*std::min_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//��ͼ�
	QString strClosePrice = QString("%1").arg(m_vecPrice.back());//���̼�
	// �ɽ�������ʵ���㷨�ǵ�ǰ�������һ���ɽ�����ȥ��ȥһ���������һ���ɽ���
	QString strVolume = QString("%1").arg(m_vecVolume.back() - m_vecVolume.front());//�ɽ���

	strProcessData = strHydm + "," + strLastDay + "," + strCurDay + "," +
		strOpenPrice + "," + strHighestPrice + "," + strLowestPrice + "," + strClosePrice + "," + strVolume + "\n";

}

bool ThreadOneDay::AppendToRedisOneDay(const QString& strHydm, const QString& strProcessData)
{
	//�л���db6���ݿ�
	if (!m_redis.SwitchDb(6))
		return false;

	//�������
	return m_redis.SetKeyValue(strHydm.toStdString(), strProcessData.toStdString());
}

