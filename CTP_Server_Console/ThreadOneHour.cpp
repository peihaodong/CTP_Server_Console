#include "ThreadOneHour.h"
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <deque>
#include "CCsvData.h"
#include <iostream>

ThreadOneHour::ThreadOneHour()
	:m_pTimerOneHour(nullptr)
{
	//����redis���ݿ�
	bool bRt = false;
#ifdef _DEBUG
	bRt = m_redis.ConnectRedis();
#else
	bRt = m_redis.ConnectRedis("42.192.1.47", 6379, "www.nadanaes.com");
#endif 
	if (bRt)
		std::cout << "1Сʱ�߳�redis���ӳɹ�" << std::endl;
	else
		std::cout << "1Сʱ�߳�redis����ʧ��" << std::endl;
}

ThreadOneHour::~ThreadOneHour()
{
	if (m_pTimerOneHour)
	{
		delete m_pTimerOneHour;
		m_pTimerOneHour = nullptr;
	}
}

void ThreadOneHour::CloseThread()
{
	//�رն�ʱ��
	if (m_pTimerOneHour->isActive())//�ж϶�ʱ���Ƿ�����
		m_pTimerOneHour->stop();	//ֹͣ��ʱ��
}

void ThreadOneHour::run()
{
	std::cout << "1Сʱ�̶߳�ʱ������" << std::endl;

	//��ʼ����ʱ��
	m_pTimerOneHour = new QTimer;
	//���ö�ʱ���Ƿ�Ϊ���δ�����Ĭ��Ϊ false ��δ���
	m_pTimerOneHour->setSingleShot(false);
	//��ʱ�������źŲ�
	connect(m_pTimerOneHour, SIGNAL(timeout()), this, SLOT(slotTimerOneHour()), Qt::DirectConnection);
	//������ʱ��
	m_pTimerOneHour->start(60 * 1000 * 60);//������������ʱ��, �����ö�ʱ��ʱ�䣺����
	//m_pTimerOneHour->start(60 * 1000);

	//�����¼�ѭ��
	exec();
}

void ThreadOneHour::slotTimerOneHour()
{
	QDateTime curTime = QDateTime::currentDateTime();
	QString strCurDateTime = curTime.toString("yyyy.MM.dd hh:mm:ss");	//��ǰ�����ַ���
	std::cout << strCurDateTime.toStdString() << "  1Сʱ�̶߳�ʱ����������ʼ�ӹ�csv�ļ�����.............................." << std::endl;

	//return;

	//�浱�������
	QString strCurDay = curTime.toString("yyyy.MM.dd");	//�����ַ���
	QString strAppDir = QCoreApplication::applicationDirPath();
	QDir dir(strAppDir);
	dir.mkdir("csv");//����csvĿ¼
	dir.cd("csv");//����csvĿ¼
	dir.mkdir(strCurDay);//��������Ŀ¼
	dir.cd(strCurDay);//��������Ŀ¼
	QFileInfoList fileInfoList = dir.entryInfoList();//�õ������csv�ļ�·������

	//����ÿһ��csv�ļ�
	for (int i = 0; i < fileInfoList.size(); i++)
	{
		QFileInfo info = fileInfoList.at(i);
		if (info.suffix() != "csv")
			continue;
		QString strFileName = info.baseName();	//��Լ������
		QString strFilePath = info.filePath();	//csv�ļ�·��

		std::cout << strCurDateTime.toStdString() << "  1Сʱ�̣߳���ʼ�ӹ� " << strFileName.toStdString() << " csv����" << std::endl;
		QTime time;
		time.start();

		//��ȡredis���ݿ��иú�Լ�������һ�����ݵ�ʱ��
		QString strRedisLastDateTime = GetRedisLastDateTimeOneHour(strFileName);
		//�ӹ�csv�ļ���1Сʱ������
		std::vector<QString> vecProcessData;
		ProcessCsvDataOneHour(strFilePath, strRedisLastDateTime, vecProcessData);

		std::cout << "�ӹ�ʱ����" << time.elapsed() / 1000.0 << "s" << std::endl;
		if (vecProcessData.size() == 0)
		{
			std::cout << strCurDateTime.toStdString() << "  1Сʱ�̣߳�" << strFileName.toStdString() << " û�мӹ������ݣ���鿴csv�ļ����Ƿ���������" << std::endl;
			continue;
		}
		std::cout << strCurDateTime.toStdString() << "  1Сʱ�̣߳�" << strFileName.toStdString() << " �ӹ�������" << vecProcessData.size() << "��" << std::endl;

		//1Сʱ�������ӵ�redis���ݿ�
		bool bRt = AppendToRedisOneHour(strFileName, vecProcessData);
		if (bRt)
			std::cout << strCurDateTime.toStdString() << "  1Сʱ�̣߳�" << strFileName.toStdString() << " ����ȫ�����ӵ�redis��" << std::endl;
		else
			std::cout << strCurDateTime.toStdString() << "  1Сʱ�̣߳�" << strFileName.toStdString() << " ���ֻ�ȫ������û�����ӵ�redis��" << std::endl;
	}
}

