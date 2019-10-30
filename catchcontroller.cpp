#include "catchcontroller.h"


CatchController::CatchController(QList<DeviceInterface*>* devicelist, TimeSyncHandler* ts_handler,
                                 LogFileHandler* logfile_handler, QObject *parent):
    QObject(parent),
    m_device_list(devicelist),
    m_timesync_handler_ptr(ts_handler),
    m_logfile_handler_ptr(logfile_handler)
{
    connect(&m_timesyncTimer, &QTimer::timeout, this, &CatchController::onTimeSyncTimerExpired);
    connect(&m_downloadTimer, &QTimer::timeout, this, &CatchController::onDownloadTimerExpired);
    connect(m_timesync_handler_ptr, &TimeSyncHandler::time_sync_finished, this, &CatchController::onTimeSyncOfDevXfinished);


}

//bool CatchController::sdEnabled() const
//{
//    return m_sdEnabled;
//}


/* TIME SYNC MULTIMPLE DEVICES */
//
//
//

void CatchController::onTimeSyncTimerExpired()
{
    switch (time_sync_state)
    {
    case SETTING_CONN_MODE:
        qDebug()<<"TS-> Setting Conn Mode failed!"; // show the devil..
        break;
    case SYNCING:
        qDebug()<<"TS-> Sync failed!"; // show the devil..
        break;
    }
    m_timesyncTimer.stop();
}

void CatchController::onTimeSyncOfDevXfinished(bool success, int id)
{
    if ( success )
    {
        qDebug()<<"CC --> onTimeSyncOfDevXfinished"<<success<<id; // show the devil they must have a look at it...

        m_device_list->at(id)->deviceIsTimeSynced = true;
    }
    else
    {
        qDebug()<<"ERROR! CC --> onTimeSyncOfDevXfinished"<<success<<id; // show the devil they must have a look at it...
    }
    if ( (id + 1) < m_device_list->size() )
    {   // this relies on it that we started with 0 and continue upwards..
        id_in_sync = id + 1;
        this->startTimeSyncOfDevX( id_in_sync );

    }
    else
    {
        m_timesyncTimer.stop();
        time_sync_state = NOT_RUNNING;
        emit timeSyncOfAllDevFinished(true);
    }
}

void CatchController::startTimeSyncOfDevX(int id)
{
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    qDebug()<<"BlueZ D->BUS API does not really support connection change from peripheral side!";
    qDebug()<<"We can set conn parameters permanently on Linux side in kernel/debug/bluetooth/hci0 or via hcitool";
    qDebug()<<"Starting TS OF DEV"<<id;
    m_timesyncTimer.stop();
    m_timesyncTimer.setInterval(4000);
    m_timesyncTimer.start();
    time_sync_state = SYNCING;
    m_timesync_handler_ptr->start_time_sync(id);
#endif
#if (defined(Q_OS_ANDROID))
    m_device_list->at(id)->invokeStartConnModeChangeProcedure(FAST);

    if ( change_conn_of_other_devices)
    {
        for (int i = 0; i< m_device_list->size(); i++)
        {
            if ( i != id)
                m_device_list->at(id)->invokeStartConnModeChangeProcedure(SLOW);
        }
        remaining_c = m_device_list->size();
    }
    else
    {
        remaining_c = 1;
    }
    time_sync_state = SETTING_CONN_MODE;
    m_timesyncTimer.setInterval(8500);
    m_timesyncTimer.start();
#endif
}

void CatchController::startTimesyncAllDevices()
{
    id_in_sync = 0;
    startTimeSyncOfDevX(id_in_sync);
}

//
//
//
/* SHARED BY TS, DL, START */
//
//
//

