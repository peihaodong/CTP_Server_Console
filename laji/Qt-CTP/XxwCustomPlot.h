#ifndef XCUSTOMPLOT_H
#define XCUSTOMPLOT_H

#include "XxwTracer.h"
#include "qcustomplot.h"
#include <QObject>
#include <QList>

class XxwCustomPlot:public QCustomPlot
{
    Q_OBJECT

public:
    XxwCustomPlot(QWidget *parent = 0);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);
	//重写事件过滤器的事件
	bool eventFilter(QCustomPlot *, QEvent *, QKeyEvent *);

protected:
	virtual void keyPressEvent(QKeyEvent *event);

public:
	void keyPressEventUp(QKeyEvent *event);
	void keyPressEventDown(QKeyEvent *event);
	void keyPressEventLeft(QKeyEvent *event);
	void keyPressEventRight(QKeyEvent *event);

	void keyReleaseEventUp(QKeyEvent *event);
	void keyReleaseEventDown(QKeyEvent *event);
	void keyReleaseEventLeft(QKeyEvent *event);
	void keyReleaseEventRight(QKeyEvent *event);

public:
    ///
    /// \brief 设置是否显示鼠标追踪器
    /// \param show:是否显示
    ///
    void showTracer(bool show)
    {
        m_isShowTracer = show;
        if(m_xTracer)
            m_xTracer->setVisible(m_isShowTracer);
        foreach (XxwTracer *tracer, m_dataTracers)
        {
            if(tracer)
                tracer->setVisible(m_isShowTracer);
        }
        if(m_lineTracer)
            m_lineTracer->setVisible(m_isShowTracer);
    }

    ///
    /// \brief 是否显示鼠标追踪器
    /// \return
    ///
    bool isShowTracer(){return m_isShowTracer;};

private:
    bool m_isShowTracer;//是否显示追踪器（鼠标在图中移动，显示对应的值）
    XxwTracer *m_xTracer;//x轴
    XxwTracer *m_yTracer;//y轴
    QList<XxwTracer *> m_dataTracers;//
    XxwTraceLine  *m_lineTracer;//直线


private:
	/*	Ui::KeyEvent *ui;*/
	bool keyUp;           // 向上方向键按下的标志
	bool keyDown;         // 向下方向键按下的标志
	bool keyLeft;         // 向左方向键按下的标志
	bool keyRight;        // 向右方向键按下的标志
	bool move;            // 是否完成了一次移动


	QCustomPlot *m_plot;//图表

};

#endif // XCUSTOMPLOT_H
