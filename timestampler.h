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
 *          -> TimeSync routine for BLE DeviceHandlers
 *          -> Message Propagation Delay Measurement (both ways)
 */

#define T_SYNC_MSG_CNT 33

class DeviceHandler;

class TimeStampler : public QObject
{
    Q_OBJECT

private:
    DeviceHandler* m_deviceHandler;
    QElapsedTimer m_etimer;
    QTimer m_send_timer;
    quint16 m_send_timer_repeat_count;

    QVector<quint32> travelling_times; //"the total delay minus remote processing time" in decimillisec;
    quint32 m_last_msg_ts;
    quint8 m_dev_idx_in_sync;
    void send_time_sync_msg();

signals:
    void time_sync_completed();

public:
    TimeStampler(QObject *parent = 0);

    void setRefToDevHandlerArr(DeviceHandler* dev_handler_arr);

    void start_time_stamp();
    uint32_t get_timestamp_decims();
    uint32_t get_timestamp_ms();
    uint32_t get_timestamp_us();

    uint32_t get_diff_in_decims_to_current_ts ( uint32_t someTimeStamp );
    void start_time_sync(quint8 devIdxToSync);

public slots:
    void time_sync_msg_arrived(QByteArray msg);
    void time_sync_msg_sent(QByteArray msg);
    void send_timer_expired();

};

#endif // TIMESTAMPLER_H