void CatchController::onConnUpdateOfDevXfinished(bool success, int id)
{
    qDebug()<<"onConnUpdateOfDevXfinished()   "<<"succ"<<success<<"id"<<id;
    if (success)
    {
        m_device_list->at(id)->deviceIsInRequiredConnectionState = true;
    }
    else
    {
        remaining_c = 0; // TODO -> thia is stupid...
        qDebug()<<"CC -->  FAILED! onConnUpdateOfDevXfinished"<<success<<id; // show the devil if its till unknown to the audience..
    }
    remaining_c--;
    if ( !remaining_c )
    {
        if (time_sync_state == SETTING_CONN_MODE)
        {
            qDebug()<<"Starting TS OF ALL";
            m_timesyncTimer.stop();
            m_timesyncTimer.setInterval(5000);
            m_timesyncTimer.start();
            time_sync_state = SYNCING;
            m_timesync_handler_ptr->start_time_sync(id_in_sync);
        }
        if ( download_state == SETTING_CONN_MODE )
        {
            qDebug()<<"Starting DL OF ALL";
            m_downloadTimer.stop();
            m_downloadTimer.setInterval(45000);
            m_downloadTimer.start();
            download_state = DOWNLOADING;
            emit m_device_list->at(id_in_dl)->invokeStartDownloadAllDataProcedure();
        }

        if (sendingStartState == SETTING_CONN_MODE )
        {
            sendingStartState = STOPPED;
            for(int i=0; i<m_device_list->size(); i++)
                emit m_device_list->at(i)->sendCmdStart();
        }
    }
}

//
//
//
/* DOWNLOAD DATA FROM MULTIPLE DEVICES */
//
//
//


void CatchController::startDownloadFromAllDevices()
{
    id_in_dl = 0;
    startDownloadOfDevX(id_in_dl);
}


void CatchController::startDownloadOfDevX(int id)
{

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    qDebug()<<"BlueZ D->BUS API does not really support connection change from peripheral side!";
    qDebug()<<"We can set conn parameters permanently on Linux side in kernel/debug/bluetooth/hci0 or via hcitool";
    m_downloadTimer.setInterval(90000);
    m_downloadTimer.start();
    download_state = DOWNLOADING;
    emit m_device_list->at(id_in_dl)->invokeStartDownloadAllDataProcedure();
#endif

#if (defined(Q_OS_ANDROID))
    qDebug()<<"Android Download Version";
    m_device_list->at(id)->invokeStartConnModeChangeProcedure(FAST);

    ///
    change_conn_of_other_devices = true;
    ///

    if ( change_conn_of_other_devices)
    {
        for (int i = 0; i< m_device_list->size(); i++)
        {
            if ( i != id)
                m_device_list->at(i)->invokeStartConnModeChangeProcedure(SLOW);
        }
        remaining_c = m_device_list->size();
    }
    else
    {
        remaining_c = 1;
    }
    download_state = SETTING_CONN_MODE;

    m_downloadTimer.setInterval(8500 );
    m_downloadTimer.start();
#endif
}

///
/// \brief CatchController::onDownloadOfDeviceXfinished
/// \param success
/// \param id
///
void CatchController::onDownloadOfDeviceXfinished(bool success, int id)
{
    if ( !success )
    {
        qDebug()<<"CC-DL --> onDownloadOfDeviceXfinished"<<success<<id; // show the devil they must have a look at it...
    }
    // todo -> modify this to differentiate by dev type, wearable or not../add this property to deviceinfo in deviceinterface
    if ( (id + 1) < m_device_list->size() )
    {   // this relies on it that we started with 0 and continue upwards..
        id_in_dl = id + 1;
        this->startDownloadOfDevX( id_in_dl );
    }
    else
    {
        m_downloadTimer.stop();
        download_state = NOT_RUNNING;
        emit downloadOfAllDevFinished(true);
        qDebug()<<"Downloaded everything from all connected devices";
    }
}


//
//
//
/* DIVERSE FUNCTIONS */
//
//
//


///
/// \brief CatchController::sendStartToAllDevices
///
void CatchController::sendStartToAllDevices()
{
#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    for(int i=0; i<m_device_list->size(); i++)
        emit m_device_list->at(i)->sendCmdStart();
#endif
#if (defined(Q_OS_ANDROID))
    for (int i = 0; i< m_device_list->size(); i++)
    {
        m_device_list->at(i)->invokeStartConnModeChangeProcedure(FAST);
    }
    remaining_c = m_device_list->size();
    sendingStartState = SETTING_CONN_MODE;
#endif
}

