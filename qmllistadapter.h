#ifndef QMLLISTADAPTER_H
#define QMLLISTADAPTER_H

#include <QObject>
#include <QVariant>
#include <QQmlListProperty>
#include "deviceinterface.h"


class QmlListAdapter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariant interfaces READ interfaces NOTIFY interfacesChanged)

public:

    QmlListAdapter(QObject *parent = nullptr);

    QList<DeviceInterface *>* getList();

    QVariant interfaces();

//    int interfaceCount() const;
//    DeviceInterface* interface(int) const;

private:
//    static int interfaceCount(QQmlListProperty<DeviceInterface>*);
//    static DeviceInterface* interface(QQmlListProperty<DeviceInterface>*, int);

    QList<QObject*>* m_deviceInterfaces; -> QList<DeviceInterface*>* m_deviceInterfaces;

signals:
    void interfacesChanged();

public slots:
};

#endif // QMLLISTADAPTER_H



