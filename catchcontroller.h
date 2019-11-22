#ifndef CATCHCONTROLLER_H
#define CATCHCONTROLLER_H

#include "global.h"
#include <QObject>
#include <QTimer>

#define START_TS_FROM_CC 1

#define SETTING_CONN_MODE 1u
#define SYNCING           2u
#define DOWNLOADING       2u
#define NOT_RUNNING       3u

#define NEXT_REQ_START_TS 1u

class DeviceInterface;
class LogFileHandler;
class TimeSyncHandler;
// this class should be known to both devicehandlers, and he should also know both of them.. should act as middleman, in timesync, appmainstate, sd turn onof etc things.

class CatchController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString devicesMainState MEMBER m_devicesMainState NOTIFY mainStateOfAllDevicesChanged)
    Q_PROPERTY(bool devicesConnected READ devicesConnected NOTIFY allSelectedDevicesAreConnected)
    Q_PROPERTY(bool sdEnabled READ sdEnabled NOTIFY sdEnabledChanged)
    Q_PROPERTY(bool bleUplEnabled READ bleUplEnabled NOTIFY bleUplEnabledChanged) // wenn du magst kannst daraus member machen und aus qml setzen

private:
    //quint8 m_appState;
    //bool m_sdEnabled;

    QTimer m_timesyncTimer;
    QTimer m_downloadTimer;
    QTimer m_nextRequestTimer;

    LogFileHandler* m_logfile_handler_ptr;
    TimeSyncHandler* m_timesync_handler_ptr;
    QList<DeviceInterface*>* m_device_list;

    QString m_devicesMainState;
    ///
    /// todo: is it final like this? group?
    ///
    quint8 time_sync_state;
    int    remaining_c;
    int    id_in_sync;
    int    id_in_dl;
    int    download_state;
    int    sending_start_state;
    int    m_nextRequest;
    quint8 m_lastCatchSuccess;
    quint32 m_triggerTimestamp;

    ///
    /// todo: is it final like this? group?
    ///

    bool m_devicesConnected = false;

    bool m_sdEnabled;

    bool m_bleUplEnabled;

public:
    CatchController(QList<DeviceInterface*>* devicelist, TimeSyncHandler* ts_handler,
                    LogFileHandler* logfile_handler, QObject *parent = nullptr);
    ~CatchController(){} // todo we need to destroy this safely

    //bool sdEnabled() const;

    bool devicesConnected() const
    {
        return m_devicesConnected;
    }

    bool sdEnabled() const
    {
        return m_sdEnabled;
    }

    bool bleUplEnabled() const
    {
        return m_bleUplEnabled;
    }

signals:
    //    void appStateChanged (quint8 state);
    //    void sdEnabledChanged();

    void timeSyncOfAllDevFinished(bool success);
    void downloadOfAllDevFinished(bool success);

    void allWearablesAreWaitingForDownload(); //QML propmt on this signal for CATCH, DROP, FLOP

    void allSelectedDevicesAreConnected(bool areThey);

    //    void transferProgressChanged(uint8_t percentage);
    //    void showProgressMessage(QString mainText, QString subText, int percent, uint8_t flag);
    //    void progressFinished();

    //void mainStateOfAllDevicesChanged(const QString& devicesMainState);
   // void mainStateOfAllDevicesChanged(QString devicesMainState);
   void mainStateOfAllDevicesChanged();

    void devicesAliveChanged(bool devicesAlive);

    void invokeQmlError(QString error);

    void sdEnabledChanged(bool sdEnabled);

    void bleUplEnabledChanged(bool bleUplEnabled);

public slots:

    void startTimesyncAllDevices(); // QML Start this in background during init dialog
    void onTimeSyncTimerExpired();
    void onTimeSyncOfDevXfinished(bool success, int idx);
    void startTimeSyncOfDevX(int id);

//    void startDownloadFromAllDevices(bool catchSuccess); // QML use this if user clicks catch or drop
    void onDownloadTimerExpired();
    void onDownloadOfDeviceXfinished(bool success, int idx);
    void startDownloadOfDevX(int id);

    void onConnUpdateOfDevXfinished(bool success, int idx); // todo connect both of them

    void sendStartToAllDevices(); // QML use this to restart acquisition

    void sendStopToAllDevices(); // QML make this available while ready to trigger or simpler while !stopped ..

    void onSensorDataAvailableArrived(int idx);

    void onMainStateOfDevXChanged(quint16 state, int idx);

    void onRequestDispatchToOtherDevices(QByteArray value, int idx);

    void onConnAliveOfDevXChanged(bool isItAlive, int idx);

    void setLoggingMedia(bool toSd, bool sendOverBle); // QML, use this during startup dialog

    void onNextRequestTimerExpired();

    void onCatchSuccessConfirmed(quint8 catchSuccess);
};

#endif // CATCHCONTROLLER_H
