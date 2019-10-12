#include "devicecontroller.h"
#include "deviceinterface.h"
#include "deviceinfo.h"
#include <QBluetoothHostInfo>

DeviceInterface::DeviceInterface (TimeSyncHandler* ts_handler, CatchController* catch_controller,
                                  LogFileHandler* logfile_handler, int idx, BluetoothBaseClass *parent):
    BluetoothBaseClass(parent),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler),
    m_catch_controller_ptr(catch_controller), (m_dev_nfo.idx)(idx)

{
    m_dev_handler_ptr = new DeviceController(m_device_idx, m_ident_str); // we have allocate this dinamically, there is no other way to pass it like this to a thread
}

// TODO we can add relevant information to deviceinfo also - or just keep it general
void DeviceInterface::initializeDevice(QBluetoothHostInfo *hostInfo, DeviceInfo *deviceInfo)
{
    //qRegisterMetaType<QBluetoothDeviceInfo>();
    m_dev_handler_ptr->moveToThread(&m_thread_controller);
    //connect(this, SIGNAL(signal_printThreadId()), &dh, SLOT(printThreadId()), Qt::QueuedConnection); // old syntax
    connect(this, &DeviceInterface::signal_printThreadId, m_dev_handler_ptr, &DeviceController::slot_printThreadId, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::deviceInitializationSignal, m_dev_handler_ptr, &DeviceController::deviceInitializationSlot, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::ble_uart_tx_sig, m_dev_handler_ptr, &DeviceController::ble_uart_tx, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::ble_uart_send_cmd_ok_sig, m_dev_handler_ptr, &DeviceController::ble_uart_send_cmd_ok, Qt::QueuedConnection); // new syntax
    connect(this, &DeviceInterface::ble_uart_send_cmd_with_resp_sig, m_dev_handler_ptr, &DeviceController::ble_uart_send_cmd_with_resp, Qt::QueuedConnection); // new syntax

    connect(m_dev_handler_ptr, &DeviceController::timeSyncMessageArrived, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_arrived);
    connect(m_dev_handler_ptr, &DeviceController::time_sync_msg_sent, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_sent);


    connect(m_dev_handler_ptr, &DeviceController::write_type_to_file_sig, m_logfile_handler_ptr, &LogFileHandler::write_type_to_file_slot);
    connect(m_dev_handler_ptr, &DeviceController::add_to_log_fil_sig, m_logfile_handler_ptr, &LogFileHandler::add_to_log_fil_slot);

    connect(&m_thread_controller, &QThread::started, this, &DeviceInterface::onDeviceThreadStarted); // new syntax


    qDebug()<<"is adapterAddr here ok?!?!?!?"<<hostInfo;
    m_thread_controller.start();

    deviceInfo = deviceInfo;

    emit signal_printThreadId();
    emit deviceInitializationSignal(hostInfo, deviceInfo); // modify to only use a part of it..
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

    m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"TS in Trigger MSG", QString::number(rec_ts));
    //
    if (data[6] == 1<<1)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"Trigger Source", "MAG");
    else if (data[6] == 1<<2)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"Trigger Source", "ACC");
    else
        m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"Trigger Source", "Things got messed up..");
    // TODO this is not threadsafe
    m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"Trigger MSG Received", QString::number(m_timesync_handler_ptr->get_timestamp_us()));
}

void DeviceInterface::onAliveArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    alive_msg_t * tptr;
    tptr =  (alive_msg_t *) &data[0];
    alive_msg = *tptr;
    m_dev_nfo.mainState = stateToString(alive_msg.main_state);

    if (alive_msg.last_error)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_dev_nfo.ident_str,"Last Error", QString(data[4]));

    qDebug()<<m_dev_nfo.ident_str<" --- ALIVE: -STATE- "<<m_dev_nfo.mainState<<" -SUB STATE- "<<alive_msg.sub_state<<" -LAST ERROR- "<<alive_msg.last_error;

    // TODO: SD!

    setInfo("Alive!");
}