void ThreadOneHour::ProcessCsvDataOneHour(const QString& strFilePath, const QString& strRedisLastDateTime,
	std::vector<QString>& vecProcessData) const
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

	//����������õ�����strRedisLastDateTimeʱ�������
	std::deque<QString> deqCsvData;
	QDateTime curDateTime = QDateTime::currentDateTime();
	QString strCurDate = curDateTime.toString("yyyy.MM.dd");	//��������

	for (int i = nCountRow - 2; i >= 0; i--)
	{
		QStringList listCellData = listRowData[i].split(",");
		QString strCsvTime = listCellData[1];
		
		//�Ƚ�strRedisDateTimeʱ���Ƿ�С��strCsvDateTimeʱ�䣬С�Ļ���˵��������������Ҫ�ӹ���
		//���strRedisDateTimeΪ�գ�ֱ����������
		QString strCsvDateTime = strCurDate + " " + strCsvTime;
		if (strRedisLastDateTime != "")
		{
			if (CompDateTime(strCsvDateTime, strRedisLastDateTime) < 0)
				break;
		}
		deqCsvData.push_front(listRowData[i]);//�������������ӵ�deqCsvData������
	}
	if (deqCsvData.size() == 0)
		return;

	//�����ݰ�3���ӷ���
	QString strDateTimeStart, strDateTimeEnd;
	CCsvData csvdata;
	for (int i = 0; i < deqCsvData.size(); i++)
	{
		//�ж�csvʱ���Ƿ���strTimeStart��strTimeEndʱ����
		QStringList listCellData = deqCsvData[i].split(",");
		QString strCsvDateTime = strCurDate + " " + listCellData[1];
		if (!IsItInTime(strCsvDateTime, strDateTimeStart, strDateTimeEnd))
		{//���¸�ֵʱ���ַ���
			if (strDateTimeEnd == "")
			{
				strDateTimeStart = strCsvDateTime;
				strDateTimeStart = strDateTimeStart.left(strDateTimeStart.size() - 2);
				strDateTimeStart += "00";
				QDateTime datetimeTemp = QDateTime::fromString(strDateTimeStart, "yyyy.MM.dd hh:mm:ss");
				strDateTimeEnd = datetimeTemp.addSecs(60 * 60).toString("yyyy.MM.dd hh:mm:ss");
			}
			else
			{
				strDateTimeStart = strDateTimeEnd;
				QDateTime datetimeTemp = QDateTime::fromString(strDateTimeStart, "yyyy.MM.dd hh:mm:ss");
				strDateTimeEnd = datetimeTemp.addSecs(60 * 60).toString("yyyy.MM.dd hh:mm:ss");

				i--;
				continue;
			}
		}
		//��������
		csvdata.AppendData(strDateTimeStart, strDateTimeEnd, deqCsvData[i]);
	}

	//�ӹ�����
	std::map<QString, std::vector<QString> > mapCsvData = csvdata.GetCsvData();
	if (mapCsvData.size() == 0)
		return;
	for (auto iter = mapCsvData.begin(); iter != std::end(mapCsvData); iter++)
	{
		QString strProcess;
		QString strKey = iter->first;
		std::vector<QString> vecData = iter->second;

		QString strStartTime, strEndTime;//��ʼʱ�䣬����ʱ��
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

		QString strHydm;//��Լ����
		std::vector<double> m_vecPrice;
		std::vector<double> m_vecVolume;
		for (int i = 0; i < vecData.size(); i++)
		{
			QStringList dataRow = vecData[i].split(",");
			if (i == 0)
				strHydm = dataRow[0];
			double dPrice = dataRow[2].toDouble();	//���¼�
			double dVolume = dataRow[3].toDouble();	//�ɽ���
			m_vecPrice.push_back(dPrice);
			m_vecVolume.push_back(dVolume);
		}

		QString strOpenPrice = QString("%1").arg(m_vecPrice.front());	//���̼�
		QString strHighestPrice = QString("%1").arg(*std::max_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//��߼�
		QString strLowestPrice = QString("%1").arg(*std::min_element(m_vecPrice.cbegin(), m_vecPrice.cend()));//��ͼ�
		QString strClosePrice = QString("%1").arg(m_vecPrice.back());//���̼�
		// �ɽ�������ʵ���㷨�ǵ�ǰ�������һ���ɽ�����ȥ��ȥһ���������һ���ɽ���
		QString strVolume = QString("%1").arg(m_vecVolume.back() - m_vecVolume.front());//�ɽ���

		strProcess = strHydm + "," + strCurDate + "," + strStartTime + "," + strEndTime + "," +
			strOpenPrice + "," + strHighestPrice + "," + strLowestPrice + "," + strClosePrice + "," + strVolume + "\n";
		vecProcessData.push_back(strProcess);
	}

	//ȥ�����һ��ʱ��ε�����
	if (vecProcessData.size() > 0)
		vecProcessData.pop_back();
}

