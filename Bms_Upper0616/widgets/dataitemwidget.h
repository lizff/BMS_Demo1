#ifndef DATAITEMWIDGET_H
#define DATAITEMWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>

class DataItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DataItemWidget(QWidget *parent = nullptr);

    // 对外接口：设置名称、数值、单位
    void setItemName(const QString& name);
    void setItemValue(const QString& val);
    void setItemValue(double val, int precision = 2);
    void setItemUnit(const QString& unit);

private:
    QHBoxLayout* m_layout;
    QLabel* m_labName;    // 数据名称：SOH、循环次数
    QLabel* m_labValue;   // 实时数值
    QLabel* m_labUnit;    // 单位：V、A、%、℃
};

#endif // DATAITEMWIDGET_H
