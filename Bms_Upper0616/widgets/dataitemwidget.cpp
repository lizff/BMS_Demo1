#include "dataitemwidget.h"
#include <QFont>

DataItemWidget::DataItemWidget(QWidget *parent)
    : QWidget(parent)
{
    // 水平布局：名称 - 数值 - 单位
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(4,2,4,2);
    m_layout->setSpacing(8);

    m_labName = new QLabel(this);
    m_labValue = new QLabel("0.00", this);
    m_labUnit = new QLabel(this);

    // 样式微调
    QFont valFont = m_labValue->font();
    valFont.setPointSize(10);
    valFont.setBold(true);
    m_labValue->setFont(valFont);

    m_layout->addWidget(m_labName);
    m_layout->addStretch(); // 自动拉伸，名称居左、数值靠右
    m_layout->addWidget(m_labValue);
    m_layout->addWidget(m_labUnit);
}

void DataItemWidget::setItemName(const QString &name)
{
    m_labName->setText(name);
}

void DataItemWidget::setItemValue(const QString &val)
{
    m_labValue->setText(val);
}

void DataItemWidget::setItemValue(double val, int precision)
{
        m_labValue->setText(QString::number(val, 'f', precision));
}

void DataItemWidget::setItemUnit(const QString &unit)
{
    m_labUnit->setText(unit);
}
