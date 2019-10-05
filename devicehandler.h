// ?? L2CAP Connection Parameter Update Response, TX POWER

#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include "bluetoothbaseclass.h"
#include "logfilehandler.h"
#include "timestampler.h"

#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>

#define CDSM_STATE_INIT													( 0u )
#define CDSM_STATE_RUNNING                                              (	1u << 0u )
#define CDSM_STATE_READY_TO_BE_TRIGGERED                                ( 1u << 1u )	/* There is already data for 1 sec in the buffer. */
#define CDSM_STATE_TRIGGERED                                            (	1u << 2u )
#define CDSM_STATE_POST_TRIGGER_DATA_COLLECTED                          ( 1u << 3u )  /* The needed amount of data after the trigger has been collected, lets save it to the SD card. */
#define CDSM_STATE_STOPPING                                             ( 1u << 4u )
#define CDSM_STATE_STOPPED                                              ( 1u << 5u )
#define CDSM_STATE_RESTARTING                                           ( 1u << 6u )
#define CDSM_STATE_ERROR 												( 1u << 7u )
#define CDSM_STATE_COLLECT_DATA                                         (	CDSM_STATE_RUNNING | CDSM_STATE_READY_TO_BE_TRIGGERED | CDSM_STATE_TRIGGERED )

#define CDSM_SUBSTATE_STOPPING                                  0u
#define	CDSM_SUBSTATE_SAVING_AUDIO                              1u
#define CDSM_SUBSTATE_SAVING_MAGNETO                            2u
#define CDSM_SUBSTATE_SAVING_PRESSURE                           3u
#define CDSM_SUBSTATE_SAVING_ACC                                4u
#define CDSM_SUBSTATE_SAVING_GYRO                               5u
#define CDSM_SUBSTATE_SENDING_DATA_COLLECTED                    6u

struct huge_chunk_indexed_byterray_t
{
    quint16 received;
    QByteArray barr;
};

class DeviceInfo;
class TimeStampler;

class DeviceHandler : public BluetoothBaseClass
{
    Q_OBJECT

    Q_PROPERTY(bool alive READ alive NOTIFY aliveChanged)
    Q_PROPERTY(AddressType addressType READ addressType WRITE setAddressType)
    Q_PROPERTY(QString deviceAddress MEMBER m_deviceAddress NOTIFY deviceAddressChanged)
    Q_PROPERTY(QString deviceState MEMBER m_deviceState NOTIFY deviceStateChanged)
    Q_PROPERTY(bool sdEnabled MEMBER m_sdEnabled NOTIFY sdEnabledChanged)
    Q_PROPERTY(qint16 fileIndexOnDevice MEMBER m_fileIndexOnDevice NOTIFY fileIndexOnDeviceChanged)
    Q_PROPERTY(bool writeValid READ getWriteValid NOTIFY writeValidChanged)

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

    QString m_ident_str;
    quint8 m_ident_idx;

    QByteArray m_huge_chunk;
    QVector<huge_chunk_indexed_byterray_t> m_hc_vec; // huge chunk indexed list
    QList<quint16> m_hc_missed;

    bool m_multi_chunk_mode; // huge chunk on multiple characteristics
    bool m_sdEnabled;
    QBluetoothAddress m_adapterAddress;
    QString m_deviceAddress;
    QString m_deviceState;
    qint16 m_fileIndexOnDevice;
    quint8 m_deviceSubState;
    quint8 m_deviceLastError;

    bool m_found_BLE_UART_Service;
    //  use main identifier to select which to select
    QLowEnergyController *m_control;
    QLowEnergyService *m_service;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;

    DeviceInfo *m_currentDevice;
    QLowEnergyDescriptor m_notificationDescriptor;

    DeviceHandler* m_refToOtherDevice;
    LogFileHandler* m_refToFileHandler;
    TimeStampler* m_refToTimeStampler;

    QLowEnergyCharacteristic m_writeCharacteristic;
    QLowEnergyService::WriteMode m_writeMode;
    QLowEnergyCharacteristic m_readCharacteristic;
    QLowEnergyCharacteristic m_rxCharacteriscitPool[9];
    QLowEnergyDescriptor m_rxCharacteriscitPoolDescriptors[9];

    uint16_t m_missed_to_request;
    uint16_t m_missed_in_request;



    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();

    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void ble_uart_rx(const QLowEnergyCharacteristic &c, const QByteArray &value);
    QString state_to_string(uint8_t tmp);

    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);
    void update_currentService();

    void searchCharacteristic();
    void printProperties(QLowEnergyCharacteristic::PropertyTypes);

    void requestMissingPackage();

    void ackHugeChunk();

    void update_conn_period();

    void parse_n_write_received_pool (uint16_t tmp_write_pointer, uint8_t type );

public:
    DeviceHandler(QObject *parent = 0);

    Q_INVOKABLE void sendCMDStringFromTerminal(const QString &str);
    Q_INVOKABLE void requestBLESensorData(void);

    enum class AddressType {
        PublicAddress,
        RandomAddress
    };
    Q_ENUM(AddressType)

    void setBtAdapter(QBluetoothAddress addr)
    {
        m_adapterAddress = addr;
    }
    void setDevice(DeviceInfo *device);
    void setAddressType(AddressType type);
    AddressType addressType() const;

    void setRefToOtherDevice (DeviceHandler* t_dev_handler);
    void setRefToFileHandler (LogFileHandler* t_fil_helper);
    void setRefToTimeStampler (TimeStampler* t_time_stampler);

    void setIdentifier(QString str, quint8 idx, QBluetoothAddress addr);

    bool alive() const;
    void ble_uart_tx(const QByteArray &value);

    bool getWriteValid(void)
    {
        return m_writeCharacteristic.isValid();
    }

signals:
    void aliveChanged();
    void deviceAddressChanged();
    void deviceStateChanged();
    void fileIndexOnDeviceChanged();
    void aliveArrived();
    void sensorDataAvailable();
    void sensorDataReceived();
    void sdEnabledChanged();
    void writeValidChanged();
    void transferProgressChanged(uint8_t percentage);
    void showProgressMessage(QString mainText, QString subText, int percent, uint8_t flag);
    void progressFinished();

public slots:
    void disconnectService();
    void onConnectionParamUpdated(const QLowEnergyConnectionParameters &newParameters);

private slots:
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onConnected(void);
    void onShutUpSet(bool shutUp);
    void setConnParams(double min_peri, double max_peri);

};

#endif // DEVICEHANDLER_H
