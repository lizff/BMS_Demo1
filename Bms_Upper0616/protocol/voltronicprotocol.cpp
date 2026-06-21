#include "voltronicprotocol.h"
#include <QtGlobal>
#include <QDebug>

VoltronicProtocol::VoltronicProtocol(
        QObject *parent)
    : ProtocolBase(parent)
{
    m_lastRequestAddr = 0;
    m_lastRequestCount = 0;
    m_waitingResponse = false;

    // ========== 新增：初始化超时检测 ==========
    m_timeoutCount = 0;
    m_hasValidData = false;
    m_lastReceiveTime = QDateTime::currentDateTime();
    m_lastCommState = false;  // 初始为异常

      // 创建心跳定时器
      m_heartbeatTimer = new QTimer(this);
      m_heartbeatTimer->setInterval(HEARTBEAT_INTERVAL);
      connect(m_heartbeatTimer, &QTimer::timeout,
              this, &VoltronicProtocol::onHeartbeatTimeout);
      m_heartbeatTimer->start();

//      // 初始状态：通信异常（还没收到数据）
//      emit communicationStateChanged(false);
}

// CRC16计算（Modbus标准）
quint16 VoltronicProtocol::calcModbusCRC(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i)
    {
        crc ^= (quint8)data[i];
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void VoltronicProtocol::parseResponseFrame(const QByteArray &frame)
{
    qDebug() << "解析应答帧:" << frame.toHex(' ');

    // 1. CRC校验
    if (frame.size() < 6)
    {
        qDebug() << "应答帧长度错误";
        return;
    }

    // 【修正】Modbus CRC是低字节在前、高字节在后，所以：
    // frame[frame.size()-2] 是低字节，frame[frame.size()-1] 是高字节
    quint8 crcLow = (quint8)frame[frame.size()-2];
    quint8 crcHigh = (quint8)frame[frame.size()-1];
    quint16 recvCrc = (crcHigh << 8) | crcLow;

    // 计算CRC时，要去掉最后2字节
    quint16 calcCrc = calcModbusCRC(frame.left(frame.size()-2));

    if (recvCrc != calcCrc)
    {
        qDebug() << "应答帧CRC校验失败，recv=" << QString::number(recvCrc, 16)
                 << " calc=" << QString::number(calcCrc, 16);
        return;
    }

    quint8 funcCode = frame[1];
    if (funcCode != 0x03)
    {
        qDebug() << "不支持的功能码：" << funcCode;
        return;
    }

    // 2. 提取数据域
    quint16 regCount = (quint8)frame[2] << 8 | (quint8)frame[3];
    int dataByteLen = regCount * 2;
    QByteArray dataBuf = frame.mid(4, dataByteLen);

    if (dataBuf.size() != dataByteLen)
    {
        qDebug() << "数据域长度错误";
        return;
    }

    // 3. 必须有对应的请求帧才能解析
    if (!m_waitingResponse)
    {
        qDebug() << "没有对应的请求帧，无法解析应答数据";
        return;
    }

    // ========== 新增：收到有效应答，更新接收时间 ==========
       m_lastReceiveTime = QDateTime::currentDateTime();

       // 如果之前没有有效数据，现在有了
       if (!m_hasValidData) {
           m_hasValidData = true;
           m_timeoutCount = 0;
           m_lastCommState = true;  // ← 加上这行
           emit communicationStateChanged(true);
           qDebug() << "【通信状态】首次收到有效数据 → 正常";
       }

    // 4. 初始化数据结构体
    BmsData bmsData{};
    memset(&bmsData, 0, sizeof(BmsData));
    bmsData.communicationOk = true;

    // 5. 根据请求的起始地址，解析对应数据
    switch (m_lastRequestAddr)
    {
        case 0x0030: // 充电电流
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.chargeCurrent = raw / 10.0f;
            qDebug() << "充电电流：" << bmsData.chargeCurrent << " A";
            break;
        }
        case 0x0031: // 放电电流
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.dischargeCurrent = raw / 10.0f;
            qDebug() << "放电电流：" << bmsData.dischargeCurrent << " A";
            break;
        }
        case 0x0032: // 总电压
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.totalVoltage = raw / 10.0f;
            qDebug() << "总电压：" << bmsData.totalVoltage << " V";
            break;
        }
        case 0x0033: // SOC
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.soc = static_cast<float>(raw);
            qDebug() << "SOC：" << bmsData.soc << " %";
            break;
        }
        case 0x0034: // 模组总容量
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.totalCapacity = raw / 1000.0f;
            qDebug() << "模组总容量：" << bmsData.totalCapacity << " Ah";
            break;
        }
        case 0x0070: // 充电限制电压
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.voltageLimChr = raw / 10.0f;
            qDebug() << "充电限制电压" << bmsData.voltageLimChr << " V";
            break;
        }
        case 0x0071: // 放电限制电压
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.voltageLimDischr = raw / 10.0f;
            qDebug() << "放电限制电压" << bmsData.voltageLimDischr << " V";
            break;
        }
        case 0x0072: // 充电限制电流
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.currentLimchr = raw / 10.0f;
            qDebug() << "充电限制电流" << bmsData.currentLimchr << " A";
            break;
        }
        case 0x0073: // 放电限制电流
        {
            quint16 raw = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];
            bmsData.currentLimDischr = raw / 10.0f;
            qDebug() << "放电限制电流" << bmsData.currentLimDischr << "A";
            break;
        }
        case 0x0074: // 强充禁放状态
        {
        // 1. 读取原始寄存器值（大端，2字节）
                quint16 rawStatus = (quint8)dataBuf[0] << 8 | (quint8)dataBuf[1];

                // 2. 解析每个Bit的状态
                // Bit7: Charge enable
                bmsData.chargeEnable = (rawStatus & (1 << 7)) != 0;
                // Bit6: Discharge enable
                bmsData.dischargeEnable = (rawStatus & (1 << 6)) != 0;
                // Bit5: Charge immediately
                bmsData.chargeImmediately = (rawStatus & (1 << 5)) != 0;
                // Bit4: Charge immediately2
                bmsData.chargeImmediately2 = (rawStatus & (1 << 4)) != 0;
                // Bit3: Full charge request
                bmsData.fullChargeRequest = (rawStatus & (1 << 3)) != 0;

                // 3. 打印日志，方便调试
                qDebug() << "充放电状态解析："
                         << "充电使能=" << (bmsData.chargeEnable ? "允许" : "停止")
                         << " 放电使能=" << (bmsData.dischargeEnable ? "允许" : "停止")
                         << " 立即充电1=" << (bmsData.chargeImmediately ? "请求" : "无")
                         << " 立即充电2=" << (bmsData.chargeImmediately2 ? "请求" : "无")
                         << " 满充请求=" << (bmsData.fullChargeRequest ? "请求" : "无");

                break;
        }
        default:
            qDebug() << "未知请求地址：0x" << QString::number(m_lastRequestAddr, 16);
            break;
    }

