#ifndef DEVICEFINDER_H
#define DEVICEFINDER_H

#include "bluetoothbaseclass.h"

#include <QTimer>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QVariant>
#include "deviceinterface.h"
#include "connectionhandler.h"

class DeviceInfo;
class QmlListAdapter;

class DeviceFinder: public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(QVariant devices READ devices NOTIFY devicesChanged)

public:
    DeviceFinder(QmlListAdapter* deviceListAdapter, ConnectionHandler* connHandler, TimeSyncHandler* ts_handler,
                 CatchController* catch_controller, LogFileHandler* logfile_handler, QObject *parent = nullptr);
    ~DeviceFinder();

    bool scanning() const;
    QVariant devices();

    Q_INVOKABLE void addDeviceToSelection(const quint8 &idx);
    Q_INVOKABLE void removeDeviceFromSelection(const quint8 &idx);

public slots:
    void startSearch();
    //void connectToService(const QString &address);
    void connectToSelectedDevices(); // QML JUMP ON TODO


private slots:
    void addDevice(const QBluetoothDeviceInfo&);
    void scanError(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

signals:
    void scanningChanged();
    void devicesChanged();

private:
    QBluetoothDeviceDiscoveryAgent *m_deviceDiscoveryAgent;
    QList<QObject*> m_foundDevices;
    quint8 m_selectedDevicesCount;

    ConnectionHandler* m_conn_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    LogFileHandler* m_logfile_handler_ptr;
    CatchController* m_catch_controller_ptr;

    QList<DeviceInterface*>* m_deviceList;
    QmlListAdapter* m_deviceListAdapter;

};

#endif // DEVICEFINDER_H
