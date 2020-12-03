#pragma once
#include <map>
#include <vector>
#include <QString>

/***********************************************
   >   Class Name: CCsvData
   >     Describe: ���������洢һ��ʱ���ڵ�csv�ļ�����
   >       Author: peihaodong
   > Created Time: 2020��11��30��
   >         Blog: https://blog.csdn.net/phd17621680432
   >           QQ: 841382590
**********************************************/
class CCsvData
{
public:
	//�������
	void AppendData(const QString& strDateTimeStart,const QString& strDateTimeEnd,
		const QString& strRowData);

	//�õ��洢������
	inline std::map<QString, std::vector<QString> > GetCsvData() const {
		return m_mapCsvData;
	}

private:
	std::map<QString,std::vector<QString> > m_mapCsvData;
};