//    qDebug() << "【解析完成，发射信号】总电压:" << bmsData.totalVoltage;
    // 6. 发送数据给UI
    emit bmsDataReady(bmsData);

    // 7. 标记应答已处理，等待下一次请求
    m_waitingResponse = false;
}

// 解析请求帧（仅处理主机发的请求）
bool VoltronicProtocol::parseRequestFrame(const QByteArray &frame)
{
    qDebug() << "解析请求帧:" << frame.toHex(' ');

    // 1. 长度必须是8字节
    if (frame.size() != 8)
    {
        return false;
    }

    // 2. CRC校验
    quint16 recvCrc = (quint8)frame[frame.size()-1] << 8 | (quint8)frame[frame.size()-2];
    quint16 calcCrc = calcModbusCRC(frame.left(frame.size()-2));
    if (recvCrc != calcCrc)
    {
        qDebug() << "请求帧CRC校验失败";
        return false;
    }

    // 3. 提取请求信息
    quint16 startAddr = (quint8)frame[2] << 8 | (quint8)frame[3];
    quint16 regCount = (quint8)frame[4] << 8 | (quint8)frame[5];

    m_lastRequestAddr = startAddr;
    m_lastRequestCount = regCount;
    m_waitingResponse = true;

    qDebug() << "请求帧解析成功：起始地址=0x" << QString::number(startAddr, 16)
             << " 寄存器数量=" << regCount;

    return true;
}

