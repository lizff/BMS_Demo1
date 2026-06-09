#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>

#include <QSerialPort>

class SerialWorker : public QObject
{
    Q_OBJECT

public:
    explicit SerialWorker(QObject *parent = nullptr);

signals:

    // 收到数据
    void dataReceived(QByteArray data);

    // 状态信息
    void serialInfo(QString info);

public slots:

    // 打开串口
    void openSerial(QString portName,
                    int baudRate,
                    int dataBits,
                    int parity,
                    int stopBits);

    // 关闭串口
    void closeSerialPort();

    // 发送数据
    void sendData(QByteArray data);

private slots:

    // 接收数据
    void onReadyRead();

private:

    QSerialPort *m_serial;
};

#endif // SERIALWORKER_H
