#pragma once

#include <QObject>

class QtCustomTraderSpi : public QObject
{
	Q_OBJECT

public:
	QtCustomTraderSpi(QObject *parent);
	~QtCustomTraderSpi();
};
