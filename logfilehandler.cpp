#include "logfilehandler.h"
#include <QStandardPaths>
#include <QFile>
#include <QtDebug>
#include "ble_uart.h"
#include <QString>

#include <QElapsedTimer> // delete later

LogFileHandler::LogFileHandler(QObject *parent) : QObject(parent),
    m_currFileIndex(0), m_lastType(0),
    m_lastPath("NONE")
{
    m_logFilBuf = new QString();

#if (defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID))
    m_fileLocation = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);

#endif

#if (defined(Q_OS_ANDROID))
    m_fileLocation = QStandardPaths::locate(QStandardPaths::DownloadLocation, QString(), QStandardPaths::LocateDirectory);
#endif

    m_paintDataList.reserve(TYPE_COUNT*2);
    m_paintDataList.append(new PaintData(TYPE_AUD, QString("L").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_AUD, QString("R").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_ACC, QString("L").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_ACC, QString("R").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_GYR, QString("L").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_GYR, QString("R").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_MAG, QString("L").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_MAG, QString("R").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_PRS, QString("L").at(0).toLatin1(), QVector<QVariant>(0)));
    m_paintDataList.append(new PaintData(TYPE_PRS, QString("R").at(0).toLatin1(), QVector<QVariant>(0)));
}

void LogFileHandler::sortArray(QByteArray *arr, uint16_t wp)
{
    QByteArray tmpArray = arr->left(wp);
    arr->remove(0,wp);
    arr->append(tmpArray);
}

QVector<QVariant> LogFileHandler::bytesToInt16(QByteArray arr, uint16_t step)
{
    int16_t tmp_int;
    QVector<QVariant> intvec;//(QVector<QVariant>(arr.size()/2));
    intvec.reserve(arr.size()/2);

    for (int i=0; i<arr.size(); i+=2)
    {
        tmp_int = static_cast<int16_t>(static_cast<int16_t>(arr[i+1]) << 8);
        tmp_int |= static_cast<int16_t>( arr[i] ) & 0xFF;
        intvec.append(QVariant(tmp_int));
    }
    return intvec;
}

//QVector<QVariant> LogFileHandler::bytesToInt16(QByteArray arr)
//{
//    int16_t tmp_int;
//    QVector<QVariant> intvec;//(QVector<QVariant>(arr.size()/2));
//    intvec.reserve(arr.size()/2);

//    for (int i=0; i<arr.size(); i+=2)
//    {
//        tmp_int = static_cast<int16_t>(static_cast<int16_t>(arr[i+1]) << 8);
//        tmp_int |= static_cast<int16_t>( arr[i] ) & 0xFF;
//        intvec.append(QVariant(tmp_int));
//    }
//    return intvec;
//}

QVector<QVariant> LogFileHandler::bytesToFloat32(QByteArray arr, uint16_t step)
{
    float tmp_float;
    QByteArray tmp_arr;
    QVector<QVariant> floatvec;//(QVector<QVariant>(arr.size()/4));
    floatvec.reserve(arr.size()/4);

    for (int i=0;i<arr.size();i=i+4)
    {
        tmp_arr = arr.mid(i,4);
        QDataStream stream(tmp_arr);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision); // for float (4-bytes / 32-bits)
        stream.setByteOrder(QDataStream::LittleEndian);
        stream >> tmp_float;
        floatvec.append(QVariant(tmp_float));
    }
    return floatvec;
}

