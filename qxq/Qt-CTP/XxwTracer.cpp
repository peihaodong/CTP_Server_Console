#include "XxwTracer.h"
#include <QColorDialog>

#pragma warning(disable:4996)

XxwTracer::XxwTracer(QCustomPlot *_plot, TracerType _type, QObject *parent)
    : QObject(parent),
      m_plot(_plot),
      m_type(_type)
{
    m_visible = true;
    m_tracer = Q_NULLPTR;// 跟踪的点
    m_label = Q_NULLPTR;// 显示的数值
    m_arrow = Q_NULLPTR;// 箭头
    if (m_plot)
    {
        QColor clrDefault(Qt::white);                        //颜色
        QBrush brushDefault(Qt::NoBrush);                    //画刷
        QPen penDefault(clrDefault);                         //线型
        //        penDefault.setBrush(brushDefault);
        penDefault.setWidthF(1);

        m_tracer = new QCPItemTracer(m_plot);                 //跟踪点
       /* m_tracer->setStyle(QCPItemTracer::tsCircle);*/
       /* m_tracer->setPen(penDefault);*/
		m_tracer->setStyle(QCPItemTracer::tsPlus);//游标样式：十字星、圆圈、方框等
		m_tracer->setPen(QPen(Qt::DashLine));//游标线型：虚线
        m_tracer->setBrush(brushDefault);

        m_label = new QCPItemText(m_plot);                     //显示的数值
        m_label->setLayer("overlay");
        m_label->setClipToAxisRect(false);
		//显示文本周围的黑色边框
        m_label->setPadding(QMargins(5, 5, 5, 5));
        m_label->setBrush(brushDefault);
        m_label->setPen(penDefault);//显示文本周围的   颜色 边框 

        m_label->position->setParentAnchor(m_tracer->position);
//      m_label->setFont(QFont("宋体", 8));
        m_label->setFont(QFont("Arial", 8));
        m_label->setColor(clrDefault);
        m_label->setText("");


        m_arrow = new QCPItemLine(m_plot);                       //箭头
        QPen  arrowPen(clrDefault, 1);                           //箭头颜色
        m_arrow->setPen(penDefault);                             //箭头画笔方式
        m_arrow->setLayer("overlay");
        m_arrow->setClipToAxisRect(false);                       //设置元件在整个QCustomPlot可见
        m_arrow->setHead(QCPLineEnding::esNone);           //设置头部为箭头形状

        switch (m_type)
        {
        case XAxisTracer:                                         //依附在x轴上显示x值
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);
            m_tracer->setSize(7);
            m_label->setPositionAlignment(Qt::AlignLeft | Qt::AlignBottom);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_arrow->end);
            m_arrow->start->setCoords(0, 0);//偏移量
            break;
        }
        case YAxisTracer:                                           //依附在y轴上显示y值
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptAxisRectRatio);
            m_tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
            m_tracer->setSize(7);
            m_label->setPositionAlignment(Qt::AlignRight | Qt::AlignHCenter);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_label->position);
            m_arrow->start->setCoords(0, 0);//偏移量
            break;
        }
        case DataTracer:                                             //在图中显示x,y值
        {
            m_tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            m_tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);
            m_tracer->setSize(5);

            m_label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            m_arrow->end->setParentAnchor(m_tracer->position);
            m_arrow->start->setParentAnchor(m_arrow->end);
            m_arrow->start->setCoords(0, 0);
            break;
        }
        default:
            break;
        }
        setVisible(false);
    }
}

XxwTracer::~XxwTracer()
{
    if(m_plot)
    {
        if (m_tracer)
            m_plot->removeItem(m_tracer);
        if (m_label)
            m_plot->removeItem(m_label);
        if (m_arrow)
            m_plot->removeItem(m_arrow);
    }
}

void XxwTracer::setPen(const QPen &pen)
{
    if(m_tracer)
        m_tracer->setPen(pen);
    if(m_arrow)
        m_arrow->setPen(pen);
}

void XxwTracer::setBrush(const QBrush &brush)
{
    if(m_tracer)
        m_tracer->setBrush(brush);
}

