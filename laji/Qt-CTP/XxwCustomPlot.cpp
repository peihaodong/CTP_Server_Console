#include "XxwCustomPlot.h"
#include <winuser.h>
#include <windef.h>

XxwCustomPlot::XxwCustomPlot(QWidget *parent)
    :QCustomPlot(parent)
    ,m_isShowTracer(false)
    ,m_xTracer(Q_NULLPTR)
    ,m_yTracer(Q_NULLPTR)
    ,m_dataTracers(QList<XxwTracer *>())
    ,m_lineTracer(Q_NULLPTR)
{
	keyUp = false;        // 初始化变量
	keyDown = false;
	keyLeft = false;
	keyRight = false;
}

void XxwCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    QCustomPlot::mouseMoveEvent(event);

	if (m_isShowTracer)
	{
		//当前鼠标位置（像素坐标）
		int x_pos = event->pos().x();
		int y_pos = event->pos().y();

		//像素坐标转成实际的x,y轴的坐标
		float x_val = this->xAxis->pixelToCoord(x_pos);
		float y_val = this->yAxis->pixelToCoord(y_pos);

		if (Q_NULLPTR == m_xTracer)
			m_xTracer = new XxwTracer(this, XxwTracer::XAxisTracer);//x轴
		m_xTracer->updatePosition(x_val, y_val);

		if (Q_NULLPTR == m_yTracer)
			m_yTracer = new XxwTracer(this, XxwTracer::YAxisTracer);//y轴
		m_yTracer->updatePosition(x_val, y_val);

// 		int nTracerCount = m_dataTracers.count();
// 		int nGraphCount = graphCount();
// 		if (nTracerCount < nGraphCount)
// 		{
// 			for (int i = nTracerCount; i < nGraphCount; ++i)
// 			{
// 				XxwTracer *tracer = new XxwTracer(this, XxwTracer::DataTracer);
// 				m_dataTracers.append(tracer);
// 			}
// 		}
// 		else if (nTracerCount > nGraphCount)
// 		{
// 			for (int i = nGraphCount; i < nTracerCount; ++i)
// 			{
// 				XxwTracer *tracer = m_dataTracers[i];
// 				if (tracer)
// 				{
// 					tracer->setVisible(false);
// 				}
// 			}
// 		}
// 		for (int i = 0; i < nGraphCount; ++i)
// 		{
// 			XxwTracer *tracer = m_dataTracers[i];
// 			if (!tracer)
// 				tracer = new XxwTracer(this, XxwTracer::DataTracer);
// 			tracer->setVisible(true);
// 			tracer->setPen(this->graph(i)->pen());
// 			tracer->setBrush(Qt::NoBrush);
// 			tracer->setLabelPen(this->graph(i)->pen());
// 			auto iter = this->graph(i)->data()->findBegin(x_val);
// 			double value = iter->mainValue();
// 			//            double value = this->graph(i)->data()->findBegin(x_val)->value;
// 			tracer->updatePosition(x_val, value);
// 		}

		if (Q_NULLPTR == m_lineTracer)
			m_lineTracer = new XxwTraceLine(this, XxwTraceLine::Both);//直线
		m_lineTracer->updatePosition(x_val, y_val);

		this->replot();//曲线重绘
	}
}

bool XxwCustomPlot::eventFilter(QCustomPlot *obj, QEvent *e, QKeyEvent *ek)
{
	if (obj == m_plot)
	{
		if (ek->key() == Qt::Key_Up)
		{
// 			e->type() == QEvent::MouseButtonPress;
// 			QMouseEvent *mouseEvent = (QMouseEvent *)ek;
// 			mouseEvent->buttons()&Qt::LeftButton;
// 			mouseEvent->MouseMove;
// 			mouseEvent->Wheel;
			POINT p;
			while (1) 
			{
				GetCursorPos(&p);//获取鼠标坐标 
				SetCursorPos(p.x + 3, p.y);//更改鼠标坐标 
			/*	Sleep(1);//控制移动时间间隔 */
			}
		}
// 		if (e->type() == QEvent::MouseButtonPress)
// 		{
// 			QMouseEvent * ev = static_cast<QMouseEvent *>(e);
// 			QString str = QString("事件过滤器中：：鼠标按下了 x = %1   y = %2  globalX = %3 globalY = %4 ").arg(ev->x()).arg(ev->y()).arg(ev->globalX()).arg(ev->globalY());
// 			qDebug() << str;
// 			return true; //true代表用户自己处理这个事件，不向下分发
// 		}
	}

	//其他默认处理
	return QWidget::eventFilter(obj, e);
}

void XxwCustomPlot::keyPressEvent(QKeyEvent *event)
{
	if (!m_plot)
		return QWidget::keyPressEvent(event);

	if (event->key() == Qt::Key_Up)
	{
		
	}
	else if (event->key() == Qt::Key_Down)
	{
		
	}
	else if (event->key() == Qt::Key_Left)
	{
		POINT p;
		GetCursorPos(&p);//获取鼠标坐标 
		SetCursorPos(p.x - 3, p.y);//更改鼠标坐标 
	}
	else if (event->key() == Qt::Key_Right)
	{
		POINT p;
		GetCursorPos(&p);//获取鼠标坐标 
		SetCursorPos(p.x + 3, p.y);//更改鼠标坐标 
	}

	return QWidget::keyPressEvent(event);
}

void XxwCustomPlot::keyPressEventUp(QKeyEvent *event)
{
	if (!m_plot)
		return;
	if (event->key() == Qt::Key_Up)
	{
// 		if (event->isAutoRepeat())
// 			return;                          // 按键重复时不做处理
		keyUp = true;                        // 标记向上方向键已经按下
	}
}

void XxwCustomPlot::keyPressEventDown(QKeyEvent *event)
{

}

void XxwCustomPlot::keyPressEventLeft(QKeyEvent *event)
{

}

void XxwCustomPlot::keyPressEventRight(QKeyEvent *event)
{

}

void XxwCustomPlot::keyReleaseEventUp(QKeyEvent *event)
{
	if (!m_plot)
		return;
	if (event->key() != Qt::Key_Up)
		return;
	// 允许用户用鼠标拖动轴范围，用鼠标滚轮扩大和点击选择图形:
	m_plot->setInteractions(QCP::iRangeDrag | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);
}

void XxwCustomPlot::keyReleaseEventDown(QKeyEvent *event)
{

}

void XxwCustomPlot::keyReleaseEventLeft(QKeyEvent *event)
{

}

void XxwCustomPlot::keyReleaseEventRight(QKeyEvent *event)
{

}


