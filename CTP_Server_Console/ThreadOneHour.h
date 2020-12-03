#pragma once

#include <QThread>
#include "../../PhdRedis.h"
#include <QTimer>

/***********************************************
   >   Class Name: ThreadOneHour
   >     Describe: 1小时线程
   >       Author: peihaodong
   > Created Time: 2020年11月30日
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/

/*
redis数据存储类型
	0：合约代码 1：日期 2：开始时间'[' 3：结束时间')' 4：开盘价 5：最高价 6：最低价 7：收盘价 8：成交量
*/
class ThreadOneHour : public QThread
{
	Q_OBJECT

public:
	ThreadOneHour();
	~ThreadOneHour();

	//	关闭线程，然后调用quit()和wait()函数
	void CloseThread();

protected:
	//	线程函数，开始线程，用start()间接调用
	virtual void run() override;

private slots:
	//	定时器对应槽函数
	void slotTimerOneHour();

private:

	//	加工csv文件中1小时的数据
	void ProcessCsvDataOneHour(const QString& strFilePath, const QString& strRedisLastDateTime,
		std::vector<QString>& vecProcessData) const;

	//	1小时数据添加到redis数据库
	bool AppendToRedisOneHour(const QString& strHydm, const std::vector<QString>& vecProcessData);

private:

	//	读取redis数据库中该合约代码最后一条数据的结束时间
	//（如果redis中没有该合约代码的数据，返回""）
	QString GetRedisLastDateTimeOneHour(const QString& strHydm);

	//	比较两个日期时间的大小，如果strDateTime1比strDateTime2大，返回1；如果小，返回-1,；相等，返回0
	int CompDateTime(const QString& strDateTime1, const QString& strDateTime2) const;

	//	strTime时间是否在strTime1时间和strTime2时间之间
	bool IsItInTime(const QString& strDateTime, const QString& strDateTimeStart, const QString& strDateTimeEnd) const;

private:
	QTimer* m_pTimerOneHour;		//定时器实例
	PhdRedis m_redis;			//redis数据库对象
};