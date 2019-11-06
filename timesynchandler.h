#ifndef TIMESTAMPLER_H
#define TIMESTAMPLER_H

#include "deviceinterface.h"
#include "ble_uart.h"

#include <QTimer>
#include <QElapsedTimer>
#include <devicecontroller.h>
#include <QObject>
#include <QVector>

/* This class provides
 *          -> uSec TimeStamping
 *          -> TimeSync -Pushalike NTP ove BLE UART
 *          -> Travelling Time Measurement
 */
#define TS_MEASURE_MSG_CNT          30 //10
#define TS_COMPENSATED_MSG_CNT      60 //40
#define TS_TIMEOUT_DELAY_MS         250
#define TS_TRSH_FACTOR              (float)1.5 //(float)2.5

#define START_WAITS_FOR_ACK         (1<<0)
#define MEASURING_TRAVELING_TIME    (1<<1)
#define STOPPED                     (1<<2)
#define SENDING_COMPENSATED         (1<<3)
#define STOP_WAITS_FOR_ACK          (1<<4)

class DeviceInterface;

class TimeSyncHandler : public QObject
{
    Q_OBJECT

private:
//    DeviceHandler* m_deviceHandler;
    QElapsedTimer m_etimer;
    QTimer m_timeout_timer;
    quint16 m_send_repeat_count = 0;
    quint32 m_sync_state = 0;
    QList<DeviceInterface*>* m_device_interfaces_ptr;


    QVector<quint32> travelling_times; //"the total delay minus remote processing time" in decimillisec;
    quint32 m_last_msg_ts;
    int m_dev_idx_in_sync;
    quint32 m_travelling_time_acceptance_trsh;
    void send_time_sync_msg();
    void send_compensated_time_sync_msg();
    void calculate_compensation();

signals:
//    void time_sync_completed();
//    void time_sync_failed();
    void time_sync_finished(bool success, int id);

public:
    TimeSyncHandler(QList<DeviceInterface*>* device_interfaces, QObject *parent = nullptr);

    //void setRefToDevHandlerArr(DeviceHandler* dev_handler_arr);

    void start_time_stamp();
    uint32_t get_timestamp_ms();
    uint32_t get_timestamp_us();

    uint32_t get_diff_in_us_to_current_ts ( uint32_t some_ts );
    void start_time_sync(int devIdxToSync);

public slots:
    void slot_time_sync_msg_sent( int idx );
    void slot_time_sync_msg_arrived(const QByteArray &msg);
    void timeout_timer_expired();
};

#endif // TIMESTAMPLER_H
