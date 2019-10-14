#include "catchcontroller.h"


CatchController::CatchController(QList<DeviceInterface*>* devicelist, TimeSyncHandler* ts_handler,
                                 LogFileHandler* logfile_handler, QObject *parent):
          QObject(parent),
          m_device_list(devicelist),
          m_timesync_handler_ptr(ts_handler),
          m_logfile_handler_ptr(logfile_handler)
{

}

bool CatchController::sdEnabled() const
{
    return m_sdEnabled;
}

quint8 time_sync_state;
int    remaining_c;
int    id_in_sync;
bool change_conn_of_other_devices;
#define SETTING_CONN_MODE 1u
#define SYNCING           2u
#define DOWNLOADING       2u
#define NOT_RUNNING       3u

/* TIME SYNC MULTIMPLE DEVICES */
//
//
//

void CatchController::onTimeSyncTimerExpired()
{
    switch (time_sync_state)
    {
        case SETTING_CONN_MODE:
            break;
        case SYNCING:
            break;
    }
    // show the devil..
}

void CatchController::onTimeSyncOfDevXfinished(bool success, int id)
{
    if ( success )
    {
        m_device_list->at(id)->deviceInfo->deviceIsTimeSynced = true;
    }
    else
    {
        qDebug()<<"CC --> onTimeSyncOfDevXfinished"<<success<<id; // show the devil they must have a look at it...
    }
    if ( id++ < m_device_list->size() )
    {   // this relies on it that we started with 0 and continue upwards..
        this->startTimeSyncOfDevX(id++);
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
    m_device_list->at(id)->set_peri_conn_mode_sig(FAST);

    if ( change_conn_of_other_devices)
    {
         for (int i = 0; i< m_device_list->size(); i++)
         {
             if ( i != id)
                 m_device_list->at(id)->set_peri_conn_mode_sig(SLOW);
         }
         remaining_c = m_device_list->size();
    }
    else
    {
        remaining_c = 1;
    }
    time_sync_state = SETTING_CONN_MODE;

    m_timesyncTimer.singleShot(2000, this, &CatchController::onTimeSyncTimerExpired);
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
    if (success)
    {
        m_device_list->at(id)->deviceInfo->deviceIsInRequiredConnectionState = true;
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
            m_timesyncTimer.stop();
            m_timesyncTimer.singleShot(4000, this, &CatchController::onTimeSyncTimerExpired);
            time_sync_state = SYNCING;
            m_timesync_handler_ptr->start_time_sync(id_in_sync);
        }
        if ( dl_state == SETTING_CONN_MODE )
        {
            m_downloadTimer.stop();
            m_downloadTimer.singleShot(20000, this, &CatchController::onTimeSyncTimerExpired);
            dl_state = DOWNLOADING;
            m_device_list-->request_sensordata...        }
    }
}

//
//
//
/* DOWNLOAD DATA FROM MULTIPLE DEVICES */
//
//
//

int id_in_dl;


void CatchController::startDownloadFromAllDevices()
{
    id_in_dl = 0;
    startDownloadOfDevX(id_in_dl)
}

void CatchController::startDownloadOfDevX(int id)
{
    m_device_list->at(id)->set_peri_conn_mode_sig(FAST);

    if ( change_conn_of_other_devices)
    {
         for (int i = 0; i< m_device_list->size(); i++)
         {
             if ( i != id)
                 m_device_list->at(id)->set_peri_conn_mode_sig(SLOW);
         }
         remaining_c = m_device_list->size();
    }
    else
    {
        remaining_c = 1;
    }
    time_sync_state = SETTING_CONN_MODE;

    m_downloadTimer.singleShot(2000, this, &CatchController::onDownloadTimerExpired);

}

void CatchController::onDownloadTimerExpired()
{

}

void CatchController::onDownloadOfDeviceXfinished(bool success, int id)
{

}

//
//
//
