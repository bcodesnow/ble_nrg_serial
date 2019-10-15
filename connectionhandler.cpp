// this class is a walking dead..

#include "connectionhandler.h"
#include <QtBluetooth/qtbluetooth-config.h>

ConnectionHandler::ConnectionHandler(QObject *parent) : QObject(parent)
{
    qDebug()<<"Connection Handler -> hostModeStateChanged of QBluetoothLocalDevice";
    connect(&m_localDevice, &QBluetoothLocalDevice::hostModeStateChanged,
            this, &ConnectionHandler::hostModeChanged);

    qDebug()<<"Fetching Adapter List";


    m_adapterList = QBluetoothLocalDevice::allDevices(); // todo android us old way
    if ( m_adapterList.size() != 0 )
    {
        qDebug()<<m_adapterList.at(0).address();
        qDebug()<<m_adapterList.at(0).name();
        qDebug()<<"Adapter List Size: "<< m_adapterList.size();
    }
}

bool ConnectionHandler::alive() const
{
    return m_localDevice.isValid() && m_localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff;
}

bool ConnectionHandler::requiresAddressType() const
{
#if QT_CONFIG(bluez)
    return true;
#else
    return false;
#endif
}

QString ConnectionHandler::name() const
{
    return m_localDevice.name();
}

QString ConnectionHandler::address() const
{
    return m_localDevice.address().toString();
}

void ConnectionHandler::hostModeChanged(QBluetoothLocalDevice::HostMode /*mode*/)
{
    emit deviceChanged();
}

//QVariant ConnectionHandler::adapters()
//{
//    return QVariant::fromValue(m_adapters);
//}

//QList<QBluetoothAddress> ConnectionHandler::getAdaptersAddr()
//{
//    QList<QBluetoothAddress> tmplist;
//    tmplist.clear();
//    for (int i=0;i<m_adapters.size();i++) {
//        tmplist.append(((AdapterInfo*)m_adapters.at(i))->getAddress());
//    }
//    return tmplist;
//}

//void ConnectionHandler::addAdapter(QString name, QBluetoothAddress addr)
//{
//    qDebug()<<"BT-Adapter added:"<<name<<addr;
//    m_adapters.append(new AdapterInfo(name, addr));
//}

//void ConnectionHandler::scanAdapters()
//{
//    m_adapters.clear();
//    QList<QBluetoothHostInfo> adapterList = QBluetoothLocalDevice::allDevices();
//    for (int i=0;i<adapterList.size();i++) {
//        addAdapter(adapterList.at(i).name(),adapterList.at(i).address());
//    }
//}

//void ConnectionHandler::initBtAdapters(QBluetoothAddress &leftaddr, QBluetoothAddress &rightaddr)
//{
//    scanAdapters();
//    QList<QBluetoothAddress> adapterAddrList = this->getAdaptersAddr();

//    int adapterCount = adapterAddrList.size();
//    if (adapterCount == 1) {
//        // set address to both handles
//        leftaddr = rightaddr = adapterAddrList.at(0);
//    }
//    else if (adapterCount > 1) {
//        // set first two addresses to handles
//        leftaddr = adapterAddrList.at(0);
//        rightaddr = adapterAddrList.at(1);
//    }
//    else {
//        // none adapters found, which means QBluetoothLocalDevice is not supported on OS
//    }
//}


