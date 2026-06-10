#include "serialworker.h"

SerialWorker::SerialWorker(QObject *parent)
    : QObject(parent)
{
    m_serial = new QSerialPort(this);

    // 接收信号
    connect(m_serial,
            &QSerialPort::readyRead,
            this,
            &SerialWorker::onReadyRead);
}

// 打开串口
void SerialWorker::openSerial(QString portName,
                              int baudRate,
                              int dataBits,
                              int parity,
                              int stopBits)
{
//    // 已打开
//    if(m_serial->isOpen())
//    {
//        m_serial->close();
//    }

    // 设置串口号
    m_serial->setPortName(portName);

    // 波特率
    m_serial->setBaudRate(baudRate);

    // 数据位
    m_serial->setDataBits((QSerialPort::DataBits)dataBits);

    // 停止位
    m_serial->setStopBits((QSerialPort::StopBits)stopBits);

    // 校验位
    m_serial->setParity(
                  (QSerialPort::Parity)parity);

    // 流控
    m_serial->setFlowControl(
                QSerialPort::NoFlowControl);

    // 打开
    bool ok =
            m_serial->open(
                QIODevice::ReadWrite);

    if(ok)
    {
        emit serialInfo(
                    "串口打开成功");
    }
    else
    {
        emit serialInfo(
                    "串口打开失败");
    }
}

// 关闭串口
void SerialWorker::closeSerialPort()
{
    if(m_serial->isOpen())
    {
        m_serial->close();

        emit serialInfo(
                    "串口已关闭");
    }
}

// 发送数据
void SerialWorker::sendData(QByteArray data)
{
    if(m_serial->isOpen())
    {
        m_serial->write(data);
    }
}

void SerialWorker::refreshPorts()
{
    QStringList portList;

    QList<QSerialPortInfo> ports =
            QSerialPortInfo::availablePorts();

    for(const QSerialPortInfo &info : ports)
    {
        portList.append(info.portName());
    }

    emit portsReady(portList);
}

int SerialWorker::isOpenSerial()
{
    return m_serial->isOpen();
}

// 接收数据
void SerialWorker::onReadyRead()
{
    QByteArray data =
            m_serial->readAll();

    emit dataReceived(data);
}
