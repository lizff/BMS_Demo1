#ifndef PROTOCOLBASE_H
#define PROTOCOLBASE_H

#include <QObject>
#include "../models/bmsdata.h"

class ProtocolBase : public QObject
{
    Q_OBJECT

public:

    explicit ProtocolBase(
            QObject *parent = nullptr);

    // 接收原始数据
    virtual void inputData(
            const QByteArray &data) = 0;

signals:
    // 输出解析后的标准数据
    void bmsDataReady(BmsData data);

    // ========== 通信状态变化信号 ==========
    void communicationStateChanged(bool isOk);
    void timeoutInfoUpdated(int count, qint64 elapsed);  // ← 新增
};

#endif // PROTOCOLBASE_H

