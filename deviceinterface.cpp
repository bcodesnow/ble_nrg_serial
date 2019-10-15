#include "devicecontroller.h"
#include "deviceinterface.h"
#include "deviceinfo.h"
#include <QBluetoothHostInfo>

DeviceInterface::DeviceInterface (TimeSyncHandler* ts_handler, CatchController* catch_controller,
                                  LogFileHandler* logfile_handler, BluetoothBaseClass *parent):
    BluetoothBaseClass(parent),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler),
    m_catch_controller_ptr(catch_controller)
{
}

// TODO we can add relevant information to deviceinfo also - or just keep it general
void DeviceInterface::initializeDevice(QBluetoothHostInfo *hostInfo, DeviceInfo *deviceInfo)
{
    m_dev_handler_ptr = new DeviceController(deviceInfo->getDeviceIndex() , deviceInfo->getDeviceIdentifier()); // we have allocate this dinamically, there is no other way to pass it like this to a thread


    m_dev_handler_ptr->moveToThread(&m_thread_controller);

    // let the qt magic begin..
    //connect(this, SIGNAL(signal_printThreadId()), &dh, SLOT(printThreadId()), Qt::QueuedConnection); // old syntax
    connect(this, &DeviceInterface::invokePrintThreadId, m_dev_handler_ptr, &DeviceController::printThreadId, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::invokeInitializeDevice, m_dev_handler_ptr, &DeviceController::initializeDevice, Qt::QueuedConnection);

    connect(this, &DeviceInterface::invokeBleUartTx, m_dev_handler_ptr, &DeviceController::bleUartTx, Qt::QueuedConnection);
    connect(this, &DeviceInterface::invokeBleUartSendCmdOk, m_dev_handler_ptr, &DeviceController::bleUartSendCmdOk, Qt::QueuedConnection);
    connect(this, &DeviceInterface::invokeBleUartSendCmdWithResp, m_dev_handler_ptr, &DeviceController::bleUartSendCmdWithResp, Qt::QueuedConnection);

    connect(m_dev_handler_ptr, &DeviceController::timeSyncMessageArrived, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_arrived, Qt::QueuedConnection);
    connect(m_dev_handler_ptr, &DeviceController::timeSyncMsgSent, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_sent, Qt::QueuedConnection);


    connect(m_dev_handler_ptr, &DeviceController::invokeWriteTypeToFile, m_logfile_handler_ptr, &LogFileHandler::write_type_to_file_slot, Qt::QueuedConnection);
    connect(m_dev_handler_ptr, &DeviceController::invokeAddToLogFile, m_logfile_handler_ptr, &LogFileHandler::add_to_log_fil_slot, Qt::QueuedConnection);

    connect(&m_thread_controller, &QThread::started, this, &DeviceInterface::onDeviceThreadStarted); // new syntax


    qDebug()<<"is adapterAddr here ok?!?!?!?"<<hostInfo;
    m_thread_controller.start();

    m_deviceInfo = deviceInfo;

    emit invokePrintThreadId();

    emit invokeInitializeDevice(hostInfo, deviceInfo);
}

void DeviceInterface::onDeviceThreadStarted()
{
    qDebug()<<"Device Thread Started!";
}

void DeviceInterface::onTriggeredArrived(QByteArray value)
{
    // todo
    //            if (m_sdEnabled)
    //                m_refToFileHandler->add_to_log_fil(m_ident_str,"File ID on Device", QString::number(data[1]));

    uint32_t rec_ts;
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());

    rec_ts = 0;
    rec_ts = ( (uint32_t) data[2] ) << 24;
    rec_ts |=( (uint32_t) data[3] )<< 16;
    rec_ts |=( (uint32_t) data[4] )<< 8;
    rec_ts |= ( (uint32_t) data[5] );

    m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"TS in Trigger MSG", QString::number(rec_ts));
    //
    if (data[6] == 1<<1)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"Trigger Source", "MAG");
    else if (data[6] == 1<<2)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"Trigger Source", "ACC");
    else
        m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"Trigger Source", "Things got messed up..");

    m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"Trigger MSG Received", QString::number(m_timesync_handler_ptr->get_timestamp_us()));
}

void DeviceInterface::onAliveArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    alive_msg_t * tptr;
    tptr =  (alive_msg_t *) &data[0];
    alive_msg = *tptr;
    m_deviceInfo->setDeviceMainState( stateToString(alive_msg.main_state) );

    if (alive_msg.last_error)
        m_logfile_handler_ptr->add_to_log_fil_slot(m_deviceInfo->getDeviceIdentifier(),"Last Error", QString(data[4]));

    qDebug()<<m_deviceInfo->getDeviceIdentifier()<<" --- ALIVE: -STATE- "<<m_deviceInfo->getDeviceMainState()<<" -SUB STATE- "<<alive_msg.sub_state<<" -LAST ERROR- "<<alive_msg.last_error;

    // TODO: SD!

    setInfo("Alive!");
}



void DeviceInterface::sendCmdStart()
{
        QByteArray tba;
        tba.resize(1);
        tba[0] = CMD_START;
        invokeBleUartSendCmdWithResp(tba);
}

void DeviceInterface::sendCmdStop()
{
        QByteArray tba;
        tba.resize(1);
        tba[0] = CMD_STOP;
        invokeBleUartSendCmdWithResp(tba);
}

//void DeviceInterface::sendCmdEnableSdLogging(bool enable)
//{
    //    QByteArray tba;
    //    tba.resize(2);
    //    tba[0] = TURN_ON_SD_LOGGING;
    //    tba[1] = enable;
    //    if (m_initializedDevicesList[0] == true)
    //        m_deviceHandler[0].ble_uart_tx(tba);
    //    if (m_initializedDevicesList[1] == true)
    //        m_deviceHandler[1].ble_uart_tx(tba);
//}
