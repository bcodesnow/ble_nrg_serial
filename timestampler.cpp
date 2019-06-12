#include "timestampler.h"
#include "devicehandler.h"
#include <QTimer>
#include <QObject>
#include <algorithm> // min&max..

TimeStampler::TimeStampler(QObject *parent): m_deviceHandler(0)
{
    connect(&m_timeout_timer, SIGNAL(timeout()), this, SLOT(timeout_timer_expired()) );
    connect(&m_send_timer, SIGNAL(timeout()), this, SLOT(send_timer_expired()) );
    m_send_timer.setSingleShot(false);
    m_send_timer.setInterval(TS_SENDING_PERIOD_MS);
    m_timeout_timer.setSingleShot(true);
    m_timeout_timer.setInterval(TS_TIMEOUT_DELAY_MS);
}

void TimeStampler::time_sync_msg_sent(QByteArray msg)
{
    m_last_msg_ts = get_timestamp_us();
}

uint32_t TimeStampler::get_diff_in_us_to_current_ts(uint32_t some_ts)
{
    return ( ( m_etimer.nsecsElapsed() / 1000 ) - some_ts );
}

void TimeStampler::setRefToDevHandlerArr(DeviceHandler *dev_handler_arr)
{
    m_deviceHandler = dev_handler_arr;
}

void TimeStampler::start_time_stamp()
{
    m_etimer.start();
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
    uint32_t tstamp = get_timestamp_us();
    tba.resize(7);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_TIMESTAMP_IN_PAYLOAD;
    tba[2] = (uint8_t) m_send_timer_repeat_count;
    tba[3] = ( tstamp >> 24 ) & 0xFF ;
    tba[4] = ( tstamp >> 16 ) & 0xFF ;
    tba[5] = ( tstamp >>  8 ) & 0xFF ;
    tba[6] =   tstamp & 0xFF ;

    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
    m_timeout_timer.start();
}

void TimeStampler::send_compensated_time_sync_msg()
{
    QByteArray tba;
    uint32_t tstamp = get_timestamp_us();
    tba.resize(7);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_TIMESTAMP_IN_PAYLOAD;
    tba[2] = (uint8_t) m_send_timer_repeat_count;
    tba[3] = ( tstamp >> 24 ) & 0xFF ;
    tba[4] = ( tstamp >> 16 ) & 0xFF ;
    tba[5] = ( tstamp >>  8 ) & 0xFF ;
    tba[6] =   tstamp & 0xFF ;

    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
    m_timeout_timer.start();

}

void TimeStampler::time_sync_msg_arrived(QByteArray msg)
{
    m_timeout_timer.stop();

    switch (m_sync_state)
    {
    case START_WAITS_FOR_ACK:
        if (msg.at(2) != TS_CMD_ACK)
        {
            m_sync_state = STOPPED;
            qCritical()<<"TS: Wrong msg arrived.. we were waiting for an ACK..";
        }
        else
        {
            // all good, lets go!)
            m_send_timer.start();
        }
        break;
    case MEASURING_TRAVELING_TIME:
        travelling_times.append( get_diff_in_us_to_current_ts( m_last_msg_ts ) );
        if (msg.at(2) != m_send_timer_repeat_count)
            qCritical()<<"Shit got fkd up during time sync.. msgcount error!";
        // TODO: also log the processing times!
        break;
    case CALCULATING_COMPENSATION:
    {
        quint32 min = *std::min_element(travelling_times.constBegin(), travelling_times.constEnd());
        quint32 avg = std::accumulate(travelling_times.constBegin(), travelling_times.constEnd(), 0) / travelling_times.size() ;

        qInfo()<<"Travelling Times Calulated: ";
        qInfo()<<"Minimum: "<<min;
        qInfo()<<"Avarage: "<<avg;

        m_travelling_time_acceptance_trsh = min * 1.1;


        m_sync_state = SENDING_COMPENSATED;
        m_send_timer.start();
        break;
    }
    case SENDING_COMPENSATED:
        if (  get_diff_in_us_to_current_ts( m_last_msg_ts ) < m_travelling_time_acceptance_trsh )
        {
            // This was a good one!
            m_send_timer.stop();
            QByteArray tba;
            tba.resize(2);
            tba[0] = TS_MSG;
            tba[1] = TS_CMD_LAST_ONE_WAS_GOOD_ONE;
            m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
        }
        break;
    case STOP_WAITS_FOR_ACK:
        if (msg.at(2) != TS_CMD_ACK)
        {
            m_sync_state = STOPPED;
            qCritical()<<"TS: Wrong msg arrived.. we were waiting for an ACK..";
        }
        else
        {
            // we did it!
            qInfo()<<"TS: Sync completed!";
            emit time_sync_completed();
        }
        break;
    }
}

void TimeStampler::start_time_sync(quint8 devIdxToSync)
{
    m_dev_idx_in_sync = devIdxToSync;
    m_sync_state = START_WAITS_FOR_ACK;

    QByteArray tba;
    tba.resize(2);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_SYNC_START;
    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);

    m_timeout_timer.start();
}

void TimeStampler::send_timer_expired()
{
    m_send_timer_repeat_count++;
    if ( m_sync_state == MEASURING_TRAVELING_TIME)
    {
        if (m_send_timer_repeat_count == TS_MEASURE_MSG_CNT)
        {
            // last msg sent
            m_send_timer_repeat_count = 0;
            m_sync_state = CALCULATING_COMPENSATION;
        }
        else
        {
            send_time_sync_msg();
        }
    }
    else if (m_sync_state == SENDING_COMPENSATED)
    {
        if (m_send_timer_repeat_count == TS_COMPENSATED_MSG_CNT)
        {
            // last msg sent
            m_send_timer_repeat_count = 0;
            m_sync_state = CALCULATING_COMPENSATION;
            qCritical()<<"TS: Max retry reached, failed!";
            emit time_sync_failed();
        }
        else
        {
            send_compensated_time_sync_msg();
        }
    }
}

void TimeStampler::timeout_timer_expired()
{
    qCritical()<<"TimeSync Failed - > Timeout! - > Current State : "<<m_sync_state;
    m_sync_state = STOPPED;
}

