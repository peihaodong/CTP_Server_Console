#pragma once

#include <QThread>
#include "../../PhdRedis.h"
#include <QTimer>

/***********************************************
   >   Class Name: ThreadOneDay
   >     Describe: 1���߳�
   >       Author: peihaodong
   > Created Time: 2020��11��30��
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/

/*
redis���ݴ洢����
	0����Լ���� 1����ʼ����'[' 2����������')' 3�����̼� 4����߼� 5����ͼ� 6�����̼� 7���ɽ���
*/
class ThreadOneDay : public QThread
{
	Q_OBJECT

public:
	ThreadOneDay();
	~ThreadOneDay();

	//	�ر��̣߳�Ȼ�����quit()��wait()����
	void CloseThread();

protected:
	//	�̺߳�������ʼ�̣߳���start()��ӵ���
	virtual void run() override;

private slots:
	//	��ʱ����Ӧ�ۺ���
	void slotTimerOneDay();

private:

	//	�ӹ�csv�ļ���1�������
	void ProcessCsvDataOneDay(const QString& strFilePath, const QString& strLastDay, const QString& strCurDay, QString& strProcessData) const;

	//	1��������ӵ�redis���ݿ�
	bool AppendToRedisOneDay(const QString& strHydm, const QString& strProcessData);

private:
	QTimer* m_pTimerOneDay;		//��ʱ��ʵ��
	PhdRedis m_redis;			//redis���ݿ����
};