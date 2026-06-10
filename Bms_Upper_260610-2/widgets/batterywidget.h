#ifndef BATTERYWIDGET_H
#define BATTERYWIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QTimer>

class BatteryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BatteryWidget(QWidget *parent = nullptr);

    void setValue(int value);

protected:
    void paintEvent(QPaintEvent *event);

private:
    int m_value;

    int m_displayValue;

    QTimer *timer;
};

#endif // BATTERYWIDGET_H
