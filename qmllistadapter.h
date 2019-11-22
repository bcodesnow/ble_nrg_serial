#ifndef QMLLISTADAPTER_H
#define QMLLISTADAPTER_H

#include <QObject>
#include <QVariant>
#include <QAbstractListModel>
#include <QHash>
#include "deviceinterface.h"

// TODO: CLeanup and comment!

class QmlListAdapter : public QAbstractListModel
{
    Q_OBJECT

//    Q_PROPERTY(QVariant interfaces READ interfaces NOTIFY interfacesChanged)

public:
    enum ListElementRoles {
        item = Qt::UserRole + 1,
    };
    QmlListAdapter(QObject *parent = nullptr);

    QList<DeviceInterface *>* getList();

//    QVariant interfaces();

//    int interfaceCount() const;
//    DeviceInterface* interface(int) const;

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    QHash<int, QByteArray>  roleNames() const;
    Q_INVOKABLE void rst_model();
private:
//    static int interfaceCount(QQmlListProperty<DeviceInterface>*);
//    static DeviceInterface* interface(QQmlListProperty<DeviceInterface>*, int);

    QList<DeviceInterface*>* m_deviceInterfaces;


signals:
    void interfacesChanged();

public slots:
};

#endif // QMLLISTADAPTER_H



