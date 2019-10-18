#include "qmllistadapter.h"

QmlListAdapter::QmlListAdapter(QObject *parent): QAbstractListModel(parent)
{
    m_deviceInterfaces = new QList<DeviceInterface*>();
    //b = static_cast<QList<DeviceInterface*>>(a);
}

QList<DeviceInterface *> *QmlListAdapter::getList()
{
    return m_deviceInterfaces;
}

//QVariant QmlListAdapter::interfaces()
//{
//    return QVariant::fromValue(m_deviceInterfaces);
//}

QVariant QmlListAdapter::data(const QModelIndex &index, int role) const
{
    QObject* item = m_deviceInterfaces->at(index.row());
    return QVariant::fromValue(item);

//    if (!hasIndex(index.row(), index.column(), index.parent()))
//          return QVariant();

//    if (role == InterfaceRole) {
//        QVariant v;
//        //v.setValue(static_cast<QObject *>(m_deviceInterfaces[index.row()]));
//        v.setValue(static_cast<QObject *>(m_deviceInterfaces->at(index.row())));
//        return v;
//    }
//    return QVariant();
}

QModelIndex QmlListAdapter::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int QmlListAdapter::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_deviceInterfaces->size();
}


QHash<int, QByteArray> QmlListAdapter::roleNames() const
{
    static QHash<int, QByteArray> roles;
    roles[item] = "item";
    return roles;
}

void QmlListAdapter::rst_model()
{
    this->beginResetModel();
    this->endResetModel();
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
//    return reinterpret_cast< QmlListAdapter* >(list->/*data*/)->interface(i);
//}

//int QmlListAdapter::interfaceCount(QQmlListProperty<DeviceInterface>* list) {
//    return reinterpret_cast< QmlListAdapter* >(list->data)->interfaceCount();
//}
