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
	bool keyUp;           // 向上方向键按下的标志
	bool keyDown;         // 向下方向键按下的标志
	bool keyLeft;         // 向左方向键按下的标志
	bool keyRight;        // 向右方向键按下的标志
	bool move;            // 是否完成了一次移动

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
