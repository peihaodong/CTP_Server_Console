#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <QObject>
#include <qevent.h>
#include "ui_QtChart.h"
#include "qcustomplot.h"
#include <QKeyEvent>

 
class KeyEvent : public QCustomPlot
{
	Q_OBJECT

public:
	KeyEvent(QCustomPlot *_plot, QWidget *parent = 0);
	~KeyEvent();

private:
/*	Ui::KeyEvent *ui;*/
	bool keyUp;           // ���Ϸ�������µı�־
	bool keyDown;         // ���·�������µı�־
	bool keyLeft;         // ����������µı�־
	bool keyRight;        // ���ҷ�������µı�־
	bool move;            // �Ƿ������һ���ƶ�

protected:
	void keyPressEventUp(QKeyEvent *event);
	void keyPressEventDown(QKeyEvent *event);
	void keyPressEventLeft(QKeyEvent *event);
	void keyPressEventRight(QKeyEvent *event);

	void keyReleaseEventUp(QKeyEvent *event);
	void keyReleaseEventDown(QKeyEvent *event);
	void keyReleaseEventLeft(QKeyEvent *event);
	void keyReleaseEventRight(QKeyEvent *event);

};
