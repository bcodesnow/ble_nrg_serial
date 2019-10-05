#include "timestampler.h"
#include "devicehandler.h"
#include <QTimer>
#include <QObject>
#include <algorithm> // min&max..
#include <QThread>

TimeStampler::TimeStampler(QObject *parent): m_deviceHandler(0)
{
    Q_UNUSED(parent);
    connect(&m_timeout_timer, SIGNAL(timeout()), this, SLOT(timeout_timer_expired()) );
    m_timeout_timer.setSingleShot(true);
    m_timeout_timer.setInterval(TS_TIMEOUT_DELAY_MS);
}

void TimeStampler::time_sync_msg_sent(const QByteArray &msg)
{
    // toDo m_last_msg_ts is only relevant if we are mesuring...
    qInfo()<<"TS: Sync Msg. Sent!";
    m_last_msg_ts = get_timestamp_us();
}

uint32_t TimeStampler::get_diff_in_us_to_current_ts(uint32_t some_ts)
{
    return ( m_etimer.nsecsElapsed() / 1000 ) - some_ts;
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
    return ( m_etimer.nsecsElapsed() / 1000 );
}

void TimeStampler::send_time_sync_msg()
{
    m_send_repeat_count++;

    QByteArray tba;
    uint32_t tstamp = get_timestamp_us();
    tba.resize(7);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_TIMESTAMP_IN_PAYLOAD;
    tba[2] = (uint8_t) m_send_repeat_count;
    tba[3] = ( tstamp >> 24 ) & 0xFF ;
    tba[4] = ( tstamp >> 16 ) & 0xFF ;
    tba[5] = ( tstamp >>  8 ) & 0xFF ;
    tba[6] =   tstamp & 0xFF ;
    qDebug()<<"TS: send_time_sync_msg() -> "<<tstamp;

    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
    m_timeout_timer.start();
}

void TimeStampler::send_compensated_time_sync_msg()
{
    m_send_repeat_count++;

    QByteArray tba;
    uint32_t tstamp = get_timestamp_us() + ( m_travelling_time_acceptance_trsh / 2 ) /* - processingTime!*/;
    tba.resize(7);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_TIMESTAMP_IN_PAYLOAD;
    tba[2] = (uint8_t) m_send_repeat_count;
    tba[3] = ( tstamp >> 24 ) & 0xFF ;
    tba[4] = ( tstamp >> 16 ) & 0xFF ;
    tba[5] = ( tstamp >>  8 ) & 0xFF ;
    tba[6] =   tstamp & 0xFF ;
    m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);

    m_timeout_timer.start();

    qDebug()<<"TS: send_compensated_time_sync_msg() -> "<<tstamp;
    qDebug()<<"TS: Amount of Compensation -> "<<( m_travelling_time_acceptance_trsh / 2 );
}

void TimeStampler::calculate_compensation()
{
    quint32 min = *std::min_element(travelling_times.constBegin(), travelling_times.constEnd());
    quint32 max = *std::max_element(travelling_times.constBegin(), travelling_times.constEnd());
    quint32 avg = std::accumulate(travelling_times.constBegin(), travelling_times.constEnd(), 0) / travelling_times.size() ;

    qInfo()<<"TS: Travelling Times Calculated: ";
    qInfo()<<"TS: Minimum: "<<min;
    qInfo()<<"TS: Maximum: "<<max;
    qInfo()<<"TS: Avarage: "<<avg;

    m_travelling_time_acceptance_trsh =int ( (float) min *  TS_TRSH_FACTOR );
    //m_travelling_time_acceptance_trsh = avg;
    QThread::msleep(8); // give it just a few msecs to breathe
    qInfo()<<"Calculated Threshold: "<< m_travelling_time_acceptance_trsh;

    m_sync_state = SENDING_COMPENSATED;
    send_compensated_time_sync_msg();
}

void TimeStampler::time_sync_msg_arrived(const QByteArray &msg)
{
    m_timeout_timer.stop();
    qDebug()<<"Arrived: "<<msg;
    switch (m_sync_state)
    {
    case START_WAITS_FOR_ACK:

        if (msg.at(1) != TS_CMD_ACK)
        {
            m_sync_state = STOPPED;
            qCritical()<<"TS: Wrong msg arrived.. we were waiting for an ACK..";
        }
        else
        {
            // all good, lets go!)
            send_time_sync_msg();
            m_sync_state = MEASURING_TRAVELING_TIME;
        }
        break;
        //
    case MEASURING_TRAVELING_TIME:
        travelling_times.append( get_diff_in_us_to_current_ts( m_last_msg_ts ) );
        qDebug()<<"TS: Travelling Time -> "<< travelling_times.last() << " us";

        if (int(msg.at(2)) != m_send_repeat_count)
            qCritical()<<"TS: Shit got fkd up during time sync.. msg idx in Received message is false!"
                         " msg.at(2): "<<int(msg[2])<<" != "<<"m_send_repeat_count: "<<m_send_repeat_count;

        if (m_send_repeat_count == TS_MEASURE_MSG_CNT)
        {
            // last msg sent
            m_send_repeat_count = 0;
            calculate_compensation();
        }
        else
        {
            send_time_sync_msg();
        }
        // TODO: also log the processing times!
        break;
        //
    case SENDING_COMPENSATED:
        if (  get_diff_in_us_to_current_ts( m_last_msg_ts ) < m_travelling_time_acceptance_trsh )
        {
            // This was a good one!
            QByteArray tba;
            tba.resize(2);
            tba[0] = TS_MSG;
            tba[1] = TS_CMD_LAST_ONE_WAS_GOOD_ONE;
            m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
            m_sync_state = STOP_WAITS_FOR_ACK;
            m_send_repeat_count = 0;
        }
        else
        {
            if (m_send_repeat_count == TS_COMPENSATED_MSG_CNT)
            {
                // last msg sent
                m_send_repeat_count = 0;
                qCritical()<<"TS: Max retry reached, failed!";
                emit time_sync_failed();
            }
            else
            {
                send_compensated_time_sync_msg();
            }
        }
        break;
        //
    case STOP_WAITS_FOR_ACK:
        if (msg.at(1) != TS_CMD_ACK)
        {
            m_sync_state = STOPPED;
            qCritical()<<"TS: Wrong msg arrived.. we were waiting for an ACK..";
        }
        else
        {
            // we did it!
            qInfo()<<"TS: Sync completed! Current TS: " << get_timestamp_us();
            emit time_sync_completed();
        }
        break;
        //
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

void TimeStampler::timeout_timer_expired()
{
    qCritical()<<"TimeSync Failed - > Timeout! - > Current State : "<<m_sync_state;
    m_sync_state = STOPPED;
}

