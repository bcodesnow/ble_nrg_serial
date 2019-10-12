#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "devicehandler_types.h"
#include "logfilehandler.h"
#include "timesynchandler.h"
#include <QElapsedTimer>
#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QThread>
#include <QBluetoothHostInfo>

#define HUGE_CHUNK_IN_PROCESS       (1u<<1u)
#define CONN_PARAMS_IN_CHANGE       (1u<<3u)

class DeviceInfo;
class TimeSyncHandler;

extern QString stateToString(uint8_t tmp);

class DeviceController : public QObject
{
    Q_OBJECT
    //Q_PROPERTY(bool writeValid READ getWriteValid NOTIFY writeValidChanged)

private:
    const QString BLE_UART_RX_CHAR = "{d773f2e2-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_TX_CHAR = "{d873f2e1-b19e-11e2-9e96-0800200c9a66}";
    const QString BLE_UART_SERVICE = "{d973f2e0-b19e-11e2-9e96-0800200c9a66}";
    const QStringList BLE_UART_TX_POOL = {
        "{8073f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8173f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8273f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8373f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8473f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8573f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8673f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8773f2e2-b19e-11e2-9e96-0800200c9a66}",
        "{8873f2e2-b19e-11e2-9e96-0800200c9a66}",
    };

    QElapsedTimer m_debugTimer; // temp
    QTimer m_timeoutTimer;
    QTimer m_connParamTimer;

    bool m_found_BLE_UART_Service;

    QBluetoothAddress m_adapterAddress;
    int m_ident_idx; //
    QString m_ident_str;

    quint8 downloading_sensor_data_active;


    QVector<huge_chunk_indexed_byterray_t> m_hc_vec; // huge chunk indexed list
    QList<quint16> m_hc_missed; // list of missed package ids
    QByteArray* m_last_hc_payload_ptr;


    huge_chunk_start_t hc_transfer_struct; // ble uart package type
    huge_chunk_helper_t hc_helper_struct;
    cmd_resp_struct_t cmd_resp_struct;

    uint32_t rec_ts; // interface



    quint8 m_dev_requested_conn_mode; // stay
    conn_param_info_t m_dev_conn_param_info; // ble uart package type
    quint8 m_conn_param_operation;

    quint8 retries_remaining;
    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    DeviceInfo *m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyService::WriteMode m_writeMode;
    QLowEnergyCharacteristic m_readCharacteristic;
    QLowEnergyCharacteristic m_rxCharacteriscitPool[9];
    QLowEnergyDescriptor m_rxCharacteriscitPoolDescriptors[9];

    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void searchCharacteristic();
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void update_currentService();

    // Helper Functions
    void printProperties(QLowEnergyCharacteristic::PropertyTypes);
    Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str); // TODO -> this would not work..
    void printThroughput();

    // BLE UART Functions
    void ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void parse_n_write_received_pool (uint16_t tmp_write_pointer, uint8_t type );

    void setConnParamMode(uint8_t mode);

//    void connect_rx_sigs_to_slots()
//    {
//        connect(this, &DeviceHandler::startHugeChunkAckProcArrived, this, &DeviceHandler::onStartHugeChunkAckProcArrived);
//        connect(this, &DeviceHandler::startHugeChunkArrived, this, &DeviceHandler::onStartHugeChunkArrived);

//        connect(this, &DeviceHandler::triggeredArrived, this, &DeviceHandler::onTriggeredArrived); // connect to catch ControllerWith Queued!!

//        //connect(this, &DeviceHandler::timeSyncMessageArrived, this, &DeviceHandler::onTimeSyncMessageArrived);

//        connect(this, &DeviceHandler::connParamInfoArrived, this, &DeviceHandler::onConnParamInfoArrived);

//        connect(this, &DeviceHandler::sensorDataAvailableArrived, this, &DeviceHandler::onSensorDataAvailableArrived);

//        connect(this, &DeviceHandler::replyMissingPackageArrived, this, &DeviceHandler::onReplyMissingPackageArrived);
//    }



public:
    DeviceController(int idx, QString identifier, QObject *parent = nullptr);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    friend     QString stateToString(uint8_t tmp);
    void connectToPeripheral(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

/*    void setRefToOtherDevice (DeviceHandler* t_dev_handler);
    void setRefToFileHandler (LogFileHandler* t_fil_helper);
    void setRefToTimeStampler (TimeSyncHandler* t_time_stampler)*/
    //void setBtAdapter(QBluetoothAddress addr); // modify to pass the pointer

    void setIdentifier(QString str, quint8 idx, QBluetoothAddress addr);

    bool connectionAlive() const; // this is a connection state..


    //    bool getWriteValid(void)
    //    {
    //        return m_writeCharacteristic.isValid();
    //    }


    inline bool isDeviceInRequestedConnState();

    void setShutUp(bool shutUp);
    void setConnParamsOnCentral(uint8_t mode);
    void setRequestedConnParamsOnDevice(uint8_t mode);
    void requestMissingPackage();
    void sendAckHugeChunk();
    void requestSensorData();

signals:
    void sendingOverBleEnabledChanged();
    void aliveChanged(); // alive information gets notified to handler ... rename it to connectionAlive or sth...

    void aliveArrived(QByteArray value); // alive msg arrives
    void startHugeChunkAckProcArrived(QByteArray value);
    void startHugeChunkArrived();
    void triggeredArrived();
    void timeSyncMessageArrived(QByteArray value);

    void connParamInfoArrived();

    void sensorDataAvailableArrived();
    void replyMissingPackageArrived();
    void requestDispatchToOtherDevices(QByteArray value, quint8 ident_idx );
    void time_sync_msg_sent(QByteArray value, int idx);

    void requestedConnModeReached(bool success, quint8 mode);

//    void writeValidChanged();
//    void transferProgressChanged(uint8_t percentage);
//    void showProgressMessage(QString mainText, QString subText, int percent, uint8_t flag);
//    void progressFinished();

    // Log File Handler Connections
    void write_type_to_file_sig(QString ident, QByteArray* data, uint8_t type, uint16_t wp);
    void add_to_log_fil_sig(QString ident, QString key, QString val);

public slots:
    void ble_uart_tx(const QByteArray &value);
    bool ble_uart_send_cmd_with_resp(const QByteArray &value, quint16 timeout = 200, quint8 retry = 5);
    void ble_uart_send_cmd_ok();
    void deviceInitializationSlot(QBluetoothHostInfo* hostInfo, DeviceInfo* deviceInfo);

    void disconnectService();

    void onSensorDataRequested(void); // start download

    void slot_printThreadId() { qDebug()<<"Thread id of device:"<<QThread::currentThreadId(); }

private slots:
    void onCentralConnectionUpdated(const QLowEnergyConnectionParameters &newParameters);

    void onConnParamInfoArrived();
    void onConnParamTimerExpired();


    //void onAliveArrived(QByteArray value); // alive msg gets handled
    //void onTriggeredArrived(QByteArray value);

    void onStartHugeChunkArrived();
    void onStartHugeChunkAckProcArrived(QByteArray value);

    //void onTimeSyncMessageArrived(QByteArra value); //unimplemented.. we need to use the same instanz to keep it simple.


    void onSensorDataAvailableArrived();

    void onReplyMissingPackageArrived(QByteArray value);

    void onCmdTimerExpired();

    void onConnected(void);
    void onDisconnected(void);
//    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void set_peri_conn_mode(quint8 mode);

    // relevant to qml..


};

#endif // DEVICEHANDLER_H


