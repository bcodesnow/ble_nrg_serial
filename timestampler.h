#ifndef TIMESTAMPLER_H
#define TIMESTAMPLER_H

#include <QTimer>
#include <QElapsedTimer>
#include <devicehandler.h>


class TimeStampler : public QElapsedTimer
{
private:
    qint64 m_lastTimeStamp;
    qint64 m_mode; // TIMESTAMP + MESSAGE TIMER
    DeviceHandler* m_device_handler_arr;

public:
    TimeStampler(DeviceHandler* device_handler_arr )
    {
    }
    void startResponseDelayTimer(quint16 delayInMs, quint8 idOfDeviceHandlerRequesting)
    {

    }
    // elapsed returns ms


};

#endif // TIMESTAMPLER_H
