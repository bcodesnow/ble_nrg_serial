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
 *          -> TimeSync -vPushalike NTP ove BLE UART
 *          -> Travelling Time Measurement
 */

#define START_WAITS_FOR_ACK         (1<<0)
#define MEASURING_TRAVELING_TIME    (1<<1)
#define UNUSED                      (1<<2)
#define CALCULATING_COMPENSATION    (1<<3)
#define SENDING_COMPENSATED         (1<<4)

#define TS_MEASURE_MSG_CNT          35
#define TS_COMPENSATED_MSG_CNT      50
#define TS_SENDING_PERIOD_MS        25

class DeviceHandler;

class TimeStampler : public QObject
{
    Q_OBJECT

private:
    DeviceHandler* m_deviceHandler;
    QElapsedTimer m_etimer;
    QTimer m_send_timer;
    QTimer m_timeout_timer;
    quint16 m_send_timer_repeat_count = 0;
    quint32 m_sync_state = 0;


    QVector<quint32> travelling_times; //"the total delay minus remote processing time" in decimillisec;
    quint32 m_last_msg_ts;
    quint8 m_dev_idx_in_sync;
    quint32 m_travelling_time_acceptance_trsh;
    void send_time_sync_msg();
    void send_compensated_time_sync_msg();

signals:
    void time_sync_completed();
    void time_sync_failed();
    void last_ts_msg_sent();

public:
    TimeStampler(QObject *parent = 0);

    void setRefToDevHandlerArr(DeviceHandler* dev_handler_arr);

    void start_time_stamp();
    uint32_t get_timestamp_ms();
    uint32_t get_timestamp_us();

    uint32_t get_diff_in_us_to_current_ts ( uint32_t someTimeStamp );
    void start_time_sync(quint8 devIdxToSync);

public slots:
    void time_sync_msg_arrived(QByteArray msg);
    void time_sync_msg_sent(QByteArray msg);
    void send_timer_expired();
    void timeout_timer_expired();

};

#endif // TIMESTAMPLER_H
