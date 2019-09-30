#include "adapterinfo.h"

AdapterInfo::AdapterInfo(const QString name, const QBluetoothAddress addr)
{
    m_name = name;
    m_addr = addr;
}

QString AdapterInfo::getName()
{
    return m_name;
}

QBluetoothAddress AdapterInfo::getAddress()
{
    return m_addr;
}

QString AdapterInfo::getAddressStr()
{
    return m_addr.toString();
}
