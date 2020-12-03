#pragma once
#include <map>
#include <vector>
#include <QString>

/***********************************************
   >   Class Name: CCsvData
   >     Describe: 该类用来存储一段时间内的csv文件数据
   >       Author: peihaodong
   > Created Time: 2020年11月30日
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/
class CCsvData
{
public:
	//添加数据
	void AppendData(const QString& strDateTimeStart,const QString& strDateTimeEnd,
		const QString& strRowData);

	//得到存储的数据
	inline std::map<QString, std::vector<QString> > GetCsvData() const {
		return m_mapCsvData;
	}

private:
	std::map<QString,std::vector<QString> > m_mapCsvData;
};