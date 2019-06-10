#ifndef TIMESTAMPLER_H
#define TIMESTAMPLER_H

#include "devicehandler.h"

#include <QTimer>
#include <QElapsedTimer>
#include <devicehandler.h>
#include <QObject>
#include <QVector>

/* This class provides
 *          -> uSec TimeStamping
 *          -> TimeSync routine for ble devicehandlers
 *          -> Message Propagation Delay Measurement
 */

#define T_SYNC_MSG_CNT 50

class DeviceHandler;

class TimeStampler : public QObject
{
    Q_OBJECT
private:
    qint64 m_lastTimeStamp;
    qint64 m_mode; // TIMESTAMP + MESSAGE TIMER
    DeviceHandler* m_device_handler_arr;
    QElapsedTimer m_etimer;
    QTimer m_send_timer;
    quint16 m_send_timer_repeat_count;
    QVector<quint32> m_one_way_msg_latency;
    QVector<quint32> m_two_way_msg_latency;

signals:
    void time_sync_completed();

public:
    TimeStampler(QObject *parent = 0);

    void setRefToDevHandlerArr(DeviceHandler* dev_handler_arr_ptr)
    {
        m_device_handler_arr = dev_handler_arr_ptr;
    }

    void start_time_stamp()
    {
        m_etimer.start();
    }

    uint32_t get_timestamp_us()
    {
        return m_etimer.nsecsElapsed() / 1000;
    }

    uint32_t get_timestamp_ms()
    {
        return m_etimer.elapsed();
    }

    void send_time_sync_msg()
    {

    }

    void start_time_sync()
    {
        m_send_timer.setInterval(20);
        m_send_timer.start();
    }

    void send_timer_expired()
    {
        m_send_timer_repeat_count++;
        if (m_send_timer_repeat_count == T_SYNC_MSG_CNT)
        {
            m_send_timer_repeat_count = 0;

        }
    }

    void startResponseDelayTimer(quint16 delayInMs, quint8 idOfDeviceHandlerRequesting)
    {

    }
    // elapsed returns ms

public slots:
    void time_sync_msg_arrived(QByteArray* msg);



};

#endif // TIMESTAMPLER_H