///
/// \brief CatchController::sendStopToAllDevices
///
///
void CatchController::sendStopToAllDevices()
{
    for(int i=0; i<m_device_list->size(); i++)
        emit m_device_list->at(i)->sendCmdStop();
}

///
/// \brief CatchController::onSensorDataAvailableArrived
/// \param idx
///
void CatchController::onSensorDataAvailableArrived(int idx)
{
    emit m_device_list->at(idx)->invokeBleUartSendCmdOk();
    int notReady = 0;
    m_device_list->at(idx)->m_sensorDataWaitingForDownload = true;

    for (int i = 0; i < m_device_list->size(); i++)
        if ( m_device_list->at(i)->getDeviceType() == DeviceInfo::Wearable )
            if (m_device_list->at(i)->m_sensorDataWaitingForDownload != true  )
                notReady++;
    if ( !notReady )
        emit allWearablesAreWaitingForDownload();

    if ( !notReady )
        qDebug()<<"allWearablesAreWaitingForDownload()";
}

///
/// \brief CatchController::onMainStateOfDevXChanged
/// \param state
/// \param idx
///
void CatchController::onMainStateOfDevXChanged(quint16 state, int idx)
{
    bool notTheSame = false;
    for (int i = 0; i<m_device_list->size(); i++)
    {
        if ( i != idx )
        {
            if ( m_device_list->at(i)->getLastMainState() != state )
            {
                notTheSame = true;
                break;
            }
        }
    }
    if ( !notTheSame )
    {
        emit mainStateOfAllDevicesChanged(stateToString(state));
        qDebug()<<"MainStateOfAllDevicesChanged";
    }
}

///
/// \brief CatchController::onDownloadTimerExpired
///
void CatchController::onDownloadTimerExpired()
{
    switch (download_state)
    {
    case SETTING_CONN_MODE:
        qDebug()<<"CC-DL-> Setting Conn Mode failed!"; // show the devil..
        break;
    case DOWNLOADING:
        qDebug()<<"CC-DL-> "
                  "Download failed!"; // show the devil..
        break;
    }
    m_downloadTimer.stop();
}


///
/// \brief CatchController::onRequestDispatchToOtherDevices
/// \param value
/// \param idx
///
void CatchController::onRequestDispatchToOtherDevices(QByteArray value, int idx)
{
    for ( int i=0; i<m_device_list->size(); i++)
    {
        if (i != idx)
            m_device_list->at(i)->invokeBleUartSendCmdWithResp(value);
    }
}

///
/// \brief CatchController::onConnAliveOfDevXChanged
/// \param isItAlive
/// \param idx
///

void CatchController::onConnAliveOfDevXChanged(bool isItAlive, int idx)
{
    bool missing = false;
    m_device_list->at(idx)->setConnectionAlive(isItAlive);
    if (!isItAlive)
    {
        m_devicesConnected = false;
        emit allSelectedDevicesAreConnected(false);
        return;
    }


    for (int i=0; i<m_device_list->size(); i++)
    {
        if ( !m_device_list->at(i)->connectionAlive() )
            missing = true;
    }
    if (!missing)
    {
        m_devicesConnected = true;
        emit allSelectedDevicesAreConnected(true);
    }

    qDebug()<<"onConnAliveOfDevXChanged(bool isItAlive ="<<isItAlive<<"idx="<<idx<<")";
}


///
/// \brief CatchController::setLoggingMedia
/// \param toSd
/// \param sendOverBle
///
void CatchController::setLoggingMedia(bool toSd, bool sendOverBle)
{
    for ( int i=0; i<m_device_list->size(); i++)
    {
        if (m_device_list->at(i)->getDeviceType() == DeviceInfo::Wearable)
            m_device_list->at(i)->sendCmdSetLoggingMedia(toSd, sendOverBle);
    }
}

