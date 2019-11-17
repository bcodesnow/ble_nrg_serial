#ifndef LOGFILEHANDLER_H
#define LOGFILEHANDLER_H

#include <QObject>
#include <QDataStream>
#include <QVariant>
#include <QDateTime>
#include <QDir>
#include <QDebug>

#define PLOT_DATA 1
#define VERBOSITY_LEVEL 1
#define ALLOW_WRITE_TO_FILE 1
#define WRITE_BERNHARD_INFO_TO_LOG_FILE 1

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

private:
    quint64 m_currFileIndex;
    QString m_lastPath;
    quint8  m_lastType; // and last type make out when it increments.
    QString* m_logFilBuf;
    QString m_fileLocation;
    QString m_currUser;
    QString m_currDir;
    QString m_currCatchMode;
    QList<QObject*> m_paintDataList;

    quint64 m_fileIndex;

signals:
    void lastPathChanged(void);
    void idxChanged(quint64 tidx);

    void paintDataListChanged();
    void updateAllPainters(QList<QObject*> datalist);

    void invokeCreateGoogleFolder(QString name);
    void invokeGoogleUpload(QString filename, QByteArray data);

    void fileIndexChanged();

public:
    explicit LogFileHandler(QObject *parent = nullptr);

    void sortArray(QByteArray *arr, uint16_t wp);
    QVector<QVariant> bytesToInt16(QByteArray arr, uint16_t step = 0);
    QVector<QVariant> bytesToFloat32(QByteArray arr, uint16_t step = 0);


    QString getHomeLocation();
    QVariant getPaintDataList();


public slots:
    void finishLogFile();
    void incrementFileIndex();
    void addToLogFil(QString ident, QString key, QString val);
    void writeTypeToLogFil(QString ident, QByteArray* data, quint8 type, quint16 wp);
    void resetFileIndex(); // UNUSED TODO

    void setCurrDir (QString username, bool g_enabled); //TODO Dominique
    void setCurrCatchMode (QString mode);
};

#endif // LOGFILEHANDLER_H


