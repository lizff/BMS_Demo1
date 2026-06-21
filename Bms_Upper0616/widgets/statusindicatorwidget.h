#ifndef STATUSINDICATORWIDGET_H
#define STATUSINDICATORWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QTimer>

enum LedColor
{
    Gray,
    Green,
    Red
};

class StatusIndicatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusIndicatorWidget(
            const QString &text,
            QWidget *parent = nullptr);

    void setState(bool state);
    void setColor(LedColor color);

    bool state() const;

    void setBlink(bool enable);

    void setText(const QString &text);

private slots:
    void onBlinkTimeout();

private:
    QFrame *m_led;
    QLabel *m_label;

    bool m_state;
    bool m_blink;
    bool m_visibleState;

    QTimer *m_timer;

    LedColor m_color= Green;
};

#endif