void XxwTracer::setLabelPen(const QPen &pen)
{
    if(m_label)
    {
        m_label->setPen(pen);
        m_label->setBrush(Qt::NoBrush);
        m_label->setColor(pen.color());
    }
}

void XxwTracer::setText(const QString &text)
{
    if(m_label)
        m_label->setText(text);
}

void XxwTracer::setVisible(bool vis)
{
    m_visible = vis;
    if(m_tracer)
        m_tracer->setVisible(m_visible);
    if(m_label)
        m_label->setVisible(m_visible);
    if(m_arrow)
        m_arrow->setVisible(m_visible);
}


void XxwTracer::mouseLeaveEvent(QMouseEvent *event)
{
	if (event->type() == QEvent::Leave)     //当鼠标离开当前页面时
	{
		m_plot->setVisible(false);                  //不显示游标事件
/*		void QCPItemTracer::draw(QCPPainter *painter);*/
	}
}

void XxwTracer::updatePosition(double xValue, double yValue)
{
    if (!m_visible)
    {
        setVisible(true);
        m_visible = true;
    }

    if (yValue > m_plot->yAxis->range().upper)
        yValue = m_plot->yAxis->range().upper;

    switch (m_type)
    {
    case XAxisTracer:
    {
        m_tracer->position->setCoords(xValue, 1);
        m_label->position->setCoords(0, 15);
        m_arrow->start->setCoords(0, 15);
        m_arrow->end->setCoords(0, 0);
        setText(QString::number(xValue));
        break;
    }
    case YAxisTracer:
    {
        m_tracer->position->setCoords(0, yValue);
        m_label->position->setCoords(-20, 0);
//      m_arrow->start->setCoords(20, 0);
//      m_arrow->end->setCoords(0, 0);
        setText(QString::number(yValue));
        break;
    }
    case DataTracer:
    {
        m_tracer->position->setCoords(xValue, yValue);
        m_label->position->setCoords(20, 20);
        setText(QString("x:%1,y:%2").arg(xValue).arg(yValue));
        break;
    }
    default:
        break;
    }
}

XxwTraceLine::XxwTraceLine(QCustomPlot *_plot, LineType _type, QObject *parent)
    : QObject(parent),
      m_type(_type),
      m_plot(_plot)
{
    m_lineV = Q_NULLPTR;
    m_lineH = Q_NULLPTR;
    initLine();
}

XxwTraceLine::~XxwTraceLine()
{
    if(m_plot)
    {
        if (m_lineV)
            m_plot->removeItem(m_lineV);
        if (m_lineH)
            m_plot->removeItem(m_lineH);
    }
}

void XxwTraceLine::initLine()
{
    if(m_plot)
    {
        QPen linesPen(Qt::white, 2, Qt::DashLine);

        if(VerticalLine == m_type || Both == m_type)
        {
            m_lineV = new QCPItemStraightLine(m_plot);//垂直线
            m_lineV->setLayer("overlay");
            m_lineV->setPen(linesPen);                //设置颜色
            m_lineV->setClipToAxisRect(true);         //设置控件可见
            m_lineV->point1->setCoords(0, 0);
            m_lineV->point2->setCoords(0, 0);
        }

        if(HorizonLine == m_type || Both == m_type)
        {
            m_lineH = new QCPItemStraightLine(m_plot);//水平线
            m_lineH->setLayer("overlay");
            m_lineH->setPen(linesPen);
            m_lineH->setClipToAxisRect(true);          //设置控件可见
            m_lineH->point1->setCoords(0, 0);
            m_lineH->point2->setCoords(0, 0);
        }
    }
}

void XxwTraceLine::updatePosition(double xValue, double yValue)
{
    if(VerticalLine == m_type || Both == m_type)
    {
        if(m_lineV)
        {
            m_lineV->point1->setCoords(xValue, m_plot->yAxis->range().lower);     //设置坐标
            m_lineV->point2->setCoords(xValue, m_plot->yAxis->range().upper);
        }
    }

    if(HorizonLine == m_type || Both == m_type)
    {
        if(m_lineH)
        {
            m_lineH->point1->setCoords(m_plot->xAxis->range().lower, yValue);
            m_lineH->point2->setCoords(m_plot->xAxis->range().upper, yValue);
        }
    }
}


