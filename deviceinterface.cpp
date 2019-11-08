#include "devicecontroller.h"
#include "deviceinterface.h"
#include "deviceinfo.h"
#include <QBluetoothHostInfo>

DeviceInterface::DeviceInterface (TimeSyncHandler* ts_handler, CatchController* catch_controller,
                                  LogFileHandler* logfile_handler, DeviceInfo *parent):
    DeviceInfo(parent),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler),
    m_catch_controller_ptr(catch_controller)
{
//m_textTimer.setInterval(500);
//connect(&m_textTimer, &QTimer::timeout, this, &DeviceInterface::onTextTimerTrig);
//m_textTimer.setSingleShot(false);
//m_textTimer.start();
}


// TODO we can add relevant information to deviceinfo also - or just keep it general
void DeviceInterface::initializeDevice(QBluetoothHostInfo *hostInfo)
{

    m_deviceController = new DeviceController(this->getDeviceIndex() , this->getDeviceIdentifier()); // we have allocate this dinamically, there is no other way to pass it like this to a thread

    // let the qt magic begin..
    m_deviceController->moveToThread(&m_thread_controller);

    //connect(this, SIGNAL(signal_printThreadId()), &dh, SLOT(printThreadId()), Qt::QueuedConnection); // old syntax
    connect(this, &DeviceInterface::invokePrintThreadId, m_deviceController, &DeviceController::printThreadId, Qt::QueuedConnection); // new syntax

    connect(this, &DeviceInterface::invokeInitializeDevice, m_deviceController, &DeviceController::initializeDevice, Qt::QueuedConnection);

    connect(this, &DeviceInterface::invokeBleUartTx, m_deviceController, &DeviceController::bleUartTx, Qt::QueuedConnection);
    connect(this, &DeviceInterface::invokeBleUartSendCmdOk, m_deviceController, &DeviceController::bleUartSendCmdOk, Qt::QueuedConnection);
    connect(this, &DeviceInterface::invokeBleUartSendCmdWithResp, m_deviceController, &DeviceController::bleUartSendCmdWithResp, Qt::QueuedConnection);

    connect(m_deviceController, &DeviceController::timeSyncMessageArrived, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_arrived, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::timeSyncMsgSent, m_timesync_handler_ptr, &TimeSyncHandler::slot_time_sync_msg_sent, Qt::QueuedConnection);

    connect(m_deviceController, &DeviceController::aliveArrived, this, &DeviceInterface::onAliveArrived, Qt::QueuedConnection);

    connect(m_deviceController, &DeviceController::invokeWriteTypeToFile, m_logfile_handler_ptr, &LogFileHandler::write_type_to_file_slot, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::invokeAddToLogFile, m_logfile_handler_ptr, &LogFileHandler::add_to_log_fil_slot, Qt::QueuedConnection);

    connect(&m_thread_controller, &QThread::started, this, &DeviceInterface::onDeviceThreadStarted); // new syntax

    connect(this, &DeviceInterface::mainStateOfDevXChanged, m_catch_controller_ptr, &CatchController::onMainStateOfDevXChanged, Qt::DirectConnection);
    connect(this, &DeviceInterface::invokeStartConnModeChangeProcedure, m_deviceController, &DeviceController::startConnModeChangeProcedure, Qt::QueuedConnection);
    connect(this, &DeviceInterface::invokeStartDownloadAllDataProcedure, m_deviceController, &DeviceController::startDownloadAllDataProcedure, Qt::QueuedConnection);
//    connect(this, &DeviceInterface::invokeSendCmdStart, m_deviceController, &DeviceController::sendStartToDevice, Qt::QueuedConnection);

    connect(m_deviceController, &DeviceController::connectionAlive, m_catch_controller_ptr, &CatchController::onConnAliveOfDevXChanged, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::requestedConnModeReached, m_catch_controller_ptr, &CatchController::onConnUpdateOfDevXfinished, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::allDataDownloaded, m_catch_controller_ptr, &CatchController::onDownloadOfDeviceXfinished, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::requestDispatchToOtherDevices, m_catch_controller_ptr, &CatchController::onRequestDispatchToOtherDevices, Qt::QueuedConnection);
    connect(m_deviceController, &DeviceController::sensorDataAvailableArrived, m_catch_controller_ptr, &CatchController::onSensorDataAvailableArrived, Qt::QueuedConnection);


    m_thread_controller.start();

#ifdef USE_DEBUG
    QBluetoothDeviceInfo tdi = this->getDevice(); // todo test if we cann pass it directly
    qDebug()<<"The name is there"<<this->getName();
#endif
    emit invokePrintThreadId();

    emit invokeInitializeDevice(hostInfo, &m_device);
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

    m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"TS in Trigger MSG", QString::number(rec_ts));
    //
    if (data[6] == 1<<1)
        m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"Trigger Source", "MAG");
    else if (data[6] == 1<<2)
        m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"Trigger Source", "ACC");
    else
        m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"Trigger Source", "Things got messed up..");

    m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"Trigger MSG Received", QString::number(m_timesync_handler_ptr->get_timestamp_us()));
}

void DeviceInterface::onAliveArrived(QByteArray value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    alive_msg_t * tptr;
    alive_msg_t lastAliveMsg = alive_msg;
    tptr =  (alive_msg_t *) &data[1];
    alive_msg = *tptr;

    if (lastAliveMsg.isSDMounted)
        qDebug()<<"SD mounted";

    if (lastAliveMsg.mainState != alive_msg.mainState)
    {
        this->setDeviceMainState( stateToString(tptr->mainState) );
        emit mainStateOfDevXChanged(tptr->mainState, this->getDeviceIndex());
    }

    if (alive_msg.lastError)
        m_logfile_handler_ptr->add_to_log_fil_slot(this->getDeviceIdentifier(),"Last Error", QString(alive_msg.lastError)); // there is something bad!

    qDebug()<<this->getDeviceIndex()<<this->getDeviceIdentifier()<<" --- ALIVE: -STATE- "<<this->getDeviceMainState()<<" -LAST ERROR- "<<alive_msg.lastError;

    // TODO: SD!

    //setInfo("Alive!");
}


void DeviceInterface::sendCmdStop()
{
        QByteArray tba;
        tba.resize(1);
        tba[0] = CMD_STOP;
        invokeBleUartSendCmdWithResp(tba);
}

void DeviceInterface::sendCmdStart()
{
    QByteArray tba;
    qDebug()<<"Sending START"<<getDeviceIndex();
    tba.resize(1);
    tba[0] = CMD_START;
    invokeBleUartSendCmdWithResp(tba);
}

void DeviceInterface::sendCmdSetLoggingMedia(bool savingTosdEnabled, bool bleUplEnabled)
{
    QByteArray tba;
    qDebug()<<"Sending Set Logging Media";
    tba.resize(3);
    tba[0] = CMD_SET_LOGGING_MEDIA;
    tba[1] = savingTosdEnabled;
    tba[2] = bleUplEnabled;
    invokeBleUartSendCmdWithResp(tba);
}

void DeviceInterface::sendCmdWriteCatchSuccessToSd(const quint8 success)
{
    // todo this is just thrown in
    QByteArray tba;
    tba.resize(2);
    tba[0] = CMD_WRITE_CATCH_SUCCESS;
    tba[1] = success;
    invokeBleUartSendCmdWithResp(tba);
}

quint8 DeviceInterface::getLastMainState()
{
    return alive_msg.mainState;
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
