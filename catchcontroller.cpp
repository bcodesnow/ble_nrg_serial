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
}

void CatchController::onTimeSyncOfDevXfinished(bool success, int id)
{
    if ( success )
    {
        m_device_list->at(id)->deviceIsTimeSynced = true;
    }
    else
    {
        qDebug()<<"CC --> onTimeSyncOfDevXfinished"<<success<<id; // show the devil they must have a look at it...
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
    m_timesyncTimer.setInterval(200);
    m_timesyncTimer.start();
}

void CatchController::startTimesyncAllDevices()
{
    id_in_sync = 0;
    startTimeSyncOfDevX(id_in_sync);
}

//
//
//
/* SHARED BY TS AND DL */
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
        remaining_c = 0;
        qDebug()<<"CC --> onConnUpdateOfDevXfinished"<<success<<id; // show the devil if its till unknown to the audience..
    }
    remaining_c--;
    if ( !remaining_c )
    {
        if (time_sync_state == SETTING_CONN_MODE)
        {
            qDebug()<<"Starting TS OF ALL";
            m_timesyncTimer.stop();
            m_timesyncTimer.setInterval(4000);
            m_timesyncTimer.start();
            time_sync_state = SYNCING;
            m_timesync_handler_ptr->start_time_sync(id_in_sync);
        }
        if ( download_state == SETTING_CONN_MODE )
        {
            qDebug()<<"Starting DL OF ALL";
            m_downloadTimer.stop();
            m_downloadTimer.setInterval(20000);
            m_downloadTimer.start();
            download_state = DOWNLOADING;
            emit m_device_list->at(id_in_dl)->invokeStartDownloadAllDataProcedure();
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

    #if (defined(Q_OS_LINUX))
    qDebug()<<"Linux Download Version -> SHOW A BIT THE DEVIL, as it will be slow as f**k!";
    qDebug()<<"BlueZ D->BUS API DOES NOT REALLY SUPPORT CONN PARAM UPDATE!";
    m_downloadTimer.setInterval(20000);
    m_downloadTimer.start();
    download_state = DOWNLOADING;
    emit m_device_list->at(id_in_dl)->invokeStartDownloadAllDataProcedure();
    #endif

    #if (defined(Q_OS_ANDROID))
    qDebug()<<"Android Download Version";
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
    download_state = SETTING_CONN_MODE;

    m_downloadTimer.setInterval(2000);
    m_downloadTimer.start();
    #endif
}

void CatchController::sendStartToAllDevices()
{
    for(int i=0; i<m_device_list->size(); i++)
        emit m_device_list->at(i)->sendCmdStart();
}

void CatchController::sendStopToAllDevices()
{
    for(int i=0; i<m_device_list->size(); i++)
        emit m_device_list->at(i)->sendCmdStop();
}

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
    }
}

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
}

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
    }
}

//
//
//
