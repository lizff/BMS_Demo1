#include "batterywidget.h"

#include <QPainter>
#include <QLinearGradient>
#include <QFont>

BatteryWidget::BatteryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_value = 50;

    m_displayValue = 0;

    timer = new QTimer(this);

    connect(timer,&QTimer::timeout,this,[=](){

        if(m_displayValue < m_value)
            m_displayValue++;

        else if(m_displayValue > m_value)
            m_displayValue--;

        update();

    });
//    setFixedSize(500,90);
    setMinimumHeight(90);
}

void BatteryWidget::setValue(int value)
{
    m_value = value;

    if(m_value > 100)
        m_value = 100;

    if(m_value < 0)
        m_value = 0;

    timer->start(15);
}

void BatteryWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    // ================= 电池主体 =================

//    QRect batteryRect(20,25,width()-60,50);
    QRect batteryRect(20,25,width()-20,50);

    painter.setPen(QPen(QColor(120,120,120),3));

    QLinearGradient bodyGradient(
        batteryRect.topLeft(),
        batteryRect.bottomLeft()
    );

    bodyGradient.setColorAt(0,QColor(245,245,245));
    bodyGradient.setColorAt(0.5,QColor(220,220,220));
    bodyGradient.setColorAt(1,QColor(180,180,180));

    painter.setBrush(bodyGradient);

    painter.drawRoundedRect(batteryRect,10,10);

    // ================= 电池正极 =================

    QRect headRect(width()-35,38,12,24);

    painter.setBrush(QColor(180,150,60));

//    painter.drawRoundedRect(headRect,3,3);

    // ================= 电量区域 =================

    int fillWidth = (batteryRect.width()-8) * m_displayValue / 100;

    QRect fillRect(
        batteryRect.x()+3,
        batteryRect.y()+3,
        fillWidth,
        batteryRect.height()-6
    );

    QLinearGradient gradient(fillRect.topLeft(),fillRect.topRight());

    // 动态颜色
    if(m_displayValue > 20)
    {
        gradient.setColorAt(0,QColor(0,255,120));
        gradient.setColorAt(1,QColor(0,180,80));

    }
    else if(m_displayValue > 20)
    {
        gradient.setColorAt(0,QColor(255,220,0));
        gradient.setColorAt(1,QColor(255,170,0));
    }
//    else
//    {
//        gradient.setColorAt(0,QColor(255,80,80));
//        gradient.setColorAt(1,QColor(200,0,0));
//    }

    painter.setBrush(gradient);

    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(fillRect,8,8);

    // ================= 高光 =================

    QRect lightRect(
        fillRect.x(),
        fillRect.y(),
        fillRect.width(),
        fillRect.height()/2
    );

    QLinearGradient lightGradient(
        lightRect.topLeft(),
        lightRect.bottomLeft()
    );

    lightGradient.setColorAt(0,QColor(255,255,255,180));
    lightGradient.setColorAt(1,QColor(255,255,255,30));

    painter.setBrush(lightGradient);

    painter.drawRoundedRect(lightRect,8,8);

    // ================= 百分比文字 =================

    painter.setPen(Qt::black);

    QFont font;

    font.setPointSize(18);

    font.setBold(true);

    painter.setFont(font);

    painter.drawText(
        batteryRect,
        Qt::AlignCenter,
        QString("%1%").arg(m_displayValue)
    );
}
