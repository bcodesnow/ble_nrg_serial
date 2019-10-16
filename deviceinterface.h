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
class DeviceInterface : public DeviceInfo
{
    Q_OBJECT
    //Q_PROPERTY(bool heartBeat MEMBER m_heartBeat NOTIFY deviceAddressChanged)
    //Q_PROPERTY(bool connectionAlive READ connectionAlive NOTIFY aliveChanged)
    //Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    //Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)
    //Q_PROPERTY(bool sendingOverBleEnabled MEMBER m_sendingOverBleEnabled NOTIFY sendingOverBleEnabledChanged)

private:
    QThread m_thread_controller;
    DeviceController* m_dev_handler_ptr;


    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    CatchController* m_catch_controller_ptr;
    QString m_deviceAddress;

    alive_msg_t alive_msg;

    uint32_t rec_ts;

public:
    DeviceInterface(TimeSyncHandler* ts_handler, CatchController* catch_controller,
                    LogFileHandler* logfile_handler, DeviceInfo *parent = nullptr );

    ~DeviceInterface(){} // they get destroyed in the list, always start with the last

    void initializeDevice( QBluetoothHostInfo* hostInfo );
    //bool connectionAlive() const; // this is a connection state..
    friend QString stateToString(uint8_t tmp);
    void sendCmdStart();
    void sendCmdStop();
    void sendCmdWriteCatchSuccessToSd(const quint8 &success);

signals:
    void invokeInitializeDevice( QBluetoothHostInfo* hostInfo, QBluetoothDeviceInfo* deviceInfo );
    void invokePrintThreadId();
    //
    void deviceAddressChanged();
//    void deviceStateChanged();
//    void fileIndexOnDeviceChanged();
//    void sensorDataReceived();
//    void sdEnabledChanged();
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
};

#endif // DEVICEINTERFACE_H
