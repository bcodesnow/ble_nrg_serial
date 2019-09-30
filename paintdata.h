#ifndef PAINTDATA_H
#define PAINTDATA_H

#include <QObject>
#include <ble_uart.h>
#include <QDebug>

class PaintData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString paintName READ getName WRITE setName NOTIFY paintNameChanged)
    Q_PROPERTY(int paintType READ getType NOTIFY typeChanged)
    Q_PROPERTY(char paintSide READ getSide NOTIFY sideChanged)
    //
public:
    PaintData(QObject *parent)
    {
        Q_UNUSED(parent);
        m_name = "undefined";
    }
    PaintData(uint8_t type, char side, QVector<QVariant> data)
    {
        connect(this, SIGNAL(timePeriodChanged()), this, SLOT(onTimePeriodChanged()));

        m_type = type;
        m_side = side;
        setPaintBuffer(data);
        switch (m_type)
        {
        case TYPE_AUD:
            setName(QString("Audio ").append(side));
            setSamplingFreq(FREQ_AUD);
            break;
        case TYPE_GYR:
            setName(QString("Gyro ").append(side));
            setSamplingFreq(FREQ_GYR);
            break;
        case TYPE_ACC:
            setName(QString("Accelero ").append(side));
            setSamplingFreq(FREQ_ACC);
            break;
        case TYPE_PRS:
            setName(QString("Pressure ").append(side));
            setSamplingFreq(FREQ_PRS);
            break;
        case TYPE_MAG:
            setName(QString("Magneto ").append(side));
            setSamplingFreq(FREQ_MAG);
            break;
        case TYPE_LOG:
            qDebug()<<"Can't paint a logfile";
            break;
        default:
            qDebug()<<"Can't paint an unknown file";
            break;
        }
        m_enabled = true;
        emit typeChanged();
        emit sideChanged();
        //qDebug()<<"New Paintdata:"<<m_name<<m_paintBuffer.size();
    }

    void setSamplingFreq(double val)
    {
        m_samplingFreq = val;
        m_step = 1/m_samplingFreq;
        emit timePeriodChanged();
        return;
    }

    void setStepSize(double val)
    {
        m_step = val;
        m_samplingFreq = 1/m_step;
        emit timePeriodChanged();
        return;
    }

    double getSamplingFreq()
    {
        return m_samplingFreq;
    }

    double getStepSize()
    {
        return  m_step;
    }

    void setPaintBuffer (QVector<QVariant> vec)
    {
        m_paintBuffer = vec;
        emit paintBufferChanged();
    }

    void setName (QString name)
    {
        m_name = name;
        emit paintNameChanged();
    }

    QVector<QVariant> getPaintBuffer()
    {
        return m_paintBuffer;
    }

    QVector<double> getTimeVector()
    {
        return m_timeVec;
    }

    QString getName()
    {
        return m_name;
    }

    uint8_t getType()
    {
        return m_type;
    }

    char getSide()
    {
        return m_side;
    }

    QString getSideStr()
    {
        qDebug()<<"TEEEEEEEEEEST"<<QString(m_side);
        return QString(m_side);
    }

    bool getEnabled()
    {
        return m_enabled;
    }


private:
    QString m_name;
    uint8_t m_type;
    char m_side;
    double m_samplingFreq;
    double m_step;
    bool m_enabled = false;
    QVector<QVariant> m_paintBuffer;
    QVector<double> m_timeVec;

signals:
    void paintBufferChanged();
    void timePeriodChanged();
    void paintNameChanged();
    void typeChanged();
    void sideChanged();

public slots:
    void onTimePeriodChanged()
    {
        if (m_type == TYPE_AUD || m_type == TYPE_PRS) {
            // timevector for 1 axis
            m_timeVec.reserve(m_paintBuffer.size());
            for (int i=0;i<m_paintBuffer.size();i++) {
                m_timeVec.append(i*m_step);
            }
        }
        else if (m_type == TYPE_ACC || m_type == TYPE_GYR || m_type == TYPE_MAG) {
            // timevector for 3 axis (x,y,z)
            m_timeVec.reserve(m_paintBuffer.size()/3);
            for (int i=0;i<m_paintBuffer.size()/3;i++) {
                m_timeVec.append(i*m_step);
            }
        }
        else {
            qDebug()<<"Invalid type time series";
            return;
        }
        return;
    }
};

#endif // PAINTDATA_H
