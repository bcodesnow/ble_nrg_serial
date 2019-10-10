#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QThread>
#include <devicehandler.h>

// add baseclass!!!
class DeviceInterface : public QObject
{
    Q_OBJECT
private:
    QThread m_thread_controller;
    DeviceHandler* m_dev_handler_ptr;

public:
    explicit DeviceInterface(QObject *parent = nullptr);
    void init_device( QBluetoothAddress adapterAddr, DeviceInfo* device );

signals:
    void signal_printThreadId();
    void sig_init( QBluetoothAddress* adapterAddr, DeviceInfo* device );


public slots:
    void onDeviceThreadStarted();
};

#endif // DEVICEINTERFACE_H
