#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QObject>
#include <QByteArray>

class ProtocolParser : public QObject
{
    Q_OBJECT

public:
    explicit ProtocolParser(QObject *parent = nullptr);

    // 输入原始数据
    void inputData(const QByteArray &data);

signals:

    // 输出完整数据帧
    void frameReceived(QByteArray frame);

private:

    QByteArray m_buffer;

};

#endif // PROTOCOLPARSER_H
