#define _CRT_SECURE_NO_WARNINGS
#include "KeyEvent.h"

KeyEvent::KeyEvent(QCustomPlot *_plot, QWidget *parent)
	:QCustomPlot(parent)
{
/*	setFocus();                                          // 使主界面获得焦点*/

	
	move = false;
}

KeyEvent::~KeyEvent()
{
	delete m_plot;
}

void KeyEvent::keyPressEventUp(QKeyEvent *event)
{
	
	
}

void KeyEvent::keyPressEventDown(QKeyEvent *event)
{

}

void KeyEvent::keyPressEventLeft(QKeyEvent *event)
{

}

void KeyEvent::keyPressEventRight(QKeyEvent *event)
{

}

void KeyEvent::keyReleaseEventUp(QKeyEvent *event)
{
	
}

void KeyEvent::keyReleaseEventDown(QKeyEvent *event)
{

}

void KeyEvent::keyReleaseEventLeft(QKeyEvent *event)
{

}

void KeyEvent::keyReleaseEventRight(QKeyEvent *event)
{

}