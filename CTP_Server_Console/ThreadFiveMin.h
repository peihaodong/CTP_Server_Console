#pragma once

#include <QThread>
#include "../../PhdRedis.h"
#include <QTimer>

/***********************************************
   >   Class Name: ThreadFiveMin
   >     Describe: ����ӵ��߳�
   >       Author: peihaodong
   > Created Time: 2020��11��25��
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/

/*
redis���ݴ洢����
	0����Լ���� 1������ 2����ʼʱ��'[' 3������ʱ��')' 4�����̼� 5����߼� 6����ͼ� 7�����̼� 8���ɽ���
*/
class ThreadFiveMin : public QThread
{
	Q_OBJECT

public:
	ThreadFiveMin();
	~ThreadFiveMin();

	//	�ر��̣߳�Ȼ�����quit()��wait()����
	void CloseThread();

protected:
	//	�̺߳�������ʼ�̣߳���start()��ӵ���
	virtual void run() override;

private slots:
	//	��ʱ����Ӧ�ۺ���
	void slotTimerFiveMin();

private:
	//	�ӹ�csv�ļ�������ӵ�����
	void ProcessCsvDataFiveMin(const QString& strFilePath, const QString& strRedisLastDateTime,
		std::vector<QString>& vecProcessData) const;

	//	�����������ӵ�redis���ݿ�
	bool AppendToRedisFiveMin(const QString& strHydm, const std::vector<QString>& vecProcessData);

private:

	//	��ȡredis���ݿ��иú�Լ�������һ�����ݵĽ���ʱ��
	//�����redis��û�иú�Լ��������ݣ�����""��
	QString GetRedisLastDateTimeFiveMin(const QString& strHydm);

	//	�Ƚ���������ʱ��Ĵ�С�����strDateTime1��strDateTime2�󣬷���1�����С������-1,����ȣ�����0
	int CompDateTime(const QString& strDateTime1, const QString& strDateTime2) const;

	//	strTimeʱ���Ƿ���strTime1ʱ���strTime2ʱ��֮��
	bool IsItInTime(const QString& strDateTime, const QString& strDateTimeStart, const QString& strDateTimeEnd) const;

private:
	QTimer* m_pTimerFiveMin;		//��ʱ��ʵ��
	PhdRedis m_redis;		//redis���ݿ����
};
