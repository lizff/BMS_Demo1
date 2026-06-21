#include "databus.h"

DataBus* DataBus::m_instance = nullptr;

DataBus::DataBus(QObject *parent)
    : QObject(parent)
{

}

DataBus *DataBus::instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new DataBus;
    }

    return m_instance;
}
