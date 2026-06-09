#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QStringList>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);

    // 获取可用串口
    QStringList getAvailablePorts();
    //打开串口
    bool openPort(QString portName);
    //关闭串口
    void closePort();
    //打开状态
    bool isOpen();

private:
    QSerialPort *m_serial;
};

#endif // SERIALPORTMANAGER_H
