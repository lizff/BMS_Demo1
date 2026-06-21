#ifndef VOLTRONICPROTOCOL_H
#define VOLTRONICPROTOCOL_H

#include "protocolbase.h"
#include "../models/bmsdata.h"
#include "protocolbase.h"
#include <QByteArray>
#include <QTimer>
#include <QDateTime>

class VoltronicProtocol
        : public ProtocolBase
{
    Q_OBJECT

public:

    explicit VoltronicProtocol(
            QObject *parent = nullptr);

    // 接收串口原始数据
    void inputData(
            const QByteArray &data) override;

    int getTimeoutCount() const { return m_timeoutCount; }
    qint64 getElapsedSeconds() const {
        return m_lastReceiveTime.secsTo(QDateTime::currentDateTime());
    }

private slots:
    // ========== 超时检测槽函数 ==========
    void onHeartbeatTimeout();

private:
    // 协议缓存区
    QByteArray m_buffer;

    void parseFrame(
            const QByteArray &frame);

    // Modbus-RTU CRC16 校验函数
    quint16 calcModbusCRC(const QByteArray &data);

    quint16 m_queryStartAddr;

    // 保存最近一次的Modbus请求帧信息
    void parseResponseFrame(const QByteArray &frame);
    bool parseRequestFrame(const QByteArray &frame);
    quint16 m_lastRequestAddr;  // 最近一次请求的寄存器地址
    quint16 m_lastRequestCount; // 最近一次请求的寄存器数量
    bool m_waitingResponse;     // 是否在等待应答

    // ========== 超时检测相关成员 ==========
    QTimer* m_heartbeatTimer;       // 心跳检测定时器
    QDateTime m_lastReceiveTime;    // 最后一次收到有效数据的时间
    int m_timeoutCount;             // 连续超时计数
    bool m_hasValidData;            // 是否收到过有效数据
    const int MAX_TIMEOUT_COUNT = 3; // 连续3次超时判定为通信异常
    const int HEARTBEAT_INTERVAL = 1000; // 每秒检测一次

    bool m_lastCommState;  // 上一次通信状态

//    int getTimeoutCount() const { return m_timeoutCount; }
//    qint64 getElapsedSeconds() const {
//        return m_lastReceiveTime.secsTo(QDateTime::currentDateTime());
//    }

};

#endif // VOLTRONICPROTOCOL_H