bool ThreadOneHour::AppendToRedisOneHour(const QString& strHydm, const std::vector<QString>& vecProcessData)
{
	//�л���db4���ݿ�
	if (!m_redis.SwitchDb(4))
		return false;

	//��������
	bool bRt = true;
	for (int i = 0; i < vecProcessData.size(); i++)
	{
		if (!m_redis.SetKeyValue(strHydm.toStdString(), vecProcessData[i].toStdString()))
			bRt = false;
	}

	return bRt;
}

QString ThreadOneHour::GetRedisLastDateTimeOneHour(const QString& strHydm)
{
	QString strRedisLastDateTime;

	//�л���db4���ݿ�
	m_redis.SwitchDb(4);

	//��ȡ�ú�Լ�����value
	std::string strValue;
	m_redis.GetKeyValue(strHydm.toStdString(), strValue);

	//�ж�value���Ƿ�������
	QString strQValue = QString::fromStdString(strValue);
	QStringList listRowData = strQValue.split("\n");//ÿ����\n����
	int nCountRow = listRowData.size();
	if (nCountRow > 1)
	{//redis��������
		//�õ����һ�����ݵ�ʱ��
		QStringList listCellData = listRowData[nCountRow - 2].split(",");
		QString strDate = listCellData[1];//����
		QString strEndTime = listCellData[3];//����ʱ��
		strRedisLastDateTime = strDate + " " + strEndTime;
		return strRedisLastDateTime;
	}
	else
	{//redis��û����
		return strRedisLastDateTime;
	}
}

int ThreadOneHour::CompDateTime(const QString& strDateTime1, const QString& strDateTime2) const
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

bool ThreadOneHour::IsItInTime(const QString& strDateTime, const QString& strDateTimeStart, const QString& strDateTimeEnd) const
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