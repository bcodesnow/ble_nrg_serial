#include "timestampler.h"
#include "devicehandler.h"
#include <QTimer>
#include <QObject>

TimeStampler::TimeStampler(QObject *parent): m_deviceHandler(0)
{
    connect(&m_send_timer, SIGNAL(timeout()), this, SLOT(send_timer_expired()) );
    m_send_timer.setSingleShot(false);
}

void TimeStampler::time_sync_msg_sent(QByteArray msg)
{
    m_last_msg_ts = get_timestamp_decims();
}

uint32_t TimeStampler::get_diff_in_decims_to_current_ts(uint32_t someTimeStamp)
{
    return ( ( m_etimer.nsecsElapsed() / 10000 ) - someTimeStamp );
}

void TimeStampler::setRefToDevHandlerArr(DeviceHandler *dev_handler_arr)
{
    m_deviceHandler = dev_handler_arr;
}

void TimeStampler::start_time_stamp()
{
    m_etimer.start();
}

uint32_t TimeStampler::get_timestamp_decims()
{
    return m_etimer.nsecsElapsed() / 10000;
}

uint32_t TimeStampler::get_timestamp_ms()
{
    return m_etimer.elapsed();
}

uint32_t TimeStampler::get_timestamp_us()
{
    return m_etimer.nsecsElapsed() / 1000;
}

void TimeStampler::send_time_sync_msg()
{
    QByteArray tba;
    uint32_t tstamp = get_timestamp_decims();
    tba.resize(6);
    tba[0] = TIME_SYNC_FROM_SERVER;
    tba[1] = 0x55;
    tba[2] = ( tstamp>>24 ) & 0xFF ;
    tba[3] = ( tstamp>>16 ) & 0xFF ;
    tba[4] = ( tstamp>>8 ) & 0xFF ;
    tba[5] = tstamp & 0xFF ;

    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
}

void TimeStampler::time_sync_msg_arrived(QByteArray msg)
{
    travelling_times.append( get_diff_in_decims_to_current_ts( m_last_msg_ts) );
    qDebug()<<travelling_times;
}

void TimeStampler::start_time_sync(quint8 devIdxToSync)
{
    m_dev_idx_in_sync = devIdxToSync;
    m_send_timer.setInterval(33);
    m_send_timer.start();
}

void TimeStampler::send_timer_expired()
{
    m_send_timer_repeat_count++;
    if (m_send_timer_repeat_count == T_SYNC_MSG_CNT)
    {
        m_send_timer_repeat_count = 0;
    }
    else
    {
        send_time_sync_msg();
    }
}

