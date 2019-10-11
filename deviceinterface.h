#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QThread>
#include <QBluetoothHostInfo>
#include <devicehandler.h>
#include "catchcontroller.h"
#include "timesynchandler.h"
#include "logfilehandler.h"
#include "deviceinfo.h"

class DeviceHandler;
class TimeSyncHandler;
class LogFileHandler;
class CatchController;

// add baseclass!!!
class DeviceInterface : public QObject
{
    Q_OBJECT
private:
    QThread m_thread_controller;
    DeviceHandler* m_dev_handler_ptr;
    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    CatchController* m_catch_controller_ptr;
    int m_device_idx;

public:
    DeviceInterface(TimeSyncHandler* ts_handler, CatchController* catch_controller,
                    LogFileHandler* logfile_handler, int idx, QObject *parent = nullptr );

    ~DeviceInterface(); // they get destroyed in the list, always start with the last
    void init_device( QBluetoothHostInfo* hostInfo, DeviceInfo* deviceInfo );

signals:
    void signal_printThreadId();
    void sig_init( QBluetoothHostInfo* hostInfo, DeviceInfo* device );

    //
    void ble_uart_tx_sig(const QByteArray &value);
    bool ble_uart_send_cmd_with_resp_sig(const QByteArray &value, quint16 timeout = 200, quint8 retry = 5);
    void ble_uart_send_cmd_ok_sig();

    //    void adr_test_sig( QBluetoothAddress adapterAddr);
    //    void di_test_sig( DeviceInfo* device);
public slots:
    void onDeviceThreadStarted();
    void onTriggeredArrived(QByteArray value);
};

#endif // DEVICEINTERFACE_H