void LogFileHandler::writeTypeToLogFil(QString ident, QByteArray* data, quint8 type, quint16 wp)
{

#if (PLOT_DATA == 1)
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"writeTypeToLogFil"<<ident<<type<<wp<<data->size();
#endif
#else
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"writeTypeToLogFil disabled";
#endif
    return;
#endif

    QElapsedTimer filetimer;
    filetimer.start();

    QVector<QVariant> dataVec(QVector<QVariant>(0));
    // get target location
    QString tmpLocation = m_fileLocation+m_currDir+"/";
    qDebug()<<"File Path: "<<tmpLocation;
    QString idx_str = tr("%1_%2_").arg(m_currFileIndex).arg(ident);
    //  tmpLocation.append( idx_str );

    // sort by writepointer
    if (type != TYPE_LOG && !data->isEmpty())
        sortArray(data, wp);

#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"after sorting:"<<QString::number(static_cast<double>(filetimer.nsecsElapsed())/1000000, 'f', 2)<<"ms";
#endif
    // Byte conversion
    switch (type)
    {
    case TYPE_AUD:
        dataVec = bytesToInt16(*data);
        idx_str.append( QString("AUDIO") );
        break;
    case TYPE_GYR:
        dataVec = bytesToInt16(*data);
        idx_str.append( QString("GYR") );
        break;
    case TYPE_ACC:
        dataVec = bytesToInt16(*data);
        idx_str.append( QString("ACC") );
        break;
    case TYPE_PRS:
        dataVec = bytesToFloat32(*data);
        idx_str.append( QString("PRS") );
        break;
    case TYPE_MAG:
        dataVec = bytesToInt16(*data);
        idx_str.append( QString("MAG") );
        break;
    case TYPE_LOG:
        idx_str.append( QString("LOG") );
        break;
    default:
        tmpLocation.append( QString("SOMEFILE") );
    }
    tmpLocation.append(idx_str);
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"after conversion:"<<QString::number(static_cast<double>(filetimer.nsecsElapsed())/1000000, 'f', 2)<<"ms";
#endif
    if (type != TYPE_LOG && !dataVec.isEmpty())
    {
        for (int i=0;i<m_paintDataList.size();i++)
        {
            if (((PaintData*)m_paintDataList.at(i))->getType() == type &&
                    ((PaintData*)m_paintDataList.at(i))->getSide() == ident.at(0).toLatin1())
            {
                m_paintDataList.at(i)->deleteLater();
                m_paintDataList.replace(i,new PaintData(type, ident.at(0).toLatin1(), dataVec));
                emit paintDataListChanged();
            }
        }
    }
#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"after painting:"<<QString::number(static_cast<double>(filetimer.nsecsElapsed())/1000000, 'f', 2)<<"ms";
#endif

    // save as text
    QFile file(tmpLocation);
    file.open(QIODevice::WriteOnly);
    file.write(*data);
    file.close();

    // Google drive
    invokeGoogleUpload(idx_str,*data);

#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"after saving:"<<QString::number(static_cast<double>(filetimer.nsecsElapsed())/1000000, 'f', 2)<<"ms";
#endif

#if (VERBOSITY_LEVEL >= 1)
    qDebug()<<"writeTypeToLogFil took:"<<QString::number(static_cast<double>(filetimer.nsecsElapsed())/1000000, 'f', 2)<<"ms";
#endif
}

void LogFileHandler::addToLogFil(QString ident, QString key, QString val)
{
#ifdef ALLOW_WRITE_TO_FILE
    QString tmpString = ident + SEP_CHAR + key + SEP_CHAR + val + LINE_END;
    m_logFilBuf->append(tmpString);
#endif
}

void LogFileHandler::resetFileIndex()
{
    m_currFileIndex = 0;
    emit fileIndexChanged();
}

void LogFileHandler::setCurrDir(QString username, bool g_enabled)
{
    QDir dir;

    if (username.isEmpty())
        m_currUser = "dev";
    else
        m_currUser = username;
    m_currDir = "catch_data_WD_" + m_currUser + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

    if (g_enabled)
        invokeCreateGoogleFolder(m_currDir);

    dir.setPath(m_fileLocation);
    dir.mkdir(m_currDir);
#if ( VERBOSITY_LEVEL >= 0 )
    qInfo()<<"LogFileHandler::setCurrDir(QString username)"<< "\nCreated folder"<<m_currDir<<"in"<<m_fileLocation;
#endif

}

void LogFileHandler::setCurrCatchMode(QString mode)
{
    m_currCatchMode = mode;
}

//void LogFileHandler::confirm (QString ident, bool bcatch)
//{
//    addToLogFil(ident, QString("SUCCESS"), QString(( bcatch ? "CATCH" : "DROP" )) );
//}

void LogFileHandler::finishLogFile()
{
#if ( WRITE_BERNHARD_INFO_TO_LOG_FILE == 1 )
    addToLogFil("Info","Username",m_currUser);
    addToLogFil("Info","CatchMode",m_currCatchMode);
    addToLogFil("Info", QString("RecordingTime"), QString::number(2500)+" ms"); // todo use define from mci catch det
    addToLogFil("Info", QString("freq_AUDIO"), QString::number(8000));
    addToLogFil("Info", QString("freq_ACC"), QString::number(1000));
    addToLogFil("Info", QString("freq_GYR"), QString::number(1000));
    addToLogFil("Info", QString("freq_MAG"), QString::number(100));
    addToLogFil("Info", QString("freq_PRS"), QString::number(100));
#endif
#ifdef ALLOW_WRITE_TO_FILE
    QByteArray ba; // get rid of this..
    ba =  m_logFilBuf->toUtf8();
    writeTypeToLogFil("LR", &ba, TYPE_LOG, 0);
    m_logFilBuf->clear();
#endif
}

void LogFileHandler::incrementFileIndex()
{
    m_currFileIndex++;
}

QString LogFileHandler::getHomeLocation()
{
    return m_fileLocation;
}

QVariant LogFileHandler::getPaintDataList()
{
    return QVariant::fromValue(m_paintDataList);
}
