#ifndef PAINTDATA_H
#define PAINTDATA_H

#include <QObject>
#include <mci_catch_detection.h>
#include <QDebug>

#define TYPE_AUD 1u
#define	TYPE_ACC 2u
#define TYPE_GYR 3u
#define TYPE_MAG 4u
#define TYPE_PRS 5u
#define TYPE_LOG 0xAA
#define TYPE_COUNT 5

class PaintData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString paintName READ getName WRITE setName NOTIFY paintNameChanged)
    Q_PROPERTY(int paintType READ getType NOTIFY typeChanged)
    Q_PROPERTY(char paintSide READ getSide NOTIFY sideChanged)
    //
public:
    PaintData(QObject *parent);
    ~PaintData();
    PaintData(uint8_t type, char side, QVector<QVariant> data);

    void setSamplingFreq(double val);
    void setStepSize(double val);
    double getSamplingFreq();
    double getStepSize();
    void setPaintBuffer (QVector<QVariant> vec);
    void setName (QString name);
    QVector<QVariant> getPaintBuffer();
    QVector<double> getTimeVector();
    QString getName();
    uint8_t getType();
    char getSide();
    QString getSideStr();

private:
    QString m_name;
    uint8_t m_type;
    char m_side;
    double m_samplingFreq;
    double m_step;
    QVector<QVariant> m_paintBuffer;
    QVector<double> m_timeVec;

signals:
    void paintBufferChanged();
    void timePeriodChanged();
    void paintNameChanged();
    void typeChanged();
    void sideChanged();

public slots:
    void onTimePeriodChanged();
};

#endif // PAINTDATA_H
