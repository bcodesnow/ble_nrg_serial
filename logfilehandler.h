#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include "global.h"
#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QDateTime>
#include <QDir>
#include <QDebug>

#include "mci_catch_detection.h"
#include "ble_uart.h"
#include <graphpainter.h>
#include <paintdata.h>


#define SEP_CHAR " : "
#define LINE_END ";\n"

class LogFileHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint64 fileIndex MEMBER m_currFileIndex NOTIFY fileIndexChanged)
    Q_PROPERTY(QString lastPath MEMBER m_lastPath NOTIFY lastPathChanged)
    Q_PROPERTY(QVariant paintDataList READ getPaintDataList NOTIFY paintDataListChanged)
    Q_PROPERTY(QStringList currentModeIndexCatch MEMBER m_catchModeList NOTIFY catchModeListChanged) // todo: move to catchcontroller
    Q_PROPERTY(bool googleEnabled MEMBER m_googleEnabled NOTIFY googleEnabledChanged) // todo: move to catchcontroller

private:
    quint64 m_currFileIndex;
    QString m_lastPath;
    quint8  m_lastType; // and last type make out when it increments.
    QString* m_logFilBuf;
    QString m_fileLocation;
    QString m_currUser;
    QString m_currDir;
    QString m_currCatchMode;
    bool m_googleEnabled; // todo: move to catchcontroller

    QStringList m_catchModeList = {"Mixed","Standing","Running","Jumping","One hand"}; // todo: move to catchcontroller
    QList<QObject*> m_paintDataList;


    quint64 m_fileIndex;

signals:
    void lastPathChanged(void);
    void idxChanged(quint64 tidx);

    void catchModeListChanged();
    void paintDataListElementChanged(quint32 elementIndex);
    void updateAllPainters(QList<QObject*> datalist);

    void invokeCreateGoogleFolder(QString name);
    void invokeGoogleUpload(QString filename, QByteArray *data);

    void fileIndexChanged();

    void paintDataListChanged();

    void googleEnabledChanged();

public:
    explicit LogFileHandler(QObject *parent = nullptr);

    void sortArray(QByteArray *arr, uint16_t wp);
    QVector<QVariant> bytesToInt16(QByteArray *arr, uint16_t step = 1);
    QVector<QVariant> bytesTo3AxisInt16(QByteArray *arr, uint16_t step = 1);
    QVector<QVariant> bytesToFloat32(QByteArray *arr, uint16_t step = 1);


    QString getHomeLocation();
    QVariant getPaintDataList();


public slots:
    void finishLogFile();
    void incrementFileIndex();
    void addToLogFil(QString ident, QString key, QString val);
    void writeTypeToLogFil(QString ident, QByteArray* data, quint8 type, quint16 wp);
    void resetFileIndex(); // UNUSED TODO

    void setCurrDir (QString username);
    void setCurrCatchMode (int mode);

};

#endif // LOGFILEHANDLER_H


