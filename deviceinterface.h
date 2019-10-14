#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QThread>
#include <QBluetoothHostInfo>
#include <devicecontroller.h>
#include "catchcontroller.h"
#include "timesynchandler.h"
#include "logfilehandler.h"
#include "deviceinfo.h"
#include "bluetoothbaseclass.h"
#include "ble_uart_types.h"

class DeviceController;
class TimeSyncHandler;
class LogFileHandler;
class CatchController;

// todo: add baseclass ..or remove it completely and give this properties only to the catch controller.. it will be finally unused..!!!
class DeviceInterface : public BluetoothBaseClass
{
    Q_OBJECT
    Q_PROPERTY(bool connectionAlive READ connectionAlive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(QString deviceAddress MEMBER m_deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(QString deviceState MEMBER m_deviceState NOTIFY deviceStateChanged)
    Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)
    Q_PROPERTY(bool sendingOverBleEnabled MEMBER m_sendingOverBleEnabled NOTIFY sendingOverBleEnabledChanged)

private:
    QThread m_thread_controller;
    DeviceController* m_dev_handler_ptr;


    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    CatchController* m_catch_controller_ptr;


    alive_msg_t alive_msg;
    device_helper_struct_t m_dev_nfo;


public:
    DeviceInterface(TimeSyncHandler* ts_handler, CatchController* catch_controller,
                    LogFileHandler* logfile_handler, int idx, QObject *parent = nullptr );

    ~DeviceInterface(); // they get destroyed in the list, always start with the last
    void initializeDevice( QBluetoothHostInfo* hostInfo, DeviceInfo* deviceInfo );

    friend QString stateToString(uint8_t tmp);

    DeviceInfo*      deviceInfo;


signals:
    void deviceInitializationSignal( QBluetoothHostInfo* hostInfo, DeviceInfo* device );

    void signal_printThreadId();
    //
    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();
    void sensorDataReceived();
    void sdEnabledChanged();
    //
    void ble_uart_tx_sig(const QByteArray &value);
    bool ble_uart_send_cmd_with_resp_sig(const QByteArray &value, quint16 timeout = 200, quint8 retry = 5);
    void ble_uart_send_cmd_ok_sig();

    void set_peri_conn_mode_sig(quint8 mode);


public slots:
    void onDeviceThreadStarted();
    void onTriggeredArrived(QByteArray value);
    void onAliveArrived(QByteArray value);
};

#endif // DEVICEINTERFACE_H
