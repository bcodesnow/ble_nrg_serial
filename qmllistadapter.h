#ifndef QMLLISTADAPTER_H
#define QMLLISTADAPTER_H

#include <QObject>
#include <QQmlListProperty>
#include "deviceinterface.h"


class QmlListAdapter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<DeviceInterface> interfaces READ interfaces)

public:
    QmlListAdapter(QObject *parent = nullptr);
    QList<DeviceInterface *>* getList();
    QQmlListProperty<DeviceInterface> interfaces();
    int interfaceCount() const;
    DeviceInterface* interface(int) const;

private:
    static int interfaceCount(QQmlListProperty<DeviceInterface>*);
    static DeviceInterface* interface(QQmlListProperty<DeviceInterface>*, int);

    QList<DeviceInterface*>* m_deviceInterfaces;

signals:

public slots:
};

#endif // QMLLISTADAPTER_H



