#include "devicehandler.h"
#include "deviceinterface.h"
#include "deviceinfo.h"
#include <QBluetoothHostInfo>

DeviceInterface::DeviceInterface (TimeSyncHandler* ts_handler, CatchController* catch_controller,
                                  LogFileHandler* logfile_handler, int idx, QObject *parent):
    QObject(parent),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler),
    m_catch_controller_ptr(catch_controller), m_device_idx(idx)

{
    m_dev_handler_ptr = new DeviceHandler(); // we have allocate this dinamically, there is no other way to pass it like this to a thread
}

// TODO we can add relevant information to deviceinfo also - or just keep it general
void DeviceInterface::init_device( QBluetoothHostInfo* hostInfo, DeviceInfo* deviceInfo )
{
    //qRegisterMetaType<QBluetoothDeviceInfo>();
    m_dev_handler_ptr->moveToThread(&m_thread_controller);
    //connect(this, SIGNAL(signal_printThreadId()), &dh, SLOT(printThreadId()), Qt::QueuedConnection); // old syntax
    connect(this, &DeviceInterface::signal_printThreadId, m_dev_handler_ptr, &DeviceHandler::slot_printThreadId, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::sig_init, m_dev_handler_ptr, &DeviceHandler::init_slot, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::ble_uart_tx_sig, m_dev_handler_ptr, &DeviceHandler::ble_uart_tx, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::ble_uart_send_cmd_ok_sig, m_dev_handler_ptr, &DeviceHandler::ble_uart_send_cmd_ok, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::ble_uart_send_cmd_with_resp_sig, m_dev_handler_ptr, &DeviceHandler::ble_uart_send_cmd_with_resp, Qt::QueuedConnection); // new syntax

    connect(m_dev_handler_ptr, &DeviceHandler::timeSyncMessageArrived, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_arrived);
    connect(m_dev_handler_ptr, &DeviceHandler::time_sync_msg_sent, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_sent);

    connect(&m_thread_controller, &QThread::started, this, &DeviceInterface::onDeviceThreadStarted); // new syntax


    qDebug()<<"is adapterAddr here ok?!?!?!?"<<hostInfo;
    m_thread_controller.start();

    emit signal_printThreadId();
    emit sig_init(hostInfo, deviceInfo);
}

void DeviceInterface::onDeviceThreadStarted()
{
    qDebug()<<"Device Thread Started!";
}



void DeviceInterface::onTriggeredArrived(QByteArray value)
{
    uint32_t rec_ts;
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    rec_ts = 0;
    rec_ts = ( (uint32_t) data[2] ) << 24;
    rec_ts |=( (uint32_t) data[3] )<< 16;
    rec_ts |=( (uint32_t) data[4] )<< 8;
    rec_ts |= ( (uint32_t) data[5] );

    m_refToFileHandler->add_to_log_fil(m_ident_str,"TS in Trigger MSG", QString::number(rec_ts));
    //
    if (data[6] == 1<<1)
        m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "MAG");
    else if (data[6] == 1<<2)
        m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "ACC");
    else
        m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger Source", "Things got messed up..");
    // TODO this is not threadsafe
    m_refToFileHandler->add_to_log_fil(m_ident_str,"Trigger MSG Received", QString::number(m_refToTimeStampler->get_timestamp_us()));
}
