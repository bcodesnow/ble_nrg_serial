#include "paintdata.h"

//PaintData::PaintData(QObject *parent) : QObject(parent)
//{

//}

PaintData::PaintData()
{
    m_name = "undefined";
}

PaintData::~PaintData() {
}

PaintData::PaintData(uint8_t type, char side, QVector<QVariant> data)
{
    connect(this, SIGNAL(timePeriodChanged()), this, SLOT(onTimePeriodChanged()));

    m_type = type;
    m_side = side;
    setPaintBuffer(data);
    switch (m_type)
    {
    case TYPE_AUD:
        setName(QString("Audio ").append(side));
        setSamplingFreq(AUDIO_SAMPLING_FREQUENCY);
        break;
    case TYPE_GYR:
        setName(QString("Gyro ").append(side));
        setSamplingFreq(GYR_SAMPLING_FREQUENCY);
        break;
    case TYPE_ACC:
        setName(QString("Accelero ").append(side));
        setSamplingFreq(ACC_SAMPLING_FREQUENCY);
        break;
    case TYPE_PRS:
        setName(QString("Pressure ").append(side));
        setSamplingFreq(PRS_SAMPLING_FREQUENCY);
        break;
    case TYPE_MAG:
        setName(QString("Magneto ").append(side));
        setSamplingFreq(MAG_SAMPLING_FREQUENCY);
        break;
    case TYPE_LOG:
        qDebug()<<"Can't paint a logfile";
        break;
    default:
        qDebug()<<"Can't paint an unknown file";
        break;
    }
    emit typeChanged();
    emit sideChanged();
}

void PaintData::setSamplingFreq(double val)
{
    m_samplingFreq = val;
    m_step = 1/m_samplingFreq;
    emit timePeriodChanged();
    return;
}

void PaintData::setStepSize(double val)
{
    m_step = val;
    m_samplingFreq = 1/m_step;
    emit timePeriodChanged();
    return;
}

double PaintData::getSamplingFreq()
{
    return m_samplingFreq;
}

double PaintData::getStepSize()
{
    return  m_step;
}

void PaintData::setPaintBuffer(QVector<QVariant> vec)
{
    m_paintBuffer = vec;
    emit paintBufferChanged();
}

void PaintData::setName(QString name)
{
    m_name = name;
    emit paintNameChanged();
}

QVector<QVariant> PaintData::getPaintBuffer()
{
    return m_paintBuffer;
}

QVector<double> PaintData::getTimeVector()
{
    return m_timeVec;
}

QString PaintData::getName()
{
    return m_name;
}

uint8_t PaintData::getType()
{
    return m_type;
}

char PaintData::getSide()
{
    return m_side;
}

QString PaintData::getSideStr()
{
    return QString(m_side);
}

void PaintData::onTimePeriodChanged()
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
    return;
}
