#include "qmllistadapter.h"

QmlListAdapter::QmlListAdapter(QObject *parent) : QObject(parent)
{
    m_deviceInterfaces = new QList<DeviceInterface*>();
}

QList<DeviceInterface *> *QmlListAdapter::getList()
{
    return m_deviceInterfaces;
}

QVariant QmlListAdapter::interfaces()
{
    return QVariant::fromValue(m_deviceInterfaces);
}

//void QmlListAdapter::setList(QList<DeviceInterface *> *t)
//{
//    m_deviceInterfaces = t;
//}


//int QmlListAdapter::interfaceCount() const
//{
//    return m_deviceInterfaces->count();
//}

//DeviceInterface* QmlListAdapter::interface(int index) const
//{
//    return m_deviceInterfaces->at(index);
//}

//DeviceInterface* QmlListAdapter::interface(QQmlListProperty<DeviceInterface>* list, int i) {
//    return reinterpret_cast< QmlListAdapter* >(list->data)->interface(i);
//}

//int QmlListAdapter::interfaceCount(QQmlListProperty<DeviceInterface>* list) {
//    return reinterpret_cast< QmlListAdapter* >(list->data)->interfaceCount();
//}
