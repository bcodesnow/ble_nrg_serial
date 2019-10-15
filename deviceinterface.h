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

    uint32_t rec_ts;

public:
    DeviceInterface(TimeSyncHandler* ts_handler, CatchController* catch_controller,
                    LogFileHandler* logfile_handler, BluetoothBaseClass *parent = nullptr );

    ~DeviceInterface(); // they get destroyed in the list, always start with the last

    void initializeDevice( QBluetoothHostInfo* hostInfo, DeviceInfo* deviceInfo );
    //bool connectionAlive() const; // this is a connection state..
    friend QString stateToString(uint8_t tmp);

    DeviceInfo* m_deviceInfo;

    void sendCmdStart();
    void sendCmdStop();
signals:
    void invokeInitializeDevice( QBluetoothHostInfo* hostInfo, DeviceInfo* device );
    void invokePrintThreadId();
    //
    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();
    void sensorDataReceived();
    void sdEnabledChanged();
    //
    void invokeBleUartTx(const QByteArray &value);
    bool invokeBleUartSendCmdWithResp(const QByteArray &value, quint16 timeout = 200, quint8 retry = 5);
    void invokeBleUartSendCmdOk();

    void invokeStartConnModeChangeProcedure(quint8 mode);

    void invokeStartDownloadAllDataProcedure();


public slots:
    void onDeviceThreadStarted();
    void onTriggeredArrived(QByteArray value);
    void onAliveArrived(QByteArray value);
    void onSensorDataAvailableArrived();
};

#endif // DEVICEINTERFACE_H
