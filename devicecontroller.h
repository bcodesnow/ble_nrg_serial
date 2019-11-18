#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#define VERBOSITY_LEVEL 0
#define PRINT_THROUGHPUT 1
#define WAIT_X_MS_BETWEEN_CHUNKS 10
#define CHANGE_CONN_PARAM_OF_OTHER_DEVICES 0

#include "logfilehandler.h"
#include "timesynchandler.h"
#include <QElapsedTimer>
#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothDeviceInfo>
#include <QThread>
#include <QBluetoothHostInfo>

#define HUGE_CHUNK_IN_PROCESS       (1u<<1u)
#define CONN_PARAMS_IN_CHANGE       (1u<<3u)
#define CONN_MODE_CHANGE_DELAY 2000

#define NEXT_REQ_SEND_SENS_DATA  1u
#define NEXT_REQ_SEND_START      2u

class DeviceInfo;
class TimeSyncHandler;

extern QString stateToString(uint16_t tmp);
extern QString intToHexDebug(int x);

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
    uint16_t missed_pkg_cnt_to_request;
    uint16_t missed_in_request;
    bool skipped;
    bool last_received;
};

struct cmd_resp_struct_t
{
    QByteArray last_cmd;
    quint8 retry;
    quint16 timeout;
};

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

    QElapsedTimer* m_debugTimer; // temp
    QTimer* m_connParamTimer;
    QTimer* m_cmdTimer;
    QTimer* m_bleUartSendCmdWithRespBacklogTimer;

    int m_bleUartSendCmdWithRespBacklogRetries;
    QList<QByteArray*> m_bleUartSendCmdWithRespBacklog;

    bool m_bleUartServiceFound;

    QBluetoothAddress m_adapterAddress;
    int m_ident_idx; //
    QString m_ident_str;
    QTimer* m_nextRequestTimer;
    int m_nextRequest;

    //    quint8 m_progress;
    //    float m_transfSpeed;

    QVector<huge_chunk_indexed_byterray_t> m_hc_vec; // huge chunk indexed list
    QList<quint16> m_hc_missed; // list of missed package ids
    QByteArray* m_last_hc_payload_ptr; // last huge chunk - it gets passed as ref to the file handler in the different thread and will be always created here and destroyed there..


    huge_chunk_start_t hc_transfer_struct; // ble uart package type
    huge_chunk_helper_t hc_helper_struct;
    cmd_resp_struct_t cmd_resp_struct;


    quint8 m_dev_requested_conn_mode; // stay
    conn_param_info_t m_dev_conn_param_info; // ble uart package type
    quint8 retries_remaining;

    quint8 connectionValidSent;

    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    QBluetoothDeviceInfo* m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor; // do we need to remember him?

    QLowEnergyCharacteristic m_writeCharacteristic; // BLE UART RX
    QLowEnergyCharacteristic m_readCharacteristic; // BLE UART TX
    QLowEnergyService::WriteMode m_writeMode; // todo unused!

    QLowEnergyCharacteristic m_rxCharacteriscitPool[9]; // todo maybe a define for this
    QLowEnergyDescriptor m_rxCharacteriscitPoolDescriptors[9];

    //QLowEnergyController
    void connectToPeripheral(QBluetoothDeviceInfo *device);
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void searchCharacteristic();

    void updateCurrentService();

    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);

    // Helper Functions
    void printProperties(QLowEnergyCharacteristic::PropertyTypes); // make it to a be a friend..

    //Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str); // TODO -> this would not work..
    void printThroughput();
    // BLE UART Functions
    void bleUartRx(const QLowEnergyCharacteristic &c, const QByteArray &value);

    void writeReceivedChunkToFile (uint16_t tmp_write_pointer, uint8_t type );
    void setConnParamMode(uint8_t mode);

    inline bool isDeviceInRequestedConnState();

    void sendSetShutUp(bool shutUp);
    void setRequestedConnParamsOnDevice(uint8_t mode);
    void sendRequestMissingPackage();
    void sendAckOkHugeChunk();
    void sendStartOkHugeChunk();
    void sendRequestSensorData(quint8 catchSuccess = 0);

    void setConnParamsOnCentral(uint8_t mode);
    void hugeChunkDownloadFinished();

    friend  QString stateToString(uint16_t tmp);

public:
    DeviceController(int idx, QString identifier, QObject *parent = nullptr);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    void setIdentifier(QString str, quint8 idx); // make this only changeable through public member -> deviceInfo
    QString stateToString(int state);


signals:
    // Connection
    void connectionAlive(bool isItAlive, int deviceIdx);

    // Message Signals
    void aliveArrived(QByteArray value); // alive msg arrives
    void triggeredArrived(QByteArray value); // todo connect it
    void timeSyncMessageArrived(QByteArray value);

    void connParamInfoArrived();

    void sensorDataAvailableArrived(int idx);
    void replyMissingPackageArrived(QByteArray value);

    void noChunkAvailableArrived(); // Ext - Download Completed

    // Procedures (TS,HC,CONN,LF)
    void timeSyncMsgSent(int idx); // todo , the device doesnt know about ongoing tsync
    void requestDispatchToOtherDevices(QByteArray value, int ident_idx );

    void startHugeChunkAckProcArrived(QByteArray value);
    void startHugeChunkArrived();

    void allDataDownloaded(bool success, int idx); // Used By Catch Controller

    void requestedConnModeReached(bool success, quint8 mode); // ?!?!

    // Log File Handler Connections
    void invokeWriteTypeToFile(QString ident, QByteArray* data, quint8 type, quint16 wp);
    void invokeAddToLogFile(QString ident, QString key, QString val);

public slots:
    void bleUartTx(const QByteArray &value);
    bool bleUartSendCmdWithResp(const QByteArray &value, quint16 timeout = 500, quint8 retry = 5);
    void bleUartSendCmdOk();

    void initializeDevice(QBluetoothHostInfo* hostInfo, QBluetoothDeviceInfo* deviceInfo);
    void disconnectService();

    void printThreadId();

    void startConnModeChangeProcedure(quint8 mode);
    void startDownloadAllDataProcedure( quint8 catchSuccess );

    //void sendStartToDevice();
private slots:
    void onCentralConnectionUpdated(const QLowEnergyConnectionParameters &newParameters);

    void onConnParamInfoArrived();
    void onConnParamTimerExpired();

    void onStartHugeChunkArrived();
    void onStartHugeChunkAckProcArrived(QByteArray value);
    void onNoChunkAvailableArrived();
    void onReplyMissingPackageArrived(QByteArray value);

    void onCmdTimerExpired();
    void onNextRequestTimerExpired();
    void onBleUartSendCmdWithRespTimerExpired();

    void onConnected(void);
    void onDisconnected(void);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    //    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);


};

#endif // DEVICEHANDLER_H
