#ifndef DATABUS_H
#define DATABUS_H

#include <QObject>
#include "../models/bmsdata.h"

class DataBus : public QObject
{
    Q_OBJECT

public:

    static DataBus* instance();

signals:

    // BMS数据更新
    void bmsDataUpdated(BmsData data);

private:

    explicit DataBus(QObject *parent = nullptr);

    static DataBus *m_instance;
};

#endif // DATABUS_H
