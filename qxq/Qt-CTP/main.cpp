#include "QtCTP.h"
#include <QtWidgets/QApplication>

// ���ӿ�
#pragma comment (lib, "thostmduserapi_se.lib")//Ҳ���� ���������� ����
//#pragma comment (lib, "thosttraderapi_se.lib")

int main(int argc, char *argv[])
{
   /* QApplication a(argc, argv);*/

	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc,argv);

	

    QtCTP w;
    w.show();
    return a.exec();
}
