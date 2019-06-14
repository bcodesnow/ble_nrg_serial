#ifndef TIMESTAMPLER_H
#define TIMESTAMPLER_H

#include "devicehandler.h"
#include "ble_uart.h"

#include <QTimer>
#include <QElapsedTimer>
#include <devicehandler.h>
#include <QObject>
#include <QVector>

/* This class provides
 *          -> uSec TimeStamping
 *          -> TimeSync -Pushalike NTP ove BLE UART
 *          -> Travelling Time Measurement
 */
#define TS_MEASURE_MSG_CNT          15
#define TS_COMPENSATED_MSG_CNT      30
#define TS_TIMEOUT_DELAY_MS         25
#define TS_TRSH_FACTOR              (float)2.0

#define START_WAITS_FOR_ACK         (1<<0)
#define MEASURING_TRAVELING_TIME    (1<<1)
#define STOPPED                     (1<<2)
#define SENDING_COMPENSATED         (1<<3)
#define STOP_WAITS_FOR_ACK          (1<<4)

class DeviceHandler;

class TimeStampler : public QObject
{
    Q_OBJECT

private:
    DeviceHandler* m_deviceHandler;
    QElapsedTimer m_etimer;
    QTimer m_timeout_timer;
    quint16 m_send_repeat_count = 0;
    quint32 m_sync_state = 0;


    QVector<quint32> travelling_times; //"the total delay minus remote processing time" in decimillisec;
    quint32 m_last_msg_ts;
    quint8 m_dev_idx_in_sync;
    quint32 m_travelling_time_acceptance_trsh;
    void send_time_sync_msg();
    void send_compensated_time_sync_msg();
    void calculate_compensation();

signals:
    void time_sync_completed();
    void time_sync_failed();

public:
    TimeStampler(QObject *parent = 0);

    void setRefToDevHandlerArr(DeviceHandler* dev_handler_arr);

    void start_time_stamp();
    uint32_t get_timestamp_ms();
    uint32_t get_timestamp_us();

    uint32_t get_diff_in_us_to_current_ts ( uint32_t some_ts );
    void start_time_sync(quint8 devIdxToSync);

public slots:
    void time_sync_msg_sent(const QByteArray &msg);
    void time_sync_msg_arrived(const QByteArray &msg);
    void timeout_timer_expired();
};

#endif // TIMESTAMPLER_H
