#include "QtCTP.h"
#include <QtWidgets/QApplication>

// 链接库
#pragma comment (lib, "thostmduserapi_se.lib")//也可用 附加依赖项 加载
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
