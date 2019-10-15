#ifndef CATCHCONTROLLER_H
#define CATCHCONTROLLER_H

#include <QObject>
#include <timesynchandler.h>
#include <logfilehandler.h>
#include <deviceinterface.h>

class LogFileHandler;
class TimeSyncHandler;
class DeviceInterface;

// this class should be known to both devicehandlers, and he should also know both of them.. should act as middleman, in timesync, appmainstate, sd turn onof etc things.

class CatchController : public QObject
{
    Q_OBJECT
    //    Q_PROPERTY(quint8 appState MEMBER m_appState NOTIFY appStateChanged)
    //    Q_PROPERTY(bool sdEnabled READ sdEnabled NOTIFY sdEnabledChanged)

    //Q_PROPERTY(QString lastPath MEMBER m_last_path NOTIFY lastPathChanged)

private:
    //quint8 m_appState;
    //bool m_sdEnabled;

    QTimer m_timesyncTimer;
    QTimer m_downloadTimer;

    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    QList<DeviceInterface*>* m_device_list;

public:
    CatchController(QList<DeviceInterface*>* devicelist, TimeSyncHandler* ts_handler,
                    LogFileHandler* logfile_handler, QObject *parent = nullptr);
    ~CatchController();


    //bool sdEnabled() const;


signals:
    //    void appStateChanged (quint8 state);
    //    void sdEnabledChanged();

    void timeSyncOfAllDevFinished(bool success);
    void downloadOfAllDevFinished(bool success);

    void allWearablesAreWaitingForDownload();

    //    void transferProgressChanged(uint8_t percentage);
    //    void showProgressMessage(QString mainText, QString subText, int percent, uint8_t flag);
    //    void progressFinished();


public slots:

    void startTimesyncAllDevices();
    void onTimeSyncTimerExpired();
    void onTimeSyncOfDevXfinished(bool success, int idx);
    void startTimeSyncOfDevX(int id);

    void startDownloadFromAllDevices();
    void onDownloadTimerExpired();
    void onDownloadOfDeviceXfinished(bool success, int idx);
    void startDownloadOfDevX(int id);

    void onConnUpdateOfDevXfinished(bool success, int idx); // todo connect both of them

    void sendStartToAllDevices();
    void sendStopToAllDevices();

    void onSensorDataAvailableArrived(int idx)
    {
        int notReady = 0;
        m_device_list->at(idx)->m_deviceInfo->m_sensorDataWaitingForDownload = true;

        foreach (DeviceInterface* it, m_device_list)
            if ( it->m_deviceInfo->getDeviceType() == DeviceInfo::Wearable )
                if (it->m_deviceInfo->m_sensorDataWaitingForDownload != true  )
                    notReady++;
        if ( !notReady )
            emit allWearablesAreWaitingForDownload();
    }




    //#define WAITING_FOR_CONNECTION_TO_COMPLETE 1u
    //#define
    //    void handleAppState()
    //    {
    //        switch (m_appState)
    //        {
    //        case:

    //        }
    //    }
};

#endif // CATCHCONTROLLER_H
