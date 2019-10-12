#ifndef CATCHCONTROLLER_H
#define CATCHCONTROLLER_H

#include <QObject>
#include <timesynchandler.h>
#include <logfilehandler.h>

class LogFileHandler;
class TimeSyncHandler;

// this class should be known to both devicehandlers, and he should also know both of them.. should act as middleman, in timesync, appmainstate, sd turn onof etc things.

class CatchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint8 appState MEMBER m_appState NOTIFY appStateChanged)
    Q_PROPERTY(bool sdEnabled READ sdEnabled NOTIFY sdEnabledChanged)

    //Q_PROPERTY(QString lastPath MEMBER m_last_path NOTIFY lastPathChanged)

private:
    quint8 m_appState;
    bool m_sdEnabled;

    QTimer m_timesyncTimer;

    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    QList<DeviceInterface*>* m_device_list;

public:
    CatchController(QList<DeviceInterface*>* devicelist, TimeSyncHandler* ts_handler,
                            LogFileHandler* logfile_handler, QObject *parent = nullptr);
    ~CatchController();


    bool sdEnabled() const;


signals:
    void appStateChanged (quint8 state);
    void sdEnabledChanged();

    void timeSyncOfAllDevFinished(bool success);


public slots:
    void startTimesyncAllDevices();
    void onTimeSyncTimerExpired();
    void onTimeSyncOfDevXfinished(bool success, int id);
    void onConnUpdateOfDevXfinished(bool success, int id); // todo connect both of them
    void startTimeSyncOfDevX(int id);

   void startDownloadFromAllDevices();

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
