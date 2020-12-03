#pragma once

#include <QThread>
#include "../../PhdRedis.h"
#include <QTimer>

/***********************************************
   >   Class Name: ThreadOneDay
   >     Describe: 1天线程
   >       Author: peihaodong
   > Created Time: 2020年11月30日
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/

/*
redis数据存储类型
	0：合约代码 1：开始日期'[' 2：结束日期')' 3：开盘价 4：最高价 5：最低价 6：收盘价 7：成交量
*/
class ThreadOneDay : public QThread
{
	Q_OBJECT

public:
	ThreadOneDay();
	~ThreadOneDay();

	//	关闭线程，然后调用quit()和wait()函数
	void CloseThread();

protected:
	//	线程函数，开始线程，用start()间接调用
	virtual void run() override;

private slots:
	//	定时器对应槽函数
	void slotTimerOneDay();

private:

	//	加工csv文件中1天的数据
	void ProcessCsvDataOneDay(const QString& strFilePath, const QString& strLastDay, const QString& strCurDay, QString& strProcessData) const;

	//	1天数据添加到redis数据库
	bool AppendToRedisOneDay(const QString& strHydm, const QString& strProcessData);

private:
	QTimer* m_pTimerOneDay;		//定时器实例
	PhdRedis m_redis;			//redis数据库对象
};