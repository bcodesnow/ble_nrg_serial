/* ?? L2CAP Connection Parameter Update Response, TX POWER */

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"
#include "logfilehandler.h"
#include "timesynchandler.h"
#include <QElapsedTimer>
#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QThread>

#define CDSM_STATE_INIT													( 0u )
#define CDSM_STATE_RUNNING                                              ( 1u << 0u )
#define CDSM_STATE_READY_TO_BE_TRIGGERED                                ( 1u << 1u )	/* There is already data for 1 sec in the buffer. */
#define CDSM_STATE_TRIGGERED                                            ( 1u << 2u )
#define CDSM_STATE_POST_TRIGGER_DATA_COLLECTED                          ( 1u << 3u )  /* The needed amount of data after the trigger has been collected, lets save it to the SD card. */
#define CDSM_STATE_STOPPING                                             ( 1u << 4u )
#define CDSM_STATE_STOPPED                                              ( 1u << 5u )
#define CDSM_STATE_RESTARTING                                           ( 1u << 6u )
#define CDSM_STATE_ERROR 												( 1u << 7u )
#define CDSM_STATE_COLLECT_DATA                                         ( CDSM_STATE_RUNNING | CDSM_STATE_READY_TO_BE_TRIGGERED | CDSM_STATE_TRIGGERED )

#define CDSM_SUBSTATE_STOPPING                                  0u
#define	CDSM_SUBSTATE_SAVING_AUDIO                              1u
#define CDSM_SUBSTATE_SAVING_MAGNETO                            2u
#define CDSM_SUBSTATE_SAVING_PRESSURE                           3u
#define CDSM_SUBSTATE_SAVING_ACC                                4u
#define CDSM_SUBSTATE_SAVING_GYRO                               5u
#define CDSM_SUBSTATE_SENDING_DATA_COLLECTED                    6u


#define HUGE_CHUNK_IN_PROCESS       (1u<<1u)
#define CONN_PARAMS_IN_CHANGE       (1u<<3u)

struct huge_chunk_indexed_byterray_t
{
    quint16 received;
    QByteArray barr;
};

struct huge_chunk_helper_t
{
    uint16_t hc_highest_index;
    bool first_multi_chunk;
    uint16_t last_idx;
    uint16_t missed_to_request;
    uint16_t missed_in_request;
};

struct cmd_resp_struct_t
{
    QTimer cmd_timer;
    QByteArray last_cmd;
    quint8 retry;
    quint16 timeout;
} ;

class DeviceInfo;
class TimeSyncHandler;

// TODO REMOVE Bluetoothbaseclass!!!!!
class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool connectionAlive READ connectionAlive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(QString deviceAddress MEMBER m_deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(QString deviceState MEMBER m_deviceState NOTIFY deviceStateChanged)
    Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)
    Q_PROPERTY(bool sendingOverBleEnabled MEMBER m_sendingOverBleEnabled NOTIFY sendingOverBleEnabledChanged)
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

   // quint32 m_deviceFlags;
    //  use main identifier to select which to select

    QBluetoothAddress m_adapterAddress;
    QString m_deviceAddress; // Q_PROPERTY
    QString m_deviceState; // Q_PROPERTY
    QString m_ident_str; // Q_PROPERTY

    bool m_sendingOverBleEnabled; // Q_PROPERTY
    bool m_sdEnabled; // Q_PROPERTY

    qint16 m_fileIndexOnDevice; // Q_PROPERTY
    quint8 m_ident_idx;
    quint8 downloading_sensor_data_active;

    bool m_found_BLE_UART_Service;

    QVector<huge_chunk_indexed_byterray_t> m_hc_vec; // huge chunk indexed list
    QList<quint16> m_hc_missed; // list of missed package ids

    conn_param_info_t m_dev_conn_param_info; // ble uart package type
    huge_chunk_start_t hc_transfer_struct; // ble uart package type
    huge_chunk_helper_t hc_helper_struct;
    cmd_resp_struct_t cmd_resp_struct;

    quint8 m_deviceSubState;
    quint8 m_deviceLastError;
    quint8 m_dev_requested_conn_mode;

    uint32_t rec_ts;

    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    DeviceInfo *m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor;

    DeviceHandler* m_refToOtherDevice;
    LogFileHandler* m_refToFileHandler;
    TimeSyncHandler* m_refToTimeStampler;

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
    QString state_to_string(uint8_t tmp);
    void printProperties(QLowEnergyCharacteristic::PropertyTypes);
    Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str); // TODO -> this would not work..
    void printThroughput();

    // BLE UART Functions
    void ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void parse_n_write_received_pool (uint16_t tmp_write_pointer, uint8_t type );
    void setConnParamMode(uint8_t mode);



public:
    DeviceHandler(QObject *parent = nullptr);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)


    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    void setRefToOtherDevice (DeviceHandler* t_dev_handler);
    void setRefToFileHandler (LogFileHandler* t_fil_helper);
    void setRefToTimeStampler (TimeSyncHandler* t_time_stampler);
    void setBtAdapter(QBluetoothAddress addr); // modify to pass the pointer

    void setIdentifier(QString str, quint8 idx, QBluetoothAddress addr);

    bool connectionAlive() const; // this is a connection state..


    //    bool getWriteValid(void)
    //    {
    //        return m_writeCharacteristic.isValid();
    //    }

    void ble_uart_tx(const QByteArray &value);
    inline bool isDeviceInRequestedConnState();

    bool ble_uart_send_cmd_with_resp(const QByteArray &value, quint16 timeout = 200, quint8 retry = 5);
    void ble_uart_send_cmd_ok();

    void setShutUp(bool shutUp);
    void setConnParamsOnCentral(uint8_t mode);
    void setRequestedConnParamsOnDevice(uint8_t mode);
    void requestMissingPackage();
    void ackHugeChunk();
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


    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();
    void sensorDataReceived();
    void sdEnabledChanged();
    //void writeValidChanged();
    void transferProgressChanged(uint8_t percentage);
    void showProgressMessage(QString mainText, QString subText, int percent, uint8_t flag);
    void progressFinished();

public slots:
    void init_slot(QBluetoothAddress* adapterAddr, DeviceInfo* device);
    void disconnectService();

    void onSensorDataRequested(void); // start download

    void slot_printThreadId()
    {
         qDebug()<<"Thread id of device:"<<QThread::currentThreadId();
    }

private slots:
    void onConnectionParamUpdated(const QLowEnergyConnectionParameters &newParameters);
    void onAliveArrived(QByteArray value); // alive msg gets handled
    void onStartHugeChunkArrived();
    void onTriggeredArrived(QByteArray value);
    void onStartHugeChunkAckProcArrived(QByteArray value);

    //void onTimeSyncMessageArrived(QByteArra value); //unimplemented.. we need to use the same instanz to keep it simple.

    void onConnParamInfoArrived();
    void onConnParamTimerExpired();

    void onSensorDataAvailableArrived();

    void onReplyMissingPackageArrived(QByteArray value);

    void onCmdTimerExpired();

    void onConnected(void);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);

    // relevant to qml..


};

#endif // DEVICEHANDLER_H


