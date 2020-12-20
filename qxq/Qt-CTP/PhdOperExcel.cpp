#include "PhdOperExcel.h"
#include <fstream>

bool PhdOperExcel::OpenExcel(const QString& strPath)
{
	std::ifstream inFile;
	inFile.open(strPath.toStdString()); //
	
	char* str = new char[10];
	std::memset(str,0,10);
	inFile >> str;

	inFile.close();

	return true;
}