// 数据输入主逻辑
void VoltronicProtocol::inputData(const QByteArray &data)
{
    m_buffer.append(data);
    qDebug() << "【协议收到数据】" << data.toHex(' ');
    qDebug() << "[缓存]:" << m_buffer.toHex(' ');

    while (true)
    {
        // --------------------------
        // 只处理标准 Modbus 请求帧：长度必须是8字节，且严格符合请求帧格式
        // --------------------------
        if (m_buffer.size() >= 8)
        {
            QByteArray frame = m_buffer.left(8);
            quint8 funcCode = frame[1];

            // 只对功能码0x03、长度8字节的帧，做请求帧校验
            if (funcCode == 0x03)
            {
                // 【核心区分点】：Modbus请求帧的第2-3字节是「地址」，第4-5字节是「数量」
                // 而应答帧的第2-3字节是「数据长度」，且后面跟着数据域
                // 所以我们直接用帧的内容特征来判断：
                // 如果第4-5字节是「寄存器数量」，且帧尾是CRC，就是请求帧
                quint16 regCount = (quint8)frame[4] << 8 | (quint8)frame[5];
                // 寄存器数量必须在1~125之间（Modbus标准限制）
                if (regCount >= 1 && regCount <= 125)
                {
                    if (parseRequestFrame(frame))
                    {
                        m_buffer.remove(0, 8);
                        continue;
                    }
                }
            }
        }

        // --------------------------
        // 处理标准 Modbus 应答帧
        // --------------------------
        if (m_buffer.size() < 6)
            break;

        quint8 funcCode = m_buffer[1];
        if (funcCode != 0x03)
        {
            m_buffer.remove(0, 1);
            continue;
        }

        quint16 regCount = (quint8)m_buffer[2] << 8 | (quint8)m_buffer[3];
        int dataByteLen = regCount * 2;
        int totalFrameLen = 1 + 1 + 2 + dataByteLen + 2; // 这里必须是 dataByteLen，不是 regCount

        if (m_buffer.size() < totalFrameLen)
            break;

        QByteArray respFrame = m_buffer.left(totalFrameLen);
        parseResponseFrame(respFrame);
        m_buffer.remove(0, totalFrameLen);
    }
}

void VoltronicProtocol::onHeartbeatTimeout()
{
    if (!m_hasValidData) {
        m_timeoutCount++;
        if (m_timeoutCount >= MAX_TIMEOUT_COUNT) {
            if (m_lastCommState != false) {
                m_lastCommState = false;
                emit communicationStateChanged(false);
                qDebug() << "【通信状态】从未收到数据 → 异常";
            }
        }
         emit timeoutInfoUpdated(m_timeoutCount, m_lastReceiveTime.secsTo(QDateTime::currentDateTime()));
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 elapsed = m_lastReceiveTime.secsTo(now);

    if (elapsed > 2) {
        m_timeoutCount++;
        qDebug() << "【心跳】超时计数:" << m_timeoutCount << " 距上次数据:" << elapsed << "秒";

        if (m_timeoutCount >= MAX_TIMEOUT_COUNT) {
            if (m_lastCommState != false) {
                m_lastCommState = false;
                emit communicationStateChanged(false);
                qDebug() << "【通信状态】连续超时 → 异常";
            }
        }
    } else {
        if (m_timeoutCount > 0) {
            m_timeoutCount = 0;
            if (m_lastCommState != true) {
                m_lastCommState = true;
                emit communicationStateChanged(true);
                qDebug() << "【通信状态】恢复 → 正常";
            }
        }
    }

}
