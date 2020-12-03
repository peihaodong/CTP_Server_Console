#include "CCsvData.h"

void CCsvData::AppendData(const QString& strDateTimeStart, const QString& strDateTimeEnd, const QString& strRowData)
{
	QString strKey = strDateTimeStart + "-" + strDateTimeEnd;
	auto iter = m_mapCsvData.find(strKey);
	if (iter == std::end(m_mapCsvData))
	{
		std::vector<QString> vecRowData;
		vecRowData.push_back(strRowData);
		m_mapCsvData[strKey] = vecRowData;
	}
	else
	{
		iter->second.push_back(strRowData);
	}
}
