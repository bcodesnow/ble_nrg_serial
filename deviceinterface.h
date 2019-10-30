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

    //Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged)
    //Q_PROPERTY(bool heartBeat MEMBER m_heartBeat NOTIFY deviceAddressChanged)
    //Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    //Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)
    //Q_PROPERTY(bool sendingOverBleEnabled MEMBER m_sendingOverBleEnabled NOTIFY sendingOverBleEnabledChanged)

private:
    QString m_text;
    QThread m_thread_controller;
    DeviceController* m_deviceController;


    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    CatchController* m_catch_controller_ptr;
    //QString m_deviceAddress;

    alive_msg_t alive_msg;

    //uint32_t rec_ts;
    QTimer m_textTimer;

public:
    DeviceInterface(TimeSyncHandler* ts_handler, CatchController* catch_controller,
                    LogFileHandler* logfile_handler, DeviceInfo *parent = nullptr );

    ~DeviceInterface(){} // they get destroyed in the list, always start with the last

    void initializeDevice( QBluetoothHostInfo* hostInfo );
    friend QString stateToString(uint16_t tmp);

    void sendCmdStop();
    void sendCmdStart();
    void sendCmdSetLoggingMedia(bool savingTosdEnabled, bool bleUplEnabled);
    void sendCmdWriteCatchSuccessToSd(const quint8 &success);
    quint8 getLastMainState();

signals:
    void invokeInitializeDevice( QBluetoothHostInfo* hostInfo, QBluetoothDeviceInfo* deviceInfo );
    void invokePrintThreadId();
    //
    void mainStateOfDevXChanged(quint16 state, int idx);
//    void invokeSendCmdStart();

    //void deviceAddressChanged();
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


    //void textChanged(QString text);

public slots:
//    void onTextTimerTrig(){
//        static quint8 idx;
//        idx++;
//        auto printable = QStringLiteral("My magic number is %1. That's all!").arg(idx);
//        emit deviceChanged();
//        m_text = printable;
//        this->setDeviceIdentifier(printable);

//        emit textChanged(m_text);
//        qDebug()<<"timer expl"<<m_text;
//    }
    void onDeviceThreadStarted();
    void onTriggeredArrived(QByteArray value);
    void onAliveArrived(QByteArray value);
};
//Q_DECLARE_METATYPE(DeviceInterface)

#endif // DEVICEINTERFACE_H
