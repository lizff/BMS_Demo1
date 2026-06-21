#include "statusindicatorwidget.h"
#include "statusindicatorwidget.h"

#include <QHBoxLayout>

StatusIndicatorWidget::StatusIndicatorWidget(
        const QString &text,
        QWidget *parent)
    : QWidget(parent)
{   
    m_state = false;
    m_color = Gray;

    m_blink = false;
    m_visibleState = false;

    m_led = new QFrame;

    m_led->setFixedSize(14,14);

    m_label = new QLabel(text);

    QHBoxLayout *layout =
            new QHBoxLayout(this);

    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(5);

    layout->addWidget(m_led);
    layout->addWidget(m_label);

    m_timer = new QTimer(this);

    connect(m_timer,
            &QTimer::timeout,
            this,
            &StatusIndicatorWidget::onBlinkTimeout);

    setState(false);
}

bool StatusIndicatorWidget::state() const
{
    return m_state;
}

void StatusIndicatorWidget::setText(
        const QString &text)
{
    m_label->setText(text);
}

void StatusIndicatorWidget::setBlink(bool enable)
{
    m_blink = enable;

    if(enable)
    {
        m_timer->start(500);
    }
    else
    {
        m_timer->stop();
        setState(m_state);
    }
}

void StatusIndicatorWidget::onBlinkTimeout()
{
    m_visibleState = !m_visibleState;

    if(m_visibleState)
    {
        setColor(m_color);
    }
    else
    {
        m_led->setStyleSheet(
            "background:#303030;"
            "border-radius:7px;"
            "border:1px solid #555555;");
    }
}

void StatusIndicatorWidget::setColor(
        LedColor color)
{
    m_color = color;

    switch(color)
    {
    case Green:

        m_led->setStyleSheet(
            "background:#00ff66;"
            "border-radius:7px;"
            "border:1px solid #00cc55;");
        break;

    case Red:

        m_led->setStyleSheet(
            "background:#ff3030;"
            "border-radius:7px;"
            "border:1px solid #cc2020;");
        break;

    default:

        m_led->setStyleSheet(
            "background:#555555;"
            "border-radius:7px;"
            "border:1px solid #888888;");
        break;
    }
}

void StatusIndicatorWidget::setState(bool state)
{
    m_state = state;

        // 如果处于闪烁模式，由定时器控制显示，不在这里设置
        if (m_blink) {
            return;
        }

        if (state) {
            // 亮起：使用当前保存的颜色 m_color（而不是硬编码 Green）
            setColor(m_color);
        } else {
            // 熄灭：灰色
            m_led->setStyleSheet(
                "background:#303030;"
                "border-radius:7px;"
                "border:1px solid #555555;");
        }
}

