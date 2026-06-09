#include "serialportmanager.h"

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
{
    m_serial = new QSerialPort(this);
}

QStringList SerialPortManager::getAvailablePorts()
{
    QStringList portList;

    QList<QSerialPortInfo> ports =
            QSerialPortInfo::availablePorts();

    for(const QSerialPortInfo &port : ports)
    {
        portList.append(port.portName());
    }

    return portList;
}

bool SerialPortManager::openPort(QString portName)
{
    // 如果已经打开
    if(m_serial->isOpen())
    {
        m_serial->close();
    }

    // 设置串口号
    m_serial->setPortName(portName);

    // 波特率
    m_serial->setBaudRate(QSerialPort::Baud115200);

    // 数据位
    m_serial->setDataBits(QSerialPort::Data8);

    // 停止位
    m_serial->setStopBits(QSerialPort::OneStop);

    // 校验位
    m_serial->setParity(QSerialPort::NoParity);

    // 流控
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    // 打开串口
    return m_serial->open(QIODevice::ReadWrite);
}

void SerialPortManager::closePort()
{
    if(m_serial->isOpen())
    {
        m_serial->close();
    }
}

bool SerialPortManager::isOpen()
{
    return m_serial->isOpen();
}
