#include "protocolparser.h"

ProtocolParser::ProtocolParser(QObject *parent)
    : QObject(parent)
{

}

// 输入串口原始数据
void ProtocolParser::inputData(const QByteArray &data)
{
    // 放入缓存
    m_buffer.append(data);

    // 这里只做测试
    emit frameReceived(m_buffer);

    // 清空缓存
    m_buffer.clear();
}
