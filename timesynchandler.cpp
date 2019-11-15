#include "timesynchandler.h"
#include "devicecontroller.h"
#include <QTimer>
#include <QObject>
#include <algorithm> // min&max..
#include <QThread>

TimeSyncHandler::TimeSyncHandler(QList<DeviceInterface*>* device_interfaces, QObject *parent):
    m_device_interfaces_ptr(device_interfaces)
{
    Q_UNUSED(parent)
    connect(&m_timeout_timer, SIGNAL(timeout()), this, SLOT(timeout_timer_expired()) );
    m_timeout_timer.setSingleShot(true);
    m_timeout_timer.setInterval(TS_TIMEOUT_DELAY_MS);
}

void TimeSyncHandler::slot_time_sync_msg_sent(int idx)
{
    // toDo m_last_msg_ts is only relevant if we are mesuring...
    // toDo check if idx is in sync;
#if (VERBOSITY_LEVEL >= 1)
    qInfo()<<"TS: Sync Msg. Sent!"<<"";
#endif
#if ( USE_CHAR_WRITTEN_CALLBACK == 1 )
    m_last_msg_ts = get_timestamp_us();
#endif
}

uint32_t TimeSyncHandler::get_diff_in_us_to_current_ts(uint32_t some_ts)
{
    return ( m_etimer.nsecsElapsed() / 1000 ) - some_ts;
}

//void TimeSyncHandler::setRefToDevHandlerArr(DeviceHandler *dev_handler_arr)
//{
//    m_deviceHandler = dev_handler_arr;
//}

void TimeSyncHandler::start_time_stamp()
{
    m_etimer.start();
}

uint32_t TimeSyncHandler::get_timestamp_ms()
{
    return m_etimer.elapsed();
}

uint32_t TimeSyncHandler::get_timestamp_us()
{
    return ( m_etimer.nsecsElapsed() / 1000 );
}

void TimeSyncHandler::send_time_sync_msg()
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
#if ( VERBOSITY_LEVEL >= 1 )
    qDebug()<<"TS: send_time_sync_msg() -> "<<tstamp;
#endif
    //m_deviceHandler[m_dev_idx_in_sync].ble_uart_tx(tba);
    m_device_interfaces_ptr->at(m_dev_idx_in_sync)->invokeBleUartTx(tba); // emit the signal directly in the interface..
#if ( USE_CHAR_WRITTEN_CALLBACK == 0 )
    m_last_msg_ts = get_timestamp_us();
#endif
    m_timeout_timer.start();
}

void TimeSyncHandler::send_compensated_time_sync_msg()
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
    m_device_interfaces_ptr->at(m_dev_idx_in_sync)->invokeBleUartTx(tba); // emit the signal directly in the interface..

#if ( USE_CHAR_WRITTEN_CALLBACK == 0 )
    m_last_msg_ts = get_timestamp_us();
#endif

    m_timeout_timer.start();

#if ( VERBOSITY_LEVEL >= 2 )
    qDebug()<<"TS: send_compensated_time_sync_msg() -> "<<tstamp;
    qDebug()<<"TS: Amount of Compensation -> "<<( m_travelling_time_acceptance_trsh / 2 );
#endif
}

void TimeSyncHandler::calculate_compensation()
{
    quint32 min = *std::min_element(travelling_times.constBegin(), travelling_times.constEnd());
    quint32 max = *std::max_element(travelling_times.constBegin(), travelling_times.constEnd());
    quint32 avg = std::accumulate(travelling_times.constBegin(), travelling_times.constEnd(), 0) / travelling_times.size() ;

#if (VERBOSITY_LEVEL >= 2)
    qInfo()<<"TS: Travelling Times Calculated: ";
    qInfo()<<"TS: Minimum: "<<min;
    qInfo()<<"TS: Maximum: "<<max;
    qInfo()<<"TS: Avarage: "<<avg;

    qDebug()<<"Removing 3 Smallest and Biggest..";
#endif
    for (int i=0; i<3; i++)
    {
        QVector<quint32>::ConstIterator it =  std::min_element(travelling_times.constBegin(), travelling_times.constEnd());
        travelling_times.removeAt(std::distance(travelling_times.constBegin(),it)); //remove the 3 smallest
        it =  std::max_element(travelling_times.constBegin(), travelling_times.constEnd());
        travelling_times.removeAt(std::distance(travelling_times.constBegin(),it)); //remove the 3 smallest
    }
    min = *std::min_element(travelling_times.constBegin(), travelling_times.constEnd());
    max = *std::max_element(travelling_times.constBegin(), travelling_times.constEnd());
    avg = std::accumulate(travelling_times.constBegin(), travelling_times.constEnd(), 0) / travelling_times.size() ;

    #if (VERBOSITY_LEVEL >= 0)
        qInfo()<<"TS: Travelling Times Calculated: ";
        qInfo()<<"TS: Minimum: "<<min;
        qInfo()<<"TS: Maximum: "<<max;
        qInfo()<<"TS: Avarage: "<<avg;
    #endif

    #if ( USE_CHAR_WRITTEN_CALLBACK == 1)
        m_travelling_time_acceptance_trsh = int ( (float) min *  TS_TRSH_FACTOR );
    #else
        m_travelling_time_acceptance_trsh = min + ( (avg - min) / 2 );
    #endif

    qInfo()<<"Calculated Threshold: "<< m_travelling_time_acceptance_trsh;

    m_sync_state = SENDING_COMPENSATED;
    send_compensated_time_sync_msg();
}

void TimeSyncHandler::slot_time_sync_msg_arrived(const QByteArray &msg)
{
    m_timeout_timer.stop();
    //qDebug()<<"Arrived: "<<msg;
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
#if (VERBOSITY_LEVEL >= 1)
        qDebug()<<"TS: Travelling Time -> "<< travelling_times.last() << " us";
#endif
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
            m_device_interfaces_ptr->at(m_dev_idx_in_sync)->invokeBleUartTx(tba); // emit the signal directly in the interface..
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
                emit time_sync_finished(false, m_dev_idx_in_sync);
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
            qInfo()<<"TS: Sync completed! Current TS: " << get_timestamp_us()<<"It needed"<<m_send_repeat_count+1<<"compensated msgs";

            // TODO ADDDED IN TEST
            travelling_times.clear();
            m_timeout_timer.stop();
            m_send_repeat_count = 0;
            m_sync_state = STOPPED;
            m_last_msg_ts = 0;

            // TODO ADDDED IN TEST
            // TODO ADDDED IN TEST

            emit time_sync_finished(true, m_dev_idx_in_sync);








        }
        break;
        //
    case STOPPED:
        qDebug()<<"TSYNC RECEIVED A MSG IN STOPPED STATE..";
        break;
    default:
        qDebug()<<"TSYNC SWITCH GOT TO DEFAULT";
        break;
    }
}

void TimeSyncHandler::start_time_sync(int devIdxToSync)
{
    m_dev_idx_in_sync = devIdxToSync;
    qDebug()<<"Received ID"<<devIdxToSync<<"to Sync";
    m_sync_state = START_WAITS_FOR_ACK;

    QByteArray tba;
    tba.resize(2);
    tba[0] = TS_MSG;
    tba[1] = TS_CMD_SYNC_START;
    m_device_interfaces_ptr->at(m_dev_idx_in_sync)->invokeBleUartTx(tba); // emit the signal directly in the interface..

    m_timeout_timer.start();
}

void TimeSyncHandler::timeout_timer_expired()
{

    qCritical()<<"TimeSync Failed - > Timeout! - > Current State : "<<m_sync_state;
    m_sync_state = STOPPED;
}

